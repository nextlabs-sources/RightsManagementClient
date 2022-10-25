#include "stdafx.h"
#include "nxrmcoreglobal.h"
#include "nxrmdrv.h"
#include "nxrmcorehlp.h"
#include "detour.h"
#include "dragdropctl.h"
#include <Shobjidl.h>
#include <Shellapi.h>
#include <Shlobj.h>

#define DRAGDROP_NXL_EXT	L".nxl"

#ifdef __cplusplus
extern "C" {
#endif

	extern	CORE_GLOBAL_DATA Global;

	extern 	void print_guid(const GUID &id);

	extern	BOOL send_block_notification(const WCHAR *FileName, BLOCK_NOTIFICATION_TYPE Type);

	BOOL IsValidNXLFile(const WCHAR *FileName);

#ifdef __cplusplus
}
#endif

static REGISTERDRAGDROP	g_fnorg_RegisterDragDrop = NULL;
static REGISTERDRAGDROP	g_fn_RegisterDragDrop_trampoline = NULL;

static GETCLASSFILE		g_fnorg_GetClassFile = NULL;
static GETCLASSFILE		g_fn_GetClassFile_trampoline = NULL;

static DODRAGDROP		g_fnorg_DoDragDrop = NULL;
static DODRAGDROP		g_fn_DoDragDrop_trampoline = NULL;

static FINDFIRSTFILEW    g_fnorg_FindFirstFileW = NULL;
static FINDFIRSTFILEW    g_fn_FindFirstFileW_trampoline = NULL;

BOOL InitializeDragDropCtlHook(void)
{
	BOOL bRet = TRUE;

	do 
	{
		if (!Global.Ole32Handle)
		{
			Global.Ole32Handle = GetModuleHandleW(OLE32_MODULE_NAME);
		}

		if (!Global.Ole32Handle)
		{
			bRet = FALSE;
			break;
		}

		if (Global.DragDropHooksInitialized)
		{
			break;
		}

		g_fnorg_RegisterDragDrop = (REGISTERDRAGDROP)GetProcAddress(Global.Ole32Handle, DRAGDROPCTL_REGISTERDRAGDROP_PROC_NAME);

		if (g_fnorg_RegisterDragDrop)
		{
			if (!install_hook(g_fnorg_RegisterDragDrop, (PVOID*)&g_fn_RegisterDragDrop_trampoline, Core_RegisterDragDrop))
			{
				bRet = FALSE;
				break;
			}
		}

		Global.DragDropHooksInitialized = TRUE;

	} while (FALSE);

	return bRet;
}

void CleanupDragDropCtlHook(void)
{
	if (g_fn_RegisterDragDrop_trampoline)
	{
		remove_hook(g_fn_RegisterDragDrop_trampoline);
		g_fn_RegisterDragDrop_trampoline = NULL;
	}
}

HRESULT WINAPI Core_RegisterDragDrop(IN HWND hwnd, IN LPDROPTARGET pDropTarget)
{
	HRESULT hr = S_OK;

	CoreIDropTarget *pCoreIDropTarget = NULL;

	IDropTarget *pMyIDropTarget = pDropTarget;

	do 
	{
		if (!pDropTarget)
		{
			hr = g_fn_RegisterDragDrop_trampoline(hwnd, pDropTarget);
			break;
		}

		try
		{
			pCoreIDropTarget = new CoreIDropTarget(pDropTarget);

			pMyIDropTarget = (IDropTarget *)pCoreIDropTarget;

		}
		catch(std::bad_alloc e)
		{
			pMyIDropTarget = pDropTarget;
		}

		hr = g_fn_RegisterDragDrop_trampoline(hwnd, pMyIDropTarget);

	} while (FALSE);

	return hr;
}

