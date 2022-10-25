#pragma once

#include <MAPI.h>

#define OFFICE_COCREATEINSTANCE_PROC_NAME			"CoCreateInstance"
#define OFFICE_COCREATEINSTANCEEX_PROC_NAME			"CoCreateInstanceEx"
#define OFFICE_ENDPAINT_PROC_NAME					"EndPaint"
#define OFFICE_SYSFREESTRING_PROC_NAME				"SysFreeString"
#define OFFICE_SYSALLOCSTRING_PROC_NAME				"SysAllocString"
#define OFFICE_MAPISENDMAIL_PROC_NAME				"MAPISendMail"
#define OFFICE_MAPISENDMAILW_PROC_NAME				"MAPISendMailW"
#define OFFICE_STGOPENSTORAGE_PROC_NAME				"StgOpenStorage"
#define OFFICE_GETFILESIZEEX_PROC_NAME				"GetFileSizeEx"
#define OFFICE_GETFILEATTRIBUTESW_PROC_NAME			"GetFileAttributesW" 
#define OFFICE_CREATEWINDOWEXW_PROC_NAME			"CreateWindowExW"
#define OFFICE_UNPACKDDELPARAM_PROC_NAME			"UnpackDDElParam"

#define OFFICE_IDTEXTENSIBILITY2_IID	\
	{ 0xb65ad801, 0xabaf, 0x11d0, { 0xbb, 0x8b, 0x00, 0xa0, 0xc9, 0x0f, 0x27, 0x44} }

#define OFFICE_MODE_NORMAL					0x00000000
#define OFFICE_MODE_DDE						0x00000001
#define OFFICE_MODE_COMPLUS					0x00000002

#define PPT_SHARE_WND_CLASSNAME				L"NUIDialog"

#define OFFICE_EXCEL_DDE_PATENT_BEGIN			L"[new(\""
#define OFFICE_EXCEL_DDE_PATENT_END				L"\")]"
#define OFFICE_EXCEL_DDE_EMPTY_NEW				L"[new(\"\")]"

//typedef struct _OFFICE_CHILDWINDOWNODE{
//
//	HWND			hParentWnd;
//
//	HWND			hWnd;
//
//	HBITMAP			hOverlayBitmap;
//	
//	HDC				hMemDC;
//
//	LIST_ENTRY		Link;
//
//}OFFICE_CHILDWINDOWNODE,*POFFICE_CHILDWINDOWNODE;
//
//typedef struct _OFFICE_DOCUMENTNODE{
//
//	WCHAR						FileName[NXRM_MAX_PATH_LEN];
//
//	ULONGLONG					RightsMask;
//
//	LIST_ENTRY					Link;
//
//}OFFICE_DOCUMENTNODE,*POFFICE_DOCUMENTNODE;

typedef struct _OFFICE_WATERMARKED_DOCUMENT_NODE {

	HWND					hWnd;

	LIST_ENTRY				Link;

}OFFICE_WATERMARKED_DOCUMENT_NODE, *POFFICE_WATERMARKED_DOCUMENT_NODE;

BOOL InitializeOfficeHook(void);
void CleanupOfficeHook(void);

interface CoreIDTExtensibility2;
interface IDTExtensibility2;


typedef struct IDTExtensibility2Vtbl
{
	BEGIN_INTERFACE

		HRESULT (STDMETHODCALLTYPE *QueryInterface)(IDTExtensibility2 * This, REFIID riid, void **ppvObject);

		ULONG (STDMETHODCALLTYPE *AddRef)(IDTExtensibility2 * This);

		ULONG (STDMETHODCALLTYPE *Release)(IDTExtensibility2 * This);

		HRESULT (STDMETHODCALLTYPE *GetTypeInfoCount)(IDTExtensibility2 * This, UINT *pctinfo);

		HRESULT (STDMETHODCALLTYPE *GetTypeInfo)(IDTExtensibility2 * This, UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo);

		HRESULT (STDMETHODCALLTYPE *GetIDsOfNames)(IDTExtensibility2 * This, REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId);

		HRESULT (STDMETHODCALLTYPE *Invoke)(IDTExtensibility2 * This, DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);

		HRESULT (STDMETHODCALLTYPE *OnConnection)(IDTExtensibility2 * This, IDispatch * Application, enum ext_ConnectMode ConnectMode, IDispatch * AddInInst, SAFEARRAY **custom);

		HRESULT (STDMETHODCALLTYPE *OnDisconnection)(IDTExtensibility2 * This, enum ext_DisconnectMode RemoveMode, SAFEARRAY **custom);

		HRESULT (STDMETHODCALLTYPE *OnAddInsUpdate)(IDTExtensibility2 * This, SAFEARRAY **custom);

		HRESULT (STDMETHODCALLTYPE *OnStartupComplete)(IDTExtensibility2 * This, SAFEARRAY **custom);

		HRESULT (STDMETHODCALLTYPE *OnBeginShutdown)(IDTExtensibility2 * This, SAFEARRAY **custom);

	END_INTERFACE

}IDTExtensibility2Vtbl;

