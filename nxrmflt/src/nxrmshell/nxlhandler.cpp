
#include <Windows.h>
#include <winternl.h>
#include <Commctrl.h>
#include <Sddl.h>
#include <Shlobj.h>
#include <winternl.h>

#include <string>
#include <iostream>
#include <fstream>

#include <nudf\string.hpp>
#include <nudf\nxlutil.hpp>
#include <nudf\shared\nxlfmt.h>

#include "resource.h"


#ifndef CACHE_LINE
#define CACHE_LINE	64
#endif
#ifndef CACHE_ALIGN
#define CACHE_ALIGN	__declspec(align(CACHE_LINE))
#endif
#include "nxrmshellglobal.h"



DWORD CALLBACK CopyProgressRoutine(
  _In_ LARGE_INTEGER TotalFileSize,
  _In_ LARGE_INTEGER TotalBytesTransferred,
  _In_ LARGE_INTEGER StreamSize,
  _In_ LARGE_INTEGER StreamBytesTransferred,
  _In_ DWORD dwStreamNumber,
  _In_ DWORD dwCallbackReason,
  _In_ HANDLE hSourceFile,
  _In_ HANDLE hDestinationFile,
  _In_opt_ LPVOID lpData
);

DWORD CALLBACK CopyProgressThread(_In_opt_ PVOID lpData);
INT_PTR CALLBACK DlgProgressProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL IsLocalDisk(WCHAR wzDrive);
BOOL IsINetCacheFile(const WCHAR* file);
BOOL IsDownloadsFolderFile(const WCHAR* file);
BOOL IsTempFolderFile(const WCHAR* file);
bool get_original_ext(const std::wstring& file, std::wstring& ext);
std::wstring replace_ext(const std::wstring& source, const std::wstring& ext);

typedef struct _COPYINFOW {
    HWND            hWnd;
    BOOL            bCancel;
    WCHAR           FileNameW[MAX_PATH+1];
} COPYINFOW, *PCOPYINFOW;



#ifdef __cplusplus
extern "C" {
#endif

extern SHELL_GLOBAL_DATA Global;

extern 	BOOL init_rm_section_safe(void);

#ifdef __cplusplus
}
#endif


class win_path
{
public:
    win_path()
    {
    }
    win_path(const std::string& path) : _path(normalize(to_unicode(path)))
    {
    }
    win_path(const std::wstring& path) : _path(normalize(path))
    {
    }
    ~win_path()
    {
    }

    std::wstring to_nt_path()
    {
        std::wstring nt_path;

        if (!empty()) {
            nt_path = to_nt_path(path());
            if (nt_path.empty()) {
                nt_path = to_nt_path_unsafe();
            }
        }

        return std::move(nt_path);
    }

    std::wstring get_file_name() const
    {
        std::wstring file_name = get_file_name(path());
        return std::move(file_name);
    }

    std::wstring get_file_extension() const
    {
        std::wstring file_extension = get_file_extension(get_file_name());
        return std::move(file_extension);
    }

    static std::wstring get_file_name(const std::wstring& file_path)
    {
        std::wstring file_name;
        if (!file_path.empty()) {
            auto pos = file_path.find_last_of(L'\\');
            file_name = (pos == std::wstring::npos) ? file_path : file_path.substr(pos + 1);
        }
        return std::move(file_name);
    }

    static std::wstring get_file_extension(const std::wstring& file_name)
    {
        std::wstring file_extension;
        if (!file_name.empty()) {
            auto pos = file_name.find_last_of(L'.');
            file_extension = (pos == std::wstring::npos) ? L"" : file_name.substr(pos);
        }
        return std::move(file_extension);
    }

    bool empty() const { return _path.empty(); }
    const std::wstring& path() const { return _path; }
    win_path& operator = (const win_path& other)
    {
        if (this != &other) {
            _path = other.path();
        }
        return *this;
    }

