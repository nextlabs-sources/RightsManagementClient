#include "stdafx.h"
#include "nxrmcoreglobal.h"
#include "nxrmdrv.h"
#include "nxrmcorehlp.h"
#include "filedialog.h"
#include "adobe.h"

#ifdef __cplusplus
extern "C" {
#endif

	extern CORE_GLOBAL_DATA Global;

	extern void				forecast_saveas_to_engine(const WCHAR *SrcFileName, const WCHAR *SaveAsFileName);

	extern BOOL				send_block_notification(const WCHAR *FileName, BLOCK_NOTIFICATION_TYPE Type);
	
	BOOL update_active_document_from_protected_child_process(void);

#ifdef __cplusplus
}
#endif

static HRESULT takecare_winword_insert_file(WCHAR *FileName);
static HRESULT takecare_powerpoint_saveas_more_than_one_file(WCHAR *FileName);

CoreIFileDialog::CoreIFileDialog()
{
	m_uRefCount = 1;
	m_pIFileDialog = NULL;
	m_fn_CoTaskMemFree = NULL;
	m_bIsWordInsertFile = FALSE;

	m_PowerPointFileTypeBlackList = {				\
	{L"GIF Graphics Interchange Format ", 0},		\
	{L"JPEG File Interchange Format ", 0},			\
	{L"PNG Portable Network Graphics Format ", 0},	\
	{L"TIFF Tag Image File Format ", 0},			\
	{L"Device Independent Bitmap ", 0},				\
	{L"Windows Metafile ", 0},						\
	{L"Enhanced Windows Metafile ", 0}				\
	};
}

CoreIFileDialog::CoreIFileDialog(IFileDialog *pIFileDialog)
{
	HMODULE hOle32 = NULL;

	m_uRefCount = 1;
	m_pIFileDialog = pIFileDialog;
	m_bIsWordInsertFile = FALSE;

	hOle32 = GetModuleHandleW(OLE32_MODULE_NAME);

	if (hOle32)
	{
		m_fn_CoTaskMemFree = (COTASKMEMFREE)GetProcAddress(hOle32, NXRMCORE_COTASKMEMFREE_PROC_NAME);
	}

	m_PowerPointFileTypeBlackList = {				\
	{L"GIF Graphics Interchange Format ", 0},		\
	{L"JPEG File Interchange Format ", 0},			\
	{L"PNG Portable Network Graphics Format ", 0},	\
	{L"TIFF Tag Image File Format ", 0},			\
	{L"Device Independent Bitmap ", 0},				\
	{L"Windows Metafile ", 0},						\
	{L"Enhanced Windows Metafile ", 0}				\
	};
}

CoreIFileDialog::~CoreIFileDialog()
{
	if (m_pIFileDialog)
	{
		m_pIFileDialog->Release();
		m_pIFileDialog = NULL;
	}

	m_fn_CoTaskMemFree = NULL;
}

STDMETHODIMP CoreIFileDialog::QueryInterface(__RPC__in REFIID riid, _COM_Outptr_ void **ppobj)
{
	HRESULT hRet = S_OK;

	void *punk = NULL;

	*ppobj = NULL;

	do 
	{
		if(IID_IUnknown == riid)
		{
			punk = (IUnknown *)this;
		}
		else if (IID_IModalWindow == riid)
		{
			punk = (IModalWindow*)this;
		}
		else if (IID_IFileDialog == riid)
		{
			punk = (IFileDialog*)this;
		}
		//else if (IID_IFileSaveDialog == riid)
		//{
		//	hRet = m_pIFileDialog->QueryInterface(riid, &punk);

		//	if (SUCCEEDED(hRet))
		//	{
		//		CoreIFileSaveDialog *pSaveDialog = new CoreIFileSaveDialog((IFileSaveDialog*)punk);

		//		*ppobj = (void*)pSaveDialog;
		//	}

		//	break;
		//}
		else
		{
			hRet = m_pIFileDialog->QueryInterface(riid, ppobj);
			break;
		}

		AddRef();

		*ppobj = punk;

	} while (FALSE);

	return hRet;
}

STDMETHODIMP_(ULONG) CoreIFileDialog::AddRef()
{
	m_uRefCount++;

	return m_uRefCount;
}

