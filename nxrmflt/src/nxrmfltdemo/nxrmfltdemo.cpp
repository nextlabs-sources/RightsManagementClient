// nxrmfltdemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "listhlp.h"
#include <winternl.h>

#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)

#define MAX_ULONGLONG                   0xFFFFFFFFFFFFFFFF

typedef NTSTATUS (WINAPI *ZWQUERYEAFILE) (
	_In_ HANDLE FileHandle,
	_Out_ PIO_STATUS_BLOCK IoStatusBlock,
	_Out_writes_bytes_(Length) PVOID Buffer,
	_In_ ULONG Length,
	_In_ BOOLEAN ReturnSingleEntry,
	_In_reads_bytes_opt_(EaListLength) PVOID EaList,
	_In_ ULONG EaListLength,
	_In_opt_ PULONG EaIndex,
	_In_ BOOLEAN RestartScan
	);

typedef NTSTATUS (WINAPI *ZWSETEAFILE) (
	_In_ HANDLE FileHandle,
	_Out_ PIO_STATUS_BLOCK IoStatusBlock,
	_In_reads_bytes_(Length) PVOID Buffer,
	_In_ ULONG Length
	);

ULONG __stdcall notification_callback(
	ULONG				type,
	PVOID				msg,
	ULONG				Length,
	PVOID				msgctx,
	PVOID				userctx);

LONG __stdcall dbg_callback(LPCWSTR dbgstr);
BOOL __stdcall dbg_check_level(_In_ ULONG Level);

BOOL WINAPI ControlHandler(DWORD dwCtrlType);

DWORD WINAPI DemoEngWorker(_In_ LPVOID lpParameter);

HANDLE hStopEvent = NULL;

typedef struct _DEMO_ENG{

	HANDLE				hStop;

	HANDLE				hSemQueue;

	NXRMFLT_HANDLE		hNxrmFlt;

	HANDLE				hThreads[4];

	ULONG				ThreadId[4];

	LIST_ENTRY			Queue;

	CRITICAL_SECTION	QueueLock;

} DEMO_ENG, *PDEMO_ENG;

BOOL process_check_rights_request(DEMO_ENG &eng, NXRM_CHECK_RIGHTS_NOTIFICATION &req, PVOID msgctx);

BOOL process_block_notification(DEMO_ENG &eng, NXRM_BLOCK_NOTIFICATION &req, PVOID msgctx);

BOOL process_Keychain_error_notification(DEMO_ENG &eng, NXRM_KEYCHAIN_ERROR_NOTIFICATION &req, PVOID msgctx);

//BOOL test_ea(WCHAR *FileName);


typedef struct _QUEUE_ITEM{

	ULONG							Type;

	PVOID							Notification;

	LIST_ENTRY						Link;

	PVOID							Ctx;

}QUEUE_ITEM,*PQUEUE_ITEM;

#pragma pack(push, 4)

typedef struct _FILE_GET_EA_INFORMATION {
	ULONG NextEntryOffset;
	UCHAR EaNameLength;
	CHAR EaName[1];
} FILE_GET_EA_INFORMATION, *PFILE_GET_EA_INFORMATION;

typedef struct _FILE_FULL_EA_INFORMATION {
	ULONG NextEntryOffset;
	UCHAR Flags;
	UCHAR EaNameLength;
	USHORT EaValueLength;
	CHAR EaName[1];
} FILE_FULL_EA_INFORMATION, *PFILE_FULL_EA_INFORMATION;

#pragma pack(pop)

