// nxrmdrvman.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "nxrmdrvman.h"
#include "nxrmdrvmandef.h"
#include "nxrmcorehlp.h"

#include "loghlp.hpp"

#ifndef MAX_ULONGLONG
#define MAX_ULONGLONG                   0xFFFFFFFFFFFFFFFF
#endif

DECLSPEC_CACHEALIGN	LONG		g_nxrmdrvmgrcreated = 0;


DECLARE_NXRM_MODULE_DRV();
DECLARE_NXRM_MODULE_DRVMAN();

DWORD WINAPI nxrmdrvmanWorker(_In_ LPVOID lpParameter);
DWORD WINAPI nxrmdrvProcessCacheWorker(_In_ LPVOID lpParameter);

static NXRM_PROCESS_ENTRY* find_process_in_cache(rb_root *proc_cache, HANDLE process_id);
static BOOL add_process_into_cache(rb_root *proc_cache, NXRM_PROCESS_ENTRY *proc);

static HRESULT handle_update_protected_app_info_request(ULONG				type,
														PVOID				msg,
														ULONG				Length,
														PVOID				msgctx,
														PVOID				userctx);

static HRESULT handle_query_protected_app_info_request(ULONG				type,
													   PVOID				msg,
													   ULONG				Length,
													   PVOID				msgctx,
													   PVOID				userctx);