    bool is_dos_path() const
    {
        return (_path.length() >= 3 && _path[0] >= L'a' && _path[0] <= L'z' && _path[1] == L':' && _path[2] == L'\\');
    }

    bool is_unc_path() const
    {
        return (_path.length() >= 3 && _path[0] == L'\\' && _path[1] == L'\\' && _path[2] != L'\\');
    }

    bool is_local_drive(wchar_t drive)
    {
        const wchar_t drive_path[4] = { drive, L':', L'\\', 0 };
        return (DRIVE_FIXED == GetDriveTypeW(drive_path)) ? true : false;
    }

    bool is_remote_drive(wchar_t drive)
    {
        const wchar_t drive_path[4] = { drive, L':', L'\\', 0 };
        return (DRIVE_REMOTE == GetDriveTypeW(drive_path)) ? true : false;
    }


protected:
    std::wstring to_unicode(const std::string& path)
    {
        std::wstring s;
        if (0 == MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, path.c_str(), (int)path.length(), nudf::string::tempstr<wchar_t>(s, MAX_PATH), MAX_PATH)) {
            s.clear();
        }
        return std::move(s);
    }

    std::wstring normalize(const std::wstring& path)
    {
        std::wstring s(path);

        std::transform(s.begin(), s.end(), s.begin(), tolower);
        if (boost::algorithm::starts_with(path, L"\\??\\")) {
            s = s.substr(4);
        }

        if (s.length() < 3) {
            s.clear();
        }
        else {

            if (boost::algorithm::istarts_with(path, L"unc\\")) {
                // UNC path
                std::wstring new_path(L"\\");
                new_path += s.substr(3);
                s = new_path;
            }
            else if (s.length() >= 3 && s[0] >= L'a' && s[0] <= L'z' && s[1] == L':' && s[2] == L'\\') {
                // Dos path, check if it is remote drive
                if (is_remote_drive(s[0])) {
                    s = to_unc_path(s);
                }
            }
            else if (boost::algorithm::istarts_with(path, L"\\\\")) {
                // UNC path, nothing
                ;
            }
            else {
                s.clear();
            }
        }

        return std::move(s);
    }

    std::wstring to_nt_path(const std::wstring& path)
    {
        std::wstring nt_path;
        HANDLE h = INVALID_HANDLE_VALUE;

        h = ::CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (INVALID_HANDLE_VALUE == h) {
            return std::move(nt_path);
        }

        std::vector<wchar_t> buf;

        buf.resize(1025, 0);
        DWORD return_size = GetFinalPathNameByHandleW(h, buf.data(), 1024, VOLUME_NAME_NT);
        if (return_size > 1024) {
            buf.resize(return_size + 1, 0);
            GetFinalPathNameByHandleW(h, buf.data(), return_size, VOLUME_NAME_NT);
        }
        CloseHandle(h);
        h = INVALID_HANDLE_VALUE;
        nt_path = buf.data();
        return std::move(nt_path);
    }

    std::wstring to_nt_path_unsafe()
    {
        std::wstring nt_path;
        
        if (!empty()) {
            if (is_unc_path()) {
                // UNC
                nt_path = L"\\Device\\Mup";
                nt_path += _path.substr(1);
            }
            else if (is_dos_path()) {
                // Dos Path
                const wchar_t drive_name[3] = { _path[0], _path[1], 0 };
                wchar_t device_name[MAX_PATH] = { 0 };
                if (0 != QueryDosDeviceW(drive_name, device_name, MAX_PATH)) {
                    nt_path = device_name;
                    nt_path += _path.substr(2);
                }
            }
            else {
                // unknown path
                ;
            }
        }

        return std::move(nt_path);
    }

    std::wstring to_unc_path(const std::wstring& path)
    {
        std::wstring unc_path;
        UNIVERSAL_NAME_INFOW* puni = NULL;
        DWORD bufsize = 0;

        //Call WNetGetUniversalName using UNIVERSAL_NAME_INFO_LEVEL option
        wchar_t temp = 0;
        if (WNetGetUniversalNameW(path.c_str(), UNIVERSAL_NAME_INFO_LEVEL, (LPVOID)&temp, &bufsize) == ERROR_MORE_DATA) {
            // now we have the size required to hold the UNC path
            std::vector<wchar_t> buffer;
            buffer.resize(bufsize + 1, 0);
            puni = (UNIVERSAL_NAME_INFOW*)buffer.data();
            if (WNetGetUniversalNameW(path.c_str(), UNIVERSAL_NAME_INFO_LEVEL, (LPVOID)puni, &bufsize) == NO_ERROR) {
                unc_path = puni->lpUniversalName;
            }
        }
        return std::move(unc_path);
    }
    
