#include "Stdafx.h"
#include "nxrmfltman.h"
#include "nxrmfltmandef.h"
#include "loghlp.hpp"

using namespace nudf::util::log;

#ifndef MAX_ULONGLONG
#define MAX_ULONGLONG                   0xFFFFFFFFFFFFFFFF
#endif


DECLARE_NXRM_MODULE_FLTMAN();
DECLARE_NXRM_MODULE_FLT();

DECLSPEC_CACHEALIGN	LONG		g_nxrmfltmgrcreated = 0;

DWORD WINAPI nxrmfltmanWorker(_In_ LPVOID lpParameter);
BOOL Win32Path2NTPath(const WCHAR *src, WCHAR *ntpath, ULONG *ccntpath);

NXRMFLT_HANDLE __stdcall nxrmfltCreateManager(
	NXRMFLT_CALLBACK_NOTIFY		NotifyCallback, 
	LOGAPI_LOG					DebugDumpCallback,
	LOGAPI_ACCEPT				DebugDumpCheckLevelCallback,
	NXRM_KEY_BLOB				*KeyChain,
	ULONG						KeyChainSizeInByte,
	PVOID						UserContext)
{
	NXRMFLT_MANAGER	*hMgr = NULL;

	LONG nxrmfltmgrcreated = 0;

	BOOL bFailed = FALSE;

	SYSTEM_INFO SysInfo = { 0 };
	DWORD dwCPUCount = 0;

	DWORD dwThreadCount = 0;

	HRESULT hr = S_OK;

	DWORD i = 0;
	DWORD j = 0;

	NXRMFLT_CONNECTION_CONTEXT *ConnectionCtx = NULL;
	ULONG	ConnectionCtxLen = 0;

	LIST_ENTRY *ite = NULL;
	LIST_ENTRY *tmp = NULL;

	do 
	{
		nxrmfltmgrcreated = InterlockedCompareExchange(&g_nxrmfltmgrcreated, 1, 0);

		if(nxrmfltmgrcreated)
		{
			break;
		}

		hMgr = (NXRMFLT_MANAGER*)malloc(sizeof(NXRMFLT_MANAGER));

		if(!hMgr)
		{
			bFailed = TRUE;
			break;
		}

		memset(hMgr, 0, sizeof(NXRMFLT_MANAGER));

		InitializeListHead(&hMgr->MessageList);

		hMgr->DebugDumpCallback				= DebugDumpCallback;
		hMgr->DebugDumpCheckLevelCallback	= DebugDumpCheckLevelCallback;
		hMgr->NotifyCallback				= NotifyCallback;
		hMgr->UserCtx						= UserContext;
		hMgr->HideNxlExtension				= TRUE;
		hMgr->Stop							= FALSE;
		hMgr->DisableFiltering				= TRUE;
		
		GetNativeSystemInfo(&SysInfo);

		dwCPUCount = max(SysInfo.dwNumberOfProcessors, 1);
		
		dwThreadCount = min(dwCPUCount * 2, NXRMFLTMAN_MAX_PORT_THREADS);

		ConnectionCtxLen = sizeof(NXRMFLT_CONNECTION_CONTEXT) - sizeof(NXRM_KEY_BLOB) + KeyChainSizeInByte;

		if (ConnectionCtxLen > 0xffff)
		{
			bFailed = TRUE;
			break;
		}

		ConnectionCtx = (NXRMFLT_CONNECTION_CONTEXT *)malloc(ConnectionCtxLen);

		if (!ConnectionCtx)
		{
			bFailed = TRUE;
			break;
		}

		ConnectionCtx->HideNxlExtension = TRUE;
		ConnectionCtx->NumberOfKey		= KeyChainSizeInByte/sizeof(NXRM_KEY_BLOB);

		memcpy(ConnectionCtx->KeyChain, KeyChain, KeyChainSizeInByte);

		hr = FilterConnectCommunicationPort(NXRMFLT_MSG_PORT_NAME,
											0,
											ConnectionCtx,
											(WORD)ConnectionCtxLen,
											NULL,
											&hMgr->ConnectionPort);
		if(FAILED(hr))
		{
			bFailed = TRUE;
			break;
		}

		hMgr->Completion = CreateIoCompletionPort(hMgr->ConnectionPort,
												  NULL,
												  0,
												  dwThreadCount);

		if (!hMgr->Completion)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			
			bFailed = TRUE;
			break;
		}

		hMgr->StopEvent = CreateEvent(NULL,
									  TRUE,
									  FALSE,
									  NULL);

		if(!hMgr->StopEvent)
		{
			bFailed = TRUE;
			break;
		}

		for(i = 0; i < dwThreadCount; i++)
		{
			hMgr->WorkerThreadHandle[i] = CreateThread(NULL,
													   0,
													   (LPTHREAD_START_ROUTINE)nxrmfltmanWorker,
													   hMgr,
													   CREATE_SUSPENDED,
													   &hMgr->WorkerThreadId[i]);

			if(!hMgr->WorkerThreadHandle[i])
			{
				bFailed = TRUE;
				break;
			}
		}

		for(i = 0; i < dwThreadCount; i++)
		{
			ResumeThread(hMgr->WorkerThreadHandle[i]);
		}

		for(i = 0; i < dwThreadCount; i++)
		{
			NXRMFLT_MESSAGE *msg = NULL;

			msg = (NXRMFLT_MESSAGE*)malloc(sizeof(NXRMFLT_MESSAGE));

			if(msg)
			{
				memset(msg, 0, sizeof(NXRMFLT_MESSAGE));

				hr = FilterGetMessage(hMgr->ConnectionPort,
									  &msg->Header,
									  FIELD_OFFSET(NXRMFLT_MESSAGE, Ovlp),
									  &msg->Ovlp);

				if(hr == HRESULT_FROM_WIN32(ERROR_IO_PENDING))
				{
					InsertHeadList(&hMgr->MessageList, &msg->Link);

					hr = S_OK;
				}
				else
				{
					free(msg);

					bFailed = TRUE;
					break;
				}
			}
			else
			{
				bFailed = TRUE;
				break;
			}
		}

	} while (FALSE);

	if(bFailed)
	{
		InterlockedExchange(&g_nxrmfltmgrcreated, 0);

		if(hMgr)
		{
			hMgr->Stop = TRUE;

			{
				HANDLE hThreads[NXRMFLTMAN_MAX_PORT_THREADS] = {0};

				ULONG ulThreads = 0;

				if(hMgr->StopEvent)
					SetEvent(hMgr->StopEvent);

				for (i = 0, j = 0; i < NXRMFLTMAN_MAX_PORT_THREADS; i++)
				{
					if(hMgr->WorkerThreadHandle[i])
					{
						hThreads[j] = hMgr->WorkerThreadHandle[i];
						j++;
						ulThreads++;
					}
				}

				if(hMgr->ConnectionPort)
				{
					CancelIoEx(hMgr->ConnectionPort, NULL);
				}

				WaitForMultipleObjects(ulThreads, hThreads, TRUE, 30*1000);
			}

			for (i = 0; i < NXRMFLTMAN_MAX_PORT_THREADS; i++)
			{
				if(hMgr->WorkerThreadHandle[i])
				{
					CloseHandle(hMgr->WorkerThreadHandle[i]);
					hMgr->WorkerThreadHandle[i] = NULL;
					hMgr->WorkerThreadId[i] = 0;
				}
			}

			if(hMgr->StopEvent)
			{
				CloseHandle(hMgr->StopEvent);
				hMgr->StopEvent = FALSE;
			}

			if(hMgr->ConnectionPort)
			{
				CloseHandle(hMgr->ConnectionPort);
				hMgr->ConnectionPort = NULL;
			}

			if(hMgr->Completion)
			{
				CloseHandle(hMgr->Completion);
				hMgr->Completion = NULL;
			}

			FOR_EACH_LIST_SAFE(ite,tmp,&hMgr->MessageList)
			{
				NXRMFLT_MESSAGE *pNode = CONTAINING_RECORD(ite, NXRMFLT_MESSAGE, Link);

				RemoveEntryList(ite);

				free(pNode);
			}

			free(hMgr);

			hMgr = NULL;
		}
	}
	
	if (ConnectionCtx)
	{
		free(ConnectionCtx);
		ConnectionCtx = NULL;
	}

	return (NXRMFLT_HANDLE)hMgr;
}

