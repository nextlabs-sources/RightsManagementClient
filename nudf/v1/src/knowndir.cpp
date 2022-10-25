
#include <Windows.h>
#include <assert.h>
#include <Wtsapi32.h>
#include <Sddl.h>
#include <Shlobj.h>

#include <nudf\exception.hpp>
#include <nudf\knowndir.hpp>
#include <nudf\user.hpp>


using namespace nudf::win;



CKnownDirs::CKnownDirs()
{
}

CKnownDirs::~CKnownDirs()
{
}

void CKnownDirs::Load(_In_ HANDLE hToken) throw()
{
    HRESULT hr = S_OK;
    WCHAR*  pwzPath = NULL;

    // Get FOLDERID_InternetCache
    hr = SHGetKnownFolderPath(FOLDERID_InternetCache, KF_FLAG_DEFAULT_PATH|KF_FLAG_CREATE, hToken, &pwzPath);
    if(SUCCEEDED(hr)) {
        _dirInternetCache = pwzPath;
        CoTaskMemFree(pwzPath);
        pwzPath = NULL;
    }

    // Get FOLDERID_RoamingAppData
    hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_DEFAULT_PATH|KF_FLAG_CREATE, hToken, &pwzPath);
    if(SUCCEEDED(hr)) {
        _dirRoamingAppData = pwzPath;
        CoTaskMemFree(pwzPath);
        pwzPath = NULL;
    }

    // Get FOLDERID_LocalAppData
    hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT_PATH|KF_FLAG_CREATE, hToken, &pwzPath);
    if(SUCCEEDED(hr)) {
        _dirLocalAppData = pwzPath;
        CoTaskMemFree(pwzPath);
        pwzPath = NULL;
    }

    // Make sure %LocalAppData%\Temp exist
    if(!_dirLocalAppData.empty()) {
        _dirTemp = _dirLocalAppData + L"\\Temp";
        if(!CreateTempDir(hToken, _dirTemp.c_str())) {
            _dirTemp = _dirInternetCache;
        }
    }

    // Get FOLDERID_LocalAppDataLow
    hr = SHGetKnownFolderPath(FOLDERID_LocalAppDataLow, KF_FLAG_DEFAULT_PATH|KF_FLAG_CREATE, hToken, &pwzPath);
    if(SUCCEEDED(hr)) {
        _dirLocalAppDataLow = pwzPath;
        CoTaskMemFree(pwzPath);
        pwzPath = NULL;
    }

    // Get FOLDERID_Profile
    hr = SHGetKnownFolderPath(FOLDERID_Profile, KF_FLAG_DEFAULT_PATH|KF_FLAG_CREATE, hToken, &pwzPath);
    if(SUCCEEDED(hr)) {
        _dirProfile = pwzPath;
        CoTaskMemFree(pwzPath);
        pwzPath = NULL;
    }

    // Get FOLDERID_Desktop
    hr = SHGetKnownFolderPath(FOLDERID_Desktop, KF_FLAG_DEFAULT_PATH|KF_FLAG_CREATE, hToken, &pwzPath);
    if(SUCCEEDED(hr)) {
        _dirDesktop = pwzPath;
        CoTaskMemFree(pwzPath);
        pwzPath = NULL;
    }

    // Get FOLDERID_Documents
    hr = SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT_PATH|KF_FLAG_CREATE, hToken, &pwzPath);
    if(SUCCEEDED(hr)) {
        _dirDocuments = pwzPath;
        CoTaskMemFree(pwzPath);
        pwzPath = NULL;
    }

    // Get FOLDERID_Cookies
    hr = SHGetKnownFolderPath(FOLDERID_Cookies, KF_FLAG_DEFAULT_PATH|KF_FLAG_CREATE, hToken, &pwzPath);
    if(SUCCEEDED(hr)) {
        _dirCookies = pwzPath;
        CoTaskMemFree(pwzPath);
        pwzPath = NULL;
    }
}

void CKnownDirs::Clear()
{
    _dirInternetCache.clear();
    _dirLocalAppData.clear();
    _dirLocalAppDataLow.clear();
    _dirRoamingAppData.clear();
    _dirProfile.clear();
    _dirDesktop.clear();
    _dirDocuments.clear();
    _dirCookies.clear();
    _dirTemp.clear();
}

bool CKnownDirs::CreateTempDir(_In_ HANDLE hToken, _In_ const std::wstring& folder)
{
    bool bRet = false;

    if(INVALID_FILE_ATTRIBUTES != GetFileAttributesW(folder.c_str())) {
        return true;
    }

    CImpersonate imp;

    if(NULL != hToken) {
        if(!imp.Impersonate(hToken)) {
            return false;
        }
    }

    bRet = ::CreateDirectoryW(folder.c_str(), NULL) ? true : false;
    if(!bRet && ERROR_ALREADY_EXISTS==GetLastError()) {
        bRet = true;
    }

    return bRet;
}