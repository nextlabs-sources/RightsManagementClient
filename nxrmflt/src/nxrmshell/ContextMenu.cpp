#include "stdafx.h"
#include "ContextMenu.h"
#include "resource.h"
#include "nxrmflt.h"
#include <winternl.h>
#include "nxrmshellglobal.h"
#include "nxrmdrv.h"
#include "nxrmcorehlp.h"
#include "nxrmdrvman.h"

#include "zip.h"

#include <nudf\resutil.hpp>
#include <nudf\nxrmres.h>
#include <nudf\nxlfile.hpp>
#include <nudf\web\json.hpp>

#ifdef __cplusplus
extern "C" {
#endif

	extern SHELL_GLOBAL_DATA Global;

	extern 	BOOL init_rm_section_safe(void);

#ifdef __cplusplus
}
#endif

#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)

typedef NTSTATUS (WINAPI *ZWQUERYEAFILE) (
	_In_ HANDLE FileHandle,
	_Out_ PIO_STATUS_BLOCK IoStatusBlock,
	_Out_writes_bytes_(Length) PVOID Buffer,
	_In_ ULONG Length,
	_In_ BOOLEAN ReturnSingleEntry,
	_In_reads_bytes_opt_(EaListLength) PVOID EaList,
	_In_ ULONG EaListLength,
	_In_opt_ PULONG EaIndex,
	_In_ BOOLEAN RestartScan
	);

typedef NTSTATUS (WINAPI *ZWSETEAFILE) (
	_In_ HANDLE FileHandle,
	_Out_ PIO_STATUS_BLOCK IoStatusBlock,
	_In_reads_bytes_(Length) PVOID Buffer,
	_In_ ULONG Length
	);


//// {B4B506CC-112F-4739-B93B-F4F3869B6E3E}
extern "C" const GUID CLSID_IRmCtxMenu =	{0xb4b506cc, 0x112f, 0x4739, {0xb9, 0x3b, 0xf4, 0xf3, 0x86, 0x9b, 0x6e, 0x3e}};

static const WCHAR MenuGroupName[] = L"NextLabs";


static BOOL get_classify_ui(WCHAR *FileName, WCHAR *GroupName);
static BOOL check_enable_protect_menu(void);
static void PairToBuffer(_In_ const std::vector<std::pair<std::wstring,std::wstring>>& pairs, _Out_ std::vector<WCHAR>& buf);
static void BufferToPair(_In_ LPCWSTR buf, _Out_ std::vector<std::pair<std::wstring,std::wstring>>& pairs);
static BOOL IsRemoteEvaluation(const WCHAR *FileName);
static BOOL send_block_notification(const WCHAR *FileName, BLOCK_NOTIFICATION_TYPE Type);

static HBITMAP BitmapFromIcon(HICON hIcon)
{
	ICONINFO IconInfo = {0};
	HBITMAP hBitmap = NULL;

	do 
	{
		if (!hIcon)
		{
			break;
		}

		if (!GetIconInfo(hIcon, &IconInfo))
		{
			break;
		}

		hBitmap = (HBITMAP)CopyImage(IconInfo.hbmColor, IMAGE_BITMAP, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);

	} while (FALSE);

	return hBitmap;
}

BOOL IsFilteringDriverFunction(void);

IRmCtxMenu::IRmCtxMenu()
{
	WCHAR module_path[MAX_PATH] = {0};

	ULONG module_path_length = 0;

	WCHAR *p = NULL;

	m_uRefCount = 1;
	m_pwsSelectedFile = NULL;
	m_nSelectedFileCount = 0;
	m_bIsNXLFile = FALSE;
	m_bHasDecryptionRights = FALSE;
	m_bHasClassifyRight = FALSE;
	m_bShowProtectMenu = TRUE;
	m_hMainIcon = LoadIconW(Global.hModule, MAKEINTRESOURCEW(IDI_ICON_MAIN));
	m_hMainBitmap = BitmapFromIcon(m_hMainIcon);
	
	module_path_length = GetModuleFileNameW(Global.hModule, module_path, (sizeof(module_path)/sizeof(WCHAR) - 1));

	if (module_path_length)
	{
		p = wcsstr(module_path, NXRMSHELL_DLL_NAME);

		if (p && (sizeof(NXRMSHELL_DLL_NAME) >= sizeof(NXRMCOREUI_CM_UI_DLL_NAME)))
		{
			memcpy(p, NXRMCOREUI_CM_UI_DLL_NAME, sizeof(NXRMCOREUI_CM_UI_DLL_NAME));

			m_pRmUIObj = new nudf::util::CRmuObject;

			if (m_pRmUIObj)
			{
				m_pRmUIObj->Initialize(module_path);
				m_pRmUIObj->RmuInitialize();
			}
		}
	}

	m_pBypassedFilter = new std::tr1::wregex(BYPASSFILTERREGEX, std::tr1::regex_constants::icase);

	m_pCtxMenuFilter = NULL;

	m_lastPolicySN = 0;

    // Load resource
    LoadStringResource();
}

IRmCtxMenu::~IRmCtxMenu()
{
	if (m_pwsSelectedFile)
	{
		free(m_pwsSelectedFile);
	}

	if (m_hMainIcon)
	{
		DestroyIcon(m_hMainIcon);
		m_hMainIcon = NULL;
	}

	if (m_hMainBitmap)
	{
		DeleteObject(m_hMainBitmap);
		m_hMainIcon = NULL;
	}

	if (m_pRmUIObj)
	{
		m_pRmUIObj->RmuDeinitialize();
		m_pRmUIObj->Clear();

		delete m_pRmUIObj;
		m_pRmUIObj = NULL;
	}

	if (m_pBypassedFilter)
	{
		delete m_pBypassedFilter;
		m_pBypassedFilter = NULL;
	}

	if (m_pCtxMenuFilter)
	{
		delete m_pCtxMenuFilter;
		m_pCtxMenuFilter = NULL;
	}
}