ULONG __stdcall nxrmfltStopFiltering(NXRMFLT_HANDLE hMgr)
{
	NXRMFLT_MANAGER *pMgr = NULL;

	pMgr = (NXRMFLT_MANAGER *)hMgr;

	pMgr->DisableFiltering = TRUE;

	return ERROR_SUCCESS;
}

ULONG __stdcall nxrmfltStartFiltering(NXRMFLT_HANDLE hMgr)
{
	NXRMFLT_MANAGER *pMgr = NULL;

	pMgr = (NXRMFLT_MANAGER *)hMgr;

	pMgr->DisableFiltering = FALSE;

	return ERROR_SUCCESS;
}

ULONG __stdcall nxrmfltCloseManager(NXRMFLT_HANDLE hMgr)
{
	ULONG uRet = ERROR_SUCCESS;

	HANDLE hThreads[NXRMFLTMAN_MAX_PORT_THREADS] = {0};
	ULONG ulThreads = 0;

	ULONG i = 0;
	ULONG j = 0;

	LIST_ENTRY *ite = NULL;
	LIST_ENTRY *tmp = NULL;

	NXRMFLT_MANAGER *pMgr = NULL;

	pMgr = (NXRMFLT_MANAGER *)hMgr;

	do 
	{
		if(!hMgr)
		{
			uRet = ERROR_INVALID_PARAMETER;
			break;
		}

		pMgr->Stop = TRUE;

		if(pMgr->StopEvent)
			SetEvent(pMgr->StopEvent);

		for (i = 0, j = 0; i < NXRMFLTMAN_MAX_PORT_THREADS; i++)
		{
			if(pMgr->WorkerThreadHandle[i])
			{
				hThreads[j] = pMgr->WorkerThreadHandle[i];
				j++;
				ulThreads++;
			}
		}

		if(pMgr->ConnectionPort)
		{
			CancelIoEx(pMgr->ConnectionPort, NULL);
		}

		WaitForMultipleObjects(ulThreads, hThreads, TRUE, 30*1000);

		for (i = 0; i < NXRMFLTMAN_MAX_PORT_THREADS; i++)
		{
			if(pMgr->WorkerThreadHandle[i])
			{
				CloseHandle(pMgr->WorkerThreadHandle[i]);
				pMgr->WorkerThreadHandle[i] = NULL;
				pMgr->WorkerThreadId[i] = 0;
			}
		}

		if(pMgr->StopEvent)
		{
			CloseHandle(pMgr->StopEvent);
			pMgr->StopEvent = FALSE;
		}

		if(pMgr->ConnectionPort)
		{
			CloseHandle(pMgr->ConnectionPort);
			pMgr->ConnectionPort = NULL;
		}

		if(pMgr->Completion)
		{
			CloseHandle(pMgr->Completion);
			pMgr->Completion = NULL;
		}

		FOR_EACH_LIST_SAFE(ite,tmp,&pMgr->MessageList)
		{
			NXRMFLT_MESSAGE *pNode = CONTAINING_RECORD(ite, NXRMFLT_MESSAGE, Link);

			RemoveEntryList(ite);

			free(pNode);
		}

		free(hMgr);

		hMgr = NULL;

		InterlockedExchange(&g_nxrmfltmgrcreated, 0);

	} while (FALSE);

	return 0;
}

