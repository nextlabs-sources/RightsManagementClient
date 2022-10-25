

#include <Windows.h>
#include <assert.h>

#include <nudf\exception.hpp>
#include <nudf\svc.hpp>



static VOID WINAPI SvcMain(DWORD argc, LPWSTR *argv);
static DWORD WINAPI SvcHandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);
nudf::win::svc::CRefPtr nudf::win::svc::_RefIServicePtr;


using namespace nudf;
using namespace nudf::win;
using namespace nudf::win::svc;

// run as service
static SERVICE_TABLE_ENTRYW SvcDispatchTable[] = {
    { NULL, (LPSERVICE_MAIN_FUNCTIONW)SvcMain},
    { NULL, NULL }
};


DWORD WINAPI SvcHandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
    nudf::win::svc::IService* spService = (nudf::win::svc::IService*)lpContext;

    if(spService == NULL) {
        return ERROR_INVALID_HANDLE;
    }

    return (DWORD)spService->Control(dwControl, dwEventType, lpEventData, NULL);
}

VOID WINAPI SvcMain(DWORD argc, LPWSTR *argv)
{
    if(NULL == _RefIServicePtr.Get()) {
        SetLastError(ERROR_SERVICE_DOES_NOT_EXIST);
        throw WIN32ERROR();
    }
    
    ((IService*)_RefIServicePtr.Get())->Start(argc, (LPCWSTR*)argv);
}


//
//  Service Interface
//

nudf::win::svc::IService::IService()
{
    memset(&m_Status, 0, sizeof(m_Status));
    m_Status.dwCurrentState = SERVICE_STOPPED;
    m_Status.dwServiceType = SERVICE_STOPPED;
    m_Status.dwServiceSpecificExitCode = 0;
    m_Status.dwWin32ExitCode = 0;
    m_Status.dwControlsAccepted =  SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_PRESHUTDOWN | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP |
                                   SERVICE_ACCEPT_HARDWAREPROFILECHANGE | SERVICE_ACCEPT_POWEREVENT | SERVICE_ACCEPT_SESSIONCHANGE | SERVICE_ACCEPT_TIMECHANGE;
}

nudf::win::svc::IService::IService(_In_ LPCWSTR Name)
{
    if(NULL != Name) {
        m_Name = Name;
    }
    memset(&m_Status, 0, sizeof(m_Status));
    m_Status.dwCurrentState = SERVICE_STOPPED;
    SvcDispatchTable[0].lpServiceName = (LPWSTR)m_Name.c_str();
}

nudf::win::svc::IService::~IService()
{
    Stop();
}

VOID nudf::win::svc::IService::Run()
{
    SvcDispatchTable[0].lpServiceName = (LPWSTR)m_Name.c_str();
    if (!StartServiceCtrlDispatcherW(SvcDispatchTable)) {
        throw WIN32ERROR();
    }
}

LONG nudf::win::svc::IService::Start(_In_ int Argc, _In_ LPCWSTR* Argv) throw()
{
    LONG    lRet = 0L;

    try {

        m_hStatus = RegisterServiceCtrlHandlerExW((LPWSTR)m_Name.c_str(), SvcHandlerEx, this);
        if (NULL == m_hStatus) {
            throw WIN32ERROR();
        }

        m_Status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
        m_Status.dwServiceSpecificExitCode = 0;
        SetStatus(SERVICE_START_PENDING, NO_ERROR, 2000);
        
        m_hStopEvt = ::CreateEventW(NULL, TRUE, FALSE, NULL);
        if (NULL == m_hStopEvt) {
            throw WIN32ERROR();
        }

        // Started
        SetStatus(SERVICE_RUNNING, NO_ERROR, 0);

        // Fire start event
        OnStart();

        //
        //  Wait for stop event
        //
        WaitForSingleObject(m_hStopEvt, INFINITE);

        // Fire stop event
        OnStop();

        // Stopped
        SetStatus(SERVICE_STOPPED, 0, 0);
        CloseHandle(m_hStopEvt);
        m_hStopEvt = NULL;
    }
    catch(const nudf::CException& e) {
        lRet = e.GetCode();
        if(NULL != m_hStopEvt) {
            CloseHandle(m_hStopEvt);
            m_hStopEvt = NULL;
        }
        SetStatus(SERVICE_STOPPED, lRet, 0);
    }

    return lRet;
}

