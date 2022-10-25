

#ifndef __NXRMTRAY_CLIENT_IPC_HPP__
#define __NXRMTRAY_CLIENT_IPC_HPP__

#include <nudf\exception.hpp>
#include <nudf\asyncpipe.hpp>
#include <nudf\web\json.hpp>

namespace nxrm {
namespace tray {


class CConnServ : public NX::async_pipe::server
{
public:
    CConnServ();
    virtual ~CConnServ();
    virtual void on_read(unsigned char* data, unsigned long* size, bool* write_response);
};

class serv_status
{
public:
    serv_status() : _connected(false), _debug(true), _external_authn(false)
    {
    }

    ~serv_status()
    {
    }

    serv_status& operator = (const serv_status& other)
    {
        if (this != &other) {
            _connected = _connected;
            _debug = other.debug();
            _external_authn = other.external_authn();
            _policy_time = other.policy_time();
            _heartbeat_time = other.heartbeat_time();
            _product_version = other.product_version();
            _logon_server = other.logon_server();
            _logon_user = other.logon_user();
            _logon_user_id = other.logon_user();
        }
        return *this;
    }

    void parse(const std::wstring& ws) noexcept;

    inline bool connected() const noexcept { return _connected; }
    inline bool debug() const noexcept { return _debug; }
    inline bool external_authn() const noexcept { return _external_authn; }
    inline const std::wstring& policy_time() const noexcept { return _policy_time; }
    inline const std::wstring& heartbeat_time() const noexcept { return _heartbeat_time; }
    inline const std::wstring& product_version() const noexcept { return _product_version; }
    inline const std::wstring& logon_server() const noexcept { return _logon_server; }
    inline const std::wstring& logon_user() const noexcept { return _logon_user; }
    inline const std::wstring& logon_user_id() const noexcept { return _logon_user_id; }
    
private:
    bool            _connected;
    bool            _debug;
    bool            _external_authn;
    std::wstring    _policy_time;
    std::wstring    _heartbeat_time;
    std::wstring    _product_version;
    std::wstring    _logon_server;
    std::wstring    _logon_user;
    std::wstring    _logon_user_id;
};

class logon_ui_flag
{
public:
    logon_ui_flag() : _on(false)
    {
        ::InitializeCriticalSection(&_lock);
    }
    ~logon_ui_flag()
    {
        ::DeleteCriticalSection(&_lock);
    }

    bool on() const noexcept { bool result = false;  ::EnterCriticalSection(&_lock);  result = _on; ::LeaveCriticalSection(&_lock); return result; }
    void set() noexcept { ::EnterCriticalSection(&_lock);  _on = true; ::LeaveCriticalSection(&_lock); }
    void unset() noexcept { ::EnterCriticalSection(&_lock);  _on = false; ::LeaveCriticalSection(&_lock); }

private:
    bool    _on;
    mutable CRITICAL_SECTION _lock;
};

class logon_ui_flag_guard
{
public:
    logon_ui_flag_guard() : _p(NULL){}
    logon_ui_flag_guard(logon_ui_flag* p) : _p(p)
    {
        _p->set();
    }
    ~logon_ui_flag_guard()
    {
        if (_p != NULL) {
            _p->unset();
            _p = NULL;
        }
    }

private:
    logon_ui_flag* _p;
};

class serv_control : public NX::async_pipe::client
{
public:
    serv_control();
    virtual ~serv_control();

    static void set_serv_ctl_port(const std::wstring& port);

    serv_status ctl_query_status();
    bool ctl_update_policy();
    bool ctl_enable_debug(bool b);
    bool ctl_collect_debug();
    bool ctl_login(const std::wstring& user, const std::wstring& domain, const std::wstring& password, unsigned long* result);
    bool ctl_logout();
    bool query_default_authn_info(std::wstring& user_name, std::wstring& domain_name);
    bool set_aero_status(bool enabled);

protected:
    unsigned long session_id() noexcept;

private:
    static std::wstring _serv_port;
};

class session_guard
{
public:
    session_guard();
    virtual ~session_guard();

    bool check_existence();
    void release();

private:
    HANDLE  _h;
};

}   // namespace tray
}   // namespace nxrm


#endif