private:
    std::wstring    _path;
};

void validate_file(const std::wstring& file_path)
{
    WIN32_FIND_DATAW wsfd;
    memset(&wsfd, 0, sizeof(wsfd));
    HANDLE hFind = FindFirstFileW(file_path.c_str(), &wsfd);
    if (INVALID_HANDLE_VALUE != hFind) {
        FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }
}

// From observations, the extensions of filenames that we receive from various
// apps could have been modified to the following:
// - Outlook 2013:  "filename.ext (1)"
// - IE 11:         "filename.ext (1)"
// - Chrome 62:     "filename.ext (1)"
// - Firefox 57:    "filename.ext-1"
bool is_fix_extension_requried(const std::wstring& file_extension)
{
    std::wstring::size_type pos = file_extension.find_first_of(L" ([{-");
    return (pos != std::wstring::npos);
}

std::wstring fix_extension(const std::wstring& file_extension)
{
    std::wstring fixed_extension;
    std::wstring::size_type pos = file_extension.find_first_of(L" ([{-");
    if (pos != std::wstring::npos) {
        fixed_extension = file_extension.substr(pos);
        fixed_extension += file_extension.substr(0, pos);
    }
    else {
        fixed_extension = file_extension;
    }
    return std::move(fixed_extension);
}

std::wstring get_cache_dir()
{
    std::wstring cache_directory;

    // Get temp Folder   
    WCHAR*  pwzPath = NULL;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT_PATH | KF_FLAG_CREATE, NULL, &pwzPath);
    if (FAILED(hr)) {
        return std::wstring();
    }

    cache_directory = pwzPath;
    CoTaskMemFree(pwzPath);
    pwzPath = NULL;

    cache_directory += L"\\NextLabs";
    if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(cache_directory.c_str())) {
        ::CreateDirectoryW(cache_directory.c_str(), NULL);
    }
    cache_directory += L"\\cache";
    if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(cache_directory.c_str())) {
        ::CreateDirectoryW(cache_directory.c_str(), NULL);
    }

    return std::move(cache_directory);
}

std::wstring create_temp_folder()
{
    static const std::wstring cache_directory = get_cache_dir();
    
    // Generate temp Dir
    std::wstring temp_directory;

    GetTempFileNameW(cache_directory.c_str(), L"NXL", 0, nudf::string::tempstr<wchar_t>(temp_directory, MAX_PATH));
    if (temp_directory.empty()) {
        return temp_directory;
    }
    ::DeleteFileW(temp_directory.c_str());
    if (!::CreateDirectoryW(temp_directory.c_str(), NULL)) {
        temp_directory.clear();
        return temp_directory;
    }

    return std::move(temp_directory);
}

