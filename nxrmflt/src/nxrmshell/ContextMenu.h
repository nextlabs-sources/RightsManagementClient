#pragma once

#include <regex>

#ifndef CACHE_LINE
#define CACHE_LINE	64
#endif

#ifndef CACHE_ALIGN
#define CACHE_ALIGN	__declspec(align(CACHE_LINE))
#endif

#define MAX_MENU_ITEM		5

#define NXRMCOREUI_CM_UI_DLL_NAME			L"nxrmcmui.dll"
#define NXRMSHELL_DLL_NAME					L"nxrmshell.dll"

#define BYPASSFILTERREGEX					L"^[c-zC-Z]{1}:\\\\windows\\\\.*|.*\\.dll$|.*\\.ttf$"

#pragma pack(push, 4)

typedef struct _FILE_GET_EA_INFORMATION {
	ULONG NextEntryOffset;
	UCHAR EaNameLength;
	CHAR EaName[1];
} FILE_GET_EA_INFORMATION, *PFILE_GET_EA_INFORMATION;

typedef struct _FILE_FULL_EA_INFORMATION {
	ULONG NextEntryOffset;
	UCHAR Flags;
	UCHAR EaNameLength;
	USHORT EaValueLength;
	CHAR EaName[1];
} FILE_FULL_EA_INFORMATION, *PFILE_FULL_EA_INFORMATION;

#pragma pack(pop)

typedef enum _MenuCommand {
	CmdProtect = 0,
	CmdUnprotect,
	CmdZip,
	CmdCheckPermission,
	CmdClassify,
	CmdHelp
} MenuCommand;

class IRmCtxMenu : public IContextMenu, public IShellExtInit
{
public:
	IRmCtxMenu();
	~IRmCtxMenu();

	STDMETHODIMP QueryInterface(REFIID riid, void **ppobj);

	STDMETHODIMP_(ULONG) AddRef();

	STDMETHODIMP_(ULONG) Release();

	STDMETHODIMP QueryContextMenu(_In_  HMENU hmenu, _In_  UINT indexMenu, _In_  UINT idCmdFirst, _In_  UINT idCmdLast, _In_  UINT uFlags);

	STDMETHODIMP InvokeCommand(_In_  CMINVOKECOMMANDINFO *pici);

	STDMETHODIMP GetCommandString(_In_  UINT_PTR idCmd, _In_  UINT uType, _Reserved_  UINT *pReserved, _Out_writes_bytes_((uType & GCS_UNICODE) ? (cchMax * sizeof(wchar_t)) : cchMax) _When_(!(uType & (GCS_VALIDATEA | GCS_VALIDATEW)), _Null_terminated_)  CHAR *pszName, _In_  UINT cchMax);

	STDMETHODIMP Initialize(_In_opt_  PCIDLIST_ABSOLUTE pidlFolder, _In_opt_  IDataObject *pdtobj, _In_opt_  HKEY hkeyProgID);

protected:
    void LoadStringResource();
    void SetDefaultStringResource();
	void LoadCtxMenuRegExFilter();
	inline void ResetFileSelection();
	bool IsFixedDrive(WCHAR * filepath);
	bool IsNxlFile(WCHAR * filepath);
	bool AllocateFileBuffer(int count);
	std::wstring GetParentDir(WCHAR * const filepath);
	std::wstring GetFileName(WCHAR * const filepath);
	std::wstring GetFileExt(WCHAR * const filename);
	std::wstring GetArchivePath(WCHAR * const filepath);
private:
	ULONG					m_uRefCount;
	WCHAR					*m_pwsSelectedFile;
	BOOL					m_bFolder;
	BOOL					m_bIsNXLFile;
	BOOL					m_bHasDecryptionRights;
	BOOL					m_bHasClassifyRight;
	BOOL					m_bShowProtectMenu;
	HICON					m_hMainIcon;
	HBITMAP					m_hMainBitmap;
	nudf::util::CRmuObject	*m_pRmUIObj;
	std::wregex				*m_pBypassedFilter;
	std::wregex				*m_pCtxMenuFilter;
	ULONG					m_lastPolicySN;
    std::vector<std::wstring>   m_MenuTextW;
    std::vector<std::string>    m_MenuTextA;
	ULONG					m_nSelectedFileCount;
	ULONG					m_nSelectedNXLCount;
	BOOL					m_bMultiFileSelected;
};