STDMETHODIMP_(ULONG) CoreIFileDialog::Release()
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

STDMETHODIMP CoreIFileDialog::Show(_In_opt_  HWND hwndOwner)
{
	HRESULT hr = S_OK;
	HRESULT hrlocal = S_OK;

	IShellItem *pItem = NULL;

	WCHAR *FileName = NULL;

	WCHAR ActiveDocName[MAX_PATH] = {0};

	FILEOPENDIALOGOPTIONS fos = {0};

	UINT TypeIndex = 0;

	hr = m_pIFileDialog->Show(hwndOwner);

	if (SUCCEEDED(hr))
	{
		hrlocal = m_pIFileDialog->GetResult(&pItem);

		if (SUCCEEDED(hrlocal))
		{
			hrlocal = pItem->GetDisplayName(SIGDN_FILESYSPATH, &FileName);

			if (SUCCEEDED(hrlocal))
			{
				do 
				{
					if (!init_rm_section_safe())
					{
						break;
					}

					hr = m_pIFileDialog->GetOptions(&fos);

					if (!SUCCEEDED(hr))
					{
						break;
					}
					
					if (!(fos & FOS_OVERWRITEPROMPT))
					{
						//
						// Open?
						//
						if (m_bIsWordInsertFile)
						{
							hrlocal = takecare_winword_insert_file(FileName);

							if (!SUCCEEDED(hrlocal))
							{
								send_block_notification(FileName, NxrmdrvEmbeddedOleObjBlocked);
								hr = hrlocal;
							}
						}

						if (!Global.IsAdobeReader || (fos & FOS_ALLOWMULTISELECT))
						{
							break;
						}
					}

					if (Global.IsAdobeReader && Global.AdobeMode == ADOBE_MODE_PARENT)
					{
						update_active_document_from_protected_child_process();
					}

					EnterCriticalSection(&Global.ActiveDocFileNameLock);

					memcpy(ActiveDocName,
						   Global.ActiveDocFileName,
						   min(sizeof(ActiveDocName) - sizeof(WCHAR), sizeof(Global.ActiveDocFileName) - sizeof(WCHAR)));

					LeaveCriticalSection(&Global.ActiveDocFileNameLock);

					if (Global.IsAdobeReader && wcslen(ActiveDocName) == 0 && wcsstr(Global.AdobeCommandLine, ADOBE_IEMODE_CMD_LINE) == NULL)
					{
						hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);

						send_block_notification(ActiveDocName, NxrmdrvAdobeHookIsNotReady);

						break;
					}

					if (Global.IsPowerPoint)
					{
						hr = m_pIFileDialog->GetFileTypeIndex(&TypeIndex);

						if (SUCCEEDED(hr))
						{
							BOOL FileTypeInBlackList = FALSE;

							for (const auto &ite : m_PowerPointFileTypeBlackList)
							{
								if (ite.TypeIndex == TypeIndex)
								{
									FileTypeInBlackList = TRUE;
									break;
								}
							}

							hrlocal = takecare_powerpoint_saveas_more_than_one_file(ActiveDocName);

							if (FileTypeInBlackList && (!SUCCEEDED(hrlocal)))
							{
								send_block_notification(ActiveDocName, NxrmdrvExportSlidesBlocked);

								hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);

								break;
							}
						}
					}

					if (Global.IsMsOffice || Global.IsAdobeReader)
					{
						ULONGLONG RightsMask = 0;
						ULONGLONG CustomRightsMask = 0;
						ULONGLONG EvluationId = 0;

						//
						// take care of rights
						//
						hrlocal = nudf::util::nxl::NxrmCheckRights(ActiveDocName, &RightsMask, &CustomRightsMask, &EvluationId);

						if (SUCCEEDED(hrlocal))
						{
							//
							// don't have edit right and it's SaveAs
							//
							if (!(RightsMask & BUILTIN_RIGHT_EDIT) && _wcsicmp(ActiveDocName, FileName) == 0)
							{
								hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);

								send_block_notification(ActiveDocName, NxrmdrvSaveFileBlocked);

								break;
							}
							
							if (!(RightsMask & BUILTIN_RIGHT_SAVEAS) && _wcsicmp(ActiveDocName, FileName) != 0)
							{
								hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);

								send_block_notification(ActiveDocName, NxrmdrvSaveFileBlocked);
								
								break;
							}

							//
							// take care excel defect. Excel update its internal ActiveDoc even SaveAs blocked by nxrmflt
							// code blow make sure the destination is on local drive
							{
								WCHAR c = 0;
								WCHAR d = 0;
								WCHAR e = 0;

								WCHAR tmpFileName[4] = { 0 };

								BOOL OnRemoteDrive = FALSE;

								do
								{
									if (wcslen(FileName) < sizeof(tmpFileName) / sizeof(WCHAR))
									{
										break;
									}

									c = FileName[0];
									d = FileName[1];
									e = FileName[2];

									if (d != L':' || e != L'\\')
									{
										OnRemoteDrive = TRUE;
										break;
									}

									if ((!(c >= L'A' && c <= L'Z')) &&
										(!(c >= 'a' && c <= 'z')))
									{
										OnRemoteDrive = TRUE;
										break;
									}

									tmpFileName[0] = c;
									tmpFileName[1] = d;
									tmpFileName[2] = e;
									tmpFileName[3] = L'\0';

									if (DRIVE_FIXED != GetDriveTypeW(tmpFileName))
									{
										OnRemoteDrive = TRUE;
										break;
									}

								} while (FALSE);

								if (OnRemoteDrive)
								{
									hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
									send_block_notification(ActiveDocName, NxrmdrvSaveAsToUnprotectedVolume);
								}
							}
						}
					}

					forecast_saveas_to_engine(ActiveDocName, FileName);

				} while (FALSE);

				if (m_fn_CoTaskMemFree)
				{
					m_fn_CoTaskMemFree(FileName);
				}

				FileName = NULL;
			}
			else if (Global.IsMsOffice || Global.IsAdobeReader)
			{
				hrlocal = pItem->GetDisplayName(SIGDN_URL, &FileName);

				if (SUCCEEDED(hrlocal))
				{
					do 
					{
						ULONGLONG RightsMask = 0;
						ULONGLONG CustomRightsMask = 0;
						ULONGLONG EvluationId = 0;

						if (!init_rm_section_safe())
						{
							break;
						}

						EnterCriticalSection(&Global.ActiveDocFileNameLock);

						memcpy(ActiveDocName,
							   Global.ActiveDocFileName,
							   min(sizeof(ActiveDocName) - sizeof(WCHAR), sizeof(Global.ActiveDocFileName) - sizeof(WCHAR)));

						LeaveCriticalSection(&Global.ActiveDocFileNameLock);

						//
						// Is this a NXL File?
						//
						hrlocal = nudf::util::nxl::NxrmCheckRights(ActiveDocName, &RightsMask, &CustomRightsMask, &EvluationId);

						if (SUCCEEDED(hrlocal))
						{
							hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
							send_block_notification(ActiveDocName, NxrmdrvSaveAsToUnprotectedVolume);
						}

					} while (FALSE);

					if (m_fn_CoTaskMemFree)
					{
						m_fn_CoTaskMemFree(FileName);
					}

					FileName = NULL;
				}
			}

			pItem->Release();
		}
	}

	return hr; 
}