interface IDTExtensibility2
{
	const struct IDTExtensibility2Vtbl	*lpVtbl;
};

interface CoreIDTExtensibility2
{
	const struct IDTExtensibility2Vtbl	*lpVtbl;
	ULONG								m_uRefCount;
	IDTExtensibility2					*m_pIDTExtensibility2;
};

STDMETHODIMP CoreIDTExtensibility2_QueryInterface(IDTExtensibility2 * This, REFIID riid, void **ppvObject);

STDMETHODIMP_(ULONG) CoreIDTExtensibility2_AddRef(IDTExtensibility2 * This);

STDMETHODIMP_(ULONG) CoreIDTExtensibility2_Release(IDTExtensibility2 * This);

STDMETHODIMP CoreIDTExtensibility2_GetTypeInfoCount(IDTExtensibility2 * This, UINT *pctinfo);

STDMETHODIMP CoreIDTExtensibility2_GetTypeInfo(IDTExtensibility2 * This, UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo);

STDMETHODIMP CoreIDTExtensibility2_GetIDsOfNames(IDTExtensibility2 * This, REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId);

STDMETHODIMP CoreIDTExtensibility2_Invoke(IDTExtensibility2 * This, DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);

STDMETHODIMP CoreIDTExtensibility2_OnConnection(IDTExtensibility2 * This, IDispatch * Application, enum ext_ConnectMode ConnectMode, IDispatch * AddInInst, SAFEARRAY **custom);

STDMETHODIMP CoreIDTExtensibility2_OnDisconnection(IDTExtensibility2 * This, enum ext_DisconnectMode RemoveMode, SAFEARRAY **custom);

STDMETHODIMP CoreIDTExtensibility2_OnAddInsUpdate(IDTExtensibility2 * This, SAFEARRAY **custom);

STDMETHODIMP CoreIDTExtensibility2_OnStartupComplete(IDTExtensibility2 * This, SAFEARRAY **custom);

STDMETHODIMP CoreIDTExtensibility2_OnBeginShutdown(IDTExtensibility2 * This, SAFEARRAY **custom);

typedef HRESULT (WINAPI *OFFICE_COCREATEINSTANCEEX)(
	_In_ REFCLSID Clsid,
	_In_opt_ IUnknown * punkOuter,
	_In_ DWORD dwClsCtx,
	_In_opt_ COSERVERINFO * pServerInfo,
	_In_ DWORD dwCount,
	_Inout_updates_(dwCount) MULTI_QI * pResults
	);

HRESULT WINAPI Core_Office_CoCreateInstanceEx(
	_In_ REFCLSID Clsid,
	_In_opt_ IUnknown * punkOuter,
	_In_ DWORD dwClsCtx,
	_In_opt_ COSERVERINFO * pServerInfo,
	_In_ DWORD dwCount,
	_Inout_updates_(dwCount) MULTI_QI * pResults
	);

typedef HRESULT (WINAPI *OFFICE_COCREATEINSTANCE)(
	IN REFCLSID		rclsid, 
	IN LPUNKNOWN	pUnkOuter,
	IN DWORD		dwClsContext, 
	IN REFIID		riid, 
	OUT LPVOID FAR* ppv);

HRESULT WINAPI Core_Office_CoCreateInstance(
	IN REFCLSID		rclsid, 
	IN LPUNKNOWN	pUnkOuter,
	IN DWORD		dwClsContext, 
	IN REFIID		riid, 
	OUT LPVOID FAR* ppv);

typedef BOOL (WINAPI *ENDPAINT)(
	_In_ HWND hWnd,
	_In_ CONST PAINTSTRUCT *lpPaint);

BOOL WINAPI Core_EndPaint(
	_In_ HWND hWnd,
	_In_ CONST PAINTSTRUCT *lpPaint);

typedef void (WINAPI *SYSFREESTRING)(_In_opt_ BSTR bstrString);

typedef BSTR (WINAPI *SYSALLOCSTRING)(_In_opt_z_ const OLECHAR * psz);

