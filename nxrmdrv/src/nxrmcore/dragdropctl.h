#pragma once

#include <Ole2.h>
#include <shellapi.h>

#define DRAGDROPCTL_REGISTERDRAGDROP_PROC_NAME	"RegisterDragDrop"
#define DRAGDROPCTL_DRAGQUERYFILEW_PROC_NAME	"DragQueryFileW"
#define DRAGDROPCTL_RELEASESTGMEDIUM_PROC_NAME	"ReleaseStgMedium"
#define DRAGDROPCTL_GETCLASSFILE_PROC_NAME		"GetClassFile"
#define DRAGDROPCTL_DODRAGDROP_PROC_NAME		"DoDragDrop"
#define DRAGDROPCTL_FINDFIRSTFILEW_PROC_NAME    "FindFirstFileW"

typedef HRESULT (WINAPI *REGISTERDRAGDROP)(IN HWND hwnd, IN LPDROPTARGET pDropTarget);

HRESULT WINAPI Core_RegisterDragDrop(IN HWND hwnd, IN LPDROPTARGET pDropTarget);

typedef UINT (WINAPI *DRAGQUERYFILEW)(_In_ HDROP hDrop, _In_ UINT iFile, _Out_writes_opt_(cch) LPWSTR lpszFile, _In_ UINT cch);

typedef void (WINAPI *RELEASESTGMEDIUM)(IN LPSTGMEDIUM);

typedef HRESULT (WINAPI *GETCLASSFILE)(
	_In_ LPCOLESTR szFilename, 
	_Out_ CLSID FAR* pclsid);

typedef HANDLE(WINAPI *FINDFIRSTFILEW)(
	_In_ LPCWSTR lpFileName,
	_Out_ LPWIN32_FIND_DATAW lpFindFileData
	);

HANDLE WINAPI Core_FindFirstFileW(
	_In_ LPCWSTR lpFileName,
	_Out_ LPWIN32_FIND_DATAW lpFindFileData
);

HRESULT WINAPI Core_GetClassFile(
	_In_ LPCOLESTR szFilename, 
	_Out_ CLSID FAR* pclsid);

typedef HRESULT (WINAPI *DODRAGDROP)(IN LPDATAOBJECT pDataObj, IN LPDROPSOURCE pDropSource, IN DWORD dwOKEffects, OUT LPDWORD pdwEffect);

HRESULT WINAPI Core_DoDragDrop(
	IN LPDATAOBJECT		pDataObj, 
	IN LPDROPSOURCE		pDropSource,
	IN DWORD			dwOKEffects, 
	OUT LPDWORD			pdwEffect);

BOOL InitializeDragDropCtlHook(void);
void CleanupDragDropCtlHook(void);

BOOL InitializeOLE2CtlHook(void);
void CleanupOLE2CtlHook(void);

BOOL InitializeOfficeDragTextHook(void);
void CleanupOfficeDragTextHook(void);

BOOL InitializeOutlookDragDropHook(void);
void CleanupOutlookDragDropHook(void);

class CoreIDropTarget : public IDropTarget
{
public:
	CoreIDropTarget();
	CoreIDropTarget(IDropTarget *pTarget);
	~CoreIDropTarget();

	STDMETHODIMP QueryInterface( 
		/* [in] */ __RPC__in REFIID riid,
		/* [annotation][iid_is][out] */ 
		_COM_Outptr_  void **ppvObject);

	STDMETHODIMP_(ULONG) AddRef();

	STDMETHODIMP_(ULONG) Release();

	STDMETHODIMP DragEnter( 
		/* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD *pdwEffect);

	STDMETHODIMP DragOver( 
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD *pdwEffect);

	STDMETHODIMP DragLeave();

	STDMETHODIMP Drop( 
		/* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD *pdwEffect);

private:

	ULONG			m_uRefCount;
	IDropTarget		*m_pIDropTarget;

	DRAGQUERYFILEW		m_fn_DragQueryFileW;
	RELEASESTGMEDIUM	m_fn_ReleaseStgMedium;
};