bool copy_file_with_progress(const std::wstring& source, const std::wstring& target)
{
    HANDLE      h = NULL;
    DWORD       thread_id = 0;
    COPYINFOW   copy_info = { 0 };
    BOOL        is_cancled = FALSE;

    // Create UI thread
    memset(&copy_info, 0, sizeof(copy_info));
    copy_info.hWnd = NULL;
    copy_info.bCancel = FALSE;
    wcsncpy_s(copy_info.FileNameW, MAX_PATH, source.c_str(), _TRUNCATE);

    h = ::CreateThread(NULL, 0, CopyProgressThread, &copy_info, 0, &thread_id);
    if (NULL == h) {
        return false;
    }

    // Copy File
    bool result = CopyFileExW(source.c_str(), target.c_str(), CopyProgressRoutine, &copy_info, &is_cancled, COPY_FILE_FAIL_IF_EXISTS | COPY_FILE_NO_BUFFERING) ? true : false;

    // Destroy Progress UI and terminate thread
    SendMessageW(copy_info.hWnd, WM_DESTROY, 0, 0);
    CloseHandle(h);

    // return
    return (result && !is_cancled);
}

void CALLBACK OpenRemoteFileWithCache(HWND hWnd, HINSTANCE hInst, LPSTR lpszCmdLine, int nCmdShow)
{
#ifdef _DEBUG
    // MessageBoxW(NULL, L"Debug", L"Debug", MB_OK);
#endif    

    // Drivers are not enabled
    if (!init_rm_section_safe()) {
        return;
    }

    // 
    win_path    source_file(lpszCmdLine);
    bool        copy_required = false;
    bool        fix_extension_required = false;

    if (source_file.empty()) {
        return;
    }

    if (!boost::algorithm::iends_with(source_file.path(), L".nxl")) {
        return;
    }

    // remove L".NXL" extension
    const bool is_from_outlook = boost::algorithm::icontains(source_file.path(), L"content.outlook");
    std::wstring temp_visible_path(source_file.path().substr(0, source_file.path().length() - 4));
    if (is_from_outlook) {
        // Outlook removes some '.' in temp file name.
        // We need to recover original '.' after removing ".nxl" extension.
        //
        // For an attached filename.ext.nxl, there are three possible scenarios:
        // 1. "filename ext.nxl"
        // 2. "filename.ext (2).nxl"
        // 3. "filename ext (2).nxl"
        auto pos = temp_visible_path.find_last_of(L"\\ ");
        if (pos != std::wstring::npos && temp_visible_path[pos] == L' ') {
            if (temp_visible_path[pos + 1] != L'(' && temp_visible_path[pos + 1] != L'{' && temp_visible_path[pos + 1] != L'[') {
                // Handle Case 1: "filename ext".
                temp_visible_path[pos] = L'.';
            }
            else {
                auto pos2 = temp_visible_path.find_last_of(L"\\ .", pos - 1);
                if (pos2 != std::wstring::npos && temp_visible_path[pos2] == L' ') {
                    // Handle Case 3: "filename ext (2)".
                    temp_visible_path[pos2] = L'.';
                }
                else {
                    // No need to do anything for Case 2: "filename.ext (2)".
                }
            }
        }
    }
    const std::wstring visible_file_path(temp_visible_path);
    const std::wstring visible_file_name(win_path::get_file_name(visible_file_path));
    const std::wstring visible_file_extension(win_path::get_file_extension(visible_file_name));

    fix_extension_required = is_fix_extension_requried(visible_file_extension);
        
    if (source_file.is_dos_path()) {
        if (!source_file.is_local_drive(source_file.path()[0]) || IsINetCacheFile(source_file.path().c_str()) || IsDownloadsFolderFile(source_file.path().c_str()) || IsTempFolderFile(source_file.path().c_str())) {
            copy_required = true;
        }
    }
    else {
        assert(source_file.is_unc_path());
        copy_required = true;
    }

    win_path target_file;
    std::wstring target_visible_file;

    if (!copy_required) {

        if (fix_extension_required) {
            std::wstring moved_file;
            moved_file = visible_file_path.substr(0, visible_file_path.length() - visible_file_extension.length());
            moved_file += fix_extension(visible_file_extension);
            moved_file += L".nxl";
            ::MoveFileW(source_file.path().c_str(), moved_file.c_str());
            target_file = win_path(moved_file);
        }
        else {
            target_file = source_file;
        }
        target_visible_file = target_file.path().substr(0, target_file.path().length() - 4);
    }
    else {

        std::wstring moved_file = create_temp_folder();

        moved_file += L"\\";
        if (fix_extension_required) {
            moved_file += visible_file_name.substr(0, visible_file_name.length() - visible_file_extension.length());
            moved_file += fix_extension(visible_file_extension);
            target_visible_file = moved_file;
            moved_file += L".nxl";
        }
        else {
            moved_file += visible_file_name;
            target_visible_file = moved_file;
            moved_file += L".nxl";
        }
        target_file = win_path(moved_file);

        // copy file
        if (!copy_file_with_progress(source_file.path(), target_file.path())) {
            return;
        }

		validate_file(target_file.path());
        // notify driver
        std::wstring source_nt_path = source_file.to_nt_path();
        nudf::util::nxl::NxrmSetSourceFileName(target_visible_file.c_str(), source_nt_path.c_str());
    }

    validate_file(target_file.path());
    ShellExecuteW(NULL, NULL, target_visible_file.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

DWORD CALLBACK CopyProgressThread(_In_opt_ PVOID lpData)
{
    PCOPYINFOW lpCopyInfo = (PCOPYINFOW)lpData;

    return (DWORD)::DialogBoxParamW((HINSTANCE)Global.hModule, MAKEINTRESOURCEW(IDD_PROGRESS), NULL, DlgProgressProc, (LPARAM)lpData);
}

DWORD CALLBACK CopyProgressRoutine(
                                   _In_ LARGE_INTEGER TotalFileSize,
                                   _In_ LARGE_INTEGER TotalBytesTransferred,
                                   _In_ LARGE_INTEGER StreamSize,
                                   _In_ LARGE_INTEGER StreamBytesTransferred,
                                   _In_ DWORD dwStreamNumber,
                                   _In_ DWORD dwCallbackReason,
                                   _In_ HANDLE hSourceFile,
                                   _In_ HANDLE hDestinationFile,
                                   _In_opt_ LPVOID lpData
                                   )
{
    PCOPYINFOW  lpCopyInfo = reinterpret_cast<PCOPYINFOW>(lpData);

    if(lpCopyInfo->bCancel) {
        SendMessageW(lpCopyInfo->hWnd, WM_DESTROY, 0, 0);
        return PROGRESS_CANCEL;
    }

    if(CALLBACK_CHUNK_FINISHED == dwCallbackReason) {
        // Change Progress Information
        TotalBytesTransferred.QuadPart *= 2048;
        TotalBytesTransferred.QuadPart /= TotalFileSize.QuadPart;
        SendMessage(GetDlgItem(lpCopyInfo->hWnd, IDC_PROGRESS_BAR), PBM_SETPOS, TotalBytesTransferred.LowPart, 0);

        if(TotalFileSize.QuadPart == TotalBytesTransferred.QuadPart) {
            SetWindowTextW(GetDlgItem(lpCopyInfo->hWnd, IDC_STATIC_TITLE), L"Launching ...");
        }
    }

    return PROGRESS_CONTINUE;
}

INT_PTR CALLBACK DlgProgressProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR     nRet = FALSE;
    PCOPYINFOW  lpCopyInfo = NULL;

    
    if (WM_INITDIALOG == uMsg) {
        SetWindowLongPtrW(hWnd, DWLP_USER, (LONG_PTR)lParam);
        lpCopyInfo = reinterpret_cast<PCOPYINFOW>(lParam);
        if(NULL == lpCopyInfo) {
            return FALSE;
        }
        lpCopyInfo->hWnd = hWnd;
    }
    else {
        lpCopyInfo = reinterpret_cast<PCOPYINFOW>(GetWindowLongPtrW(hWnd, DWLP_USER));
        if(NULL == lpCopyInfo) {
            return FALSE;
        }
    }

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            HWND hWndPB = GetDlgItem(lpCopyInfo->hWnd, IDC_PROGRESS_BAR);
            SendMessage(hWndPB, PBM_SETRANGE, 0, MAKELPARAM(0, 2048));
            SendMessage(hWndPB, PBM_SETSTEP, (WPARAM) 1, 0);
            SetWindowTextW(GetDlgItem(lpCopyInfo->hWnd, IDC_STATIC_INFO), lpCopyInfo->FileNameW);
        }
        break;

    case WM_CLOSE:
        lpCopyInfo->bCancel = TRUE;
        SetWindowTextW(GetDlgItem(lpCopyInfo->hWnd, IDC_STATIC_INFO), L"Canceling operation ...");
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            SetWindowTextW(GetDlgItem(lpCopyInfo->hWnd, IDC_STATIC_INFO), L"Canceling operation ...");
            lpCopyInfo->bCancel = TRUE;
            return TRUE;
        default:
            nRet = FALSE;
            break;
        }
        break;

    case WM_DESTROY:
        ::DestroyWindow(hWnd);
        //::EndDialog(hWnd, 0);
        break;

    default:
        nRet = FALSE;
        break;
    }

    return nRet;
}

