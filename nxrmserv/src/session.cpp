

#include <Windows.h>
#include <stdio.h>
#include <assert.h>
#include <Wtsapi32.h>
#include <tlhelp32.h>
#include <Userenv.h>
#include <Sddl.h>
#include <Dwmapi.h>
#define SECURITY_WIN32
#include <security.h>

#include <string>
#include <sstream>

#include <nudf\crypto.hpp>
#include <nudf\string.hpp>
#include <nudf\pipe.hpp>
#include <nudf\shared\enginectl.h>
#include <nudf\shared\obdef.h>
#include <nudf\shared\rightsdef.h>

#include <nudf\nxrmres.h>
#include <nudf\resutil.hpp>

#include "nxrmserv.h"
#include "rest.hpp"
#include "session.hpp"
#include "servctrl.hpp"


using namespace NXLOGDB;


//
//  audit db
//

// Max db size is 64 mega bytes
#define MAX_DB_SIZE     64

static const NXLOGDB::record_layout AUDIT_DB_LAYOUT(std::vector<NXLOGDB::field_definition>({
            NXLOGDB::field_definition("timestamp", FIELD_UNSIGNED_INTEGER, FIELD_INTEGER_SIZE, FIELD_FLAG_SEQUENTIAL),
            NXLOGDB::field_definition("archived", FIELD_BOOL, FIELD_BOOL_SIZE, 0),
            NXLOGDB::field_definition("rights", FIELD_UNSIGNED_INTEGER, FIELD_INTEGER_SIZE, 0),
            NXLOGDB::field_definition("flags", FIELD_UNSIGNED_INTEGER, FIELD_INTEGER_SIZE, 0),
            NXLOGDB::field_definition("user_name", FIELD_CHAR, 64, 0),
            NXLOGDB::field_definition("user_id", FIELD_CHAR, 64, 0),
            NXLOGDB::field_definition("user_operation", FIELD_CHAR, 16, 0),
            NXLOGDB::field_definition("app_name", FIELD_CHAR, 64, 0),
            NXLOGDB::field_definition("file_type", FIELD_CHAR, 32, 0),
            NXLOGDB::field_definition("file_path", FIELD_CHAR, MAX_PATH, 0)
            }
            ));

class audit_logdb_conf : public NXLOGDB::db_conf
{
public:
    audit_logdb_conf(const std::string& description)
        : NXLOGDB::db_conf(description, MAX_DB_SIZE, NXLOGDB::BLOCK_1MB, NXLOGDB::SECTOR_4096, NXLOGDB::BLOCK_1MB, NXLOGDB::record_layout(std::vector<NXLOGDB::field_definition>({
            NXLOGDB::field_definition("timestamp", FIELD_UNSIGNED_INTEGER, FIELD_INTEGER_SIZE, FIELD_FLAG_SEQUENTIAL),
            NXLOGDB::field_definition("archived", FIELD_BOOL, FIELD_BOOL_SIZE, 0),
            NXLOGDB::field_definition("rights", FIELD_UNSIGNED_INTEGER, FIELD_INTEGER_SIZE, 0),
            NXLOGDB::field_definition("flags", FIELD_UNSIGNED_INTEGER, FIELD_INTEGER_SIZE, 0),
            NXLOGDB::field_definition("user_name", FIELD_CHAR, 64, 0),
            NXLOGDB::field_definition("user_id", FIELD_CHAR, 64, 0),
            NXLOGDB::field_definition("user_operation", FIELD_CHAR, 16, 0),
            NXLOGDB::field_definition("app_name", FIELD_CHAR, 64, 0),
            NXLOGDB::field_definition("file_type", FIELD_CHAR, 32, 0),
            NXLOGDB::field_definition("file_path", FIELD_CHAR, MAX_PATH, 0)
            }
            )))
    {
    }
    virtual ~audit_logdb_conf()
    {
    }
};

class audit_logdb_record : public NXLOGDB::db_record
{
public:
    audit_logdb_record() : NXLOGDB::db_record()
    {
    }

    audit_logdb_record( unsigned __int64 timestamp,
                        unsigned __int64 rights,
                        unsigned __int64 flags,
                        const std::wstring& user_name,
                        const std::wstring& user_id,
                        const std::wstring& user_operation,
                        const std::wstring& app_name,
                        const std::wstring& file_type,
                        const std::wstring& file_path,
                        const std::string& json_data) : NXLOGDB::db_record(0xFFFFFFFF, std::vector<NXLOGDB::field_value>({
                            NXLOGDB::field_value(timestamp),
                            NXLOGDB::field_value(false),
                            NXLOGDB::field_value(rights),
                            NXLOGDB::field_value(flags),
                            NXLOGDB::field_value(user_name, 64),
                            NXLOGDB::field_value(user_id, 64),
                            NXLOGDB::field_value(user_operation, 16),
                            NXLOGDB::field_value(app_name, 64),
                            NXLOGDB::field_value(file_type, 32),
                            NXLOGDB::field_value(file_path, MAX_PATH)
                        }),
                        std::vector<unsigned char>(json_data.begin(), json_data.end()))
    {
    }

    virtual ~audit_logdb_record()
    {
    }
};

//
//  class user
//
NX::user NX::user::get_user(PSID sid) noexcept
{
    std::vector<wchar_t>    user_name;
    unsigned long           user_name_size = 64;
    std::vector<wchar_t>    domain_name;
    unsigned long           domain_name_size = 64;
    SID_NAME_USE            sid_name_use;

    std::wstring            s_user_sid;
    std::wstring            s_user_name;
    std::wstring            s_user_domain;
    std::wstring            s_user_principle_name;

    if (!IsValidSid(sid)) {
        LOGERR(GetLastError(), L"bad sid");
        return NX::user();
    }

    LPWSTR pwzSid = NULL;
    DWORD dwLength = GetLengthSid(sid);
    if (0 == dwLength) {
        LOGERR(GetLastError(), L"empty sid");
        return NX::user();
    }
    if (!ConvertSidToStringSidW(sid, &pwzSid)) {
        LOGERR(GetLastError(), L"fail to convert sid to string");
        return NX::user();
    }

    // free
    s_user_sid = pwzSid;
    LocalFree(pwzSid);
    pwzSid = NULL;
    std::transform(s_user_sid.begin(), s_user_sid.end(), s_user_sid.begin(), toupper);

    // good, get user name and domain name
    user_name.resize(user_name_size, 0);
    domain_name.resize(user_name_size, 0);
    if (!LookupAccountSidW(NULL, sid, &user_name[0], &user_name_size, &domain_name[0], &domain_name_size, &sid_name_use)) {
        LOGERR(GetLastError(), L"fail to lookup account sid");
        return NX::user();
    }

    s_user_name = &user_name[0];
    s_user_domain = &domain_name[0];
    std::transform(s_user_domain.begin(), s_user_domain.end(), s_user_domain.begin(), toupper);

    assert(!s_user_name.empty());

    if (GLOBAL.host().in_domain()) {

        assert(!s_user_domain.empty());
        if (0 == _wcsicmp(&domain_name[0], GLOBAL.host().name().c_str())) {
            // The domain name is exactly HOSTNAME
            // This is a local user
            s_user_domain.clear();
            s_user_principle_name.clear();
        }
        else {
            // It is a domain user
            // This is in domain
            std::vector<wchar_t>    principle_name;
            unsigned long           principle_name_size = 256;
            std::wstring            sam_account;

            principle_name.resize(principle_name_size, 0);
            sam_account = s_user_domain;
            sam_account += L"\\";
            sam_account += s_user_name;
            if (::TranslateNameW(sam_account.c_str(), NameSamCompatible, NameUserPrincipal, &principle_name[0], &principle_name_size)) {
                s_user_principle_name = &principle_name[0];
            }
            else {
                // Not domain user
                s_user_domain.clear();
                s_user_principle_name.clear();
            }
        }
    }
    else {
        // Not domain user
        s_user_domain.clear();
        s_user_principle_name.clear();
    }

    return NX::user(s_user_sid, s_user_name, s_user_domain, s_user_principle_name);
}

