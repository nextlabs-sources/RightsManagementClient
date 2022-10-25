

#include <Windows.h>
#include <Shldisp.h>
#include <atlbase.h>
#include <atlcomcli.h>

#include <iostream>
#include <fstream>

#include <nudf\string.hpp>
#include <nudf\nxrmres.h>
#include <nudf\resutil.hpp>

#include "nxrmserv.h"
#include "secure.hpp"
#include "sconfig.hpp"
#include "debug.hpp"


using namespace NX;


debug_object::debug_object()
{
}

debug_object::debug_object(unsigned long session_id) : _session_id(session_id)
{
    generate_file_name(session_id);
}

debug_object::~debug_object()
{
}

bool debug_object::create() noexcept
{
    bool result = false;

    if (file().empty()) {
        return false;
    }

    const std::wstring wsdir(generate_temp_dir());
    if (wsdir.empty()) {
        return false;
    }

    try {

        if (!GLOBAL.agent_info().empty()) {

            std::wstring source;
            std::wstring target;
            std::string content;

            // agent has been registered

            //  ==> register.xml
            source = GLOBAL.dir_conf() + L"\\register.xml";
            target = wsdir + L"\\register.xml";
            if (!::CopyFileW(source.c_str(), target.c_str(), FALSE)) {
                LOGERR(GetLastError(), L"fail to copy debug file register.xml to %s", target.c_str());
            }
            //  ==> agent_info.sjs
            source = GLOBAL.dir_conf() + L"\\agent_info.sjs";
            target = wsdir + L"\\agent_info.json";
            content = NX::agent_info::decrypt(source);
            generate_file(target, content);
            //  ==> external_users.sjs
            source = GLOBAL.dir_conf() + L"\\external_users.sjs";
            target = wsdir + L"\\external_users.json";
            try {
                content = NX::sconfig::load(source, GLOBAL.agent_key().decrypt());
            }
            catch (std::exception& e) {
                UNREFERENCED_PARAMETER(e);
                content.clear();
            }
            generate_file(target, content);
            //  ==> agent_classify.sjs
            source = GLOBAL.dir_conf() + L"\\agent_classify.sjs";
            target = wsdir + L"\\agent_classify.json";
            try {
                content = NX::sconfig::load(source, GLOBAL.agent_key().decrypt());
            }
            catch (std::exception& e) {
                UNREFERENCED_PARAMETER(e);
                content.clear();
            }
            generate_file(target, content);
            //  ==> agent_policy.sjs
            source = GLOBAL.dir_conf() + L"\\agent_policy.sjs";
            target = wsdir + L"\\agent_policy.xml";
            try {
                content = NX::sconfig::load(source, GLOBAL.agent_key().decrypt());
            }
            catch (std::exception& e) {
                UNREFERENCED_PARAMETER(e);
                content.clear();
            }
            generate_file(target, content);
            //  ==> agent_policy_orig.sjs
            source = GLOBAL.dir_conf() + L"\\agent_policy_orig.sjs";
            target = wsdir + L"\\agent_policy_orig.xml";
            try {
                content = NX::sconfig::load(source, GLOBAL.agent_key().decrypt());
                generate_file(target, content);
            }
            catch (std::exception& e) {
                UNREFERENCED_PARAMETER(e);
                content.clear();
            }
            //  ==> agent_whitelist.sjs
            source = GLOBAL.dir_conf() + L"\\agent_whitelist.sjs";
            target = wsdir + L"\\agent_whitelist.json";
            try {
                content = NX::sconfig::load(source, GLOBAL.agent_key().decrypt());
            }
            catch (std::exception& e) {
                UNREFERENCED_PARAMETER(e);
                content.clear();
            }
            generate_file(target, content);
            //  ==> core_context.sjs
            source = GLOBAL.dir_conf() + L"\\core_context.sjs";
            target = wsdir + L"\\core_context.json";
            try {
                content = NX::sconfig::load(source, GLOBAL.agent_key().decrypt());
            }
            catch (std::exception& e) {
                UNREFERENCED_PARAMETER(e);
                content.clear();
            }
            generate_file(target, content);
            // ==> profiles
            decrypt_profiles(wsdir);
        }
        // ==> dump file
        copy_dump_profiles(wsdir);

        // zip
        // create zip file on user's desktop
        result = create_zip(wsdir, file());

    }
    catch (...) {
        result = false;
    }
    // notify
    std::shared_ptr<NX::session> sp = GLOBAL.serv_session().get(_session_id);
    if (sp != NULL) {
        std::wstring  title = nudf::util::res::LoadMessage(GLOBAL.res_module(), IDS_PRODUCT_NAME, 256, LANG_NEUTRAL, L"NextLabs Rights Management");
        std::wstring  info;
        if (result) {
            info = nudf::util::res::LoadMessageEx(GLOBAL.res_module(), IDS_NOTIFY_DBGLOG_COLLECTED, 1024, LANG_NEUTRAL, L"Debug data file (%s) has been generated on your desktop", file_name().c_str());
        }
        else {
            info = nudf::util::res::LoadMessage(GLOBAL.res_module(), IDS_NOTIFY_DBGLOG_COLLECT_FAILED, 1024, LANG_NEUTRAL, L"Fail to collect debug data");
        }
        sp->notify(title, info);
    }

    return result;
}