int _tmain(int argc, _TCHAR* argv[])
{
	ULONG ulRet = 0;

	DEMO_ENG Eng = { 0 };

	ULONG i = 0;

	const UCHAR keyId[0x2c] = {0x4e, 0x4c, 0x5f, 0x53, 0x48, 0x41, 0x52, 0x45, 0x76, 0x07, 0x5b, 0xb7, 0x6a, 0x2d, 0x7f, 0xda, 0x00, 0xea, 0xdc, 0x3e, 0xef, 0x07, 0x2d, 0x3a, 0x8d, 0x27, 0x56, 0xe1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb7, 0x48, 0x07, 0x54};
	const UCHAR Key[32] = {0xfa, 0x1b, 0x3b, 0x89, 0x5b, 0xf6, 0x02, 0x7a, 0x06, 0xe0, 0x76, 0x57, 0xb5, 0x6f, 0xa2, 0x39, 0x42, 0xfb, 0x23, 0x31, 0x0d, 0x14, 0x9d, 0x44, 0xd3, 0xfb, 0xb7, 0x2a, 0xba, 0xe3, 0x6b, 0xdb};

	NXRM_KEY_BLOB *pKeyChain = NULL;
	ULONG KeyChainSizeInByte = 0;

	_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF|_CRTDBG_ALLOC_MEM_DF);
	
	SetConsoleCtrlHandler(ControlHandler,TRUE);

	do 
	{
		InitializeListHead(&Eng.Queue);

		InitializeCriticalSection(&Eng.QueueLock);

		hStopEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

		if (!hStopEvent)
		{
			break;
		}

		Eng.hSemQueue = CreateSemaphoreW(NULL, 0, 8, NULL);
		
		Eng.hStop = CreateEvent(NULL, TRUE, FALSE, NULL);

		for (i =0 ; i < 4; i++)
		{
			Eng.hThreads[i] = CreateThread(NULL, 0, DemoEngWorker, &Eng, CREATE_SUSPENDED, &Eng.ThreadId[i]);
		}

		KeyChainSizeInByte = sizeof(NXRM_KEY_BLOB);

		pKeyChain = (NXRM_KEY_BLOB*)malloc(KeyChainSizeInByte);

		pKeyChain->KeKeyId.Algorithm	= NXRM_ALGORITHM_AES256;
		pKeyChain->KeKeyId.IdSize		= 0x2c;

		memcpy(&pKeyChain->KeKeyId.Id, keyId, min(sizeof(pKeyChain->KeKeyId.Id), sizeof(keyId)));
		memcpy(&pKeyChain->Key, Key, min(sizeof(pKeyChain->Key), sizeof(Key)));

		Eng.hNxrmFlt = nxrmfltCreateManager(notification_callback, dbg_callback, dbg_check_level, pKeyChain, KeyChainSizeInByte, &Eng);

		if (!Eng.hNxrmFlt)
		{
			break;
		}

		for (i = 0; i < 4; i++)
		{
			if (Eng.hThreads[i])
			{
				ResumeThread(Eng.hThreads[i]);
			}
		}

		ulRet = nxrmfltStartFiltering(Eng.hNxrmFlt);

		if (ulRet != ERROR_SUCCESS)
		{
			break;
		}

		//{
		//	HANDLE hFile = INVALID_HANDLE_VALUE;

		//	hFile = CreateFileW(L"C:\\test\\vista.pdf",
		//						GENERIC_READ,
		//						FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
		//						NULL,
		//						OPEN_EXISTING,
		//						FILE_ATTRIBUTE_NORMAL,
		//						NULL);

		//	if (hFile != INVALID_HANDLE_VALUE)
		//	{
		//		CloseHandle(hFile);
		//	}
		//}

		//nxrmfltSetSaveAsForecast(Eng.hNxrmFlt, GetCurrentProcessId(), NULL, L"C:\\test\\test.txt");

		//{
		//	HANDLE hFile = INVALID_HANDLE_VALUE;

		//	DWORD dwBytesWrites = 0;

		//	hFile = CreateFileW(L"C:\\test\\test.txt",
		//						GENERIC_READ|GENERIC_WRITE,
		//						FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
		//						NULL,
		//						OPEN_ALWAYS,
		//						FILE_ATTRIBUTE_NORMAL,
		//						NULL);

		//	if (hFile != INVALID_HANDLE_VALUE)
		//	{
		//		WriteFile(hFile, L"This is a test\n", sizeof(L"This is a test\n"), &dwBytesWrites, NULL);

		//		CloseHandle(hFile);
		//	}

		//}

		WaitForSingleObject(hStopEvent, INFINITE);

		nxrmfltStopFiltering(Eng.hNxrmFlt);

		SetEvent(Eng.hStop);

		WaitForMultipleObjects(4, Eng.hThreads, TRUE, INFINITE);

	} while (FALSE);

	if (Eng.hNxrmFlt)
	{
		nxrmfltCloseManager(Eng.hNxrmFlt);
		Eng.hNxrmFlt = NULL;
	}

	for (i = 0; i < 4; i++)
	{
		if (Eng.hThreads[i])
		{
			CloseHandle(Eng.hThreads[i]);
			Eng.hThreads[i] = NULL;
		}
	}

	if (Eng.hSemQueue)
	{
		CloseHandle(Eng.hSemQueue);
		Eng.hSemQueue = NULL;
	}

	if (Eng.hStop)
	{
		CloseHandle(Eng.hStop);
		Eng.hStop = NULL;
	}

	if (hStopEvent)
	{
		CloseHandle(hStopEvent);
		hStopEvent = NULL;
	}

	if (pKeyChain)
	{
		free(pKeyChain);
		pKeyChain = NULL;
	}

	return 0;
}