DWORD WINAPI nxrmfltmanWorker(_In_ LPVOID lpParameter)
{
	HRESULT hr = S_OK;

	NXRMFLT_MANAGER *pMgr = NULL;

	NXRMFLT_MESSAGE *msg = NULL;
	OVERLAPPED *pOvlp = NULL;
	NXRM_CHECK_RIGHTS_REPLY *reply = NULL;
	NXRM_CHECK_RIGHTS_REPLY default_reply = {0};

	DWORD outSize = 0;
	ULONG_PTR key = 0;

	BOOL success = FALSE;
	BOOL DisableFiltering = FALSE;	

	pMgr = (NXRMFLT_MANAGER *)lpParameter;

	while(!pMgr->Stop)
	{
		msg = NULL;
		reply = NULL;

		success = GetQueuedCompletionStatus(pMgr->Completion, &outSize, &key, &pOvlp, INFINITE);

		if(!success)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());

			//
			//  The completion port handle associated with it is closed 
			//  while the call is outstanding, the function returns FALSE, 
			//  *lpOverlapped will be NULL, and GetLastError will return ERROR_ABANDONED_WAIT_0
			//

			if (hr == E_HANDLE) 
			{
				LOGMAN_ERROR(pMgr, hr, L"Completion port becomes unavailable.");
				hr = S_OK;
			}
			else if (hr == HRESULT_FROM_WIN32(ERROR_ABANDONED_WAIT_0)) 
			{
				LOGMAN_ERROR(pMgr, hr, L"Completion port was closed.");
				hr = S_OK;
			}

			continue;
		}

		msg = CONTAINING_RECORD(pOvlp, NXRMFLT_MESSAGE, Ovlp);

		//
		// use in stack variable in case other thread change pMgr->DisableFiltering
		// between two ifs below
		//
		DisableFiltering = pMgr->DisableFiltering;

		if(pMgr->NotifyCallback && (!DisableFiltering))
		{
			switch (msg->Notification.Type)
			{
			case NXRMFLT_MSG_TYPE_CHECK_RIGHTS:

				reply = (NXRM_CHECK_RIGHTS_REPLY *) malloc(sizeof(NXRM_CHECK_RIGHTS_REPLY));

				if(reply)
				{
					reply->ReplyHeader.MessageId		= msg->Header.MessageId;
					reply->ReplyHeader.Status			= 0;
					reply->CheckRigtsReply.RightsMask	= 0;
					reply->CheckRigtsReply.CustomRights	= 0;
					reply->CheckRigtsReply.EvaluationId = MAX_ULONGLONG;

					pMgr->NotifyCallback(NXRMFLT_MSG_TYPE_CHECK_RIGHTS, (PVOID)(&(msg->Notification.CheckRightsMsg)), sizeof(msg->Notification.CheckRightsMsg), (PVOID)reply, pMgr->UserCtx);
				}

				break;

			case NXRMFLT_MSG_TYPE_BLOCK_NOTIFICATION:

				pMgr->NotifyCallback(NXRMFLT_MSG_TYPE_BLOCK_NOTIFICATION, (PVOID)(&(msg->Notification.BlockMsg)), sizeof(msg->Notification.BlockMsg), NULL, pMgr->UserCtx);

				break;

			case NXRMFLT_MSG_TYPE_KEYCHAIN_ERROR_NOTIFICATION:

				pMgr->NotifyCallback(NXRMFLT_MSG_TYPE_KEYCHAIN_ERROR_NOTIFICATION, (PVOID)(&(msg->Notification.KeyChainErrorMsg)), sizeof(msg->Notification.KeyChainErrorMsg), NULL, pMgr->UserCtx);

				break;

			case NXRMFLT_MSG_TYPE_PURGE_CACHE_NOTIFICATION:

				pMgr->NotifyCallback(NXRMFLT_MSG_TYPE_PURGE_CACHE_NOTIFICATION, (PVOID)(&(msg->Notification.PurgeCacheMsg)), sizeof(msg->Notification.PurgeCacheMsg), NULL, pMgr->UserCtx);

				break;

			case NXRMFLT_MSG_TYPE_PROCESS_NOTIFICATION:

				pMgr->NotifyCallback(NXRMFLT_MSG_TYPE_PROCESS_NOTIFICATION, (PVOID)(&(msg->Notification.ProcessMsg)), sizeof(msg->Notification.ProcessMsg), NULL, pMgr->UserCtx);

				break;
			default:
				break;
			}

		}

		//
		// if message is a checkrighs message AND in the case of no callback or can't allocate memory or filtering is disabled, default allow
		//
		if((pMgr->NotifyCallback == NULL || reply == NULL || DisableFiltering) && msg->Notification.Type == NXRMFLT_MSG_TYPE_CHECK_RIGHTS)
		{
			default_reply.ReplyHeader.MessageId			= msg->Header.MessageId;
			default_reply.ReplyHeader.Status			= 0;
			default_reply.CheckRigtsReply.RightsMask	= MAX_ULONGLONG;
			default_reply.CheckRigtsReply.CustomRights	= MAX_ULONGLONG;
			default_reply.CheckRigtsReply.EvaluationId	= MAX_ULONGLONG;

			hr = FilterReplyMessage(pMgr->ConnectionPort,
									&default_reply.ReplyHeader,
									sizeof(default_reply));

			if(FAILED(hr))
			{
				LOGMAN_ERROR(pMgr, hr, L"Failed to reply to nxrmflt!");
			}
		}

		if(pMgr->Stop)
		{
			continue;
		}

		hr = FilterGetMessage(pMgr->ConnectionPort,
							  &msg->Header,
							  FIELD_OFFSET(NXRMFLT_MESSAGE, Ovlp),
							  &msg->Ovlp);

		if(hr == HRESULT_FROM_WIN32(ERROR_OPERATION_ABORTED))
		{
			LOGMAN_ERROR(pMgr, hr, L"FilterGetMessage aborted!");
			continue;
		}
		else if(hr != HRESULT_FROM_WIN32(ERROR_IO_PENDING))
		{
			LOGMAN_ERROR(pMgr, hr, L"Failed to get message from nxrmflt!");
			continue;
		}

	}

	return hr;
}

