
#include <Windows.h>
#include <Windowsx.h>
#include <Commctrl.h>
#include <assert.h>

#include <algorithm>
#include <sstream>

#include <nudf\exception.hpp>
#include <nudf\shared\rightsdef.h>
#include <nudf\string.hpp>
#include <nudf\user.hpp>
#include <nudf\asyncpipe.hpp>
#include <nudf\web\json.hpp>

#include "resource.h"
#include "commonui.hpp"
#include "dlgtemplate.hpp"
#include "rightsbox.hpp"


extern void get_logon_user(std::wstring& user_name, std::wstring& user_id);

#define PROPDLG_TITLE   L"NXL Protected Document"

static void GetScreenSize(int* cx, int* cy)
{
    int taskbar_cx = 0;
    int taskbar_cy = 0;

    if(0 == taskbar_cx || 0 == taskbar_cy) {
        RECT rect;
        HWND taskBar = FindWindowW(L"Shell_traywnd", NULL);
        if(taskBar && GetWindowRect(taskBar, &rect)) {
            taskbar_cx = rect.right - rect.left;
            taskbar_cy = rect.bottom - rect.top;
        }
    }

    if(taskbar_cx > taskbar_cy) {
        *cx = GetSystemMetrics(SM_CXFULLSCREEN);
        *cy = GetSystemMetrics(SM_CYFULLSCREEN) - taskbar_cy;
    }
    else {
        *cx = GetSystemMetrics(SM_CXFULLSCREEN) - taskbar_cx;
        *cy = GetSystemMetrics(SM_CYFULLSCREEN);
    }
}

static void CenterParent(HWND hWnd)
{
    HWND hParent = GetParent(hWnd);

    int scx = 0;
    int scy = 0;
    int cx = 0;
    int cy = 0;
    int x = 0;
    int y = 0;
    RECT rc = {0, 0, 0, 0};

    GetScreenSize(&scx, &scy);
    GetWindowRect(hWnd, &rc);
    cx = rc.right-rc.left;
    cy = rc.bottom-rc.top;

    if(NULL == hParent) {
        x = (scx - cx) / 2;
        y = (scy - cy) / 2;
    }
    else {
        RECT rcParent = {0, 0, 0, 0};
        int pcx = 0;
        int pcy = 0;
        GetWindowRect(hParent, &rcParent);
        pcx = rcParent.right-rcParent.left;
        pcy = rcParent.bottom-rcParent.top;
        x = rcParent.left - (cx - pcx) / 2;
        y = rcParent.top - (cy - pcy) / 2;
        if(x < 0) x = 0;
        if(y < 0) y = 0;
        if((x + cx) > scx) x = scx - cx;
        if((y + cy) > scy) y = scy - cy;
    }
    SetWindowPos(hWnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE);
}

CGeneralPage::CGeneralPage() : CPropPageDlgTemplate(), _hIcon(NULL), _center_parent(FALSE)
{
    _psp.dwSize      = sizeof(PROPSHEETPAGEW);
    _psp.dwFlags     = PSP_USETITLE;
    _psp.hInstance   = _hInstance;
    _psp.pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_GENERAL);
    _psp.hIcon       = 0;
    _psp.pszTitle    = L"General";
    _psp.pfnDlgProc  = GetDlgProc();
    _psp.lParam      = (LPARAM)this;

    memset(&_header, 0, sizeof(_header));
}

CGeneralPage::~CGeneralPage()
{
    if(NULL != _hIcon) {
        DestroyIcon(_hIcon);
        _hIcon = NULL;
    }
}

