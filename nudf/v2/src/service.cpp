

#include <Windows.h>
#include <assert.h>


#include <algorithm>
#include <memory>

#include <nudf\eh.hpp>
#include <nudf\debug.hpp>
#include <nudf\handyutil.hpp>
#include <nudf\service.hpp>


using namespace NX;
using namespace NX::win;




//
//  class service_info
//
#define SERVICE_TYPE_CHANGED                0x00000001
#define SERVICE_START_TYPE_CHANGED          0x00000002
#define SERVICE_ERROR_CONTROL_CHANGED       0x00000004
#define SERVICE_DISPLAY_NAME_CHANGED        0x00000008
#define SERVICE_BINARY_PATH_CHANGED         0x00000010
#define SERVICE_DESCRIPTION_CHANGED         0x00000020
#define SERVICE_LOAD_ORDER_GROUP_CHANGED    0x00000040
#define SERVICE_DEPENDENCY_CHANGED          0x00000080

service_info::service_info() : _type(0), _start_type(0), _error_control(0), _tag_id(0), _change_flags(0)
{
}

service_info::service_info( const std::wstring& svc_name,
                            const std::wstring& svc_binary_path,
                            const std::wstring& svc_display_name,
                            const std::wstring& svc_description,
                            unsigned long svc_type,
                            unsigned long svc_start,
                            unsigned long svc_errctl,
                            const std::wstring& svc_load_order_group,
                            const std::vector<std::wstring>& dependencies
                            )
    : _type(svc_type), _start_type(svc_start), _error_control(svc_errctl), _tag_id(0), _change_flags(0),
      _name(svc_name), _binary_path(svc_binary_path), _display_name(svc_display_name), _description(svc_description),
      _load_order_group(svc_load_order_group), _dependencies(dependencies)
{
}

service_info::~service_info()
{
}

bool service_info::empty() const noexcept
{
    return _name.empty();
}


bool service_info::dirty() const noexcept
{
    return (0 != _change_flags);
}

void service_info::apply(SC_HANDLE h)
{
    if (0 == _change_flags) {
        return;
    }

    const wchar_t* binary_path = NULL;
    const wchar_t* load_order_group = NULL;
    const wchar_t* service_start_name = NULL;
    std::vector<wchar_t> dependencies;

    if (flags32_on(_change_flags, SERVICE_BINARY_PATH_CHANGED)) {
        binary_path = _binary_path.empty() ? NULL : _binary_path.c_str();
    }
    if (flags32_on(_change_flags, SERVICE_LOAD_ORDER_GROUP_CHANGED)) {
        load_order_group = _load_order_group.empty() ? NULL : _load_order_group.c_str();
    }
    if (flags32_on(_change_flags, SERVICE_DEPENDENCY_CHANGED)) {

        size_t size = 0;
        std::for_each(_dependencies.begin(), _dependencies.end(), [&](const std::wstring& s) {
            size += s.length() + 1;
        });
        size++;

        dependencies.resize(size, 0);
        wchar_t* p = &dependencies[0];
        std::for_each(_dependencies.begin(), _dependencies.end(), [&](const std::wstring& s) {
            wcsncpy_s(p, size, s.c_str(), _TRUNCATE);
            p += (s.length() + 1);
            size -= (s.length() + 1);
        });
    }

    if (!ChangeServiceConfigW(h,
        flags32_on(_change_flags, SERVICE_TYPE_CHANGED) ? _type : SERVICE_NO_CHANGE,                   // ServiceType
        flags32_on(_change_flags, SERVICE_START_TYPE_CHANGED) ? _start_type : SERVICE_NO_CHANGE,       // StartType
        flags32_on(_change_flags, SERVICE_ERROR_CONTROL_CHANGED) ? _error_control : SERVICE_NO_CHANGE, // ErrorControl
        flags32_on(_change_flags, SERVICE_BINARY_PATH_CHANGED) ? _binary_path.c_str() : NULL,          // BinaryPathName,
        flags32_on(_change_flags, SERVICE_LOAD_ORDER_GROUP_CHANGED) ? _load_order_group.c_str() : NULL,// LoadOrderGroup,
        NULL,              // TagId,
        dependencies.empty() ? NULL : dependencies.data(),  // Dependencies,
        NULL,              // ServiceStartName,
        NULL,              // Password,
        NULL               // DisplayName
        )) {
        throw std::exception("fail to apply service config change");
    }


    if (flags32_on(_change_flags, SERVICE_DESCRIPTION_CHANGED)) {
        SERVICE_DESCRIPTION desc_info = { (PWCHAR)_description.c_str() };
        if (!ChangeServiceConfig2W(h, SERVICE_CONFIG_DESCRIPTION, &desc_info)) {
            throw std::exception("fail to change service description");
        }
    }

    _change_flags = 0;
}