HRESULT __stdcall nxrmfltReplyMessage(
	NXRMFLT_HANDLE				hMgr, 
	PVOID						msgctx, 
	NXRMFLT_CHECK_RIGHTS_REPLY	*chkrighsreply)
{
	HRESULT	hr = S_OK;

	NXRMFLT_MANAGER *pMgr = NULL;
	NXRM_CHECK_RIGHTS_REPLY *reply = NULL;

	pMgr = (NXRMFLT_MANAGER *)hMgr;
	reply = (NXRM_CHECK_RIGHTS_REPLY *)msgctx;

	memcpy(&reply->CheckRigtsReply,
		   chkrighsreply,
		   min(sizeof(reply->CheckRigtsReply), sizeof(*chkrighsreply)));

	do 
	{

		hr = FilterReplyMessage(pMgr->ConnectionPort,
								&reply->ReplyHeader,
								sizeof(FILTER_REPLY_HEADER) + sizeof(reply->CheckRigtsReply));

		if(FAILED(hr))
		{
			LOGMAN_ERROR(pMgr, hr, L"Failed to reply message to nxrmflt!");
		}

	} while (FALSE);

	if(reply)
	{
		memset(reply, 0, sizeof(NXRM_CHECK_RIGHTS_REPLY));
		free(reply);
	}

	return hr;
}