CoreIDropTarget::CoreIDropTarget()
{
	HMODULE hShell32 = NULL;

	m_uRefCount = 0;
	m_pIDropTarget = NULL;
	
	//
	// ole32 depends on Shell32. hShell32 should not be NULL
	//
	hShell32 = GetModuleHandleW(SHELL32_MODULE_NAME);

	if (hShell32)
		m_fn_DragQueryFileW = (DRAGQUERYFILEW)GetProcAddress(hShell32, DRAGDROPCTL_DRAGQUERYFILEW_PROC_NAME);

	if (Global.Ole32Handle)
		m_fn_ReleaseStgMedium = (RELEASESTGMEDIUM)GetProcAddress(Global.Ole32Handle, DRAGDROPCTL_RELEASESTGMEDIUM_PROC_NAME);
}

CoreIDropTarget::CoreIDropTarget(IDropTarget *pTarget)
{
	HMODULE hShell32 = NULL;

	m_uRefCount = 0;
	pTarget->AddRef();
	m_pIDropTarget = pTarget;

	//
	// ole32 depends on Shell32. hShell32 should not be NULL
	//
	hShell32 = GetModuleHandleW(SHELL32_MODULE_NAME);

	if (hShell32)
		m_fn_DragQueryFileW = (DRAGQUERYFILEW)GetProcAddress(hShell32, DRAGDROPCTL_DRAGQUERYFILEW_PROC_NAME);

	if (Global.Ole32Handle)
		m_fn_ReleaseStgMedium = (RELEASESTGMEDIUM)GetProcAddress(Global.Ole32Handle, DRAGDROPCTL_RELEASESTGMEDIUM_PROC_NAME);
}

CoreIDropTarget::~CoreIDropTarget()
{
	if (m_pIDropTarget)
	{
		m_pIDropTarget->Release();
		m_pIDropTarget = NULL;
	}

	m_fn_ReleaseStgMedium = NULL;
	m_fn_DragQueryFileW = NULL;
}

STDMETHODIMP CoreIDropTarget::QueryInterface( 
	/* [in] */ __RPC__in REFIID riid,
	/* [annotation][iid_is][out] */ 
	_COM_Outptr_  void **ppvObject)
{
	HRESULT hRet = S_OK;

	void *punk = NULL;

	*ppvObject = NULL;

	do 
	{
		if(IID_IUnknown == riid)
		{
			punk = (IUnknown *)this;
		}
		else if (IID_IDropTarget == riid)
		{
			punk = (IDropTarget*)this;
		}
		else
		{
			hRet = m_pIDropTarget->QueryInterface(riid, ppvObject);
			break;
		}

		AddRef();

		*ppvObject = punk;

	} while (FALSE);

	return hRet;
}

STDMETHODIMP_(ULONG) CoreIDropTarget::AddRef()
{
	m_uRefCount++;

	return m_uRefCount;
}

STDMETHODIMP_(ULONG) CoreIDropTarget::Release()
{
	ULONG uCount = 0;

	if(m_uRefCount)
		m_uRefCount--;

	uCount = m_uRefCount;

	if(!uCount)
	{
		delete this;
	}

	return uCount;
}