ULONG __stdcall notification_callback(
	ULONG				type,
	PVOID				msg,
	ULONG				Length,
	PVOID				msgctx,
	PVOID				userctx)
{
	DEMO_ENG *pEng = (DEMO_ENG*)userctx;

	QUEUE_ITEM *pItem = NULL;
	
	do 
	{
		if (type == NXRMFLT_MSG_TYPE_BLOCK_NOTIFICATION)
		{
			if (Length != sizeof(NXRM_BLOCK_NOTIFICATION))
			{
				break;
			}

			pItem = (QUEUE_ITEM*)malloc(sizeof(QUEUE_ITEM));

			if (!pItem)
			{
				break;
			}

			pItem->Type = NXRMFLT_MSG_TYPE_BLOCK_NOTIFICATION;

			pItem->Notification = malloc(sizeof(NXRM_BLOCK_NOTIFICATION));

			if (!pItem->Notification)
			{
				break;
			}

			memcpy(pItem->Notification,
				   msg,
				   sizeof(NXRM_BLOCK_NOTIFICATION));

			pItem->Ctx = msgctx;

			EnterCriticalSection(&pEng->QueueLock);

			InsertTailList(&pEng->Queue, &pItem->Link);

			LeaveCriticalSection(&pEng->QueueLock);

			ReleaseSemaphore(pEng->hSemQueue, 1, NULL);
			
			break;
		}
		else if (type == NXRMFLT_MSG_TYPE_CHECK_RIGHTS)
		{
			if (Length != sizeof(NXRM_CHECK_RIGHTS_NOTIFICATION))
			{
				break;
			}

			pItem = (QUEUE_ITEM*)malloc(sizeof(QUEUE_ITEM));

			if (!pItem)
			{
				break;
			}

			pItem->Type = NXRMFLT_MSG_TYPE_CHECK_RIGHTS;

			pItem->Notification = malloc(sizeof(NXRM_CHECK_RIGHTS_NOTIFICATION));
			
			if (!pItem->Notification)
			{
				break;
			}

			memcpy(pItem->Notification, 
				   msg, 
				   sizeof(NXRM_CHECK_RIGHTS_NOTIFICATION));

			pItem->Ctx = msgctx;

			EnterCriticalSection(&pEng->QueueLock);

			InsertTailList(&pEng->Queue, &pItem->Link);

			LeaveCriticalSection(&pEng->QueueLock);

			ReleaseSemaphore(pEng->hSemQueue, 1, NULL);

			break;
		}
		else if (type == NXRMFLT_MSG_TYPE_KEYCHAIN_ERROR_NOTIFICATION)
		{
			if (Length != sizeof(NXRM_KEYCHAIN_ERROR_NOTIFICATION))
			{
				break;
			}

			pItem = (QUEUE_ITEM*)malloc(sizeof(QUEUE_ITEM));

			if (!pItem)
			{
				break;
			}

			pItem->Type = NXRMFLT_MSG_TYPE_KEYCHAIN_ERROR_NOTIFICATION;

			pItem->Notification = malloc(sizeof(NXRM_KEYCHAIN_ERROR_NOTIFICATION));

			if (!pItem->Notification)
			{
				break;
			}

			memcpy(pItem->Notification, 
				   msg, 
				   sizeof(NXRM_KEYCHAIN_ERROR_NOTIFICATION));

			pItem->Ctx = msgctx;

			EnterCriticalSection(&pEng->QueueLock);

			InsertTailList(&pEng->Queue, &pItem->Link);

			LeaveCriticalSection(&pEng->QueueLock);

			ReleaseSemaphore(pEng->hSemQueue, 1, NULL);

			break;
		}
		else
		{
			//
			// error
			//
		}
	} while (FALSE);

	return 0;
}

LONG __stdcall dbg_callback(LPCWSTR dbgstr)
{
	return printf("[Demo] %S",dbgstr);
}

BOOL __stdcall dbg_check_level(_In_ ULONG Level)
{
	return TRUE;
}