NXRMDRV_HANDLE NXRMDRVMAN_API __stdcall nxrmdrvmanCreateManager(
	NXRMDRV_CALLBACK_NOTIFY		NotifyCallback, 
	LOGAPI_LOG					DebugDumpCallback,
	LOGAPI_ACCEPT				DebugDumpCheckLevelCallback,
	PVOID						UserContext)
{
	NXRMDRV_MANAGER	*hMgr = NULL;

	LONG nxrmdrvmgrcreated = 0;

	BOOL bFailed = FALSE;
	BOOL bCriticalSectionInitialized = FALSE;
	BOOL bProtectedAppInfoListCriticalSectionInitialized = FALSE;

	do 
	{
		nxrmdrvmgrcreated = InterlockedCompareExchange(&g_nxrmdrvmgrcreated, 1, 0);

		if (nxrmdrvmgrcreated)
		{
            assert(NULL != hMgr);
            if(NULL!=DebugDumpCheckLevelCallback && NULL!=DebugDumpCallback && DebugDumpCheckLevelCallback(LOGWARNING))
            {
                nudf::util::log::CLogEntry le(LOGWARNING, NXMODNAME(), (LONG)0, __FILE__, __FUNCTION__, __LINE__, L"Manager already exists.");
                DebugDumpCallback(le.GetData().c_str());
            }
			break;
		}

		hMgr = (NXRMDRV_MANAGER*)malloc(sizeof(NXRMDRV_MANAGER));

		if (!hMgr)
		{
			bFailed = TRUE;
            if(NULL!=DebugDumpCheckLevelCallback && NULL!=DebugDumpCallback && DebugDumpCheckLevelCallback(LOGERROR))
            {
                nudf::util::log::CLogEntry le(LOGERROR, NXMODNAME(), (LONG)0, __FILE__, __FUNCTION__, __LINE__, L"Fail to allocate memory for Manager Object.");
                DebugDumpCallback(le.GetData().c_str());
            }
			break;
		}

		memset(hMgr, 0, sizeof(NXRMDRV_MANAGER));

		hMgr->DebugDumpCallback				= DebugDumpCallback;
		hMgr->DebugDumpCheckLevelCallback	= DebugDumpCheckLevelCallback;
		hMgr->NotifyCallback				= NotifyCallback;
		hMgr->UserCtx						= UserContext;
		hMgr->Stop							= FALSE;
		hMgr->DisableFiltering				= TRUE;

		hMgr->nxrmdrvsection = init_transporter_server();

		if (!hMgr->nxrmdrvsection)
		{
			bFailed = FALSE;
            LOGMAN_WARN(hMgr, L"CreateManager: Fail to init transporter server");
			break;
		}

		hMgr->ProcessCache.rb_node	= NULL;
		hMgr->ProcessCacheInfo		= NULL;
		hMgr->ProcessCacheInfoSize	= 0;

		InitializeCriticalSection(&hMgr->ProcessCacheLock);
		
		bCriticalSectionInitialized = TRUE;

		InitializeListHead(&hMgr->ProtectedModeAppInfoList);

		InitializeCriticalSection(&hMgr->ProtectedModeAppInfoListLock);

		bProtectedAppInfoListCriticalSectionInitialized = TRUE;

		hMgr->StopEvent	= CreateEventW(NULL, TRUE, FALSE, NULL);

		if (!hMgr->StopEvent)
		{
			bFailed = TRUE;
            LOGMAN_ERROR(hMgr, GetLastError(), L"CreateManager: Fail to create stop event");
			break;
		}

		hMgr->WorkerThread = CreateThread(NULL,
										  0,
										  nxrmdrvmanWorker,
										  (LPVOID)hMgr,
										  CREATE_SUSPENDED,
										  &hMgr->WorkerThreadId);

		if (!hMgr->WorkerThread)
		{
			bFailed = TRUE;
            LOGMAN_ERROR(hMgr, GetLastError(), L"CreateManager: Fail to create worker thread");
			break;
		}

		ResumeThread(hMgr->WorkerThread);

		hMgr->ProcessCacheThread = CreateThread(NULL,
												0,
												nxrmdrvProcessCacheWorker,
												(LPVOID)hMgr,
												CREATE_SUSPENDED,
												&hMgr->ProcessCacheThreadId);

		if (!hMgr->ProcessCacheThread)
		{
			bFailed = TRUE;
            LOGMAN_ERROR(hMgr, GetLastError(), L"CreateManager: Fail to create cache worker thread");
			break;
		}

		ResumeThread(hMgr->ProcessCacheThread);

	} while (FALSE);

	if (bFailed)
	{
		InterlockedExchange(&g_nxrmdrvmgrcreated, 0);

		if (hMgr)
		{
			hMgr->Stop = TRUE;

			if (hMgr->StopEvent)
			{
				SetEvent(hMgr->StopEvent);
			}

			if (hMgr->WorkerThread)
			{
				WaitForSingleObject(hMgr->WorkerThread, 15*1000);

				CloseHandle(hMgr->WorkerThread);
				hMgr->WorkerThread = NULL;
			}

			if (hMgr->ProcessCacheThread)
			{
				WaitForSingleObject(hMgr->ProcessCacheThread, 15 * 1000);

				CloseHandle(hMgr->ProcessCacheThread);
				hMgr->ProcessCacheThread = NULL;
			}

			if (hMgr->StopEvent)
			{
				CloseHandle(hMgr->StopEvent);
				hMgr->StopEvent = NULL;
			}

			if (hMgr->nxrmdrvsection)
			{
				close_transporter_server(hMgr->nxrmdrvsection);
				hMgr->nxrmdrvsection = NULL;
			}

			if (bCriticalSectionInitialized)
			{
				DeleteCriticalSection(&hMgr->ProcessCacheLock);
				bCriticalSectionInitialized = FALSE;
			}

			if (bProtectedAppInfoListCriticalSectionInitialized)
			{
				DeleteCriticalSection(&hMgr->ProtectedModeAppInfoListLock);
				bProtectedAppInfoListCriticalSectionInitialized = FALSE;
			}

			free(hMgr);

			hMgr = NULL;
		}
	}

	return (NXRMDRV_HANDLE)hMgr;
}
ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanStart(NXRMDRV_HANDLE hMgr)
{
	NXRMDRV_MANAGER	*pMgr = NULL;

	pMgr = (NXRMDRV_MANAGER *)hMgr;

	pMgr->DisableFiltering = FALSE;

	return ERROR_SUCCESS;
}

ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanStop(NXRMDRV_HANDLE hMgr)
{
	NXRMDRV_MANAGER	*pMgr = NULL;

	pMgr = (NXRMDRV_MANAGER *)hMgr;

	pMgr->DisableFiltering = TRUE;

	return ERROR_SUCCESS;
}

ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostCheckObligationsResponse(
	NXRMDRV_HANDLE					hMgr, 
	PVOID							msgctx, 
	NXRM_OBLIGATION					*Obligations,
	ULONG							ObligationsLength)
{
	ULONG uRet = ERROR_SUCCESS;

	NXRMDRV_MANAGER	*pMgr = NULL;

	pMgr = (NXRMDRV_MANAGER *)hMgr;

	do 
	{
		if (!pMgr)
		{
			uRet = ERROR_INVALID_PARAMETER;
			break;
		}

		if(!msgctx)
		{
			uRet = ERROR_INVALID_PARAMETER;            
            LOGMAN_ERROR(hMgr, GetLastError(), L"Invalid message context");
			break;
		}

		if(((NXRMRECORD*)msgctx)->hdr.record_type != NXRMDRV_MSG_TYPE_CHECKOBLIGATION)
		{
			uRet = ERROR_INVALID_PARAMETER;
            LOGMAN_ERROR(hMgr, GetLastError(), L"Mismatch message type");
			break;
		}

		if (!post_response(pMgr->nxrmdrvsection, (NXRMRECORD*)msgctx, Obligations, ObligationsLength))
		{
			uRet = ERROR_INVALID_PARAMETER;
            LOGMAN_ERROR(hMgr, GetLastError(), L"Fail to post response");
			break;
		}
		
	} while (FALSE);

	return uRet;
}

ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostOfficeGetCustomUIResponse(
	NXRMDRV_HANDLE					hMgr,
	PVOID							msgctx,
	OFFICE_GET_CUSTOMUI_RESPONSE	*response)
{
	ULONG uRet = ERROR_SUCCESS;

	NXRMDRV_MANAGER	*pMgr = NULL;

	pMgr = (NXRMDRV_MANAGER *)hMgr;

	do 
	{
		if (!pMgr)
		{
			uRet = ERROR_INVALID_PARAMETER;
			break;
		}

		if(!msgctx)
		{
			uRet = ERROR_INVALID_PARAMETER;
            LOGMAN_ERROR(hMgr, GetLastError(), L"Invalid message context");
			break;
		}

		if(((NXRMRECORD*)msgctx)->hdr.record_type != NXRMDRV_MSG_TYPE_GET_CUSTOMUI)
		{
			uRet = ERROR_INVALID_PARAMETER;
            LOGMAN_ERROR(hMgr, GetLastError(), L"Mismatch message type");
			break;
		}

		if (!post_response(pMgr->nxrmdrvsection, (NXRMRECORD*)msgctx, response, sizeof(OFFICE_GET_CUSTOMUI_RESPONSE)))
		{
			uRet = ERROR_INVALID_PARAMETER;
            LOGMAN_ERROR(hMgr, GetLastError(), L"Fail to post response");
			break;
		}

	} while (FALSE);

	return uRet;

}

ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostSaveAsForecastResponse(
	NXRMDRV_HANDLE					hMgr,
	PVOID							msgctx)
{
	ULONG uRet = ERROR_SUCCESS;

	NXRMDRV_MANAGER	*pMgr = NULL;

	pMgr = (NXRMDRV_MANAGER *)hMgr;

	do 
	{
		if (!pMgr)
		{
			uRet = ERROR_INVALID_PARAMETER;
			break;
		}

		if(!msgctx)
		{
			uRet = ERROR_INVALID_PARAMETER;
            LOGMAN_ERROR(hMgr, GetLastError(), L"Invalid message context");
			break;
		}

		if(((NXRMRECORD*)msgctx)->hdr.record_type != NXRMDRV_MSG_TYPE_SAVEAS_FORECAST)
		{
			uRet = ERROR_INVALID_PARAMETER;
            LOGMAN_ERROR(hMgr, GetLastError(), L"Mismatch message type");
			break;
		}

		if (!post_response(pMgr->nxrmdrvsection, (NXRMRECORD*)msgctx, NULL, 0))
		{
			uRet = ERROR_INVALID_PARAMETER;
            LOGMAN_ERROR(hMgr, GetLastError(), L"Fail to post response");
			break;
		}

	} while (FALSE);

	return uRet;

}

ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostGetClassifyUIResponse(
	NXRMDRV_HANDLE					hMgr,
	PVOID							msgctx,
	GET_CLASSIFY_UI_RESPONSE		*response)
{
	ULONG uRet = ERROR_SUCCESS;

	NXRMDRV_MANAGER	*pMgr = NULL;

	pMgr = (NXRMDRV_MANAGER *)hMgr;

	do 
	{
		if (!pMgr)
		{
			uRet = ERROR_INVALID_PARAMETER;
			break;
		}

		if(!msgctx)
		{
			uRet = ERROR_INVALID_PARAMETER;
            LOGMAN_ERROR(hMgr, GetLastError(), L"Invalid message context");
			break;
		}

		if(((NXRMRECORD*)msgctx)->hdr.record_type != NXRMDRV_MSG_TYPE_GET_CLASSIFYUI)
		{
			uRet = ERROR_INVALID_PARAMETER;
            LOGMAN_ERROR(hMgr, GetLastError(), L"Mismatch message type");
			break;
		}

		if (!post_response(pMgr->nxrmdrvsection, (NXRMRECORD*)msgctx, response, sizeof(GET_CLASSIFY_UI_RESPONSE)))
		{
			uRet = ERROR_INVALID_PARAMETER;
            LOGMAN_ERROR(hMgr, GetLastError(), L"Fail to post response");
			break;
		}

	} while (FALSE);

	return uRet;

}

ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostBlockNotificationResponse(
	NXRMDRV_HANDLE					hMgr,
	PVOID							msgctx)
{
	ULONG uRet = ERROR_SUCCESS;

	NXRMDRV_MANAGER	*pMgr = NULL;

	pMgr = (NXRMDRV_MANAGER *)hMgr;

	do 
	{
		if (!pMgr)
		{
			uRet = ERROR_INVALID_PARAMETER;
			break;
		}

		if(!msgctx)
		{
			uRet = ERROR_INVALID_PARAMETER;
            LOGMAN_ERROR(hMgr, GetLastError(), L"Invalid message context");
			break;
		}

		if(((NXRMRECORD*)msgctx)->hdr.record_type != NXRMDRV_MSG_TYPE_BLOCK_NOTIFICATION)
		{
			uRet = ERROR_INVALID_PARAMETER;
            LOGMAN_ERROR(hMgr, GetLastError(), L"Mismatch message type");
			break;
		}

		if (!post_response(pMgr->nxrmdrvsection, (NXRMRECORD*)msgctx, NULL, 0))
		{
			uRet = ERROR_INVALID_PARAMETER;
            LOGMAN_ERROR(hMgr, GetLastError(), L"Fail to post response");
			break;
		}

	} while (FALSE);

	return uRet;
}

ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostCheckProtectMenuResponse(
	NXRMDRV_HANDLE					hMgr,
	PVOID							msgctx,
	CHECK_PROTECT_MENU_RESPONSE		*response)
{
	ULONG uRet = ERROR_SUCCESS;

	NXRMDRV_MANAGER	*pMgr = NULL;

	pMgr = (NXRMDRV_MANAGER *)hMgr;

	do 
	{
		if (!pMgr)
		{
			uRet = ERROR_INVALID_PARAMETER;
			break;
		}

		if(!msgctx)
		{
			uRet = ERROR_INVALID_PARAMETER;
            LOGMAN_ERROR(hMgr, GetLastError(), L"Invalid message context");
			break;
		}

		if(((NXRMRECORD*)msgctx)->hdr.record_type != NXRMDRV_MSG_TYPE_CHECK_PROTECT_MENU)
		{
			uRet = ERROR_INVALID_PARAMETER;
            LOGMAN_ERROR(hMgr, GetLastError(), L"Mismatch message type");
			break;
		}

		if (!post_response(pMgr->nxrmdrvsection, (NXRMRECORD*)msgctx, response, sizeof(CHECK_PROTECT_MENU_RESPONSE)))
		{
			uRet = ERROR_INVALID_PARAMETER;
            LOGMAN_ERROR(hMgr, GetLastError(), L"Fail to post response");
			break;
		}

	} while (FALSE);

	return uRet;
}

ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostGetCtxMenuRegExResponse(
	NXRMDRV_HANDLE					hMgr,
	PVOID							msgctx,
	QUERY_CTXMENUREGEX_RESPONSE		*response)
{
	ULONG uRet = ERROR_SUCCESS;

	NXRMDRV_MANAGER	*pMgr = NULL;

	pMgr = (NXRMDRV_MANAGER *)hMgr;

	do 
	{
		if (!pMgr)
		{
			uRet = ERROR_INVALID_PARAMETER;
			break;
		}

		if(!msgctx)
		{
			uRet = ERROR_INVALID_PARAMETER;
			LOGMAN_ERROR(hMgr, GetLastError(), L"Invalid message context");
			break;
		}

		if(((NXRMRECORD*)msgctx)->hdr.record_type != NXRMDRV_MSG_TYPE_GET_CTXMENUREGEX)
		{
			uRet = ERROR_INVALID_PARAMETER;
			LOGMAN_ERROR(hMgr, GetLastError(), L"Mismatch message type");
			break;
		}

		if (!post_response(pMgr->nxrmdrvsection, (NXRMRECORD*)msgctx, response, sizeof(QUERY_CTXMENUREGEX_RESPONSE)))
		{
			uRet = ERROR_INVALID_PARAMETER;
			LOGMAN_ERROR(hMgr, GetLastError(), L"Fail to post response");
			break;
		}

	} while (FALSE);

	return uRet;
}

ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostUpdateCoreCtxResponse(
	NXRMDRV_HANDLE					hMgr,
	PVOID							msgctx)
{
	ULONG uRet = ERROR_SUCCESS;

	NXRMDRV_MANAGER	*pMgr = NULL;

	pMgr = (NXRMDRV_MANAGER *)hMgr;

	do
	{
		if (!pMgr)
		{
			uRet = ERROR_INVALID_PARAMETER;
			break;
		}

		if (!msgctx)
		{
			uRet = ERROR_INVALID_PARAMETER;
			LOGMAN_ERROR(hMgr, GetLastError(), L"Invalid message context");
			break;
		}

		if (((NXRMRECORD*)msgctx)->hdr.record_type != NXRMDRV_MSG_TYPE_UPDATE_CORE_CTX)
		{
			uRet = ERROR_INVALID_PARAMETER;
			LOGMAN_ERROR(hMgr, GetLastError(), L"Mismatch message type");
			break;
		}

		if (!post_response(pMgr->nxrmdrvsection, (NXRMRECORD*)msgctx, NULL, 0))
		{
			uRet = ERROR_INVALID_PARAMETER;
			LOGMAN_ERROR(hMgr, GetLastError(), L"Fail to post response");
			break;
		}

	} while (FALSE);

	return uRet;
}

ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostQueryCoreCtxResponse(
	NXRMDRV_HANDLE					hMgr,
	PVOID							msgctx,
	QUERY_CORE_CTX_RESPONSE			*response)
{
	ULONG uRet = ERROR_SUCCESS;

	NXRMDRV_MANAGER	*pMgr = NULL;

	pMgr = (NXRMDRV_MANAGER *)hMgr;

	do
	{
		if (!pMgr)
		{
			uRet = ERROR_INVALID_PARAMETER;
			break;
		}

		if (!msgctx)
		{
			uRet = ERROR_INVALID_PARAMETER;
			LOGMAN_ERROR(hMgr, GetLastError(), L"Invalid message context");
			break;
		}

		if (((NXRMRECORD*)msgctx)->hdr.record_type != NXRMDRV_MSG_TYPE_QUERY_CORE_CTX)
		{
			uRet = ERROR_INVALID_PARAMETER;
			LOGMAN_ERROR(hMgr, GetLastError(), L"Mismatch message type");
			break;
		}

		if (!post_response(pMgr->nxrmdrvsection, (NXRMRECORD*)msgctx, response, sizeof(QUERY_CORE_CTX_RESPONSE)))
		{
			uRet = ERROR_INVALID_PARAMETER;
			LOGMAN_ERROR(hMgr, GetLastError(), L"Fail to post response");
			break;
		}

	} while (FALSE);

	return uRet;
}

ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostQueryServiceResponse(
	NXRMDRV_HANDLE					hMgr,
	PVOID							msgctx,
	QUERY_SERVICE_RESPONSE			*response)
{
	ULONG uRet = ERROR_SUCCESS;

	NXRMDRV_MANAGER	*pMgr = NULL;

	pMgr = (NXRMDRV_MANAGER *)hMgr;

	do
	{
		if (!pMgr)
		{
			uRet = ERROR_INVALID_PARAMETER;
			break;
		}

		if (!msgctx)
		{
			uRet = ERROR_INVALID_PARAMETER;
			LOGMAN_ERROR(hMgr, GetLastError(), L"Invalid message context");
			break;
		}

		if (((NXRMRECORD*)msgctx)->hdr.record_type != NXRMDRV_MSG_TYPE_QUERY_SERVICE)
		{
			uRet = ERROR_INVALID_PARAMETER;
			LOGMAN_ERROR(hMgr, GetLastError(), L"Mismatch message type");
			break;
		}

		if (!post_response(pMgr->nxrmdrvsection, (NXRMRECORD*)msgctx, response, sizeof(QUERY_SERVICE_RESPONSE)))
		{
			uRet = ERROR_INVALID_PARAMETER;
			LOGMAN_ERROR(hMgr, GetLastError(), L"Fail to post response");
			break;
		}

	} while (FALSE);

	return uRet;
}

ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostUpdateDwmWindowResponse(
	NXRMDRV_HANDLE					hMgr,
	PVOID							msgctx)
{
	ULONG uRet = ERROR_SUCCESS;

	NXRMDRV_MANAGER	*pMgr = NULL;

	pMgr = (NXRMDRV_MANAGER *)hMgr;

	do
	{
		if (!pMgr)
		{
			uRet = ERROR_INVALID_PARAMETER;
			break;
		}

		if (!msgctx)
		{
			uRet = ERROR_INVALID_PARAMETER;
			LOGMAN_ERROR(hMgr, GetLastError(), L"Invalid message context");
			break;
		}

		if (((NXRMRECORD*)msgctx)->hdr.record_type != NXRMDRV_MSG_TYPE_UPDATE_DWM_WINDOW)
		{
			uRet = ERROR_INVALID_PARAMETER;
			LOGMAN_ERROR(hMgr, GetLastError(), L"Mismatch message type");
			break;
		}

		if (!post_response(pMgr->nxrmdrvsection, (NXRMRECORD*)msgctx, NULL, 0))
		{
			uRet = ERROR_INVALID_PARAMETER;
			LOGMAN_ERROR(hMgr, GetLastError(), L"Fail to post response");
			break;
		}

	} while (FALSE);

	return uRet;
}

ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostCheckProcessRightsResponse(
	NXRMDRV_HANDLE					hMgr,
	PVOID							msgctx,
	CHECK_PROCESS_RIGHTS_RESPONSE	*response)
{
	ULONG uRet = ERROR_SUCCESS;

	NXRMDRV_MANAGER	*pMgr = NULL;

	pMgr = (NXRMDRV_MANAGER *)hMgr;

	do
	{
		if (!pMgr)
		{
			uRet = ERROR_INVALID_PARAMETER;
			break;
		}

		if (!msgctx)
		{
			uRet = ERROR_INVALID_PARAMETER;
			LOGMAN_ERROR(hMgr, GetLastError(), L"Invalid message context");
			break;
		}

		if (((NXRMRECORD*)msgctx)->hdr.record_type != NXRMDRV_MSG_TYPE_CHECK_PROCESS_RIGHTS)
		{
			uRet = ERROR_INVALID_PARAMETER;
			LOGMAN_ERROR(hMgr, GetLastError(), L"Mismatch message type");
			break;
		}

		if (!post_response(pMgr->nxrmdrvsection, (NXRMRECORD*)msgctx, response, sizeof(CHECK_PROCESS_RIGHTS_RESPONSE)))
		{
			uRet = ERROR_INVALID_PARAMETER;
			LOGMAN_ERROR(hMgr, GetLastError(), L"Fail to post response");
			break;
		}

	} while (FALSE);

	return uRet;
}

ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostUpdateOverlayWindowResponse(
	NXRMDRV_HANDLE					hMgr,
	PVOID							msgctx)
{
	ULONG uRet = ERROR_SUCCESS;

	NXRMDRV_MANAGER	*pMgr = NULL;

	pMgr = (NXRMDRV_MANAGER *)hMgr;

	do
	{
		if (!pMgr)
		{
			uRet = ERROR_INVALID_PARAMETER;
			break;
		}

		if (!msgctx)
		{
			uRet = ERROR_INVALID_PARAMETER;
			LOGMAN_ERROR(hMgr, GetLastError(), L"Invalid message context");
			break;
		}

		if (((NXRMRECORD*)msgctx)->hdr.record_type != NXRMDRV_MSG_TYPE_UPDATE_OVERLAY_WINDOW)
		{
			uRet = ERROR_INVALID_PARAMETER;
			LOGMAN_ERROR(hMgr, GetLastError(), L"Mismatch message type");
			break;
		}

		if (!post_response(pMgr->nxrmdrvsection, (NXRMRECORD*)msgctx, NULL, 0))
		{
			uRet = ERROR_INVALID_PARAMETER;
			LOGMAN_ERROR(hMgr, GetLastError(), L"Fail to post response");
			break;
		}

	} while (FALSE);

	return uRet;
}

ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanQueryProcessInfo(
	NXRMDRV_HANDLE				hMgr, 
	ULONG						ProcessId, 
	NXRM_PROCESS_ENTRY			*Info)
{
	HRESULT	nRet = ERROR_SUCCESS;

	NXRMDRV_MANAGER	*pMgr = NULL;

	NXRM_PROCESS_ENTRY		Entry;
	NXRM_PROCESS_ENTRY		*pEntry = NULL;
	ULONG					bytesin;
	ULONG					bytesout;

	pMgr	= (NXRMDRV_MANAGER*)hMgr;

	do 
	{
		if(!pMgr)
		{
			nRet = ERROR_INVALID_HANDLE;
			break;
		}

		EnterCriticalSection(&pMgr->ProcessCacheLock);

		do 
		{
			if((HANDLE)(ULONG_PTR)ProcessId == pMgr->LastCachedProcessNode.process_id && 
			   ProcessId != 0)
			{
				memcpy(Info, &pMgr->LastCachedProcessNode, sizeof(*Info));
				pEntry = (NXRM_PROCESS_ENTRY*)((ULONG_PTR)(-1));// bypass pEntry == NULL check after break
				break;
			}

			pEntry = find_process_in_cache(&pMgr->ProcessCache, (HANDLE)(ULONG_PTR)ProcessId);

			if(pEntry)
			{
				Info->process_id	= (HANDLE)(ULONG_PTR)ProcessId;
				Info->parent_id		= pEntry->parent_id;
				Info->session_id	= pEntry->session_id;
				Info->hooked		= pEntry->hooked;
				Info->platform		= pEntry->platform;
				Info->is_service	= pEntry->is_service;

				memcpy(Info->process_path,
					   pEntry->process_path,
					   min(sizeof(Info->process_path), sizeof(pEntry->process_path)));

				memcpy(&pMgr->LastCachedProcessNode, Info, sizeof(pMgr->LastCachedProcessNode));

			}

		} while (FALSE);

		LeaveCriticalSection(&pMgr->ProcessCacheLock);

		//
		// query kernel directly when we can't find it in user mode cache
		//
		if(!pEntry)
		{
			memset(&Entry,0,sizeof(Entry));

			bytesin		= sizeof(NXRM_PROCESS_ENTRY);
			bytesout	= 0;

			nRet = query_process_information(pMgr->nxrmdrvsection,
											 (HANDLE)(ULONG_PTR)ProcessId,
											 bytesin,
											 (UCHAR*)&Entry,
											 &bytesout);
			if(nRet != ERROR_SUCCESS)
			{
				break;
			}

			Info->process_id	= (HANDLE)(ULONG_PTR)ProcessId;
			Info->parent_id		= Entry.parent_id;
			Info->session_id	= Entry.session_id;
			Info->hooked		= Entry.hooked;
			Info->platform		= Entry.platform;
			Info->is_service	= Entry.is_service;

			memcpy(Info->process_path,
				   Entry.process_path,
				   min(sizeof(Info->process_path), sizeof(Entry.process_path)));

			memcpy(&pMgr->LastCachedProcessNode, Info, sizeof(pMgr->LastCachedProcessNode));

		}

	} while(FALSE);

	return nRet;
}

ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanCloseManager(NXRMDRV_HANDLE hMgr)
{
	ULONG uRet = ERROR_SUCCESS;

	NXRMDRV_MANAGER	*pMgr = NULL;

	HANDLE hEvents[2] = {0};

	LIST_ENTRY *ite = NULL;
	LIST_ENTRY *tmp = NULL;

	pMgr = (NXRMDRV_MANAGER*)hMgr;

	do 
	{
		if (!hMgr)
		{
			uRet = ERROR_INVALID_PARAMETER;
			break;
		}

		pMgr->DisableFiltering = TRUE;
		pMgr->Stop = TRUE;

		SetEvent(pMgr->StopEvent);

		hEvents[0] = pMgr->WorkerThread;
		hEvents[1] = pMgr->ProcessCacheThread;

		WaitForMultipleObjects(sizeof(hEvents)/sizeof(HANDLE),
							   hEvents,
							   TRUE,
							   30 * 1000);

		CloseHandle(pMgr->WorkerThread);
		CloseHandle(pMgr->ProcessCacheThread);

		CloseHandle(pMgr->StopEvent);

		if (pMgr->ProcessCacheInfo)
		{
			//
			// we don't need to free node in rb_tree. Instead, we free ProcessCacheInfo
			//
			free(pMgr->ProcessCacheInfo);
			pMgr->ProcessCacheInfo = NULL;
			pMgr->ProcessCacheInfoSize = 0;
		}

		close_transporter_server(pMgr->nxrmdrvsection);
		
		pMgr->nxrmdrvsection = NULL;

		DeleteCriticalSection(&pMgr->ProcessCacheLock);

		FOR_EACH_LIST_SAFE(ite, tmp, &pMgr->ProtectedModeAppInfoList)
		{
			PROTECTEDMODEAPPINFO *pNode = CONTAINING_RECORD(ite, PROTECTEDMODEAPPINFO, Link);

			RemoveEntryList(ite);

			free(pNode);
		}

		free(pMgr);

		pMgr = NULL;

		InterlockedExchange(&g_nxrmdrvmgrcreated, 0);
		
	} while (FALSE);

	return uRet;
}

