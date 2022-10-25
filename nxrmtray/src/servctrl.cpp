

#include <windows.h>

#include <string>
#include <vector>
#include <exception>

#include <nudf\string.hpp>
#include <nudf\web\json.hpp>


#include "nxrmdrv.h"
#include "nxrmdrvman.h"
#include "nxrmcorehlp.h"

#include "servctrl.hpp"

using namespace NXSERV;

class drvcore_controller
{
public:
    drvcore_controller() : _h(NULL), _session_id(-1){
        ProcessIdToSessionId(GetCurrentProcessId(), &_session_id);
        InitializeCriticalSection(&_lock);
    }
    virtual ~drvcore_controller() { disconnect(); DeleteCriticalSection(&_lock); }

    bool connect()
    {
        bool result = false;
        ::EnterCriticalSection(&_lock);
        result = is_connected();
        if (!result) {
            _h = init_transporter_client();
            if (_h != NULL) {
                if (!drvcore_is_enabled()) {
                    close_transporter_client(_h);
                    _h = NULL;
                }
            }
            result = is_connected();
        }
        ::LeaveCriticalSection(&_lock);
        return result;
    }

    void disconnect()
    {
        if (NULL != _h) {
            close_transporter_client(_h);
            _h = NULL;
        }
    }

    inline bool is_connected() const { return (NULL != _h); }
    inline unsigned long get_session_id() const { return _session_id; }

    bool drvcore_is_good_version()
    {
        if (!is_connected()) {
            throw std::exception("Not connect to nxrm driver yet");
        }
        return (is_good_version(_h) ? true : false);
    }

    bool drvcore_is_enabled()
    {
        if (!is_connected()) {
            throw std::exception("Not connect to nxrm driver yet");
        }
        return (is_transporter_enabled(_h) ? true : false);
    }

    bool drvcore_is_channel_on(long channel_id)
    {
        if (!is_connected()) {
            throw std::exception("Not connect to nxrm driver yet");
        }
        return (is_xxx_channel_on(_h, channel_id) ? true : false);
    }

    bool drvcore_is_process_protected(unsigned long process_id = 0)
    {
        if (!is_connected()) {
            throw std::exception("Not connect to nxrm driver yet");
        }
        return (is_protected_process(_h, (0 == process_id) ? GetCurrentProcessId() : process_id) ? true : false);
    }

    bool drvcore_is_overlay_bitmap_ready()
    {
        if (!is_connected()) {
            throw std::exception("Not connect to nxrm driver yet");
        }
        return (is_overlay_bitmap_ready(_h) ? true : false);
    }

    unsigned long drvcore_get_policy_serial_number()
    {
        if (!is_connected()) {
            throw std::exception("Not connect to nxrm driver yet");
        }
        return (unsigned long)get_rm_policy_sn(_h);
    }

    std::vector<HWND> drvcore_get_overlay_windows()
    {
        std::vector<unsigned long> buf;
        std::vector<HWND> hwnds;
        unsigned long size = NXRM_MAX_DWM_WND;

        buf.resize(NXRM_MAX_DWM_WND, 0);
        get_overlay_windows(_h, buf.data(), &size);
        for (int i = 0; i < (int)size; i++) {
            hwnds.push_back((HWND)(ULONG_PTR)buf[i]);
        }

        return std::move(hwnds);
    }

    void drvcore_set_dwm_active_session(unsigned long session_id)
    {
        if (!is_connected()) {
            throw std::exception("Not connect to nxrm driver yet");
        }
        if (session_id == 0 || session_id == 0xFFFFFFFF) {
            throw std::exception("Invalid session id");
        }

        set_dwm_active_session(_h, session_id);
    }

    // request


    // notify


    std::vector<unsigned char> send_request(unsigned long type, std::vector<unsigned char>& request_data, bool wait)
    {
        return send_request(type, request_data.empty() ? NULL : request_data.data(), (unsigned long)request_data.size(), wait);
    }

    std::vector<unsigned char> send_request(unsigned long type, const void* request_data, unsigned long data_size, bool wait)
    {
        if (!is_connected()) {
            throw std::exception("Not connect to nxrm driver yet");
        }

        HANDLE request = submit_request(_h, type, (PVOID)request_data, data_size);
        if (NULL == request) {
            throw std::exception("fail to submit request");
        }
        if (!wait) {
            return std::vector<unsigned char>();
        }

        std::vector<unsigned char> response_data;
        unsigned long bytes_returned = 0;

        response_data.resize(4096, 0);
        if (!wait_for_response(request, _h, response_data.data(), (unsigned long)response_data.size(), &bytes_returned)) {
            throw std::exception("fail to submit request");
        }

        if (bytes_returned != 4096) {
            response_data.resize(bytes_returned);
        }

        return std::move(response_data);
    }

