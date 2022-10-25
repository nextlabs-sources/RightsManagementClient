// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "nxrmcoreglobal.h"
#include "nxrmdrv.h"
#include "nxrmcorehlp.h"
#include "office.h"
#include "screenoverlay.h"
#include "printingctl.h"
#include "loadmodule.h"
#include "adobe.h"
#include "saveasctl.h"
#include "clipboardctl.h"
#include "dragdropctl.h"
#include "direct3d.h"

void InitializeGlobalData(void);
void CleanupGlobalData(void);

static BOOL is_win7(void);
static BOOL is_win8andabove(void);
static BOOL is_win10(void);
static BOOL is_appcontainer(void);
static BOOL is_office2010(void);
static BOOL is_office2007orlower(void);
static BOOL is_adobe_xi(void);
static ULONGLONG find_excel_module_checksum(void);
static PIMAGE_NT_HEADERS RtlImageNtHeader (IN PVOID BaseAddress);

#ifdef __cplusplus
extern "C" {
#endif

	typedef LONG (WINAPI *GETCURRENTPACKAGEFULLNAME)(
		_Inout_										ULONG *packageFullNameLength,
		_Out_writes_opt_(*packageFullNameLength)	PWSTR packageFullName
		);

	typedef BOOL (WINAPI *GETTOKENINFORMATION)(
		_In_       HANDLE					TokenHandle,
		_In_       TOKEN_INFORMATION_CLASS	TokenInformationClass,
		_Out_opt_  LPVOID					TokenInformation,
		_In_       DWORD					TokenInformationLength,
		_Out_      PDWORD					ReturnLength
		);

	typedef BOOL (WINAPI *OPENPROCESSTOKEN)(
		_In_   HANDLE	ProcessHandle,
		_In_   DWORD	DesiredAccess,
		_Out_  PHANDLE	TokenHandle
		);

	ULONGLONG get_module_checksum(PVOID pBase);

	CORE_GLOBAL_DATA	Global;

#ifdef __cplusplus
}
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
					   DWORD  ul_reason_for_call,
					   LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		
		InitializeGlobalData();

		if (Global.IsMsOffice && (!Global.IsOffice2007OrLower))
		{
			InitializeOfficeHook();

			InitializePrintingCtlHook();

			InitializeScreenOverlayFunctions();

			InitializeLoadModuleHook();

			//InitializeOfficeSendMailHook();

			if (!Global.IsExcel)
				InitializeCopyFileExWHook();
			else
				InitializeExcelDDEHook();

			InitializeOfficeDragTextHook();

			if (Global.IsPowerPoint)
				takecare_powerpoint_create_in_memory_content_from_nxl_file();

			if (Global.IsWinWord)
				takecare_winword_create_in_memory_content_from_nxl_file();
		}

		if (Global.IsAdobeReader)
		{
			InitializeScreenOverlayFunctions();

			InitializeAdobeReaderHook();

			InitializePrintingCtlHook();

			InitializeLoadModuleHook();
		}

		InitializeSaveAsCtlHook();

		InitializeLegacySaveAsCtlHook();

		InitializeClipboardCtlHook();

		InitializeOLE2CtlHook();

		if (Global.IsExplorer)
		{
			InitializeExplorerRenameHook();

			InitializeLoadModuleHook();
		}

		if (Global.IsDllHost)
		{
			InitializeCopyFileExWHook();
		}

		if (Global.IsBrower)
		{
			InitializeLoadModuleHook();

			InitializeDragDropCtlHook();
		}

		if (Global.IsDWM)
		{
			if (Global.IsWin7)
			{
				InitializeDWMHook();
			}
			else if (Global.IsWin10)
			{
				InitializeLoadModuleHook();
			}
			else
			{
				//
				// don't do anything for Win8 and Win8.1
			}
		}

		if (Global.IsJT2Go)
		{
			InitializePrintingCtlHook();
		}

		if (Global.IsOutlook && (!Global.IsWin7))
		{
			InitializeOutlookDragDropHook();
		}

		// disable thread attach to improve
		// performance
		//
		DisableThreadLibraryCalls(hModule);
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:

		CleanupAdobeReaderHook();

		CleanupPrintingCtlHook();

		CleanupOfficeHook();

		CleanupLoadModuleHook();

		CleanupSaveAsCtlHook();

		CleanupLegacySaveAsCtlHook();

		CleanupClipboardCtlHook();

		CleanupExplorerRenameHook();

		CleanupCopyFileExWHook();

		CleanupOfficeSendMailHook();

		CleanupPowerpointSendMailHook();

		CleanupDragDropCtlHook();

		CleanupOLE2CtlHook();

		CleanupAdobeSendMailHooks();

		CleanupOfficeDragTextHook();

		CleanupExcelDDEHook();

		CleanupAdobeAcroWinMainSandboxHook();

		CleanupDWMHook();

		CleanupOutlookDragDropHook();

		CleanupGlobalData();

		break;
	}
	return TRUE;
}