STDMETHODIMP CoreIDropTarget::DragEnter( 
	/* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
	/* [in] */ DWORD grfKeyState,
	/* [in] */ POINTL pt,
	/* [out][in] */ __RPC__inout DWORD *pdwEffect)
{
	HRESULT		hr = S_OK;

	FORMATETC   FmtEtc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM   Stg = {0};
	HDROP       hDrop = NULL;

	STGMEDIUM	NewStg = {0};

	SIZE_T		DropFilesSize = 0;
	SIZE_T		NewDropFilesSize = 0;

	DROPFILES	*pDropFiles = NULL;
	DROPFILES	*pNewDropFiles = NULL;

	int			nNumFiles = 0;

	WCHAR *FileName = NULL;
	BOOL bStop = FALSE;
	BOOL bFoundNXLFile = FALSE;

	WCHAR *NewFileName = NULL;

	SIZE_T		FileNameLength = 0;

	do 
	{
		if (m_fn_DragQueryFileW == NULL ||
			m_fn_ReleaseStgMedium == NULL)
		{
			break;
		}

		memset(&Stg, 0, sizeof(Stg));

		Stg.tymed = CF_HDROP;

		if (FAILED(pDataObj->GetData(&FmtEtc, &Stg))) 
		{
			break;
		}

		hDrop = (HDROP)GlobalLock(Stg.hGlobal);

		if (NULL == hDrop) 
		{
			break;
		}

		pDropFiles = (DROPFILES *)hDrop;

		if (!pDropFiles->fWide)
		{
			break;
		}

		DropFilesSize = GlobalSize(Stg.hGlobal);

		nNumFiles = m_fn_DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);

		NewDropFilesSize = DropFilesSize + 4*sizeof(WCHAR)*nNumFiles;

		NewStg.tymed = TYMED_HGLOBAL ;

		NewStg.hGlobal = (HGLOBAL)GlobalAlloc(GMEM_MOVEABLE, NewDropFilesSize);

		if (!NewStg.hGlobal)
		{
			break;
		}

		pNewDropFiles = (DROPFILES *)GlobalLock(NewStg.hGlobal);

		memset(pNewDropFiles, 0, NewDropFilesSize);

		memcpy(pNewDropFiles,
			   pDropFiles,
			   min(pDropFiles->pFiles, NewDropFilesSize));

		NewFileName = (WCHAR *)((UCHAR*)pNewDropFiles + pNewDropFiles->pFiles);

		bStop = FALSE;
		FileName = (WCHAR*)((UCHAR*)pDropFiles + pDropFiles->pFiles);

		FileNameLength = wcslen(FileName)*sizeof(WCHAR);

		while (!bStop && ((ULONG_PTR)FileName < (ULONG_PTR)((UCHAR*)pDropFiles + DropFilesSize)))
		{
			hr = nudf::util::nxl::NxrmIsDecryptedFile(FileName);

			if (hr == S_OK)
			{
				bFoundNXLFile = TRUE;

				memcpy(NewFileName,
					   FileName,
					   FileNameLength);

				memcpy(NewFileName + FileNameLength/sizeof(WCHAR),
					   DRAGDROP_NXL_EXT,
					   wcslen(DRAGDROP_NXL_EXT)*sizeof(WCHAR) + sizeof(WCHAR));

			}
			else
			{
				memcpy(NewFileName,
					   FileName,
					   FileNameLength + sizeof(WCHAR));
			}

			FileName = (FileName + wcslen(FileName) + 1);

			NewFileName = (NewFileName + wcslen(NewFileName) + 1);

			FileNameLength = wcslen(FileName)*sizeof(WCHAR);

			if (*FileName == L'\0')
			{
				*NewFileName = L'\0';	// double NULL
				bStop = TRUE;
			}
		}

		GlobalUnlock(NewStg.hGlobal);

		if (!bFoundNXLFile)
		{
			break;
		}

		hr = pDataObj->SetData(&FmtEtc, &NewStg, TRUE);

	} while (FALSE);

	if (Stg.hGlobal)	// m_fn_ReleaseStgMedium MUST not be NULL if Stg.hGlobal is NOT NULL
	{
		GlobalUnlock(Stg.hGlobal);
		m_fn_ReleaseStgMedium(&Stg);
	}

	if (NewStg.hGlobal)	// m_fn_ReleaseStgMedium MUST not be NULL if NewStg.hGlobal is NOT NULL
	{
		m_fn_ReleaseStgMedium(&NewStg);
	}

	return m_pIDropTarget->DragEnter(pDataObj, grfKeyState, pt, pdwEffect);
}

STDMETHODIMP CoreIDropTarget::DragOver( 
	/* [in] */ DWORD grfKeyState,
	/* [in] */ POINTL pt,
	/* [out][in] */ __RPC__inout DWORD *pdwEffect)
{
	return m_pIDropTarget->DragOver(grfKeyState, pt, pdwEffect);
}

STDMETHODIMP CoreIDropTarget::DragLeave()
{
	return m_pIDropTarget->DragLeave();
}