    void send_notify(unsigned long type, const std::vector<unsigned char>& request_data)
    {
        if (!is_connected()) {
            throw std::exception("Not connect to nxrm driver yet");
        }

        if (!submit_notify(_h, type, (PVOID)request_data.data(), (unsigned long)request_data.size())) {
            throw std::exception("fail to submit notification");
        }
    }


private:
    // no copy/move is allowed
    drvcore_controller& operator = (const drvcore_controller& other) { return *this; }
    drvcore_controller& operator = (drvcore_controller&& other) { return *this; }
private:
    HANDLE  _h;
    CRITICAL_SECTION _lock;
    unsigned long _session_id;
};

static drvcore_controller controller;


serv_status NXSERV::query_serv_status() noexcept
{
    serv_status status;

    try {

        controller.connect();

        NX::web::json::value json_request = NX::web::json::value::object();
        json_request[NXSERV_REQUEST_PARAM_CODE] = NX::web::json::value::number(CTL_SERV_QUERY_STATUS);
        std::wstring ws_request = json_request.serialize();
        std::string request = NX::utility::conversions::utf16_to_utf8(ws_request);

        QUERY_SERVICE_REQUEST request_blob = { 0 };
        memset(&request_blob, 0, sizeof(QUERY_SERVICE_REQUEST));
        request_blob.SessionId = controller.get_session_id();
        request_blob.ProcessId = GetCurrentProcessId();
        request_blob.ThreadId = GetCurrentThreadId();
        if (0 != request.length()) {
            memcpy(request_blob.Data, request.c_str(), min(request.length(), MAX_SERVICE_DATA_LENGTH));
        }

        std::vector<unsigned char> result = controller.send_request(NXRMDRV_MSG_TYPE_QUERY_SERVICE, &request_blob, (unsigned long)sizeof(QUERY_SERVICE_REQUEST), true);
        if (!result.empty()) {
            std::wstring ws_result = NX::utility::conversions::utf8_to_utf16(std::string(result.begin(), result.end()));
            NX::web::json::value json_result = NX::web::json::value::parse(ws_result);

            status = serv_status(json_result.as_object()[NXSERV_REQUEST_PARAM_CONNECTED].as_bool(),
                json_result.as_object()[NXSERV_REQUEST_PARAM_DEBUG_MODE].as_bool(),
                json_result.as_object()[NXSERV_REQUEST_PARAM_POLICY_TIME].as_string(),
                json_result.as_object()[NXSERV_REQUEST_PARAM_HEARTBEAT_TIME].as_string(),
                json_result.as_object()[NXSERV_REQUEST_PARAM_PRODUCT_VERSION].as_string(),
                json_result.as_object()[NXSERV_REQUEST_PARAM_LOGON_TENANT].as_string(),
                json_result.as_object()[NXSERV_REQUEST_PARAM_LOGON_USER].as_string(),
                json_result.as_object()[NXSERV_REQUEST_PARAM_LOGON_TIME].as_string()
                );
        }
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        status.clear();
    }

    return status;
}