void service_info::clear() noexcept
{
    _change_flags = 0;
    _type = 0;
    _start_type = 0;
    _error_control = 0;
    _tag_id = 0;
    _name.clear();
    _display_name.clear();
    _binary_path.clear();
    _description.clear();
    _load_order_group.clear();
    _dependencies.clear();
}

service_info& service_info::operator = (const service_info& other) noexcept
{
    if (this != &other) {
        _type = other.type();
        _start_type = other.start_type();
        _error_control = other.error_control();
        _tag_id = other.tag_id();
        _name = other.name();
        _display_name = other.display_name();
        _binary_path = other.binary_path();
        _description = other.description();
        _load_order_group = other.load_order_group();
        _dependencies = other.dependencies();
    }
    return *this;
}

bool service_info::operator == (const service_info& other) noexcept
{
    if (this == &other) {
        return true;
    }

    return (type() == other.type()
        && name() == other.name()
        && start_type() == other.start_type()
        && error_control() == other.error_control()
        && tag_id() == other.tag_id()
        && display_name() == other.display_name()
        && binary_path() == other.binary_path()
        && description() == other.description()
        && load_order_group() == other.load_order_group()
        && dependencies() == other.dependencies()
        );
}

void service_info::set_type(unsigned long v)
{
    _type = v;
    _change_flags |= SERVICE_TYPE_CHANGED;
}

void service_info::set_start_type(unsigned long v)
{
    _start_type = v;
    _change_flags |= SERVICE_START_TYPE_CHANGED;
}

void service_info::set_error_control(unsigned long v)
{
    _error_control = v;
    _change_flags |= SERVICE_ERROR_CONTROL_CHANGED;
}

void service_info::set_display_name(const std::wstring& v)
{
    _display_name = v;
    _change_flags |= SERVICE_DISPLAY_NAME_CHANGED;
}

void service_info::set_binary_path(const std::wstring& v)
{
    _binary_path = v;
    _change_flags |= SERVICE_BINARY_PATH_CHANGED;
}

void service_info::set_description(const std::wstring& v)
{
    _description = v;
    _change_flags |= SERVICE_DESCRIPTION_CHANGED;
}

void service_info::set_load_order_group(const std::wstring& v)
{
    _load_order_group = v;
    _change_flags |= SERVICE_LOAD_ORDER_GROUP_CHANGED;
}

void service_info::set_dependencies(const std::vector<std::wstring>& v)
{
    _dependencies = v;
    _change_flags |= SERVICE_DEPENDENCY_CHANGED;
}

void service_info::add_dependency(const std::wstring& v)
{
    if (_dependencies.end() == std::find_if(_dependencies.begin(), _dependencies.end(), [&](const std::wstring& s) -> bool {
        return (0 == _wcsicmp(v.c_str(), s.c_str()));
    })) {
        _dependencies.push_back(v);
        _change_flags |= SERVICE_DEPENDENCY_CHANGED;
    }
}

void service_info::remove_dependency(const std::wstring& v)
{
    auto pos = std::find_if(_dependencies.begin(), _dependencies.end(), [&](const std::wstring& s) -> bool {
        return (0 == _wcsicmp(v.c_str(), s.c_str()));
    });
    if (pos != _dependencies.end()) {
        _dependencies.erase(pos);
        _change_flags |= SERVICE_DEPENDENCY_CHANGED;
    }
}





//
//  class service_instance
//
static unsigned long __stdcall service_handler_ex(unsigned long ctl_code, unsigned long event_type, void* event_data, void* context);
static void __stdcall service_main(unsigned long argc, wchar_t** argv);
static std::shared_ptr<service_instance>    instance_ptr;

// run as service
static SERVICE_TABLE_ENTRYW service_dispatch_table[] = {
    { NULL, (LPSERVICE_MAIN_FUNCTIONW)service_main },
    { NULL, NULL }
};


