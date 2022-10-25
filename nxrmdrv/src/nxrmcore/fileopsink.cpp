#include "stdafx.h"
#include "nxrmcoreglobal.h"
#include "nxrmdrv.h"
#include "nxrmcorehlp.h"
#include "fileopsink.h"

#define NXL_FILEOP_FLAG_READONLY_DST	(0x00000001)
#define NXL_FILEOP_FLAG_DELETE_DST		(0x00000002)

typedef struct _FILEOPNXL_NODE
{
	WCHAR		SrcFileName[MAX_PATH];

	WCHAR		DstFileName[MAX_PATH];

	ULONG		Flags;

	LIST_ENTRY	Link;

}FILEOPNXL_NODE, *PFILEOPNXL_NODE;

#ifdef __cplusplus
extern "C" {
#endif

	extern	CORE_GLOBAL_DATA Global;

	extern	BOOL send_block_notification(const WCHAR *FileName, BLOCK_NOTIFICATION_TYPE Type);

#ifdef __cplusplus
}
#endif

CoreIFileOperationProgressSink::CoreIFileOperationProgressSink()
{
	HMODULE hOle32 = NULL;
	HMODULE	hShell32 = NULL;

	m_uRefCount = 1;

	InitializeListHead(&m_nxlFilesList);

	InitializeCriticalSection(&m_nxlFilesListLock);

	hOle32 = GetModuleHandleW(OLE32_MODULE_NAME);

	if (hOle32)
	{
		m_fn_CoTaskMemFree = (COTASKMEMFREE)GetProcAddress(hOle32, NXRMCORE_COTASKMEMFREE_PROC_NAME);
	}
	else
	{
		m_fn_CoTaskMemFree = NULL;
	}

	hShell32 = GetModuleHandleW(SHELL32_MODULE_NAME);

	if (hShell32)
	{
		m_fn_SHChangeNotify = (SHCHANGENOTIFY)GetProcAddress(hShell32, NXRMCORE_SHCHANGENOTIFY_PROC_NAME);
	}
	else
	{
		m_fn_SHChangeNotify = NULL;
	}
}

CoreIFileOperationProgressSink::~CoreIFileOperationProgressSink()
{
	LIST_ENTRY *ite = NULL;
	LIST_ENTRY *tmp = NULL;

	EnterCriticalSection(&m_nxlFilesListLock);

	FOR_EACH_LIST_SAFE(ite, tmp, &m_nxlFilesList)
	{
		FILEOPNXL_NODE *pNode = CONTAINING_RECORD(ite, FILEOPNXL_NODE, Link);

		RemoveEntryList(ite);

		free(pNode);
	}

	LeaveCriticalSection(&m_nxlFilesListLock);

	DeleteCriticalSection(&m_nxlFilesListLock);

	m_fn_CoTaskMemFree = NULL;
}