STDMETHODIMP CoreIFileDialog::SetFileTypes(/* [in] */ UINT cFileTypes,
										   /* [size_is][in] */ __RPC__in_ecount_full(cFileTypes) const COMDLG_FILTERSPEC *rgFilterSpec)
{
	if (Global.IsPowerPoint)
	{
		for (UINT i = 0; i < cFileTypes; i++)
		{
			std::for_each (m_PowerPointFileTypeBlackList.begin(), m_PowerPointFileTypeBlackList.end(), [&](auto &ite) {
				if (_wcsicmp(rgFilterSpec[i].pszName, ite.Name) == 0)
				{
					ite.TypeIndex = i + 1;
				}
			});
		}
	}

	return m_pIFileDialog->SetFileTypes(cFileTypes, rgFilterSpec);
}

STDMETHODIMP CoreIFileDialog::SetFileTypeIndex(/* [in] */ UINT iFileType)
{
	return m_pIFileDialog->SetFileTypeIndex(iFileType);
}

STDMETHODIMP CoreIFileDialog::GetFileTypeIndex(/* [out] */ __RPC__out UINT *piFileType)
{
	return m_pIFileDialog->GetFileTypeIndex(piFileType);
}

STDMETHODIMP CoreIFileDialog::Advise(/* [in] */ __RPC__in_opt IFileDialogEvents *pfde,
									 /* [out] */ __RPC__out DWORD *pdwCookie)
{
	return m_pIFileDialog->Advise(pfde, pdwCookie);
}