DWORD WINAPI nxrmdrvmanWorker(_In_ LPVOID lpParameter)
{
	UINT uRet = 0;

	NXRMDRV_MANAGER	*pMgr = NULL;
	NXRMRECORD		*record = NULL;

	BOOL	bstop = FALSE;

	DWORD	dwWaitRet = 0;

	HANDLE	hEvents[2];

	pMgr = (NXRMDRV_MANAGER*)lpParameter;

	hEvents[0] = pMgr->StopEvent;
	hEvents[1] = get_transporter_semaphore(pMgr->nxrmdrvsection);

	enable_transporter(pMgr->nxrmdrvsection);

	while(!bstop)
	{

		dwWaitRet = WaitForMultipleObjects(sizeof(hEvents)/sizeof(HANDLE),
										   hEvents,
										   FALSE,
										   100);
		switch (dwWaitRet)
		{
		case WAIT_OBJECT_0:
			bstop = TRUE;
			break;

		case WAIT_OBJECT_0+1:

			record = find_pending_request(pMgr->nxrmdrvsection);

			if(record)
			{
				if(record->hdr.record_type == NXRMDRV_MSG_TYPE_UPDATE_PROTECTEDMODEAPPINFO)
				{
					if (ERROR_SUCCESS != handle_update_protected_app_info_request(record->hdr.record_type, (PVOID)record->data, record->length, (PVOID)record, (PVOID)pMgr))
					{
						record->hdr.record_state = NXRM_RECORD_STATE_BAD_RECORD;
					}
				}
				else if (record->hdr.record_type == NXRMDRV_MSG_TYPE_QUERY_PROTECTEDMODEAPPINFO)
				{
					if (ERROR_SUCCESS != handle_query_protected_app_info_request(record->hdr.record_type, (PVOID)record->data, record->length, (PVOID)record, (PVOID)pMgr))
					{
						record->hdr.record_state = NXRM_RECORD_STATE_BAD_RECORD;
					}
				}
				else
				{
					if(pMgr->NotifyCallback)
					{
						if(ERROR_SUCCESS != pMgr->NotifyCallback(record->hdr.record_type, (PVOID)record->data, record->length, (PVOID)record, pMgr->UserCtx))
						{
							record->hdr.record_state = NXRM_RECORD_STATE_BAD_RECORD;
						}
					}
				}
			}

			if(GetTickCount() - pMgr->WorkerThreadRecyleTick > 1000)
			{
				//
				// It's time to do some recycle work
				//
				recycle_cancel_request(pMgr->nxrmdrvsection);
				pMgr->WorkerThreadRecyleTick = GetTickCount();
			}

			break;

		case WAIT_FAILED:
			Sleep(10);
			break;

		case WAIT_TIMEOUT:
		default:
			if(GetTickCount() - pMgr->WorkerThreadRecyleTick > 1000)
			{
				//
				// It's time to do some recycle work
				//
				recycle_cancel_request(pMgr->nxrmdrvsection);
				pMgr->WorkerThreadRecyleTick = GetTickCount();
			}
			break;
		}
	}

	disable_transporter(pMgr->nxrmdrvsection);

	return uRet;
}

