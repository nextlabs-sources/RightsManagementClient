#include "stdafx.h"
#include "nxrmcoreglobal.h"
#include "nxrmdrv.h"
#include "nxrmcorehlp.h"
#include "detour.h"
#include "screenoverlay.h"

#ifdef __cplusplus
extern "C" {
#endif

	extern						CORE_GLOBAL_DATA Global;

	GETDC						g_fnorg_GetDC = NULL;

	RELEASEDC					g_fnorg_ReleaseDC = NULL;

	GETCLIENTRECT				g_fnorg_GetClientRect = NULL;

	INVALIDATERECT				g_fnorg_InvalidateRect = NULL;

	UPDATEWINDOW				g_fnorg_UpdateWindow = NULL;

	ISWINDOWVISIBLE				g_fnorg_IsWindowVisible = NULL;

#ifdef __cplusplus
}
#endif

static WNDMSG_DISPATCHMESSAGEW	g_fnorg_DispatchMessageW = NULL;
static WNDMSG_DISPATCHMESSAGEW	g_fn_DispatchMessageW_trampoline = NULL;

static WNDMSG_DISPATCHMESSAGEA	g_fnorg_DispatchMessageA = NULL;
static WNDMSG_DISPATCHMESSAGEA	g_fn_DispatchMessageA_trampoline = NULL;

BOOL InitializeScreenOverlayFunctions(void)
{
	BOOL bRet = TRUE;

	do 
	{
		if (Global.user32Handle == NULL)
		{
			Global.user32Handle = GetModuleHandleW(USER32_MODULE_NAME);
		}

		if (Global.user32Handle == NULL)
		{
			bRet = FALSE;
			break;
		}

		if (Global.gdi32Handle == NULL)
		{
			Global.gdi32Handle = GetModuleHandleW(GDI32_MODULE_NAME);
		}

		if (Global.gdi32Handle == NULL)
		{
			bRet = FALSE;
			break;
		}

		if (Global.ScreenOverlayFunctionsInitialized)
		{
			break;
		}

		g_fnorg_GetDC = (GETDC)GetProcAddress(Global.user32Handle, NXRMCORE_GETDC_PROC_NAME);

		if (!g_fnorg_GetDC)
		{
			bRet = FALSE;
			break;
		}

		g_fnorg_ReleaseDC = (RELEASEDC)GetProcAddress(Global.user32Handle, NXRMCORE_RELEASEDC_PROC_NAME);

		if (!g_fnorg_ReleaseDC)
		{
			bRet = FALSE;
			break;
		}

		g_fnorg_GetClientRect = (GETCLIENTRECT)GetProcAddress(Global.user32Handle, NXRMCORE_GETCLIENTRECT_PROC_NAME);

		if (!g_fnorg_GetClientRect)
		{
			bRet = FALSE;
			break;
		}

		g_fnorg_InvalidateRect = (INVALIDATERECT)GetProcAddress(Global.user32Handle, NXRMCORE_INVALIDATERECT_PROC_NAME);

		if (!g_fnorg_InvalidateRect)
		{
			bRet = FALSE;
			break;
		}

		g_fnorg_UpdateWindow = (UPDATEWINDOW)GetProcAddress(Global.user32Handle, NXRMCORE_UPDATEWINDOW_PROC_NAME);

		if (!g_fnorg_UpdateWindow)
		{
			bRet = FALSE;
			break;
		}

		g_fnorg_IsWindowVisible = (ISWINDOWVISIBLE)GetProcAddress(Global.user32Handle, NXRMCORE_ISWINDOWVISIBLE_PROC_NAME);

		if (!g_fnorg_IsWindowVisible)
		{
			bRet = FALSE;
			break;
		}

		Global.ScreenOverlayFunctionsInitialized = TRUE;

	} while(FALSE);

	return bRet;
}

BOOL InitializeWndMsgHooks(void)
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

		if (Global.WndMsgHooksInitialized)
		{
			break;
		}

		g_fnorg_DispatchMessageW = (WNDMSG_DISPATCHMESSAGEW)GetProcAddress(Global.user32Handle, WNDMSG_DISPATCHMESSAGEW_PROC_NAME);

		if (g_fnorg_DispatchMessageW)
		{
#ifdef _AMD64_

			BYTE *p = (BYTE*)g_fnorg_DispatchMessageW;
			LONG off_set = 0;

			if (*p != 0x33 ||
				*(p + 1) != 0xd2 ||
				*(p + 2) != 0xe9)
			{
				bRet = FALSE;
				break;
			}

			p += 2;

			off_set = *(LONG*)(p + 1);

			g_fnorg_DispatchMessageW = (WNDMSG_DISPATCHMESSAGEW)((p + 5) + off_set);

#endif
			if (!install_hook(g_fnorg_DispatchMessageW, (PVOID*)&g_fn_DispatchMessageW_trampoline, Core_DispatchMessageW))
			{
				bRet = FALSE;
				break;
			}
		}

#ifndef _AMD64_

		g_fnorg_DispatchMessageA = (WNDMSG_DISPATCHMESSAGEA)GetProcAddress(Global.user32Handle, WNDMSG_DISPATCHMESSAGEA_PROC_NAME);

		if (g_fnorg_DispatchMessageA)
		{
			if (!install_hook(g_fnorg_DispatchMessageA, (PVOID*)&g_fn_DispatchMessageA_trampoline, Core_DispatchMessageA))
			{
				bRet = FALSE;
				break;
			}
		}

#endif

		Global.WndMsgHooksInitialized = TRUE;

	} while (FALSE);

	return bRet;
}

void CleanupWndMsgHooks(void)
{
	if (g_fn_DispatchMessageA_trampoline)
	{
		remove_hook(g_fn_DispatchMessageA_trampoline);
		g_fn_DispatchMessageA_trampoline = NULL;
	}

	if (g_fn_DispatchMessageW_trampoline)
	{
		remove_hook(g_fn_DispatchMessageW_trampoline);
		g_fn_DispatchMessageW_trampoline = NULL;
	}
}

LRESULT	WINAPI Core_DispatchMessageW(__in CONST MSG *lpMsg)
{
	return g_fn_DispatchMessageW_trampoline(lpMsg);
}

LRESULT	WINAPI Core_DispatchMessageA(__in CONST MSG *lpMsg)
{
	return g_fn_DispatchMessageA_trampoline(lpMsg);
}
