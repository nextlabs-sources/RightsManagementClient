#pragma once

#include <shobjidl.h>

#define WINWORD_INSERTFILE_REGKEY		L"\\SOFTWARE\\Microsoft\\Office\\15.0\\Common\\Open Find\\Microsoft Word\\Settings\\Insert File"
#define WINWORD2010_INSERTFILE_REGKEY	L"\\SOFTWARE\\Microsoft\\Office\\14.0\\Common\\Open Find\\Microsoft Word\\Settings\\Insert File"
#define WINWORD_INSERTFILE_VALUE		L"ClientGUID"

#define WINWORD_INSERTFILE_TITLE		L"Insert File"

class CoreIFileDialog : public IFileDialog
{
public:
	CoreIFileDialog();
	CoreIFileDialog(IFileDialog *pIFileDialog);
	~CoreIFileDialog();

	STDMETHODIMP QueryInterface(/* [in] */ __RPC__in REFIID riid,
								/* [annotation][iid_is][out] */	_COM_Outptr_  void **ppvObject);

	STDMETHODIMP_(ULONG) AddRef();

	STDMETHODIMP_(ULONG) Release();

	STDMETHODIMP Show(_In_opt_  HWND hwndOwner);

	STDMETHODIMP SetFileTypes(/* [in] */ UINT cFileTypes,
							  /* [size_is][in] */ __RPC__in_ecount_full(cFileTypes) const COMDLG_FILTERSPEC *rgFilterSpec);

	STDMETHODIMP SetFileTypeIndex(/* [in] */ UINT iFileType);

	STDMETHODIMP GetFileTypeIndex(/* [out] */ __RPC__out UINT *piFileType);

	STDMETHODIMP Advise(/* [in] */ __RPC__in_opt IFileDialogEvents *pfde,
						/* [out] */ __RPC__out DWORD *pdwCookie);

	STDMETHODIMP Unadvise(/* [in] */ DWORD dwCookie);

	STDMETHODIMP SetOptions(/* [in] */ FILEOPENDIALOGOPTIONS fos);

	STDMETHODIMP GetOptions(/* [out] */ __RPC__out FILEOPENDIALOGOPTIONS *pfos);

	STDMETHODIMP SetDefaultFolder(/* [in] */ __RPC__in_opt IShellItem *psi);

	STDMETHODIMP SetFolder(/* [in] */ __RPC__in_opt IShellItem *psi);

	STDMETHODIMP GetFolder(/* [out] */ __RPC__deref_out_opt IShellItem **ppsi);

	STDMETHODIMP GetCurrentSelection(/* [out] */ __RPC__deref_out_opt IShellItem **ppsi);

	STDMETHODIMP SetFileName(/* [string][in] */ __RPC__in_string LPCWSTR pszName);

	STDMETHODIMP GetFileName(/* [string][out] */ __RPC__deref_out_opt_string LPWSTR *pszName);

	STDMETHODIMP SetTitle(/* [string][in] */ __RPC__in_string LPCWSTR pszTitle);

	STDMETHODIMP SetOkButtonLabel(/* [string][in] */ __RPC__in_string LPCWSTR pszText);

	STDMETHODIMP SetFileNameLabel(/* [string][in] */ __RPC__in_string LPCWSTR pszLabel);

	STDMETHODIMP GetResult(/* [out] */ __RPC__deref_out_opt IShellItem **ppsi);

	STDMETHODIMP AddPlace(/* [in] */ __RPC__in_opt IShellItem *psi,
						  /* [in] */ FDAP fdap);

	STDMETHODIMP SetDefaultExtension(/* [string][in] */ __RPC__in_string LPCWSTR pszDefaultExtension);

	STDMETHODIMP Close(/* [in] */ HRESULT hr);

	STDMETHODIMP SetClientGuid(/* [in] */ __RPC__in REFGUID guid);

	STDMETHODIMP ClearClientData();

	STDMETHODIMP SetFilter(/* [in] */ __RPC__in_opt IShellItemFilter *pFilter);

private:

	ULONG			m_uRefCount;
	IFileDialog		*m_pIFileDialog;

	BOOL			m_bIsWordInsertFile;

	COTASKMEMFREE	m_fn_CoTaskMemFree;

	typedef struct _FileTypeIndexNode {

		WCHAR	Name[64];

		UINT	TypeIndex;
	
	}FileTypeIndexNode;

	std::vector<FileTypeIndexNode> m_PowerPointFileTypeBlackList;
};


class CoreIFileSaveDialog : public IFileSaveDialog
{
public:
	CoreIFileSaveDialog();
	CoreIFileSaveDialog(IFileSaveDialog *pIFileDialog);
	~CoreIFileSaveDialog();

	STDMETHODIMP QueryInterface(/* [in] */ __RPC__in REFIID riid,
								/* [annotation][iid_is][out] */	_COM_Outptr_  void **ppvObject);

	STDMETHODIMP_(ULONG) AddRef();

	STDMETHODIMP_(ULONG) Release();

	STDMETHODIMP Show(_In_opt_  HWND hwndOwner);

	STDMETHODIMP SetFileTypes(/* [in] */ UINT cFileTypes,
							  /* [size_is][in] */ __RPC__in_ecount_full(cFileTypes) const COMDLG_FILTERSPEC *rgFilterSpec);

	STDMETHODIMP SetFileTypeIndex(/* [in] */ UINT iFileType);