DWORD WINAPI nxrmdrvProcessCacheWorker(_In_ LPVOID lpParameter)
{
	UINT uRet = 0;

	NXRMDRV_MANAGER	*pMgr = NULL;

	BOOL	bstop = FALSE;

	DWORD	dwWaitRet = 0;
	DWORD	dwBytesReturn = 0;
	BOOL	bCacheValid = FALSE;

	rb_node	*node = NULL;

	HANDLE	hEvent[2];

	LIST_ENTRY *ite = NULL;
	LIST_ENTRY *tmp = NULL;

	pMgr = (NXRMDRV_MANAGER*)lpParameter;

	hEvent[0] = pMgr->StopEvent;
	hEvent[1] = get_notify_event(pMgr->nxrmdrvsection);

	while(!bstop)
	{

		dwWaitRet = WaitForMultipleObjects(sizeof(hEvent)/sizeof(HANDLE),
										   hEvent,
										   FALSE,
										   INFINITE);
		switch (dwWaitRet)
		{
		case WAIT_OBJECT_0:
			bstop = TRUE;
			break;

		case WAIT_OBJECT_0+1:

			if(ERROR_MORE_DATA == get_loader_snapshot(pMgr->nxrmdrvsection, 0, NULL, &dwBytesReturn))
			{
				EnterCriticalSection(&pMgr->ProcessCacheLock);

				if(dwBytesReturn > pMgr->ProcessCacheInfoSize)
				{
					do 
					{
						if(pMgr->ProcessCacheInfo)
						{
							free(pMgr->ProcessCacheInfo);
							pMgr->ProcessCacheInfo = NULL;
							pMgr->ProcessCacheInfoSize = 0;
						}

						dwBytesReturn = (dwBytesReturn + 20 * sizeof(NXRM_PROCESS_ENTRY) + 4095) & (~4095);

						pMgr->ProcessCacheInfo = (NXRM_PROCESS_INFO*)malloc(dwBytesReturn);

						if(!pMgr->ProcessCacheInfo)
						{
							pMgr->ProcessCacheInfoSize = 0;
							break;
						}

						pMgr->ProcessCacheInfoSize = dwBytesReturn;

					} while (FALSE);

				}

				do 
				{
					if(!pMgr->ProcessCacheInfo)
					{
						pMgr->ProcessCacheInfoSize = 0;
						break;
					}

					pMgr->ProcessCache.rb_node = NULL;

					if(ERROR_SUCCESS != get_loader_snapshot(pMgr->nxrmdrvsection, pMgr->ProcessCacheInfoSize, (UCHAR*)pMgr->ProcessCacheInfo, &dwBytesReturn))
					{
						free(pMgr->ProcessCacheInfo);
						pMgr->ProcessCacheInfo = NULL;
						pMgr->ProcessCacheInfoSize = 0;
						break;
					}

					bCacheValid = FALSE;

					for(ULONG i = 0; i < pMgr->ProcessCacheInfo->NumberOfEntry;i++)
					{
						add_process_into_cache(&pMgr->ProcessCache, &pMgr->ProcessCacheInfo->Entry[i]);

						if(!bCacheValid)
						{
							if(pMgr->ProcessCacheInfo->Entry[i].process_id == pMgr->LastCachedProcessNode.process_id)
							{
								if(memcmp(pMgr->ProcessCacheInfo->Entry[i].process_path,
										  pMgr->LastCachedProcessNode.process_path,
										  sizeof(pMgr->LastCachedProcessNode.process_path)) == 0)
								{
									bCacheValid = TRUE;
								}
							}

						}
					}

					if(!bCacheValid)
					{
						memset(&pMgr->LastCachedProcessNode, 0, sizeof(pMgr->LastCachedProcessNode));
					}

				} while (FALSE);

				LeaveCriticalSection(&pMgr->ProcessCacheLock);

				//
				// go through ProtectedMode App list
				//
				EnterCriticalSection(&pMgr->ProtectedModeAppInfoListLock);

				FOR_EACH_LIST_SAFE(ite, tmp, &pMgr->ProtectedModeAppInfoList)
				{
					PROTECTEDMODEAPPINFO *pNode = CONTAINING_RECORD(ite, PROTECTEDMODEAPPINFO, Link);

					if (ERROR_NOT_FOUND == nxrmdrvmanIsProcessAlive((NXRMDRV_HANDLE)pMgr, pNode->ProcessId) ||
						ERROR_NOT_FOUND == nxrmdrvmanIsProcessAlive((NXRMDRV_HANDLE)pMgr, pNode->ProtectedChildProcessId))
					{
						RemoveEntryList(ite);

						free(pNode);
					}
				}

				LeaveCriticalSection(&pMgr->ProtectedModeAppInfoListLock);

			}
			else
			{                
                LOGMAN_ERROR(pMgr, GetLastError(), L"Can not query the size of current loader snapshot");
			}

			break;
		case WAIT_FAILED:
			Sleep(10);
			break;

		case WAIT_TIMEOUT:
		default:
			break;
		}
	}

	return uRet;
}