HRESULT NXRMFLTMAN_API __stdcall nxrmfltSetSaveAsForecast(
	NXRMFLT_HANDLE				hMgr,
	ULONG						ProcessId,
	CONST WCHAR					*SrcFileName,
	CONST WCHAR					*SaveAsFileName)
{
	HRESULT	hr = S_OK;

	NXRMFLT_MANAGER *pMgr = NULL;

	NXRMFLT_COMMAND_MSG	*Msg = NULL;

	NXRMFLT_SAVEAS_FORECAST *pSaveAsForecast = NULL;

	ULONG CommandLength = 0;

	ULONG BytesReturn = 0;

	pMgr = (NXRMFLT_MANAGER *)hMgr;

	WCHAR NTPath[MAX_PATH] = { 0 };
	ULONG ccNTPath = 0;

	DWORD dwret;

	do 
	{
		if (!SaveAsFileName)
		{
			hr = __HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

			LOGMAN_ERROR(pMgr, hr, L"SaveAsFileName can't be NULL!");

			break;
		}

		CommandLength = sizeof(NXRMFLT_COMMAND_MSG) - sizeof(UCHAR) + sizeof(NXRMFLT_SAVEAS_FORECAST);

		Msg = (NXRMFLT_COMMAND_MSG *)malloc(CommandLength);

		if (!Msg)
		{
			hr = E_OUTOFMEMORY;

			LOGMAN_ERROR(pMgr, hr, L"Failed to build SaveAs forecast command!");

			break;
		}

		memset(Msg, 0, CommandLength);

		Msg->Command = nxrmfltSaveAsForecast;
		Msg->Size = sizeof(NXRMFLT_SAVEAS_FORECAST);
		
		pSaveAsForecast = (NXRMFLT_SAVEAS_FORECAST *)Msg->Data;

		pSaveAsForecast->ProcessId = ProcessId;

		dwret = GetLongPathNameW(SaveAsFileName, pSaveAsForecast->SaveAsFileName, sizeof(pSaveAsForecast->SaveAsFileName));

		if (0 == dwret || dwret >= sizeof(pSaveAsForecast->SaveAsFileName)) {
			WCHAR * pfind;
			memcpy(NTPath, SaveAsFileName, min(sizeof(NTPath) - sizeof(WCHAR), wcslen(SaveAsFileName) * sizeof(WCHAR)));
			pfind = wcsrchr(NTPath, L'\\');
			if (pfind) {
				//try convert path only
				* pfind = 0;
				dwret = GetLongPathNameW(NTPath, pSaveAsForecast->SaveAsFileName, sizeof(pSaveAsForecast->SaveAsFileName));
				if (dwret && dwret < sizeof(pSaveAsForecast->SaveAsFileName)) {
					const WCHAR *pfilename;
					pfilename = wcsrchr(SaveAsFileName, L'\\');
					wcscat_s(pSaveAsForecast->SaveAsFileName, pfilename);
				}
				else {
					//user original string
					memcpy(pSaveAsForecast->SaveAsFileName,
						SaveAsFileName,
						min(sizeof(pSaveAsForecast->SaveAsFileName) - sizeof(WCHAR), wcslen(SaveAsFileName) * sizeof(WCHAR)));
				}
			}
			else {
				//user original string
				memcpy(pSaveAsForecast->SaveAsFileName,
					SaveAsFileName,
					min(sizeof(pSaveAsForecast->SaveAsFileName) - sizeof(WCHAR), wcslen(SaveAsFileName) * sizeof(WCHAR)));
			}
		}

		if (0 == lstrcmpiW(pSaveAsForecast->SaveAsFileName + (wcslen(pSaveAsForecast->SaveAsFileName) - 4), L".nxl"))
		{//found nxl extension. remove it
			pSaveAsForecast->SaveAsFileName[wcslen(pSaveAsForecast->SaveAsFileName) - 4] = 0;
		}

		memset(NTPath, 0, sizeof(NTPath));

		ccNTPath = sizeof(NTPath) / sizeof(WCHAR);

		if (!Win32Path2NTPath(pSaveAsForecast->SaveAsFileName, NTPath, &ccNTPath))
		{
			LOGMAN_ERROR(pMgr, hr, L"Failed to convert path %s\n", pSaveAsForecast->SaveAsFileName);
			break;
		}

		memset(pSaveAsForecast->SaveAsFileName, 0, sizeof(pSaveAsForecast->SaveAsFileName));
		memcpy(pSaveAsForecast->SaveAsFileName,
				NTPath,
			   min(sizeof(pSaveAsForecast->SaveAsFileName) - sizeof(WCHAR), wcslen(NTPath)*sizeof(WCHAR)));
		
		if (SrcFileName)
		{
			dwret = GetLongPathNameW(SrcFileName, pSaveAsForecast->SourceFileName, sizeof(pSaveAsForecast->SourceFileName));

			if (0 == dwret || dwret >= sizeof(pSaveAsForecast->SourceFileName)) {
				//user original string
				memcpy(pSaveAsForecast->SourceFileName,
					SrcFileName,
					min(sizeof(pSaveAsForecast->SourceFileName) - sizeof(WCHAR), wcslen(SrcFileName) * sizeof(WCHAR)));
			}

			if (0== lstrcmpiW(pSaveAsForecast->SourceFileName + (wcslen(pSaveAsForecast->SourceFileName) - 4), L".nxl"))
			{//found nxl extension. remove it
				pSaveAsForecast->SourceFileName[wcslen(pSaveAsForecast->SourceFileName) - 4] = 0;
			}

			memset(NTPath, 0, sizeof(NTPath));

			ccNTPath = sizeof(NTPath) / sizeof(WCHAR);

			if (!Win32Path2NTPath(pSaveAsForecast->SourceFileName, NTPath, &ccNTPath))
			{
				LOGMAN_ERROR(pMgr, hr, L"Failed to convert path %s\n", pSaveAsForecast->SourceFileName);
				break;
			}
			memset(pSaveAsForecast->SourceFileName, 0, sizeof(pSaveAsForecast->SourceFileName));
			memcpy(pSaveAsForecast->SourceFileName,
				NTPath,
				min(sizeof(pSaveAsForecast->SourceFileName) - sizeof(WCHAR), wcslen(NTPath) * sizeof(WCHAR)));

		}
		else
		{
			memset(pSaveAsForecast->SourceFileName, 0, sizeof(pSaveAsForecast->SourceFileName));
		}
		
		hr = FilterSendMessage(pMgr->ConnectionPort,
							   Msg,
							   CommandLength,
							   NULL,
							   0,
							   &BytesReturn);
		if(FAILED(hr))
		{
			LOGMAN_ERROR(pMgr, hr, L"Failed to reply message to nxrmflt!");
		}

	} while (FALSE);

	if (Msg)
	{
		free(Msg);
	}

	return hr;
}