STDMETHODIMP CoreIFileDialog::Unadvise(/* [in] */ DWORD dwCookie)
{
	return m_pIFileDialog->Unadvise(dwCookie);
}

STDMETHODIMP CoreIFileDialog::SetOptions(/* [in] */ FILEOPENDIALOGOPTIONS fos)
{
	return m_pIFileDialog->SetOptions(fos);
}

STDMETHODIMP CoreIFileDialog::GetOptions(/* [out] */ __RPC__out FILEOPENDIALOGOPTIONS *pfos)
{
	return m_pIFileDialog->GetOptions(pfos);
}

STDMETHODIMP CoreIFileDialog::SetDefaultFolder(/* [in] */ __RPC__in_opt IShellItem *psi)
{
	return m_pIFileDialog->SetDefaultFolder(psi);
}

STDMETHODIMP CoreIFileDialog::SetFolder(/* [in] */ __RPC__in_opt IShellItem *psi)
{
	return m_pIFileDialog->SetFolder(psi);
}

STDMETHODIMP CoreIFileDialog::GetFolder(/* [out] */ __RPC__deref_out_opt IShellItem **ppsi)
{
	return m_pIFileDialog->GetFolder(ppsi);
}

STDMETHODIMP CoreIFileDialog::GetCurrentSelection(/* [out] */ __RPC__deref_out_opt IShellItem **ppsi)
{
	return m_pIFileDialog->GetCurrentSelection(ppsi);
}

STDMETHODIMP CoreIFileDialog::SetFileName(/* [string][in] */ __RPC__in_string LPCWSTR pszName)
{
	return m_pIFileDialog->SetFileName(pszName);
}

STDMETHODIMP CoreIFileDialog::GetFileName(/* [string][out] */ __RPC__deref_out_opt_string LPWSTR *pszName)
{
	return m_pIFileDialog->GetFileName(pszName);
}

STDMETHODIMP CoreIFileDialog::SetTitle(/* [string][in] */ __RPC__in_string LPCWSTR pszTitle)
{
	if (Global.IsWinWord && pszTitle)
	{
		if (_wcsicmp(pszTitle, WINWORD_INSERTFILE_TITLE) == 0)
		{
			m_bIsWordInsertFile = TRUE;
		}
	}

	return m_pIFileDialog->SetTitle(pszTitle);
}

STDMETHODIMP CoreIFileDialog::SetOkButtonLabel(/* [string][in] */ __RPC__in_string LPCWSTR pszText)
{
	return m_pIFileDialog->SetOkButtonLabel(pszText);
}

STDMETHODIMP CoreIFileDialog::SetFileNameLabel(/* [string][in] */ __RPC__in_string LPCWSTR pszLabel)
{
	return m_pIFileDialog->SetFileNameLabel(pszLabel);
}

STDMETHODIMP CoreIFileDialog::GetResult(/* [out] */ __RPC__deref_out_opt IShellItem **ppsi)
{
	return m_pIFileDialog->GetResult(ppsi);
}

STDMETHODIMP CoreIFileDialog::AddPlace(/* [in] */ __RPC__in_opt IShellItem *psi,
									   /* [in] */ FDAP fdap)
{
	return m_pIFileDialog->AddPlace(psi, fdap);
}

STDMETHODIMP CoreIFileDialog::SetDefaultExtension(/* [string][in] */ __RPC__in_string LPCWSTR pszDefaultExtension)
{
	return m_pIFileDialog->SetDefaultExtension(pszDefaultExtension);
}