ULONG WINAPI Core_MAPISendMail(
	LHANDLE lhSession,
	ULONG_PTR ulUIParam,
	lpMapiMessage lpMessage,
	FLAGS flFlags,
	ULONG ulReserved
	);

ULONG WINAPI Core_MAPISendMailW(
	LHANDLE lhSession,
	ULONG_PTR ulUIParam,
	_In_ lpMapiMessageW lpMessage,
	FLAGS flFlags,
	ULONG ulReserved
	);

BOOL WINAPI Core_DestroyWindow(
	_In_ HWND hWnd
	);

BOOL InitializeOfficeSendMailHook(void);
void CleanupOfficeSendMailHook(void);

BOOL InitializePowerpointSendMailHook(void);
void CleanupPowerpointSendMailHook(void);

BOOL InitializeOffice2013DestroyWindowHook(void);
void CleanupOffice2013DestroyWindowHook(void);

typedef HRESULT (WINAPI *OFFICE_STGOPENSTORAGE)(
	_In_opt_ _Null_terminated_ const WCHAR* pwcsName,
	_In_opt_ IStorage* pstgPriority,
	_In_ DWORD grfMode,
	_In_opt_z_ SNB snbExclude,
	_In_ DWORD reserved,
	_Outptr_ IStorage** ppstgOpen);

HRESULT WINAPI Core_StgOpenStorage(
	_In_opt_ _Null_terminated_ const WCHAR* pwcsName,
	_In_opt_ IStorage* pstgPriority,
	_In_ DWORD grfMode,
	_In_opt_z_ SNB snbExclude,
	_In_ DWORD reserved,
	_Outptr_ IStorage** ppstgOpen);

typedef BOOL (WINAPI *OFFICE_GETFILESIZEEX)(
	_In_ HANDLE hFile,
	_Out_ PLARGE_INTEGER lpFileSize
	);

BOOL WINAPI	Core_GetFileSizeEx(
	_In_ HANDLE hFile,
	_Out_ PLARGE_INTEGER lpFileSize
	);

typedef DWORD (WINAPI *OFFICE_GETFILEATTRIBUTESW)(
	_In_ LPCWSTR lpFileName
	);

DWORD WINAPI Core_GetFileAttributesW(
	_In_ LPCWSTR lpFileName
	);

typedef HWND (WINAPI *OFFICE_CREATEWINDOWEXW)(
	_In_ DWORD dwExStyle,
	_In_opt_ LPCWSTR lpClassName,
	_In_opt_ LPCWSTR lpWindowName,
	_In_ DWORD dwStyle,
	_In_ int X,
	_In_ int Y,
	_In_ int nWidth,
	_In_ int nHeight,
	_In_opt_ HWND hWndParent,
	_In_opt_ HMENU hMenu,
	_In_opt_ HINSTANCE hInstance,
	_In_opt_ LPVOID lpParam);

HWND WINAPI Core_CreateWindowExW(
	_In_ DWORD dwExStyle,
	_In_opt_ LPCWSTR lpClassName,
	_In_opt_ LPCWSTR lpWindowName,
	_In_ DWORD dwStyle,
	_In_ int X,
	_In_ int Y,
	_In_ int nWidth,
	_In_ int nHeight,
	_In_opt_ HWND hWndParent,
	_In_opt_ HMENU hMenu,
	_In_opt_ HINSTANCE hInstance,
	_In_opt_ LPVOID lpParam);

typedef BOOL (WINAPI *OFFICE_UNPACKDDELPARAM)(
	_In_  UINT      msg,
	_In_  LPARAM    lParam,
	_Out_ PUINT_PTR puiLo,
	_Out_ PUINT_PTR puiHi
	);

BOOL WINAPI Core_UnpackDDElParam(
	_In_  UINT      msg,
	_In_  LPARAM    lParam,
	_Out_ PUINT_PTR puiLo,
	_Out_ PUINT_PTR puiHi
	);

BOOL InitializeExcelDDEHook(void);
void CleanupExcelDDEHook(void);

typedef ULONG(WINAPI *EXCEL2013_COPYWORKSHEET)(PVOID P1, PVOID P2, const WCHAR *WorkSheetName, PVOID P4, PVOID P5, PVOID P6, PVOID P7);

ULONG WINAPI Core_Excel2013_CopyWorkSheet(PVOID P1, PVOID P2, const WCHAR *WorkSheetName, PVOID P4, PVOID P5, PVOID P6, PVOID P7);

void takecare_powerpoint_create_in_memory_content_from_nxl_file(void);
void takecare_winword_create_in_memory_content_from_nxl_file(void);