unsigned long __stdcall service_handler_ex(unsigned long ctl_code, unsigned long event_type, void* event_data, void* context)
{
    if (NULL == event_data) {
        return ERROR_INVALID_HANDLE;
    }
    return (unsigned long)((service_instance*)context)->control(ctl_code, event_type, event_data, NULL);
}

void __stdcall service_main(unsigned long argc, wchar_t** argv)
{
    if (NULL == instance_ptr.get()) {
        SetLastError(ERROR_SERVICE_DOES_NOT_EXIST);
        throw std::exception("service not exist");
    }

    instance_ptr->start(argc, (const wchar_t**)argv);
}

service_instance::service_instance() : _ctls_accepted(SERVICE_ACCEPT_DEFAULT_CTLS), _h(NULL), _stop_event(NULL), _stop_hint(0)
{
}

service_instance::service_instance(const std::wstring& name, unsigned long ctls_accepted) : _name(name), _ctls_accepted(ctls_accepted), _h(NULL), _stop_event(NULL), _stop_hint(0)
{
}

service_instance::~service_instance()
{
}

void service_instance::run()
{
    instance_ptr.reset(this);
    service_dispatch_table[0].lpServiceName = (LPWSTR)_name.c_str();
    if (!StartServiceCtrlDispatcherW(service_dispatch_table)) {
        throw std::exception("fail to start service dispatch");
    }
}

void service_instance::run_as_console()
{
    try {

        _stop_event = ::CreateEventW(NULL, TRUE, FALSE, NULL);
        if (NULL == _stop_event) {
            throw std::exception("fail to create stop event");
        }

        // Fire start event
        on_start(0, NULL);

        //
        //  Wait for stop event
        //
        WaitForSingleObject(_stop_event, INFINITE);

        // Fire stop event
        on_stop();

        CloseHandle(_stop_event);
        _stop_event = NULL;
    }
    catch (std::exception& e) {

        UNREFERENCED_PARAMETER(e);
        if (NULL != _stop_event) {
            CloseHandle(_stop_event);
            _stop_event = NULL;
        }
    }
}

long service_instance::start(int argc, const wchar_t** argv) noexcept
{
    long    lRet = 0L;

    try {

        _stop_event = ::CreateEventW(NULL, TRUE, FALSE, NULL);
        if (NULL == _stop_event) {
            throw std::exception("fail to create stop event");
        }

        _h = ::RegisterServiceCtrlHandlerExW((LPWSTR)_name.c_str(), service_handler_ex, this);
        if (NULL == _h) {
            throw std::exception("fail to register service control handler");
        }

        _status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
        _status.dwServiceSpecificExitCode = 0;
        set_status(SERVICE_START_PENDING, NO_ERROR, 2000);

        // Fire start event
        on_start(argc, argv);

        // Started
        set_status(SERVICE_RUNNING, NO_ERROR, 0);

        //
        //  Wait for stop event
        //
        WaitForSingleObject(_stop_event, INFINITE);

        // Fire stop event
        on_stop();

        // Stopped
        set_status(SERVICE_STOPPED, 0, 0);
        CloseHandle(_stop_event);
        _stop_event = NULL;
    }
    catch (std::exception& e) {

        UNREFERENCED_PARAMETER(e);
        if (NULL != _stop_event) {
            CloseHandle(_stop_event);
            _stop_event = NULL;
        }
        set_status(SERVICE_STOPPED, GetLastError(), 0);
    }

    return lRet;
}

void service_instance::stop() noexcept
{
    if (NULL == _h || SERVICE_STOPPED == _status.dwCurrentState || SERVICE_STOP_PENDING == _status.dwCurrentState) {
        return;
    }

    assert(NULL != _stop_event);
    set_status(SERVICE_STOP_PENDING, NO_ERROR, 0);
    (void)SetEvent(_stop_event);
}

long service_instance::pause() noexcept
{
    long result = 0L;

    if (SERVICE_RUNNING != _status.dwCurrentState) {
        return ERROR_SERVICE_NOT_ACTIVE;
    }

    set_status(SERVICE_PAUSE_PENDING, NO_ERROR, 0);
    try {
        on_pause();
        set_status(SERVICE_PAUSED, NO_ERROR, 0);
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        set_status(SERVICE_RUNNING, NO_ERROR, 0);
        result = GetLastError();
        if (0 == result) {
            // unknown error
            result = ERROR_GENERIC_COMMAND_FAILED;
        }
    }

    return result;
}