void debug_object::decrypt_profiles(const std::wstring& wsdir)
{
    const std::wstring ws_find(GLOBAL.dir_profiles() + L"\\S-1-5-*");
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW wfd = { 0 };

    hFind = FindFirstFileW(ws_find.c_str(), &wfd);
    if (INVALID_HANDLE_VALUE != hFind) {
        do {

            if (FILE_ATTRIBUTE_DIRECTORY != (FILE_ATTRIBUTE_DIRECTORY & wfd.dwFileAttributes)) {
                continue;
            }

            try {
                std::wstring source = GLOBAL.dir_profiles() + L"\\" + wfd.cFileName + L"\\profile.sjs";
                std::wstring target = wsdir + L"\\profiles-" + wfd.cFileName + L".json";
                if (INVALID_FILE_ATTRIBUTES != GetFileAttributesW(source.c_str())) {
                    std::string content = NX::sconfig::load(source, GLOBAL.agent_key().decrypt());
                    generate_file(target, content);
                }
            }
            catch (std::exception& e) {
                UNREFERENCED_PARAMETER(e);
                LOGERR(GetLastError(), L"fail to copy user (%s) profile.sjs", wfd.cFileName);
            }

        } while (FindNextFileW(hFind, &wfd));
        FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }
}

void debug_object::copy_dump_profiles(const std::wstring& wsdir)
{
    const std::wstring ws_find(GLOBAL.dir_root() + L"\\DebugDump*.txt");
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW wfd = { 0 };

    hFind = FindFirstFileW(ws_find.c_str(), &wfd);
    if (INVALID_HANDLE_VALUE != hFind) {
        do {

            if (FILE_ATTRIBUTE_DIRECTORY == (FILE_ATTRIBUTE_DIRECTORY & wfd.dwFileAttributes)) {
                continue;
            }

            try {
                std::wstring source = GLOBAL.dir_root() + L"\\" + wfd.cFileName;
                std::wstring target = wsdir + L"\\" + wfd.cFileName;
                if (!::CopyFileW(source.c_str(), target.c_str(), FALSE)) {
                    LOGERR(GetLastError(), L"fail to copy debug dump to %s", target.c_str());
                }
            }
            catch (std::exception& e) {
                UNREFERENCED_PARAMETER(e);
            }

        } while (FindNextFileW(hFind, &wfd));
        FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }
}