VOID nudf::win::svc::IService::Stop() throw()
{
    if (NULL == m_hStatus || SERVICE_STOPPED == m_Status.dwCurrentState || SERVICE_STOP_PENDING == m_Status.dwCurrentState) {
        return;
    }
    

    SetStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
    (void)SetEvent(m_hStopEvt);
    SetStatus(m_Status.dwCurrentState, NO_ERROR, 0);
}

LONG nudf::win::svc::IService::Pause() throw()
{
    LONG    lRet = 0L;

    if (SERVICE_RUNNING != m_Status.dwCurrentState) {
        return ERROR_SERVICE_NOT_ACTIVE;
    }

    SetStatus(SERVICE_PAUSE_PENDING, NO_ERROR, 0);
    try {
        OnPause();
        SetStatus(SERVICE_PAUSED, NO_ERROR, 0);
    }
    catch (const nudf::CException& e) {
        SetStatus(SERVICE_RUNNING, NO_ERROR, 0);
        lRet = e.GetCode();
    }

    return lRet;
}

LONG nudf::win::svc::IService::Resume() throw()
{
    LONG    lRet = 0L;

    if (SERVICE_PAUSED != m_Status.dwCurrentState) {
        return ERROR_EXCEPTION_IN_SERVICE;
    }

    SetStatus(SERVICE_CONTINUE_PENDING, NO_ERROR, 0);
    try {
        OnPause();
        SetStatus(SERVICE_RUNNING, NO_ERROR, 0);
    }
    catch (const nudf::CException& e) {
        SetStatus(SERVICE_PAUSED, NO_ERROR, 0);
        lRet = e.GetCode();
    }

    return lRet;
}

VOID nudf::win::svc::IService::SetStatus(_In_ ULONG State, _In_ ULONG ExitCode, _In_ ULONG Hint) throw()
{
    static ULONG checkpoint = 1;

    // Fill in the SERVICE_STATUS structure.
    m_Status.dwCurrentState  = State;
    m_Status.dwWin32ExitCode = ExitCode;
    m_Status.dwWaitHint      = Hint;
    m_Status.dwControlsAccepted =  SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_PRESHUTDOWN | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP |
                                   SERVICE_ACCEPT_HARDWAREPROFILECHANGE | SERVICE_ACCEPT_POWEREVENT | SERVICE_ACCEPT_SESSIONCHANGE | SERVICE_ACCEPT_TIMECHANGE;

    if (State == SERVICE_START_PENDING) {
        m_Status.dwControlsAccepted &= ~SERVICE_ACCEPT_STOP;
    }
    else {
        m_Status.dwControlsAccepted |= SERVICE_ACCEPT_STOP;
    }

    if ((State == SERVICE_RUNNING) || (State == SERVICE_STOPPED)) {
        m_Status.dwCheckPoint = 0;
    }
    else {
        m_Status.dwCheckPoint = checkpoint++;
    }

    // Report the status of the service to the SCM.
    BOOL bRet = SetServiceStatus(m_hStatus, &m_Status);
    assert(bRet);
}

