#include "stdafx.h"
#include "nxrmcoreglobal.h"
#include "nxrmdrv.h"
#include "nxrmcorehlp.h"
#include "detour.h"
#include "loadmodule.h"
#include "office.h"
#include "screenoverlay.h"
#include "printingctl.h"
#include "direct2d.h"
#include "direct3d.h"
#include "adobe.h"
#include "saveasctl.h"
#include "clipboardctl.h"
#include "dragdropctl.h"
#include "emailctl.h"

#ifdef __cplusplus
extern "C" {
#endif

	extern						CORE_GLOBAL_DATA Global;

	LOADLIBRARYEXW				g_fnorg_LoadLibraryExW = NULL;

	LOADLIBRARYEXW				g_fn_LoadLibraryExW_trampoline = NULL;

	LOADIMAGEW					g_fnorg_LoadLibraryW = NULL;

	LOADIMAGEW					g_fn_LoadLibraryW_trampoline = NULL;

	FREELIBRARY					g_fnorg_FreeLibrary = NULL;

	FREELIBRARY					g_fn_FreeLibrary_trampoline = NULL;

#ifdef __cplusplus
}
#endif

BOOL InitializeLoadModuleHook(void)
{
	BOOL bRet = TRUE;

	HMODULE	hKernelBase = NULL;

	do 
	{
		hKernelBase = GetModuleHandleW(KERNELBASE_MODULE_NAME);

		if (!hKernelBase)
		{
			//
			// NO WAY, kernelbase is there as long as we are still a Win32 subsystem
			//
			bRet = FALSE;
			break;
		}

		g_fnorg_LoadLibraryExW = (LOADLIBRARYEXW)GetProcAddress(hKernelBase, LOADMODULE_LOADLIBRARYEXW_PROC_NAME);

		if (!g_fnorg_LoadLibraryExW)
		{
			bRet = FALSE;
			break;
		}

		if (!install_hook(g_fnorg_LoadLibraryExW, (PVOID*)&g_fn_LoadLibraryExW_trampoline, Core_LoadLibraryExW))
		{
			bRet = FALSE;
			break;
		}

		//if (Global.IsPowerPoint)
		//{
		//	g_fnorg_FreeLibrary = (FREELIBRARY)GetProcAddress(hKernelBase, LOADMODULE_FREELIBRARY_PROC_NAME);

		//	if (g_fnorg_FreeLibrary)
		//	{
		//		install_hook(g_fnorg_FreeLibrary, (PVOID*)&g_fn_FreeLibrary_trampoline, Core_FreeLibrary);
		//	}
		//}

	} while (FALSE);

	return bRet;
}

void CleanupLoadModuleHook(void)
{
	if (g_fn_LoadLibraryExW_trampoline)
	{
		remove_hook(g_fn_LoadLibraryExW_trampoline);
		g_fn_LoadLibraryExW_trampoline = NULL;
	}

	if (g_fn_FreeLibrary_trampoline)
	{
		remove_hook(g_fn_FreeLibrary_trampoline);
		g_fn_FreeLibrary_trampoline = NULL;
	}
}