static NXRM_PROCESS_ENTRY* find_process_in_cache(rb_root *proc_cache, HANDLE process_id)
{
	NXRM_PROCESS_ENTRY *process = NULL;

	rb_node *node = NULL;

	node = proc_cache->rb_node;

	while (node) 
	{ 
		process = rb_entry(node, NXRM_PROCESS_ENTRY, node);

		if (process_id < process->process_id)
		{
			node = node->rb_left;
			process = NULL;
		}
		else if (process_id > process->process_id)
		{
			node = node->rb_right;
			process = NULL;
		}
		else 
			break; 
	}

	return process; 
}

static BOOL add_process_into_cache(rb_root *proc_cache, NXRM_PROCESS_ENTRY *proc)
{
	NXRM_PROCESS_ENTRY *process = NULL;

	rb_node **ite;
	rb_node *parent = NULL;

	ite = &(proc_cache->rb_node);
	parent = NULL;

	while (*ite) 
	{ 
		process = rb_entry(*ite, NXRM_PROCESS_ENTRY, node);

		parent = *ite;

		if (proc->process_id < process->process_id) 
			ite = &((*ite)->rb_left); 
		else if (proc->process_id > process->process_id) 
			ite = &((*ite)->rb_right); 
		else 
			return FALSE; 
	}

	rb_link_node(&proc->node, parent, ite);
	rb_insert_color(&proc->node, proc_cache);

	return TRUE;
}