void IRmCtxMenu::LoadStringResource()
{
    WCHAR wzResDll[MAX_PATH+1] = {0};
    std::wstring wsMenuText;

    GetModuleFileNameW(Global.hModule, wzResDll, MAX_PATH);
    WCHAR* pos = wcsrchr(wzResDll, L'\\');
    if(pos == NULL) {
        SetDefaultStringResource();
        return;
    }

    *(pos+1) = L'\0';
    wcsncat_s(wzResDll, MAX_PATH, L"nxrmres.dll", _TRUNCATE);

    HMODULE hRes = ::LoadLibraryW(wzResDll);
    if(NULL == hRes) {
        SetDefaultStringResource();
        return;
    }

    wsMenuText = nudf::util::res::LoadMessage(hRes, IDS_MENU_PROTECT, 256, LANG_NEUTRAL, L"Protect...");
    m_MenuTextW.push_back(wsMenuText);
    m_MenuTextA.push_back(std::string(wsMenuText.begin(), wsMenuText.end()));
    wsMenuText = nudf::util::res::LoadMessage(hRes, IDS_MENU_REMOVEPROTECT, 256, LANG_NEUTRAL, L"Remove Protection...");
    m_MenuTextW.push_back(wsMenuText);
    m_MenuTextA.push_back(std::string(wsMenuText.begin(), wsMenuText.end()));
	wsMenuText = nudf::util::res::LoadMessage(hRes, IDS_MENU_ZIP, 256, LANG_NEUTRAL, L"Add to ");
	m_MenuTextW.push_back(wsMenuText);
	m_MenuTextA.push_back(std::string(wsMenuText.begin(), wsMenuText.end()));
	wsMenuText = nudf::util::res::LoadMessage(hRes, IDS_MENU_CHECKPERMISSION, 256, LANG_NEUTRAL, L"Check Permission...");
    m_MenuTextW.push_back(wsMenuText);
    m_MenuTextA.push_back(std::string(wsMenuText.begin(), wsMenuText.end()));
    wsMenuText = nudf::util::res::LoadMessage(hRes, IDS_MENU_CLASSIFY, 256, LANG_NEUTRAL, L"Classify");
    m_MenuTextW.push_back(wsMenuText);
    m_MenuTextA.push_back(std::string(wsMenuText.begin(), wsMenuText.end()));
    wsMenuText = nudf::util::res::LoadMessage(hRes, IDS_MENU_HELP, 256, LANG_NEUTRAL, L"Help");
    m_MenuTextW.push_back(wsMenuText);
    m_MenuTextA.push_back(std::string(wsMenuText.begin(), wsMenuText.end()));

    ::FreeLibrary(hRes);
    hRes = NULL;
}

void IRmCtxMenu::SetDefaultStringResource()
{
    // Unicode
    m_MenuTextW.push_back(L"Protect...");
    m_MenuTextW.push_back(L"Remove Protection...");
	m_MenuTextW.push_back(L"Add to ");
    m_MenuTextW.push_back(L"Check Permission...");
    m_MenuTextW.push_back(L"Classify");
    m_MenuTextW.push_back(L"Help");
    // Ansi
    m_MenuTextA.push_back("Protect...");
    m_MenuTextA.push_back("Remove Protection...");
	m_MenuTextA.push_back("Add to ");
    m_MenuTextA.push_back("Check Permission...");
    m_MenuTextA.push_back("Classify");
    m_MenuTextA.push_back("Help");
}