HANDLE WINAPI Core_LoadLibraryExW(LPCWSTR lpFileName, HANDLE hFile, DWORD dwFlags)
{
	HMODULE	hLib = NULL;

	hLib = g_fn_LoadLibraryExW_trampoline(lpFileName, hFile, dwFlags);

	do 
	{
		if (dwFlags & LOAD_LIBRARY_AS_DATAFILE ||
		    dwFlags & DONT_RESOLVE_DLL_REFERENCES ||
		    dwFlags & LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE ||
		    dwFlags & LOAD_LIBRARY_AS_IMAGE_RESOURCE ||
		    hLib == NULL)
		{
			break;
		}

		if (Global.IsMsOffice)
		{
			if (!Global.Ole32Handle)
			{
				if (GetModuleHandleW(OLE32_MODULE_NAME))
				{
					EnterCriticalSection(&Global.Ole32Lock);

					if (!Global.Ole32Handle)
					{
						InitializeOfficeHook();

						InitializeScreenOverlayFunctions();

						InitializePrintingCtlHook();

						InitializeOLE2CtlHook();

						InitializeOfficeDragTextHook();
					}

					LeaveCriticalSection(&Global.Ole32Lock);
				}
			}

			if (!Global.ClipboardCtlHooksInitialized)
			{
				if (GetModuleHandleW(USER32_MODULE_NAME))
				{
					EnterCriticalSection(&Global.user32Lock);

					if (!Global.ClipboardCtlHooksInitialized)
					{
						InitializeClipboardCtlHook();
					}

					LeaveCriticalSection(&Global.user32Lock);
				}
			}

			//if (!Global.Mapi32Handle)
			//{
			//	if (GetModuleHandleW(MAPI32_MODULE_NAME))
			//	{
			//		EnterCriticalSection(&Global.Mapi32Lock);

			//		if (!Global.Mapi32Handle)
			//		{
			//			InitializeOfficeSendMailHook();
			//		}

			//		LeaveCriticalSection(&Global.Mapi32Lock);
			//	}
			//}

			//if (Global.IsPowerPoint)
			//{
			//	if (!Global.MSMAPI32Handle)
			//	{
			//		if (GetModuleHandleW(MSMAPI32_MODULE_NAME))
			//		{
			//			EnterCriticalSection(&Global.MSMAPI32Lock);

			//			if (!Global.MSMAPI32Handle)
			//			{
			//				InitializePowerpointSendMailHook();
			//			}

			//			LeaveCriticalSection(&Global.MSMAPI32Lock);
			//		}
			//	}
			//}

			break;
		}
		
		if (Global.IsAdobeReader)
		{
			if (!Global.gdi32Handle)
			{
				if (GetModuleHandleW(GDI32_MODULE_NAME))
				{
					EnterCriticalSection(&Global.gdi32Lock);

					if (!Global.gdi32Handle)
					{
						InitializePrintingCtlHook();
					}

					LeaveCriticalSection(&Global.gdi32Lock);
				}
			}

			if (!Global.AcroWinMainSandboxHookInitialized)
			{
				WCHAR module_name[MAX_PATH] = { 0 };

				if (lpFileName)
				{
					memcpy(module_name,
						   lpFileName,
						   min(sizeof(module_name) - sizeof(WCHAR), wcslen(lpFileName)*sizeof(WCHAR)));

					_wcslwr_s(module_name, sizeof(module_name) / sizeof(WCHAR));

					if (wcsstr(module_name, ACRORD32_MODULE_NAME))
					{
						Global.AcroRd32Handle = hLib;

						InitializeAdobeAcroWinMainSandboxHook();
					}
				}
			}

			if (!Global.AdobeHooksInitialized)
			{
				InitializeAdobeReaderHook();
			}

			if (!Global.AdobeSendMailHooksInitialized)
			{
				if (Global.IsAdobeReaderXI)
				{
					WCHAR module_name[MAX_PATH] = {0};

					if (lpFileName)
					{
						memcpy(module_name, 
							   lpFileName,
							   min(sizeof(module_name) - sizeof(WCHAR), wcslen(lpFileName)*sizeof(WCHAR)));

						_wcslwr_s(module_name, sizeof(module_name)/sizeof(WCHAR));

						if (wcsstr(module_name, L"sendmail.api"))
						{
							InitializeAdobeSendMailHooks();
						}
					}
				}
				else
				{
					InitializeAdobeSendMailHooks();
				}
			}

			if (!Global.ScreenOverlayFunctionsInitialized)
			{
				InitializeScreenOverlayFunctions();
			}
		}

		if (Global.IsBrower)
		{
			if (!Global.DragDropHooksInitialized)
			{
				if (GetModuleHandleW(OLE32_MODULE_NAME))
				{
					EnterCriticalSection(&Global.Ole32Lock);

					if (!Global.DragDropHooksInitialized)
					{
						InitializeDragDropCtlHook();
					}

					LeaveCriticalSection(&Global.Ole32Lock);
				}

			}
		}

		if (!Global.Comdlg32Handle)
		{
			if (GetModuleHandleW(COMDLG32_MODULE_NAME))
			{
				EnterCriticalSection(&Global.Comdlg32Lock);

				if (!Global.Comdlg32Handle)
				{
					InitializeLegacySaveAsCtlHook();
				}

				LeaveCriticalSection(&Global.Comdlg32Lock);
			}
		}

		if (!Global.IsWin8AndAbove)
		{
			if (!Global.Ole32Handle)
			{
				if (GetModuleHandleW(OLE32_MODULE_NAME))
				{
					EnterCriticalSection(&Global.Ole32Lock);

					if (!Global.Ole32Handle)
					{
						InitializeSaveAsCtlHook();
					}

					LeaveCriticalSection(&Global.Ole32Lock);
				}
			}
		}
		else
		{
			if (!Global.CombaseHandle)
			{
				if (GetModuleHandleW(COMBASE_MODULE_NAME))
				{
					EnterCriticalSection(&Global.CombaseLock);

					if (!Global.CombaseHandle)
					{
						InitializeSaveAsCtlHook();
					}

					LeaveCriticalSection(&Global.CombaseLock);
				}
			}
		}

		if (!Global.user32Handle)
		{
			if (GetModuleHandleW(USER32_MODULE_NAME))
			{
				EnterCriticalSection(&Global.user32Lock);

				if (!Global.user32Handle)
				{
					InitializeClipboardCtlHook();
				}

				LeaveCriticalSection(&Global.user32Lock);
			}
		}

		if (!Global.OLE2CtlHooksInitialized)
		{
			if (GetModuleHandleW(OLE32_MODULE_NAME))
			{
				EnterCriticalSection(&Global.Ole32Lock);

				if (!Global.OLE2CtlHooksInitialized)
				{
					InitializeOLE2CtlHook();
				}

				LeaveCriticalSection(&Global.Ole32Lock);
			}
		}

		if (Global.IsExplorer && (!Global.ExplorerSendMailHooksInitialized))
		{
			if (GetModuleHandleW(MAPI32_MODULE_NAME))
			{
				EnterCriticalSection(&Global.Mapi32Lock);

				if (!Global.ExplorerSendMailHooksInitialized)
				{
					InitializeExplorerSendEmailHook();
				}

				LeaveCriticalSection(&Global.Mapi32Lock);
			}
		}

		if (Global.IsDWM && (!Global.DWMHooksInitialized))
		{
			if (GetModuleHandleW(DXGI_MODULE_NAME))
			{
				EnterCriticalSection(&Global.dxgiLock);

				if (!Global.DWMHooksInitialized)
				{
					InitializeDWMHook();
				}

				LeaveCriticalSection(&Global.dxgiLock);
			}
		}

		if (Global.IsDWM && (!Global.WindowscodecsHookInitialized))
		{
			if (GetModuleHandleW(WINDOWSCODECS_MODULE_NAME))
			{
				EnterCriticalSection(&Global.WindowscodecsLock);

				if (!Global.WindowscodecsHookInitialized)
				{
					InitializedWindowsCodesHook();
				}

				LeaveCriticalSection(&Global.WindowscodecsLock);
			}
		}

	} while (FALSE);

	return hLib;
}

//BOOL WINAPI Core_FreeLibrary(HMODULE hModule)
//{
//	do 
//	{
//		if (hModule == Global.MSMAPI32Handle && hModule != NULL)
//		{
//			EnterCriticalSection(&Global.MSMAPI32Lock);
//
//			if (hModule == Global.MSMAPI32Handle)
//			{
//				CleanupPowerpointSendMailHook();
//			}
//
//			LeaveCriticalSection(&Global.MSMAPI32Lock);
//		}
//
//	} while (FALSE);
//
//	return g_fn_FreeLibrary_trampoline(hModule);
//}