BOOL IsLocalDisk(WCHAR wzDrive)
{
    WCHAR wzDrivePath[4] = {0, L':', L'\\', 0};

    if(wzDrive >= L'a' && wzDrive <= L'z') {
        wzDrive -= 0x20;
    }
    if(wzDrive < L'A' && wzDrive > L'Z') {
        return FALSE;
    }

    wzDrivePath[0] = wzDrive;
    return (DRIVE_FIXED == GetDriveTypeW(wzDrivePath)) ? TRUE : FALSE;
}

BOOL IsINetCacheFile(const WCHAR* file)
{
    static std::wstring wsINetTempFolder;
    static std::wstring wsINetCacheFolder;

    if(NULL==file || L'\0'==file[0]) {
        return FALSE;
    }

    if(wsINetTempFolder.empty()) {

        // Get temp Folder   
        WCHAR*  pwzFolder = NULL;
        HRESULT hr = SHGetKnownFolderPath(FOLDERID_InternetCache, KF_FLAG_DEFAULT_PATH|KF_FLAG_CREATE, NULL, &pwzFolder);
        if(FAILED(hr)) {
            return FALSE;
        }
        wsINetTempFolder = pwzFolder;
        wsINetTempFolder += L"\\";
        CoTaskMemFree(pwzFolder);
    }

    if(wsINetCacheFolder.empty()) {

        // Get temp Folder   
        WCHAR*  pwzFolder = NULL;
        HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT_PATH|KF_FLAG_CREATE, NULL, &pwzFolder);
        if(FAILED(hr)) {
            return FALSE;
        }
        wsINetCacheFolder = pwzFolder;
        wsINetCacheFolder += L"\\Microsoft\\Windows\\INetCache\\";
        CoTaskMemFree(pwzFolder);
    }

    return ((0 == _wcsnicmp(file, wsINetTempFolder.c_str(), wsINetTempFolder.length())) || (0 == _wcsnicmp(file, wsINetCacheFolder.c_str(), wsINetCacheFolder.length())));
}

