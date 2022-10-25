#pragma once

#include <Wincodec.h>
#include <d2d1.h>
#define NXRMCORE_DLL_SPINCOUNT						1000
#define NXRMCORE_SECTION_SPINCOUNT					300
#define NXRMCORE_DOCWINDOWLIST_SPINCOUNT			500
#define NXRMCORE_CHILDWINDOWLIST_SPINCOUNT			500
#define NXRMCORE_DOCUMENTLIST_SPINCOUNT				500
#define NXRMCORE_WICCONVERTER_SPINCOUNT				500
#define NXRMCORE_PAGEVIEWCTXLIST_SPINCOUNT			500
#define NXRMCORE_ACRORDWNDLIST_SPINCOUNT			500
#define NXRMCORE_CREATESWAPCHAIN_SPINCOUNT			500
#define NXRMCORE_ACTIVEDOCFILENAME_SPINCOUNT		300
#define NXRMCORE_WATERMARKDOCLIST_SPINCOUNT			300
#define NXRMCORE_EMAILDOCFILENAME_SPINCOUNT			200
#define NXRMCORE_PRINTDOCFILENAME_SPINCOUNT			200
#define NXRMCORE_PRESENTDWM_SPINCOUNT				200
#define NXRMCORE_D3D11GLOBAL_SPINCOUNT				200
#define NXRMCORE_D3D10GLOBAL_SPINCOUNT				200

#define NXRMCORE_SELECTOBJECT_PROC_NAME				"SelectObject"
#define NXRMCORE_LOADIMAGEW_PROC_NAME				"LoadImageW"
#define NXRMCORE_DELETEDC_PROC_NAME					"DeleteDC"
#define NXRMCORE_DELETEOBJECT_PROC_NAME				"DeleteObject"
#define NXRMCORE_CALLWINDOWPROCW_PROC_NAME			"CallWindowProcW"
#define NXRMCORE_GETDC_PROC_NAME					"GetDC"
#define NXRMCORE_GETCLIENTRECT_PROC_NAME			"GetClientRect"
#define NXRMCORE_GETOBJECTW_PROC_NAME				"GetObjectW"
#define NXRMCORE_RELEASEDC_PROC_NAME				"ReleaseDC"
#define NXRMCORE_SETWINDOWDISPLAYAFFINITY_PROC_NAME	"SetWindowDisplayAffinity"
#define NXRMCORE_GETWINDOWDISPLAYAFFINITY_PROC_NAME	"GetWindowDisplayAffinity"
#define NXRMCORE_CREATECOMPATIBLEDC_PROC_NAME		"CreateCompatibleDC"
#define NXRMCORE_GETWINDOW_PROC_NAME				"GetWindow"
#define NXRMCORE_GETPARENT_PROC_NAME				"GetParent"
#define NXRMCORE_GETCLIENTRECT_PROC_NAME			"GetClientRect"
#define NXRMCORE_COTASKMEMFREE_PROC_NAME			"CoTaskMemFree"
#define NXRMCORE_COMMANDLINETOARGVW_PROC_NAME		"CommandLineToArgvW"
#define NXRMCORE_SHCHANGENOTIFY_PROC_NAME			"SHChangeNotify"
#define NXRMCORE_GETWINDOWMODULEFILENAMEW_PROC_NAME	"GetWindowModuleFileNameW"
#define NXRMCORE_GETGUITHREADINFO_PROC_NAME			"GetGUIThreadInfo"
#define NXRMCORE_D2D1CREATEFACTORY_PROC_NAME		"D2D1CreateFactory"
#define NXRMCORE_INVALIDATERECT_PROC_NAME			"InvalidateRect"
#define NXRMCORE_UPDATEWINDOW_PROC_NAME				"UpdateWindow"
#define NXRMCORE_DESTROYWINDOW_PROC_NAME			"DestroyWindow"
#define NXRMCORE_ISWINDOWVISIBLE_PROC_NAME			"IsWindowVisible"

typedef HGDIOBJ (WINAPI *SELECTOBJECT)(_In_ HDC hdc, _In_ HGDIOBJ h);

typedef HANDLE (WINAPI *LOADIMAGEW)(
	_In_opt_	HINSTANCE	hInst,
	_In_		LPCWSTR		name,
	_In_		UINT		type,
	_In_		int			cx,
	_In_		int			cy,
	_In_		UINT		fuLoad);

typedef BOOL (WINAPI *DELETEDC)( _In_ HDC hdc);

typedef BOOL (WINAPI *DELETEOBJECT)( _In_ HGDIOBJ ho);