STDMETHODIMP IRmCtxMenu::QueryInterface(REFIID riid, void **ppobj)
{
	HRESULT hRet = S_OK;

	void *punk = NULL;

	*ppobj = NULL;

	do 
	{
		if(IID_IUnknown == riid || IID_IShellExtInit == riid) 
		{
			punk = (IShellExtInit *)this;
		}
		else if (IID_IContextMenu == riid)
		{
			punk = (IContextMenu*)this;
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

STDMETHODIMP_(ULONG) IRmCtxMenu::AddRef()
{
	m_uRefCount++;

	return m_uRefCount;
}

STDMETHODIMP_(ULONG) IRmCtxMenu::Release()
{
	ULONG uCount = 0;

	if(m_uRefCount)
		m_uRefCount--;

	uCount = m_uRefCount;

	if(!uCount)
	{
		delete this;
		InterlockedDecrement(&Global.ContextMenuInstanceCount);
	}

	return uCount;
}

STDMETHODIMP IRmCtxMenu::GetCommandString(_In_ UINT_PTR idCmd, _In_ UINT uType, _Reserved_ UINT *pReserved, _Out_writes_bytes_((uType & GCS_UNICODE) ? (cchMax * sizeof(wchar_t)) : cchMax) _When_(!(uType & (GCS_VALIDATEA | GCS_VALIDATEW)), _Null_terminated_) CHAR *pszName, _In_ UINT cchMax)
{
	HRESULT hr = S_OK;

	do 
	{
		if (uType & GCS_HELPTEXT)
		{
			if (idCmd >= MAX_MENU_ITEM)
			{
				hr = E_INVALIDARG;
				break;
			}

			if (uType & GCS_UNICODE)
			{
                wcsncpy_s((LPWCH)pszName, cchMax, m_MenuTextW[idCmd].c_str(), _TRUNCATE);
			}
			else
			{
                strncpy_s(pszName, cchMax, m_MenuTextA[idCmd].c_str(), _TRUNCATE);
			}
		}
		else
		{
			hr = E_INVALIDARG;
		}

	} while (FALSE);

	return hr;
}

STDMETHODIMP IRmCtxMenu::InvokeCommand(_In_  CMINVOKECOMMANDINFO *pici)
{
	HRESULT hr = S_OK;
	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	WCHAR ClassifyUIXMLFileName[MAX_PATH] = {0};
	WCHAR GroupName[MAX_PATH] = {0};

	std::vector<std::pair<std::wstring,std::wstring>> tag;

	std::vector<WCHAR> wzTag;

	HANDLE hFile = INVALID_HANDLE_VALUE;

	HWND hWnd = GetActiveWindow();

	UCHAR *pTagData = NULL;

	USHORT TagDataLength = 0;

	do 
	{
		// Verb is a string? ignore this
		if (0 != HIWORD(pici->lpVerb)) 
		{
			hr = E_INVALIDARG;
			break;
		}

		if (m_pwsSelectedFile == NULL) 
		{
			hr = E_INVALIDARG;
			break;
		}

		switch (LOWORD(pici->lpVerb))
		{
		case CmdProtect:
			
			do 
			{

				if (!get_classify_ui(ClassifyUIXMLFileName, GroupName))
				{
					break;
				}

				hr = m_pRmUIObj->RmuShowClassifyDialogEx(hWnd, m_pwsSelectedFile, ClassifyUIXMLFileName, GroupName, GetUserDefaultLangID(), FALSE, tag);

				if (hr != S_OK)
				{
					break;
				}

				PairToBuffer(tag, wzTag);

				hr = nudf::util::nxl::NxrmEncryptFileEx(m_pwsSelectedFile, NXL_SECTION_TAGS, wzTag.empty() ? NULL : (UCHAR*)(&wzTag[0]), (USHORT)(wzTag.size() * sizeof(WCHAR)));
				
			} while (FALSE);
			
			break;
		case CmdUnprotect:

			do 
			{
				for (int i = 0; i < (int)m_nSelectedFileCount; i++) {
					WCHAR * pBuffer = m_pwsSelectedFile + (MAX_PATH + 1) * i;
					if (m_bMultiFileSelected) {//for single file selection. no .nxl extension attached.
						WCHAR * pfind;
						pfind = wcsstr(pBuffer, L".nxl");
						if (pfind && pfind[4] == 0) {
							*pfind = 0;//remove .nxl extension.
						}
						else {
							continue;//skip the non nxl file.
						}
					}
					if (!IsRemoteEvaluation(pBuffer))
					{
						hr = nudf::util::nxl::NxrmCheckRights(pBuffer, &RightsMask, &CustomRightsMask, &EvaluationId);
					}
					else
					{
						hr = nudf::util::nxl::NxrmCheckRightsNoneCache(pBuffer, &RightsMask, &CustomRightsMask, &EvaluationId);
					}

					if (!(RightsMask & BUILTIN_RIGHT_DECRYPT))
					{
						send_block_notification(pBuffer, NxrmdrvNotAuthorized);
					}
					else {
						hr = nudf::util::nxl::NxrmDecryptFile(pBuffer);

						if (SUCCEEDED(hr))
						{
							SHChangeNotify(SHCNE_UPDATEITEM,
								SHCNF_PATH,
								pBuffer,
								NULL);
						}
					}

				}
			} while (FALSE);

			break;
		case CmdZip:
			do {
				HZIP hz;
				std::wstring zipfile;
				if (m_bFolder) {
					zipfile = GetArchivePath((WCHAR *)GetParentDir(m_pwsSelectedFile).c_str());
				}
				else {
					zipfile = GetArchivePath(m_pwsSelectedFile);
				}
				hz = CreateZip(zipfile.c_str(), 0);
				WCHAR * pBuffer = m_pwsSelectedFile;
				if (!m_bMultiFileSelected) {//only one nxl file is selected.
					wcscat_s(pBuffer, MAX_PATH, L".nxl");
				}
				for (ULONG i = 0; i < m_nSelectedFileCount; i++) {
					pBuffer = m_pwsSelectedFile + (MAX_PATH + 1) * i;
					std::wstring filename = GetFileName(pBuffer);
					ZipAdd(hz, filename.c_str(), pBuffer);
				}
				CloseZip(hz);
			} while (FALSE);
			break;
		case CmdCheckPermission:

			if (!IsRemoteEvaluation(m_pwsSelectedFile))
			{
				hr = nudf::util::nxl::NxrmCheckRights(m_pwsSelectedFile, &RightsMask, &CustomRightsMask, &EvaluationId);
			}
			else
			{
				hr = nudf::util::nxl::NxrmCheckRightsNoneCache(m_pwsSelectedFile, &RightsMask, &CustomRightsMask, &EvaluationId);
			}

			m_pRmUIObj->RmuShowDetailsPropPageSimple(hWnd, m_pwsSelectedFile, RightsMask);

			break;
		case CmdClassify:

			do
			{
				if (!get_classify_ui(ClassifyUIXMLFileName, GroupName))
				{
					break;
				}

				hFile = CreateFileW(m_pwsSelectedFile,
									GENERIC_READ|FILE_WRITE_EA,
									FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
									NULL,
									OPEN_EXISTING,
									FILE_ATTRIBUTE_NORMAL,
									NULL);

				if (hFile == INVALID_HANDLE_VALUE)
				{
					break;
				}

				pTagData = (UCHAR*)malloc(64*1024);

				if (!pTagData)
				{
					break;
				}

				memset(pTagData, 0, 64*1024);

				TagDataLength = (USHORT)(64*1024 - 1);

				hr = nudf::util::nxl::NxrmReadTagsEx(hFile, pTagData, &TagDataLength);

				if (hr != S_OK)
				{
					break;
				}

				BufferToPair((LPCWSTR)pTagData, tag);

				hr = m_pRmUIObj->RmuShowClassifyDialogEx(hWnd, 
														 m_pwsSelectedFile, 
														 ClassifyUIXMLFileName,
														 GroupName,
														 GetUserDefaultLangID(), 
														 FALSE, 					  
														 tag);

				if (hr != S_OK)
				{
					break;
				}

				PairToBuffer(tag, wzTag);

				hr = nudf::util::nxl::NxrmSyncNXLHeader(hFile, NXL_SECTION_TAGS, (UCHAR*)(&wzTag[0]), (USHORT)(wzTag.size() * sizeof(WCHAR)));

			} while (FALSE);

			if (hFile != INVALID_HANDLE_VALUE)
			{
				CloseHandle(hFile);
				hFile = INVALID_HANDLE_VALUE;
			}

			if (pTagData)
			{
				free(pTagData);
			}

			if (wcslen(ClassifyUIXMLFileName))
			{
				DeleteFileW(ClassifyUIXMLFileName);
			}

			break;
		case CmdHelp:
			break;
		default:
			hr = E_INVALIDARG;
			break;
		}

	} while (FALSE);

	return hr;
}

STDMETHODIMP IRmCtxMenu::QueryContextMenu(_In_  HMENU hmenu, _In_  UINT indexMenu, _In_  UINT idCmdFirst, _In_  UINT idCmdLast, _In_  UINT uFlags)
{
	HRESULT hr = S_OK;

	UINT    uCmdID = idCmdFirst;
	HMENU   hSubMenu = NULL;
	UINT    uSubMenuFlags = 0;
	DWORD	dwAttrs = 0;
	BOOL	bIsServiceRunning = FALSE;
	BOOL	bZipFileExist = FALSE;
	do 
	{
		// If Flags contains CMF_DEFAULTONLY, ignore it
		if ((uFlags & CMF_DEFAULTONLY) || m_pwsSelectedFile == NULL || m_nSelectedFileCount == 0) 
		{
			hr = MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
			break;
		}

		dwAttrs = GetFileAttributesW(m_pwsSelectedFile);

		if (FILE_ATTRIBUTE_DIRECTORY == (FILE_ATTRIBUTE_DIRECTORY & dwAttrs) && dwAttrs != INVALID_FILE_ATTRIBUTES)
		{//it should not be a folder. ignore
			hr = MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
			break;
		}

		bIsServiceRunning = IsFilteringDriverFunction();

		// Insert START separator
		InsertMenuW(hmenu, indexMenu++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);

		// Create/Insert Sub Menus
		hSubMenu = CreateMenu();

		if (NULL == hSubMenu) 
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			break;
		}

        // Insert sub menu item: Protect ...
        uSubMenuFlags = MF_STRING | MF_BYPOSITION;

        if ((!m_bShowProtectMenu) || (m_bIsNXLFile) || (!bIsServiceRunning) || m_bMultiFileSelected)
        {
            uSubMenuFlags |= MF_GRAYED;
        }

        InsertMenuW(hSubMenu, 0, uSubMenuFlags, uCmdID++, m_MenuTextW[CmdProtect].c_str());
        SetMenuItemBitmaps(hSubMenu, 0, MF_BYPOSITION, NULL, NULL); // m_bmpLock, m_bmpLock);

        // Insert sub menu item: Remove Protection ...
        uSubMenuFlags = MF_STRING | MF_BYPOSITION;

        if ((!bIsServiceRunning) || (!m_bHasDecryptionRights && !m_bMultiFileSelected) || m_nSelectedNXLCount == 0)
        {
            uSubMenuFlags |= MF_GRAYED;
        }

        InsertMenuW(hSubMenu, 1, uSubMenuFlags, uCmdID++, m_MenuTextW[CmdUnprotect].c_str());
        SetMenuItemBitmaps(hSubMenu, 1, MF_BYPOSITION, NULL, NULL); // m_bmpUnlock, m_bmpUnlock);

		// Insert sub menu item: Add archive ...
		uSubMenuFlags = MF_STRING | MF_BYPOSITION;
		std::wstring menustr = m_MenuTextW[CmdZip].c_str();
		std::wstring strtmp;
		if (m_bFolder) {
			strtmp = GetArchivePath((WCHAR *)GetParentDir(m_pwsSelectedFile).c_str());
		}
		else {
			strtmp = GetArchivePath(m_pwsSelectedFile);
		}

		bZipFileExist = PathFileExists(strtmp.c_str());
		strtmp = GetFileName((WCHAR *)strtmp.c_str());
		menustr += strtmp;

		if ((!bIsServiceRunning) || m_nSelectedNXLCount == 0 || bZipFileExist)
		{
			uSubMenuFlags |= MF_GRAYED;
		}

		InsertMenuW(hSubMenu, 2, uSubMenuFlags, uCmdID++, menustr.c_str());
		SetMenuItemBitmaps(hSubMenu, 2, MF_BYPOSITION, NULL, NULL); // m_bmpUnlock, m_bmpUnlock);

        // Insert END separator
        InsertMenuW(hSubMenu, 3, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);

        // Insert sub menu item: Permission ...
        uSubMenuFlags = MF_STRING | MF_BYPOSITION;

        if ((!m_bIsNXLFile) || (!bIsServiceRunning) || m_bMultiFileSelected) 
        {
            uSubMenuFlags |= MF_GRAYED;
        }

        InsertMenuW(hSubMenu, 4, uSubMenuFlags, uCmdID++, m_MenuTextW[CmdCheckPermission].c_str());
        SetMenuItemBitmaps(hSubMenu, 4, MF_BYPOSITION, NULL, NULL); // m_bmpUser, m_bmpUser);

        // Insert END separator
        InsertMenuW(hSubMenu, 5, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);

        // Insert sub menu item: Classify ...
        uSubMenuFlags = MF_STRING | MF_BYPOSITION;

        if ((!m_bShowProtectMenu) || (!m_bIsNXLFile) || (!bIsServiceRunning) || (!m_bHasClassifyRight) || m_bMultiFileSelected) 
        {
            uSubMenuFlags |= MF_GRAYED;
        }

        InsertMenuW(hSubMenu, 6, uSubMenuFlags, uCmdID++, m_MenuTextW[CmdClassify].c_str());
        SetMenuItemBitmaps(hSubMenu, 6, MF_BYPOSITION, NULL, NULL); // m_bmpUser, m_bmpUser);

        // Insert Sub Menus
        InsertMenuW(hmenu, indexMenu, MF_STRING | MF_POPUP | MF_BYPOSITION, (UINT_PTR)hSubMenu, MenuGroupName);
        SetMenuItemBitmaps(hmenu, indexMenu, MF_BYPOSITION, m_hMainBitmap, m_hMainBitmap);

		// Finally, let Windows Explorer know how many menu items have been added
		hr = MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, uCmdID - idCmdFirst + 1);

	} while (FALSE);

	return hr;
}

void inline IRmCtxMenu::ResetFileSelection()
{
	if (m_pwsSelectedFile)
	{
		free(m_pwsSelectedFile);
		m_pwsSelectedFile = NULL;
	}

	m_nSelectedFileCount = 0;
}

bool IRmCtxMenu::IsFixedDrive(WCHAR * filepath)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	WCHAR tmpFileName[MAX_PATH + 1] = { 0 };

	hFile = CreateFileW(filepath,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_DIRECTORY,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE){//try open as directory
		hFile = CreateFileW(filepath,
			GENERIC_READ,
			FILE_SHARE_READ ,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS,
			NULL);
		if(hFile == INVALID_HANDLE_VALUE)
			return false;
	}

	DWORD dwRet = GetFinalPathNameByHandleW(hFile, tmpFileName, sizeof(tmpFileName) / sizeof(WCHAR) - 1, VOLUME_NAME_DOS);

	if (dwRet >= sizeof(tmpFileName) / sizeof(WCHAR) || dwRet < 7) // L"\\?\c:\"
	{
		//
		// buffer is too small. path is too deep
		//
		CloseHandle(hFile);
		return false;
	}

	WCHAR c = tmpFileName[4];
	WCHAR d = tmpFileName[5];
	WCHAR e = tmpFileName[6];

	if (d != L':' || e != L'\\')
	{
		CloseHandle(hFile);
		return false;
	}

	if ((!(c >= L'A' && c <= L'Z')) &&
		(!(c >= 'a' && c <= 'z')))
	{
		CloseHandle(hFile);
		return false;
	}

	tmpFileName[7] = L'\0';

	if (DRIVE_FIXED != GetDriveTypeW(tmpFileName))
	{
		CloseHandle(hFile);
		return false;
	}

	CloseHandle(hFile);

	return true;
}