// Downloads folder is the folder that Chrome 62 uses for temp downloaded files.
BOOL IsDownloadsFolderFile(const WCHAR* file)
{
    static std::wstring wsDownloadsFolder;

    if(NULL==file || L'\0'==file[0]) {
        return FALSE;
    }

    // Get Downlods Folder
    WCHAR*  pwzFolder = NULL;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Downloads, KF_FLAG_DEFAULT, NULL, &pwzFolder);
    if(FAILED(hr)) {
        return FALSE;
    }
    wsDownloadsFolder = pwzFolder;
    wsDownloadsFolder += L"\\";
    CoTaskMemFree(pwzFolder);

    return (0 == _wcsnicmp(file, wsDownloadsFolder.c_str(), wsDownloadsFolder.length()));
}

// User's temp folder is the folder that Firefox 57 uses for temp downloaded files.
BOOL IsTempFolderFile(const WCHAR* file)
{
    static std::wstring wsTempFolder;

    if(NULL==file || L'\0'==file[0]) {
        return FALSE;
    }

    // Get Temp Folder
    WCHAR   wzBuffer[MAX_PATH + 1];
    const DWORD nBufferLength = _countof(wzBuffer);
    if (GetTempPath(nBufferLength, wzBuffer) == 0) {
        return FALSE;
    }

    wsTempFolder = wzBuffer;

    return (0 == _wcsnicmp(file, wsTempFolder.c_str(), wsTempFolder.length()));
}

