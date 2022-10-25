// nxrmdrvdemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <nudf\shared\listhlp.h>
#include <nudf\image\bitmap.hpp>
#include <nudf\shared\officelayout.h>
#include <nudf\nxlutil.hpp>

#define MAX_ULONGLONG                   0xFFFFFFFFFFFFFFFF


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

	NXRMDRV_HANDLE		hNxrmMan;

	HANDLE				hThreads[4];

	ULONG				ThreadId[4];

	LIST_ENTRY			Queue;

	CRITICAL_SECTION	QueueLock;

} DEMO_ENG, *PDEMO_ENG;

typedef struct _QUEUE_ITEM{

	PVOID							Request;

	ULONG							Type;

	LIST_ENTRY						Link;

	PVOID							Ctx;

}QUEUE_ITEM,*PQUEUE_ITEM;

BOOL process_get_customui_request(DEMO_ENG &eng, OFFICE_GET_CUSTOMUI_REQUEST &req, PVOID msgctx);

BOOL process_check_obligations_request(DEMO_ENG &eng, CHECK_OBLIGATION_REQUEST &req, PVOID msgctx);

BOOL process_saveas_forecast_request(DEMO_ENG &eng, SAVEAS_FORECAST_REQUEST &req, PVOID msgctx);

BOOL process_get_classifyui_request(DEMO_ENG &eng, GET_CLASSIFY_UI_REQUEST &req, PVOID msgctx);

BOOL process_block_notification_request(DEMO_ENG &eng, BLOCK_NOTIFICATION_REQUEST &req, PVOID msgctx);