STDMETHODIMP CoreIFileDialog::Close(/* [in] */ HRESULT hr)
{
	return m_pIFileDialog->Close(hr);
}

STDMETHODIMP CoreIFileDialog::SetClientGuid(/* [in] */ __RPC__in REFGUID guid)
{
	return m_pIFileDialog->SetClientGuid(guid);
}

STDMETHODIMP CoreIFileDialog::ClearClientData()
{
	return m_pIFileDialog->ClearClientData();
}

STDMETHODIMP CoreIFileDialog::SetFilter(/* [in] */ __RPC__in_opt IShellItemFilter *pFilter)
{
	return m_pIFileDialog->SetFilter(pFilter);
}


CoreIFileSaveDialog::CoreIFileSaveDialog()
{
	m_uRefCount = 1;
	m_dwCookie = 0;
	m_pIFileSaveDialog = NULL;
	m_fn_CoTaskMemFree = NULL;
}

CoreIFileSaveDialog::CoreIFileSaveDialog(IFileSaveDialog *pIFileSaveDialog)
{
	HMODULE hOle32 = NULL;

	m_uRefCount = 1;
	m_dwCookie = 0;
	m_pIFileSaveDialog = pIFileSaveDialog;

	hOle32 = GetModuleHandleW(OLE32_MODULE_NAME);

	if (hOle32)
	{
		m_fn_CoTaskMemFree = (COTASKMEMFREE)GetProcAddress(hOle32, NXRMCORE_COTASKMEMFREE_PROC_NAME);
	}
}

CoreIFileSaveDialog::~CoreIFileSaveDialog()
{
	if (m_pIFileSaveDialog)
	{
		m_pIFileSaveDialog->Release();
		m_pIFileSaveDialog = NULL;
	}

	m_fn_CoTaskMemFree = NULL;
}

STDMETHODIMP CoreIFileSaveDialog::QueryInterface(__RPC__in REFIID riid, _COM_Outptr_ void **ppobj)
{
	HRESULT hRet = S_OK;

	void *punk = NULL;

	*ppobj = NULL;

	do 
	{
		if(IID_IUnknown == riid)
		{
			punk = (IUnknown *)this;
		}
		else if (IID_IModalWindow == riid)
		{
			punk = (IModalWindow*)this;
		}
		else if (IID_IFileDialog == riid)
		{
			punk = (IFileDialog*)this;
		}
		else if (IID_IFileSaveDialog == riid)
		{
			punk = (IFileSaveDialog*)this;
		}
		else
		{
			hRet = m_pIFileSaveDialog->QueryInterface(riid, ppobj);
			break;
		}

		AddRef();

		*ppobj = punk;

	} while (FALSE);

	return hRet;
}

STDMETHODIMP_(ULONG) CoreIFileSaveDialog::AddRef()
{
	m_uRefCount++;

	return m_uRefCount;
}

STDMETHODIMP_(ULONG) CoreIFileSaveDialog::Release()
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

STDMETHODIMP CoreIFileSaveDialog::Show(_In_opt_  HWND hwndOwner)
{
	HRESULT hr = S_OK;
	HRESULT hrlocal = S_OK;

	IShellItem *pItem = NULL;

	WCHAR *FileName = NULL;

	WCHAR ActiveDocName[MAX_PATH] = {0};

	hr = m_pIFileSaveDialog->Show(hwndOwner);

	if (SUCCEEDED(hr))
	{
		hrlocal = m_pIFileSaveDialog->GetResult(&pItem);

		if (SUCCEEDED(hrlocal))
		{
			hrlocal = pItem->GetDisplayName(SIGDN_FILESYSPATH, &FileName);

			if (SUCCEEDED(hrlocal))
			{
				do 
				{
					if (!init_rm_section_safe())
					{
						break;
					}

					if (Global.IsAdobeReader && Global.AdobeMode == ADOBE_MODE_PARENT)
					{
						update_active_document_from_protected_child_process();
					}

					EnterCriticalSection(&Global.ActiveDocFileNameLock);

					memcpy(ActiveDocName, 
						   Global.ActiveDocFileName,
						   min(sizeof(ActiveDocName), sizeof(Global.ActiveDocFileName)));

					LeaveCriticalSection(&Global.ActiveDocFileNameLock);

					forecast_saveas_to_engine(ActiveDocName, FileName);

				} while (FALSE);

				if (m_fn_CoTaskMemFree)
				{
					m_fn_CoTaskMemFree(FileName);
				}

				FileName = NULL;
			}

			pItem->Release();
		}
	}

	return hr; 
}