bool IRmCtxMenu::AllocateFileBuffer(int count)
{
	ULONG bufsize = (MAX_PATH + 1) * sizeof(WCHAR) * count;

	if (m_pwsSelectedFile)
	{
		free(m_pwsSelectedFile);
		m_pwsSelectedFile = NULL;
	}
	m_pwsSelectedFile = (WCHAR*)malloc(bufsize);
	if (!m_pwsSelectedFile)
	{
		return false;
	}
	memset(m_pwsSelectedFile, 0, bufsize);
	return true;
}

std::wstring IRmCtxMenu::GetParentDir(WCHAR * filepath)
{
	std::wstring wsParentDir = filepath;
	std::wstring::size_type pos = wsParentDir.find_last_of(L"\\");

	if (std::wstring::npos == pos) {
		wsParentDir = L"";
		return wsParentDir;
	}

	wsParentDir = wsParentDir.substr(0, pos + 1);   // Include last L'\\'
	if (L'\\' == filepath[0] && L'\\' == filepath[1] && L'\0' != filepath[2]
		&& L'\\' != filepath[2] && NULL != wcschr(&filepath[2], L'\\')) {// IsUncPath()
		const WCHAR* pos = wcschr((wsParentDir.c_str() + 2), L'\\');
		assert(NULL != pos);
		if (L'\0' != (*(pos + 1))) {
			// This is not root, remove last L'\\'
			wsParentDir = wsParentDir.substr(0, wsParentDir.length() - 1);
		}
	}
	else {
		if (wsParentDir.length() > 3) {
			// This is not root, remove last L'\\'
			wsParentDir = wsParentDir.substr(0, wsParentDir.length() - 1);
		}
	}

	return wsParentDir;
}