HRESULT NXRMFLTMAN_API __stdcall nxrmfltSetPolicyChanged(NXRMFLT_HANDLE	hMgr)
{
	HRESULT	hr = S_OK;

	NXRMFLT_MANAGER *pMgr = NULL;

	NXRMFLT_COMMAND_MSG	*Msg = NULL;

	ULONG CommandLength = 0;

	ULONG BytesReturn = 0;

	pMgr = (NXRMFLT_MANAGER *)hMgr;

	do 
	{
		CommandLength = sizeof(NXRMFLT_COMMAND_MSG) - sizeof(UCHAR);

		Msg = (NXRMFLT_COMMAND_MSG *)malloc(CommandLength);

		if (!Msg)
		{
			hr = E_OUTOFMEMORY;

			LOGMAN_ERROR(pMgr, hr, L"Failed to build policy changed command due to out of memory!");

			break;
		}

		memset(Msg, 0, CommandLength);

		Msg->Command = nxrmfltPolicyChanged;
		Msg->Size = 0;

		hr = FilterSendMessage(pMgr->ConnectionPort,
							   Msg,
							   CommandLength,
							   NULL,
							   0,
							   &BytesReturn);
		if(FAILED(hr))
		{
			LOGMAN_ERROR(pMgr, hr, L"Failed to send policy changed message to nxrmflt!");
		}

	} while (FALSE);

	if (Msg)
	{
		free(Msg);
	}

	return hr;
}

