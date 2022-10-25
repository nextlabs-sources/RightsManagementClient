
#pragma once
#ifndef __NXSERV_CTRL_HPP__
#define __NXSERV_CTRL_HPP__


#include <string>
#include <vector>

namespace NXSERV {

typedef enum NXSERV_REQUEST {
    CTL_SERV_UNKNOWN = 0,
    CTL_SERV_QUERY_STATUS,
    CTL_SERV_UPDATE_POLICY,
    CTL_SERV_ENABLE_DEBUG,
    CTL_SERV_DISABLE_DEBUG,
    CTL_SERV_COLLECT_DEBUGLOG,
    CTL_SERV_QUERY_AUTHN_INFO,
    CTL_SERV_LOGIN,
    CTL_SERV_LOGOUT,
    CTL_SERV_SET_DWM_STATUS,
    CTL_SERV_EXPORT_ACTIVITY_LOG,
    CTL_SERV_WRITE_LOG
} NXSERV_REQUEST;


#define NXSERV_REQUEST_PARAM_CODE            L"code"
#define NXSERV_REQUEST_PARAM_RESULT          L"result"
#define NXSERV_REQUEST_PARAM_CONNECTED       L"is_connected"
#define NXSERV_REQUEST_PARAM_DEBUG_MODE      L"is_debug_mode"
#define NXSERV_REQUEST_PARAM_POLICY_TIME     L"policy_time"
#define NXSERV_REQUEST_PARAM_HEARTBEAT_TIME  L"heartbeat_time"
#define NXSERV_REQUEST_PARAM_PRODUCT_VERSION L"product_version"
#define NXSERV_REQUEST_PARAM_AUTHN_SERVER_URL    L"authn_server_url"
#define NXSERV_REQUEST_PARAM_AUTHN_RETURN_URL    L"authn_reutrn_url"
#define NXSERV_REQUEST_PARAM_LOGON_TENANT    L"logon_tenant"
#define NXSERV_REQUEST_PARAM_LOGON_USER      L"logon_user"
#define NXSERV_REQUEST_PARAM_LOGON_TIME      L"logon_time"
#define NXSERV_REQUEST_PARAM_EMABLED         L"enabled"
#define NXSERV_REQUEST_PARAM_LEVEL           L"level"
#define NXSERV_REQUEST_PARAM_MESSAGE         L"message"
#define NXSERV_REQUEST_PARAM_FILE            L"file"

class serv_status
{
public:
    serv_status()
    {
    }

    serv_status(bool connected,
        bool debug_mode,
        const std::wstring& policy_time,
        const std::wstring& heartbeat_time,
        const std::wstring& product_version,
        const std::wstring& logon_tenant,
        const std::wstring& logon_user,
        const std::wstring& logon_time
        ) : 
        _connected(connected),
        _debug_mode(debug_mode),
        _policy_time(policy_time),
        _heartbeat_time(heartbeat_time),
        _product_version(product_version),
        _logon_tenant(logon_tenant),
        _logon_user(logon_user),
        _logon_time(logon_time)
    {
    }

    ~serv_status()
    {
    }


    inline bool empty() const { return _product_version.empty(); }
    void clear()
    {
        _connected = false;
        _debug_mode = false;
        _policy_time.clear();
        _heartbeat_time.clear();
        _product_version.clear();
        _logon_tenant.clear();
        _logon_user.clear();
        _logon_time.clear();
    }

    inline bool is_connected() const { return _connected; }
    inline bool is_debug_mode_on() const { return _debug_mode; }
    inline std::wstring get_policy_time() const { return _policy_time; }
    inline std::wstring get_heartbeat_time() const { return _heartbeat_time; }
    inline std::wstring get_product_version() const { return _product_version; }
    inline std::wstring get_logon_tenant() const { return _logon_tenant; }
    inline std::wstring get_logon_user() const { return _logon_user; }
    inline std::wstring get_logon_time() const { return _logon_time; }
    serv_status& operator = (const serv_status& other)
    {
        if (this != &other) {
            _connected = other.is_connected();
            _debug_mode = other.is_debug_mode_on();
            _policy_time = other.get_policy_time();
            _heartbeat_time = other.get_heartbeat_time();
            _product_version = other.get_product_version();
            _logon_tenant = other.get_logon_tenant();
            _logon_user = other.get_logon_user();
            _logon_time = other.get_logon_time();
        }
        return *this;
    }

private:
    bool            _connected;
    bool            _debug_mode;
    std::wstring    _policy_time;
    std::wstring    _heartbeat_time;
    std::wstring    _product_version;
    std::wstring    _logon_tenant;
    std::wstring    _logon_user;
    std::wstring    _logon_time;
};


class authn_info
{
public:
    authn_info() {}
    authn_info(const std::wstring& server_url, const std::wstring& return_url) : _server_url(server_url), _return_url(return_url)
    {
    }
    virtual ~authn_info() {}


    inline bool empty() const { return _server_url.empty(); }
    void clear()
    {
        _server_url.clear();
        _return_url.clear();
    }

    inline const std::wstring& get_server_url() const { return _server_url; }
    inline const std::wstring& get_return_url() const { return _return_url; }
    authn_info& operator = (const authn_info& other)
    {
        if (this != &other) {
            _server_url = other.get_server_url();
            _return_url = other.get_return_url();
        }
        return *this;
    }

private:
    std::wstring    _server_url;
    std::wstring    _return_url;
};

class authn_result
{
public:
    authn_result() {}
    authn_result(const std::wstring& user_name,
        const std::wstring& user_id,
        const std::wstring& user_token,
        const std::vector<std::pair<std::wstring, std::wstring>>& user_attributes
        ) : _user_name(user_name), _user_id(user_id), _user_token(user_token), _user_attributes(user_attributes)
    {
    }
    virtual ~authn_result() {}

    inline bool empty() const { return _user_name.empty(); }
    void clear()
    {
        _user_name.clear();
        _user_id.clear();
        _user_token.clear();
        _user_attributes.clear();
    }

    inline const std::wstring& get_user_name() const { return _user_name; }
    inline const std::wstring& get_user_id() const { return _user_id; }
    inline const std::wstring& get_user_token() const { return _user_token; }
    inline const std::vector<std::pair<std::wstring, std::wstring>>& get_user_attributes() const { return _user_attributes; }
    authn_result& operator = (const authn_result& other)
    {
        if (this != &other) {
            _user_name = other.get_user_name();
            _user_id = other.get_user_id();
            _user_token = other.get_user_token();
            _user_attributes = other.get_user_attributes();
        }
        return *this;
    }

private:
    std::wstring    _user_name;
    std::wstring    _user_id;
    std::wstring    _user_token;
    std::vector<std::pair<std::wstring, std::wstring>> _user_attributes;
};


serv_status query_serv_status() noexcept;
bool update_policy() noexcept;
bool enable_debug(bool enable) noexcept;
bool collect_debug_log() noexcept;
authn_info query_authn_info() noexcept;
authn_result log_in() noexcept;
bool log_out() noexcept;
bool set_dwm_status(bool enabled) noexcept;
bool export_activity_log(const std::wstring& file) noexcept;
bool write_log(int level, const std::wstring& message) noexcept;

}



#endif