void InitializeGlobalData(void)
{
	WCHAR	process_name[MAX_PATH];
	DWORD	dwRet = 0;

#pragma warning(disable: 6031)
	InitializeCriticalSectionAndSpinCount(&Global.SectionLock, NXRMCORE_SECTION_SPINCOUNT);
	InitializeCriticalSectionAndSpinCount(&Global.CombaseLock, NXRMCORE_DLL_SPINCOUNT);
	InitializeCriticalSectionAndSpinCount(&Global.Ole32Lock, NXRMCORE_DLL_SPINCOUNT);
	InitializeCriticalSectionAndSpinCount(&Global.user32Lock, NXRMCORE_DLL_SPINCOUNT);
	InitializeCriticalSectionAndSpinCount(&Global.dxgiLock, NXRMCORE_DLL_SPINCOUNT);
	InitializeCriticalSectionAndSpinCount(&Global.AnnotsLock, NXRMCORE_DLL_SPINCOUNT);
	InitializeCriticalSectionAndSpinCount(&Global.PageViewCtxListLock, NXRMCORE_PAGEVIEWCTXLIST_SPINCOUNT);
	InitializeCriticalSectionAndSpinCount(&Global.gdi32Lock, NXRMCORE_DLL_SPINCOUNT);
	InitializeCriticalSectionAndSpinCount(&Global.CreateSwapChainLock, NXRMCORE_CREATESWAPCHAIN_SPINCOUNT);
	InitializeCriticalSectionAndSpinCount(&Global.Comdlg32Lock, NXRMCORE_DLL_SPINCOUNT);
	InitializeCriticalSectionAndSpinCount(&Global.ActiveDocFileNameLock, NXRMCORE_ACTIVEDOCFILENAME_SPINCOUNT);
	InitializeCriticalSectionAndSpinCount(&Global.Mapi32Lock, NXRMCORE_DLL_SPINCOUNT);
	InitializeCriticalSectionAndSpinCount(&Global.MSMAPI32Lock, NXRMCORE_DLL_SPINCOUNT);
	InitializeCriticalSectionAndSpinCount(&Global.DDEDocFileNameLock, NXRMCORE_ACTIVEDOCFILENAME_SPINCOUNT);
	InitializeCriticalSectionAndSpinCount(&Global.WaterMarkedDocumentListLock, NXRMCORE_WATERMARKDOCLIST_SPINCOUNT);
	InitializeCriticalSectionAndSpinCount(&Global.EmailDocFileNameLock, NXRMCORE_EMAILDOCFILENAME_SPINCOUNT);
	InitializeCriticalSectionAndSpinCount(&Global.MsoLock, NXRMCORE_DLL_SPINCOUNT);
	InitializeCriticalSectionAndSpinCount(&Global.AcroRd32Lock, NXRMCORE_DLL_SPINCOUNT);
	InitializeCriticalSectionAndSpinCount(&Global.WindowscodecsLock, NXRMCORE_DLL_SPINCOUNT);
	InitializeCriticalSectionAndSpinCount(&Global.PresentDWMLock, NXRMCORE_PRESENTDWM_SPINCOUNT);
	InitializeCriticalSectionAndSpinCount(&Global.D3D11GlobalLock, NXRMCORE_D3D11GLOBAL_SPINCOUNT);
	InitializeCriticalSectionAndSpinCount(&Global.D3D10GlobalLock, NXRMCORE_D3D10GLOBAL_SPINCOUNT);

	InitializeListHead(&Global.PageViewCtxList);
	InitializeListHead(&Global.WaterMarkedDocumentList);

	Global.Initalized		= FALSE;
	Global.Section			= NULL;
	Global.IsWow64Process	= FALSE;
	Global.CombaseHandle	= NULL;

	ProcessIdToSessionId(GetCurrentProcessId(), &Global.SessionId);
	
	IsWow64Process(GetCurrentProcess(), &Global.IsWow64Process);

	memset(Global.ProcessName, 0, sizeof(Global.ProcessName));

	if (GetModuleFileNameW(NULL,process_name,MAX_PATH))
	{
		dwRet = GetLongPathNameW(process_name,
								 Global.ProcessName,
								 sizeof(Global.ProcessName)/sizeof(WCHAR));
		if(!dwRet)
		{
			memcpy(Global.ProcessName,
				   process_name,
				   min(sizeof(Global.ProcessName)-sizeof(WCHAR),sizeof(process_name)-sizeof(WCHAR)));
		}
		else if(dwRet > sizeof(Global.ProcessName)/sizeof(WCHAR))
		{
			memcpy(Global.ProcessName,
				   process_name,
				   min(sizeof(Global.ProcessName)-sizeof(WCHAR),sizeof(process_name)-sizeof(WCHAR)));
		}
		else
		{

		}
	}

	_wcslwr_s(Global.ProcessName, sizeof(Global.ProcessName)/sizeof(WCHAR));

	Global.IsWin10			= is_win10();
	Global.IsWin8AndAbove	= is_win8andabove();
	Global.IsWin7			= is_win7();
	Global.Ole32Handle		= NULL;
	Global.CombaseHandle	= NULL;

	Global.ActiveWnd = NULL;
	Global.Opacity = 10;

	if (wcsstr(Global.ProcessName, POWERPOINT_MODULE_NAME))
	{
		Global.IsPowerPoint = TRUE;
		Global.IsMsOffice	= TRUE;

		Global.IsOffice2010 = is_office2010();
		Global.IsOffice2007OrLower = is_office2007orlower();
	}
	else
	{
		Global.IsPowerPoint = FALSE;
	}

	if (wcsstr(Global.ProcessName, WINWORD_MODULE_NAME))
	{
		Global.IsWinWord	= TRUE;
		Global.IsMsOffice	= TRUE;
		Global.IsOffice2010 = is_office2010();
		Global.IsOffice2007OrLower = is_office2007orlower();
	}
	else
	{
		Global.IsWinWord	= FALSE;
	}

	if (wcsstr(Global.ProcessName, EXCEL_MODULE_NAME))
	{
		Global.IsExcel		= TRUE;
		Global.IsMsOffice	= TRUE;
		Global.IsOffice2010 = is_office2010();
		Global.IsOffice2007OrLower = is_office2007orlower();
		Global.ExcelModuleChecksum = find_excel_module_checksum();
		Global.Excel2013CopyWorkSheetOffset = 0;
	}
	else
	{
		Global.IsExcel		= FALSE;
	}

	if (Global.IsMsOffice)
	{
		Global.OfficeCommandLine = GetCommandLineW();

		if (wcsstr(Global.OfficeCommandLine, L"/Embedding") ||
			wcsstr(Global.OfficeCommandLine, L"-Embedding"))
		{
			Global.OfficeMode = OFFICE_MODE_COMPLUS;
		}
		else if (Global.IsExcel && wcsstr(Global.OfficeCommandLine, L"/dde /n"))
		{
			//
			// Excel is NOT in COMPLUS mode but we set it anyway here
			// to force StgOpenStorage to update ActiveDocument. We are going to
			// change mode to DDE mode later when connecting Addon
			//
			Global.OfficeMode = OFFICE_MODE_COMPLUS;
		}
		else
		{
			Global.OfficeMode = OFFICE_MODE_NORMAL;
		}

		if (GetModuleHandleW(OFFICE365_APPV_MODULE_NAME))
		{
			Global.IsOffice365AppV = TRUE;
		}
		else
		{
			Global.IsOffice365AppV = FALSE;
		}

		Global.MsoHandle = NULL;
		Global.OfficeMsoHookInitialized = FALSE;
	}

	if (wcsstr(Global.ProcessName, ADOBEREADER_MODULE_NAME))
	{
		Global.IsAdobeReader = TRUE;
		Global.D2D1Handle = NULL;
		Global.D2D1Factory = NULL;
		Global.IsAdobeReaderXI = is_adobe_xi();

		Global.AdobeCommandLine = GetCommandLineW();

		if (wcsstr(Global.AdobeCommandLine, ADOBE_PROTECTEDCHILD_CMD_LINE))
		{
			Global.AdobeMode = ADOBE_MODE_PROTECTEDCHILD;
		}
		else
		{
			Global.AdobeMode = ADOBE_MODE_PARENT;
		}

		if (wcsstr(Global.AdobeCommandLine, ADOBE_PRINT_CMD_LINE)) {
			Global.IsLaunchPrinting = TRUE;
		}
		else {
			Global.IsLaunchPrinting = FALSE;
		}

		Global.AcroRd32Handle = NULL;
		Global.AcroRd32ModuleChecksum = 0ULL;
		Global.AcroWinMainSandboxHookInitialized = FALSE;
		Global.AcroHFTInitialized = 0;
	}
	else
	{
		Global.IsAdobeReader = FALSE;
	}

	Global.AdobeSendMailHooksInitialized = FALSE;

	memset(Global.EmailDocFileName, 0, sizeof(Global.EmailDocFileName));

	if (wcsstr(Global.ProcessName, DLLHOST_MODULE_NAME))
	{
		Global.IsDllHost = TRUE;
	}
	else
	{
		Global.IsDllHost = FALSE;
	}

	Global.AdobeHooksInitialized = FALSE;
	Global.AnnotsHandle = NULL;
	
	Global.ScreenOverlayFunctionsInitialized = FALSE;
	Global.user32Handle = NULL;
	Global.gdi32Handle = NULL;

	Global.WICFactory	= NULL;

	Global.IsAppContainer	= is_appcontainer();
	Global.IsRuntimeBroker	= FALSE;

	Global.DXGIHooksInitialized = FALSE;
	Global.dxgiHandle			= NULL;

	Global.LegacySaveAsCtlHooksInitialized	= FALSE;
	Global.Comdlg32Handle					= NULL;
	Global.CopyFileExWHookInitialized		= FALSE;

	if (wcsstr(Global.ProcessName, VEVIEWER_MODULE_NAME))
	{
		Global.IsVEViewer = TRUE;
		Global.VEViewerCommandLine = GetCommandLineW();
	}
	else
	{
		Global.IsVEViewer = FALSE;
	}

	memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));
	memset(Global.DDEDocFileName, 0, sizeof(Global.DDEDocFileName));

	if (wcsstr(Global.ProcessName, EXPLORER_MODULE_NAME))
	{
		Global.IsExplorer = TRUE;
	}
	else
	{
		Global.IsExplorer = FALSE;
	}

	Global.ClipboardCtlHooksInitialized = FALSE;
	
	Global.Mapi32Handle = NULL;
	Global.OfficeSendMailHooksInitialized = FALSE;
	Global.MSMAPI32Handle = NULL;
	Global.OfficePowerPointSendMailHooksInitialized = FALSE;

	Global.DragDropHooksInitialized = FALSE;

	if (wcsstr(Global.ProcessName, IE_MODULE_NAME) ||
		wcsstr(Global.ProcessName, CHROME_MODULE_NAME) ||
		wcsstr(Global.ProcessName, FF_MODULE_NAME))
	{
		Global.IsBrower = TRUE;
	}

	Global.OLE2CtlHooksInitialized = FALSE;

	Global.ExplorerSendMailHooksInitialized = FALSE;

	Global.OfficeDragDropTextHooksInitialized = FALSE;

	if (wcsstr(Global.ProcessName, DWM_MODULE_NAME))
	{
		Global.IsDWM = TRUE;

		Global.DWMHooksInitialized = FALSE;
		Global.D3DHandle = NULL;
		Global.WindowscodecsHandle = NULL;
	}
	else
	{
		Global.IsDWM = FALSE;
	}

	if (wcsstr(Global.ProcessName, JT2GO_MODULE_NAME) ||
		wcsstr(Global.ProcessName, JT2GO_NG_MODULE_NAME))
	{
		Global.IsJT2Go = TRUE;
	}
	else
	{
		Global.IsJT2Go = FALSE;
	}

	if (wcsstr(Global.ProcessName, OUTLOOK_MODULE_NAME))
	{
		Global.IsOutlook = TRUE;
	}
	else
	{
		Global.IsOutlook = FALSE;
	}

	Global.OutlookDragDropHooksInitialized = FALSE;

}