HRESULT NXRMFLTMAN_API __stdcall nxrmfltUpdateKeyChain(
	NXRMFLT_HANDLE				hMgr,
	NXRM_KEY_BLOB				*KeyChain,
	ULONG						KeyChainSizeInByte)
{
	HRESULT	hr = S_OK;

	NXRMFLT_MANAGER *pMgr = NULL;

	NXRMFLT_COMMAND_MSG	*Msg = NULL;

	ULONG CommandLength = 0;

	ULONG BytesReturn = 0;

	pMgr = (NXRMFLT_MANAGER *)hMgr;

	do 
	{
		CommandLength = sizeof(NXRMFLT_COMMAND_MSG) - sizeof(UCHAR) + KeyChainSizeInByte;

		Msg = (NXRMFLT_COMMAND_MSG *)malloc(CommandLength);

		if (!Msg)
		{
			hr = E_OUTOFMEMORY;

			LOGMAN_ERROR(pMgr, hr, L"Failed to build update keychain command!");

			break;
		}

		memset(Msg, 0, CommandLength);

		Msg->Command = nxrmfltSetKeys;
		Msg->Size = KeyChainSizeInByte;

		memcpy(Msg->Data,
			   KeyChain,
			   KeyChainSizeInByte);

		hr = FilterSendMessage(pMgr->ConnectionPort,
							   Msg,
							   CommandLength,
							   NULL,
							   0,
							   &BytesReturn);
		if(FAILED(hr))
		{
			LOGMAN_ERROR(pMgr, hr, L"Failed to send update keychain message to nxrmflt!");
		}

	} while (FALSE);

	if (Msg)
	{
		free(Msg);
	}

	return hr;
}

