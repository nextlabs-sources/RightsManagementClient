#pragma once

#include "nxrmdrv.h"
#include <nudf\shared\logdef.h>
#include <nudf\shared\rightsdef.h>
#include <nudf\shared\obdef.h>

#ifdef _X86_
#ifdef NXRMDRVMAN_EXPORTS
#define NXRMDRVMAN_API __declspec(dllexport)
#else
#define NXRMDRVMAN_API __declspec(dllimport)
#endif
#else
#define NXRMDRVMAN_API
#endif

typedef PVOID	NXRMDRV_HANDLE;

#define NXRMDRV_MSG_TYPE_CHECKOBLIGATION	(0x90000001)
#define NXRMDRV_MSG_TYPE_GET_CUSTOMUI		(0x90000002)
#define NXRMDRV_MSG_TYPE_SAVEAS_FORECAST	(0x90000003)
#define NXRMDRV_MSG_TYPE_GET_CLASSIFYUI		(0x90000005)
#define NXRMDRV_MSG_TYPE_BLOCK_NOTIFICATION	(0x90000006)
#define NXRMDRV_MSG_TYPE_CHECK_PROTECT_MENU	(0x90000007)

#define NXRMDRV_MSG_TYPE_UPDATE_PROTECTEDMODEAPPINFO	(0x90000008)
#define NXRMDRV_MSG_TYPE_QUERY_PROTECTEDMODEAPPINFO		(0x90000009)

#define NXRMDRV_MSG_TYPE_GET_CTXMENUREGEX	(0x9000000A)

#define NXRMDRV_MSG_TYPE_UPDATE_CORE_CTX	(0x9000000B)
#define NXRMDRV_MSG_TYPE_QUERY_CORE_CTX		(0x9000000C)

#define NXRMDRV_MSG_TYPE_QUERY_SERVICE		(0x9000000D)

#define NXRMDRV_MSG_TYPE_UPDATE_DWM_WINDOW		(0x9000000E)
#define NXRMDRV_MSG_TYPE_UPDATE_OVERLAY_WINDOW	(0x9000000F)

#define NXRMDRV_MSG_TYPE_CHECK_PROCESS_RIGHTS	(0x90000010)

#define NXRMDRV_OFFICE_POWERPOINT			(0x8001)
#define NXRMDRV_OFFICE_WINWORD				(0x8002)
#define NXRMDRV_OFFICE_EXCEL				(0x8003)
#define NXRMDRV_OFFICE_OUTLOOK				(0x8004)

#define NXRMDRV_DWM_WINDOW_ADD				(0)
#define NXRMDRV_DWM_WINDOW_DELETE			(1)

#define NXRMDRV_OVERLAY_WINDOW_ADD			(3)
#define NXRMDRV_OVERLAY_WINDOW_DELETE		(4)

#define	MAX_OFFICE_VER_LENGTH				8

#define MAX_CTXMENUREGEX_LENGTH				1024

#define MAX_SERVICE_DATA_LENGTH				2048