void debug_object::generate_file_name(unsigned long session_id) noexcept
{
    static volatile long long id = 0;

    std::wstring user_name;
    std::wstring file_name;
    std::wstring desktop_path;
    SYSTEMTIME st = { 0 };
    GetLocalTime(&st);

    _file.clear();
    _file_name.clear();

    std::shared_ptr<NX::session> sp = GLOBAL.serv_session().get(session_id);
    if (sp == NULL) {
        LOGERR(0, L"debug_object: session not exists");
        return;
    }

    user_name = sp->user().principle_name().empty() ? sp->user().name() : sp->user().principle_name();
    desktop_path = sp->session_dirs().GetDesktopDir();
    if (desktop_path.empty()) {
        LOGERR(0, L"debug_object: user's desktop path not exists");
        return;
    }

    swprintf_s(nudf::string::tempstr<wchar_t>(file_name, MAX_PATH), MAX_PATH, L"NXT-%s-%s-%04d%02d%02d%02d%02d%02d-%I64d.zip",
        GLOBAL.host().fully_qualified_domain_name().c_str(), user_name.c_str(),
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
        InterlockedAdd64(&id, 1));

    _file = desktop_path + L"\\" + file_name;
    _file_name = file_name;
}

std::wstring debug_object::generate_temp_dir()
{
    std::wstring wsdir;
    GetTempFileNameW(GLOBAL.dir_wintemp().c_str(), L"NX", 0, nudf::string::tempstr<wchar_t>(wsdir, MAX_PATH));
    ::DeleteFileW(wsdir.c_str());
    if (!CreateDirectoryW(wsdir.c_str(), NULL)) {
        LOGERR(GetLastError(), L"fail to create temp folder for debug dump (%s)", wsdir.c_str());
        wsdir.clear();
    }
    return wsdir;
}

bool debug_object::generate_file(const std::wstring& file, const std::string& content)
{
    bool result = false;

    try {
        std::ofstream fp;
        fp.open(file, std::ofstream::binary | std::ofstream::trunc);
        if (!fp.is_open() || !fp.good()) {
            LOGERR(GetLastError(), L"fail to create debug file (%s)", file.c_str());
            throw std::exception("fail to create debug file");
        }

        fp.write(content.c_str(), content.length());
        if (fp.fail() || fp.bad()) {
            LOGERR(GetLastError(), L"fail to write debug content to file (%s)", file.c_str());
            throw std::exception("fail to write debug content");
        }
        result = true;
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        result = false;
    }
    return result;
}

void debug_object::find_sub_files(const std::wstring& root_dir, std::vector<std::wstring>& dirs, std::vector<std::wstring>& files)
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW wfd;

    const std::wstring dir_file_pattern(root_dir + L"\\*");

    memset(&wfd, 0, sizeof(wfd));
    hFind = FindFirstFileW(dir_file_pattern.c_str(), &wfd);
    if (INVALID_HANDLE_VALUE != hFind) {
        do {

            if (0 == _wcsicmp(L".", wfd.cFileName) || 0 == _wcsicmp(L"..", wfd.cFileName)) {
                continue;
            }
            if (FILE_ATTRIBUTE_SYSTEM == (FILE_ATTRIBUTE_SYSTEM & wfd.dwFileAttributes)) {
                continue;
            }

            if (FILE_ATTRIBUTE_DIRECTORY == (FILE_ATTRIBUTE_DIRECTORY & wfd.dwFileAttributes)) {
                dirs.push_back(std::wstring(root_dir + L"\\" + wfd.cFileName));
            }
            else {
                files.push_back(std::wstring(root_dir + L"\\" + wfd.cFileName));
            }
        } while (FindNextFileW(hFind, &wfd));
        FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }
}

bool debug_object::remove_directory(const std::wstring& folder)
{
    bool result = true;
    std::vector<std::wstring> dirs;
    std::vector<std::wstring> files;

    find_sub_files(folder, dirs, files);

    // remove sub dirs
    std::for_each(dirs.begin(), dirs.end(), [&](const std::wstring& dir) {
        if (!remove_directory(dir)) {
            result = false;
        }
    });

    // remove sub files
    std::for_each(files.begin(), files.end(), [&](const std::wstring& file) {
        if (!remove_file(file)) {
            result = false;
        }
    });

    return result;
}