BOOL CGeneralPage::OnInitialize()
{
    BOOL hr = TRUE;
    SHFILEINFOW sfi = {0};

    if(_center_parent) {
        CenterParent(GetParent(GetHwnd()));
    }

	CoInitialize(NULL);
    hr = (BOOL)SHGetFileInfoW(_file.c_str(), -1, &sfi, sizeof(sfi), SHGFI_ICON);
    CoUninitialize();
    if(hr) {
        _hIcon = sfi.hIcon;
        Static_SetIcon(GetDlgItem(GetHwnd(), IDC_FILE_ICON), _hIcon);
    }

    std::wstring name;
    std::wstring path;
    std::wstring::size_type pos = _file.find_last_of(L'\\');
    if(std::wstring::npos != pos) {
        name = _file.substr(pos+1);
        path = _file.substr(0, pos);
    }
    else {
        name = _file;
        path = L"Unknown";
    }

    // Set Information
    SetWindowTextW(GetDlgItem(GetHwnd(), IDC_EDIT_OBJECTNAME), name.c_str());
    SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_LOCATION), path.c_str());

    WCHAR wzVer[128] = {0};
    swprintf_s(wzVer, 128, L"%d.%d", _header.Basic.Version >> 16, (_header.Basic.Version & 0xFFFF));
    SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_NXL_VERSION), wzVer);

    std::wstring wsGuid = nudf::string::FromGuid<wchar_t>((const GUID*)(&_header.Basic.Thumbprint), true);
    SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_NXL_GUID), wsGuid.c_str());

    SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_NXL_MSG), _header.Signature.Message);

    if(NXL_ALGORITHM_AES128 == _header.Crypto.Algorithm) {
        SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_NXL_CEALG), L"AES 128");
    }
    else if(NXL_ALGORITHM_AES256 == _header.Crypto.Algorithm) {
        SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_NXL_CEALG), L"AES 256");
    }
    else {
        SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_NXL_CEALG), L"Unknown");
    }

    std::wstring keyId;
    if(NXL_ALGORITHM_AES128 == _header.Crypto.PrimaryKey.KeKeyId.Algorithm) {
        SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_NXL_KEALG), L"AES 128");
        keyId = nudf::string::FromBytes<wchar_t>(_header.Crypto.PrimaryKey.KeKeyId.Id, _header.Crypto.PrimaryKey.KeKeyId.IdSize);
        SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_NXL_KEID), keyId.c_str());
    }
    else if(NXL_ALGORITHM_AES256 == _header.Crypto.PrimaryKey.KeKeyId.Algorithm) {
        SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_NXL_KEALG), L"AES 256");
        keyId = nudf::string::FromBytes<wchar_t>(_header.Crypto.PrimaryKey.KeKeyId.Id, _header.Crypto.PrimaryKey.KeKeyId.IdSize);
        SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_NXL_KEID), keyId.c_str());
    }
    else {
        SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_NXL_KEALG), L"Unknown");
        SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_NXL_KEID), L"N/A");
    }

    if(NXL_ALGORITHM_AES128 == _header.Crypto.RecoveryKey.KeKeyId.Algorithm) {
        SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_NXL_KRALG), L"AES 128");
        keyId = nudf::string::FromBytes<wchar_t>(_header.Crypto.RecoveryKey.KeKeyId.Id, _header.Crypto.RecoveryKey.KeKeyId.IdSize);
        SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_NXL_KRID), keyId.c_str());
    }
    else if(NXL_ALGORITHM_AES256 == _header.Crypto.RecoveryKey.KeKeyId.Algorithm) {
        SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_NXL_KRALG), L"AES 256");
        keyId = nudf::string::FromBytes<wchar_t>(_header.Crypto.RecoveryKey.KeKeyId.Id, _header.Crypto.RecoveryKey.KeKeyId.IdSize);
        SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_NXL_KRID), keyId.c_str());
    }
    else if(NXL_ALGORITHM_RSA1024 == _header.Crypto.RecoveryKey.KeKeyId.Algorithm) {
        SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_NXL_KRALG), L"RSA 1024");
        keyId = nudf::string::FromBytes<wchar_t>(_header.Crypto.RecoveryKey.KeKeyId.Id, _header.Crypto.RecoveryKey.KeKeyId.IdSize);
        SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_NXL_KRID), keyId.c_str());
    }
    else if(NXL_ALGORITHM_RSA2048 == _header.Crypto.RecoveryKey.KeKeyId.Algorithm) {
        SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_NXL_KRALG), L"RSA 2048");
        keyId = nudf::string::FromBytes<wchar_t>(_header.Crypto.RecoveryKey.KeKeyId.Id, _header.Crypto.RecoveryKey.KeKeyId.IdSize);
        SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_NXL_KRID), keyId.c_str());
    }
    else {
        SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_NXL_KRALG), L"N/A");
        SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_NXL_KRID), L"N/A");
    }

    return TRUE;
}

