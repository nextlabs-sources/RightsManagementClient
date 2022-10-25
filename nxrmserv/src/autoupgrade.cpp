

#include <Windows.h>


#include <string>

#include <boost\algorithm\string.hpp>

#include <nudf\string.hpp>
#include <nudf\ziputil.hpp>

#include "nxrmserv.h"
#include "rest.hpp"
#include "autoupgrade.hpp"



namespace {

bool check_update(std::wstring& new_version, std::wstring& download_url, std::wstring& pkg_checksum)
{
    bool result = false;
    NX::rest_checkupdate_result updateinfo;
    // check
    if (GLOBAL.rest().check_update(updateinfo) && updateinfo.has_new_version()) {
        result = true;
        new_version = updateinfo.new_version();
        download_url = updateinfo.download_url();
        pkg_checksum = updateinfo.checksum();
    }
    return result;
}

std::wstring generate_temp_folder()
{
    std::wstring dir;
    // Prepare download folder
    GetTempFileNameW(GLOBAL.dir_wintemp().c_str(), L"NXU", 0, nudf::string::tempstr<wchar_t>(dir, MAX_PATH));
    ::DeleteFileW(dir.c_str());
    if (!CreateDirectoryW(dir.c_str(), NULL)) {
        dir.clear();
    }
    return std::move(dir);
}

std::wstring download_pkg(const std::wstring& download_url, const std::wstring& dir)
{
    // Download
    std::wstring file;

    // check url
    auto pos = download_url.find_last_of(L"/");
    if (pos == std::wstring::npos) {
        LOGERR(ERROR_INVALID_DATA, L"AutoUpdate: Invalid URL (%s)", download_url.c_str());
        return L"";
    }

    // compose file path
    file = dir;
    if (!boost::algorithm::ends_with(file, L"\\")) {
        file += L"\\";
    }
    file += download_url.substr(pos + 1);

    ::DeleteFileW(file.c_str());
    LOGINF(L"AutoUpdate: Downloading ...");
    HRESULT hr = URLDownloadToFileW(NULL, download_url.c_str(), file.c_str(), 0, NULL);
    if (FAILED(hr) || INVALID_FILE_ATTRIBUTES == GetFileAttributesW(file.c_str())) {
        LOGERR(hr, L"AutoUpdate: Download failed (0x%08X)", hr);
        return L"";
    }

    LOGINF(L"AutoUpdate: Download succeed (%s)", file.c_str());
    return std::move(file);
}

bool start_installer(const std::wstring dir)
{
    // Good, start installer now
    std::wstring msiexec_cmd = L"C:\\Windows\\System32\\msiexec.exe /i \"";
    msiexec_cmd += dir;
    msiexec_cmd += L"\\NextLabs Rights Management.msi";
    msiexec_cmd += L"\" /quiet /norestart /L*V \"";
    // The update.log cannot be put into RMC install folder because it will block installation process.
    msiexec_cmd += GLOBAL.dir_wintemp();
    msiexec_cmd += L"\\NextLabs-RMC-Update.log\"";

    // Create Process
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);
    if (!::CreateProcessW(NULL, (LPWSTR)msiexec_cmd.c_str(), NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, dir.c_str(), &si, &pi)) {
        LOGERR(GetLastError(), L"AutoUpdate: Fail to initiate install process (%d)", GetLastError());
        return false;
    }
    LOGINF(L"AutoUpdate: Installing ...");
    ResumeThread(pi.hThread);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}

}

bool NX::auto_upgrade() noexcept
{
    std::wstring _new_version;
    std::wstring _download_url;
    std::wstring _pkg_checksum;

    // check
    if (!check_update(_new_version, _download_url, _pkg_checksum)) {
        return false;
    }
    LOGINF(L"AutoUpdate: New version is found ...\r\n\tVersion: %s\r\n\tUrl: %s\r\n\tChecksum: %s", _new_version.c_str(), _download_url.c_str(), _pkg_checksum.c_str());

    // create temp install folder
    std::wstring _dir = generate_temp_folder();
    if (_dir.empty()) {
        LOGERR(GetLastError(), L"AutoUpdate: Fail to create temp folder for downloading");
        return false;
    }

    // download
    std::wstring _file = download_pkg(_download_url, _dir);
    if (_file.empty()) {
        return false;
    }

    // unpack
    nudf::util::CZip zip;
    if (!zip.Unzip(_file, _dir)) {
        LOGERR(GetLastError(), L"AutoUpdate: Fail to unzip installer package (%d, %s)", GetLastError(), _file.c_str());
        return false;
    }

    return start_installer(_dir);
}