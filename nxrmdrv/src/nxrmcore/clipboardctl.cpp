#include "stdafx.h"
#include "nxrmcoreglobal.h"
#include "nxrmdrv.h"
#include "nxrmcorehlp.h"
#include "detour.h"
#include "clipboardctl.h"
#include "adobe.h"

#ifdef __cplusplus
extern "C" {
#endif

	extern CORE_GLOBAL_DATA Global;

	extern  BOOL update_active_document_from_protected_child_process(void);

#ifdef __cplusplus
}
#endif

static OPENCLIPBOARD	g_fnorg_OpenClipboard = NULL;
static OPENCLIPBOARD	g_fn_OpenClipboard_trampoline = NULL;

BOOL InitializeClipboardCtlHook(void)
{
	BOOL bRet = TRUE;

	do 
	{
		if (!Global.user32Handle)
		{
			Global.user32Handle = GetModuleHandleW(USER32_MODULE_NAME);
		}

		if (!Global.user32Handle)
		{
			bRet = FALSE;
			break;
		}

		if (Global.ClipboardCtlHooksInitialized)
		{
			break;
		}

		g_fnorg_OpenClipboard = (OPENCLIPBOARD)GetProcAddress(Global.user32Handle, CLIPBOARDCTL_OPENCLIPBOARD_PROC_NAME);

		if (g_fnorg_OpenClipboard)
		{
			if (!install_hook(g_fnorg_OpenClipboard, (PVOID*)&g_fn_OpenClipboard_trampoline, Core_OpenClipboard))
			{
				bRet = FALSE;
				break;
			}
		}

		Global.ClipboardCtlHooksInitialized = TRUE;

	} while (FALSE);

	return bRet;

}


void CleanupClipboardCtlHook(void)
{
	if (g_fn_OpenClipboard_trampoline)
	{
		remove_hook(g_fn_OpenClipboard_trampoline);
		g_fn_OpenClipboard_trampoline = NULL;
	}
}

BOOL WINAPI Core_OpenClipboard(_In_opt_ HWND hWndNewOwner)
{
	BOOL bRet = TRUE;

	BOOL SkipOrignal = FALSE;

	BOOL AdobeHookIsReady = FALSE;

	do 
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		if (Global.DisableClipboard)
		{
			bRet = FALSE;
			SkipOrignal = TRUE;
			SetLastError(ERROR_ACCESS_DISABLED_BY_POLICY);
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

		AdobeHookIsReady = wcslen(Global.ActiveDocFileName) == 0 ? FALSE : TRUE;

		LeaveCriticalSection(&Global.ActiveDocFileNameLock);

		if (!AdobeHookIsReady && wcsstr(Global.AdobeCommandLine, ADOBE_IEMODE_CMD_LINE) == NULL)
		{
			bRet = FALSE;
			SkipOrignal = TRUE;
			SetLastError(ERROR_ACCESS_DISABLED_BY_POLICY);
			break;
		}

	} while (FALSE);

	if (!SkipOrignal)
	{
		bRet = g_fn_OpenClipboard_trampoline(hWndNewOwner);
	}

	return bRet;
}