#define MAX_CTX_COUNT						64

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct _OFFICE_GET_CUSTOMUI_REQUEST {

		ULONG				ProcessId;

		ULONG				ThreadId;

		WCHAR				OfficeVersion[MAX_OFFICE_VER_LENGTH];

//		+-------------------------+-------------------------+
//		|     SubLanguage ID      |   Primary Language ID   |
//		+-------------------------+-------------------------+
//		15                    10  9                         0   bit

		USHORT				OfficeLanguageId;

		USHORT				OfficeProduct;

		WCHAR				TempPath[MAX_PATH];

	}OFFICE_GET_CUSTOMUI_REQUEST, *POFFICE_GET_CUSTOMUI_REQUEST;

	typedef struct _OFFICE_GET_CUSTOMUI_RESPONSE {

		WCHAR				CustomUIFileName[MAX_PATH];

	}OFFICE_GET_CUSTOMUI_RESPONSE, *POFFICE_GET_CUSTOMUI_RESPONSE;

	typedef struct _CHECK_OBLIGATION_REQUEST {

		ULONG				ProcessId;

		ULONG				ThreadId;

		ULONGLONG			EvaluationId;

		WCHAR				FileName[MAX_PATH];

		WCHAR				TempPath[MAX_PATH];

	}CHECK_OBLIGATION_REQUEST, *PCHECK_OBLIGATION_REQUEST;

	typedef struct _SAVEAS_FORECAST_REQUEST {

		ULONG				ProcessId;

		ULONG				ThreadId;

		WCHAR				SourceFileName[MAX_PATH];

		WCHAR				SaveAsFileName[MAX_PATH];

	}SAVEAS_FORECAST_REQUEST, *PSAVEAS_FORECAST_REQUEST;

	typedef struct _GET_CLASSIFY_UI_REQUEST {

		ULONG				ProcessId;

		ULONG				ThreadId;

		ULONG				SessionId;

		USHORT				LanguageId;

		WCHAR				TempPath[MAX_PATH];

	}GET_CLASSIFY_UI_REQUEST, *PGET_CLASSIFY_UI_REQUEST;

	typedef struct _GET_CLASSIFY_UI_RESPONSE {

		WCHAR				ClassifyUIFileName[MAX_PATH];

		WCHAR				GroupName[MAX_PATH];

	}GET_CLASSIFY_UI_RESPONSE, *PGET_CLASSIFY_UI_RESPONSE;

	typedef enum _BLOCK_NOTIFICATION_TYPE {

		NxrmdrvSaveFileBlocked = 0x0001,
		NXrmdrvPrintingBlocked,
		NxrmdrvCopyFileToSystemRootBlocked,
		NxrmdrvSendMailBlocked,
		NxrmdrvEmbeddedOleObjBlocked,
		NxrmdrvExportSlidesBlocked,
		NxrmdrvSaveAsToUnprotectedVolume,
		NxrmdrvAdobeHookIsNotReady,
		NxrmdrvNotAuthorized,
		NxrmdrvShellPrintBlocked

	} BLOCK_NOTIFICATION_TYPE;

	typedef struct _BLOCK_NOTIFICATION_REQUEST {

		ULONG					ProcessId;

		ULONG					ThreadId;

		ULONG					SessionId;

		USHORT					LanguageId;
		
		BLOCK_NOTIFICATION_TYPE	Type;
	
		WCHAR					FileName[MAX_PATH];

		WCHAR					FileName2[MAX_PATH];

	}BLOCK_NOTIFICATION_REQUEST, *PBLOCK_NOTIFICATION_REQUEST;

	typedef struct _CHECK_PROTECT_MENU_REQUEST {

		ULONG					ProcessId;

		ULONG					ThreadId;

		ULONG					SessionId;

	}CHECK_PROTECT_MENU_REQUEST, *PCHECK_PROTECT_MENU_REQUEST;

	typedef struct _CHECK_PROTECT_MENU_RESPONSE {

		ULONG					EnableProtectMenu;

	} CHECK_PROTECT_MENU_RESPONSE, *PCHECK_PROTECT_MENU_RESPONSE;

	typedef struct _UPDATE_PROTECTEDMODEAPPINFO_REQUEST {

		ULONG					ProcessId;

		ULONG					ThreadId;

		ULONG					SessionId;

		WCHAR					ActiveDocFileName[MAX_PATH];

		ULONGLONG				RightsMask;

		ULONGLONG				CustomRights;

		ULONGLONG				EvaluationId;

	} UPDATE_PROTECTEDMODEAPPINFO_REQUEST, *PUPDATE_PROTECTEDMODEAPPINFO_REQUEST;

	typedef struct _UPDATE_PROTECTEDMODEAPPINFO_RESPONSE {

		ULONG					Ack;
	
	} UPDATE_PROTECTEDMODEAPPINFO_RESPONSE, *PUPDATE_PROTECTEDMODEAPPINFO_RESPONSE;

	typedef struct _QUERY_PROTECTEDMODEAPPINFO_REQUEST {

		ULONG					ProcessId;

		ULONG					ThreadId;

		ULONG					SessionId;

	} QUERY_PROTECTEDMODEAPPINFO_REQUEST, *PQUERY_PROTECTEDMODEAPPINFO_REQUEST;

	typedef struct _QUERY_PROTECTEDMODEAPPINFO_RESPONSE {

		ULONG					Ack;

		WCHAR					ActiveDocFileName[MAX_PATH];

		ULONGLONG				RightsMask;

		ULONGLONG				CustomRights;

		ULONGLONG				EvaluationId;

	} QUERY_PROTECTEDMODEAPPINFO_RESPONSE, *PQUERY_PROTECTEDMODEAPPINFO_RESPONSE;

	typedef struct _QUERY_CTXMENUREGEX_REQUEST {

		ULONG					ProcessId;

		ULONG					ThreadId;

		ULONG					SessionId;

	} QUERY_CTXMENUREGEX_REQUEST, *PQUERY_CTXMENUREGEX_REQUEST;

	typedef struct _QUERY_CTXMENUREGEX_RESPONSE {

		WCHAR					CtxMenuRegEx[MAX_CTXMENUREGEX_LENGTH];
	
	} QUERY_CTXMENUREGEX_RESPONSE, *PQUERY_CTXMENUREGEX_RESPONSE;

	typedef struct _UPDATE_CORE_CTX_REQUEST {

		ULONG					ProcessId;

		ULONG					ThreadId;

		ULONG					SessionId;

		WCHAR					ModuleFullPath[MAX_PATH];

		ULONGLONG				ModuleChecksum;

		ULONGLONG				CtxData[MAX_CTX_COUNT];

	} UPDATE_CORE_CTX_REQUEST, *PUPDATE_CORE_CTX_REQUEST;

	typedef struct _QUERY_CORE_CTX_REQUEST {

		ULONG					ProcessId;

		ULONG					ThreadId;

		ULONG					SessionId;

		WCHAR					ModuleFullPath[MAX_PATH];

		ULONGLONG				ModuleChecksum;

	} QUERY_CORE_CTX_REQUEST, *PQUERY_CORE_CTX_REQUEST;

	typedef struct _QUERY_CORE_CTX_RESPONSE {

		ULONGLONG				ModuleChecksum;

		ULONGLONG				CtxData[MAX_CTX_COUNT];

	} QUERY_CORE_CTX_RESPONSE, *PQUERY_CORE_CTX_RESPONSE;

	typedef struct _QUERY_SERVICE_REQUEST {

		ULONG					ProcessId;

		ULONG					ThreadId;

		ULONG					SessionId;
		
		UCHAR					Data[MAX_SERVICE_DATA_LENGTH];

	}QUERY_SERVICE_REQUEST, *PQUERY_SERVICE_REQUEST;

	typedef struct _QUERY_SERVICE_RESPONSE {

		UCHAR					Data[MAX_SERVICE_DATA_LENGTH];

	} QUERY_SERVICE_RESPONSE, *PQUERY_SERVICE_RESPONSE;

	typedef struct _UPDATE_DWM_WINDOW_REQUEST {

		ULONG					ProcessId;

		ULONG					ThreadId;

		ULONG					SessionId;

		ULONG					hWnd;

		ULONG					Op;			// NXRMDRV_DWM_WINDOW_ADD or NXRMDRV_DWM_WINDOW_DELETE

	} UPDATE_DWM_WINDOW_REQUEST, *PUPDATE_DWM_WINDOW_REQUEST;

	typedef struct _UPDATE_OVERLAY_WINDOW_REQUEST {

		ULONG					ProcessId;

		ULONG					ThreadId;

		ULONG					SessionId;

		ULONG					hWnd;

		ULONG					Op;			// NXRMDRV_OVERLAY_WINDOW_ADD or NXRMDRV_OVERLAY_WINDOW_DELETE

	} UPDATE_OVERLAY_WINDOW_REQUEST, *PUPDATE_OVERLAY_WINDOW_REQUEST;

	typedef struct _CHECK_PROCESS_RIGHTS_REQUEST {

		ULONG					ProcessId;

		ULONG					SessionId;
	
	} CHECK_PROCESS_RIGHTS_REQUEST, *PCHECK_PROCESS_RIGHTS_REQUEST;

	typedef struct _CHECK_PROCESS_RIGHTS_RESPONSE {

		ULONGLONG				Rights;

		ULONGLONG				CustomRights;

	} CHECK_PROCESS_RIGHTS_RESPONSE, *PCHECK_PROCESS_RIGHTS_RESPONSE;

	/************************************************************************/
	/*                                                                      */
	/*	call back functions													*/
	/*                                                                      */
	/************************************************************************/

	typedef ULONG (__stdcall *NXRMDRV_CALLBACK_NOTIFY)(
		ULONG				type,
		PVOID				msg,
		ULONG				Length,
		PVOID				msgctx,
		PVOID				userctx);

	NXRMDRV_HANDLE NXRMDRVMAN_API __stdcall nxrmdrvmanCreateManager(
		NXRMDRV_CALLBACK_NOTIFY		NotifyCallback, 
		LOGAPI_LOG					DebugDumpCallback,
		LOGAPI_ACCEPT				DebugDumpCheckLevelCallback,
		PVOID						UserContext); 

	ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostCheckObligationsResponse(
		NXRMDRV_HANDLE					hMgr, 
		PVOID							msgctx, 
		NXRM_OBLIGATION					*Obligations,
		ULONG							ObligationsLength);

	ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostOfficeGetCustomUIResponse(
		NXRMDRV_HANDLE					hMgr,
		PVOID							msgctx,
		OFFICE_GET_CUSTOMUI_RESPONSE	*response);

	ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostSaveAsForecastResponse(
		NXRMDRV_HANDLE					hMgr,
		PVOID							msgctx);

	ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostGetClassifyUIResponse(
		NXRMDRV_HANDLE					hMgr,
		PVOID							msgctx,
		GET_CLASSIFY_UI_RESPONSE		*response);

	ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostBlockNotificationResponse(
		NXRMDRV_HANDLE					hMgr,
		PVOID							msgctx);

	ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostCheckProtectMenuResponse(
		NXRMDRV_HANDLE					hMgr,
		PVOID							msgctx,
		CHECK_PROTECT_MENU_RESPONSE		*response);

	ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostGetCtxMenuRegExResponse(
		NXRMDRV_HANDLE					hMgr,
		PVOID							msgctx,
		QUERY_CTXMENUREGEX_RESPONSE		*response);

	ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostUpdateCoreCtxResponse(
		NXRMDRV_HANDLE					hMgr,
		PVOID							msgctx);

	ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostQueryCoreCtxResponse(
		NXRMDRV_HANDLE					hMgr,
		PVOID							msgctx,
		QUERY_CORE_CTX_RESPONSE			*response);

	ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostQueryServiceResponse(
		NXRMDRV_HANDLE					hMgr,
		PVOID							msgctx,
		QUERY_SERVICE_RESPONSE			*response);

	ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostUpdateDwmWindowResponse(
		NXRMDRV_HANDLE					hMgr,
		PVOID							msgctx);

	ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostUpdateOverlayWindowResponse(
		NXRMDRV_HANDLE					hMgr,
		PVOID							msgctx);

	ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanPostCheckProcessRightsResponse(
		NXRMDRV_HANDLE					hMgr,
		PVOID							msgctx,
		CHECK_PROCESS_RIGHTS_RESPONSE	*response);

	ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanQueryProcessInfo(
		NXRMDRV_HANDLE				hMgr, 
		ULONG						ProcessId, 
		NXRM_PROCESS_ENTRY			*Info);

	ULONG	NXRMDRVMAN_API	__stdcall nxrmdrvmanIsRequestCancelled(
		NXRMDRV_HANDLE				hMgr, 
		PVOID						msgctx);

	ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanIsProcessAlive(
		NXRMDRV_HANDLE				hMgr, 
		ULONG						PID);

	ULONG	NXRMDRVMAN_API	__stdcall nxrmdrvmanIncreasePolicySN(
		NXRMDRV_HANDLE				hMgr);

	ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanSetOverlayBitmapStatus(
		NXRMDRV_HANDLE				hMgr,
		ULONG						SessionId,
		BOOL						bReady);

	ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanSetOverlayProtectedWindows(
		NXRMDRV_HANDLE				hMgr,
		ULONG						SessionId,
		ULONG						*hWnd,
		ULONG						*cbSize);

	ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanStart(NXRMDRV_HANDLE hMgr);

	ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanStop(NXRMDRV_HANDLE hMgr);

	ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanCloseManager(NXRMDRV_HANDLE hMgr);

	ULONG	NXRMDRVMAN_API __stdcall nxrmdrvmanEnableAntiTampering(BOOL Enable);

#ifdef __cplusplus
}
#endif