bool debug_object::remove_file(const std::wstring& file)
{
    unsigned long file_attrs = GetFileAttributesW(file.c_str());
    if (FILE_ATTRIBUTE_DIRECTORY == (FILE_ATTRIBUTE_DIRECTORY & file_attrs)) {
        return false;
    }

    if (FILE_ATTRIBUTE_READONLY == (FILE_ATTRIBUTE_READONLY & file_attrs)) {
        file_attrs &= (~FILE_ATTRIBUTE_READONLY);
        SetFileAttributesW(file.c_str(), file_attrs);
    }

    return ::DeleteFileW(file.c_str()) ? true : false;
}

bool debug_object::create_zip(const std::wstring& wsdir, const std::wstring& zipfile)
{
    bool result = false;

    if (!create_empty_zip(zipfile)) {
        return false;
    }

	CoInitialize(NULL);
    try {

        HRESULT                 hr;
        CComPtr<IShellDispatch> spISD;
        CComPtr<Folder>         spToFolder;
        CComVariant             vDir;
        CComVariant             vFile;
        CComVariant             vOpt;

        vDir.vt = VT_BSTR;
        vDir.bstrVal = ::SysAllocStringLen(NULL, (UINT)zipfile.length() + 3);
        RtlSecureZeroMemory(vDir.bstrVal, sizeof(WCHAR)*(zipfile.length() + 3));
        memcpy(vDir.bstrVal, zipfile.c_str(), sizeof(WCHAR)*zipfile.length());

        vFile.vt = VT_BSTR;
        vFile.bstrVal = ::SysAllocStringLen(NULL, (UINT)wsdir.length() + 3);
        RtlSecureZeroMemory(vFile.bstrVal, sizeof(WCHAR)*(wsdir.length() + 3));
        memcpy(vFile.bstrVal, wsdir.c_str(), sizeof(WCHAR)*wsdir.length());

        vOpt.vt = VT_I4;
        vOpt.lVal = 0x0614; //FOF_NO_UI;  //Do not display a progress dialog box, not useful in compression

        hr = ::CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_IShellDispatch, (void **)&spISD);
        if (SUCCEEDED(hr) && NULL != spISD.p) {

            // Destination is our zip file
            hr = spISD->NameSpace(vDir, &spToFolder);
            if (SUCCEEDED(hr) && NULL != spToFolder.p) {

                // Copying and compressing the source files to our zip
                hr = spToFolder->CopyHere(vFile, vOpt);

                // CopyHere() creates a separate thread to copy files and 
                // it may happen that the main thread exits before the 
                // copy thread is initialized. So we put the main thread to sleep 
                // for a second to give time for the copy thread to start.
                int i = 0;
                while (i < 30) {
                    Sleep(1000);
                    HANDLE h = ::CreateFileW(zipfile.c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                    if (INVALID_HANDLE_VALUE != h) {
                        CloseHandle(h);
                        h = INVALID_HANDLE_VALUE;
                        break;
                    }
                }

                // Done
                spToFolder.Release();
                result = true;
            }

            spISD.Release();
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        result = false;
    }
    catch (...) {
        result = false;
    }
    CoUninitialize();

    return result;
}

bool debug_object::create_empty_zip(const std::wstring& zipfile) noexcept
{
    NX::sa_everyone se(GENERIC_READ|GENERIC_WRITE);
    HANDLE h = ::CreateFileW(zipfile.c_str(), GENERIC_WRITE | GENERIC_READ, 0, se, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == h) {
        LOGERR(GetLastError(), L"fail to generate empty zip file (%d, %s)", GetLastError(), zipfile.c_str());
        return false;
    }
    CloseHandle(h);
    h = INVALID_HANDLE_VALUE;
    return true;
}