ULONG	NXRMDRVMAN_API	__stdcall nxrmdrvmanIsRequestCancelled(
	NXRMDRV_HANDLE				hMgr, 
	PVOID						msgctx)
{
	BOOLEAN bRet = FALSE;

	NXRMRECORD	*record = NULL;

	do 
	{
		record = (NXRMRECORD*)msgctx;

		if(record->hdr.record_cancelled)
		{
			bRet = TRUE;
		}

	} while (FALSE);

	return bRet;
}

ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanIsProcessAlive(
	NXRMDRV_HANDLE				hMgr, 
	ULONG						PID)
{
	HRESULT	nRet = ERROR_NOT_FOUND;

	NXRMDRV_MANAGER		*pMgr = NULL;
	NXRM_PROCESS_ENTRY	*pEntry = NULL;

	pMgr	= (NXRMDRV_MANAGER*)hMgr;

	do 
	{
		if(!pMgr)
		{
			nRet = ERROR_INVALID_HANDLE;
			break;
		}

		EnterCriticalSection(&pMgr->ProcessCacheLock);

		do 
		{
			if((HANDLE)(ULONG_PTR)PID == pMgr->LastCachedProcessNode.process_id &&
			   PID != 0)
			{
				nRet = ERROR_SUCCESS;
				break;
			}

			pEntry = find_process_in_cache(&pMgr->ProcessCache, (HANDLE)(ULONG_PTR)PID);

			if(pEntry)
			{
				nRet = ERROR_SUCCESS;
				break;
			}
			else
			{
				nRet = ERROR_NOT_FOUND;
			}

		}while(FALSE);

		LeaveCriticalSection(&pMgr->ProcessCacheLock);

	}while (FALSE);

	return nRet;
}

