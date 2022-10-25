

#pragma once
#ifndef __NUDF_SERVICE_HPP__
#define __NUDF_SERVICE_HPP__


#include <Windows.h>
#include <Dbt.h>

#include <string>
#include <vector>

namespace NX {
namespace win {


class service_info
{
public:
    service_info();
    service_info(const std::wstring& svc_name,
                 const std::wstring& svc_binary_path,
                 const std::wstring& svc_display_name,
                 const std::wstring& svc_description,
                 unsigned long svc_type,
                 unsigned long svc_start,
                 unsigned long svc_errctl = 0,
                 const std::wstring& svc_load_order_group = std::wstring(),
                 const std::vector<std::wstring>& dependencies = std::vector<std::wstring>()
                 );
    virtual ~service_info();

    bool empty() const noexcept;
    void clear() noexcept;
    bool dirty() const noexcept;
    void apply(SC_HANDLE h);
    service_info& operator = (const service_info& other) noexcept;
    bool operator == (const service_info& other) noexcept;

    inline unsigned long type() const noexcept { return _type; }
    inline unsigned long start_type() const noexcept { return _start_type; }
    inline unsigned long error_control() const noexcept { return _error_control; }
    inline unsigned long tag_id() const noexcept { return _tag_id; }
    inline const std::wstring& name() const noexcept { return _name; }
    inline const std::wstring& display_name() const noexcept { return _display_name; }
    inline const std::wstring& binary_path() const noexcept { return _binary_path; }
    inline const std::wstring& description() const noexcept { return _description; }
    inline const std::wstring& load_order_group() const noexcept { return _load_order_group; }
    inline const std::vector<std::wstring>& dependencies() const noexcept { return _dependencies; }

    void set_type(unsigned long v);
    void set_start_type(unsigned long v);
    void set_error_control(unsigned long v);
    void set_display_name(const std::wstring& v);
    void set_binary_path(const std::wstring& v);
    void set_description(const std::wstring& v);
    void set_load_order_group(const std::wstring& v);
    void set_dependencies(const std::vector<std::wstring>& v);
    void add_dependency(const std::wstring& v);
    void remove_dependency(const std::wstring& v);

private:
    unsigned long   _type;
    unsigned long   _start_type;
    unsigned long   _error_control;
    unsigned long   _tag_id;
    std::wstring    _name;
    std::wstring    _display_name;
    std::wstring    _binary_path;
    std::wstring    _description;
    std::wstring    _load_order_group;
    std::vector<std::wstring>   _dependencies;
    unsigned long   _change_flags;
};

class service_status : public SERVICE_STATUS
{
public:
    service_status()
    {
        clear();
    }

    virtual ~service_status()
    {
    }

    inline unsigned long type() const noexcept { return dwServiceType; }
    inline unsigned long state() const noexcept { return dwCurrentState; }
    inline unsigned long ctls_accepted() const noexcept { return dwControlsAccepted; }
    inline unsigned long win32_exit_code() const noexcept { return dwWin32ExitCode; }
    inline unsigned long service_exit_code() const noexcept { return dwServiceSpecificExitCode; }
    inline unsigned long check_point() const noexcept { return dwCheckPoint; }
    inline unsigned long wait_hint() const noexcept { return dwWaitHint; }

    inline bool is_running() const noexcept { return (dwCurrentState == SERVICE_RUNNING); }
    inline bool is_stopped() const noexcept { return (dwCurrentState == SERVICE_STOPPED); }
    inline bool is_paused() const noexcept { return (dwCurrentState == SERVICE_PAUSED); }
    inline bool is_start_pending() const noexcept { return (dwCurrentState == SERVICE_START_PENDING); }
    inline bool is_stop_pending() const noexcept { return (dwCurrentState == SERVICE_STOP_PENDING); }
    inline bool is_pause_pending() const noexcept { return (dwCurrentState == SERVICE_PAUSE_PENDING); }
    inline bool is_continue_pending() const noexcept { return (dwCurrentState == SERVICE_CONTINUE_PENDING); }
    inline bool is_pending() const noexcept { return (is_start_pending() | is_stop_pending() | is_pause_pending() | is_continue_pending()); }