int _tmain(int argc, _TCHAR* argv[])
{
	ULONG ulRet = 0;

	DEMO_ENG Eng = { 0 };

	ULONG i = 0;

	NXRM_PROCESS_ENTRY ProcessInfo = {0};

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

		Eng.hNxrmMan = nxrmdrvmanCreateManager(notification_callback, dbg_callback, dbg_check_level, &Eng);

		if (!Eng.hNxrmMan)
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

		ulRet = nxrmdrvmanEnableAntiTampering(TRUE);

		ulRet = nxrmdrvmanStart(Eng.hNxrmMan);

		if (ulRet != ERROR_SUCCESS)
		{
			break;
		}
		//DebugBreak();
		//{
		//	HANDLE hFile = INVALID_HANDLE_VALUE;

		//	DWORD dwBytesWrites = 0;

		//	hFile = CreateFileW(L"C:\\test\\test.txt",
		//		GENERIC_READ|FILE_READ_EA|FILE_WRITE_EA,
		//		FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
		//		NULL,
		//		OPEN_EXISTING,
		//		FILE_ATTRIBUTE_NORMAL,
		//		NULL);

		//	if (hFile != INVALID_HANDLE_VALUE)
		//	{
		//		nudf::util::nxl::NxrmSyncNXLHeader(hFile, NXL_SECTION_TAGS, (UCHAR*)("TAG1=ITAR"), strlen("TAG1=ITAR"));

		//		CloseHandle(hFile);
		//	}

		//}

		WaitForSingleObject(hStopEvent, INFINITE);

		nxrmdrvmanStop(Eng.hNxrmMan);

		SetEvent(Eng.hStop);

		WaitForMultipleObjects(4, Eng.hThreads, TRUE, INFINITE);

	} while (FALSE);

	if (Eng.hNxrmMan)
	{
		nxrmdrvmanCloseManager(Eng.hNxrmMan);
		Eng.hNxrmMan = NULL;
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
		switch (type)
		{
		case NXRMDRV_MSG_TYPE_GET_CUSTOMUI:

			pItem = (QUEUE_ITEM*)malloc(sizeof(QUEUE_ITEM));

			if (!pItem)
			{
				break;
			}

			pItem->Request = malloc(sizeof(OFFICE_GET_CUSTOMUI_REQUEST));

			if (!pItem->Request)
			{
				free(pItem);
				pItem = NULL;
				break;
			}

			pItem->Type = NXRMDRV_MSG_TYPE_GET_CUSTOMUI;

			memcpy(pItem->Request, 
				   msg, 
				   min(sizeof(OFFICE_GET_CUSTOMUI_REQUEST), Length));
			break;

		case NXRMDRV_MSG_TYPE_CHECKOBLIGATION:

			pItem = (QUEUE_ITEM*)malloc(sizeof(QUEUE_ITEM));

			if (!pItem)
			{
				break;
			}

			pItem->Request = malloc(sizeof(CHECK_OBLIGATION_REQUEST));

			if (!pItem->Request)
			{
				free(pItem);
				pItem = NULL;
				break;
			}

			pItem->Type = NXRMDRV_MSG_TYPE_CHECKOBLIGATION;

			memcpy(pItem->Request, 
				   msg, 
				   min(sizeof(CHECK_OBLIGATION_REQUEST), Length));
			break;

		case NXRMDRV_MSG_TYPE_SAVEAS_FORECAST:

			pItem = (QUEUE_ITEM *)malloc(sizeof(QUEUE_ITEM));

			if (!pItem)
			{
				break;
			}

			pItem->Request = malloc(sizeof(SAVEAS_FORECAST_REQUEST));

			if (!pItem->Request)
			{
				free(pItem);
				pItem = NULL;
				break;
			}

			pItem->Type = NXRMDRV_MSG_TYPE_SAVEAS_FORECAST;

			memcpy(pItem->Request,
				   msg,
				   min(sizeof(SAVEAS_FORECAST_REQUEST), Length));

			break;
		
		case NXRMDRV_MSG_TYPE_GET_CLASSIFYUI:

			pItem = (QUEUE_ITEM *)malloc(sizeof(QUEUE_ITEM));

			if (!pItem)
			{
				break;
			}

			pItem->Request = malloc(sizeof(GET_CLASSIFY_UI_REQUEST));

			if (!pItem->Request)
			{
				free(pItem);
				pItem = NULL;
				break;
			}

			pItem->Type = NXRMDRV_MSG_TYPE_GET_CLASSIFYUI;

			memcpy(pItem->Request,
				   msg,
				   min(sizeof(GET_CLASSIFY_UI_REQUEST), Length));

			break;
		
		case NXRMDRV_MSG_TYPE_BLOCK_NOTIFICATION:

			pItem = (QUEUE_ITEM *)malloc(sizeof(QUEUE_ITEM));

			if (!pItem)
			{
				break;
			}

			pItem->Request = malloc(sizeof(BLOCK_NOTIFICATION_REQUEST));

			if (!pItem->Request)
			{
				free(pItem);
				pItem = NULL;
				break;
			}

			pItem->Type = NXRMDRV_MSG_TYPE_BLOCK_NOTIFICATION;

			memcpy(pItem->Request,
				   msg,
				   min(sizeof(BLOCK_NOTIFICATION_REQUEST), Length));

			break;

		default:
			break;
		}

		if (!pItem)
		{
			break;
		}

		pItem->Ctx = msgctx;

		EnterCriticalSection(&pEng->QueueLock);

		InsertTailList(&pEng->Queue, &pItem->Link);

		LeaveCriticalSection(&pEng->QueueLock);

		ReleaseSemaphore(pEng->hSemQueue, 1, NULL);

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

	HRESULT hr = S_OK;

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
			case NXRMDRV_MSG_TYPE_GET_CUSTOMUI:
				process_get_customui_request(*pEng, *((OFFICE_GET_CUSTOMUI_REQUEST*)pItem->Request), pItem->Ctx);
				break;
			case NXRMDRV_MSG_TYPE_CHECKOBLIGATION:
				process_check_obligations_request(*pEng, *((CHECK_OBLIGATION_REQUEST*)pItem->Request), pItem->Ctx);
				break;
			case NXRMDRV_MSG_TYPE_SAVEAS_FORECAST:
				process_saveas_forecast_request(*pEng, *((SAVEAS_FORECAST_REQUEST*)pItem->Request), pItem->Ctx);
				break;
			case NXRMDRV_MSG_TYPE_GET_CLASSIFYUI:
				process_get_classifyui_request(*pEng, *((GET_CLASSIFY_UI_REQUEST*)pItem->Request), pItem->Ctx);
				break;
			case NXRMDRV_MSG_TYPE_BLOCK_NOTIFICATION:
				process_block_notification_request(*pEng, *((BLOCK_NOTIFICATION_REQUEST*)pItem->Request), pItem->Ctx);
				break;
			default:
				break;
			}

			free(pItem->Request);
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