void CGeneralPage::OnPsnSetActive(_In_ LPPSHNOTIFY lppsn)
{
}

void CGeneralPage::OnPsnKillActive(_In_ LPPSHNOTIFY lppsn)
{
}

void CGeneralPage::OnPsnApply(_In_ LPPSHNOTIFY lppsn)
{
}

void CGeneralPage::OnPsnReset(_In_ LPPSHNOTIFY lppsn)
{
}

void CGeneralPage::OnPsnHelp(_In_ LPPSHNOTIFY lppsn)
{
}

void CGeneralPage::OnPsnWizBack(_In_ LPPSHNOTIFY lppsn)
{
}

void CGeneralPage::OnPsnWizNext(_In_ LPPSHNOTIFY lppsn)
{
}

void CGeneralPage::OnPsnWizFinish(_In_ LPPSHNOTIFY lppsn)
{
}

void CGeneralPage::OnPsnQueryCancel(_In_ LPPSHNOTIFY lppsn)
{
}

void CGeneralPage::OnPsnGetObject(_In_ LPPSHNOTIFY lppsn)
{
}

void CGeneralPage::OnPsnTranslateAccelerator(_In_ LPPSHNOTIFY lppsn)
{
}

void CGeneralPage::OnPsnQueryInitialFocus(_In_ LPPSHNOTIFY lppsn)
{
}

CPermissionPage::CPermissionPage() : CPropPageDlgTemplate(), _bEditMode(FALSE), _hEditImgList(NULL), _hViewImgList(NULL), _InitialRights(0), _NewRights(0), _center_parent(FALSE)
{
    _psp.dwSize      = sizeof(PROPSHEETPAGEW);
    _psp.dwFlags     = PSP_USETITLE;
    _psp.hInstance   = _hInstance;
    _psp.pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_PERMISSION);
    _psp.hIcon       = 0;
    _psp.pszTitle    = L"Permissions";
    _psp.pfnDlgProc  = GetDlgProc();
    _psp.lParam      = (LPARAM)this;
}

CPermissionPage::~CPermissionPage()
{
}