    inline bool accept_pause_continue() const noexcept { return (0 != (SERVICE_ACCEPT_PAUSE_CONTINUE & dwControlsAccepted)); }
    inline bool accept_stop() const noexcept { return (0 != (SERVICE_ACCEPT_STOP & dwControlsAccepted)); }
    inline bool accept_preshutdown() const noexcept { return (0 != (SERVICE_ACCEPT_PRESHUTDOWN & dwControlsAccepted)); }
    inline bool accept_shutdown() const noexcept { return (0 != (SERVICE_ACCEPT_SHUTDOWN & dwControlsAccepted)); }
    inline bool accept_hwprofile_change() const noexcept { return (0 != (SERVICE_ACCEPT_HARDWAREPROFILECHANGE & dwControlsAccepted)); }
    inline bool accept_power_event() const noexcept { return (0 != (SERVICE_ACCEPT_POWEREVENT & dwControlsAccepted)); }
    inline bool accept_session_change() const noexcept { return (0 != (SERVICE_ACCEPT_SESSIONCHANGE & dwControlsAccepted)); }
    inline bool accept_time_change() const noexcept { return (0 != (SERVICE_ACCEPT_TIMECHANGE & dwControlsAccepted)); }

    void clear() noexcept
    {
        dwServiceType = 0;
        dwCurrentState = 0;
        dwControlsAccepted = 0;
        dwWin32ExitCode = 0;
        dwServiceSpecificExitCode = 0;
        dwCheckPoint = 0;
        dwWaitHint = 0;
    }
};


#define SERVICE_ACCEPT_ALL_CTLS     (SERVICE_ACCEPT_PAUSE_CONTINUE \
                                     | SERVICE_ACCEPT_PRESHUTDOWN \
                                     | SERVICE_ACCEPT_SHUTDOWN \
                                     | SERVICE_ACCEPT_STOP \
                                     | SERVICE_ACCEPT_HARDWAREPROFILECHANGE \
                                     | SERVICE_ACCEPT_POWEREVENT \
                                     | SERVICE_ACCEPT_SESSIONCHANGE \
                                     | SERVICE_ACCEPT_TIMECHANGE)

#define SERVICE_ACCEPT_DEFAULT_CTLS (SERVICE_ACCEPT_PAUSE_CONTINUE \
                                     | SERVICE_ACCEPT_PRESHUTDOWN \
                                     | SERVICE_ACCEPT_SHUTDOWN \
                                     | SERVICE_ACCEPT_STOP \
                                     | SERVICE_ACCEPT_SESSIONCHANGE)


class service_instance
{
public:
    virtual ~service_instance();

    void run();
    void run_as_console();

    // this should never be called, only used internally
    long start(int argc, const wchar_t** argv) noexcept;

protected:
    service_instance();
    service_instance(const std::wstring& name, unsigned long ctls_accepted = SERVICE_ACCEPT_DEFAULT_CTLS);

protected:
    
    // service events
    virtual void on_start(int argc, const wchar_t** argv){}
    virtual void on_stop() noexcept {}
    virtual void on_pause(){}
    virtual void on_resume(){}
    virtual void on_preshutdown() noexcept {}
    virtual void on_shutdown() noexcept {}
    virtual void on_param_change() noexcept {}
    virtual void on_netbind_add() noexcept {}
    virtual void on_netbind_remove() noexcept {}
    virtual void on_netbind_enable() noexcept {}
    virtual void on_netbind_disable() noexcept {}