BOOL process_get_customui_request(DEMO_ENG &eng, OFFICE_GET_CUSTOMUI_REQUEST &req, PVOID msgctx)
{
	BOOL bRet = TRUE;

	UINT uRet = 0;

	HANDLE hFile = INVALID_HANDLE_VALUE;

	DWORD BytesWriten = 0;

	HRESULT hr = S_OK;

	OFFICE_GET_CUSTOMUI_RESPONSE resp = {0};

	NXRM_PROCESS_ENTRY ProcessInfo = {0};

	ULONG ulRet = 0;
	do 
	{
		if (nxrmdrvmanIsRequestCancelled(eng.hNxrmMan, msgctx))
		{
			break;
		}


		ulRet = nxrmdrvmanQueryProcessInfo(eng.hNxrmMan, req.ProcessId, &ProcessInfo);

		if (ulRet != ERROR_SUCCESS)
		{
			printf("Failed to query process information\n");
		}
		else
		{
			printf("Received request from Office application %S\n", ProcessInfo.process_path);
			printf("\tOffice Version is:\t%S\n", req.OfficeVersion);
			printf("\tOffice Language ID is:\t%hu\n", req.OfficeLanguageId);
			printf("\tOffice temp folder is:\t%S\n", req.TempPath);
		}

		uRet = GetTempFileNameW(req.TempPath,
								L"nxrm",
								0,
								resp.CustomUIFileName);

		if (!uRet)
		{
			bRet = FALSE;
			break;
		}

		hFile = CreateFileW(resp.CustomUIFileName,
							GENERIC_WRITE,
							0,
							NULL,
							CREATE_ALWAYS,
							FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_SEQUENTIAL_SCAN,
							NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			bRet = FALSE;
			break;
		}

		if (!WriteFile(hFile, 
					   OFFICE_LAYOUT_XML, 
					   (DWORD)strlen(OFFICE_LAYOUT_XML),
					   &BytesWriten,
					   NULL))
		{
			bRet = FALSE;
			break;
		}

		CloseHandle(hFile);
		
		hFile = INVALID_HANDLE_VALUE;

		hr = nxrmdrvmanPostOfficeGetCustomUIResponse(eng.hNxrmMan, msgctx, &resp);

	} while (FALSE);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	return bRet;
}

BOOL process_check_obligations_request(DEMO_ENG &eng, CHECK_OBLIGATION_REQUEST &req, PVOID msgctx)
{
	BOOL bRet = TRUE;

	BYTE buf[1024] = {0};

	NXRM_OBLIGATION *resp = (NXRM_OBLIGATION *)buf;

	ULONG resplen = 0;

	HRESULT hr = S_OK;

	nudf::image::CTextBitmap *pTextBitmap = new nudf::image::CTextBitmap;

	const WCHAR bitmappath[] = OB_OVERLAY_PARAM_IMAGE L"=C:\\test\\test.bmp";

	const WCHAR FrontName[] = L"Sitka Text";

	HFONT hFont = NULL;

	std::wstring watermark;

	do 
	{
		if (nxrmdrvmanIsRequestCancelled(eng.hNxrmMan, msgctx))
		{
			break;
		}
		
		printf("Received Check Obligation request:\n\tEvaluationId:\t%I64u\n", req.EvaluationId);

		hFont = CreateFontW(36,
							0, 
							0, 
							0, 
							FW_BOLD,
							TRUE, 
							FALSE, 
							FALSE,
							DEFAULT_CHARSET,
							OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,
							CLEARTYPE_NATURAL_QUALITY,
							DEFAULT_PITCH | FF_DONTCARE,
							FrontName);

		if (!hFont)
		{
			hr = nxrmdrvmanPostCheckObligationsResponse(eng.hNxrmMan, msgctx, resp, sizeof(NXRM_OBLIGATION));
			break;
		}

		watermark += req.FileName;
		watermark += L"@NextLabs";

		pTextBitmap->Create(watermark.c_str(), hFont, RGB(255,255,255), RGB(0,0,0));
		pTextBitmap->Rotate(NULL, -45);
		//pTextBitmap->SetTransparency(0x3f);
		pTextBitmap->ToFile(L"C:\\test\\test.bmp");

		resp->NextOffset	= 0;
		resp->Id			= OB_ID_OVERLAY;

		memcpy(resp->Params, bitmappath, sizeof(bitmappath));

		resplen = sizeof(NXRM_OBLIGATION) - sizeof(WCHAR) + sizeof(bitmappath);

		hr = nxrmdrvmanPostCheckObligationsResponse(eng.hNxrmMan, msgctx, resp, resplen);

		if (FAILED(hr))
		{
			printf("Failed to post response\n");
		}

	} while (FALSE);

	if (hFont)
	{
		DeleteObject(hFont);
		hFont = NULL;
	}

	if (pTextBitmap)
	{
		delete pTextBitmap;
	}

	return bRet;
}