BOOL CPermissionPage::OnInitialize()
{
    HWND hList = GetDlgItem(GetHwnd(), IDC_LIST_RIGHTS);
    RECT rc;


    LVCOLUMNW   col;
    LVITEMW     item;

    
    if(_center_parent) {
        CenterParent(GetParent(GetHwnd()));
    }

    GetClientRect(hList, &rc);

    // try to ask nxrmtray.exe to get logon user ifnormation
    std::wstring logon_user;
    std::wstring logon_user_id;
    get_logon_user(logon_user, logon_user_id);
    if (logon_user.empty()) {
        nudf::win::CUser user;
        try {
            user.GetProcessUser(GetCurrentProcess());
            logon_user = user.GetAccountName();
            logon_user_id = user.GetSid().GetSidStr();
        }
        catch (const nudf::CException& e) {
            UNREFERENCED_PARAMETER(e);
        }
    }
    SetWindowTextW(GetDlgItem(GetHwnd(), IDC_EDIT_USERNAME), logon_user.c_str());
    
    SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_USERID), _file.c_str());
    
    ListView_SetExtendedListViewStyle(hList, /*LVS_EX_CHECKBOXES|*/LVS_EX_BORDERSELECT|LVS_EX_SUBITEMIMAGES);
    memset(&col, 0, sizeof(col));
    col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
    col.fmt = LVCFMT_LEFT;
    col.cx = ((rc.right - rc.left) * 6) / 10;
    col.pszText = L"Permission";
    ListView_InsertColumn(hList, 0, &col);
    memset(&col, 0, sizeof(col));
    col.mask = LVCF_FMT | LVCF_WIDTH;
    col.fmt = LVCFMT_IMAGE;
    col.cx = ((rc.right - rc.left) * 3) / 10;
    col.pszText = L"Status";
    ListView_InsertColumn(hList, 1, &col);

    _hEditImgList = ImageList_LoadImage(_hInstance, MAKEINTRESOURCEW(IDB_BMP_CHKBOX1), 16, 2, CLR_NONE, IMAGE_BITMAP, 0);
    _hViewImgList = ImageList_LoadImage(_hInstance, MAKEINTRESOURCEW(IDB_BMP_CHKBOX2), 16, 2, CLR_NONE, IMAGE_BITMAP, 0);
    //ListView_SetImageList(hList, _hViewImgList, LVSIL_STATE);
    ListView_SetImageList(hList, _hViewImgList, LVSIL_SMALL);


    // Set Status Colume
    memset(&item, 0, sizeof(item));
    item.mask   = LVIF_TEXT;
    item.iItem  = 0;
    item.pszText= L"Full Control";
    ListView_InsertItem(hList, &item);
    item.iItem  = 1;
    item.pszText= L"View";
    ListView_InsertItem(hList, &item);
    item.iItem  = 2;
    item.pszText= L"Edit";
    ListView_InsertItem(hList, &item);
    item.iItem  = 3;
    item.pszText= L"Print";
    ListView_InsertItem(hList, &item);
    item.iItem  = 4;
    item.pszText= L"Copy/Paste";
    ListView_InsertItem(hList, &item);
    item.iItem  = 5;
    item.pszText= L"Save As";
    ListView_InsertItem(hList, &item);
    item.iItem  = 6;
    item.pszText= L"Remove Protection";
    ListView_InsertItem(hList, &item);
    item.iItem  = 7;
    item.pszText= L"Screen Capture";
    ListView_InsertItem(hList, &item);
    item.iItem  = 8;
    item.pszText= L"Send";
    ListView_InsertItem(hList, &item);
    item.iItem  = 9;
    item.pszText= L"Classify";
    ListView_InsertItem(hList, &item);
    

    // Set Permissions Colume
    EnableEditMode(FALSE);

    return TRUE;
}

BOOL CPermissionPage::OnNotify(_In_ LPNMHDR lpnmhdr)
{
    if(IDC_LIST_RIGHTS == lpnmhdr->idFrom) {
        HWND hList = GetDlgItem(GetHwnd(), IDC_LIST_RIGHTS);
        if(NM_CLICK == lpnmhdr->code) {
            LVHITTESTINFO itestinfo;

            assert(hList == lpnmhdr->hwndFrom);
            memset(&itestinfo, 0, sizeof(itestinfo));
            itestinfo.pt = ((LPNMITEMACTIVATE)lpnmhdr)->ptAction;
            if(-1 != ListView_SubItemHitTest(hList, &itestinfo)) {
                if(_bEditMode && 1 == itestinfo.iSubItem && 0 != ((LVHT_ONITEMICON|LVHT_ONITEMSTATEICON)&itestinfo.flags)) {
                    // Change image
                    CheckItem(itestinfo.iItem, !IsItemChecked(itestinfo.iItem));
                }
            }
        }
    }
    else {
        ;
    }

    return FALSE;
}