bool NXSERV::update_policy() noexcept
{
    bool result = false;
    try {

        controller.connect();

        NX::web::json::value json_request = NX::web::json::value::object();
        json_request[NXSERV_REQUEST_PARAM_CODE] = NX::web::json::value::number(CTL_SERV_UPDATE_POLICY);
        std::wstring ws_request = json_request.serialize();
        std::string request = NX::utility::conversions::utf16_to_utf8(ws_request);

        QUERY_SERVICE_REQUEST request_blob = { 0 };
        memset(&request_blob, 0, sizeof(QUERY_SERVICE_REQUEST));
        request_blob.SessionId = controller.get_session_id();
        request_blob.ProcessId = GetCurrentProcessId();
        request_blob.ThreadId = GetCurrentThreadId();
        if (0 != request.length()) {
            memcpy(request_blob.Data, request.c_str(), min(request.length(), MAX_SERVICE_DATA_LENGTH));
        }

        (void)controller.send_request(NXRMDRV_MSG_TYPE_QUERY_SERVICE, &request_blob, (unsigned long)sizeof(QUERY_SERVICE_REQUEST), false);
        result = true;
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
    return result;
}

bool NXSERV::enable_debug(bool enable) noexcept
{
    bool result = false;
    try {

        controller.connect();

        NX::web::json::value json_request = NX::web::json::value::object();
        json_request[NXSERV_REQUEST_PARAM_CODE] = NX::web::json::value::number(enable ? CTL_SERV_ENABLE_DEBUG : CTL_SERV_DISABLE_DEBUG);
        std::wstring ws_request = json_request.serialize();
        std::string request = NX::utility::conversions::utf16_to_utf8(ws_request);

        QUERY_SERVICE_REQUEST request_blob = { 0 };
        memset(&request_blob, 0, sizeof(QUERY_SERVICE_REQUEST));
        request_blob.SessionId = controller.get_session_id();
        request_blob.ProcessId = GetCurrentProcessId();
        request_blob.ThreadId = GetCurrentThreadId();
        if (0 != request.length()) {
            memcpy(request_blob.Data, request.c_str(), min(request.length(), MAX_SERVICE_DATA_LENGTH));
        }

        (void)controller.send_request(NXRMDRV_MSG_TYPE_QUERY_SERVICE, &request_blob, (unsigned long)sizeof(QUERY_SERVICE_REQUEST), false);
        result = true;
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
    return result;
}

bool NXSERV::collect_debug_log() noexcept
{
    bool result = false;
    try {

        controller.connect();

        NX::web::json::value json_request = NX::web::json::value::object();
        json_request[NXSERV_REQUEST_PARAM_CODE] = NX::web::json::value::number(CTL_SERV_COLLECT_DEBUGLOG);
        std::wstring ws_request = json_request.serialize();
        std::string request = NX::utility::conversions::utf16_to_utf8(ws_request);

        QUERY_SERVICE_REQUEST request_blob = { 0 };
        memset(&request_blob, 0, sizeof(QUERY_SERVICE_REQUEST));
        request_blob.SessionId = controller.get_session_id();
        request_blob.ProcessId = GetCurrentProcessId();
        request_blob.ThreadId = GetCurrentThreadId();
        if (0 != request.length()) {
            memcpy(request_blob.Data, request.c_str(), min(request.length(), MAX_SERVICE_DATA_LENGTH));
        }

        (void)controller.send_request(NXRMDRV_MSG_TYPE_QUERY_SERVICE, &request_blob, (unsigned long)sizeof(QUERY_SERVICE_REQUEST), false);
        result = true;
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
    return result;
}

authn_info NXSERV::query_authn_info() noexcept
{
    authn_info info_result;

    try {

        controller.connect();

        NX::web::json::value json_request = NX::web::json::value::object();
        json_request[NXSERV_REQUEST_PARAM_CODE] = NX::web::json::value::number(CTL_SERV_QUERY_AUTHN_INFO);
        std::wstring ws_request = json_request.serialize();
        std::string request = NX::utility::conversions::utf16_to_utf8(ws_request);

        QUERY_SERVICE_REQUEST request_blob = { 0 };
        memset(&request_blob, 0, sizeof(QUERY_SERVICE_REQUEST));
        request_blob.SessionId = controller.get_session_id();
        request_blob.ProcessId = GetCurrentProcessId();
        request_blob.ThreadId = GetCurrentThreadId();
        if (0 != request.length()) {
            memcpy(request_blob.Data, request.c_str(), min(request.length(), MAX_SERVICE_DATA_LENGTH));
        }

        std::vector<unsigned char> result = controller.send_request(NXRMDRV_MSG_TYPE_QUERY_SERVICE, &request_blob, (unsigned long)sizeof(QUERY_SERVICE_REQUEST), true);
        if (!result.empty()) {
            std::wstring ws_result = NX::utility::conversions::utf8_to_utf16(std::string(result.begin(), result.end()));
            NX::web::json::value json_result = NX::web::json::value::parse(ws_result);

            info_result = authn_info(json_result.as_object()[L"authn_server_url"].as_string(),
                json_result.as_object()[L"authn_return_url"].as_string()
                );
        }
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        info_result.clear();
    }

    return info_result;
}

authn_result NXSERV::log_in() noexcept
{
    return authn_result();
}

bool NXSERV::log_out() noexcept
{
    bool result = false;
    try {

        controller.connect();

        NX::web::json::value json_request = NX::web::json::value::object();
        json_request[NXSERV_REQUEST_PARAM_CODE] = NX::web::json::value::number(CTL_SERV_LOGOUT);
        std::wstring ws_request = json_request.serialize();
        std::string request = NX::utility::conversions::utf16_to_utf8(ws_request);

        QUERY_SERVICE_REQUEST request_blob = { 0 };
        memset(&request_blob, 0, sizeof(QUERY_SERVICE_REQUEST));
        request_blob.SessionId = controller.get_session_id();
        request_blob.ProcessId = GetCurrentProcessId();
        request_blob.ThreadId = GetCurrentThreadId();
        if (0 != request.length()) {
            memcpy(request_blob.Data, request.c_str(), min(request.length(), MAX_SERVICE_DATA_LENGTH));
        }

        (void)controller.send_request(NXRMDRV_MSG_TYPE_QUERY_SERVICE, &request_blob, (unsigned long)sizeof(QUERY_SERVICE_REQUEST), false);
        result = true;
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
    return result;
}

bool NXSERV::set_dwm_status(bool enabled) noexcept
{
    bool result = false;
    try {

        controller.connect();

        NX::web::json::value json_request = NX::web::json::value::object();
        json_request[NXSERV_REQUEST_PARAM_CODE] = NX::web::json::value::number(CTL_SERV_SET_DWM_STATUS);
        json_request[NXSERV_REQUEST_PARAM_EMABLED] = NX::web::json::value::boolean(enabled);
        std::wstring ws_request = json_request.serialize();
        std::string request = NX::utility::conversions::utf16_to_utf8(ws_request);

        QUERY_SERVICE_REQUEST request_blob = { 0 };
        memset(&request_blob, 0, sizeof(QUERY_SERVICE_REQUEST));
        request_blob.SessionId = controller.get_session_id();
        request_blob.ProcessId = GetCurrentProcessId();
        request_blob.ThreadId = GetCurrentThreadId();
        if (0 != request.length()) {
            memcpy(request_blob.Data, request.c_str(), min(request.length(), MAX_SERVICE_DATA_LENGTH));
        }

        (void)controller.send_request(NXRMDRV_MSG_TYPE_QUERY_SERVICE, &request_blob, (unsigned long)sizeof(QUERY_SERVICE_REQUEST), false);
        result = true;
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
    return result;
}

bool NXSERV::export_activity_log(const std::wstring& file) noexcept
{
    bool result = false;
    try {

        controller.connect();

        NX::web::json::value json_request = NX::web::json::value::object();
        json_request[NXSERV_REQUEST_PARAM_CODE] = NX::web::json::value::number(CTL_SERV_EXPORT_ACTIVITY_LOG);
        json_request[NXSERV_REQUEST_PARAM_FILE] = NX::web::json::value::string(file);
        std::wstring ws_request = json_request.serialize();
        std::string request = NX::utility::conversions::utf16_to_utf8(ws_request);

        QUERY_SERVICE_REQUEST request_blob = { 0 };
        memset(&request_blob, 0, sizeof(QUERY_SERVICE_REQUEST));
        request_blob.SessionId = controller.get_session_id();
        request_blob.ProcessId = GetCurrentProcessId();
        request_blob.ThreadId = GetCurrentThreadId();
        if (0 != request.length()) {
            memcpy(request_blob.Data, request.c_str(), min(request.length(), MAX_SERVICE_DATA_LENGTH));
        }

        (void)controller.send_request(NXRMDRV_MSG_TYPE_QUERY_SERVICE, &request_blob, (unsigned long)sizeof(QUERY_SERVICE_REQUEST), false);
        result = true;
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
    return result;
}

bool NXSERV::write_log(int level, const std::wstring& message) noexcept
{
    bool result = false;
    try {

        controller.connect();

        NX::web::json::value json_request = NX::web::json::value::object();
        json_request[NXSERV_REQUEST_PARAM_CODE] = NX::web::json::value::number(CTL_SERV_WRITE_LOG);
        json_request[NXSERV_REQUEST_PARAM_LEVEL] = NX::web::json::value::number(level);
        json_request[NXSERV_REQUEST_PARAM_MESSAGE] = NX::web::json::value::string(message);
        std::wstring ws_request = json_request.serialize();
        std::string request = NX::utility::conversions::utf16_to_utf8(ws_request);

        QUERY_SERVICE_REQUEST request_blob = { 0 };
        memset(&request_blob, 0, sizeof(QUERY_SERVICE_REQUEST));
        request_blob.SessionId = controller.get_session_id();
        request_blob.ProcessId = GetCurrentProcessId();
        request_blob.ThreadId = GetCurrentThreadId();
        if (0 != request.length()) {
            memcpy(request_blob.Data, request.c_str(), min(request.length(), MAX_SERVICE_DATA_LENGTH));
        }

        (void)controller.send_request(NXRMDRV_MSG_TYPE_QUERY_SERVICE, &request_blob, (unsigned long)sizeof(QUERY_SERVICE_REQUEST), false);
        result = true;
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
    return result;
}