NX::user NX::user::get_token_user(void* token) noexcept
{
    PTOKEN_USER UserInfo = NULL;
    DWORD       ReturnLength = 0;

    std::vector<unsigned char>  buf;
    PTOKEN_USER                 user_info = NULL;
    DWORD                       size = 0;

    GetTokenInformation(token, TokenUser, NULL, 0, &size);
    if (0 == size) {
        LOGERR(GetLastError(), L"fail to get token user buffer size");
        return NX::user();
    }

    size += sizeof(TOKEN_USER);
    buf.resize(size, 0);
    user_info = (PTOKEN_USER)(&buf[0]);

    if (!GetTokenInformation(token, TokenUser, user_info, size, &size)) {
        LOGERR(GetLastError(), L"fail to get token user info");
        return NX::user();
    }

    return get_user(user_info->User.Sid);
}

NX::user NX::user::get_process_user(unsigned long process_id) noexcept
{
    HANDLE h = NULL;
    HANDLE token = NULL;

    if (0 == process_id || process_id == GetCurrentProcessId()) {
        h = GetCurrentProcess();
    }
    else {
        h = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, process_id);
    }

    if (h == NULL) {
        LOGERR(GetLastError(), L"fail to open process");
        return NX::user();
    }

    if (!OpenProcessToken(h, TOKEN_READ, &token)) {
        LOGERR(GetLastError(), L"fail to get process token");
        CloseHandle(h);
        return NX::user();
    }

    NX::user u = get_token_user(token);
    CloseHandle(token);
    CloseHandle(h);

    return u;
}

NX::user NX::user::get_session_user(unsigned long session_id) noexcept
{
    HANDLE token = NULL;

    if (!WTSQueryUserToken(session_id, &token)) {
        LOGERR(GetLastError(), L"fail to get session token");
        return NX::user();
    }

    NX::user u = get_token_user(token);
    CloseHandle(token);

    return u;
}


//
//  class NX::session_app
//

namespace {
class session_guid
{
public:
    session_guid()
    {
        std::vector<UCHAR> md5;
        unsigned __int64 buf[2] = { 0, 0 };
        buf[0] = time(NULL);
        buf[1] = (__int64)GetCurrentProcessId();
        nudf::crypto::ToMd5(buf, sizeof(buf), md5);
        memcpy(&_guid, &md5[0], sizeof(GUID));
        _sguid = nudf::string::FromBytes<wchar_t>(&md5[0], (unsigned long)md5.size());

    }
    ~session_guid() {}

    inline const GUID& guid() const noexcept { return _guid; }
    inline const std::wstring& serialize() const noexcept { return _sguid; }

private:
    GUID            _guid;
    std::wstring    _sguid;
};

class app_control
{
public:
    app_control() {}
    app_control(const std::wstring& port) : _port(port)
    {
    }
    virtual ~app_control()
    {
    }

    void quit() noexcept
    {
        NX::async_pipe::client c(4096);

        try {
            if (c.connect(_port)) {
                NX::web::json::value v = NX::web::json::value::object();
                v[L"code"] = NX::web::json::value::string(L"quit");
                std::wstring ws = v.serialize();
                std::string s = NX::utility::conversions::utf16_to_utf8(ws);
                std::vector<unsigned char> data;
                data.resize(s.length() + 1, 0);
                memcpy(&data[0], s.c_str(), s.length());
                c.write(data);
                c.disconnect();
            }
        }
        catch (std::exception& e) {
            UNREFERENCED_PARAMETER(e);
        }
    }

private:
    std::wstring _port;
};
}

NX::session_app::session_app() : _ssid(-1), _stopping(false)
{
    _hevts[0] = ::CreateEventW(NULL, FALSE, FALSE, NULL);
    _hevts[1] = NULL;
}

NX::session_app::session_app(unsigned long id) : _ssid(id), _stopping(false)
{
    std::wstringstream ss;
    ss << L"nxrmtray-" << (int)id;
    _port = ss.str();
    _hevts[0] = ::CreateEventW(NULL, FALSE, FALSE, NULL);
    _hevts[1] = NULL;
}

NX::session_app::~session_app()
{
    stop();
    if (NULL != _hevts[0]) {
        CloseHandle(_hevts[0]);
        _hevts[0] = NULL;
    }
    if (NULL != _hevts[1]) {
        CloseHandle(_hevts[1]);
        _hevts[1] = NULL;
    }
}

void NX::session_app::start()
{
    if (NULL == _hevts[0]) {
        LOGERR(GetLastError(), L"stop event is not initialized");
        throw std::exception("NX::session_app::start --> stop event is not initialized");
    }

    // always reset stop event
    ::ResetEvent(_hevts[0]);

    // local event used by thread
    HANDLE evt = ::CreateEventW(NULL, FALSE, FALSE, NULL);
    _worker = std::thread(NX::session_app::workthread, this, evt);
    ::WaitForSingleObject(evt, INFINITE);
    CloseHandle(evt); evt = NULL;
    if (NULL == _hevts[1] && _worker.joinable()) {
        _worker.join();
        LOGERR(GetLastError(), L"fail to start tray app");
        throw std::exception("NX::session_app::start --> fail to start tray app");
    }
}

void NX::session_app::stop()
{
    _stopping = true;
    if (NULL != _hevts[0]) {
        SetEvent(_hevts[0]);
    }

    if (_worker.joinable()) {
        _worker.join();
    }
    _stopping = false;
}