BOOL CPermissionPage::OnCommand(WORD notify, WORD id, HWND hwnd)
{
    if(IDC_BTN_EDITRIGHTS == id) {
        HWND hEdit = GetDlgItem(GetHwnd(), IDC_BTN_EDITRIGHTS);
        if(BN_CLICKED  == notify) {
            EnableEditMode(!_bEditMode);
        }
    }

    return FALSE;
}

void CPermissionPage::UpdateRightsView()
{
    HWND hList = GetDlgItem(GetHwnd(), IDC_LIST_RIGHTS);
    int         nCount = ListView_GetItemCount(hList);
    LVITEMW     item;
    memset(&item, 0, sizeof(item));
    item.mask   = LVIF_IMAGE;
    item.iSubItem  = 1;

    // Set Image List
    ListView_SetImageList(hList, _bEditMode ? _hEditImgList : _hViewImgList, LVSIL_SMALL);

    // Full Control ?
    if(BUILTIN_RIGHT_ALL == (BUILTIN_RIGHT_ALL & _InitialRights)) {
        item.iImage = 1;
        item.iItem  = 0;
        ListView_SetItem(hList, &item);
    }
    else {
        item.iImage = 0;
        item.iItem  = 0;
        ListView_SetItem(hList, &item);
    }

    // Set each right
    for(int i=1; i<nCount; i++) {
        if(0 != (_InitialRights&ItemToRights(i))) {
            item.iImage = 1;
            item.iItem  = i;
            ListView_SetItem(hList, &item);
        }
        else {
            item.iImage = 0;
            item.iItem  = i;
            ListView_SetItem(hList, &item);
        }
    }
}

void CPermissionPage::EnableEditMode(BOOL bEnabled)
{
    _bEditMode = bEnabled;
    if(bEnabled) {
        _NewRights = _InitialRights;
    }
    else {
        _InitialRights = _NewRights;
    }
    UpdateRightsView();

}

void CPermissionPage::CheckItem(int iItem, BOOL bCheck)
{
    HWND    hList = GetDlgItem(GetHwnd(), IDC_LIST_RIGHTS);
    int     nCount = 0;
    LVITEMW item;
    memset(&item, 0, sizeof(item));
    item.mask   = LVIF_IMAGE;
    item.iImage = bCheck ? 1 : 0;
    item.iItem  = iItem;
    item.iSubItem  = 1;
    ListView_SetItem(hList, &item);

    if(0 == iItem) {
        // Check/Uncheck Full Control
        nCount = ListView_GetItemCount(hList);
        _NewRights = bCheck ? BUILTIN_RIGHT_ALL : 0;
        for(int i=1; i<nCount; i++) {
            item.iItem  = i;
            item.iItem  = i;
            ListView_SetItem(hList, &item);
        }
    }
    else {
        // Check/Uncheck Single Right
        _NewRights = bCheck ? (_NewRights | ItemToRights(iItem)) : (_NewRights & (~ItemToRights(iItem)));
    }
}

BOOL CPermissionPage::IsItemChecked(int iItem)
{
    int     nCount = 0;

    if(0 == iItem) {
        return (BUILTIN_RIGHT_ALL == _NewRights);
    }

    nCount = ListView_GetItemCount(GetDlgItem(GetHwnd(), IDC_LIST_RIGHTS));
    if(iItem >= nCount) {
        return FALSE;
    }

    return (0 != (_NewRights & ItemToRights(iItem)));
}

ULONGLONG CPermissionPage::ItemToRights(int iItem)
{
    switch(iItem)
    {
    case 0: return BUILTIN_RIGHT_ALL;
    case 1: return BUILTIN_RIGHT_VIEW;
    case 2: return BUILTIN_RIGHT_EDIT;
    case 3: return BUILTIN_RIGHT_PRINT;
    case 4: return BUILTIN_RIGHT_CLIPBOARD;
    case 5: return BUILTIN_RIGHT_SAVEAS;
    case 6: return BUILTIN_RIGHT_DECRYPT;
    case 7: return BUILTIN_RIGHT_SCREENCAP;
    case 8: return BUILTIN_RIGHT_SEND;
    case 9: return BUILTIN_RIGHT_CLASSIFY;
    default: break;
    }

    return 0ULL;
}