ULONG	NXRMDRVMAN_API	__stdcall nxrmdrvmanIncreasePolicySN(
	NXRMDRV_HANDLE				hMgr)
{
	HRESULT	nRet = ERROR_SUCCESS;

	NXRMDRV_MANAGER		*pMgr = NULL;

	pMgr	= (NXRMDRV_MANAGER*)hMgr;

	do 
	{
		if(!pMgr)
		{
			nRet = ERROR_INVALID_HANDLE;
			break;
		}

		increase_rm_policy_sn(pMgr->nxrmdrvsection);

	}while (FALSE);

	return nRet;
}

ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanSetOverlayProtectedWindows(
	NXRMDRV_HANDLE				hMgr,
	ULONG						SessionId,
	ULONG						*hWnd,
	ULONG						*cbSize)
{
	HRESULT	nRet = ERROR_SUCCESS;

	NXRMDRV_MANAGER		*pMgr = NULL;

	ULONG cbCopiedSize = 0;

	pMgr = (NXRMDRV_MANAGER*)hMgr;

	do
	{
		if (!pMgr)
		{
			nRet = ERROR_INVALID_HANDLE;
			break;
		}

		if (get_dwm_active_session(pMgr->nxrmdrvsection) == SessionId)
		{
			cbCopiedSize = set_overlay_windows(pMgr->nxrmdrvsection, hWnd, *cbSize);

#ifdef _DEBUG

			for (ULONG i = 0; i < cbCopiedSize / sizeof(ULONG); i++)
			{
				LOGMAN_DBG(pMgr, L"Adding Overlay Wnd[%d]=%x", i, hWnd[i]);
			}
#endif

		}

		*cbSize = cbCopiedSize;

	} while (FALSE);

	return nRet;
}

ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanSetOverlayBitmapStatus(
	NXRMDRV_HANDLE				hMgr,
	ULONG						SessionId,
	BOOL						bReady)
{
	HRESULT	nRet = ERROR_SUCCESS;

	NXRMDRV_MANAGER		*pMgr = NULL;

	ULONG cbCopiedSize = 0;

	pMgr = (NXRMDRV_MANAGER*)hMgr;

	do
	{
		if (!pMgr)
		{
			nRet = ERROR_INVALID_HANDLE;
			break;
		}

		//if (get_dwm_active_session(pMgr->nxrmdrvsection) == SessionId)
		{
			if (bReady)
				set_channel_mask(pMgr->nxrmdrvsection, NX_CHANNEL_MASK_OVERLAY_BITMAP_READY);
			else
				set_channel_mask(pMgr->nxrmdrvsection, 0);
		}

	} while (FALSE);

	return nRet;
}