void CleanupGlobalData(void)
{
	LIST_ENTRY *ite = NULL;
	LIST_ENTRY *tmp = NULL;

	if(Global.Section)
	{
		close_transporter_client(Global.Section);
		Global.Section = NULL;
	}

	FOR_EACH_LIST_SAFE(ite, tmp, &Global.PageViewCtxList)
	{
		PAGEVIEWCTX *pNode = CONTAINING_RECORD(ite, PAGEVIEWCTX, Link);

		RemoveEntryList(ite);

		free(pNode);
	}

	FOR_EACH_LIST_SAFE(ite, tmp, &Global.WaterMarkedDocumentList)
	{
		OFFICE_WATERMARKED_DOCUMENT_NODE *pNode = CONTAINING_RECORD(ite, OFFICE_WATERMARKED_DOCUMENT_NODE, Link);

		RemoveEntryList(ite);

		free(pNode);
	}

	DeleteCriticalSection(&Global.Ole32Lock);
	DeleteCriticalSection(&Global.SectionLock);
	DeleteCriticalSection(&Global.CombaseLock);
	DeleteCriticalSection(&Global.user32Lock);
	DeleteCriticalSection(&Global.dxgiLock);
	DeleteCriticalSection(&Global.AnnotsLock);
	DeleteCriticalSection(&Global.PageViewCtxListLock);
	DeleteCriticalSection(&Global.gdi32Lock);
	DeleteCriticalSection(&Global.CreateSwapChainLock);
	DeleteCriticalSection(&Global.Comdlg32Lock);
	DeleteCriticalSection(&Global.ActiveDocFileNameLock);
	DeleteCriticalSection(&Global.Mapi32Lock);
	DeleteCriticalSection(&Global.MSMAPI32Lock);
	DeleteCriticalSection(&Global.DDEDocFileNameLock);
	DeleteCriticalSection(&Global.WaterMarkedDocumentListLock);
	DeleteCriticalSection(&Global.EmailDocFileNameLock);
	DeleteCriticalSection(&Global.MsoLock);
	DeleteCriticalSection(&Global.AcroRd32Lock);
	DeleteCriticalSection(&Global.WindowscodecsLock);
	DeleteCriticalSection(&Global.PresentDWMLock);
	DeleteCriticalSection(&Global.D3D11GlobalLock);
	DeleteCriticalSection(&Global.D3D10GlobalLock);
}