long service_instance::resume() noexcept
{
    long result = 0L;

    if (SERVICE_PAUSED != _status.dwCurrentState) {
        return ERROR_EXCEPTION_IN_SERVICE;
    }

    set_status(SERVICE_CONTINUE_PENDING, NO_ERROR, 0);
    try {
        on_resume();
        set_status(SERVICE_RUNNING, NO_ERROR, 0);
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        set_status(SERVICE_PAUSED, NO_ERROR, 0);
        result = GetLastError();
        if (0 == result) {
            // unknown error
            result = ERROR_GENERIC_COMMAND_FAILED;
        }
    }

    return result;
}

void service_instance::set_status(unsigned long state, unsigned long exit_code, unsigned long hint) noexcept
{
    static unsigned long check_point = 1;

    // Fill in the SERVICE_STATUS structure.
    _status.dwCurrentState = state;
    _status.dwWin32ExitCode = exit_code;
    _status.dwWaitHint = hint;
    _status.dwControlsAccepted = _ctls_accepted;

    if (state == SERVICE_START_PENDING) {
        _status.dwControlsAccepted &= ~SERVICE_ACCEPT_STOP;
    }
    else {
        _status.dwControlsAccepted |= SERVICE_ACCEPT_STOP;
    }

    if ((state == SERVICE_RUNNING) || (state == SERVICE_STOPPED)) {
        _status.dwCheckPoint = 0;
    }
    else {
        _status.dwCheckPoint = check_point++;
    }

    // Report the status of the service to the SCM.
    (VOID)SetServiceStatus(_h, &_status);
}

void service_instance::register_device_event()
{
    DEV_BROADCAST_DEVICEINTERFACE nfilter;
    memset(&nfilter, 0, sizeof(nfilter));
    nfilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    nfilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

    _dev_notify_h = RegisterDeviceNotificationW(_h, &nfilter, DEVICE_NOTIFY_SERVICE_HANDLE | DEVICE_NOTIFY_ALL_INTERFACE_CLASSES);
    if (NULL == _dev_notify_h) {
        bool stop_here = false;
        on_device_event_register_failure(&stop_here);
        if (stop_here) {
            throw std::exception("fail to register device notification");
        }
    }
}