    // device events
    virtual void on_device_event_register_failure(bool* stop) { *stop = false; }
    virtual long on_device_arrival(_In_ DEV_BROADCAST_HDR* dbch) noexcept { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual long on_device_remove_complete(_In_ DEV_BROADCAST_HDR* dbch) noexcept { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual long on_device_query_remove(_In_ DEV_BROADCAST_HDR* dbch) noexcept { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual long on_device_query_remove_failed(_In_ DEV_BROADCAST_HDR* dbch) noexcept { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual long on_device_remove_pending(_In_ DEV_BROADCAST_HDR* dbch) noexcept { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual long on_device_custom_event(_In_ DEV_BROADCAST_HDR* dbch) noexcept { return ERROR_CALL_NOT_IMPLEMENTED; }

    // hardware profile events
    virtual long on_hwprofile_changed() noexcept { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual long on_hwprofile_query_change() noexcept { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual long on_hwprofile_change_canceled() noexcept { return ERROR_CALL_NOT_IMPLEMENTED; }

    // power events
    virtual long on_power_status_changed() noexcept { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual long on_power_resume_auto() noexcept { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual long on_power_resume_suspend() noexcept { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual long on_power_suspend() noexcept { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual long on_power_setting_changed(_In_ POWERBROADCAST_SETTING* pbs) noexcept { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual long on_power_lowbattery() noexcept { return ERROR_CALL_NOT_IMPLEMENTED; }

    // session events
    virtual long on_session_conn(_In_ WTSSESSION_NOTIFICATION* wtsn) noexcept { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual long on_session_disconn(_In_ WTSSESSION_NOTIFICATION* wtsn) noexcept { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual long on_session_remote_conn(_In_ WTSSESSION_NOTIFICATION* wtsn) noexcept { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual long on_session_remote_disconn(_In_ WTSSESSION_NOTIFICATION* wtsn) noexcept { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual long on_session_logon(_In_ WTSSESSION_NOTIFICATION* wtsn) noexcept { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual long on_session_logoff(_In_ WTSSESSION_NOTIFICATION* wtsn) noexcept { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual long on_session_lock(_In_ WTSSESSION_NOTIFICATION* wtsn) noexcept { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual long on_session_unlock(_In_ WTSSESSION_NOTIFICATION* wtsn) noexcept { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual long on_session_remote_control(_In_ WTSSESSION_NOTIFICATION* wtsn) noexcept { return ERROR_CALL_NOT_IMPLEMENTED; }

public:
    long control(unsigned long ctl_code, unsigned long event_type, void* event_data, void* context) noexcept;

private:
    // control
    void stop() noexcept;
    long pause() noexcept;
    long resume() noexcept;
    void set_status(unsigned long state, unsigned long exit_code, unsigned long hint) noexcept;
    void register_device_event();
    
private:
    std::wstring            _name;
    SERVICE_STATUS_HANDLE   _h;
    service_status          _status;
    HANDLE                  _dev_notify_h;
    HANDLE                  _stop_event;
    unsigned long           _stop_hint;
    unsigned long           _ctls_accepted;

private:
    static service_instance* _unique_instance_ptr;
};

class service_control
{
public:
    enum {
        service_not_open = 0,
        service_opened,
        service_created,
    };
public:
    service_control();
    virtual ~service_control();

    static bool exists(const std::wstring& name);
    static bool remove(const std::wstring& name);


    inline bool empty() const noexcept { return _si.empty(); }
    inline bool opened() const noexcept { return (NULL == _h); }

    void create(const service_info& si);
    void open(const std::wstring& name, bool read_only=false);
    void close();


    void start(bool wait = false, unsigned long wait_time = 5000);
    void stop(bool wait = false, unsigned long wait_time = 5000);
    void pause(bool wait = false, unsigned long wait_time = 5000);
    void resume(bool wait = false, unsigned long wait_time = 5000);
    void enable(unsigned long start_type = SERVICE_AUTO_START);
    void disable(bool wait = false, unsigned long wait_time = 5000);
    void remove(bool wait = false, unsigned long wait_time = 5000);

    service_status query_status(bool wait_if_pending = false, unsigned long wait_time = 5000);

private:
    service_info    _si;
    SC_HANDLE       _h;
    SC_HANDLE       _h_mgr;
};


}   // NX::win
}   // NX


#endif  // #ifndef __NX_SERVICE_HPP__