static BOOL is_win7(void)
{
	OSVERSIONINFOEXW osvi;
	DWORDLONG dwlConditionMask = 0;
	int op1 = VER_EQUAL;

	// Initialize the OSVERSIONINFOEX structure.
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXW));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
	osvi.dwMajorVersion = 6;
	osvi.dwMinorVersion = 1;
	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op1);
	VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, op1);

	return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask);
}

static BOOL is_win8andabove(void)
{
	OSVERSIONINFOEXW osvi;
	DWORDLONG dwlConditionMask = 0;

	int op1 = VER_GREATER_EQUAL;

	// Initialize the OSVERSIONINFOEX structure.
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXW));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
	osvi.dwMajorVersion = 6;
	osvi.dwMinorVersion = 2;
	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op1);
	VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, op1);

	return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask);
}

static BOOL is_win10(void)
{
	OSVERSIONINFOEXW osvi;
	DWORDLONG dwlConditionMask = 0;

	int op1 = VER_EQUAL;

	// Initialize the OSVERSIONINFOEX structure.
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXW));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
	osvi.dwMajorVersion = 10;
	osvi.dwMinorVersion = 0;

	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op1);

	return VerifyVersionInfoW(&osvi, VER_MAJORVERSION, dwlConditionMask);
}

static BOOL is_appcontainer(void)
{
	BOOL bRet = FALSE;

	HMODULE hKernelBase = NULL;
	HANDLE	hToken = NULL;
	DWORD	dwIsAppContainer = 0;
	DWORD	dwLen = 0;

	GETTOKENINFORMATION	fn_GetTokenInformation = NULL;
	OPENPROCESSTOKEN	fn_OpenProcessToken = NULL;

	do 
	{
		hKernelBase = GetModuleHandleW(L"KernelBase.Dll");

		if(!hKernelBase)
		{
			break;
		}

		fn_OpenProcessToken		= (OPENPROCESSTOKEN)GetProcAddress(hKernelBase,"OpenProcessToken");
		fn_GetTokenInformation	= (GETTOKENINFORMATION)GetProcAddress(hKernelBase,"GetTokenInformation");

		if(fn_GetTokenInformation == NULL || 
		   fn_OpenProcessToken == NULL)
		{
			break;
		}

		fn_OpenProcessToken((HANDLE)(-1),TOKEN_QUERY,&hToken);

		if(!hToken)
		{
			break;
		}

		if(!fn_GetTokenInformation(hToken,
								   TokenIsAppContainer,
								   &dwIsAppContainer,
								   sizeof(dwIsAppContainer),
								   &dwLen))
		{
			break;
		}

		bRet = (BOOL)dwIsAppContainer;

	} while (FALSE);

	if(hToken)
	{
		CloseHandle(hToken);
		hToken = NULL;
	}

	return bRet;
}