int CPermissionPage::RightsToItem(ULONGLONG rights)
{
    switch(rights)
    {
    case BUILTIN_RIGHT_ALL: return 0;
    case BUILTIN_RIGHT_VIEW: return 1;
    case BUILTIN_RIGHT_EDIT: return 2;
    case BUILTIN_RIGHT_PRINT: return 3;
    case BUILTIN_RIGHT_CLIPBOARD: return 4;
    case BUILTIN_RIGHT_SAVEAS: return 5;
    case BUILTIN_RIGHT_DECRYPT: return 6;
    case BUILTIN_RIGHT_SCREENCAP: return 7;
    case BUILTIN_RIGHT_SEND: return 8;
    case BUILTIN_RIGHT_CLASSIFY: return 9;
    default: break;
    }

    return -1;
}

void CPermissionPage::OnPsnSetActive(_In_ LPPSHNOTIFY lppsn)
{
}

void CPermissionPage::OnPsnKillActive(_In_ LPPSHNOTIFY lppsn)
{
}

void CPermissionPage::OnPsnApply(_In_ LPPSHNOTIFY lppsn)
{
}

void CPermissionPage::OnPsnReset(_In_ LPPSHNOTIFY lppsn)
{
}

void CPermissionPage::OnPsnHelp(_In_ LPPSHNOTIFY lppsn)
{
}

void CPermissionPage::OnPsnWizBack(_In_ LPPSHNOTIFY lppsn)
{
}

void CPermissionPage::OnPsnWizNext(_In_ LPPSHNOTIFY lppsn)
{
}

void CPermissionPage::OnPsnWizFinish(_In_ LPPSHNOTIFY lppsn)
{
}

void CPermissionPage::OnPsnQueryCancel(_In_ LPPSHNOTIFY lppsn)
{
}

void CPermissionPage::OnPsnGetObject(_In_ LPPSHNOTIFY lppsn)
{
}

void CPermissionPage::OnPsnTranslateAccelerator(_In_ LPPSHNOTIFY lppsn)
{
}

void CPermissionPage::OnPsnQueryInitialFocus(_In_ LPPSHNOTIFY lppsn)
{
}

CClassifyPage::CClassifyPage() : CPropPageDlgTemplate(), _center_parent(FALSE)
{
    _psp.dwSize      = sizeof(PROPSHEETPAGEW);
    _psp.dwFlags     = PSP_USETITLE;
    _psp.hInstance   = _hInstance;
    _psp.pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_DETAIL);
    _psp.hIcon       = 0;
    _psp.pszTitle    = L"Classification";
    _psp.pfnDlgProc  = GetDlgProc();
    _psp.lParam      = (LPARAM)this;
}

CClassifyPage::~CClassifyPage()
{
    if(NULL != _hIcon) {
        DestroyIcon(_hIcon);
        _hIcon = NULL;
    }
    if(NULL != _tooltip) {
        ::DestroyWindow(_tooltip);
        _tooltip = NULL;
    }
}