long service_instance::control(unsigned long ctl_code, unsigned long event_type, void* event_data, void* context) noexcept
{
    long result = NO_ERROR;

    try {
        switch (ctl_code)
        {
        case SERVICE_CONTROL_STOP:
            stop();
            return NO_ERROR;

        case SERVICE_CONTROL_CONTINUE:
            result = resume();
            break;

        case SERVICE_CONTROL_PAUSE:
            result = pause();
            break;

        case SERVICE_CONTROL_PARAMCHANGE:
            on_param_change();
            break;

        case SERVICE_CONTROL_PRESHUTDOWN:
            on_preshutdown();
            break;

        case SERVICE_CONTROL_SHUTDOWN:
            on_shutdown();
            break;

        case SERVICE_CONTROL_INTERROGATE:
            break;

        case SERVICE_CONTROL_NETBINDADD:
            on_netbind_add();
            break;

        case SERVICE_CONTROL_NETBINDREMOVE:
            on_netbind_remove();
            break;

        case SERVICE_CONTROL_NETBINDENABLE:
            on_netbind_enable();
            break;

        case SERVICE_CONTROL_NETBINDDISABLE:
            on_netbind_disable();
            break;

            //
            // Codes supported by HandlerEx only
            //
        case SERVICE_CONTROL_DEVICEEVENT:
            switch (event_type)
            {
            case DBT_DEVICEARRIVAL:
                result = on_device_arrival((DEV_BROADCAST_HDR*)event_data);
                break;
            case DBT_DEVICEREMOVECOMPLETE:
                result = on_device_remove_complete((DEV_BROADCAST_HDR*)event_data);
                break;
            case DBT_DEVICEQUERYREMOVE:
                result = on_device_query_remove((DEV_BROADCAST_HDR*)event_data);
                break;
            case DBT_DEVICEQUERYREMOVEFAILED:
                result = on_device_query_remove_failed((DEV_BROADCAST_HDR*)event_data);
                break;
            case DBT_DEVICEREMOVEPENDING:
                result = on_device_remove_pending((DEV_BROADCAST_HDR*)event_data);
                break;
            case DBT_CUSTOMEVENT:
                result = on_device_custom_event((DEV_BROADCAST_HDR*)event_data);
                break;
            default:
                break;
            }
            break;

        case SERVICE_CONTROL_HARDWAREPROFILECHANGE:
            switch (event_type)
            {
            case DBT_CONFIGCHANGED:
                result = on_hwprofile_changed();
                break;
            case DBT_QUERYCHANGECONFIG:
                result = on_hwprofile_query_change();
                break;
            case DBT_CONFIGCHANGECANCELED:
                result = on_hwprofile_change_canceled();
                break;
            default:
                break;
            }
            break;

        case SERVICE_CONTROL_POWEREVENT:
            switch (event_type)
            {
            case PBT_APMPOWERSTATUSCHANGE:
                result = on_power_status_changed();
                break;
            case PBT_APMRESUMEAUTOMATIC:
                result = on_power_resume_auto();
                break;
            case PBT_APMRESUMESUSPEND:
                result = on_power_resume_suspend();
                break;
            case PBT_APMSUSPEND:
                result = on_power_suspend();
                break;
            case PBT_POWERSETTINGCHANGE:
                result = on_power_setting_changed((POWERBROADCAST_SETTING*)event_data);
                break;
            case PBT_APMBATTERYLOW:
                result = on_power_lowbattery();
                break;
            case PBT_APMOEMEVENT:
            case PBT_APMQUERYSUSPEND:
            case PBT_APMQUERYSUSPENDFAILED:
            case PBT_APMRESUMECRITICAL:
            default:
                break;
            }
            break;

        case SERVICE_CONTROL_SESSIONCHANGE:
            switch (event_type)
            {
            case WTS_CONSOLE_CONNECT:
                result = on_session_conn((WTSSESSION_NOTIFICATION*)event_data);
                break;
            case WTS_CONSOLE_DISCONNECT:
                result = on_session_disconn((WTSSESSION_NOTIFICATION*)event_data);
                break;
            case WTS_REMOTE_CONNECT:
                result = on_session_remote_conn((WTSSESSION_NOTIFICATION*)event_data);
                break;
            case WTS_REMOTE_DISCONNECT:
                result = on_session_remote_disconn((WTSSESSION_NOTIFICATION*)event_data);
                break;
            case WTS_SESSION_LOGON:
                result = on_session_logon((WTSSESSION_NOTIFICATION*)event_data);
                break;
            case WTS_SESSION_LOGOFF:
                result = on_session_logoff((WTSSESSION_NOTIFICATION*)event_data);
                break;
            case WTS_SESSION_LOCK:
                result = on_session_lock((WTSSESSION_NOTIFICATION*)event_data);
                break;
            case WTS_SESSION_UNLOCK:
                result = on_session_unlock((WTSSESSION_NOTIFICATION*)event_data);
                break;
            case WTS_SESSION_REMOTE_CONTROL:
                result = on_session_remote_control((WTSSESSION_NOTIFICATION*)event_data);
                break;
            case WTS_SESSION_CREATE:
            case WTS_SESSION_TERMINATE:
            default:
                break;
            }
            break;

        default:
            break;
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }

    return result;
}




//
//
//

service_control::service_control()
{
}

service_control::~service_control()
{
    close();
}

bool service_control::exists(const std::wstring& name)
{
    return false;
}

bool service_control::remove(const std::wstring& name)
{
    bool result = false;
    SC_HANDLE h_mgr = NULL;
    SC_HANDLE h = NULL;

    try {

        h_mgr = ::OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (NULL == h_mgr) {
            throw std::exception("fail to open service manager");
        }
        h = ::OpenServiceW(h_mgr, name.c_str(), DELETE | SERVICE_ALL_ACCESS);
        if (NULL == h) {
            throw std::exception("fail to open service");
        }

        result = DeleteService(h) ? true : false;
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }

    return result;
}

void service_control::create(const service_info& si)
{
    if (si.name().empty()) {
        throw std::exception("inavlid service name");
    }

    if (opened()) {
        close();
    }

    _h_mgr = ::OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (NULL == _h_mgr) {
        throw std::exception("fail to open service manager");
    }

    std::vector<wchar_t> dependency_buffer;

    if (!si.dependencies().empty()) {

        size_t size = 0;
        std::for_each(si.dependencies().begin(), si.dependencies().end(), [&](const std::wstring& s) {
            size += s.length() + 1;
        });
        size++;

        dependency_buffer.resize(size, 0);
        wchar_t* p = &dependency_buffer[0];
        std::for_each(si.dependencies().begin(), si.dependencies().end(), [&](const std::wstring& s) {
            wcsncpy_s(p, size, s.c_str(), _TRUNCATE);
            p += (s.length() + 1);
            size -= (s.length() + 1);
        });
    }

    _h = CreateServiceW(_h_mgr,
        si.name().c_str(),
        si.display_name().empty() ? si.name().c_str() : si.display_name().c_str(),
        SERVICE_ALL_ACCESS,
        si.type(),
        si.start_type(),
        si.error_control(),
        si.binary_path().c_str(),
        si.load_order_group().empty() ? NULL : si.load_order_group().c_str(),
        NULL,
        dependency_buffer.empty() ? NULL : dependency_buffer.data(),
        NULL,
        NULL);
    if (NULL == _h) {
        CloseServiceHandle(_h_mgr);
        _h_mgr = NULL;
        throw std::exception("fail to create service");
    }

    // set description
    SERVICE_DESCRIPTION desc_info = { (PWCHAR)(si.description().empty() ? si.display_name().c_str() : si.description().c_str()) };
    (VOID)ChangeServiceConfig2W(_h, SERVICE_CONFIG_DESCRIPTION, &desc_info);
}

void service_control::open(const std::wstring& name, bool read_only)
{
    const unsigned long service_mgr_desired_access = read_only ? (STANDARD_RIGHTS_REQUIRED | SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS | SERVICE_ENUMERATE_DEPENDENTS) : (DELETE | SERVICE_ALL_ACCESS);
    const unsigned long service_desired_access = read_only ? (SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE) : SC_MANAGER_ALL_ACCESS;

    if (opened()) {
        close();
    }

    _h_mgr = ::OpenSCManagerW(NULL, NULL, service_mgr_desired_access);
    if (NULL == _h_mgr) {
        throw std::exception("fail to open service manager");
    }

    _h = ::OpenServiceW(_h_mgr, name.c_str(), service_desired_access);
    if (NULL == _h) {
        CloseServiceHandle(_h_mgr);
        _h_mgr = NULL;
        throw std::exception("fail to open service");
    }
}

void service_control::close()
{
    if (_h != NULL) {
        CloseServiceHandle(_h);
        _h = NULL;
    }
    if (_h_mgr != NULL) {
        CloseServiceHandle(_h_mgr);
        _h_mgr = NULL;
    }
    _si.clear();
}

void service_control::start(bool wait, unsigned long wait_time)
{
    if (!opened()) {
        throw std::exception("service is not opened");
    }

    service_status status = query_status(false, 0);
    if (status.is_running()) {
        SetLastError(ERROR_SERVICE_ALREADY_RUNNING);
        return;
    }
    else if (status.is_paused()) {
        resume(wait, wait_time);
    }
    else if (status.is_start_pending()) {
        status = query_status(wait, wait_time);
    }
    else if (status.is_stop_pending() || status.is_pause_pending()) {
        SetLastError(ERROR_SERVICE_CANNOT_ACCEPT_CTRL);
        throw std::exception("service is pending, cannot start at this time");
    }
    else if (status.is_continue_pending()) {
        status = query_status(wait, wait_time);
    }
    else {
        assert(status.is_stopped());
        if (!StartServiceW(_h, 0, NULL)) {
            throw std::exception("fail to start service");
        }
        status = query_status(wait, wait_time);
    }

    __assume(0);
}

void service_control::stop(bool wait, unsigned long wait_time)
{
    if (!opened()) {
        throw std::exception("service is not opened");
    }

    service_status status = query_status(false, 0);
    if (status.is_stopped()) {
        return;
    }
    else if (status.is_stop_pending()) {
        status = query_status(wait, wait_time);
        return;
    }
    else if (status.is_start_pending() || status.is_continue_pending() || status.is_pause_pending()) {
        SetLastError(ERROR_SERVICE_CANNOT_ACCEPT_CTRL);
        throw std::exception("service is pending, cannot start at this time");
    }
    else {
        assert(status.is_stopped() || status.is_running());
        if (!ControlService(_h, SERVICE_CONTROL_STOP, &status)) {
            throw std::exception("fail to stop service");
        }
        if (!status.is_stopped()) {
            status = query_status(wait, wait_time);
        }
    }

    __assume(0);
}

void service_control::pause(bool wait, unsigned long wait_time)
{
    if (!opened()) {
        throw std::exception("service is not opened");
    }

    service_status status = query_status(false, 0);
    if (status.is_paused()) {
        return;
    }
    else if (status.is_stopped()) {
        SetLastError(ERROR_SERVICE_NOT_ACTIVE);
        throw std::exception("service is not running");
    }
    else if (status.is_pending()) {
        SetLastError(ERROR_SERVICE_CANNOT_ACCEPT_CTRL);
        throw std::exception("service is pending, cannot start at this time");
    }
    else {
        assert(status.is_running());
        if (!ControlService(_h, SERVICE_CONTROL_PAUSE, &status)) {
            throw std::exception("fail to pause service");
        }
        if (!status.is_paused()) {
            status = query_status(wait, wait_time);
        }
    }
}

void service_control::resume(bool wait, unsigned long wait_time)
{
    if (!opened()) {
        throw std::exception("service is not opened");
    }

    service_status status = query_status(false, 0);
    if (status.is_running()) {
        return;
    }
    else if (status.is_stopped()) {
        SetLastError(ERROR_SERVICE_NOT_ACTIVE);
        throw std::exception("service is not running");
    }
    else if (status.is_pending()) {
        SetLastError(ERROR_SERVICE_CANNOT_ACCEPT_CTRL);
        throw std::exception("service is pending, cannot start at this time");
    }
    else {
        assert(status.is_paused());
        if (!ControlService(_h, SERVICE_CONTROL_CONTINUE, &status)) {
            throw std::exception("fail to resume service");
        }
        if (!status.is_running()) {
            status = query_status(wait, wait_time);
        }
    }
}

void service_control::enable(unsigned long start_type)
{
    if (!opened()) {
        throw std::exception("service is not opened");
    }

    assert(start_type == SERVICE_AUTO_START || start_type == SERVICE_SYSTEM_START || start_type == SERVICE_BOOT_START);

    if (_si.start_type() != SERVICE_DISABLED) {
        _si.set_start_type(start_type);
        _si.apply(_h);
    }
}

void service_control::disable(bool wait, unsigned long wait_time)
{
    service_status status;

    if (!opened()) {
        throw std::exception("service is not opened");
    }

    status = query_status();
    if (!status.is_stopped()) {
        stop(wait, wait_time);
        if (!status.is_stopped()) {
            throw std::exception("fail to stop service before disable it");
        }
    }

    _si.set_start_type(SERVICE_DISABLED);
    _si.apply(_h);
}

service_status service_control::query_status(bool wait_if_pending, unsigned long wait_time)
{
    service_status status;

    static const unsigned long wait_interval = 200;

    if (!opened()) {
        throw std::exception("service is not opened");
    }

    ULONG dwTotalWaitTime = 0;

    if (!QueryServiceStatus(_h, &status)) {
        throw std::exception("fail to query service status");
    }

    if (wait_if_pending && 0 != wait_time) {

        while (status.is_pending()) {

            // wait
            Sleep(wait_interval);
            // get again
            if (!QueryServiceStatus(_h, &status)) {
                throw std::exception("fail to query service status");
            }
            // not pending any more
            if (!status.is_pending()) {
                break;
            }
            // still pending ...
            if (INFINITE == wait_time) {
                continue;
            }
            if (wait_time < wait_interval) {
                // timeout
                SetLastError(ERROR_TIMEOUT);
                break;
            }
            // 
            wait_time -= wait_interval;
        }
    }

    return status;
}

void service_control::remove(bool wait, unsigned long wait_time)
{
    if (!opened()) {
        throw std::exception("service is not opened");
    }

    service_status ss = query_status();
    if (!ss.is_stopped()) {
        // stop it
        stop(wait, wait_time);
        ss = query_status();
    }

    if (ss.is_pending()) {
        throw std::exception("service is not stopped");
    }

    if (!::DeleteService(_h)) {
        throw std::exception("fail to delete service");
    }

    close();
}