static BOOL is_office2010(void)
{
	BOOL bRet = FALSE;

	IMAGE_NT_HEADERS		*pNtHdr = NULL;
	IMAGE_OPTIONAL_HEADER	*pOptHdr = NULL;

	HMODULE		hOfficeApp = NULL;

	do 
	{
		if (!Global.IsMsOffice)
		{
			break;
		}

		hOfficeApp = GetModuleHandleW(NULL);

		if(!hOfficeApp)
		{
			break;
		}

		pNtHdr = (IMAGE_NT_HEADERS *)RtlImageNtHeader(hOfficeApp);

		if(!pNtHdr)
		{
			break;
		}

		pOptHdr = &pNtHdr->OptionalHeader;

		if(!pOptHdr)
		{
			break;
		}

		if(pOptHdr->MajorLinkerVersion == 9 && pOptHdr->MinorLinkerVersion == 0)
		{
			bRet = TRUE;
			break;
		}

	} while (FALSE);

	return bRet;
}

static BOOL is_office2007orlower(void)
{
	BOOL bRet = FALSE;

	IMAGE_NT_HEADERS		*pNtHdr = NULL;
	IMAGE_OPTIONAL_HEADER	*pOptHdr = NULL;

	HMODULE		hOfficeApp = NULL;

	do 
	{
		if (!Global.IsMsOffice)
		{
			break;
		}

		hOfficeApp = GetModuleHandleW(NULL);

		if(!hOfficeApp)
		{
			break;
		}

		pNtHdr = (IMAGE_NT_HEADERS *)RtlImageNtHeader(hOfficeApp);

		if(!pNtHdr)
		{
			break;
		}

		pOptHdr = &pNtHdr->OptionalHeader;

		if(!pOptHdr)
		{
			break;
		}

		if(pOptHdr->MajorLinkerVersion <= 8)
		{
			bRet = TRUE;
			break;
		}

	} while (FALSE);

	return bRet;
}