BOOL process_saveas_forecast_request(DEMO_ENG &eng, SAVEAS_FORECAST_REQUEST &req, PVOID msgctx)
{
	BOOL bRet = TRUE;

	UINT uRet = 0;

	HRESULT hr = S_OK;

	NXRM_PROCESS_ENTRY ProcessInfo = {0};

	ULONG ulRet = 0;
	do 
	{
		if (nxrmdrvmanIsRequestCancelled(eng.hNxrmMan, msgctx))
		{
			break;
		}

		ulRet = nxrmdrvmanQueryProcessInfo(eng.hNxrmMan, req.ProcessId, &ProcessInfo);

		if (ulRet != ERROR_SUCCESS)
		{
			printf("Failed to query process information\n");
		}
		else
		{
			printf("Received SaveAs forecast request from %S\n", ProcessInfo.process_path);
			printf("\tSource file is %S\n", wcslen(req.SourceFileName)?req.SourceFileName:L"N/A");
			printf("\tSaveAs file is %S\n", req.SaveAsFileName);
		}

		hr = nxrmdrvmanPostSaveAsForecastResponse(eng.hNxrmMan, msgctx);

	} while (FALSE);

	return bRet;

}

BOOL process_get_classifyui_request(DEMO_ENG &eng, GET_CLASSIFY_UI_REQUEST &req, PVOID msgctx)
{
	BOOL bRet = TRUE;

	UINT uRet = 0;

	HRESULT hr = S_OK;

	GET_CLASSIFY_UI_RESPONSE resp = {0};

	NXRM_PROCESS_ENTRY ProcessInfo = {0};

	ULONG ulRet = 0;

	do 
	{
		if (nxrmdrvmanIsRequestCancelled(eng.hNxrmMan, msgctx))
		{
			break;
		}


		ulRet = nxrmdrvmanQueryProcessInfo(eng.hNxrmMan, req.ProcessId, &ProcessInfo);

		if (ulRet != ERROR_SUCCESS)
		{
			printf("Failed to query process information\n");
		}
		else
		{
			printf("Received Classify UI request from application %S\n", ProcessInfo.process_path);
			printf("\tLanguage ID is:\t%hu\n", req.LanguageId);
			printf("\tTemp folder is:\t%S\n", req.TempPath);
		}

		uRet = GetTempFileNameW(req.TempPath,
								L"nxrm",
								0,
								resp.ClassifyUIFileName);

		if (!uRet)
		{
			bRet = FALSE;
			break;
		}

		if (!CopyFileExW(L".\\Classify.xml", resp.ClassifyUIFileName, NULL, NULL, NULL, 0))
		{
			printf("Failed to copy xml file\n");
		}

		hr = nxrmdrvmanPostGetClassifyUIResponse(eng.hNxrmMan, msgctx, &resp);

	} while (FALSE);

	return bRet;
}

BOOL process_block_notification_request(DEMO_ENG &eng, BLOCK_NOTIFICATION_REQUEST &req, PVOID msgctx)
{
	BOOL bRet = TRUE;

	UINT uRet = 0;

	HRESULT hr = S_OK;

	NXRM_PROCESS_ENTRY ProcessInfo = {0};

	ULONG ulRet = 0;

	do 
	{
		if (nxrmdrvmanIsRequestCancelled(eng.hNxrmMan, msgctx))
		{
			break;
		}

		ulRet = nxrmdrvmanQueryProcessInfo(eng.hNxrmMan, req.ProcessId, &ProcessInfo);

		if (ulRet != ERROR_SUCCESS)
		{
			printf("Failed to query process information\n");
		}
		else
		{
			printf("Received Block notification from %S\n", ProcessInfo.process_path);
			printf("\tSession Id is:\t%d\n", req.SessionId);
			printf("\tLanguage ID is:\t%hu\n", req.LanguageId);
			printf("\tType is:\t%hu\n", req.Type);
		}

		hr = nxrmdrvmanPostBlockNotificationResponse(eng.hNxrmMan, msgctx);

	} while (FALSE);

	return bRet;

}