static std::wstring find_attribute(const wchar_t* attrs, const wchar_t* n)
{
    while(L'\0' != attrs) {

        std::wstring name;
        std::wstring value;
        std::wstring line = attrs;
        attrs += (line.length() + 1);   // move to next

        std::wstring::size_type pos = line.find_first_of(L"=");
        if(pos == std::wstring::npos) {
            name = line;
        }
        else {
            name = line.substr(0, pos);
            value = line.substr(pos+1);
        }
        if(0 == _wcsicmp(name.c_str(), n)) {
            return value;
        }
    }

    return std::wstring();
}

bool get_original_ext(const std::wstring& file, std::wstring& ext)
{
    std::ifstream   ifs;

    ifs.open(file, std::ios_base::in|std::ios_base::binary);
    if(!ifs.good()) {
        return false;
    }

    // get length of file:
    ifs.seekg (0, ifs.end);
    size_t length = (size_t)ifs.tellg();
    ifs.seekg (0, ifs.beg);
    if(length < NXL_MIN_SIZE) {
        return false;
    }

    std::vector<unsigned char> buf;

    buf.resize(NXL_MIN_SIZE, 0);
    ifs.read((char*)&buf[0], NXL_MIN_SIZE);
    if(ifs.fail()) {
        return false;
    }

    PCNXL_HEADER header = (PCNXL_HEADER)&buf[0];
    if(header->Signature.Code.HighPart != NXL_SIGNATURE_HIGH || header->Signature.Code.LowPart != NXL_SIGNATURE_LOW) {
        return false;
    }

    const wchar_t* attrs = (const wchar_t*)&buf[NXL_SCNDATA_OFFSET];
    ext = find_attribute(attrs, L"$FileExt");
    return true;
}

std::wstring::size_type find_last_substr(const std::wstring& str, const std::wstring& substr)
{
    std::wstring::size_type last_pos = std::wstring::npos;
    std::wstring::size_type pos = 0;
    
    while(std::wstring::npos != (pos=str.find(substr, pos))) {
        last_pos = pos;
        pos += substr.length();
    }

    return last_pos;
}

std::wstring replace_ext(const std::wstring& source, const std::wstring& ext)
{
    std::wstring new_path;

    if(ext.empty() || ext[0] != L'.' || ext.length()==1) {
        return source;
    }
    
    //std::wstring::size_type pos = source.find(ext);
    std::wstring::size_type pos = find_last_substr(source, ext);
    if(pos == std::wstring::npos) {
        // Not found?
        std::wstring special_ext(L" ");
        special_ext += &ext[1];
        //pos = source.find(special_ext);
        pos = find_last_substr(source, special_ext);
        if(pos == std::wstring::npos) {
            new_path = source;
            new_path += ext;
        }
        else {
            new_path = source.substr(0, pos);
            new_path += ext;
        }
    }
    else {
        new_path = source.substr(0, pos);
        new_path += ext;
    }

    return new_path;
}