static PIMAGE_NT_HEADERS RtlImageNtHeader (IN PVOID BaseAddress)
{
	PIMAGE_NT_HEADERS NtHeader;
	PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)BaseAddress;

	if(DosHeader && DosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return NULL;
	}

	if(DosHeader && DosHeader->e_magic == IMAGE_DOS_SIGNATURE)
	{
		NtHeader = (PIMAGE_NT_HEADERS)((ULONG_PTR)BaseAddress + DosHeader->e_lfanew);

		if(NtHeader->Signature == IMAGE_NT_SIGNATURE)
			return NtHeader;
	}

	return NULL;
}

static BOOL is_adobe_xi(void)
{
	BOOL bRet = FALSE;

	IMAGE_NT_HEADERS		*pNtHdr = NULL;
	IMAGE_OPTIONAL_HEADER	*pOptHdr = NULL;

	HMODULE		hAdobeApp = NULL;

	do 
	{
		if (!Global.IsAdobeReader)
		{
			break;
		}

		hAdobeApp = GetModuleHandleW(NULL);

		if(!hAdobeApp)
		{
			break;
		}

		pNtHdr = (IMAGE_NT_HEADERS *)RtlImageNtHeader(hAdobeApp);

		if(!pNtHdr)
		{
			break;
		}

		pOptHdr = &pNtHdr->OptionalHeader;

		if(!pOptHdr)
		{
			break;
		}

		if(pOptHdr->MajorLinkerVersion == 10 && pOptHdr->MinorLinkerVersion == 0)
		{
			bRet = TRUE;
			break;
		}

	} while (FALSE);

	return bRet;
}