BOOL WINAPI ControlHandler(DWORD dwCtrlType)
{
	switch ( dwCtrlType )
	{
	case CTRL_BREAK_EVENT:  // use Ctrl+C or Ctrl+Break to simulate
	case CTRL_C_EVENT:      //
	case CTRL_CLOSE_EVENT:
		if(hStopEvent)
		{
			SetEvent(hStopEvent);
		}

		return TRUE;
	}

	return FALSE;
}

DWORD WINAPI DemoEngWorker(_In_ LPVOID lpParameter)
{
	DWORD dwRet = 0;

	BOOL bStop = FALSE;

	LIST_ENTRY *ite = NULL;

	QUEUE_ITEM *pItem = NULL;

	HANDLE hEvents[2] = { 0 };

	DEMO_ENG *pEng = (DEMO_ENG*)lpParameter;

	hEvents[0] = pEng->hSemQueue;
	hEvents[1] = pEng->hStop;

	while(!bStop)
	{
		dwRet = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);

		if (dwRet == WAIT_OBJECT_0)
		{
			EnterCriticalSection(&pEng->QueueLock);

			ite = RemoveHeadList(&pEng->Queue);

			LeaveCriticalSection(&pEng->QueueLock);

			pItem = CONTAINING_RECORD(ite, QUEUE_ITEM, Link);

			switch (pItem->Type)
			{
			case NXRMFLT_MSG_TYPE_KEYCHAIN_ERROR_NOTIFICATION:
				process_Keychain_error_notification(*pEng, *((NXRM_KEYCHAIN_ERROR_NOTIFICATION*)pItem->Notification), pItem->Ctx);
				break;
			case NXRMFLT_MSG_TYPE_CHECK_RIGHTS:
				process_check_rights_request(*pEng, *((NXRM_CHECK_RIGHTS_NOTIFICATION*)pItem->Notification), pItem->Ctx);
				break;
			case NXRMFLT_MSG_TYPE_BLOCK_NOTIFICATION:
				process_block_notification(*pEng, *((NXRM_BLOCK_NOTIFICATION*)pItem->Notification), pItem->Ctx);
				break;
			default:
				break;
			}

			free(pItem->Notification);
			free(pItem);
		}
		else if(dwRet == WAIT_OBJECT_0 + 1)
		{
			bStop = TRUE;
			continue;
		}
		else
		{
			Sleep(100);
		}
	}

	return 0;
}

BOOL process_check_rights_request(DEMO_ENG &eng, NXRM_CHECK_RIGHTS_NOTIFICATION &req, PVOID msgctx)
{
	BOOL bRet = TRUE;

	HRESULT hr = S_OK;

	NXRMFLT_CHECK_RIGHTS_REPLY	chkrighsreply = {0};

	//const ULONGLONG RightsMask = BUILTIN_RIGHT_VIEW|BUILTIN_RIGHT_EDIT|BUILTIN_RIGHT_CLIPBOARD|BUILTIN_RIGHT_ANNOTATE|BUILTIN_RIGHT_DECRYPT|BUILTIN_RIGHT_SAVEAS|BUILTIN_RIGHT_CLASSIFY;
	const ULONGLONG RightsMask = BUILTIN_RIGHT_VIEW;

	printf("Received Check rights request:\n\tPID:\t\t%d\n\tTID:\t\t%d\n\tFile name:\t%S\n", req.ProcessId, req.ThreadId, req.FileName);

	chkrighsreply.RightsMask	= RightsMask;
	chkrighsreply.CustomRights	= 0;
	chkrighsreply.EvaluationId	= 0x000000007f7f8a9aULL;

	hr = nxrmfltReplyMessage(eng.hNxrmFlt, msgctx, &chkrighsreply);

	//hr = nxrmfltReplyMessage(pEng->hNxrmFlt, pItem->Ctx, 0);

	if (FAILED(hr))
	{
		printf("Failed to reply message\n");
	}

	return bRet;
}

BOOL process_block_notification(DEMO_ENG &eng, NXRM_BLOCK_NOTIFICATION &req, PVOID msgctx)
{
	BOOL bRet = TRUE;

	printf("Received block notification:\n\tSID:\t\t%d\n\tPID:\t\t%d\n\tTID:\t\t%d\n\tFile name:\t%S\n", req.SessionId, req.ProcessId, req.ThreadId, req.FileName);

	return bRet;
}

BOOL process_Keychain_error_notification(DEMO_ENG &eng, NXRM_KEYCHAIN_ERROR_NOTIFICATION &req, PVOID msgctx)
{
	BOOL bRet = TRUE;

	printf("Received Keychain error notification:\n\tFile name:\t%S\n", req.FileName);

	return bRet;
}