STDMETHODIMP CoreIFileSaveDialog::SetFileTypes(/* [in] */ UINT cFileTypes,
											   /* [size_is][in] */ __RPC__in_ecount_full(cFileTypes) const COMDLG_FILTERSPEC *rgFilterSpec)
{
	return m_pIFileSaveDialog->SetFileTypes(cFileTypes, rgFilterSpec);
}

STDMETHODIMP CoreIFileSaveDialog::SetFileTypeIndex(/* [in] */ UINT iFileType)
{
	return m_pIFileSaveDialog->SetFileTypeIndex(iFileType);
}

STDMETHODIMP CoreIFileSaveDialog::GetFileTypeIndex(/* [out] */ __RPC__out UINT *piFileType)
{
	return m_pIFileSaveDialog->GetFileTypeIndex(piFileType);
}

STDMETHODIMP CoreIFileSaveDialog::Advise(/* [in] */ __RPC__in_opt IFileDialogEvents *pfde,
										 /* [out] */ __RPC__out DWORD *pdwCookie)
{
	return m_pIFileSaveDialog->Advise(pfde, pdwCookie);
}

STDMETHODIMP CoreIFileSaveDialog::Unadvise(/* [in] */ DWORD dwCookie)
{
	return m_pIFileSaveDialog->Unadvise(dwCookie);
}

STDMETHODIMP CoreIFileSaveDialog::SetOptions(/* [in] */ FILEOPENDIALOGOPTIONS fos)
{
	return m_pIFileSaveDialog->SetOptions(fos);
}

STDMETHODIMP CoreIFileSaveDialog::GetOptions(/* [out] */ __RPC__out FILEOPENDIALOGOPTIONS *pfos)
{
	return m_pIFileSaveDialog->GetOptions(pfos);
}

STDMETHODIMP CoreIFileSaveDialog::SetDefaultFolder(/* [in] */ __RPC__in_opt IShellItem *psi)
{
	return m_pIFileSaveDialog->SetDefaultFolder(psi);
}

STDMETHODIMP CoreIFileSaveDialog::SetFolder(/* [in] */ __RPC__in_opt IShellItem *psi)
{
	return m_pIFileSaveDialog->SetFolder(psi);
}

STDMETHODIMP CoreIFileSaveDialog::GetFolder(/* [out] */ __RPC__deref_out_opt IShellItem **ppsi)
{
	return m_pIFileSaveDialog->GetFolder(ppsi);
}

STDMETHODIMP CoreIFileSaveDialog::GetCurrentSelection(/* [out] */ __RPC__deref_out_opt IShellItem **ppsi)
{
	return m_pIFileSaveDialog->GetCurrentSelection(ppsi);
}

STDMETHODIMP CoreIFileSaveDialog::SetFileName(/* [string][in] */ __RPC__in_string LPCWSTR pszName)
{
	return m_pIFileSaveDialog->SetFileName(pszName);
}

STDMETHODIMP CoreIFileSaveDialog::GetFileName(/* [string][out] */ __RPC__deref_out_opt_string LPWSTR *pszName)
{
	return m_pIFileSaveDialog->GetFileName(pszName);
}

STDMETHODIMP CoreIFileSaveDialog::SetTitle(/* [string][in] */ __RPC__in_string LPCWSTR pszTitle)
{
	return m_pIFileSaveDialog->SetTitle(pszTitle);
}

STDMETHODIMP CoreIFileSaveDialog::SetOkButtonLabel(/* [string][in] */ __RPC__in_string LPCWSTR pszText)
{
	return m_pIFileSaveDialog->SetOkButtonLabel(pszText);
}

STDMETHODIMP CoreIFileSaveDialog::SetFileNameLabel(/* [string][in] */ __RPC__in_string LPCWSTR pszLabel)
{
	return m_pIFileSaveDialog->SetFileNameLabel(pszLabel);
}