std::wstring IRmCtxMenu::GetFileName(WCHAR * filepath)
{
	std::wstring wsFilename = filepath;
	std::wstring::size_type pos = wsFilename.find_last_of(L"\\");
	if (std::wstring::npos != pos) {
		wsFilename = wsFilename.substr(pos + 1);
	}

	return wsFilename;
}

std::wstring IRmCtxMenu::GetFileExt(WCHAR * filename)
{
	std::wstring wsfilename = GetFileName(filename);
	std::wstring::size_type pos = wsfilename.find_last_of(L".");
	if (std::wstring::npos != pos) {
		wsfilename = wsfilename.substr(pos + 1);
	}
	else {
		wsfilename = L"";
	}

	return wsfilename;
}

std::wstring IRmCtxMenu::GetArchivePath(WCHAR * filepath)
{
	std::wstring wsfilepath = GetParentDir(filepath);
	std::wstring wsfilename = GetFileName(filepath);
	std::wstring wsfileext = GetFileExt(filepath);

	while (wsfileext.length() != 0) {
		std::wstring::size_type pos = wsfilename.find_last_of(L".");
		if (std::wstring::npos != pos) {
			wsfilename = wsfilename.substr(0, pos);
		}
		wsfileext = GetFileExt((WCHAR *)wsfilename.c_str());
	}

	wsfilename += L".zip";
	wsfilepath += L"\\" + wsfilename;

	return wsfilepath;
}