typedef LRESULT (WINAPI *CALLWINDOWPROCW)(
	_In_ WNDPROC	lpPrevWndFunc,
	_In_ HWND		hWnd,
	_In_ UINT		Msg,
	_In_ WPARAM		wParam,
	_In_ LPARAM		lParam);

typedef HDC (WINAPI *GETDC)( _In_  HWND hWnd);

typedef BOOL (WINAPI *GETCLIENTRECT)(_In_ HWND hWnd, _Out_ LPRECT lpRect);

typedef int (WINAPI *GETOBJECTW)(_In_ HANDLE h, _In_ int c, _Out_writes_bytes_opt_(c) LPVOID pv);

typedef int (WINAPI *RELEASEDC)(_In_opt_ HWND hWnd, _In_ HDC hDC);

typedef BOOL (WINAPI *SETWINDOWDISPLAYAFFINITY)(_In_ HWND hWnd, _In_ DWORD dwAffinity);

typedef BOOL (WINAPI *GETWINDOWDISPLAYAFFINITY)(_In_ HWND hWnd,	_Out_ DWORD* pdwAffinity);

typedef HDC (WINAPI *CREATECOMPATIBLEDC)( _In_opt_ HDC hdc);

typedef HWND (WINAPI *GETWINDOW)(_In_ HWND hWnd, _In_ UINT uCmd);

typedef HWND (WINAPI *GETPARENT)(_In_ HWND hWnd);

typedef BOOL (WINAPI *GETCLIENTRECT)(_In_ HWND hWnd, _Out_ LPRECT lpRect);

typedef void (WINAPI *COTASKMEMFREE)(_In_opt_ __drv_freesMem(Mem) _Post_invalid_ LPVOID pv);

typedef LPWSTR* (WINAPI *COMMANDLINETOARGVW)(_In_ LPCWSTR lpCmdLine, _Out_ int* pNumArgs);

typedef void (WINAPI *SHCHANGENOTIFY)(LONG wEventId, UINT uFlags, _In_opt_ LPCVOID dwItem1, _In_opt_ LPCVOID dwItem2);

typedef UINT (WINAPI *GETWINDOWMODULEFILENAMEW)(_In_ HWND hwnd,	_Out_writes_to_(cchFileNameMax, return) LPWSTR pszFileName,	_In_ UINT cchFileNameMax);

typedef BOOL (WINAPI *GETGUITHREADINFO)(_In_ DWORD idThread, _Inout_ PGUITHREADINFO pgui);

typedef HRESULT (WINAPI	*D2D1CREATEFACTORY)(_In_ D2D1_FACTORY_TYPE factoryType, _In_ REFIID riid, _In_opt_ CONST D2D1_FACTORY_OPTIONS *pFactoryOptions, _Out_ void **ppIFactory);

typedef BOOL (WINAPI *INVALIDATERECT)(_In_opt_ HWND hWnd, _In_opt_ CONST RECT *lpRect, _In_ BOOL bErase);

typedef BOOL (WINAPI *UPDATEWINDOW)(_In_ HWND hWnd);

typedef BOOL (WINAPI *DESTROYWINDOW)(_In_ HWND hWnd);