LONG nudf::win::svc::IService::Control(_In_ ULONG ControlCode, _In_ ULONG EvtType, _In_opt_ LPVOID EvtData, _In_opt_ LPVOID Context) throw()
{
    long lRet = NO_ERROR;


    switch (ControlCode)
    {
    case SERVICE_CONTROL_STOP:
        Stop();
        return NO_ERROR;

    case SERVICE_CONTROL_CONTINUE:
        lRet = Resume();
        break;

    case SERVICE_CONTROL_PAUSE:
        lRet = Pause();
        break;

    case SERVICE_CONTROL_PARAMCHANGE:
        OnParamChange();
        break;

    case SERVICE_CONTROL_PRESHUTDOWN:
        OnPreshutdown();
        break;

    case SERVICE_CONTROL_SHUTDOWN:
        OnShutdown();
        break;

    case SERVICE_CONTROL_INTERROGATE:
        break;

    case SERVICE_CONTROL_NETBINDADD:
        OnNetbindAdd();
        break;

    case SERVICE_CONTROL_NETBINDREMOVE:
        OnNetbindRemove();
        break;

    case SERVICE_CONTROL_NETBINDENABLE:
        OnNetbindEnable();
        break;

    case SERVICE_CONTROL_NETBINDDISABLE:
        OnNetbindDisable();
        break;

        //
        // Codes supported by HandlerEx only
        //
    case SERVICE_CONTROL_DEVICEEVENT:
        switch (EvtType)
        {
        case DBT_DEVICEARRIVAL:
            lRet = OnDeviceArrival((DEV_BROADCAST_HDR*)EvtData);
            break;
        case DBT_DEVICEREMOVECOMPLETE:
            lRet = OnDeviceRemoveComplete((DEV_BROADCAST_HDR*)EvtData);
            break;
        case DBT_DEVICEQUERYREMOVE:
            lRet = OnDeviceQueryRemove((DEV_BROADCAST_HDR*)EvtData);
            break;
        case DBT_DEVICEQUERYREMOVEFAILED:
            lRet = OnDeviceQueryRemoveFailed((DEV_BROADCAST_HDR*)EvtData);
            break;
        case DBT_DEVICEREMOVEPENDING:
            lRet = OnDeviceRemovePending((DEV_BROADCAST_HDR*)EvtData);
            break;
        case DBT_CUSTOMEVENT:
            lRet = OnDeviceCustomEvent((DEV_BROADCAST_HDR*)EvtData);
            break;
        default:
            break;
        }
        break;

    case SERVICE_CONTROL_HARDWAREPROFILECHANGE:
        switch (EvtType)
        {
        case DBT_CONFIGCHANGED:
            lRet = OnHwprofileChanged();
            break;
        case DBT_QUERYCHANGECONFIG:
            lRet = OnHwprofileQueryChange();
            break;
        case DBT_CONFIGCHANGECANCELED:
            lRet = OnHwprofileChangeCanceled();
            break;
        default:
            break;
        }
        break;

    case SERVICE_CONTROL_POWEREVENT:
        switch (EvtType)
        {
        case PBT_APMPOWERSTATUSCHANGE:
            lRet = OnPowerStatusChanged();
            break;
        case PBT_APMRESUMEAUTOMATIC:
            lRet = OnPowerResumeAuto();
            break;
        case PBT_APMRESUMESUSPEND:
            lRet = OnPowerResumeSuspend();
            break;
        case PBT_APMSUSPEND:
            lRet = OnPowerSuspend();
            break;
        case PBT_POWERSETTINGCHANGE:
            lRet = OnPowerSettingChanged((POWERBROADCAST_SETTING*)EvtData);
            break;
        case PBT_APMBATTERYLOW:
            lRet = OnPowerLowbattery();
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
        switch (EvtType)
        {
        case WTS_CONSOLE_CONNECT:
            lRet = OnSessionConn((WTSSESSION_NOTIFICATION*)EvtData);
            break;
        case WTS_CONSOLE_DISCONNECT:
            lRet = OnSessionDisconn((WTSSESSION_NOTIFICATION*)EvtData);
            break;
        case WTS_REMOTE_CONNECT:
            lRet = OnSessionRemoteConn((WTSSESSION_NOTIFICATION*)EvtData);
            break;
        case WTS_REMOTE_DISCONNECT:
            lRet = OnSessionRemoteDisconn((WTSSESSION_NOTIFICATION*)EvtData);
            break;
        case WTS_SESSION_LOGON:
            lRet = OnSessionLogon((WTSSESSION_NOTIFICATION*)EvtData);
            break;
        case WTS_SESSION_LOGOFF:
            lRet = OnSessionLogoff((WTSSESSION_NOTIFICATION*)EvtData);
            break;
        case WTS_SESSION_LOCK:
            lRet = OnSessionLock((WTSSESSION_NOTIFICATION*)EvtData);
            break;
        case WTS_SESSION_UNLOCK:
            lRet = OnSessionUnlock((WTSSESSION_NOTIFICATION*)EvtData);
            break;
        case WTS_SESSION_REMOTE_CONTROL:
            lRet = OnSessionRemoteControl((WTSSESSION_NOTIFICATION*)EvtData);
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

    return lRet;
}


//
//  Service COnfig
//

nudf::win::svc::CConfig::CConfig() throw() : m_dwServiceType(0),  m_dwStartType(SERVICE_DEMAND_START),  m_dwErrorControl(SERVICE_ERROR_NORMAL),  m_dwTagId(0)
{
}

nudf::win::svc::CConfig::~CConfig() throw()
{
}

VOID nudf::win::svc::CConfig::Load(_In_ SC_HANDLE hService)
{
    std::vector<unsigned char> buf;
    LPQUERY_SERVICE_CONFIG lpServiceConfig = NULL;
    DWORD cbBufSize = 0;
    DWORD cbBytesNeeded = 0;

    QueryServiceConfigW(hService, NULL, 0, &cbBytesNeeded);
    if(ERROR_INSUFFICIENT_BUFFER != GetLastError()) {
        throw WIN32ERROR();
    }

    cbBufSize = cbBytesNeeded + 2;
    buf.resize(cbBufSize, 0);
    lpServiceConfig = (LPQUERY_SERVICE_CONFIG)buf.data();
    
    if(!QueryServiceConfigW(hService, lpServiceConfig, cbBufSize, &cbBytesNeeded))
    if(ERROR_INSUFFICIENT_BUFFER != GetLastError()) {
        throw WIN32ERROR();
    }

    Clear();
    m_dwServiceType = lpServiceConfig->dwServiceType;
    m_dwStartType = lpServiceConfig->dwStartType;
    m_dwErrorControl = lpServiceConfig->dwErrorControl;
    m_dwTagId = lpServiceConfig->dwTagId;
    m_wsBinaryPathName = lpServiceConfig->lpBinaryPathName?lpServiceConfig->lpBinaryPathName:L"";
    m_wsLoadOrderGroup = lpServiceConfig->lpLoadOrderGroup?lpServiceConfig->lpLoadOrderGroup:L"";
    m_wsServiceStartName = lpServiceConfig->lpServiceStartName?lpServiceConfig->lpServiceStartName:L"";
    m_wsDisplayName = lpServiceConfig->lpDisplayName?lpServiceConfig->lpDisplayName:L"";
    while(NULL != lpServiceConfig->lpDependencies && lpServiceConfig->lpDependencies[0] != L'\0') {
        m_vDependencies.push_back(lpServiceConfig->lpDependencies);
        lpServiceConfig->lpDependencies += (m_vDependencies[m_vDependencies.size()-1].length() + 1);
    }
    
    lpServiceConfig = NULL;
}

VOID nudf::win::svc::CConfig::Clear() throw()
{
    m_dwServiceType = 0;
    m_dwStartType = SERVICE_DEMAND_START;
    m_dwErrorControl = SERVICE_ERROR_NORMAL;
    m_dwTagId = 0;
    m_wsBinaryPathName = L"";
    m_wsLoadOrderGroup = L"";
    m_wsServiceStartName = L"";
    m_wsDisplayName = L"";
    m_vDependencies.clear();
}

CConfig& nudf::win::svc::CConfig::operator= (const CConfig& Config) throw()
{
    if((&Config) != this) {
        Clear();
        m_dwServiceType = Config.GetServiceType();
        m_dwStartType = Config.GetStartType();
        m_dwErrorControl = Config.GetErrorControl();
        m_dwTagId = Config.GetTagId();
        m_wsBinaryPathName = Config.GetBinaryPathName();
        m_wsLoadOrderGroup = Config.GetLoadOrderGroup();
        m_wsServiceStartName = Config.GetServiceStartName();
        m_wsDisplayName = Config.GetDisplayName();
        m_vDependencies = Config.GetDependencies();
    }
    return *this;
}

//
//  Service Object
//

nudf::win::svc::CService::CService()
{
}

nudf::win::svc::CService::~CService()
{
    Close();
}

VOID nudf::win::svc::CService::Create(_In_ LPCWSTR Name, _In_ const CConfig& Config)
{
    ULONG   TagId = 0;
    LPCWSTR DisplayName = Config.GetDisplayName().empty() ? Name : Config.GetDisplayName().c_str();
    LPCWSTR BinaryPath = Config.GetBinaryPathName().c_str();
    LPCWSTR LoadOrderGroup = Config.GetLoadOrderGroup().empty() ? NULL : Config.GetLoadOrderGroup().c_str();
    DWORD   ServiceType = Config.GetServiceType();
    DWORD   StartType = Config.GetStartType();
    DWORD   ErrorControl = Config.GetErrorControl();

    Close();

    m_hSvcMgr = OpenSCManagerW(NULL, NULL,  SC_MANAGER_ALL_ACCESS); 
    if (NULL == m_hSvcMgr)  {
        throw WIN32ERROR();
    }

    m_hSvc = CreateServiceW(m_hSvcMgr,
                            Name,
                            Config.GetDisplayName().empty() ? Name : Config.GetDisplayName().c_str(),
                            SERVICE_ALL_ACCESS,
                            Config.GetServiceType(),
                            Config.GetStartType(),
                            Config.GetErrorControl(),
                            Config.GetBinaryPathName().c_str(),
                            Config.GetLoadOrderGroup().empty() ? NULL : Config.GetLoadOrderGroup().c_str(),
                            NULL,
                            NULL,
                            NULL,
                            NULL);
    if (NULL == m_hSvc)  {
        DWORD dwErr = GetLastError();
        CloseServiceHandle(m_hSvcMgr);
        m_hSvcMgr = NULL;
        SetLastError(dwErr);
        throw WIN32ERROR();
    }


    m_Name = Name;
    m_Config = Config;
    m_Config.SetTagId(TagId);
}

VOID nudf::win::svc::CService::Open(_In_ LPCWSTR Name, _In_ BOOL ReadOnly)
{
    ULONG   dwDesiredAccess = ReadOnly ? GENERIC_READ : (DELETE | SERVICE_ALL_ACCESS);

    Close();

    m_hSvcMgr = OpenSCManagerW(NULL, NULL,  SC_MANAGER_ALL_ACCESS); 
    if (NULL == m_hSvcMgr)  {
        throw WIN32ERROR();
    }

    m_hSvc = OpenServiceW( m_hSvcMgr, Name, dwDesiredAccess);
    if (NULL == m_hSvc)  {
        DWORD dwErr = GetLastError();
        CloseServiceHandle(m_hSvcMgr);
        m_hSvcMgr = NULL;
        SetLastError(dwErr);
        throw WIN32ERROR();
    }

    try {
        m_Config.Load(m_hSvc);
    }
    catch(const nudf::CException& e) {
        CloseServiceHandle(m_hSvc);
        m_hSvc = NULL;
        CloseServiceHandle(m_hSvcMgr);
        m_hSvcMgr = NULL;
        throw e;
    }

    m_Name = Name;
}

VOID nudf::win::svc::CService::Close() throw()
{
    if(NULL != m_hSvc) {
        CloseServiceHandle(m_hSvc);
        m_hSvc = NULL;
    }
    if(NULL != m_hSvcMgr) {
        CloseServiceHandle(m_hSvcMgr);
        m_hSvcMgr = NULL;
    }
    m_Config.Clear();
    m_Name = L"";
}

VOID nudf::win::svc::CService::Start(_In_ BOOL StartDependencies, _In_ ULONG WaitTime)
{
    SERVICE_STATUS  Status;
    ULONG lRet = 0;


    lRet = GetStatus(&Status, 0UL);
    if(0 != lRet) {
        SetLastError(lRet);
        throw WIN32ERROR();
    }

    if(SERVICE_STOP_PENDING != Status.dwCurrentState && SERVICE_STOPPED != Status.dwCurrentState) {
        // Already running
        return;
    }

    if(SERVICE_STOP_PENDING == Status.dwCurrentState) {
        lRet = GetStatus(&Status, WaitTime);
        if(0 != lRet) {
            SetLastError(lRet);
            throw WIN32ERROR();
        }
    }

    assert(SERVICE_STOPPED == Status.dwCurrentState);

    if(!StartServiceW(m_hSvc, 0, NULL)) {
        throw WIN32ERROR();
    }

    lRet = GetStatus(&Status, WaitTime);
    if(0 != lRet) {
        SetLastError(lRet);
        throw WIN32ERROR();
    }
}

VOID nudf::win::svc::CService::Stop(_In_ ULONG WaitTime)
{
    SERVICE_STATUS  Status;
    ULONG lRet = 0;


    lRet = GetStatus(&Status, 0UL);
    if(0 != lRet) {
        SetLastError(lRet);
        throw WIN32ERROR();
    }

    if(SERVICE_STOP_PENDING == Status.dwCurrentState) {
        if(0 == WaitTime) {
            return;
        }
        lRet = GetStatus(&Status, WaitTime);
        if(0 != lRet) {
            SetLastError(lRet);
            throw WIN32ERROR();
        }
    }

    if(SERVICE_STOPPED == Status.dwCurrentState) {
        // Already stopped
        return;
    }

    if(!ControlService(m_hSvc, SERVICE_CONTROL_STOP, &Status)) {
        throw WIN32ERROR();
    }

    if(SERVICE_STOP_PENDING == Status.dwCurrentState) {
        if(0 == WaitTime) {
            return;
        }
        lRet = GetStatus(&Status, WaitTime);
        if(0 != lRet) {
            SetLastError(lRet);
            throw WIN32ERROR();
        }
    }
}

VOID nudf::win::svc::CService::Pause(_In_ ULONG WaitTime)
{
    SERVICE_STATUS  Status;
    ULONG lRet = 0;


    lRet = GetStatus(&Status, 0UL);
    if(0 != lRet) {
        SetLastError(lRet);
        throw WIN32ERROR();
    }

    if(SERVICE_PAUSE_PENDING == Status.dwCurrentState) {
        if(0 == WaitTime) {
            return;
        }
        lRet = GetStatus(&Status, WaitTime);
        if(0 != lRet) {
            SetLastError(lRet);
            throw WIN32ERROR();
        }
    }
    if(SERVICE_PAUSED == Status.dwCurrentState) {
        return;
    }

    if(!ControlService(m_hSvc, SERVICE_CONTROL_PAUSE, &Status)) {
        throw WIN32ERROR();
    }

    if(SERVICE_PAUSE_PENDING == Status.dwCurrentState) {
        if(0 == WaitTime) {
            return;
        }
        lRet = GetStatus(&Status, WaitTime);
        if(0 != lRet) {
            SetLastError(lRet);
            throw WIN32ERROR();
        }
    }
}

VOID nudf::win::svc::CService::Resume(_In_ ULONG WaitTime)
{
    SERVICE_STATUS  Status;
    ULONG lRet = 0;


    lRet = GetStatus(&Status, 0UL);
    if(0 != lRet) {
        SetLastError(lRet);
        throw WIN32ERROR();
    }

    if(SERVICE_CONTINUE_PENDING == Status.dwCurrentState) {
        if(0 == WaitTime) {
            return;
        }
        lRet = GetStatus(&Status, WaitTime);
        if(0 != lRet) {
            SetLastError(lRet);
            throw WIN32ERROR();
        }
    }
    if(SERVICE_RUNNING == Status.dwCurrentState) {
        return;
    }

    if(!ControlService(m_hSvc, SERVICE_CONTROL_CONTINUE, &Status)) {
        throw WIN32ERROR();
    }

    if(SERVICE_CONTINUE_PENDING == Status.dwCurrentState) {
        if(0 == WaitTime) {
            return;
        }
        lRet = GetStatus(&Status, WaitTime);
        if(0 != lRet) {
            SetLastError(lRet);
            throw WIN32ERROR();
        }
    }
}

VOID nudf::win::svc::CService::Enable(_In_ ULONG StartType)
{
    SetServiceType(StartType);
}

VOID nudf::win::svc::CService::Disable()
{
    SetServiceType(SERVICE_DISABLED);
}

VOID nudf::win::svc::CService::Delete(_In_ ULONG WaitTime)
{
    Stop(WaitTime);
    DeleteService(m_hSvc);
}

ULONG nudf::win::svc::CService::GetStatus(_Out_ LPSERVICE_STATUS Status, _In_ ULONG PendingWaitTime) throw()
{
    ULONG dwTotalWaitTime = 0;

    if(!QueryServiceStatus(m_hSvc, Status)) {
        return GetLastError();
    }

    while(0 != PendingWaitTime && IsPendingState(Status->dwCurrentState)) {

        if(INFINITE != PendingWaitTime && dwTotalWaitTime > PendingWaitTime) {
            return ERROR_TIMEOUT;
        }

        ULONG dwWaitTime = max(Status->dwWaitHint, 300);
        Sleep(dwWaitTime);
        dwTotalWaitTime += dwWaitTime;

        if(!QueryServiceStatus(m_hSvc, Status)) {
            return GetLastError();
        }
    }

    return 0UL;
}

VOID nudf::win::svc::CService::SetBinaryPathName(_In_ const std::wstring& BinaryPathName)
{
    if(!ChangeServiceConfigW(m_hSvc,
                             SERVICE_NO_CHANGE, // ServiceType
                             SERVICE_NO_CHANGE, // StartType
                             SERVICE_NO_CHANGE, // ErrorControl
                             BinaryPathName.c_str(),    // BinaryPathName,
                             NULL,              // LoadOrderGroup,
                             NULL,              // TagId,
                             NULL,              // Dependencies,
                             NULL,              // ServiceStartName,
                             NULL,              // Password,
                             NULL               // DisplayName
                             )) {
        throw WIN32ERROR();
    }
}

VOID nudf::win::svc::CService::SetLoadOrderGroup(_In_ const std::wstring& LoadOrderGroup)
{
    if(!ChangeServiceConfigW(m_hSvc,
                             SERVICE_NO_CHANGE, // ServiceType
                             SERVICE_NO_CHANGE, // StartType
                             SERVICE_NO_CHANGE, // ErrorControl
                             NULL,              // BinaryPathName,
                             LoadOrderGroup.c_str(),    // LoadOrderGroup,
                             NULL,              // TagId,
                             NULL,              // Dependencies,
                             NULL,              // ServiceStartName,
                             NULL,              // Password,
                             NULL               // DisplayName
                             )) {
        throw WIN32ERROR();
    }
}

VOID nudf::win::svc::CService::SetDisplayName(_In_ const std::wstring& DisplayName)
{
    if(!ChangeServiceConfigW(m_hSvc,
                             SERVICE_NO_CHANGE, // ServiceType
                             SERVICE_NO_CHANGE, // StartType
                             SERVICE_NO_CHANGE, // ErrorControl
                             NULL,              // BinaryPathName,
                             NULL,              // LoadOrderGroup,
                             NULL,              // TagId,
                             NULL,              // Dependencies,
                             NULL,              // ServiceStartName,
                             NULL,              // Password,
                             DisplayName.c_str()// DisplayName
                             )) {
        throw WIN32ERROR();
    }
}

VOID nudf::win::svc::CService::SetDescription(_In_ const std::wstring& Description)
{
    SERVICE_DESCRIPTION Info = {(PWCHAR)Description.c_str()};
    if(!ChangeServiceConfig2W(m_hSvc, SERVICE_CONFIG_DESCRIPTION, &Info)) {
        throw WIN32ERROR();
    }
}

VOID nudf::win::svc::CService::SetServiceType(_In_ ULONG ServiceType)
{
    if(!ChangeServiceConfigW(m_hSvc,
                             ServiceType,       // ServiceType
                             SERVICE_NO_CHANGE, // StartType
                             SERVICE_NO_CHANGE, // ErrorControl
                             NULL,              // BinaryPathName,
                             NULL,              // LoadOrderGroup,
                             NULL,              // TagId,
                             NULL,              // Dependencies,
                             NULL,              // ServiceStartName,
                             NULL,              // Password,
                             NULL               // DisplayName
                             )) {
        throw WIN32ERROR();
    }

    m_Config.SetServiceType(ServiceType);
}

VOID nudf::win::svc::CService::SetStartType(_In_ ULONG StartType)
{
    if(!ChangeServiceConfigW(m_hSvc,
                             SERVICE_NO_CHANGE, // ServiceType
                             StartType,         // StartType
                             SERVICE_NO_CHANGE, // ErrorControl
                             NULL,              // BinaryPathName,
                             NULL,              // LoadOrderGroup,
                             NULL,              // TagId,
                             NULL,              // Dependencies,
                             NULL,              // ServiceStartName,
                             NULL,              // Password,
                             NULL               // DisplayName
                             )) {
        throw WIN32ERROR();
    }

    m_Config.SetStartType(StartType);
}

VOID nudf::win::svc::CService::SetErrorControl(_In_ ULONG ErrorControl)
{
    if(!ChangeServiceConfigW(m_hSvc,
                             SERVICE_NO_CHANGE, // ServiceType
                             SERVICE_NO_CHANGE, // StartType
                             ErrorControl,      // ErrorControl
                             NULL,              // BinaryPathName,
                             NULL,              // LoadOrderGroup,
                             NULL,              // TagId,
                             NULL,              // Dependencies,
                             NULL,              // ServiceStartName,
                             NULL,              // Password,
                             NULL               // DisplayName
                             )) {
        throw WIN32ERROR();
    }

    m_Config.SetErrorControl(ErrorControl);
}

VOID nudf::win::svc::CService::SetDependencies(_In_ const std::vector<std::wstring>& Dependencies)
{
    PWCHAR  NewDependencies = NULL;
    ULONG   NewDependenciesLength = 0;

    if(!Dependencies.empty()) {

        for(int i=0; i<(int)Dependencies.size(); i++) {
            NewDependenciesLength += (ULONG)Dependencies[i].length() + 1;
        }
        NewDependenciesLength += 1; // Extra NULL-Terminator

        NewDependencies = new (std::nothrow) WCHAR[NewDependenciesLength];
        if(NULL == NewDependencies) {
            SetLastError(ERROR_OUTOFMEMORY);
            throw WIN32ERROR();
        }
        memset(NewDependencies, 0, sizeof(WCHAR)*NewDependenciesLength);

        PWCHAR Str = NewDependencies;
        for(int i=0; i<(int)Dependencies.size(); i++) {
            memcpy(Str, Dependencies[i].c_str(), Dependencies[i].length() * sizeof(WCHAR));
            Str += (Dependencies[i].length() + 1);
        }
    }

    if(!ChangeServiceConfigW(m_hSvc,
                             SERVICE_NO_CHANGE, // ServiceType
                             SERVICE_NO_CHANGE, // StartType
                             SERVICE_NO_CHANGE, // ErrorControl
                             NULL,              // BinaryPathName,
                             NULL,              // LoadOrderGroup,
                             NULL,              // TagId,
                             (NULL != NewDependencies) ? NewDependencies : L"", //Dependencies,
                             NULL,              // ServiceStartName,
                             NULL,              // Password,
                             NULL               // DisplayName
                             )) {
        if(NULL != NewDependencies){
            delete []NewDependencies;
            NewDependencies = NULL;
        }
        throw WIN32ERROR();
    }

    if(NULL != NewDependencies){
        delete []NewDependencies;
        NewDependencies = NULL;
    }

    m_Config.SetDependencies(Dependencies);
}


BOOL nudf::win::svc::Exist(_In_ LPCWSTR Name) throw()
{
    SC_HANDLE hSvcMgr = NULL;
    SC_HANDLE hSvc = NULL;
    DWORD dwError = 0;

    hSvcMgr = OpenSCManagerW(NULL, NULL,  SC_MANAGER_ENUMERATE_SERVICE); 
    if (NULL == hSvcMgr)  {
        return FALSE;
    }

    hSvc = OpenServiceW(hSvcMgr, Name, SERVICE_QUERY_CONFIG);
    dwError = GetLastError();
    if(NULL != hSvc) {
        CloseServiceHandle(hSvc);
    }
    if(NULL != hSvcMgr) {
        CloseServiceHandle(hSvcMgr);
    }

    return (ERROR_SERVICE_DOES_NOT_EXIST == dwError) ? FALSE : TRUE;
}

VOID nudf::win::svc::Delete(_In_ LPCWSTR Name)
{
    nudf::win::svc::CService Svc;

    Svc.Open(Name, FALSE);
    Svc.Delete();
}