BOOL Win32Path2NTPath(const WCHAR *src, WCHAR *ntpath, ULONG *ccntpath)
{
	BOOL bRet = FALSE;

	LONG dstlength = 0;

	ULONG srclength = 0;
	ULONG CharCount = 0;

	WCHAR DeviceName[MAX_PATH] = { 0 };

	WCHAR DosDrive[16] = { 0 };

	WCHAR *p = (WCHAR*)src;

	do
	{
		srclength = (ULONG)wcslen(p);

		if (srclength < 4)
		{
			dstlength = 0;
			break;
		}

		if (!wcsstr(src, L"\\"))
		{
			//
			// in the case of there is no "\" in path
			//
			if (*ccntpath > wcslen(src))
			{
				dstlength = (LONG)swprintf_s(ntpath, *ccntpath, L"%s", src);

				if (dstlength != -1)
				{
					bRet = TRUE;
				}
				else
				{
					dstlength = 0;
				}
			}
			else
			{
				dstlength = (ULONG)(wcslen(DeviceName) + wcslen(p));
			}

			break;
		}

		if (src[0] == L'\\' &&
			src[1] == L'\\' &&
			src[2] == L'.' &&
			src[3] == L'\\')
		{
			dstlength = 0;
			break;
		}

		//
		// skip \\?\ prefix
		// 
		if (src[0] == L'\\' &&
			src[1] == L'\\' &&
			src[2] == L'?' &&
			src[3] == L'\\')
		{
			p += 4;
		}

		srclength = (ULONG)wcslen(p);

		if (srclength < 2)
		{
			dstlength = 0;
			break;
		}

		//
		// p point to C:\\path\filename.ext
		//			  ^
		//			  |
		//			  p
		//

		memcpy(DosDrive,
			p,
			2 * sizeof(WCHAR));

		p += 2;

		//
		// p point to C:\\path\filename.ext
		//				^
		//				|
		//				p
		//


		CharCount = QueryDosDeviceW(DosDrive,
			DeviceName,
			(sizeof(DeviceName) / sizeof(WCHAR)) - 1);

		if (CharCount == 0)
		{
			//
			// not a DOS path
			//
			dstlength = 0;
		}

		if (*ccntpath > wcslen(DeviceName) + wcslen(p))
		{
			dstlength = (LONG)swprintf_s(ntpath, *ccntpath, L"%s%s", DeviceName, p);

			if (dstlength != -1)
			{
				bRet = TRUE;
			}
			else
			{
				dstlength = 0;
			}
		}
		else
		{
			dstlength = (ULONG)(wcslen(DeviceName) + wcslen(p));
		}

	} while (FALSE);

	*ccntpath = dstlength;

	return bRet;
}
