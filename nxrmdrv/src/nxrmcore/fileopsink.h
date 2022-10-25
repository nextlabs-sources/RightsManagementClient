#pragma once

#include <shobjidl.h>
#include <shlobj.h>

class CoreIFileOperationProgressSink : public IFileOperationProgressSink
{
public:
	CoreIFileOperationProgressSink();
	~CoreIFileOperationProgressSink();

	STDMETHODIMP QueryInterface(/* [in] */ __RPC__in REFIID riid,
								/* [annotation][iid_is][out] */ _COM_Outptr_  void **ppvObject);

	STDMETHODIMP_(ULONG) AddRef();

	STDMETHODIMP_(ULONG) Release();

	 STDMETHODIMP StartOperations();

	 STDMETHODIMP FinishOperations(/* [in] */ HRESULT hrResult);

	 STDMETHODIMP PreRenameItem(/* [in] */ DWORD dwFlags,
								/* [in] */ __RPC__in_opt IShellItem *psiItem,
								/* [string][unique][in] */ __RPC__in_opt_string LPCWSTR pszNewName);

	 STDMETHODIMP PostRenameItem(/* [in] */ DWORD dwFlags,
								 /* [in] */ __RPC__in_opt IShellItem *psiItem,
								 /* [string][in] */ __RPC__in_string LPCWSTR pszNewName,
								 /* [in] */ HRESULT hrRename,
								 /* [in] */ __RPC__in_opt IShellItem *psiNewlyCreated);

	 STDMETHODIMP PreMoveItem(/* [in] */ DWORD dwFlags,
							  /* [in] */ __RPC__in_opt IShellItem *psiItem,
							  /* [in] */ __RPC__in_opt IShellItem *psiDestinationFolder,
							  /* [string][unique][in] */ __RPC__in_opt_string LPCWSTR pszNewName);

	 STDMETHODIMP PostMoveItem(/* [in] */ DWORD dwFlags,
							   /* [in] */ __RPC__in_opt IShellItem *psiItem,
							   /* [in] */ __RPC__in_opt IShellItem *psiDestinationFolder,
							   /* [string][unique][in] */ __RPC__in_opt_string LPCWSTR pszNewName,
							   /* [in] */ HRESULT hrMove,
							   /* [in] */ __RPC__in_opt IShellItem *psiNewlyCreated);

	 STDMETHODIMP PreCopyItem(/* [in] */ DWORD dwFlags,
							  /* [in] */ __RPC__in_opt IShellItem *psiItem,
							  /* [in] */ __RPC__in_opt IShellItem *psiDestinationFolder,
							  /* [string][unique][in] */ __RPC__in_opt_string LPCWSTR pszNewName);

	 STDMETHODIMP PostCopyItem(/* [in] */ DWORD dwFlags,
							   /* [in] */ __RPC__in_opt IShellItem *psiItem,
							   /* [in] */ __RPC__in_opt IShellItem *psiDestinationFolder,
							   /* [string][unique][in] */ __RPC__in_opt_string LPCWSTR pszNewName,
							   /* [in] */ HRESULT hrCopy,
							   /* [in] */ __RPC__in_opt IShellItem *psiNewlyCreated);

	 STDMETHODIMP PreDeleteItem(/* [in] */ DWORD dwFlags,
							    /* [in] */ __RPC__in_opt IShellItem *psiItem);

	 STDMETHODIMP PostDeleteItem(/* [in] */ DWORD dwFlags,
								 /* [in] */ __RPC__in_opt IShellItem *psiItem,
								 /* [in] */ HRESULT hrDelete,
								 /* [in] */ __RPC__in_opt IShellItem *psiNewlyCreated);

	 STDMETHODIMP PreNewItem(/* [in] */ DWORD dwFlags,
							 /* [in] */ __RPC__in_opt IShellItem *psiDestinationFolder,
							 /* [string][unique][in] */ __RPC__in_opt_string LPCWSTR pszNewName);

	 STDMETHODIMP PostNewItem(/* [in] */ DWORD dwFlags,
							  /* [in] */ __RPC__in_opt IShellItem *psiDestinationFolder,
							  /* [string][unique][in] */ __RPC__in_opt_string LPCWSTR pszNewName,
							  /* [string][unique][in] */ __RPC__in_opt_string LPCWSTR pszTemplateName,
							  /* [in] */ DWORD dwFileAttributes,
							  /* [in] */ HRESULT hrNew,
							  /* [in] */ __RPC__in_opt IShellItem *psiNewItem);

	 STDMETHODIMP UpdateProgress(/* [in] */ UINT iWorkTotal,
								 /* [in] */ UINT iWorkSoFar);

	 STDMETHODIMP ResetTimer();

	 STDMETHODIMP PauseTimer();

	 STDMETHODIMP ResumeTimer();

private:

	ULONG				m_uRefCount;

	LIST_ENTRY			m_nxlFilesList;
	CRITICAL_SECTION	m_nxlFilesListLock;

	COTASKMEMFREE		m_fn_CoTaskMemFree;
	SHCHANGENOTIFY		m_fn_SHChangeNotify;
};