STDMETHODIMP CoreIFileSaveDialog::GetResult(/* [out] */ __RPC__deref_out_opt IShellItem **ppsi)
{
	return m_pIFileSaveDialog->GetResult(ppsi);
}

STDMETHODIMP CoreIFileSaveDialog::AddPlace(/* [in] */ __RPC__in_opt IShellItem *psi,
										   /* [in] */ FDAP fdap)
{
	return m_pIFileSaveDialog->AddPlace(psi, fdap);
}

STDMETHODIMP CoreIFileSaveDialog::SetDefaultExtension(/* [string][in] */ __RPC__in_string LPCWSTR pszDefaultExtension)
{
	return m_pIFileSaveDialog->SetDefaultExtension(pszDefaultExtension);
}

STDMETHODIMP CoreIFileSaveDialog::Close(/* [in] */ HRESULT hr)
{
	return m_pIFileSaveDialog->Close(hr);
}

STDMETHODIMP CoreIFileSaveDialog::SetClientGuid(/* [in] */ __RPC__in REFGUID guid)
{
	return m_pIFileSaveDialog->SetClientGuid(guid);
}

STDMETHODIMP CoreIFileSaveDialog::ClearClientData()
{
	return m_pIFileSaveDialog->ClearClientData();
}

STDMETHODIMP CoreIFileSaveDialog::SetFilter(/* [in] */ __RPC__in_opt IShellItemFilter *pFilter)
{
	return m_pIFileSaveDialog->SetFilter(pFilter);
}

STDMETHODIMP CoreIFileSaveDialog::SetSaveAsItem(/* [in] */ __RPC__in_opt IShellItem *psi)
{
	return m_pIFileSaveDialog->SetSaveAsItem(psi);
}

STDMETHODIMP CoreIFileSaveDialog::SetProperties(/* [in] */ __RPC__in_opt IPropertyStore *pStore)
{
	return m_pIFileSaveDialog->SetProperties(pStore);
}

STDMETHODIMP CoreIFileSaveDialog::SetCollectedProperties(/* [in] */ __RPC__in_opt IPropertyDescriptionList *pList,
														 /* [in] */ BOOL fAppendDefault)
{
	return m_pIFileSaveDialog->SetCollectedProperties(pList, fAppendDefault);
}

STDMETHODIMP CoreIFileSaveDialog::GetProperties(/* [out] */ __RPC__deref_out_opt IPropertyStore **ppStore)
{
	return m_pIFileSaveDialog->GetProperties(ppStore);
}

STDMETHODIMP CoreIFileSaveDialog::ApplyProperties(/* [in] */ __RPC__in_opt IShellItem *psi,
												  /* [in] */ __RPC__in_opt IPropertyStore *pStore,
												  /* [unique][in] */ __RPC__in_opt HWND hwnd,
												  /* [unique][in] */ __RPC__in_opt IFileOperationProgressSink *pSink)
{
	return m_pIFileSaveDialog->ApplyProperties(psi, pStore, hwnd, pSink);
}

CoreIFileDialogEvents::CoreIFileDialogEvents()
{
	m_uRefCount = 1;
}

CoreIFileDialogEvents::~CoreIFileDialogEvents()
{

}

STDMETHODIMP CoreIFileDialogEvents::QueryInterface(/* [in] */ __RPC__in REFIID riid,
												   /* [annotation][iid_is][out] */ _COM_Outptr_  void **ppobj)
{
	HRESULT hRet = S_OK;

	void *punk = NULL;

	*ppobj = NULL;

	do 
	{
		if(IID_IUnknown == riid)
		{
			punk = (IUnknown *)this;
		}
		else if (IID_IFileDialogEvents == riid)
		{
			punk = (IFileDialogEvents*)this;
		}
		else
		{
			hRet = E_NOINTERFACE;
			break;
		}

		AddRef();

		*ppobj = punk;

	} while (FALSE);

	return hRet;

}

STDMETHODIMP_(ULONG) CoreIFileDialogEvents::AddRef()
{
	m_uRefCount++;

	return m_uRefCount;
}

STDMETHODIMP_(ULONG) CoreIFileDialogEvents::Release()
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