	STDMETHODIMP GetFileTypeIndex(/* [out] */ __RPC__out UINT *piFileType);

	STDMETHODIMP Advise(/* [in] */ __RPC__in_opt IFileDialogEvents *pfde,
						/* [out] */ __RPC__out DWORD *pdwCookie);

	STDMETHODIMP Unadvise(/* [in] */ DWORD dwCookie);

	STDMETHODIMP SetOptions(/* [in] */ FILEOPENDIALOGOPTIONS fos);

	STDMETHODIMP GetOptions(/* [out] */ __RPC__out FILEOPENDIALOGOPTIONS *pfos);

	STDMETHODIMP SetDefaultFolder(/* [in] */ __RPC__in_opt IShellItem *psi);

	STDMETHODIMP SetFolder(/* [in] */ __RPC__in_opt IShellItem *psi);

	STDMETHODIMP GetFolder(/* [out] */ __RPC__deref_out_opt IShellItem **ppsi);

	STDMETHODIMP GetCurrentSelection(/* [out] */ __RPC__deref_out_opt IShellItem **ppsi);

	STDMETHODIMP SetFileName(/* [string][in] */ __RPC__in_string LPCWSTR pszName);

	STDMETHODIMP GetFileName(/* [string][out] */ __RPC__deref_out_opt_string LPWSTR *pszName);

	STDMETHODIMP SetTitle(/* [string][in] */ __RPC__in_string LPCWSTR pszTitle);

	STDMETHODIMP SetOkButtonLabel(/* [string][in] */ __RPC__in_string LPCWSTR pszText);

	STDMETHODIMP SetFileNameLabel(/* [string][in] */ __RPC__in_string LPCWSTR pszLabel);

	STDMETHODIMP GetResult(/* [out] */ __RPC__deref_out_opt IShellItem **ppsi);

	STDMETHODIMP AddPlace(/* [in] */ __RPC__in_opt IShellItem *psi,
						  /* [in] */ FDAP fdap);

	STDMETHODIMP SetDefaultExtension(/* [string][in] */ __RPC__in_string LPCWSTR pszDefaultExtension);

	STDMETHODIMP Close(/* [in] */ HRESULT hr);

	STDMETHODIMP SetClientGuid(/* [in] */ __RPC__in REFGUID guid);

	STDMETHODIMP ClearClientData();

	STDMETHODIMP SetFilter(/* [in] */ __RPC__in_opt IShellItemFilter *pFilter);

	STDMETHODIMP SetSaveAsItem(/* [in] */ __RPC__in_opt IShellItem *psi);

	STDMETHODIMP SetProperties(/* [in] */ __RPC__in_opt IPropertyStore *pStore);

	STDMETHODIMP SetCollectedProperties(/* [in] */ __RPC__in_opt IPropertyDescriptionList *pList,
										/* [in] */ BOOL fAppendDefault);

	STDMETHODIMP GetProperties(/* [out] */ __RPC__deref_out_opt IPropertyStore **ppStore);

	STDMETHODIMP ApplyProperties(/* [in] */ __RPC__in_opt IShellItem *psi,
								 /* [in] */ __RPC__in_opt IPropertyStore *pStore,
								 /* [unique][in] */ __RPC__in_opt HWND hwnd,
								 /* [unique][in] */ __RPC__in_opt IFileOperationProgressSink *pSink);

private:

	ULONG				m_uRefCount;
	DWORD				m_dwCookie;

	IFileSaveDialog		*m_pIFileSaveDialog;
	
	COTASKMEMFREE		m_fn_CoTaskMemFree;
};

class CoreIFileDialogEvents : public IFileDialogEvents
{
public:
	CoreIFileDialogEvents();
	~CoreIFileDialogEvents();

	STDMETHODIMP QueryInterface( 
		/* [in] */ __RPC__in REFIID riid,
		/* [annotation][iid_is][out] */ 
		_COM_Outptr_  void **ppvObject);

	STDMETHODIMP_(ULONG) AddRef();

	STDMETHODIMP_(ULONG) Release();

	STDMETHODIMP OnFileOk(/* [in] */ __RPC__in_opt IFileDialog *pfd);

	STDMETHODIMP OnFolderChanging(/* [in] */ __RPC__in_opt IFileDialog *pfd,
								  /* [in] */ __RPC__in_opt IShellItem *psiFolder);

	STDMETHODIMP OnFolderChange(/* [in] */ __RPC__in_opt IFileDialog *pfd);

	STDMETHODIMP OnSelectionChange(/* [in] */ __RPC__in_opt IFileDialog *pfd);

	STDMETHODIMP OnShareViolation(/* [in] */ __RPC__in_opt IFileDialog *pfd,
								  /* [in] */ __RPC__in_opt IShellItem *psi,
								  /* [out] */ __RPC__out FDE_SHAREVIOLATION_RESPONSE *pResponse);

	STDMETHODIMP OnTypeChange(/* [in] */ __RPC__in_opt IFileDialog *pfd);

	STDMETHODIMP OnOverwrite(/* [in] */ __RPC__in_opt IFileDialog *pfd,
							 /* [in] */ __RPC__in_opt IShellItem *psi,
							 /* [out] */ __RPC__out FDE_OVERWRITE_RESPONSE *pResponse);

private:

	ULONG		m_uRefCount;
};