bool IRmCtxMenu::IsNxlFile(WCHAR * filepath)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;

	WCHAR tmpFileName[MAX_PATH + 1] = { 0 };

	wcscat_s(tmpFileName, sizeof(tmpFileName) / sizeof(WCHAR), filepath);
	wcscat_s(tmpFileName, sizeof(tmpFileName) / sizeof(WCHAR), L".nxl");

	hFile = CreateFileW(tmpFileName,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		// try file name without ".NXL"
		hFile = CreateFileW(filepath,
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			// If we can open this file, the file without ".nxl" extension must exist.
			// We need to check if this file is a NXL file
			// (Sometimes user may remove ".nxl" extension, so it is not reliable
			//  to recognize NXL file only by ".nxl" extension, we need to check
			//  NXL file header in this case)

			// Check NXL file header
			NXL_SIGNATURE   Signature = { 0 };
			DWORD           dwRead = 0;
			memset(&Signature, 0, sizeof(Signature));
			if (::ReadFile(hFile, &Signature, sizeof(Signature), &dwRead, NULL) && Signature.Code.HighPart == NXL_SIGNATURE_HIGH && Signature.Code.LowPart == NXL_SIGNATURE_LOW)
			{
				// Good, this is a NXL file without ".nxl" extension
				// Append ".nxl" extension automatically
				CloseHandle(hFile);
				hFile = INVALID_HANDLE_VALUE;
				std::wstring extName = PathFindExtension(m_pwsSelectedFile);
				if (_wcsicmp(extName.c_str(), L".nxl"))
				{
					if (::MoveFileW(m_pwsSelectedFile, tmpFileName))
					{
						WIN32_FIND_DATA FindFileData = { 0 };

						//
						// notify driver about new NXL file
						//
						hFile = FindFirstFileW(tmpFileName, &FindFileData);

						if (hFile != INVALID_HANDLE_VALUE)
						{
							FindClose(hFile);
							hFile = INVALID_HANDLE_VALUE;
						}

						// Try to open Again
						hFile = CreateFileW(tmpFileName,
							GENERIC_READ,
							FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);
					}
				}
			}
			else
			{
				// NOT NXL file
				CloseHandle(hFile);
				hFile = INVALID_HANDLE_VALUE;
			}
		}
	}
	if (INVALID_HANDLE_VALUE == hFile)
		return false;

	CloseHandle(hFile);
	return true;
}

STDMETHODIMP IRmCtxMenu::Initialize(_In_opt_  PCIDLIST_ABSOLUTE pidlFolder, _In_opt_  IDataObject *pdtobj, _In_opt_  HKEY hkeyProgID)
{
	HRESULT hr = S_OK;

	FORMATETC   FmtEtc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM   Stg = {0};
	HDROP       hDrop = NULL;

	int         nNumFiles;

	HANDLE hFile = INVALID_HANDLE_VALUE;

	WCHAR tmpFileName[MAX_PATH+1] = {0};

	DWORD	dwAttrs = 0;

	DWORD dwRet = 0;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	m_bFolder = false;
	m_bMultiFileSelected = false;
	do
	{
		memset(&Stg, 0, sizeof(Stg));

		Stg.tymed = CF_HDROP;

		m_nSelectedFileCount = 0;
		m_nSelectedNXLCount = 0;

		// Find CF_HDROP data in pDataObj
		if (FAILED(pdtobj->GetData(&FmtEtc, &Stg)))
		{
			hr = E_INVALIDARG;
			break;
		}

		// Get the pointer pointing to real data
		hDrop = (HDROP)GlobalLock(Stg.hGlobal);

		if (NULL == hDrop)
		{
			hr = E_INVALIDARG;
			break;
		}

		// How many files are selected?
		nNumFiles = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);

		if (nNumFiles > 1)
			m_bMultiFileSelected = true;

		if (!AllocateFileBuffer(nNumFiles)) {
			hr = E_OUTOFMEMORY;
			break;
		}

		if (0 == DragQueryFileW(hDrop, 0, tmpFileName, MAX_PATH))//backup the first entry to tmpFileName
		{
			hr = E_UNEXPECTED;
			break;
		}

		if (!IsFixedDrive(tmpFileName)) {
			ResetFileSelection();
			break;
		}

		LoadCtxMenuRegExFilter();
		int i;
		for (i = 0; i < nNumFiles; i++) {
			WCHAR * pBuffer = m_pwsSelectedFile + (MAX_PATH + 1) * i;
			if (0 == DragQueryFileW(hDrop, i, pBuffer, MAX_PATH))
			{
				hr = E_UNEXPECTED;
				m_nSelectedFileCount = 0;
				break;
			}

			if (std::tr1::regex_match(pBuffer, *m_pBypassedFilter))
			{
				continue;
			}

			dwAttrs = GetFileAttributesW(pBuffer);

			if (FILE_ATTRIBUTE_DIRECTORY == (FILE_ATTRIBUTE_DIRECTORY & dwAttrs) && dwAttrs != INVALID_FILE_ATTRIBUTES)
			{
				m_bFolder = true;
				m_bIsNXLFile = FALSE;
				if (nNumFiles > 1){//multiple file and folder are selected.
					m_nSelectedFileCount = 0;
					break;
				}

				wcscat_s(tmpFileName, MAX_PATH, L"\\*");
				m_bMultiFileSelected = true;//set as true even only one nxl file inside.

				int filecount = nNumFiles;//set minimum buffer size for a folder, 1 is the start point.
				
				WIN32_FIND_DATA FindFileData;
				HANDLE hFind;
				hFind = FindFirstFileW(tmpFileName, &FindFileData);//the folder name is copied to m_pwsSelectedFile
				if (INVALID_HANDLE_VALUE == hFind) {
					m_nSelectedFileCount = 0;
					break;
				}
				do  {
					WCHAR fullfilepath[MAX_PATH + 1] = { 0 };
					wcscpy_s(fullfilepath, tmpFileName);
					fullfilepath[wcslen(fullfilepath) - 1] = 0;//reset last * to null
					wcscat_s(fullfilepath, FindFileData.cFileName);

					if (std::tr1::regex_match(fullfilepath, *m_pBypassedFilter))
					{
						continue;
					}
					if (m_pCtxMenuFilter)
					{
						if (!std::tr1::regex_match(fullfilepath, *m_pCtxMenuFilter))
						{
							continue;
						}
					}

					dwAttrs = GetFileAttributesW(fullfilepath);


					if (FILE_ATTRIBUTE_DIRECTORY != (FILE_ATTRIBUTE_DIRECTORY & dwAttrs)) {//only process for files
						if (m_nSelectedFileCount + 1 > (ULONG)filecount) {
							WCHAR * pbackup = m_pwsSelectedFile;
							m_pwsSelectedFile = NULL;
							if (!AllocateFileBuffer(filecount + 10)) {
								hr = E_OUTOFMEMORY;
								m_nSelectedFileCount = 0;
								free(pbackup);
								break;
							}
							memcpy(m_pwsSelectedFile, pbackup, (MAX_PATH + 1) * sizeof(WCHAR) * filecount);
							filecount += 10;
							free(pbackup);
						}
						if (IsNxlFile(fullfilepath)) {
							wcscat_s(fullfilepath, MAX_PATH, L".nxl");
							m_nSelectedNXLCount++;
						}
						pBuffer = m_pwsSelectedFile + (MAX_PATH + 1) * m_nSelectedFileCount;
						wcscpy_s(pBuffer, MAX_PATH, fullfilepath);
						m_nSelectedFileCount++;
					}
				} while (FindNextFileW(hFind, &FindFileData));
			}
			else {
				if (m_pCtxMenuFilter)
				{
					if (!std::tr1::regex_match(pBuffer, *m_pCtxMenuFilter))
					{
						continue;
					}
				}
				if (IsNxlFile(pBuffer)) {
					if(m_bMultiFileSelected)//add .nxl extension only for multiple file selection
						wcscat_s(pBuffer, MAX_PATH, L".nxl");
					m_nSelectedNXLCount++;
				}
				m_nSelectedFileCount++;
			}
		}

		if (m_nSelectedFileCount == 0 || (m_nSelectedNXLCount == 0 && m_bFolder) || (m_nSelectedNXLCount == 0 && m_nSelectedFileCount > 1)) {//nothing is selected or no NXL is selected for multiple selection
			ResetFileSelection();
			break;
		}


		hFile = INVALID_HANDLE_VALUE;
		if (nNumFiles == 1 && !m_bFolder) {//only do following check when ONE file is selected.
			m_bShowProtectMenu = check_enable_protect_menu();
			m_bIsNXLFile = IsNxlFile(m_pwsSelectedFile);
			if (m_bIsNXLFile)
			{
				hr = nudf::util::nxl::NxrmCheckRights(m_pwsSelectedFile, &RightsMask, &CustomRightsMask, &EvaluationId);

				if (SUCCEEDED(hr))
				{
					m_bHasDecryptionRights = (RightsMask & BUILTIN_RIGHT_DECRYPT) ? TRUE : FALSE;
					m_bHasClassifyRight = (RightsMask & BUILTIN_RIGHT_CLASSIFY) ? TRUE : FALSE;
				}
			}
		}
		
	} while (FALSE);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	if (Stg.hGlobal)
	{
		GlobalUnlock(Stg.hGlobal);
		ReleaseStgMedium(&Stg);
	}

	return hr;
}