STDMETHODIMP CoreIFileDialogEvents::OnFileOk(/* [in] */ __RPC__in_opt IFileDialog *pfd)
{
	HRESULT hr = S_OK;

	return hr;
}

STDMETHODIMP CoreIFileDialogEvents::OnFolderChanging(/* [in] */ __RPC__in_opt IFileDialog *pfd,
													 /* [in] */ __RPC__in_opt IShellItem *psiFolder)
{
	HRESULT hr = S_OK;

	return hr;
}

STDMETHODIMP CoreIFileDialogEvents::OnFolderChange(/* [in] */ __RPC__in_opt IFileDialog *pfd)
{
	HRESULT hr = S_OK;

	return hr;
}

STDMETHODIMP CoreIFileDialogEvents::OnSelectionChange(/* [in] */ __RPC__in_opt IFileDialog *pfd)
{
	HRESULT hr = S_OK;

	return hr;
}

STDMETHODIMP CoreIFileDialogEvents::OnShareViolation(/* [in] */ __RPC__in_opt IFileDialog *pfd,
													 /* [in] */ __RPC__in_opt IShellItem *psi,
													 /* [out] */ __RPC__out FDE_SHAREVIOLATION_RESPONSE *pResponse)
{
	HRESULT hr = S_OK;

	return hr;
}

STDMETHODIMP CoreIFileDialogEvents::OnTypeChange(/* [in] */ __RPC__in_opt IFileDialog *pfd)
{
	HRESULT hr = S_OK;

	return hr;
}

STDMETHODIMP CoreIFileDialogEvents::OnOverwrite(/* [in] */ __RPC__in_opt IFileDialog *pfd,
												/* [in] */ __RPC__in_opt IShellItem *psi,
												/* [out] */ __RPC__out FDE_OVERWRITE_RESPONSE *pResponse)
{
	HRESULT hr = S_OK;

	return hr;
}

BOOL update_active_document_from_protected_child_process(void)
{
	BOOL bRet = FALSE;

	QUERY_PROTECTEDMODEAPPINFO_REQUEST req = {0};
	QUERY_PROTECTEDMODEAPPINFO_RESPONSE resp = {0};

	NXCONTEXT Ctx = NULL;

	ULONG bytesret = 0;

	do 
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		req.ProcessId	= GetCurrentProcessId();
		req.ThreadId	= GetCurrentThreadId();

		ProcessIdToSessionId(req.ProcessId, &req.SessionId);

		Ctx = submit_request(Global.Section, NXRMDRV_MSG_TYPE_QUERY_PROTECTEDMODEAPPINFO, &req, sizeof(req));

		if (!Ctx)
		{
			break;
		}

		if (!wait_for_response(Ctx, Global.Section, (PVOID)&resp, sizeof(resp), &bytesret))
		{
			Ctx = NULL;

			break;
		}

		if (resp.Ack != 0)
		{
			break;
		}

		EnterCriticalSection(&Global.ActiveDocFileNameLock);

		memset(Global.ActiveDocFileName, 0 , sizeof(Global.ActiveDocFileName));

		memcpy(Global.ActiveDocFileName,
			   resp.ActiveDocFileName,
			   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(resp.ActiveDocFileName)*sizeof(WCHAR)));

		LeaveCriticalSection(&Global.ActiveDocFileNameLock);

		bRet = TRUE;

	} while (FALSE);

	return bRet;
}

static HRESULT takecare_winword_insert_file(WCHAR *FileName)
{
	HRESULT hr = S_OK;

	HRESULT hrlocal = S_OK;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvluationId = 0;

	do 
	{
		//
		// take care of rights
		//
		hrlocal = nudf::util::nxl::NxrmCheckRights(FileName, &RightsMask, &CustomRightsMask, &EvluationId);

		if (!SUCCEEDED(hrlocal))
		{
			break;
		}

		if (!(RightsMask & BUILTIN_RIGHT_DECRYPT))
		{
			hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
		}

	} while (FALSE);

	return hr;
}

static HRESULT takecare_powerpoint_saveas_more_than_one_file(WCHAR *FileName)
{
	return takecare_winword_insert_file(FileName);
}