static ULONGLONG find_excel_module_checksum(void)
{
	ULONGLONG Checksum = 0ULL;

	IMAGE_NT_HEADERS		*pNtHdr = NULL;
	IMAGE_OPTIONAL_HEADER	*pOptHdr = NULL;

	HMODULE		hExcelApp = NULL;

	do
	{
		if (!Global.IsExcel) 
		{
			break;
		}

		hExcelApp = GetModuleHandleW(NULL);

		if (!hExcelApp)
		{
			break;
		}

		pNtHdr = (IMAGE_NT_HEADERS *)RtlImageNtHeader(hExcelApp);

		if (!pNtHdr)
		{
			break;
		}

		pOptHdr = &pNtHdr->OptionalHeader;

		if (!pOptHdr)
		{
			break;
		}

		Checksum = *(ULONGLONG*)(&(pOptHdr->CheckSum));

	} while (FALSE);

	return Checksum;
}

ULONGLONG get_module_checksum(PVOID pBase)
{
	ULONGLONG Checksum = 0ULL;

	IMAGE_NT_HEADERS		*pNtHdr = NULL;
	IMAGE_OPTIONAL_HEADER	*pOptHdr = NULL;

	do
	{
		pNtHdr = (IMAGE_NT_HEADERS *)RtlImageNtHeader(pBase);

		if (!pNtHdr)
		{
			break;
		}

		pOptHdr = &pNtHdr->OptionalHeader;

		if (!pOptHdr)
		{
			break;
		}

		Checksum = *(ULONGLONG*)(&(pOptHdr->CheckSum));

	} while (FALSE);

	return Checksum;
}