#include "stdafx.h"
#include "nxrmcoreglobal.h"
#include "nxrmdrv.h"
#include "nxrmcorehlp.h"
#include "detour.h"
#include "printingctl.h"
#include "adobe.h"

#ifdef __cplusplus
extern "C" {
#endif

	extern	CORE_GLOBAL_DATA Global;

	extern	BOOL send_block_notification(const WCHAR *FileName, BLOCK_NOTIFICATION_TYPE Type);

	extern  BOOL update_active_document_from_protected_child_process(void);

#ifdef __cplusplus
}
#endif

static STARTDOCW	g_fnorg_StartDocW = NULL;
static STARTDOCW	g_fn_StartDocW_trampoline = NULL;

static BOOL allow_process_to_print(void);

BOOL InitializePrintingCtlHook(void)
{
	BOOL bRet = TRUE;

	do 
	{
		if (!Global.gdi32Handle)
		{
			Global.gdi32Handle = GetModuleHandleW(GDI32_MODULE_NAME);
		}

		if (!Global.gdi32Handle)
		{
			bRet = FALSE;
			break;
		}

		if (Global.PrintingCtlHooksInitialized)
		{
			break;
		}

		g_fnorg_StartDocW = (STARTDOCW)GetProcAddress(Global.gdi32Handle, PRINTINGCTL_STARTDOCW_PROC_NAME);

		if (g_fnorg_StartDocW)
		{
			if (!install_hook(g_fnorg_StartDocW, (PVOID*)&g_fn_StartDocW_trampoline, (Global.IsMsOffice || Global.IsAdobeReader) ? Core_StartDocW : Core_StartDocW2))
			{
				bRet = FALSE;
				break;
			}
		}

		Global.PrintingCtlHooksInitialized = TRUE;

	} while (FALSE);

	return bRet;
}

void CleanupPrintingCtlHook(void)
{
	if (g_fn_StartDocW_trampoline)
	{
		remove_hook(g_fn_StartDocW_trampoline);
		g_fn_StartDocW_trampoline = NULL;
	}

	Global.PrintingCtlHooksInitialized = FALSE;
}

int WINAPI Core_StartDocW(_In_ HDC hdc, _In_ CONST DOCINFOW *lpdi)
{
	int nRet = 0;

	BOOL SkipOrignal = FALSE;

	WCHAR ActiveDoc[MAX_PATH] = {0};

	do 
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		if (Global.DisablePrinting)
		{
			nRet = -1;
			SkipOrignal = TRUE;
			SetLastError(ERROR_ACCESS_DISABLED_BY_POLICY);

			EnterCriticalSection(&Global.ActiveDocFileNameLock);

			memcpy(ActiveDoc,
				   Global.ActiveDocFileName,
				   min(sizeof(ActiveDoc) - sizeof(WCHAR), sizeof(Global.ActiveDocFileName) - sizeof(WCHAR)));

			LeaveCriticalSection(&Global.ActiveDocFileNameLock);

			send_block_notification(ActiveDoc, NXrmdrvPrintingBlocked);

			break;
		}

		if (!Global.IsAdobeReader)
		{
			break;
		}

		if (Global.AdobeMode == ADOBE_MODE_PARENT)
		{
			update_active_document_from_protected_child_process();
		}

		EnterCriticalSection(&Global.ActiveDocFileNameLock);

		memcpy(ActiveDoc,
			   Global.ActiveDocFileName,
			   min(sizeof(ActiveDoc) - sizeof(WCHAR), sizeof(Global.ActiveDocFileName) - sizeof(WCHAR)));

		LeaveCriticalSection(&Global.ActiveDocFileNameLock);

		if (wcslen(ActiveDoc) == 0 && Global.IsLaunchPrinting) {
			nRet = -1;
			SkipOrignal = TRUE;
			SetLastError(ERROR_ACCESS_DISABLED_BY_POLICY);

			send_block_notification(ActiveDoc, NxrmdrvShellPrintBlocked);
			break;
		}

		if (wcslen(ActiveDoc) == 0 && wcsstr(Global.AdobeCommandLine, ADOBE_IEMODE_CMD_LINE) == NULL)
		{
			nRet = -1;
			SkipOrignal = TRUE;
			SetLastError(ERROR_ACCESS_DISABLED_BY_POLICY);

			send_block_notification(ActiveDoc, NxrmdrvAdobeHookIsNotReady);

			break;
		}

	} while (FALSE);

	if (!SkipOrignal)
	{
		nRet = g_fn_StartDocW_trampoline(hdc, lpdi);
	}

	return nRet;
}

int WINAPI Core_StartDocW2(_In_ HDC hdc, _In_ CONST DOCINFOW *lpdi)
{
	int nRet = 0;

	BOOL SkipOrignal = FALSE;

	WCHAR ActiveDoc[MAX_PATH] = { 0 };

	const WCHAR *pName = NULL;

	do
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		if (allow_process_to_print())
		{
			break;
		}

		nRet = -1;
		SkipOrignal = TRUE;
		SetLastError(ERROR_ACCESS_DISABLED_BY_POLICY);

		pName = lpdi->lpszDocName ? lpdi->lpszDocName : L"";

		EnterCriticalSection(&Global.ActiveDocFileNameLock);

		memcpy(ActiveDoc,
			   pName,
			   min(sizeof(ActiveDoc) - sizeof(WCHAR), wcslen(pName) * sizeof(WCHAR)));

		LeaveCriticalSection(&Global.ActiveDocFileNameLock);

		send_block_notification(ActiveDoc, NXrmdrvPrintingBlocked);

	} while (FALSE);

	if (!SkipOrignal)
	{
		nRet = g_fn_StartDocW_trampoline(hdc, lpdi);
	}

	return nRet;
}

static BOOL allow_process_to_print(void)
{
	BOOL bRet = TRUE;

	CHECK_PROCESS_RIGHTS_REQUEST req = { 0 };
	CHECK_PROCESS_RIGHTS_RESPONSE resp = { 0 };

	NXCONTEXT Ctx = NULL;

	ULONG bytesret = 0;

	do 
	{
		req.ProcessId = GetCurrentProcessId();
		req.SessionId = Global.SessionId;

		Ctx = submit_request(Global.Section, NXRMDRV_MSG_TYPE_CHECK_PROCESS_RIGHTS, &req, sizeof(req));

		if (!Ctx)
		{
			break;
		}

		if (!wait_for_response(Ctx, Global.Section, (PVOID)&resp, sizeof(resp), &bytesret))
		{
			Ctx = NULL;

			break;
		}

		if (!(resp.Rights & BUILTIN_RIGHT_PRINT))
		{
			bRet = FALSE;
		}

	} while (FALSE);

	return bRet;
}