BOOL IsFilteringDriverFunction(void)
{
	BOOL bRet = TRUE;

	if (!init_rm_section_safe())
	{
		bRet = FALSE;
	}

	return bRet;
}

static BOOL get_classify_ui(WCHAR *FileName, WCHAR *GroupName)
{
	BOOL bRet = FALSE;

	GET_CLASSIFY_UI_REQUEST		Request = {0};
	GET_CLASSIFY_UI_RESPONSE	Resp = {0};

	NXCONTEXT Ctx = NULL;

	ULONG RespSize = sizeof(Resp);

	LANGID	LangId = {0};

	do 
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		LangId = GetUserDefaultLangID();

		Request.LanguageId	= LangId;
		Request.ProcessId	= GetCurrentProcessId();
		Request.ThreadId	= GetCurrentThreadId();
		
		if(!ProcessIdToSessionId(Request.ProcessId, &Request.SessionId))
		{
			break;
		}

		if (!GetTempPathW(sizeof(Request.TempPath)/sizeof(WCHAR), Request.TempPath))
		{
			break;
		}

		if (!GetLongPathNameW(Request.TempPath, Request.TempPath, sizeof(Request.TempPath)/sizeof(WCHAR)))
		{
			break;
		}

		Ctx = submit_request(Global.Section, NXRMDRV_MSG_TYPE_GET_CLASSIFYUI, &Request, sizeof(Request));

		if (!Ctx)
		{
			break;
		}

		if (!wait_for_response(Ctx, Global.Section, &Resp, sizeof(Resp), &RespSize))
		{
			Ctx = NULL;
			break;
		}

		memcpy(FileName, 
			   Resp.ClassifyUIFileName, 
			   min(wcslen(Resp.ClassifyUIFileName)*sizeof(WCHAR), MAX_PATH - sizeof(WCHAR)));

		memcpy(GroupName,
			   Resp.GroupName,
			   min(wcslen(Resp.GroupName)*sizeof(WCHAR), MAX_PATH - sizeof(WCHAR)));

		bRet = TRUE;

	} while (FALSE);

	return bRet;
}