BOOL CClassifyPage::OnInitialize()
{
    HWND hList = GetDlgItem(GetHwnd(), IDC_LIST_CLASSIFICATIONS);
    
    LVCOLUMNW   col;
    LVITEMW     item;
    RECT        rc;
    
    BOOL hr = TRUE;
    SHFILEINFOW sfi = {0};    

    
    if(_center_parent) {
        CenterParent(GetParent(GetHwnd()));
    }

	CoInitialize(NULL);
    hr = (BOOL)SHGetFileInfoW(_file.c_str(), -1, &sfi, sizeof(sfi), SHGFI_ICON);
    CoUninitialize();
    if(hr) {
        _hIcon = sfi.hIcon;
        Static_SetIcon(GetDlgItem(GetHwnd(), IDC_TAG_FILE_ICON), _hIcon);
    }

    std::wstring name;
    std::wstring path;
    std::wstring::size_type pos = _file.find_last_of(L'\\');
    if(std::wstring::npos != pos) {
        name = _file.substr(pos+1);
        path = _file.substr(0, pos);
    }
    else {
        name = _file;
        path = L"Unknown";
    }

    // Set Information
    SetWindowTextW(GetDlgItem(GetHwnd(), IDC_EDIT_TAG_OBJECTNAME), name.c_str());
    SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_TAG_FILELOCATION), path.c_str());

    // Set Tooltip
    _tooltip = CreateWindowExW(0, L"tooltips_class32", NULL, 
             WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, 
             CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
             CW_USEDEFAULT, NULL, NULL, NULL, NULL);

    TOOLINFOW    toolinfo = {0};
    memset(&toolinfo, 0, sizeof(toolinfo));    
    toolinfo.cbSize = sizeof(toolinfo);
    toolinfo.uFlags = TTF_SUBCLASS | TTF_TRANSPARENT;
    toolinfo.hwnd = GetDlgItem(GetHwnd(), IDC_EDIT_TAG_OBJECTNAME);
    toolinfo.uId = IDC_EDIT_TAG_OBJECTNAME;
    toolinfo.hinst = NULL;
    toolinfo.lpszText = (LPWSTR)name.c_str();
    toolinfo.lParam = NULL;
    GetClientRect(toolinfo.hwnd, &toolinfo.rect);
    SendMessageW(_tooltip, TTM_ADDTOOL, 0, (LPARAM)&toolinfo);

    GetClientRect(hList, &rc);


    ListView_SetExtendedListViewStyle(hList, LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);

    memset(&col, 0, sizeof(col));
    col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
    col.fmt = LVCFMT_LEFT;
    col.cx = (rc.right - rc.left) / 3 - 4;
    col.pszText = L"Name";
    ListView_InsertColumn(hList, 0, &col);
    col.cx = (rc.right - rc.left) - col.cx - 4;
    col.pszText = L"Value";
    ListView_InsertColumn(hList, 1, &col);

    memset(&item, 0, sizeof(item));
    item.mask   = LVIF_TEXT;

    for(int i=0; i<(int)_classifydata.size(); i++) {
        item.iItem   = i;
        item.iSubItem= 0;
        item.pszText = (LPWSTR)_classifydata[i].first.c_str();
        ListView_InsertItem(hList, &item);
        item.iSubItem= 1;
        item.pszText = (LPWSTR)_classifydata[i].second.c_str();
        ListView_SetItem(hList, &item);
    }

    return TRUE;
}

void CClassifyPage::OnPsnSetActive(_In_ LPPSHNOTIFY lppsn)
{
}

void CClassifyPage::OnPsnKillActive(_In_ LPPSHNOTIFY lppsn)
{
}

void CClassifyPage::OnPsnApply(_In_ LPPSHNOTIFY lppsn)
{
}

void CClassifyPage::OnPsnReset(_In_ LPPSHNOTIFY lppsn)
{
}

void CClassifyPage::OnPsnHelp(_In_ LPPSHNOTIFY lppsn)
{
}

void CClassifyPage::OnPsnWizBack(_In_ LPPSHNOTIFY lppsn)
{
}

void CClassifyPage::OnPsnWizNext(_In_ LPPSHNOTIFY lppsn)
{
}

void CClassifyPage::OnPsnWizFinish(_In_ LPPSHNOTIFY lppsn)
{
}

void CClassifyPage::OnPsnQueryCancel(_In_ LPPSHNOTIFY lppsn)
{
}

void CClassifyPage::OnPsnGetObject(_In_ LPPSHNOTIFY lppsn)
{
}