STDMETHODIMP CoreIFileOperationProgressSink::QueryInterface(__RPC__in REFIID riid, _COM_Outptr_ void **ppobj)
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
		else if (IID_IFileOperationProgressSink == riid)
		{
			punk = (IFileOperationProgressSink*)this;
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

STDMETHODIMP_(ULONG) CoreIFileOperationProgressSink::AddRef()
{
	m_uRefCount++;

	return m_uRefCount;
}

STDMETHODIMP_(ULONG) CoreIFileOperationProgressSink::Release()
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

STDMETHODIMP CoreIFileOperationProgressSink::StartOperations()
{
	HRESULT hr = S_OK;

	return hr;

}

STDMETHODIMP CoreIFileOperationProgressSink::FinishOperations(/* [in] */ HRESULT hrResult)
{
	HRESULT hr = S_OK;

	return hr;
}

STDMETHODIMP CoreIFileOperationProgressSink::PreRenameItem(/* [in] */ DWORD dwFlags,
														   /* [in] */ __RPC__in_opt IShellItem *psiItem,
														   /* [string][unique][in] */ __RPC__in_opt_string LPCWSTR pszNewName)
{
	HRESULT hr = S_OK;

	return hr;
}

STDMETHODIMP CoreIFileOperationProgressSink::PostRenameItem(/* [in] */ DWORD dwFlags,
															/* [in] */ __RPC__in_opt IShellItem *psiItem,
															/* [string][in] */ __RPC__in_string LPCWSTR pszNewName,
															/* [in] */ HRESULT hrRename,
															/* [in] */ __RPC__in_opt IShellItem *psiNewlyCreated)
{
	HRESULT hr = S_OK;

	return hr;
}

STDMETHODIMP CoreIFileOperationProgressSink::PreMoveItem(/* [in] */ DWORD dwFlags,
														 /* [in] */ __RPC__in_opt IShellItem *psiItem,
														 /* [in] */ __RPC__in_opt IShellItem *psiDestinationFolder,
														 /* [string][unique][in] */ __RPC__in_opt_string LPCWSTR pszNewName)
{
	HRESULT hr = S_OK;

	WCHAR *pSrc = NULL;
	WCHAR *pDst = NULL;

	WIN32_FILE_ATTRIBUTE_DATA	fileattrs;

	DWORD	dwSize = 0;

	WCHAR DstFileName[MAX_PATH] = {0};
	WCHAR DstNXLFileName[MAX_PATH] = {0};

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRights = 0;
	ULONGLONG EvaluatonId = 0;

	HANDLE hFile = INVALID_HANDLE_VALUE;

	FILE_BASIC_INFO BasicInfo = {0};
	FILE_END_OF_FILE_INFO EOFInfo = {0};

	LIST_ENTRY *ite = NULL;
	FILEOPNXL_NODE *pNode = NULL;

	BOOL FileCreated = FALSE;

	do 
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		if(psiItem == NULL)
		{
			break;
		}

		if(S_OK != psiItem->GetDisplayName(SIGDN_FILESYSPATH, &pSrc))
		{
			break;
		}

		if (S_OK != nudf::util::nxl::NxrmCheckRights(pSrc, &RightsMask, &CustomRights, &EvaluatonId))
		{
			break;
		}

		if (dwFlags & TSF_OVERWRITE_EXIST)
		{
			break;
		}

		//
		// check destination NXL file exist or not
		//
		if(S_OK != psiDestinationFolder->GetDisplayName(SIGDN_FILESYSPATH, &pDst))
		{
			break;
		}

		memset(&fileattrs, 0, sizeof(fileattrs));
		memset(DstFileName, 0, sizeof(DstFileName));
		memset(DstNXLFileName, 0, sizeof(DstNXLFileName));

		if(pszNewName == NULL)
		{
			WCHAR *p = NULL;

			p = wcsrchr(pSrc,L'\\');

			if(!p)
			{
				break;
			}

			if(*(pDst + wcslen(pDst) - 1) == L'\\')
			{
				//
				// skip the "\" in the source
				//
				p++;
			}

			if(*p == L'\0')
			{
				break;
			}

			if(wcslen(pDst) + wcslen(p) >= sizeof(DstNXLFileName)/sizeof(WCHAR))
			{
				break;
			}

			swprintf_s(DstNXLFileName,
					   sizeof(DstNXLFileName)/sizeof(WCHAR),
					   L"%s%s.nxl",
					   pDst,
					   p);

			swprintf_s(DstFileName,
					   sizeof(DstFileName)/sizeof(WCHAR),
					   L"%s%s",
					   pDst,
					   p);
		}
		else if(wcslen(pszNewName) == 0)
		{
			WCHAR *p = NULL;

			p = wcsrchr(pSrc, L'\\');

			if(!p)
			{
				break;
			}

			if(*(pDst + wcslen(pDst) - 1) == L'\\')
			{
				//
				// skip the "\" in the source
				//
				p++;
			}

			if(*p == L'\0')
			{
				break;
			}

			if(wcslen(pDst) + wcslen(p) >= sizeof(DstNXLFileName)/sizeof(WCHAR))
			{
				break;
			}

			swprintf_s(DstNXLFileName,
					   sizeof(DstNXLFileName)/sizeof(WCHAR),
					   L"%s%s.nxl",
					   pDst,
					   p);

			swprintf_s(DstFileName,
					   sizeof(DstFileName)/sizeof(WCHAR),
					   L"%s%s",
					   pDst,
					   p);

		}
		else
		{
			if(wcslen(pDst) + wcslen(pszNewName) >= sizeof(DstNXLFileName)/sizeof(WCHAR))
			{
				break;
			}

			swprintf_s(DstNXLFileName,
					   sizeof(DstNXLFileName)/sizeof(WCHAR),
					   L"%s\\%s.nxl",
					   pDst,
					   pszNewName);

			swprintf_s(DstFileName,
					   sizeof(DstFileName)/sizeof(WCHAR),
					   L"%s\\%s",
					   pDst,
					   pszNewName);

			if (0 == _wcsicmp(pDst, L"C:\\"))
			{
				send_block_notification(DstFileName, NxrmdrvSaveFileBlocked);

				hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
				break;
			}
		}

		if(!GetFileAttributesExW(DstNXLFileName, GetFileExInfoStandard, &fileattrs))
		{
			EnterCriticalSection(&m_nxlFilesListLock);

			FOR_EACH_LIST(ite, &m_nxlFilesList)
			{
				pNode = CONTAINING_RECORD(ite, FILEOPNXL_NODE, Link);

				if (_wcsicmp(pNode->SrcFileName, pSrc) == 0)
				{
					break;
				}
				else
				{
					pNode = NULL;
				}
			}

			if (!pNode)
			{
				pNode = (FILEOPNXL_NODE *)malloc(sizeof(FILEOPNXL_NODE));

				if (pNode)
				{
					memset(pNode, 0, sizeof(FILEOPNXL_NODE));

					memcpy(pNode->SrcFileName,
						   pSrc,
						   min(sizeof(pNode->SrcFileName) - sizeof(WCHAR), wcslen(pSrc)*sizeof(WCHAR)));

					memcpy(pNode->DstFileName,
						   DstFileName,
						   min(sizeof(pNode->DstFileName) - sizeof(WCHAR), wcslen(DstFileName)*sizeof(WCHAR)));

					InsertHeadList(&m_nxlFilesList, &pNode->Link);
				}
			}
			else
			{
				pNode->Flags &= ~(NXL_FILEOP_FLAG_DELETE_DST);
			}

			LeaveCriticalSection(&m_nxlFilesListLock);

			break;
		}

		//
		// NXL file exists
		//

		hFile = CreateFileW(DstFileName,
							GENERIC_READ|GENERIC_WRITE,
							FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
							NULL,
							OPEN_ALWAYS,
							FILE_ATTRIBUTE_NORMAL,
							NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			break;
		}

		FileCreated = (GetLastError() == ERROR_ALREADY_EXISTS) ? FALSE : TRUE;

		BasicInfo.FileAttributes			= fileattrs.dwFileAttributes;

		BasicInfo.ChangeTime.HighPart		= fileattrs.ftLastWriteTime.dwHighDateTime;
		BasicInfo.ChangeTime.LowPart		= fileattrs.ftLastWriteTime.dwLowDateTime;

		BasicInfo.CreationTime.HighPart		= fileattrs.ftCreationTime.dwHighDateTime;
		BasicInfo.CreationTime.LowPart		= fileattrs.ftCreationTime.dwLowDateTime;

		BasicInfo.LastAccessTime.HighPart	= fileattrs.ftLastAccessTime.dwHighDateTime;
		BasicInfo.LastAccessTime.LowPart	= fileattrs.ftLastAccessTime.dwLowDateTime;

		BasicInfo.LastWriteTime.QuadPart = BasicInfo.ChangeTime.QuadPart;

		SetFileInformationByHandle(hFile, FileBasicInfo, &BasicInfo, sizeof(BasicInfo));

		EOFInfo.EndOfFile.HighPart	= fileattrs.nFileSizeHigh;
		EOFInfo.EndOfFile.LowPart	= fileattrs.nFileSizeLow;

		SetFileInformationByHandle(hFile, FileEndOfFileInfo, &EOFInfo, sizeof(EOFInfo));

		EnterCriticalSection(&m_nxlFilesListLock);

		FOR_EACH_LIST(ite, &m_nxlFilesList)
		{
			pNode = CONTAINING_RECORD(ite, FILEOPNXL_NODE, Link);

			if (_wcsicmp(pNode->SrcFileName, pSrc) == 0)
			{
				break;
			}
			else
			{
				pNode = NULL;
			}
		}

		if (!pNode)
		{
			pNode = (FILEOPNXL_NODE *)malloc(sizeof(FILEOPNXL_NODE));

			if (pNode)
			{
				memset(pNode, 0, sizeof(FILEOPNXL_NODE));

				pNode->Flags |= (FileCreated ? NXL_FILEOP_FLAG_DELETE_DST : 0);

				if (fileattrs.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
				{
					pNode->Flags |= NXL_FILEOP_FLAG_READONLY_DST;
				}

				memcpy(pNode->SrcFileName,
					   pSrc,
					   min(sizeof(pNode->SrcFileName) - sizeof(WCHAR), wcslen(pSrc)*sizeof(WCHAR)));

				memcpy(pNode->DstFileName,
					   DstFileName,
					   min(sizeof(pNode->DstFileName) - sizeof(WCHAR), wcslen(DstFileName)*sizeof(WCHAR)));

				InsertHeadList(&m_nxlFilesList, &pNode->Link);
			}
		}
		else
		{
			pNode->Flags |= (FileCreated ? NXL_FILEOP_FLAG_DELETE_DST : 0);

			if (fileattrs.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
			{
				pNode->Flags |= NXL_FILEOP_FLAG_READONLY_DST;
			}
		}

		LeaveCriticalSection(&m_nxlFilesListLock);

	} while (FALSE);

	if (pSrc && m_fn_CoTaskMemFree)
	{
		m_fn_CoTaskMemFree(pSrc);
		pSrc = NULL;
	}

	if (pDst && m_fn_CoTaskMemFree)
	{
		m_fn_CoTaskMemFree(pDst);
		pDst = NULL;
	}

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	return hr;
}

STDMETHODIMP CoreIFileOperationProgressSink::PostMoveItem(/* [in] */ DWORD dwFlags,
														  /* [in] */ __RPC__in_opt IShellItem *psiItem,
														  /* [in] */ __RPC__in_opt IShellItem *psiDestinationFolder,
														  /* [string][unique][in] */ __RPC__in_opt_string LPCWSTR pszNewName,
														  /* [in] */ HRESULT hrMove,
														  /* [in] */ __RPC__in_opt IShellItem *psiNewlyCreated)
{
	HRESULT hr = S_OK;

	WCHAR *pSrc = NULL;
	WCHAR *pDst = NULL;

	WCHAR NXLFileName[MAX_PATH] = {0};

	LIST_ENTRY *ite = NULL;

	FILEOPNXL_NODE *pNode = NULL;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRights = 0;
	ULONGLONG EvaluationId = 0;

	do 
	{
		if (!psiItem)
		{
			break;
		}

		if(S_OK != psiItem->GetDisplayName(SIGDN_FILESYSPATH, &pSrc))
		{
			break;
		}

		EnterCriticalSection(&m_nxlFilesListLock);

		FOR_EACH_LIST(ite, &m_nxlFilesList)
		{
			pNode = CONTAINING_RECORD(ite, FILEOPNXL_NODE, Link);

			//
			// Possible hrMove values are:	COPYENGINE_E_USER_CANCELLED
			//								COPYENGINE_S_PENDING
			//								COPYENGINE_S_USER_IGNORED
			//								S_OK
			//

			if (_wcsicmp(pNode->SrcFileName, pSrc) == 0 && hrMove != COPYENGINE_S_PENDING)
			{
				RemoveEntryList(ite);
				break;
			}
			else
			{
				pNode = NULL;
			}
		}

		LeaveCriticalSection(&m_nxlFilesListLock);

		if (!pNode)
		{
			//
			// source file is not NXL file or fail to copy
			//
			break;
		}

		if (!SUCCEEDED(hrMove))
		{
			break;
		}

		if (psiNewlyCreated == NULL)
		{
			break;
		}

		if (S_OK != psiNewlyCreated->GetDisplayName(SIGDN_FILESYSPATH, &pDst))
		{
			break;
		}

		swprintf_s(NXLFileName, 
				   sizeof(NXLFileName)/sizeof(WCHAR),
				   L"%s.nxl",
				   pDst);

		MoveFileExW(pDst,
					NXLFileName,
					MOVEFILE_REPLACE_EXISTING);

		if (m_fn_SHChangeNotify)
		{
			m_fn_SHChangeNotify(SHCNE_RENAMEITEM,
								SHCNF_PATH,
								pDst,
								NXLFileName);

			m_fn_SHChangeNotify(SHCNE_UPDATEITEM,
								SHCNF_PATH,
								pDst,
								NULL);
		}

	} while (FALSE);

	if (pNode)
	{
		if (pNode->Flags & NXL_FILEOP_FLAG_DELETE_DST)
		{
			if (pNode->Flags & NXL_FILEOP_FLAG_READONLY_DST)
			{
				SetFileAttributesW(pNode->DstFileName, FILE_ATTRIBUTE_NORMAL);
			}

			if (S_OK == nudf::util::nxl::NxrmCheckRights(pSrc, &RightsMask, &CustomRights, &EvaluationId))
			{
				DeleteFileW(pNode->DstFileName);

				if (m_fn_SHChangeNotify)
				{
					m_fn_SHChangeNotify(SHCNE_DELETE,
										SHCNF_PATH,
										pNode->DstFileName,
										NULL);

				}
			}
		}

		free(pNode);

		pNode = NULL;
	}

	if (pSrc && m_fn_CoTaskMemFree)
	{
		m_fn_CoTaskMemFree(pSrc);
		pSrc = NULL;
	}

	if (pDst && m_fn_CoTaskMemFree)
	{
		m_fn_CoTaskMemFree(pDst);
		pDst = NULL;
	}

	return hr;
}

STDMETHODIMP CoreIFileOperationProgressSink::PreCopyItem(/* [in] */ DWORD dwFlags,
														 /* [in] */ __RPC__in_opt IShellItem *psiItem,
														 /* [in] */ __RPC__in_opt IShellItem *psiDestinationFolder,
														 /* [string][unique][in] */ __RPC__in_opt_string LPCWSTR pszNewName)
{
	HRESULT hr = S_OK;

	WCHAR *pSrc = NULL;
	WCHAR *pDst = NULL;

	WIN32_FILE_ATTRIBUTE_DATA	fileattrs;

	DWORD	dwSize = 0;

	WCHAR DstFileName[MAX_PATH] = {0};
	WCHAR DstNXLFileName[MAX_PATH] = {0};

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRights = 0;
	ULONGLONG EvaluationId = 0;

	HANDLE hFile = INVALID_HANDLE_VALUE;

	FILE_BASIC_INFO BasicInfo = {0};
	FILE_END_OF_FILE_INFO EOFInfo = {0};

	LIST_ENTRY *ite = NULL;
	FILEOPNXL_NODE *pNode = NULL;
	
	BOOL FileCreated = FALSE;

	do 
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		if(psiItem == NULL)
		{
			break;
		}

		if(S_OK != psiItem->GetDisplayName(SIGDN_FILESYSPATH, &pSrc))
		{
			break;
		}

		if (S_OK != nudf::util::nxl::NxrmCheckRights(pSrc, &RightsMask, &CustomRights, &EvaluationId))
		{
			break;
		}
		
		if (dwFlags & TSF_OVERWRITE_EXIST)
		{
			break;
		}

		//
		// check destination NXL file exist or not
		//
		if(S_OK != psiDestinationFolder->GetDisplayName(SIGDN_FILESYSPATH, &pDst))
		{
			break;
		}

		memset(&fileattrs, 0, sizeof(fileattrs));
		memset(DstFileName, 0, sizeof(DstFileName));
		memset(DstNXLFileName, 0, sizeof(DstNXLFileName));

		if(pszNewName == NULL)
		{
			WCHAR *p = NULL;

			p = wcsrchr(pSrc,L'\\');

			if(!p)
			{
				break;
			}

			if(*(pDst + wcslen(pDst) - 1) == L'\\')
			{
				//
				// skip the "\" in the source
				//
				p++;
			}

			if(*p == L'\0')
			{
				break;
			}

			if(wcslen(pDst) + wcslen(p) >= sizeof(DstNXLFileName)/sizeof(WCHAR))
			{
				break;
			}

			swprintf_s(DstNXLFileName,
					   sizeof(DstNXLFileName)/sizeof(WCHAR),
					   L"%s%s.nxl",
					   pDst,
					   p);

			swprintf_s(DstFileName,
					   sizeof(DstFileName)/sizeof(WCHAR),
					   L"%s%s",
					   pDst,
					   p);
		}
		else if(wcslen(pszNewName) == 0)
		{
			WCHAR *p = NULL;

			p = wcsrchr(pSrc, L'\\');

			if(!p)
			{
				break;
			}

			if(*(pDst + wcslen(pDst) - 1) == L'\\')
			{
				//
				// skip the "\" in the source
				//
				p++;
			}

			if(*p == L'\0')
			{
				break;
			}

			if(wcslen(pDst) + wcslen(p) >= sizeof(DstNXLFileName)/sizeof(WCHAR))
			{
				break;
			}

			swprintf_s(DstNXLFileName,
					   sizeof(DstNXLFileName)/sizeof(WCHAR),
					   L"%s%s.nxl",
					   pDst,
					   p);

			swprintf_s(DstFileName,
					   sizeof(DstFileName)/sizeof(WCHAR),
					   L"%s%s",
					   pDst,
					   p);

			if (0 == _wcsicmp(pDst, L"C:\\"))
			{
				send_block_notification(DstFileName, NxrmdrvSaveFileBlocked);

				hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
				break;
			}
		}
		else
		{
			if(wcslen(pDst) + wcslen(pszNewName) >= sizeof(DstNXLFileName)/sizeof(WCHAR))
			{
				break;
			}

			swprintf_s(DstNXLFileName,
					   sizeof(DstNXLFileName)/sizeof(WCHAR),
					   L"%s\\%s.nxl",
					   pDst,
					   pszNewName);

			swprintf_s(DstFileName,
					   sizeof(DstFileName)/sizeof(WCHAR),
					   L"%s\\%s",
					   pDst,
					   pszNewName);
		}

		if(!GetFileAttributesExW(DstNXLFileName, GetFileExInfoStandard, &fileattrs))
		{
			EnterCriticalSection(&m_nxlFilesListLock);

			FOR_EACH_LIST(ite, &m_nxlFilesList)
			{
				pNode = CONTAINING_RECORD(ite, FILEOPNXL_NODE, Link);

				if (_wcsicmp(pNode->SrcFileName, pSrc) == 0)
				{
					break;
				}
				else
				{
					pNode = NULL;
				}
			}

			if (!pNode)
			{
				pNode = (FILEOPNXL_NODE *)malloc(sizeof(FILEOPNXL_NODE));

				if (pNode)
				{
					memset(pNode, 0, sizeof(FILEOPNXL_NODE));

					memcpy(pNode->SrcFileName,
						   pSrc,
						   min(sizeof(pNode->SrcFileName) - sizeof(WCHAR), wcslen(pSrc)*sizeof(WCHAR)));

					memcpy(pNode->DstFileName,
						   DstFileName,
						   min(sizeof(pNode->DstFileName) - sizeof(WCHAR), wcslen(DstFileName)*sizeof(WCHAR)));

					InsertHeadList(&m_nxlFilesList, &pNode->Link);
				}
			}
			else
			{
				pNode->Flags &= ~(NXL_FILEOP_FLAG_DELETE_DST);
			}

			LeaveCriticalSection(&m_nxlFilesListLock);

			break;
		}

		//
		// NXL file exists
		//
		
		hFile = CreateFileW(DstFileName,
							GENERIC_READ|GENERIC_WRITE,
							FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
							NULL,
							OPEN_ALWAYS,
							FILE_ATTRIBUTE_NORMAL,
							NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			break;
		}

		FileCreated = (GetLastError() == ERROR_ALREADY_EXISTS) ? FALSE : TRUE;

		BasicInfo.FileAttributes			= fileattrs.dwFileAttributes;

		BasicInfo.ChangeTime.HighPart		= fileattrs.ftLastWriteTime.dwHighDateTime;
		BasicInfo.ChangeTime.LowPart		= fileattrs.ftLastWriteTime.dwLowDateTime;
		
		BasicInfo.CreationTime.HighPart		= fileattrs.ftCreationTime.dwHighDateTime;
		BasicInfo.CreationTime.LowPart		= fileattrs.ftCreationTime.dwLowDateTime;

		BasicInfo.LastAccessTime.HighPart	= fileattrs.ftLastAccessTime.dwHighDateTime;
		BasicInfo.LastAccessTime.LowPart	= fileattrs.ftLastAccessTime.dwLowDateTime;

		BasicInfo.LastWriteTime.QuadPart = BasicInfo.ChangeTime.QuadPart;

		SetFileInformationByHandle(hFile, FileBasicInfo, &BasicInfo, sizeof(BasicInfo));

		EOFInfo.EndOfFile.HighPart	= fileattrs.nFileSizeHigh;
		EOFInfo.EndOfFile.LowPart	= fileattrs.nFileSizeLow;

		SetFileInformationByHandle(hFile, FileEndOfFileInfo, &EOFInfo, sizeof(EOFInfo));

		EnterCriticalSection(&m_nxlFilesListLock);

		FOR_EACH_LIST(ite, &m_nxlFilesList)
		{
			 pNode = CONTAINING_RECORD(ite, FILEOPNXL_NODE, Link);

			 if (_wcsicmp(pNode->SrcFileName, pSrc) == 0)
			 {
				 break;
			 }
			 else
			 {
				 pNode = NULL;
			 }
		}

		if (!pNode)
		{
			pNode = (FILEOPNXL_NODE *)malloc(sizeof(FILEOPNXL_NODE));

			if (pNode)
			{
				memset(pNode, 0, sizeof(FILEOPNXL_NODE));

				pNode->Flags |= (FileCreated ? NXL_FILEOP_FLAG_DELETE_DST : 0);

				if (fileattrs.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
				{
					pNode->Flags |= NXL_FILEOP_FLAG_READONLY_DST;
				}

				memcpy(pNode->SrcFileName,
					   pSrc,
					   min(sizeof(pNode->SrcFileName) - sizeof(WCHAR), wcslen(pSrc)*sizeof(WCHAR)));

				memcpy(pNode->DstFileName,
					   DstFileName,
					   min(sizeof(pNode->DstFileName) - sizeof(WCHAR), wcslen(DstFileName)*sizeof(WCHAR)));

				InsertHeadList(&m_nxlFilesList, &pNode->Link);
			}
		}
		else
		{
			pNode->Flags |= (FileCreated ? NXL_FILEOP_FLAG_DELETE_DST : 0);

			if (fileattrs.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
			{
				pNode->Flags |= NXL_FILEOP_FLAG_READONLY_DST;
			}
		}

		LeaveCriticalSection(&m_nxlFilesListLock);

	} while (FALSE);

	if (pSrc && m_fn_CoTaskMemFree)
	{
		m_fn_CoTaskMemFree(pSrc);
		pSrc = NULL;
	}

	if (pDst && m_fn_CoTaskMemFree)
	{
		m_fn_CoTaskMemFree(pDst);
		pDst = NULL;
	}

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	return hr;
}

STDMETHODIMP CoreIFileOperationProgressSink::PostCopyItem(/* [in] */ DWORD dwFlags,
														  /* [in] */ __RPC__in_opt IShellItem *psiItem,
														  /* [in] */ __RPC__in_opt IShellItem *psiDestinationFolder,
														  /* [string][unique][in] */ __RPC__in_opt_string LPCWSTR pszNewName,
														  /* [in] */ HRESULT hrCopy,
														  /* [in] */ __RPC__in_opt IShellItem *psiNewlyCreated)
{
	HRESULT hr = S_OK;

	WCHAR *pSrc = NULL;
	WCHAR *pDst = NULL;

	WCHAR NXLFileName[MAX_PATH] = {0};

	LIST_ENTRY *ite = NULL;

	FILEOPNXL_NODE *pNode = NULL;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRights = 0;
	ULONGLONG EvaluationId = 0;

	do 
	{
		if (!psiItem)
		{
			break;
		}

		if(S_OK != psiItem->GetDisplayName(SIGDN_FILESYSPATH, &pSrc))
		{
			break;
		}

		EnterCriticalSection(&m_nxlFilesListLock);

		FOR_EACH_LIST(ite, &m_nxlFilesList)
		{
			pNode = CONTAINING_RECORD(ite, FILEOPNXL_NODE, Link);
			
			//
			// Possible hrCopy values are:	COPYENGINE_E_USER_CANCELLED
			//								COPYENGINE_S_PENDING
			//								COPYENGINE_S_USER_IGNORED
			//								S_OK
			//
			if (_wcsicmp(pNode->SrcFileName, pSrc) == 0 && hrCopy != COPYENGINE_S_PENDING)
			{
				RemoveEntryList(ite);
				break;
			}
			else
			{
				pNode = NULL;
			}
		}

		LeaveCriticalSection(&m_nxlFilesListLock);

		if (!pNode)
		{
			//
			// source file is not NXL file or fail to copy
			//
			break;
		}

		if (!SUCCEEDED(hrCopy))
		{
			break;
		}
		
		if (psiNewlyCreated == NULL)
		{
			break;
		}

		if (S_OK != psiNewlyCreated->GetDisplayName(SIGDN_FILESYSPATH, &pDst))
		{
			break;
		}

		swprintf_s(NXLFileName, 
				   sizeof(NXLFileName)/sizeof(WCHAR),
				   L"%s.nxl",
				   pDst);

		MoveFileExW(pDst,
					NXLFileName,
					MOVEFILE_REPLACE_EXISTING);

		if (m_fn_SHChangeNotify)
		{
			m_fn_SHChangeNotify(SHCNE_RENAMEITEM,
								SHCNF_PATH,
								pDst,
								NXLFileName);

			m_fn_SHChangeNotify(SHCNE_UPDATEITEM,
								SHCNF_PATH,
								pDst,
								NULL);
		}


	} while (FALSE);

	if (pNode)
	{
		if (pNode->Flags & NXL_FILEOP_FLAG_DELETE_DST)
		{
			if (pNode->Flags & NXL_FILEOP_FLAG_READONLY_DST)
			{
				SetFileAttributesW(pNode->DstFileName, FILE_ATTRIBUTE_NORMAL);
			}

			if (S_OK == nudf::util::nxl::NxrmCheckRights(pSrc, &RightsMask, &CustomRights, &EvaluationId))
			{
				DeleteFileW(pNode->DstFileName);

				if (m_fn_SHChangeNotify)
				{
					m_fn_SHChangeNotify(SHCNE_DELETE,
										SHCNF_PATH,
										pNode->DstFileName,
										NULL);
				}
			}
		}

		free(pNode);

		pNode = NULL;
	}

	if (pSrc && m_fn_CoTaskMemFree)
	{
		m_fn_CoTaskMemFree(pSrc);
		pSrc = NULL;
	}

	if (pDst && m_fn_CoTaskMemFree)
	{
		m_fn_CoTaskMemFree(pDst);
		pDst = NULL;
	}

	return hr;
}

STDMETHODIMP CoreIFileOperationProgressSink::PreDeleteItem(/* [in] */ DWORD dwFlags,
														   /* [in] */ __RPC__in_opt IShellItem *psiItem)
{
	HRESULT hr = S_OK;

	return hr;

}

STDMETHODIMP CoreIFileOperationProgressSink::PostDeleteItem(/* [in] */ DWORD dwFlags,
															/* [in] */ __RPC__in_opt IShellItem *psiItem,
															/* [in] */ HRESULT hrDelete,
															/* [in] */ __RPC__in_opt IShellItem *psiNewlyCreated)
{
	HRESULT hr = S_OK;

	return hr;

}

STDMETHODIMP CoreIFileOperationProgressSink::PreNewItem(/* [in] */ DWORD dwFlags,
														/* [in] */ __RPC__in_opt IShellItem *psiDestinationFolder,
														/* [string][unique][in] */ __RPC__in_opt_string LPCWSTR pszNewName)
{
	HRESULT hr = S_OK;

	return hr;

}

STDMETHODIMP CoreIFileOperationProgressSink::PostNewItem(/* [in] */ DWORD dwFlags,
														 /* [in] */ __RPC__in_opt IShellItem *psiDestinationFolder,
														 /* [string][unique][in] */ __RPC__in_opt_string LPCWSTR pszNewName,
														 /* [string][unique][in] */ __RPC__in_opt_string LPCWSTR pszTemplateName,
														 /* [in] */ DWORD dwFileAttributes,
														 /* [in] */ HRESULT hrNew,
														 /* [in] */ __RPC__in_opt IShellItem *psiNewItem)
{
	HRESULT hr = S_OK;

	return hr;

}

STDMETHODIMP CoreIFileOperationProgressSink::UpdateProgress(/* [in] */ UINT iWorkTotal,
															/* [in] */ UINT iWorkSoFar)
{
	HRESULT hr = S_OK;

	return hr;

}

STDMETHODIMP CoreIFileOperationProgressSink::ResetTimer()
{
	HRESULT hr = S_OK;

	return hr;

}

STDMETHODIMP CoreIFileOperationProgressSink::PauseTimer()
{
	HRESULT hr = S_OK;

	return hr;

}

STDMETHODIMP CoreIFileOperationProgressSink::ResumeTimer()
{
	HRESULT hr = S_OK;

	return hr;
}