static HRESULT handle_update_protected_app_info_request(ULONG				type,
														PVOID				msg,
														ULONG				Length,
														PVOID				msgctx,
														PVOID				userctx)
{
	HRESULT hr = E_UNEXPECTED;

	UPDATE_PROTECTEDMODEAPPINFO_REQUEST *pReq = NULL;
	UPDATE_PROTECTEDMODEAPPINFO_RESPONSE resp = {0};

	NXRMDRV_MANAGER *pMgr = (NXRMDRV_MANAGER *)userctx;

	NXRM_PROCESS_ENTRY AppInfo = {0};

	LIST_ENTRY *ite = NULL;

	PROTECTEDMODEAPPINFO *pProtectedModeAppInfo = NULL;
	
	do 
	{
		if (type != NXRMDRV_MSG_TYPE_UPDATE_PROTECTEDMODEAPPINFO)
		{
			break;
		}

		if (Length != sizeof(UPDATE_PROTECTEDMODEAPPINFO_REQUEST))
		{
			break;
		}

		if (nxrmdrvmanIsRequestCancelled((NXRMDRV_HANDLE)pMgr, msgctx))
		{
			break;
		}

		pReq = (UPDATE_PROTECTEDMODEAPPINFO_REQUEST *)msg;

		hr = nxrmdrvmanQueryProcessInfo((NXRMDRV_HANDLE)pMgr, pReq->ProcessId, &AppInfo);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		EnterCriticalSection(&pMgr->ProtectedModeAppInfoListLock);

		FOR_EACH_LIST(ite, &pMgr->ProtectedModeAppInfoList)
		{
			pProtectedModeAppInfo = CONTAINING_RECORD(ite, PROTECTEDMODEAPPINFO, Link);

			if (pProtectedModeAppInfo->ProcessId == (ULONG)(ULONG_PTR)AppInfo.parent_id)
			{
				break;
			}
			else
			{
				pProtectedModeAppInfo = NULL;
			}
		}

		if (pProtectedModeAppInfo)
		{
			//
			// found the record, update it
			//
			memset(pProtectedModeAppInfo->ActiveDocFileName,
				   0,
				   sizeof(pProtectedModeAppInfo->ActiveDocFileName));

			memcpy(pProtectedModeAppInfo->ActiveDocFileName,
				   pReq->ActiveDocFileName,
				   min(sizeof(pProtectedModeAppInfo->ActiveDocFileName) - sizeof(WCHAR), wcslen(pReq->ActiveDocFileName)*sizeof(WCHAR)));

			pProtectedModeAppInfo->ProtectedChildProcessId = (ULONG)(ULONG_PTR)AppInfo.process_id;

			pProtectedModeAppInfo->RightsMask	= pReq->RightsMask;
			pProtectedModeAppInfo->CustomRights = pReq->CustomRights;
			pProtectedModeAppInfo->EvaluationId	= pReq->EvaluationId;
		}
		else
		{
			//
			// no record found, create it
			//

			pProtectedModeAppInfo = (PROTECTEDMODEAPPINFO*)malloc(sizeof(PROTECTEDMODEAPPINFO));

			if (pProtectedModeAppInfo)
			{
				memset(pProtectedModeAppInfo->ActiveDocFileName,
					   0,
					   sizeof(pProtectedModeAppInfo->ActiveDocFileName));

				memcpy(pProtectedModeAppInfo->ActiveDocFileName,
					   pReq->ActiveDocFileName,
					   min(sizeof(pProtectedModeAppInfo->ActiveDocFileName) - sizeof(WCHAR), wcslen(pReq->ActiveDocFileName)*sizeof(WCHAR)));

				pProtectedModeAppInfo->ProtectedChildProcessId	= (ULONG)(ULONG_PTR)AppInfo.process_id;
				pProtectedModeAppInfo->ProcessId				= (ULONG)(ULONG_PTR)AppInfo.parent_id;

				pProtectedModeAppInfo->RightsMask	= pReq->RightsMask;
				pProtectedModeAppInfo->CustomRights = pReq->CustomRights;
				pProtectedModeAppInfo->EvaluationId	= pReq->EvaluationId;

				InsertHeadList(&pMgr->ProtectedModeAppInfoList, &pProtectedModeAppInfo->Link);
			}
		}

		LeaveCriticalSection(&pMgr->ProtectedModeAppInfoListLock);

		if (!pProtectedModeAppInfo)
		{
			LOGMAN_ERROR(pMgr, GetLastError(), L"Update protected mode app information failed due to out of memory");

			resp.Ack = 0xffffffff;
		}

		if (!post_response(pMgr->nxrmdrvsection, (NXRMRECORD*)msgctx, (PVOID)&resp, sizeof(resp)))
		{
			LOGMAN_ERROR(pMgr, GetLastError(), L"Fail to post Update protected mode app information response");
		}

		hr = S_OK;

	} while (FALSE);

	return hr;
}

static HRESULT handle_query_protected_app_info_request(ULONG				type,
													   PVOID				msg,
													   ULONG				Length,
													   PVOID				msgctx,
													   PVOID				userctx)
{
	HRESULT hr = E_UNEXPECTED;

	QUERY_PROTECTEDMODEAPPINFO_REQUEST *pReq = NULL;
	QUERY_PROTECTEDMODEAPPINFO_RESPONSE resp = {0};

	NXRMDRV_MANAGER *pMgr = (NXRMDRV_MANAGER *)userctx;

	LIST_ENTRY *ite = NULL;

	PROTECTEDMODEAPPINFO *pProtectedModeAppInfo = NULL;

	do 
	{
		if (type != NXRMDRV_MSG_TYPE_QUERY_PROTECTEDMODEAPPINFO)
		{
			break;
		}

		if (Length != sizeof(QUERY_PROTECTEDMODEAPPINFO_REQUEST))
		{
			break;
		}

		if (nxrmdrvmanIsRequestCancelled((NXRMDRV_HANDLE)pMgr, msgctx))
		{
			break;
		}

		pReq = (QUERY_PROTECTEDMODEAPPINFO_REQUEST *)msg;

		EnterCriticalSection(&pMgr->ProtectedModeAppInfoListLock);

		FOR_EACH_LIST(ite, &pMgr->ProtectedModeAppInfoList)
		{
			pProtectedModeAppInfo = CONTAINING_RECORD(ite, PROTECTEDMODEAPPINFO, Link);

			if (pProtectedModeAppInfo->ProcessId == pReq->ProcessId)
			{
				break;
			}
			else
			{
				pProtectedModeAppInfo = NULL;
			}
		}

		if (pProtectedModeAppInfo)
		{
			//
			// found the record, copy it to response
			//
			resp.Ack = 0;

			memcpy(resp.ActiveDocFileName,
				   pProtectedModeAppInfo->ActiveDocFileName,
				   min(sizeof(resp.ActiveDocFileName) - sizeof(WCHAR), wcslen(pProtectedModeAppInfo->ActiveDocFileName)*sizeof(WCHAR)));

			resp.RightsMask		= pProtectedModeAppInfo->RightsMask;
			resp.CustomRights	= pProtectedModeAppInfo->CustomRights;
			resp.EvaluationId	= pProtectedModeAppInfo->EvaluationId;
		}
		else
		{
			//
			// no record found, set Act to error
			//
			resp.Ack = 0xffffffff;
		}

		LeaveCriticalSection(&pMgr->ProtectedModeAppInfoListLock);

		if (!post_response(pMgr->nxrmdrvsection, (NXRMRECORD*)msgctx, (PVOID)&resp, sizeof(resp)))
		{
			LOGMAN_ERROR(pMgr, GetLastError(), L"Fail to post query protected mode app information response");
		}

		hr = S_OK;

	} while (FALSE);

	return hr;
}