STDMETHODIMP CoreIDropTarget::Drop( 
	/* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
	/* [in] */ DWORD grfKeyState,
	/* [in] */ POINTL pt,
	/* [out][in] */ __RPC__inout DWORD *pdwEffect)
{
	return m_pIDropTarget->Drop(pDataObj, grfKeyState, pt, pdwEffect);
}

BOOL InitializeOLE2CtlHook(void)
{
	BOOL bRet = TRUE;

	do 
	{
		if (!Global.Ole32Handle)
		{
			Global.Ole32Handle = GetModuleHandleW(OLE32_MODULE_NAME);
		}

		if (!Global.Ole32Handle)
		{
			bRet = FALSE;
			break;
		}

		if (Global.OLE2CtlHooksInitialized)
		{
			break;
		}

		g_fnorg_GetClassFile = (GETCLASSFILE)GetProcAddress(Global.Ole32Handle, DRAGDROPCTL_GETCLASSFILE_PROC_NAME);

		if (g_fnorg_GetClassFile)
		{
			if (!install_hook(g_fnorg_GetClassFile, (PVOID*)&g_fn_GetClassFile_trampoline, Core_GetClassFile))
			{
				bRet = FALSE;
				break;
			}
		}

		Global.OLE2CtlHooksInitialized = TRUE;

	} while (FALSE);

	return bRet;
}

void CleanupOLE2CtlHook(void)
{
	if (g_fn_GetClassFile_trampoline)
	{
		remove_hook(g_fn_GetClassFile_trampoline);
		g_fn_GetClassFile_trampoline = NULL;
	}
}

HRESULT WINAPI Core_GetClassFile(
	_In_ LPCOLESTR szFilename, 
	_Out_ CLSID FAR* pclsid)
{
	HRESULT hr = S_OK;

	BOOL bAllow = TRUE;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	do 
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(szFilename, &RightsMask, &CustomRightsMask, &EvaluationId);

		if (FAILED(hr))
		{
			break;
		}

		if (!(RightsMask & BUILTIN_RIGHT_DECRYPT))
		{
			bAllow = FALSE;
		}

	} while (FALSE);

	if (bAllow)
	{
		return g_fn_GetClassFile_trampoline(szFilename, pclsid);
	}
	else
	{
		send_block_notification(szFilename, NxrmdrvEmbeddedOleObjBlocked);

		return MK_E_CANTOPENFILE;
	}
}

BOOL InitializeOfficeDragTextHook(void)
{
	BOOL bRet = TRUE;

	do 
	{
		if (!Global.Ole32Handle)
		{
			Global.Ole32Handle = GetModuleHandleW(OLE32_MODULE_NAME);
		}

		if (!Global.Ole32Handle)
		{
			bRet = FALSE;
			break;
		}

		if (Global.OfficeDragDropTextHooksInitialized)
		{
			break;
		}

		g_fnorg_DoDragDrop = (DODRAGDROP)GetProcAddress(Global.Ole32Handle, DRAGDROPCTL_DODRAGDROP_PROC_NAME);

		if (g_fnorg_DoDragDrop)
		{
			if (!install_hook(g_fnorg_DoDragDrop, (PVOID*)&g_fn_DoDragDrop_trampoline, Core_DoDragDrop))
			{
				bRet = FALSE;
				break;
			}
		}

		Global.OfficeDragDropTextHooksInitialized = TRUE;

	} while (FALSE);

	return bRet;

}

void CleanupOfficeDragTextHook(void)
{
	if (g_fn_DoDragDrop_trampoline)
	{
		remove_hook(g_fn_DoDragDrop_trampoline);
		g_fn_DoDragDrop_trampoline = NULL;
	}
}