static void PairToBuffer(_In_ const std::vector<std::pair<std::wstring,std::wstring>>& pairs, _Out_ std::vector<WCHAR>& buf)
{
	buf.clear();
	for(int i=0; i<(int)pairs.size(); i++) {
		if(pairs[i].first.empty() || pairs[i].second.empty()) {
			continue;
		}
		std::wstring ws(pairs[i].first);
		ws += L"=";
		ws += pairs[i].second;
		for(int j=0; j<(int)ws.length(); j++) {
			buf.push_back(ws.c_str()[j]);
		}
		buf.push_back(L'\0');
	}
	if(!buf.empty()) {
		buf.push_back(L'\0');
	}
}

static void BufferToPair(_In_ LPCWSTR buf, _Out_ std::vector<std::pair<std::wstring,std::wstring>>& pairs)
{
	pairs.clear();
	while(buf[0] != L'\0') {
		std::wstring wsPair(buf);
		buf += (wsPair.length() + 1);
		std::wstring name;
		std::wstring value;
		std::wstring::size_type pos = wsPair.find_first_of(L'=');
		if(pos == std::wstring::npos) {
			continue;
		}
		name = wsPair.substr(0, pos);
		value = wsPair.substr(pos+1);
		pairs.push_back(std::pair<std::wstring,std::wstring>(name,value));
	}
}

static BOOL check_enable_protect_menu(void)
{
	BOOL bRet = TRUE;

	CHECK_PROTECT_MENU_REQUEST	Request = {0};
	CHECK_PROTECT_MENU_RESPONSE	Resp = {0};

	NXCONTEXT Ctx = NULL;

	ULONG RespSize = sizeof(Resp);

	do 
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		Request.ProcessId	= GetCurrentProcessId();
		Request.ThreadId	= GetCurrentThreadId();

		if(!ProcessIdToSessionId(Request.ProcessId, &Request.SessionId))
		{
			break;
		}

		Ctx = submit_request(Global.Section, NXRMDRV_MSG_TYPE_CHECK_PROTECT_MENU, &Request, sizeof(Request));

		if (!Ctx)
		{
			break;
		}

		if (!wait_for_response(Ctx, Global.Section, &Resp, sizeof(Resp), &RespSize))
		{
			Ctx = NULL;
			break;
		}

		if (Resp.EnableProtectMenu)
		{
			bRet = TRUE;
		}
		else
		{
			bRet = FALSE;
		}

	} while (FALSE);

	return bRet;
}

void IRmCtxMenu::LoadCtxMenuRegExFilter()
{
	QUERY_CTXMENUREGEX_REQUEST	Request = {0};
	QUERY_CTXMENUREGEX_RESPONSE	Resp = {0};

	NXCONTEXT Ctx = NULL;

	ULONG RespSize = sizeof(Resp);

	ULONG PolicySN = 0;

	do 
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		if (!is_good_version(Global.Section))
		{
			if (m_pCtxMenuFilter)
			{
				delete m_pCtxMenuFilter;
				m_pCtxMenuFilter = NULL;
			}

			break;
		}

		PolicySN = (ULONG)get_rm_policy_sn(Global.Section);

		if (PolicySN == m_lastPolicySN && m_lastPolicySN)
		{
			break;
		}

		Request.ProcessId	= GetCurrentProcessId();
		Request.ThreadId	= GetCurrentThreadId();

		if(!ProcessIdToSessionId(Request.ProcessId, &Request.SessionId))
		{
			break;
		}

		Ctx = submit_request(Global.Section, NXRMDRV_MSG_TYPE_GET_CTXMENUREGEX, &Request, sizeof(Request));

		if (!Ctx)
		{
			break;
		}

		if (!wait_for_response(Ctx, Global.Section, &Resp, sizeof(Resp), &RespSize))
		{
			Ctx = NULL;
			break;
		}

		if (m_pCtxMenuFilter)
		{
			delete m_pCtxMenuFilter;
			m_pCtxMenuFilter = NULL;
		}

		try
		{
			m_pCtxMenuFilter = new std::tr1::wregex(Resp.CtxMenuRegEx, std::tr1::regex_constants::icase);
		}
		catch (const std::regex_error)
		{
			m_pCtxMenuFilter = NULL;
			break;
		}

		m_lastPolicySN = PolicySN;

	} while (FALSE);

	return;
}

static BOOL IsRemoteEvaluation(const WCHAR *FileName)
{
	BOOL bRet = FALSE;

	NX::NXL::nxl_file nxl_file;

	bool validated = false;

	do 
	{
		try
		{
			nxl_file.open(FileName, std::vector<NX::NXL::nxl_key_pkg>());
			
			bRet = (nxl_file.is_remote_eval(&validated) == true);
		}
		catch (std::exception)
		{
			
		}

	} while (FALSE);

	return bRet;
}

BOOL send_block_notification(const WCHAR *FileName, BLOCK_NOTIFICATION_TYPE Type)
{
	BOOL bRet = TRUE;

	BLOCK_NOTIFICATION_REQUEST Req = { 0 };

	NXCONTEXT Ctx = NULL;

	ULONG bytesret = 0;

	do
	{
		Req.ProcessId = GetCurrentProcessId();
		Req.ThreadId = GetCurrentThreadId();
		ProcessIdToSessionId(Req.ProcessId, &Req.SessionId);
		Req.LanguageId = GetSystemDefaultLangID();
		Req.Type = Type;

		if (FileName)
		{
			memcpy(Req.FileName,
				   FileName,
				   min(sizeof(Req.FileName) - sizeof(WCHAR), wcslen(FileName)*sizeof(WCHAR)));
		}
		else
		{
			memset(Req.FileName, 0, sizeof(Req.FileName));
		}

		Ctx = submit_request(Global.Section, NXRMDRV_MSG_TYPE_BLOCK_NOTIFICATION, &Req, sizeof(Req));

		if (!Ctx)
		{
			break;
		}

		if (!wait_for_response(Ctx, Global.Section, NULL, 0, &bytesret))
		{
			Ctx = NULL;
			bRet = FALSE;
			break;
		}

		Ctx = NULL;

	} while (FALSE);

	return bRet;
}