void CClassifyPage::OnPsnTranslateAccelerator(_In_ LPPSHNOTIFY lppsn)
{
}

void CClassifyPage::OnPsnQueryInitialFocus(_In_ LPPSHNOTIFY lppsn)
{
}



//
//
//

CRightsBox::CRightsBox() : CPropSheet()
{
}

CRightsBox::~CRightsBox()
{
}

LONG CRightsBox::ShowGeneral(_In_ HWND hParent, _In_ const std::wstring& wzFile, _In_ PCNXL_HEADER pHeader)
{
    _genPage.SetFile(wzFile);
    _genPage.SetHeader(pHeader);
    _genPage.SetCenterParent(TRUE);
    return (LONG)Create(hParent, NULL, PROPDLG_TITLE, &_genPage.GetPage(), 1, 0);
}

LONG CRightsBox::ShowPermission(_In_ HWND hParent, _In_ const std::wstring& wzFile, _In_ ULONGLONG uRights)
{
    _pemPage.SetFile(wzFile);
    _pemPage.SetRights(uRights);
    _pemPage.SetCenterParent(TRUE);
    return (LONG)Create(hParent, NULL, PROPDLG_TITLE, &_pemPage.GetPage(), 1, 0);
}

LONG CRightsBox::ShowClassification(_In_ HWND hParent, _In_ const std::wstring& wzFile, _In_ const ATTRIBUTEPAIR& classifydata)
{
    _clsPage.SetFile(wzFile);
    _clsPage.SetClassifications(classifydata);
    _clsPage.SetCenterParent(TRUE);
    return (LONG)Create(hParent, NULL, PROPDLG_TITLE, &_clsPage.GetPage(), 1, 0);
}

LONG CRightsBox::ShowDetails(_In_ HWND hParent, _In_ const std::wstring& wzFile, _In_ ULONGLONG uRights, _In_ const ATTRIBUTEPAIR& classifydata)
{
    memset(_psps, 0, sizeof(PROPSHEETPAGEW)*3);
    memcpy(&_psps[0], &_pemPage.GetPage(), sizeof(PROPSHEETPAGEW));
    memcpy(&_psps[1], &_clsPage.GetPage(), sizeof(PROPSHEETPAGEW));

    _pemPage.SetFile(wzFile);
    _pemPage.SetRights(uRights);
    _pemPage.SetCenterParent(TRUE);
    _clsPage.SetFile(wzFile);
    _clsPage.SetClassifications(classifydata);
    _clsPage.SetCenterParent(FALSE);
    return (LONG)Create(hParent, NULL, PROPDLG_TITLE, _psps, 2, 0);
}

LONG CRightsBox::ShowAll(HWND hParent, _In_ const std::wstring& wzFile, _In_opt_ PCNXL_HEADER pHeader, _In_ ULONGLONG uRights, _In_ const ATTRIBUTEPAIR& classifydata)
{
    memset(_psps, 0, sizeof(PROPSHEETPAGEW)*3);
    memcpy(&_psps[0], &_genPage.GetPage(), sizeof(PROPSHEETPAGEW));
    memcpy(&_psps[1], &_pemPage.GetPage(), sizeof(PROPSHEETPAGEW));
    memcpy(&_psps[2], &_clsPage.GetPage(), sizeof(PROPSHEETPAGEW));

    _genPage.SetFile(wzFile);
    _genPage.SetHeader(pHeader);
    _genPage.SetCenterParent(TRUE);
    _pemPage.SetFile(wzFile);
    _pemPage.SetRights(uRights);
    _pemPage.SetCenterParent(FALSE);
    _clsPage.SetFile(wzFile);
    _clsPage.SetClassifications(classifydata);
    _clsPage.SetCenterParent(FALSE);
    return (LONG)Create(hParent, NULL, PROPDLG_TITLE, _psps, 3, 0);
}