typedef BOOL (WINAPI *ISWINDOWVISIBLE)(_In_ HWND hWnd);

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct _CORE_GLOBAL_DATA{

		BOOL				Initalized;

		PVOID				Section;
		
		CRITICAL_SECTION	SectionLock;

		BOOL				IsWow64Process;

		WCHAR				ProcessName[MAX_PATH];

		BOOL				IsWin10;

		BOOL				IsWin7;

		BOOL				IsWin8AndAbove;

		ULONG				SessionId;

		//////////////////////////////////////////////////////////////////////////
		//
		// Load module related
		//
		//////////////////////////////////////////////////////////////////////////

		#define KERNELBASE_MODULE_NAME	L"kernelbase.dll"

		//////////////////////////////////////////////////////////////////////////
		//
		// COM related
		//
		//////////////////////////////////////////////////////////////////////////

		HMODULE				Ole32Handle;

		CRITICAL_SECTION	Ole32Lock;

		#define OLE32_MODULE_NAME	L"ole32.dll"

		//////////////////////////////////////////////////////////////////////////
		//
		// Windows 8 COM related
		//
		//////////////////////////////////////////////////////////////////////////

		HMODULE				CombaseHandle;

		CRITICAL_SECTION	CombaseLock;
	
		#define COMBASE_MODULE_NAME	L"combase.dll"

		//////////////////////////////////////////////////////////////////////////
		//
		// Powerpoint related
		//
		//////////////////////////////////////////////////////////////////////////

		BOOL				IsPowerPoint;

		IDispatch			*PowerPointApp;

		#define POWERPOINT_MODULE_NAME	L"powerpnt.exe"

		//////////////////////////////////////////////////////////////////////////
		//
		// Winword related
		//
		//////////////////////////////////////////////////////////////////////////

		BOOL				IsWinWord;

		IDispatch			*WordApp;

		#define WINWORD_MODULE_NAME		L"winword.exe"

		//////////////////////////////////////////////////////////////////////////
		//
		// Excel related
		//
		//////////////////////////////////////////////////////////////////////////

		BOOL				IsExcel;

		IDispatch			*ExcelApp;

		ULONGLONG			ExcelModuleChecksum;

		ULONGLONG			Excel2013CopyWorkSheetOffset;

		#define EXCEL_MODULE_NAME		L"excel.exe"

		//////////////////////////////////////////////////////////////////////////
		//
		// Office related
		//
		//////////////////////////////////////////////////////////////////////////

		BOOL				IsMsOffice;

		BOOL				OfficeHooksInitialized;

		BOOL				DisablePrinting;

		BOOL				DisableClipboard;

		BOOL				DisableSendMail;

		BOOL				IsOffice2010;

		BOOL				IsOffice2007OrLower;

		WCHAR				*OfficeCommandLine;

		ULONG				OfficeMode;

		IWICImagingFactory  *WICFactory;

		HWND				ActiveWnd;

		int					Opacity;

		ULONG				TotalWaterMarkedDocuments;

		CRITICAL_SECTION	WaterMarkedDocumentListLock;

		LIST_ENTRY			WaterMarkedDocumentList;

		BOOL				Office2013DestroyWindowHooksInitialized;

		BOOL				IsOffice365AppV;

		HMODULE				MsoHandle;

		CRITICAL_SECTION	MsoLock;

		BOOL				OfficeMsoHookInitialized;

		BOOL				ExcelDDEHookInitialized;

		#define MSO_MODULE_NAME				L"Mso.dll"
		#define OLEAUT32_MODULE_NAME		L"OleAut32.dll"
		#define OFFICE365_APPV_MODULE_NAME	L"AppVIsvSubsystems32.dll"

		//////////////////////////////////////////////////////////////////////////
		//
		// Office, Adobe Reader and SAP 3D viewer shared
		//
		//////////////////////////////////////////////////////////////////////////

		CRITICAL_SECTION	ActiveDocFileNameLock;			// share with Adobe Reader and SAP 3D viewer

		WCHAR				ActiveDocFileName[MAX_PATH];	// share with Adobe Reader and SAP 3D viewer

		CRITICAL_SECTION	DDEDocFileNameLock;

		WCHAR				DDEDocFileName[MAX_PATH];

		//////////////////////////////////////////////////////////////////////////
		//
		// Adobe Reader related
		//
		//////////////////////////////////////////////////////////////////////////

		BOOL				IsAdobeReader;

		BOOL				AdobeHooksInitialized;

		BOOL				IsAdobeReaderXI;

		BOOL				IsLaunchPrinting;

		HMODULE				D2D1Handle;

		ID2D1Factory		*D2D1Factory;

		HMODULE				AnnotsHandle;

		CRITICAL_SECTION	AnnotsLock;

		CRITICAL_SECTION	PageViewCtxListLock;

		LIST_ENTRY			PageViewCtxList;

		WCHAR				*AdobeCommandLine;

		ULONG				AdobeMode;

		BOOL				AdobeSendMailHooksInitialized;

		CRITICAL_SECTION	EmailDocFileNameLock;

		WCHAR				EmailDocFileName[MAX_PATH];

		ULONGLONG			AcroRd32ModuleChecksum;

		HMODULE				AcroRd32Handle;

		CRITICAL_SECTION	AcroRd32Lock;

		BOOL				AcroWinMainSandboxHookInitialized;

		ULONG				AcroHFTInitialized;

		#define ADOBEREADER_MODULE_NAME		L"acrord32.exe"	
		#define ANNOTS_MODULE_NAME			L"annots.api"
		#define ACROFORM_MODULE_NAME		L"AcroForm.api"
		#define PPKLITE_MODULE_NAME			L"PPKLite.api"
		#define ACCESSIBILITY_MODULE_NAME	L"Accessibility.api"
		#define ACROFORM_MODULE_NAME		L"AcroForm.api"
		#define CHECKERS_MODULE_NAME		L"Checkers.api"
		#define DIGSIG_MODULE_NAME			L"DigSig.api"
		#define DVA_MODULE_NAME				L"DVA.api"
		#define EBOOK_MODULE_NAME			L"eBook.api"
		#define ESCRIPT_MODULE_NAME			L"EScript.api"
		#define IA32_MODULE_NAME			L"IA32.api"
		#define MAKEACCESSIBLE_MODULE_NAME	L"MakeAccessible.api"
		#define MULTIMEDIA_MODULE_NAME		L"Multimedia.api"
		#define PDDOM_MODULE_NAME			L"PDDom.api"
		#define READOUTLOUD_MODULE_NAME		L"ReadOutLoud.api"
		#define REFLOW_MODULE_NAME			L"reflow.api"
		#define SAVEASRTF_MODULE_NAME		L"SaveAsRTF.api"
		#define SEARCH_MODULE_NAME			L"Search.api"
		#define SENDMAIL_MODULE_NAME		L"SendMail.api"
		#define SPELLING_MODULE_NAME		L"Spelling.api"
		#define UPDATER_MODULE_NAME			L"Updater.api"
		#define WEBLINK_MODULE_NAME			L"weblink.api"

		#define KERNEL32_MODULE_NAME	L"kernel32.dll"

		#define ACRORD32_MODULE_NAME	L"acrord32.dll"
		//////////////////////////////////////////////////////////////////////////
		//
		// Screen Overlay related
		//
		//////////////////////////////////////////////////////////////////////////

		BOOL				ScreenOverlayFunctionsInitialized;

		CRITICAL_SECTION	user32Lock;
		
		HMODULE				user32Handle;

		CRITICAL_SECTION	gdi32Lock;

		HMODULE				gdi32Handle;

		BOOL				WndMsgHooksInitialized;

		#define GDI32_MODULE_NAME		L"gdi32.dll"
		#define USER32_MODULE_NAME		L"user32.dll"
		#define MSIMG32_MODULE_NAME		L"Msimg32.dll"
		#define COMCTL32_MODULE_NAME	L"Comctl32.dll"

		//////////////////////////////////////////////////////////////////////////
		//
		// Printing Control related
		//
		//////////////////////////////////////////////////////////////////////////

		BOOL				PrintingCtlHooksInitialized;

		//////////////////////////////////////////////////////////////////////////
		//
		// Windows 8 Metro App related
		//
		//////////////////////////////////////////////////////////////////////////

		BOOL				IsAppContainer;
		
		BOOL				IsRuntimeBroker;

		#define RUNTIMEBROKER_MODULE_NAME	L"runtimebroker.exe"

		//////////////////////////////////////////////////////////////////////////
		//
		// D2D1 related
		//
		//////////////////////////////////////////////////////////////////////////

		#define D2D1_MODULE_NAME	L"d2d1.dll"

		//////////////////////////////////////////////////////////////////////////
		//
		// DXGI related
		//
		//////////////////////////////////////////////////////////////////////////

		BOOL				DXGIHooksInitialized;

		CRITICAL_SECTION	dxgiLock;

		HMODULE				dxgiHandle;

		#define DXGI_MODULE_NAME	L"dxgi.dll"

		//////////////////////////////////////////////////////////////////////////
		//
		// DWM related
		//
		//////////////////////////////////////////////////////////////////////////

		BOOL				IsDWM;

		BOOL				DWMHooksInitialized;

		BOOL				DWMGlobalObjectInitialized;

		BOOL				WindowscodecsHookInitialized;

		HMODULE				D3DHandle;
		
		HMODULE				WindowscodecsHandle;

		CRITICAL_SECTION	WindowscodecsLock;

		CRITICAL_SECTION	CreateSwapChainLock;

		CRITICAL_SECTION	PresentDWMLock;

		CRITICAL_SECTION	D3D11GlobalLock;

		CRITICAL_SECTION	D3D10GlobalLock;

		#define DWM_MAX_WINDOWS				64

		CACHE_ALIGN ULONG	hWnds[DWM_MAX_WINDOWS];

		CACHE_ALIGN ULONG	WndsAttr[DWM_MAX_WINDOWS];

		CACHE_ALIGN ULONG	LastPolicySN;

		#define DWM_MODULE_NAME				L"dwm.exe"
		#define D3D10_1_MODULE_NAME			L"D3D10_1.dll"
		#define D3D11_MODULE_NAME			L"D3D11.dll"
		#define WINDOWSCODECS_MODULE_NAME	L"Windowscodecs.dll"

		//////////////////////////////////////////////////////////////////////////
		//
		// SaveAs control related
		//
		//////////////////////////////////////////////////////////////////////////

		CACHE_ALIGN BOOL	SaveAsCtlHooksInitialized;

		BOOL				LegacySaveAsCtlHooksInitialized;

		BOOL				CopyFileExWHookInitialized;

		HMODULE				Comdlg32Handle;

		CRITICAL_SECTION	Comdlg32Lock;

		#define COMDLG32_MODULE_NAME	L"Comdlg32.dll"

		//////////////////////////////////////////////////////////////////////////
		//
		// SAP 3D Visual Enterprise Viewer related
		//
		//////////////////////////////////////////////////////////////////////////

		BOOL				IsVEViewer;

		WCHAR				*VEViewerCommandLine;

		#define VEVIEWER_MODULE_NAME	L"veviewer.exe"
		#define SHELL32_MODULE_NAME		L"Shell32.dll"

		//////////////////////////////////////////////////////////////////////////
		//
		// explorer related
		//
		//////////////////////////////////////////////////////////////////////////

		BOOL				IsExplorer;

		#define EXPLORER_MODULE_NAME	L"explorer.exe"

		//////////////////////////////////////////////////////////////////////////
		//
		// Clipboard control related
		//
		//////////////////////////////////////////////////////////////////////////

		BOOL				ClipboardCtlHooksInitialized;

		//////////////////////////////////////////////////////////////////////////
		//
		// Office Word 2010 related
		//
		//////////////////////////////////////////////////////////////////////////

		#define WWWLIB_MODULE_NAME		L"wwlib.dll"

		//////////////////////////////////////////////////////////////////////////
		//
		// Dllhost.exe related
		//
		//////////////////////////////////////////////////////////////////////////

		BOOL				IsDllHost;
		
		#define DLLHOST_MODULE_NAME		L"dllhost.exe"

		//////////////////////////////////////////////////////////////////////////
		//
		// Office Sendmail related
		//
		//////////////////////////////////////////////////////////////////////////

		BOOL				OfficeSendMailHooksInitialized;

		HMODULE				Mapi32Handle;

		CRITICAL_SECTION	Mapi32Lock;

		BOOL				OfficePowerPointSendMailHooksInitialized;

		HMODULE				MSMAPI32Handle;

		CRITICAL_SECTION	MSMAPI32Lock;

		#define MAPI32_MODULE_NAME		L"Mapi32.dll"
		#define MSMAPI32_MODULE_NAME	L"MSMAPI32.DLL"

		//////////////////////////////////////////////////////////////////////////
		//
		// Drag and drop related
		//
		//////////////////////////////////////////////////////////////////////////

		BOOL				DragDropHooksInitialized;

		//////////////////////////////////////////////////////////////////////////
		//
		// browsers related
		//
		//////////////////////////////////////////////////////////////////////////

		BOOL				IsBrower;

		#define IE_MODULE_NAME		L"iexplore.exe"
		#define CHROME_MODULE_NAME	L"chrome.exe"
		#define FF_MODULE_NAME		L"firefox.exe"

		//////////////////////////////////////////////////////////////////////////
		//
		// OLE object embedding related
		//
		//////////////////////////////////////////////////////////////////////////

		BOOL				OLE2CtlHooksInitialized;

		//////////////////////////////////////////////////////////////////////////
		//
		// Explorer send mail related
		//
		//////////////////////////////////////////////////////////////////////////

		BOOL				ExplorerSendMailHooksInitialized;

		//////////////////////////////////////////////////////////////////////////
		//
		// Office drag & drop text related
		//
		//////////////////////////////////////////////////////////////////////////

		BOOL				OfficeDragDropTextHooksInitialized;

		//////////////////////////////////////////////////////////////////////////
		//
		// JT2Go related
		//
		//////////////////////////////////////////////////////////////////////////

		BOOL				IsJT2Go;

		#define JT2GO_MODULE_NAME		L"visview.exe"
		#define JT2GO_NG_MODULE_NAME	L"visview_ng.exe"

		//////////////////////////////////////////////////////////////////////////
		//
		// outlook drag & drop related
		//
		//////////////////////////////////////////////////////////////////////////

		#define OUTLOOK_MODULE_NAME        L"outlook.exe"

		BOOL                IsOutlook;

		BOOL                OutlookDragDropHooksInitialized;

	}CORE_GLOBAL_DATA,*PCORE_GLOBAL_DATA;

#ifdef __cplusplus
}
#endif