HRESULT WINAPI Core_DoDragDrop(
	IN LPDATAOBJECT		pDataObj, 
	IN LPDROPSOURCE		pDropSource,
	IN DWORD			dwOKEffects, 
	OUT LPDWORD			pdwEffect)
{
	HRESULT hr = S_OK;

	BOOL bAllow = TRUE;

	do 
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		if (Global.DisableClipboard)
		{
			bAllow = FALSE;
		}

	} while (FALSE);

	if (bAllow)
	{
		hr = g_fn_DoDragDrop_trampoline(pDataObj, pDropSource, dwOKEffects, pdwEffect);
	}
	else
	{
		hr = DRAGDROP_S_CANCEL;
	}

	return hr;
}

BOOL InitializeOutlookDragDropHook(void)
{
	BOOL bRet = TRUE;

	HMODULE hKernelBase = NULL;

	do
	{
		if (Global.OutlookDragDropHooksInitialized)
		{
			break;
		}

		hKernelBase = GetModuleHandleW(KERNELBASE_MODULE_NAME);

		if (!hKernelBase)
		{
			bRet = FALSE;
			break;
		}

		g_fnorg_FindFirstFileW = (FINDFIRSTFILEW)GetProcAddress(hKernelBase, DRAGDROPCTL_FINDFIRSTFILEW_PROC_NAME);

		if (g_fnorg_FindFirstFileW)
		{
			if (!install_hook(g_fnorg_FindFirstFileW, (PVOID*)&g_fn_FindFirstFileW_trampoline, Core_FindFirstFileW))
			{
				bRet = FALSE;
				break;
			}
		}

		Global.OutlookDragDropHooksInitialized = TRUE;

	} while (FALSE);

	return bRet;
}

void CleanupOutlookDragDropHook(void)
{
	if (g_fn_FindFirstFileW_trampoline)
	{
		remove_hook(g_fn_FindFirstFileW_trampoline);
		g_fn_FindFirstFileW_trampoline = NULL;
	}
}

HANDLE WINAPI Core_FindFirstFileW(
	_In_ LPCWSTR lpFileName,
	_Out_ LPWIN32_FIND_DATAW lpFindFileData
)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	HRESULT hr = S_OK;

	do
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		if (!lpFileName)
		{
			break;
		}

		//
		// skip if lpFileName is NXL file
		//
		hr = nudf::util::nxl::NxrmCheckRights(lpFileName, &RightsMask, &CustomRightsMask, &EvaluationId);

		if (hr == S_OK)
		{
			break;
		}

		{
			std::wstring FileNameWithNXLExtension = lpFileName;

			FileNameWithNXLExtension += DRAGDROP_NXL_EXT;

			if (IsValidNXLFile(lpFileName))
			{

				size_t slen = wcslen(lpFileName);
				assert(slen > 4);
				if (!_wcsicmp(lpFileName + (slen - 4), L".nxl")) {
					break;
				}
				else {//add .NXL extension to file when the file doesn't have it
					if (!MoveFileW(lpFileName, FileNameWithNXLExtension.c_str()))
					{
						break;
					}

					return g_fn_FindFirstFileW_trampoline(FileNameWithNXLExtension.c_str(), lpFindFileData);
				}

			}
		}

	} while (FALSE);

	return g_fn_FindFirstFileW_trampoline(lpFileName, lpFindFileData);
}

BOOL IsValidNXLFile(const WCHAR *FileName)
{
	BOOL bRet = FALSE;

	HANDLE hFile = INVALID_HANDLE_VALUE;

	NXL_SIGNATURE   signature = { 0 };
	unsigned long bytes_read = 0;
	LARGE_INTEGER file_size = { 0 };

	do
	{
		hFile = CreateFileW(FileName,
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			break;
		}

		if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
		{
			break;
		}

		if (!ReadFile(hFile, &signature, sizeof(signature), &bytes_read, NULL))
		{
			break;
		}

		if (bytes_read != sizeof(signature))
		{
			break;
		}

		if (!GetFileSizeEx(hFile, &file_size))
		{
			break;
		}

		if (signature.Code.HighPart != NXL_SIGNATURE_HIGH || signature.Code.LowPart != NXL_SIGNATURE_LOW) {
			break;
		}

		bRet = TRUE;
	} while (FALSE);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	
	return bRet;
}