void NX::session_app::start_app()
{
    const std::wstring app_image(GLOBAL.dir_bin() + L"\\nxrmtray.exe");

    HANDLE          tk = NULL;
    STARTUPINFOW    si;
    PROCESS_INFORMATION pi;
    DWORD dwError = 0;

    if (!WTSQueryUserToken(_ssid, &tk)) {
        LOGERR(GetLastError(), L"fail to get session token");
        throw std::exception("session_app::start_app --> fail to get session token");
    }
    if (NULL == tk) {
        throw std::exception("session_app::start_app --> fail to get session token");
    }

    // kill existing app
    kill_existing_app(true);

    std::wstring port_seed;
    swprintf_s(nudf::string::tempstr<wchar_t>(port_seed, 64), 64, L"%I64d", GLOBAL.start_time());

    memset(&pi, 0, sizeof(pi));
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    si.lpDesktop = L"WinSta0\\Default";
    if (!::CreateProcessAsUserW(tk, app_image.c_str(), (LPWSTR)port_seed.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        dwError = GetLastError();
        CloseHandle(tk);
        LOGERR(dwError, L"fail to start app");
        throw std::exception("session_app::start_app --> fail to start app");
    }

    _hevts[1] = pi.hProcess;
    CloseHandle(pi.hThread);
    CloseHandle(tk);
}

static std::vector<unsigned long> find_process(const std::wstring& process_name, unsigned long session_id, bool find_all)
{
    std::vector<unsigned long> pids;

    PROCESSENTRY32W pe32;

    memset(&pe32, 0, sizeof(pe32));
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == hSnapshot) {
        return pids;
    }
    if (!Process32FirstW(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        hSnapshot = INVALID_HANDLE_VALUE;
        return pids;
    }

    do {
        const wchar_t* name = wcsrchr(pe32.szExeFile, L'\\');
        name = (NULL == name) ? pe32.szExeFile : (name + 1);
        if (0 == _wcsicmp(process_name.c_str(), name)) {
            unsigned long cur_session_id = -1;
            if (ProcessIdToSessionId(pe32.th32ProcessID, &cur_session_id) && cur_session_id == session_id) {
                pids.push_back(pe32.th32ProcessID);
                if (!find_all) {
                    break;
                }
            }
        }
    } while (Process32NextW(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
    hSnapshot = INVALID_HANDLE_VALUE;

    return pids;
}

void NX::session_app::kill_existing_app(bool safe)
{
    std::vector<unsigned long> pids = find_process(L"nxrmtray.exe", _ssid, false);
    if (!pids.empty()) {
        shutdown_app();
        Sleep(1000);
        if (safe) {
            pids = find_process(L"nxrmtray.exe", _ssid, false);
            if (!pids.empty()) {
                HANDLE h = ::OpenProcess(PROCESS_TERMINATE, FALSE, pids[0]);
                if (h != NULL) {
                    TerminateProcess(h, 0);
                    CloseHandle(h);
                    h = NULL;
                }
            }
        }
    }
}

void NX::session_app::shutdown_app()
{
    app_control appc(_port);
    appc.quit();
    if (NULL != _hevts[1]) {
        CloseHandle(_hevts[1]);
        _hevts[1] = NULL;
    }
}

void NX::session_app::wait()
{
    if (_hevts[1] == NULL) {
        return;
    }

    ResetEvent(_hevts[0]);

    while (TRUE) {
        
        unsigned long result = ::WaitForMultipleObjects(2, _hevts, FALSE, INFINITE);
        if ((WAIT_OBJECT_0 + 1) == result) {

            // client app went away by accident
            bool started = false;
            CloseHandle(_hevts[1]);
            _hevts[1] = NULL;

            if (_stopping) {
                return;
            }

            try {
                start_app();
                started = true;
            }
            catch (...) {
                started = false;
            }

            if (_stopping) {
                return;
            }
        }
        else if (WAIT_OBJECT_0 == result) {
            // stop
            shutdown_app();
            return;
        }
        else {
            LOGASSERT(L"NX::session_app::wait --> Unexpected wait result (%d)", result);
            shutdown_app();
            return;
        }
    }
}

void NX::session_app::workthread(session_app* app, HANDLE h)
{
    // start a new app
    try {
        app->start_app();
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        ; // nothing
    }

    SetEvent(h);
    
    // wait for it
    app->wait();
}


//
//  class NX::session
//

NX::session::session() : _id((unsigned long)-1), _ready(false)
{
}

NX::session::session(unsigned long session_id) : _id(session_id), _ready(false), _ssapp(session_id)
{
    _user = NX::user::get_session_user(session_id);

    HANDLE          tk = NULL;
    if (WTSQueryUserToken(session_id, &tk)) {
        _session_dirs.Load(tk);
        CloseHandle(tk);
        tk = NULL;
    }
}

NX::session::~session()
{
    stop();
}

unsigned short NX::session::session_protocol() const noexcept
{
    LPWSTR          p = NULL;
    unsigned long   size = 0;

    if(!WTSQuerySessionInformationW(NULL, _id, WTSClientProtocolType, &p, &size)) {
        LOGERR(GetLastError(), L"fail to get session protocol");
        return -1;
    }

    unsigned short proto = *((USHORT*)p);
    WTSFreeMemory(p); p = NULL;
    return proto;
}

NX::session& NX::session::operator = (const NX::session& other) noexcept
{
    if(this != &other) {
        _id = other.id();
        _user = other.user();
        _classify_group = other.classify_group();
    }
    return *this;
}

void NX::session::set_dwm_enabled(bool enabled)
{
    _is_dwm_enabled = enabled;
    LOGDBG(L"DWM Composition (Aero) is %s", enabled ? L"enabled" : L"disabled");
}

bool NX::session::check_dwm_status()
{
    HANDLE token = NULL;
    BOOL enabled = false;

    try {

        // try to impersonate
        if (!WTSQueryUserToken(_id, &token)) {
            // Fail to get current session token
            // log & Exit
            LOGWARN(GetLastError(), L"Fail to get session token");
            throw std::exception("Fail to get session token");
        }
        if (!ImpersonateLoggedOnUser(token)) {
            // Fail to impersonate current user
            // log & Exit
            LOGWARN(GetLastError(), L"Fail to impersonate logon user");
            throw std::exception("Fail to impersonate logon user");
        }

        HRESULT hr = DwmIsCompositionEnabled(&enabled);
        if (FAILED(hr)) {
            LOGWARN(hr, L"DwmIsCompositionEnabled failed");
            enabled = FALSE;
        }
        
        // Revert impersonation
        RevertToSelf();
        CloseHandle(token);
        token = NULL;
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        if (NULL != token) {
            CloseHandle(token);
            token = NULL;
        }
    }

    return enabled ? true : false;
}

void NX::session::export_activity_log(const std::wstring& file)
{
    if (!_audit_db.opened()) {
        return;
    }
    _audit_db.export_log(file);
}

void NX::session::dump_db_info(const NXLOGDB::db_conf& conf)
{
    LOGDBG(L"Audit DB Info");
    LOGDBG(L"    Description: %S", conf.get_description().c_str());
    LOGDBG(L"    DB Size: %d MB", conf.get_db_size() / 1048576);
    LOGDBG(L"    DB Flags: %08X", conf.get_db_flags());
    LOGDBG(L"    DB Block Size: %d", conf.get_block_size());
    LOGDBG(L"    DB Sector Size: %d", conf.get_sector_size());
    LOGDBG(L"    DB Sector data Size: %d", conf.get_sector_data_size());
    LOGDBG(L"    DB Sector fields Size: %d", conf.get_fields_size());
    LOGDBG(L"    DB record map offset: 0x%08X", conf.get_record_map_offset());
    LOGDBG(L"    DB record map size: 0x%08X", conf.get_record_map_size());
    LOGDBG(L"    DB record start id: 0x%08X", conf.get_record_start_id());
    LOGDBG(L"    DB record count: 0x%08X", conf.get_record_count());
    LOGDBG(L"    DB record max count: 0x%08X", conf.get_max_record_count());
    LOGDBG(L"    DB sector map offset: 0x%08X", conf.get_sector_map_offset());
    LOGDBG(L"    DB sector map size: 0x%08X", conf.get_sector_map_size());
    LOGDBG(L"    DB sector count: 0x%08X", conf.get_sector_start_offset());
    LOGDBG(L"    DB sector start offset: 0x%08X", conf.get_max_record_count());
    LOGDBG(L"    DB Layout:");
    const NXLOGDB::record_layout& layout = conf.get_record_layout();
    for (int i = 0; i < (int)layout.get_fields_def().size(); i++) {
        LOGDBG(L"      > Field #%d", i);
        LOGDBG(L"          Name:  %S", layout.get_fields_def()[i].get_field_name().c_str());
        LOGDBG(L"          Type:  %s", layout.get_fields_def()[i].get_field_type_name());
        LOGDBG(L"          Flags: %08X", layout.get_fields_def()[i].get_field_flags());
        LOGDBG(L"          Size:  %d", layout.get_fields_def()[i].get_field_size(), layout.get_fields_def()[i].get_field_size());
        /* if (FIELD_CHAR == layout.get_fields_def()[i].get_field_type()) {
        }
        else {
        }*/
    }
}

void NX::session::start() noexcept
{
    // try to find profile directory
    try {

        try {
            _profile.load(_user.id());
        }
        catch (std::exception& e) {
            UNREFERENCED_PARAMETER(e);
            _profile.set_id(_user.id());
            _profile.set_name(_user.principle_name().empty() ? _user.name() : _user.principle_name());
            _profile.set_domain(_user.domain());
            _profile.set_authn_type(GLOBAL.register_info().external_authn() ? NX::profile::ExternalAuthn : NX::profile::LocalAuthn);
            if (!GLOBAL.agent_info().empty()) {
                _profile.save();
                // this must be the first time user logon
                // or the first time user is processed by nxrmserv
                // it means there is no policy for this user
                // triger a force heart-beat (set timestamp to 1970-01-01T00:00:00)
                LOGINF(L"User (%s, %s) first time log on\r\n\t==>initiate a force heartbeat to get latest policy for this user", user().best_name().c_str(), user().id().c_str());
                std::shared_ptr<NX::job> sp = GLOBAL.jobs().get(NX::job::jn_heartbeat);
                if (sp != NULL) {
                    GLOBAL.set_policy_force_update_flag();
                    sp->trigger();
                }
                else {
                    LOGERR(ERROR_NOT_FOUND, L"heartbeat job not exists");
                }
            }
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
    
    // start app
    try {
        _ssapp.start();
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        LOGASSERT(L"fail to start app for session (%d)", _id);
    }

    // load policy bundle
    if (profile().is_external_authn()) {
        if (profile().logged_on()) {
            if (!profile().token().expired()) {
                LOGDBG(L"session (%d) start with existing external authn token: (%s, %s, %s)", id(), profile().token().user_name().c_str(), profile().token().user_domain().c_str(), profile().token().user_id().c_str());
                set_session_policy_bundle(GLOBAL.policy_bundle());
            }
            else {
                LOGDBG(L"session (%d) start with expired external authn token: (%s, %s, %s)", id(), profile().token().user_name().c_str(), profile().token().user_domain().c_str(), profile().token().user_id().c_str());
                LOGDBG(L"  --> clear this token");
                _profile.token().clear();
                _profile.save();
            }
        }
        else {
            LOGDBG(L"session (%d) start without external authn (policy bundle is empty because external authn is required)", id());
        }
    }
    else {
        set_session_policy_bundle(GLOBAL.policy_bundle());
        LOGDBG(L"session (%d) start with current windows authn (%s, %s)", id(), user().best_name().c_str(), user().id().c_str());
    }

    // initialize audit db
    std::wstring audit_db_file = GLOBAL.dir_profiles() + L"\\" + this->user().id() + L"\\audit.db";
    try {
        _audit_db.open(audit_db_file, false);
        LOGDBG(L"session (%d, user: %s, %s) open audit db (%s)", id(), user().best_name().c_str(), user().id().c_str(), audit_db_file.c_str());
        NXLOGDB::db_conf audit_db_conf = _audit_db.get_conf();
        dump_db_info(audit_db_conf);
        // Does existing db match current format?
        bool not_match_flag = (AUDIT_DB_LAYOUT.get_fields_def().size() != audit_db_conf.get_record_layout().get_fields_def().size());
        if (!not_match_flag) {
            for (int i = 0; i < (int)AUDIT_DB_LAYOUT.get_fields_def().size(); i++) {
                if (AUDIT_DB_LAYOUT.get_fields_def()[i].get_field_type() != audit_db_conf.get_record_layout().get_fields_def()[i].get_field_type()) {
                    not_match_flag = true;
                    break;
                }
                if (AUDIT_DB_LAYOUT.get_fields_def()[i].get_field_size() != audit_db_conf.get_record_layout().get_fields_def()[i].get_field_size()) {
                    not_match_flag = true;
                    break;
                }
            }
        }
        if (not_match_flag) {
            _audit_db.close();
            std::wstring backup_audit_db_file = audit_db_file + L".old";
            ::MoveFileEx(audit_db_file.c_str(), backup_audit_db_file.c_str(), MOVEFILE_REPLACE_EXISTING);
            LOGWARN(ERROR_INVALID_DATATYPE, L"DB format doesn't match, backup old db and create a new one");
            throw std::exception("db format doesn't match");
        }
    }
    catch (...) {
        std::wstring description(L"audit db for user ");
        description += this->user().id();
        const audit_logdb_conf new_db_conf(std::string(description.begin(), description.end()));
        try {
            _audit_db.create(audit_db_file, new_db_conf);
            LOGINF(L"session (%d, user: %s, %s) create audit db (%s)", id(), user().best_name().c_str(), user().id().c_str(), audit_db_file.c_str());
            dump_db_info(new_db_conf);
        }
        catch (...) {
            LOGERR(GetLastError(), L"session (%d, user: %s, %s) fail to create audit db (%s)", id(), user().best_name().c_str(), user().id().c_str(), audit_db_file.c_str());
        }
    }


    // finally, send serv status to app
    set_serv_status();
}

void NX::session::stop() noexcept
{
    GLOBAL.drv_core().set_overlay_bitmap_status(_id, false);
    if (_profile.dirty()) {
        _profile.save();
    }
    _id = (unsigned long)-1;
    _ssapp.stop();
    _user.clear();
    _profile.clear();
    _audit_db.close();
}

void NX::session::notify(const std::wstring& title, const std::wstring& msg)
{
    NX::async_pipe::client c(4096);

    try {
        if (c.connect(_ssapp.app_port())) {
            NX::web::json::value v = NX::web::json::value::object();
            v[L"code"] = NX::web::json::value::string(L"notify");
            v[L"notify_title"] = NX::web::json::value::string(title);
            v[L"notify_msg"] = NX::web::json::value::string(msg);
            std::wstring ws = v.serialize();
            std::string s = NX::utility::conversions::utf16_to_utf8(ws);
            std::vector<unsigned char> data;
            data.resize(s.length() + 1, 0);
            memcpy(&data[0], s.c_str(), s.length());
            c.write(data);
            c.disconnect();
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
}

void NX::session::set_policy_time(const std::wstring& time)
{
    NX::async_pipe::client c(4096);

    std::wstring ws_policy_time;

    if (time.empty()) {
        ws_policy_time = L"N/A";
    }
    else {
        FILETIME ft = { 0, 0 };
        SYSTEMTIME st = { 0 };
        bool utc = true;
        if (nudf::string::ToSystemTime<wchar_t>(time, &ft, &utc)) {
            if (utc) {
                FILETIME utc_ft = { 0, 0 };
                utc_ft.dwHighDateTime = ft.dwHighDateTime; utc_ft.dwLowDateTime = ft.dwLowDateTime;
                FileTimeToLocalFileTime(&utc_ft, &ft);
            }
            FileTimeToSystemTime(&ft, &st);
            swprintf_s(nudf::string::tempstr<wchar_t>(ws_policy_time, 128), 128, L"%04d-%02d-%02d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
        }
        else {
            ws_policy_time = L"N/A";
        }
    }

    try {
        if (c.connect(_ssapp.app_port())) {
            NX::web::json::value v = NX::web::json::value::object();
            v[L"code"] = NX::web::json::value::string(L"set_policy_time");
            v[L"timestamp"] = NX::web::json::value::string(ws_policy_time);
            std::wstring ws = v.serialize();
            std::string s = NX::utility::conversions::utf16_to_utf8(ws);
            std::vector<unsigned char> data;
            data.resize(s.length() + 1, 0);
            memcpy(&data[0], s.c_str(), s.length());
            c.write(data);
            c.disconnect();
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
}

void NX::session::set_heartbeat_time(const std::wstring& time)
{
    NX::async_pipe::client c(4096);
    
    try {
        if (c.connect(_ssapp.app_port())) {
            NX::web::json::value v = NX::web::json::value::object();
            v[L"code"] = NX::web::json::value::string(L"set_heartbeat_time");
            v[L"timestamp"] = NX::web::json::value::string(GLOBAL.agent_info().heartbeat_time_text());
            std::wstring ws = v.serialize();
            std::string s = NX::utility::conversions::utf16_to_utf8(ws);
            std::vector<unsigned char> data;
            data.resize(s.length() + 1, 0);
            memcpy(&data[0], s.c_str(), s.length());
            c.write(data);
            c.disconnect();
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
}

NX::web::json::value NX::session::build_serv_status_response()
{
    // log on user
    std::wstring full_user_name;
    std::wstring full_user_id;

    if (profile().is_external_authn()) {
        if (logged_on()) {
            full_user_name = profile().token().full_user_name();
            full_user_id = profile().token().user_id();
            if (full_user_name.empty()) {
                full_user_name = L"N/A";
            }
        }
        else {
            full_user_name = L"N/A";
            full_user_id.clear();
        }
    }
    else {
        // internal authn
        full_user_name = user().is_domain_user() ? user().principle_name() : user().name();
        full_user_id = user().id();
    }

    // build response
    NX::web::json::value response = NX::web::json::value::object();

    response[L"result"] = NX::web::json::value::number(ERROR_SUCCESS);
    response[L"connected"] = NX::web::json::value::boolean(GLOBAL.connected());
    response[L"debug"] = NX::web::json::value::boolean(GLOBAL.log().GetAcceptLevel() >= LOGDEBUG);
    response[L"policy_time"] = NX::web::json::value::string(GLOBAL.policy_bundle().local_bundle_time().empty() ? L"N/A" : GLOBAL.policy_bundle().local_bundle_time());
    response[L"heartbeat_time"] = NX::web::json::value::string(GLOBAL.agent_info().heartbeat_time_text());
    response[L"product_version"] = NX::web::json::value::string(GLOBAL.product().version().version_str());
    response[L"authn_type"] = NX::web::json::value::string(profile().is_local_authn() ? L"local" : L"external");
    response[L"logon_server"] = NX::web::json::value::string(GLOBAL.register_info().server());
    response[L"logon_user"] = NX::web::json::value::string(full_user_name);
    response[L"logon_user_id"] = NX::web::json::value::string(full_user_id);

    return response;
}

NX::web::json::value NX::session::build_serv_status_response2()
{
    // log on user
    std::wstring full_user_name;
    std::wstring full_user_id;

    if (profile().is_external_authn()) {
        if (logged_on()) {
            full_user_name = profile().token().full_user_name();
            full_user_id = profile().token().user_id();
            if (full_user_name.empty()) {
                full_user_name = L"N/A";
            }
        }
        else {
            full_user_name = L"N/A";
            full_user_id.clear();
        }
    }
    else {
        // internal authn
        full_user_name = user().is_domain_user() ? user().principle_name() : user().name();
        full_user_id = user().id();
    }

    // build response
    NX::web::json::value response = NX::web::json::value::object();

    response[NXSERV_REQUEST_PARAM_RESULT] = NX::web::json::value::number(ERROR_SUCCESS);
    response[NXSERV_REQUEST_PARAM_CONNECTED] = NX::web::json::value::boolean(GLOBAL.connected());
    response[NXSERV_REQUEST_PARAM_DEBUG_MODE] = NX::web::json::value::boolean(GLOBAL.log().GetAcceptLevel() >= LOGDEBUG);
    response[NXSERV_REQUEST_PARAM_POLICY_TIME] = NX::web::json::value::string(GLOBAL.policy_bundle().local_bundle_time().empty() ? L"N/A" : GLOBAL.policy_bundle().local_bundle_time());
    response[NXSERV_REQUEST_PARAM_HEARTBEAT_TIME] = NX::web::json::value::string(GLOBAL.agent_info().heartbeat_time_text());
    response[NXSERV_REQUEST_PARAM_PRODUCT_VERSION] = NX::web::json::value::string(GLOBAL.product().version().version_str());
    response[NXSERV_REQUEST_PARAM_LOGON_TENANT] = NX::web::json::value::string(GLOBAL.register_info().tenant_id());
    response[NXSERV_REQUEST_PARAM_LOGON_USER] = NX::web::json::value::string(full_user_name);
    response[NXSERV_REQUEST_PARAM_LOGON_TIME] = NX::web::json::value::string(full_user_id);

    return response;
}

void NX::session::set_serv_status()
{
    NX::async_pipe::client c(4096);

    try {

        NX::web::json::value response = build_serv_status_response();
        std::wstring ws = response.serialize();
        std::string s = NX::utility::conversions::utf16_to_utf8(ws);
        std::vector<unsigned char> data;
        data.resize(s.length() + 1, 0);
        memcpy(&data[0], s.c_str(), s.length());

        if (c.connect(_ssapp.app_port())) {
            c.write(data);
            c.disconnect();
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
}

void NX::session::send_update_response(bool updated)
{
    NX::async_pipe::client c(4096);

    try {

        // build response
        NX::web::json::value response = NX::web::json::value::object();

        response[L"code"] = NX::web::json::value::string(L"update_response");
        response[L"result"] = NX::web::json::value::boolean(updated);
        response[L"policy_time"] = NX::web::json::value::string(GLOBAL.policy_bundle().local_bundle_time().empty() ? L"N/A" : GLOBAL.policy_bundle().local_bundle_time());
        response[L"heartbeat_time"] = NX::web::json::value::string(GLOBAL.agent_info().heartbeat_time_text());

        std::wstring ws = response.serialize();
        std::string s = NX::utility::conversions::utf16_to_utf8(ws);
        std::vector<unsigned char> data;
        data.resize(s.length() + 1, 0);
        memcpy(&data[0], s.c_str(), s.length());

        if (c.connect(_ssapp.app_port())) {
            c.write(data);
            c.disconnect();
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
}

void NX::session::show_logon_ui()
{
    NX::async_pipe::client c(4096);

    try {
        if (c.connect(_ssapp.app_port())) {
            NX::web::json::value v = NX::web::json::value::object();
            v[L"code"] = NX::web::json::value::string(L"show_logon_ui");
            if (!GLOBAL.register_info().authn_domains().empty()) {
                v[L"logon_domain"] = NX::web::json::value::string(GLOBAL.register_info().authn_domains()[0]);
            }
            std::wstring ws = v.serialize();
            std::string s = NX::utility::conversions::utf16_to_utf8(ws);
            std::vector<unsigned char> data;
            data.resize(s.length() + 1, 0);
            memcpy(&data[0], s.c_str(), s.length());
            c.write(data);
            c.disconnect();
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
}

void NX::session::logoff() noexcept
{
    if (_profile.is_external_authn()) {
        LOGINF(L"User Logoff\r\n\t - User: %s\r\n\t - Id: %s", profile().token().full_user_name().c_str(), profile().token().user_id().c_str());
        // Notify driver to clean cache because user has logged off
        GLOBAL.drv_flt().set_policy_changed();
		GLOBAL.drv_core().increase_policy_sn();
        // update profile
        _profile.token().clear();
        _profile.save();
        set_session_policy_bundle(NX::EVAL::policy_bundle());
    }
}

long NX::session::logon(const std::wstring& name, const std::wstring& domain, const std::wstring& password) noexcept
{
    long result = 0;

    // make sure agent_info is NOT empty
    assert(!GLOBAL.agent_info().empty());

    // send authn request
    try {

        bool changed = false;
        NX::rest_authn_result res;
        if(!GLOBAL.rest().request_authn(name, domain, password, res))
            throw std::exception("REST call failed: request_authn");

        if (!res.ok()) {
            result = res.result();
            LOGERR(res.result(), L"Login failed: %d, %s", res.result(), res.error_message().c_str());
            throw std::exception("server return error");
        }

        // good, succeed, update log on user's information
        _profile.set_authn_token(NX::authn_token(name, res.user_name(), res.user_id(), domain, res.expire_time()));
        _profile.authn_attributes() = res.user_attributes();
        _profile.save();
        result = 200;

        // Need to rebuild session bundle
        set_session_policy_bundle(GLOBAL.policy_bundle());
        // Notify driver to clean cache because the policy has been changed for this user
        GLOBAL.drv_flt().set_policy_changed();
		GLOBAL.drv_core().increase_policy_sn();

        std::wstring title = nudf::util::res::LoadMessage(GLOBAL.res_module(), IDS_PRODUCT_NAME, 64, LANG_NEUTRAL, L"NextLabs Rights Management");
        std::wstring info = nudf::util::res::LoadMessageEx(GLOBAL.res_module(), IDS_NOTIFY_USER_LOGON, 1024, LANG_NEUTRAL, L"User %s has logged in", profile().token().full_user_name().c_str());
        this->notify(title, info);

        LOGINF(L"User Logon\r\n\t - User: %s\r\n\t - Domain: %s\r\n\t - Principle Name: %s\r\n\t - Id: %s", name.c_str(), domain.c_str(), profile().token().principle_name().c_str(), profile().token().user_id().c_str());

        // is this user first time logon?
        if (!GLOBAL.external_users().user_exists(res.user_id())) {
            GLOBAL.external_users().insert(res.user_id(), name, domain);
            GLOBAL.external_users().save();

            LOGINF(L"External User (%s, %s, %s) first time log on\r\n\t==>initiate a force heartbeat to get latest policy for this user", name.c_str(), domain.c_str(), res.user_id().c_str());
            std::shared_ptr<NX::job> sp = GLOBAL.jobs().get(NX::job::jn_heartbeat);
            if (sp != NULL) {
                GLOBAL.set_policy_force_update_flag();
                sp->trigger();
            }
            else {
                LOGERR(ERROR_NOT_FOUND, L"heartbeat job not exists");
            }
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        if (0 == result) {
            result = 461;
        }
    }

    // Trim Working Set
//#ifndef _DEBUG
//    NX::sys::trim_working_set();
//#endif
    return result;
}

void NX::session::update_policy(NX::session* ss)
{
}

void NX::session::update_user_attributes(NX::session* ss)
{
}

void NX::session::session_app_monitor(NX::session* ss)
{
}

void NX::session::set_session_policy_bundle(const NX::EVAL::policy_bundle& bundle) noexcept
{
    exclusive_locker locker(_bundle_lock);
    _bundle.clear();
    _classify_group.clear();

    if (bundle.empty()) {
        // empty
        return;
    }

    std::wstring authn_id;

    if (profile().is_external_authn()) {
        if (!profile().logged_on()) {
            LOGASSERT(L"fail to set session policy bundle because authn is required");
        }
        authn_id = profile().token().user_id();
    }
    else {
        authn_id = user().id();
    }
    
    const std::pair<std::wstring,std::vector<std::wstring>>& user_group = bundle.user_group(authn_id);
    _user_context = user_group.first;

    std::map<std::wstring, std::wstring> overlay_parameters;

    std::for_each(bundle.policy_map().begin(), bundle.policy_map().end(), [&](const std::pair<std::wstring, std::shared_ptr<NX::EVAL::policy>>& it) {
        // if this policy match the user information, put it to the buffer
        bool match_user_group = false;
        bool user_group_exist = false;
        it.second->subject_expr()->traverse_expr([&](const NX::EVAL::details::expression::condition_node* p) {
            if (p->get_condition()->name() == L"user.group") {
                user_group_exist = true;
                std::wstring group_list_str = p->get_condition()->value().serialize();
                std::vector<std::wstring> group_list;
                nudf::string::Split<wchar_t>(group_list_str, L',', group_list);
                bool found = false;
                for (auto git = group_list.begin(); git != group_list.end(); ++git) {
                    found = (user_group.second.end() != std::find(user_group.second.begin(), user_group.second.end(), *git));
                    if (found) {
                        match_user_group = true;
                        break;
                    }
                }
            }
        });
        if (!user_group_exist || match_user_group) {
            _bundle.push_back(it.second);

            // get classify policy
            auto pos = std::find_if(it.second->obligations().begin(), it.second->obligations().end(), [&](std::shared_ptr<NX::EVAL::obligation> sp)->bool {
                return ((0 == _wcsicmp(sp->name().c_str(), OB_NAME_CLASSIFY)) && (sp->parameters().end() != sp->parameters().find(L"group")));
            });
            if (pos != it.second->obligations().end()) {
                auto ob_it = (*pos)->parameters().find(L"group");
                assert((*pos)->parameters().end() != ob_it);
                if (_classify_group != L"default" && 0 == _wcsicmp((*ob_it).second.c_str(), L"default")) {
                    ; // nothing, don't overwrite existing classify group with default group
                }
                else {
                    _classify_group = (*ob_it).second;
                    // we also set the group information
                    std::transform(_classify_group.begin(), _classify_group.end(), _classify_group.begin(), tolower);
                }
            }

            // get overlay policy
            pos = std::find_if(it.second->obligations().begin(), it.second->obligations().end(), [&](std::shared_ptr<NX::EVAL::obligation> sp)->bool {
                return (0 == _wcsicmp(sp->name().c_str(), OB_NAME_OVERLAY));
            });
            if (pos != it.second->obligations().end()) {
                overlay_parameters = (*pos)->parameters();
            }
        }
    });

    if (this->logged_on()) {
        create_overlay_image(overlay_parameters);
        GLOBAL.drv_core().set_overlay_bitmap_status(_id, true);
    }

    // Trim Working Set
//#ifndef _DEBUG
//    NX::sys::trim_working_set();
//#endif

    if (_classify_group.empty()) {
        LOGWARN(ERROR_INVALID_DATA, L"fail to get session (%d) classify group, set it to \"default\"", this->id());
        _classify_group = L"default";
    }

    LOGDBG(L"\r\nSession (%d) Policy Bundle:\r\n\tcount:\t%d\r\n\tgroup:\t%s", this->id(), _bundle.size(), _classify_group.c_str());
    std::for_each(_bundle.begin(), _bundle.end(), [](std::shared_ptr<NX::EVAL::policy> p) {
        LOGDBG(L"    Policy #%s: %s", p->id().c_str(), p->name().c_str());
    });
}

template <SE_OBJECT_TYPE OBJTYPE>
void add_ace_to_object_security_descriptor(
    const std::wstring& object_name,
    const std::wstring& trustee,
    TRUSTEE_FORM trustee_form,
    unsigned long access_rights,
    ACCESS_MODE access_mode,
    unsigned long inheritance
    )
{
    PACL pOldDACL = NULL, pNewDACL = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    EXPLICIT_ACCESS ea;

    try {

        DWORD dwRes = 0;
        std::string err_info;

        dwRes = GetNamedSecurityInfoW(object_name.c_str(),
            OBJTYPE,
            DACL_SECURITY_INFORMATION,
            NULL,
            NULL,
            &pOldDACL,
            NULL,
            &pSD);
        if (ERROR_SUCCESS != dwRes) {
            err_info = "fail to get security info (";
            err_info += std::to_string((int)GetLastError());
            err_info += ")";
            throw std::exception(err_info.c_str());
        }

        // Initialize an EXPLICIT_ACCESS structure for the new ACE.
        ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
        ea.grfAccessPermissions = access_rights;
        ea.grfAccessMode = access_mode;
        ea.grfInheritance = inheritance;
        ea.Trustee.TrusteeForm = trustee_form;
        ea.Trustee.ptstrName = (LPWSTR)trustee.c_str();

        // Create a new ACL that merges the new ACE
        // into the existing DACL.
        dwRes = SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL);
        if (ERROR_SUCCESS != dwRes) {
            err_info = "fail to set entry in acl (";
            err_info += std::to_string((int)GetLastError());
            err_info += ")";
            throw std::exception(err_info.c_str());
        }

        // Attach the new ACL as the object's DACL.
        dwRes = SetNamedSecurityInfoW((LPWSTR)object_name.c_str(),
            OBJTYPE,
            DACL_SECURITY_INFORMATION,
            NULL,
            NULL,
            pNewDACL,
            NULL);
        if (ERROR_SUCCESS != dwRes) {
            err_info = "fail to set security info (";
            err_info += std::to_string((int)GetLastError());
            err_info += ")";
            throw std::exception(err_info.c_str());
        }

        if (pSD != NULL) {
            LocalFree((HLOCAL)pSD);
            pSD = NULL;
        }
        if (pNewDACL != NULL) {
            LocalFree((HLOCAL)pNewDACL);
            pNewDACL = NULL;
        }
    }
    catch (const std::exception& e) {
        if (pSD != NULL) {
            LocalFree((HLOCAL)pSD);
            pSD = NULL;
        }
        if (pNewDACL != NULL) {
            LocalFree((HLOCAL)pNewDACL);
            pNewDACL = NULL;
        }
        throw e;
    }
}

void NX::session::create_overlay_image(const std::map<std::wstring, std::wstring>& parameters)
{
    std::wstring file;
    std::wstring shost;
    std::wstring suser;
    std::wstring text = L"$(User)\\n$(Host)";
    std::wstring font_name = L"Sitka Text";
    int font_size = 16;
    std::wstring text_color = L"Gray";
    int transratio = 90;
    int rotation = -45;
    
    file = GLOBAL.dir_profiles() + L"\\" + this->user().id();
    file += L"\\nxrm_overlay.png";
    //file = _session_dirs.GetTempDir() + L"\\nxrm_overlay.png";
    shost = GLOBAL.host().in_domain() ? GLOBAL.host().fully_qualified_domain_name() : GLOBAL.host().name();

    if (profile().is_external_authn()) {
        suser = profile().token().netbios_user_name();
        if (suser.empty()) {
            suser = profile().token().full_user_name();
        }
        if (!profile().logged_on()) {
            LOGWARN(GetLastError(), L"get obligation for an unauthenticated user (external authn) - session id (%d), win user (%s)", id(), user().best_name().c_str());
        }
    }
    else {
        if (user().is_domain_user()) {
            suser = user().domain() + L"\\";
        }
        suser += user().name();
    }

    auto pos = parameters.find(L"text");
    if (pos != parameters.end() && (*pos).second.length() != 0) {
        text = (*pos).second;
    }

    pos = parameters.find(L"transparency");
    if (pos != parameters.end() && (*pos).second.length() != 0) {
        int nTrans = 90;
        if (nudf::string::ToInt<wchar_t>((*pos).second, &nTrans)) {
            transratio = nTrans;
        }
    }
    pos = parameters.find(L"fontname");
    if (pos != parameters.end() && (*pos).second.length() != 0) {
        font_name = (*pos).second;
    }
    pos = parameters.find(L"fontsize");
    if (pos != parameters.end() && (*pos).second.length() != 0) {
        int nSize = 0;
        if (nudf::string::ToInt<wchar_t>((*pos).second, &nSize)) {
            font_size = nSize;
        }
    }
    pos = parameters.find(L"textcolor");
    if (pos != parameters.end() && (*pos).second.length() != 0) {
        text_color = (*pos).second;
    }
    pos = parameters.find(L"rotation");
    if (pos != parameters.end() && (*pos).second.length() != 0) {

        if (0 == _wcsicmp((*pos).second.c_str(), L"None")) {
            rotation = 0;
        }
        else if (0 == _wcsicmp((*pos).second.c_str(), L"Clockwise")) {
            rotation = -45;
        }
        else {
            rotation = 45;
        }
    }

    NX::overlay::overlay_object obj(shost, suser, text, font_name, font_size, text_color, transratio, rotation);

    LOGDBG(L"Create Overlay Image for session (%d)", id());
    LOGDBG(L"   - Image: %s", file.c_str());
    LOGDBG(L"   - Host: %s", shost.c_str());
    LOGDBG(L"   - User: %s", suser.c_str());
    LOGDBG(L"   - Text: %s", obj.get_text().c_str());

    _overlay_image.create(file, obj);

    // set file's ACL
    try {

        add_ace_to_object_security_descriptor<SE_FILE_OBJECT>(
            file,
            L"Everyone",
            TRUSTEE_IS_NAME,
            GENERIC_READ,
            GRANT_ACCESS,
            NO_INHERITANCE
            );
        add_ace_to_object_security_descriptor<SE_FILE_OBJECT>(
            file,
            L"Users",
            TRUSTEE_IS_NAME,
            GENERIC_READ,
            GRANT_ACCESS,
            NO_INHERITANCE
            );
    }
    catch (std::exception& e) {
        LOGERR(L"Fail to set overlay_image DACL: %S", e.what());
    }
}

void NX::session::evaluate(NX::EVAL::eval_object& eval_obj) noexcept
{
    if (!profile().logged_on()) {
        // user not logon yet
        return;
    }

    shared_locker locker(_bundle_lock);
    std::for_each(_bundle.begin(), _bundle.end(), [&](std::shared_ptr<NX::EVAL::policy> p) {
        if (p->evaluate(eval_obj)) {
            eval_obj.result()->hit_policy(p);
        }
    });
}

void NX::session::audit(const NX::EVAL::eval_object& eo)
{
    unsigned __int64 flags = AUDIT_TYPE_LOG_AUTHORIZATION;


    try {

        nudf::time::CTime timestamp;
        std::wstring log_element;

        std::wstring user_name;
        std::wstring user_id;
        std::wstring user_tenant_id;
        std::wstring user_context = eo.user_context();
        std::vector<std::pair<std::wstring, std::wstring>> user_attrs;
        std::wstring app_path;
        std::wstring app_name;
        std::wstring app_publisher;
        std::vector<std::pair<std::wstring, std::wstring>> app_attrs;
        std::wstring host_name;
        std::wstring host_ipv4;
        std::vector<std::pair<std::wstring, std::wstring>> host_attrs;
        std::wstring fso_path;
        std::wstring fso_name;
        std::wstring fso_type;
        std::vector<std::pair<std::wstring, std::wstring>> res_attrs;
        std::wstring env_heartbeat;
        std::wstring env_current_time;
        std::vector<std::pair<std::wstring, std::wstring>> env_attrs;

        std::vector<std::pair<std::wstring, std::wstring>> policies;

        timestamp.Now();

        std::for_each(eo.attributes().cbegin(), eo.attributes().cend(), [&](const NX::EVAL::attribute_multimap::value_type& it) {
            if (0 == _wcsicmp(it.first.c_str(), L"user.name")) {
                user_name = it.second.serialize();
            }
            else if (0 == _wcsicmp(it.first.c_str(), L"user.id")) {
                user_id = it.second.serialize();
                std::transform(user_id.begin(), user_id.end(), user_id.begin(), toupper);
            }
            else if (0 == _wcsicmp(it.first.c_str(), L"user.tenant_id")) {
                user_tenant_id = it.second.serialize();
            }
            else if (0 == _wcsicmp(it.first.c_str(), L"application.path")) {
                app_path = it.second.serialize();
                auto pos = app_path.find_last_of(L'\\');
                app_name = (pos != std::wstring::npos) ? app_path.substr(pos + 1) : app_path;
            }
            else if (0 == _wcsicmp(it.first.c_str(), L"application.publisher")) {
                app_publisher = it.second.serialize();
            }
            else if (0 == _wcsicmp(it.first.c_str(), L"host.name")) {
                host_name = it.second.serialize();
            }
            else if (0 == _wcsicmp(it.first.c_str(), L"host.inet_addr")) {
                host_ipv4 = it.second.serialize();
            }
            else if (0 == _wcsicmp(it.first.c_str(), L"resource.fso.path")) {
                fso_path = it.second.serialize();
                auto pos = fso_path.find_last_of(L'\\');
                fso_name = (pos != std::wstring::npos) ? fso_path.substr(pos + 1) : fso_path;
                pos = fso_name.find_last_of(L'.');
                fso_type = (pos != std::wstring::npos) ? fso_name.substr(pos) : std::wstring();
            }
            else if (0 == _wcsicmp(it.first.c_str(), L"environment.time_since_last_heartbeat")) {
                env_heartbeat = it.second.serialize();
            }
            else if (0 == _wcsicmp(it.first.c_str(), L"environment.current_time.identity")) {
                env_current_time = it.second.serialize();
            }
            else if (boost::algorithm::istarts_with(it.first, L"user.")) {
                user_attrs.push_back(std::pair<std::wstring, std::wstring>(it.first.substr(5), it.second.serialize()));
            }
            else if (boost::algorithm::istarts_with(it.first, L"application.")) {
                app_attrs.push_back(std::pair<std::wstring, std::wstring>(it.first.substr(12), it.second.serialize()));
            }
            else if (boost::algorithm::istarts_with(it.first, L"host.")) {
                host_attrs.push_back(std::pair<std::wstring, std::wstring>(it.first.substr(5), it.second.serialize()));
            }
            else if (boost::algorithm::istarts_with(it.first, L"resource.fso.")) {
                res_attrs.push_back(std::pair<std::wstring, std::wstring>(it.first.substr(13), it.second.serialize()));
            }
            else if (boost::algorithm::istarts_with(it.first, L"environment.")) {
                env_attrs.push_back(std::pair<std::wstring, std::wstring>(it.first.substr(12), it.second.serialize()));
            }
            else {
                ; // ignore unknown type
            }
        });

        // user information
        NX::web::json::value json_user_info = NX::web::json::value::object();
        json_user_info[L"name"] = NX::web::json::value::string(user_name);
        json_user_info[L"id"] = NX::web::json::value::string(user_id);
        for (int i = 0; i < (int)user_attrs.size(); i++) {
            json_user_info[user_attrs[i].first] = NX::web::json::value::string(user_attrs[i].second);
        }

        // app information
        NX::web::json::value json_app_info = NX::web::json::value::object();
        json_app_info[L"path"] = NX::web::json::value::string(app_path);
        json_app_info[L"publisher"] = NX::web::json::value::string(app_publisher);
        for (int i = 0; i < (int)app_attrs.size(); i++) {
            json_app_info[app_attrs[i].first] = NX::web::json::value::string(app_attrs[i].second);
        }

        // host information
        NX::web::json::value json_host_info = NX::web::json::value::object();
        json_host_info[L"name"] = NX::web::json::value::string(host_name);
        json_host_info[L"inet_addr"] = NX::web::json::value::string(host_ipv4);
        for (int i = 0; i < (int)host_attrs.size(); i++) {
            json_host_info[host_attrs[i].first] = NX::web::json::value::string(host_attrs[i].second);
        }

        // environment information
        NX::web::json::value json_env_info = NX::web::json::value::object();
        json_env_info[L"time_since_last_heartbeat"] = NX::web::json::value::string(host_name);
        json_env_info[L"current_time"] = NX::web::json::value::string(env_current_time);
        for (int i = 0; i < (int)host_attrs.size(); i++) {
            json_env_info[env_attrs[i].first] = NX::web::json::value::string(env_attrs[i].second);
        }

        // resource
        NX::web::json::value json_res_attrs = NX::web::json::value::array();
        for (int i = 0; i < (int)res_attrs.size(); i++) {
            std::wstring res_attr_value = res_attrs[i].first + L"\\" + res_attrs[i].second;
            json_res_attrs[i] = NX::web::json::value::string(res_attr_value);
        }
        NX::web::json::value json_res_info = NX::web::json::value::object();
        json_host_info[L"path"] = NX::web::json::value::string(fso_path);
        json_host_info[L"tags"] = json_res_attrs;

        NX::web::json::value json_policy_info = NX::web::json::value::object();
        std::for_each(eo.result()->hit_policies().begin(), eo.result()->hit_policies().end(), [&](const std::pair<std::wstring, std::shared_ptr<NX::EVAL::policy>>& it) {
            json_policy_info[it.first] = NX::web::json::value::string(it.second->name());
        });
        
        NX::web::json::value json_data = NX::web::json::value::object();
        json_data[L"operation"] = NX::web::json::value::string(AUTHORIZATION_NAME);
        json_data[L"rights"] = NX::web::json::value::string(eo.result()->final_rights().serialize());
        json_data[L"user"] = json_user_info;
        json_data[L"application"] = json_app_info;
        json_data[L"host"] = json_host_info;
        json_data[L"environment"] = json_env_info;
        json_data[L"resource"] = json_res_info;
        json_data[L"policies"] = json_policy_info;

        std::string opaque_data = NX::utility::conversions::utf16_to_utf8(json_data.serialize());

        _audit_db.push_record(audit_logdb_record(timestamp.ToUll(),
            eo.result()->final_rights(),
            flags,
            user_name,
            user_id,
            AUTHORIZATION_NAME,
            app_name,
            fso_type,
            fso_name,
            opaque_data));
    }
    catch (std::exception& e) {
        LOGERR(GetLastError(), L"Fail to write audit log: %S", e.what());
    }
}


void NX::session::audit(const std::wstring& operation_name, bool allowed, const std::wstring& resource_path, const std::wstring& app_name, const std::wstring& description)
{
    unsigned __int64 final_rights = 0;
    nudf::time::CTime   timestamp;
    unsigned __int64 flags = AUDIT_TYPE_LOG_OPERATION;

    std::wstring fso_name;
    std::wstring fso_type;

    timestamp.Now();

    try {

        if (allowed) {
            final_rights = DisplayNameToRights(operation_name.c_str());
        }

        auto pos = resource_path.find_last_of(L'\\');
        fso_name = (pos == std::wstring::npos) ? resource_path : resource_path.substr(pos + 1);
        pos = fso_name.find_last_of(L'.');
        fso_type = (pos == std::wstring::npos) ? L"" : fso_name.substr(pos);


        NX::web::json::value opaque_object = NX::web::json::value::object();
        opaque_object[L"description"] = NX::web::json::value::string(description);
        std::string opaque_data = NX::utility::conversions::utf16_to_utf8(opaque_object.serialize());

        _audit_db.push_record(audit_logdb_record(timestamp.ToUll(),
            final_rights,
            flags,
            this->user().best_name(),
            this->user().id(),
            operation_name,
            app_name,
            fso_type,
            resource_path,
            opaque_data));
    }
    catch (const std::exception& e) {
        LOGERR(GetLastError(), L"Fail to write audit log: %S", e.what());
    }
}


//
//  class NX::serv_session
//
NX::serv_session::serv_session()
{
    ::InitializeCriticalSection(&_lock);
}

NX::serv_session::~serv_session()
{
    ::DeleteCriticalSection(&_lock);
}

void NX::serv_session::clear() noexcept
{
    ::EnterCriticalSection(&_lock);
    _map.clear();
    ::LeaveCriticalSection(&_lock);
}

bool NX::serv_session::empty() noexcept
{
    bool result = true;
    ::EnterCriticalSection(&_lock);
    result = _map.empty();
    ::LeaveCriticalSection(&_lock);
    return result;
}

void NX::serv_session::move(SessionMapType& ss)
{
    ::EnterCriticalSection(&_lock);
    ss = _map;
    _map.clear();
    ::LeaveCriticalSection(&_lock);
}

void NX::serv_session::copy(SessionMapType& ss)
{
    ::EnterCriticalSection(&_lock);
    ss = _map;
    ::LeaveCriticalSection(&_lock);
}

bool NX::serv_session::exists(unsigned long id) noexcept
{
    bool found = false;
    ::EnterCriticalSection(&_lock);
    SessionMapType::iterator pos = std::find_if(_map.begin(), _map.end(), [&](const std::pair<unsigned long, std::shared_ptr<NX::session>>& pi)->bool {
        return (pi.first == id);
    });
    found = (pos != _map.end()) ? true : false;
    ::LeaveCriticalSection(&_lock);
    return found;
}

std::shared_ptr<NX::session> NX::serv_session::add(unsigned long id, bool* existing) noexcept
{
    std::shared_ptr<NX::session> ss(new NX::session(id));

    ::EnterCriticalSection(&_lock);

    SessionMapType::iterator pos = std::find_if(_map.begin(), _map.end(), [&](const std::pair<unsigned long, std::shared_ptr<NX::session>>& pi)->bool {
        return (pi.first == id);
    });

    if (pos != _map.end()) {
        // A session already exists
        *existing = true;
        ss.reset();
        ss = _map[id];
    }
    else {
        // New session
        *existing = false;
        if (ss != nullptr) {
            _map[id] = ss;
        }
    }

    ::LeaveCriticalSection(&_lock);

    return ss;
}

std::shared_ptr<NX::session> NX::serv_session::del(unsigned long id) noexcept
{
    std::shared_ptr<NX::session> ss(nullptr);

    ::EnterCriticalSection(&_lock);
    SessionMapType::iterator pos = std::find_if(_map.begin(), _map.end(), [&](const std::pair<unsigned long, std::shared_ptr<NX::session>>& pi)->bool {
        return (pi.first == id);
    });
    if (pos != _map.end()) {
        ss = (*pos).second;
        _map.erase(pos);
    }
    ::LeaveCriticalSection(&_lock);

    return ss;
}

std::shared_ptr<NX::session> NX::serv_session::get(unsigned long id) noexcept
{
    std::shared_ptr<NX::session> ss(nullptr);

    ::EnterCriticalSection(&_lock);
    SessionMapType::iterator pos = std::find_if(_map.begin(), _map.end(), [&](const std::pair<unsigned long, std::shared_ptr<NX::session>>& pi)->bool {
        return (pi.first == id);
    });
    if (pos != _map.end()) {
        ss = (*pos).second;
    }
    ::LeaveCriticalSection(&_lock);

    return ss;
}
