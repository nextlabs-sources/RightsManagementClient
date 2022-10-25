
#include <Windows.h>
#include <assert.h>
#include <winioctl.h>

#include <nudf\shared\enginectl.h>
#include "clientipc.hpp"
#include "nxrmtray.h"

using namespace nxrm::tray;


extern APPSTATE    g_APPState;
extern serv_status g_serv_status;
extern logon_ui_flag g_logon_ui_flag;
extern void doShowTrayMsg(_In_ LPCWSTR wzTitle, _In_ LPCWSTR wzInfo);
extern void QuitMain();
extern void EngineLogIn(LPCWSTR wzDefaultDomain = NULL);
extern void SetPolicyTimestamp(_In_ const FILETIME* timestamp);
extern void SetPolicyTimestamp(const std::wstring& time);
extern void SetUpdateTimestamp(_In_ const FILETIME* timestamp);
extern void SetUpdateTimestamp(const std::wstring& time);
extern void UpdateTrayMenu(bool debug_on, bool external_authn, const std::wstring& logon_user);
extern void UpdateServerStatus(const nxrm::tray::serv_status& st);

CConnServ::CConnServ() : NX::async_pipe::server(4096, 5000)
{
}

CConnServ::~CConnServ()
{
    shutdown();
}

void CConnServ::on_read(unsigned char* data, unsigned long* size, bool* write_response)
{
    *write_response = false;

    try {

        std::string s((const char*)data);
        std::wstring ws = NX::utility::conversions::utf8_to_utf16(s);
        NX::web::json::value v = NX::web::json::value::parse(ws);

        const std::wstring code = v[L"code"].as_string();

        if (code == L"quit") {
            QuitMain();
        }
        else if (code == L"notify") {
            const std::wstring notify_title = v[L"notify_title"].as_string();
            const std::wstring notify_msg = v[L"notify_msg"].as_string();
            doShowTrayMsg(notify_title.c_str(), notify_msg.c_str());
        }
        else if (code == L"set_policy_time") {
            const std::wstring policy_time = v[L"timestamp"].as_string();
            SetPolicyTimestamp(policy_time);
        }
        else if (code == L"set_heartbeat_time") {
            const std::wstring heartbeat_time = v[L"timestamp"].as_string();
            SetUpdateTimestamp(heartbeat_time);
        }
        else if (code == L"set_serv_status") {
            g_serv_status.parse(ws);
            UpdateServerStatus(g_serv_status);
        }
        else if (code == L"show_logon_ui") {
            if (!g_logon_ui_flag.on()) {
                // show logon ui
                std::wstring logon_domain;
                if (v.has_field(L"logon_domain") && v.at(L"logon_domain").is_string()) {
                    logon_domain = v.at(L"logon_domain").as_string();
                }
                EngineLogIn(logon_domain.empty() ? NULL : logon_domain.c_str());
            }
        }
        else if (code == L"query_logon_user") {
            NX::web::json::value response = NX::web::json::value::object();
            response[L"result"] = NX::web::json::value::number((int)0);
            response[L"user_name"] = NX::web::json::value::string(std::wstring(g_APPState.wzCurrentUser));
            response[L"user_id"] = NX::web::json::value::string(std::wstring(g_APPState.wzCurrentUserId));
            std::wstring logon_info_w = response.serialize();
            std::string logon_info_a = NX::utility::conversions::utf16_to_utf8(logon_info_w);
            if (((unsigned long)logon_info_a.length() + 1) > buffer_size()) {
                logon_info_a.substr(0, buffer_size() - 2);
            }
            memset(data, 0, buffer_size());
            memcpy(data, logon_info_a.c_str(), logon_info_a.length());
            *size = (unsigned long)logon_info_a.length() + 1;
            *write_response = true;
        }
        else if (code == L"update_response") {
            if (v[L"result"].as_bool()) {
                const std::wstring policy_time(v[L"policy_time"].as_string());
                if (!policy_time.empty()) {
                    const std::wstring update_time(v[L"heartbeat_time"].as_string());
                    // update policy information
                    SetPolicyTimestamp(policy_time);
                    SetUpdateTimestamp(update_time);
                }
            }
			SetIconStopRotatingFlag();
        }
        else {
            ; // NOTHING
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
}

std::wstring serv_control::_serv_port;

serv_control::serv_control() : NX::async_pipe::client(4096)
{
}

serv_control::~serv_control()
{
}

void serv_control::set_serv_ctl_port(const std::wstring& port)
{
    _serv_port = port;
}

serv_status serv_control::ctl_query_status()
{
    serv_status ss;

    NX::web::json::value v = NX::web::json::value::object();
    v[L"code"] = NX::web::json::value::number((int)3);
    v[L"session"] = NX::web::json::value::number((int)session_id());
    if (!connect(serv_control::_serv_port)) {
        return ss;
    }
    std::wstring ws = v.serialize();
    std::string s = NX::utility::conversions::utf16_to_utf8(ws);
    std::vector<unsigned char> data;
    data.resize(s.length() + 1, 0);
    memcpy(&data[0], s.c_str(), s.length());
    if (!write(data)) {
        return ss;
    }
    data.clear();
    if (!read(data)) {
        return ss;
    }

    s = (char*)data.data();
    ws = NX::utility::conversions::utf8_to_utf16(s);

    ss.parse(ws);
    return ss;
}

bool serv_control::ctl_update_policy()
{
    NX::web::json::value v = NX::web::json::value::object();
    v[L"code"] = NX::web::json::value::number((int)4);
    v[L"session"] = NX::web::json::value::number((int)session_id());
    if (!connect(serv_control::_serv_port)) {
        return false;
    }
    std::wstring ws = v.serialize();
    std::string s = NX::utility::conversions::utf16_to_utf8(ws);
    std::vector<unsigned char> data;
    data.resize(s.length() + 1, 0);
    memcpy(&data[0], s.c_str(), s.length());
    if (!write(data)) {
        return false;
    }

    return true;
}

bool serv_control::ctl_enable_debug(bool b)
{
    NX::web::json::value v = NX::web::json::value::object();
    v[L"code"] = NX::web::json::value::number((int)(b ? 6 : 5));
    v[L"session"] = NX::web::json::value::number((int)session_id());
    if (!connect(serv_control::_serv_port)) {
        return false;
    }
    std::wstring ws = v.serialize();
    std::string s = NX::utility::conversions::utf16_to_utf8(ws);
    std::vector<unsigned char> data;
    data.resize(s.length() + 1, 0);
    memcpy(&data[0], s.c_str(), s.length());
    if (!write(data)) {
        return false;
    }

    return true;
}

bool serv_control::ctl_collect_debug()
{
    NX::web::json::value v = NX::web::json::value::object();
    v[L"code"] = NX::web::json::value::number((int)7);
    v[L"session"] = NX::web::json::value::number((int)session_id());
    if (!connect(serv_control::_serv_port)) {
        return false;
    }
    std::wstring ws = v.serialize();
    std::string s = NX::utility::conversions::utf16_to_utf8(ws);
    std::vector<unsigned char> data;
    data.resize(s.length() + 1, 0);
    memcpy(&data[0], s.c_str(), s.length());
    if (!write(data)) {
        return false;
    }

    return true;
}

bool serv_control::query_default_authn_info(std::wstring& user_name, std::wstring& domain_name)
{
    user_name.clear();
    domain_name.clear();

    NX::web::json::value v = NX::web::json::value::object();
    v[L"code"] = NX::web::json::value::number((int)8);
    v[L"session"] = NX::web::json::value::number((int)session_id());
    if (!connect(serv_control::_serv_port)) {
        return false;
    }
    std::wstring ws = v.serialize();
    std::string s = NX::utility::conversions::utf16_to_utf8(ws);
    std::vector<unsigned char> data;
    data.resize(s.length() + 1, 0);
    memcpy(&data[0], s.c_str(), s.length());
    if (!write(data)) {
        return false;
    }

    read(data, 60000); // wait for result

    s = std::string(data.begin(), data.end());
    ws = NX::utility::conversions::utf8_to_utf16(s);
    NX::web::json::value response = NX::web::json::value::parse(ws);
    if (response.has_field(L"result") && response[L"result"].is_number()) {
        if (0 != response[L"result"].as_integer()) {
            return false;
        }
        if (response.has_field(L"user_name") && response[L"user_name"].is_string()) {
            user_name = response[L"user_name"].as_string();
        }
        if (response.has_field(L"user_domain") && response[L"user_domain"].is_string()) {
            domain_name = response[L"user_domain"].as_string();
        }
        return true;
    }

    return false;
}

bool serv_control::ctl_login(const std::wstring& user, const std::wstring& domain, const std::wstring& password, unsigned long* result)
{
    NX::web::json::value v = NX::web::json::value::object();
    v[L"code"] = NX::web::json::value::number((int)1);
    v[L"session"] = NX::web::json::value::number((int)session_id());
    v[L"user"] = NX::web::json::value::string(user);
    v[L"domain"] = NX::web::json::value::string(domain);
    v[L"password"] = NX::web::json::value::string(password);
    if (!connect(serv_control::_serv_port)) {
        return false;
    }
    std::wstring ws = v.serialize();
    std::string s = NX::utility::conversions::utf16_to_utf8(ws);
    std::vector<unsigned char> data;
    data.resize(s.length() + 1, 0);
    memcpy(&data[0], s.c_str(), s.length());
    if (!write(data)) {
        return false;
    }

    read(data, 60000); // wait for result

    s = std::string(data.begin(), data.end());
    ws = NX::utility::conversions::utf8_to_utf16(s);
    NX::web::json::value response = NX::web::json::value::parse(ws);
    if (response.has_field(L"result") && response[L"result"].is_number()) {
        int rc = response[L"result"].as_number().to_int32();
        switch (rc)
        {
        case 200: *result = 0; break;
        case 400: *result = ERROR_INVALID_PASSWORD; break;
        case 401: *result = ERROR_NOT_AUTHENTICATED; break;
        case 460: *result = ERROR_INVALID_DOMAINNAME; break;
        case 461:
        default:
            *result = E_UNEXPECTED; break;
        }
    }
    else {
        return false;
    }
    return true;
}

bool serv_control::ctl_logout()
{
    NX::web::json::value v = NX::web::json::value::object();
    v[L"code"] = NX::web::json::value::number((int)2);
    v[L"session"] = NX::web::json::value::number((int)session_id());
    if (!connect(serv_control::_serv_port)) {
        return false;
    }
    std::wstring ws = v.serialize();
    std::string s = NX::utility::conversions::utf16_to_utf8(ws);
    std::vector<unsigned char> data;
    data.resize(s.length() + 1, 0);
    memcpy(&data[0], s.c_str(), s.length());
    if (!write(data)) {
        return false;
    }

    return true;
}

bool serv_control::set_aero_status(bool enabled)
{
    NX::web::json::value v = NX::web::json::value::object();
    v[L"code"] = NX::web::json::value::number((int)9);
    v[L"session"] = NX::web::json::value::number((int)session_id());
    v[L"aero"] = NX::web::json::value::boolean(enabled);
    if (!connect(serv_control::_serv_port)) {
        return false;
    }
    std::wstring ws = v.serialize();
    std::string s = NX::utility::conversions::utf16_to_utf8(ws);
    std::vector<unsigned char> data;
    data.resize(s.length() + 1, 0);
    memcpy(&data[0], s.c_str(), s.length());
    if (!write(data)) {
        return false;
    }

    return true;
}

unsigned long serv_control::session_id() noexcept
{
    static unsigned long _session_id = (unsigned long) -1;
    if ((unsigned long)-1 == _session_id) {
        ProcessIdToSessionId(GetCurrentProcessId(), &_session_id);
    }
    return _session_id;
}

void serv_status::parse(const std::wstring& ws) noexcept
{
    try {

        NX::web::json::value response = NX::web::json::value::parse(ws);

        if (response.has_field(L"connected")) {
            _connected = response[L"connected"].as_bool() ? 1 : 0;
        }
        if (response.has_field(L"debug")) {
            _debug = response[L"debug"].as_bool();
        }
        if (response.has_field(L"authn_type")) {
            std::wstring authn_type = response[L"authn_type"].as_string();
            _external_authn = (0 == _wcsicmp(authn_type.c_str(), L"external")) ? true : false;
        }
        if (response.has_field(L"policy_time")) {
            _policy_time = response[L"policy_time"].as_string();
        }
        if (response.has_field(L"heartbeat_time")) {
            _heartbeat_time = response[L"heartbeat_time"].as_string();
        }
        if (response.has_field(L"product_version")) {
            _product_version = response[L"product_version"].as_string();
        }
        if (response.has_field(L"logon_server")) {
            _logon_server = response[L"logon_server"].as_string();
        }
        if (response.has_field(L"logon_user")) {
            _logon_user = response[L"logon_user"].as_string();
        }
        if (response.has_field(L"logon_user_id")) {
            _logon_user_id = response[L"logon_user_id"].as_string();
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
}


session_guard::session_guard() : _h(NULL)
{
}

session_guard::~session_guard()
{
    release();
}

bool session_guard::check_existence()
{
    _h = ::CreateMutexW(NULL, TRUE, L"Local\\NextLabs.TrayApp.Guard");
    assert(NULL != _h);
    return (ERROR_ALREADY_EXISTS == GetLastError());
}

void session_guard::release()
{
    if (NULL != _h) {
        CloseHandle(_h);
        _h = NULL;
    }
}
