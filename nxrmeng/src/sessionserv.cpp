
#include <Windows.h>
#include <assert.h>
#include <Wtsapi32.h>
#include <Sddl.h>
#include <Shlobj.h>
#include <TlHelp32.h>

#include <nudf\exception.hpp>
#include <nudf\shared\enginectl.h>
#include <nudf\shared\enginectl.h>
#include <nudf\registry.hpp>

#include "nxrmeng.h"
#include "engine.hpp"
#include "sessionserv.hpp"


using namespace nxrm::engine;


//
//  class CSessionKnownDirs
//

CSessionKnownDirs::CSessionKnownDirs()
{
}

CSessionKnownDirs::~CSessionKnownDirs()
{
}

void CSessionKnownDirs::Load(_In_ HANDLE hToken) throw()
{
    CKnownDirs::Load(hToken);

    _dirNxLocalAppData = GetLocalAppDataDir();
    _dirNxLocalConf    = GetLocalAppDataDir();
    _dirNxLocalCache   = GetLocalAppDataDir();
    _dirNxLocalAppData += L"\\NextLabs";
    _dirNxLocalConf    += L"\\NextLabs\\conf";
    _dirNxLocalCache   += L"\\NextLabs\\cache";
    ::CreateDirectoryW(_dirNxLocalAppData.c_str(), NULL);
    ::CreateDirectoryW(_dirNxLocalConf.c_str(), NULL);
    ::CreateDirectoryW(_dirNxLocalCache.c_str(), NULL);
}

void CSessionKnownDirs::Clear()
{
    _dirNxLocalAppData.clear();
    _dirNxLocalConf.clear();
    _dirNxLocalCache.clear();
    CKnownDirs::Clear();
}


//
//  class CSession
//

DWORD WINAPI SessionWorkThread(_In_ LPVOID lpData);
DWORD WINAPI ClientGuardThread(_In_ LPVOID lpData);

CActiveSession::CActiveSession() : CSession(), _hClientProcess(NULL), _ShuttingDown(FALSE)
{
}

CActiveSession::CActiveSession(_In_ ULONG dwSessionId) : CSession(dwSessionId), _hClientProcess(NULL), _ShuttingDown(FALSE)
{
    WCHAR wzClientPort[128] = {0};
    swprintf_s(wzClientPort, 128, L"nxrmtray-%d", dwSessionId);
    _ClientPort = wzClientPort;
}

CActiveSession::CActiveSession(_In_ ULONG dwSessionId, _In_ LPCWSTR wzPath, _In_opt_ LPCWSTR wzArgs) : CSession(dwSessionId), _hClientProcess(NULL), _ShuttingDown(FALSE), _ClientPath(wzPath), _ClientArgs(wzArgs?wzArgs:L"")
{
    WCHAR wzClientPort[128] = {0};
    swprintf_s(wzClientPort, 128, L"nxrmtray-%d", dwSessionId);
    _ClientPort = wzClientPort;
}

CActiveSession::~CActiveSession()
{
}

VOID CActiveSession::Start()
{
    ULONG dwThreadId = 0;

    // Get Logon User Information
    if(!GetUser(_User)) {
        SetLastError(ERROR_NO_SUCH_LOGON_SESSION);
        LOGERR(ERROR_NO_SUCH_LOGON_SESSION, L"Fail to get user information");
        throw WIN32ERROR();
    }
    
    HANDLE hToken = GetToken();
    if(NULL != hToken) {
        // Load all the known dirs
        _dirs.Load(hToken);
        CloseHandle(hToken);
        hToken = NULL;
    }
    
    // Create Guard Thread, and this guard thread will start a client
    KillExistingClient();
    _hClientGuardThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ClientGuardThread, this, 0, &dwThreadId);
}

VOID CActiveSession::Stop() throw()
{
    // Shutdown the client app
    _ShuttingDown = TRUE;
    if(NULL != _hClientGuardThread) {
        // terminate target
        
        CClientConn cltconn(_ClientPort.c_str());
        try {
            cltconn.Quit();
        }
        catch(const nudf::CException& e) {
            (VOID)TerminateProcess(_hClientProcess, 0);
            UNREFERENCED_PARAMETER(e);
        }
        ::WaitForSingleObject(_hClientGuardThread, INFINITE);
        CloseHandle(_hClientGuardThread);
        _hClientGuardThread = NULL;
    }

    // Done
    _ShuttingDown = FALSE;
}

VOID CActiveSession::StartClientApp()
{
    HANDLE hToken = NULL;
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    DWORD dwError = 0;
    
    hToken = GetToken();
    if(NULL == hToken) {
        dwError = GetLastError();
        SetLastError(dwError);
        throw WIN32ERROR();
    }

    memset(&pi, 0, sizeof(pi));
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(STARTUPINFO); 
    si.lpDesktop = L"WinSta0\\Default";
    LOGDBG(L"  Tray-app starting (%s %s)", _ClientPath.c_str(), _User.GetPrincipleName().empty()?_User.GetName().c_str():_User.GetPrincipleName().c_str(), _ClientArgs.length()?_ClientArgs.c_str():L"");
    if(!::CreateProcessAsUserW(hToken, _ClientPath.c_str(), (LPWSTR)(_ClientArgs.length()?_ClientArgs.c_str():NULL), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        dwError = GetLastError();
        CloseHandle(hToken);
        SetLastError(dwError);
        throw WIN32ERROR();
    }

    _hClientProcess = pi.hProcess;
    CloseHandle(pi.hThread);
    CloseHandle(hToken);
}

void CActiveSession::KillExistingClient()
{
    HANDLE hSnap = INVALID_HANDLE_VALUE;
    PROCESSENTRY32W pe32;
    
    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(INVALID_HANDLE_VALUE == hSnap) {
        return;
    }

    memset(&pe32, 0, sizeof(pe32));
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if(!Process32FirstW(hSnap, &pe32)) {
        CloseHandle(hSnap);
        hSnap = INVALID_HANDLE_VALUE;
        return;
    }

    do {
        DWORD dwSessionId = -1;
        if(!ProcessIdToSessionId(pe32.th32ProcessID, &dwSessionId)) {
            continue;
        }
        if(dwSessionId != GetSessionId()) {
            continue;
        }
        WCHAR* pName = wcsrchr(pe32.szExeFile, L'\\');
        if(NULL==pName) pName = pe32.szExeFile;
        if(0 == _wcsicmp(pName, L"nxrmtray.exe")) {
            HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
            if(NULL != hProcess) {
                TerminateProcess(hProcess, 0);
                CloseHandle(hProcess);
            }
        }
    } while(Process32Next(hSnap, &pe32));

    CloseHandle(hSnap);
    hSnap = INVALID_HANDLE_VALUE;
}

VOID CActiveSession::ClientGuard() throw()
{
    ULONG dwWaitCount = 0;
    while(!_ShuttingDown) {

        if(NULL != _hClientProcess) {
            DWORD dwWaitResult = ::WaitForSingleObject(_hClientProcess, INFINITE);
            if(WAIT_OBJECT_0 != dwWaitResult) {
                LOGERR(L"ClientGuard: WaitForSingleObject failed (0x%08X)", dwWaitResult);
                if(dwWaitCount < 10) {
                    Sleep(1000);   // Wait 1 seconds before we start
                    dwWaitCount++;
                    LOGERR(L"             Retry WaitForSingleObject (%d)", dwWaitCount);
                    continue;
                }
                else {
                    LOGERR(L"             Reach maximum retry count (%d)", dwWaitCount);
                }
            }
            LOGDBG(L"Tray-app (%p, session: %d, user: %s) exit", _hClientProcess, GetSessionId(), _User.GetPrincipleName().empty()?_User.GetName().c_str():_User.GetPrincipleName().c_str());
            CloseHandle(_hClientProcess);
            _hClientProcess = NULL;
        }
        if(_ShuttingDown) {
            break;
        }
        Sleep(1000);   // Wait 1 seconds before we start
        if(_ShuttingDown) {
            break;
        }

        // Start Client
        try {
            LOGDBG(L"Starting tray-app for session-%d, user: %s", GetSessionId(), _User.GetPrincipleName().empty()?_User.GetName().c_str():_User.GetPrincipleName().c_str());
            StartClientApp();
            LOGDBG(L"Tray-app (%p) started", _hClientProcess);
            CClientConn cltconn(_ClientPort.c_str());
            cltconn.SetTimeStamp(&gpEngine->GetPolicyServ()->GetPolicyTimestamp(), &gpEngine->GetLastHeartBeatTime());
        }
        catch(const nudf::CException& e) {
            // Fail to start
            assert(NULL == _hClientProcess);
            LOG_EXCEPTION2(e, L"Fail to start tray-app");
            UNREFERENCED_PARAMETER(e);
        }
    }
}

DWORD WINAPI ClientGuardThread(_In_ LPVOID lpData)
{
    CActiveSession* pSession = (CActiveSession*)lpData;

    assert(NULL != lpData);
    if(NULL == lpData) {
        return ERROR_INVALID_PARAMETER;
    }

    pSession->ClientGuard();
    return 0;
}


//
//  class CSessions
//
CClientConn::CClientConn() : nudf::ipc::CPipeClient()
{
}

CClientConn::CClientConn(_In_ LPCWSTR wzClientPort) : nudf::ipc::CPipeClient(wzClientPort)
{
}

CClientConn::~CClientConn()
{
}

VOID CClientConn::Quit() throw()
{
    RM_TRAYAPP_REQUEST  Request;

    memset(&Request, 0, sizeof(Request));
    Request.Header.Size = sizeof(Request);
    Request.Header.CtlCode = TRAYAPP_CTL_QUIT;
    Request.Exit.Reserved = 0;

    try {
        Connect(5000);
        this->Request((PUCHAR)&Request, Request.Header.Size, NULL, 0, NULL);
        Disconnect();
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
    }
}

VOID CClientConn::Notify(_In_ const std::wstring& title, _In_ const std::wstring& info) throw()
{
    RM_TRAYAPP_REQUEST  Request;

    memset(&Request, 0, sizeof(Request));
    Request.Header.Size = sizeof(Request);
    Request.Header.CtlCode = TRAYAPP_CTL_NOTIFY;
    wcsncpy_s(Request.Notify.InfoTitle, 64, title.c_str(), _TRUNCATE);
    wcsncpy_s(Request.Notify.Info, 256, info.c_str(), _TRUNCATE);
    
    try {
        Connect(5000);
        this->Request((PUCHAR)&Request, Request.Header.Size, NULL, 0, NULL);
        Disconnect();
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
    }
}

VOID CClientConn::SetServerStatus(_In_ const std::wstring& server, _In_ BOOL connected) throw()
{
    RM_TRAYAPP_REQUEST  Request;

    memset(&Request, 0, sizeof(Request));
    Request.Header.Size = sizeof(Request);
    Request.Header.CtlCode = TRAYAPP_CTL_SET_SERVER_STATUS;
    wcsncpy_s(Request.SetServerStatus.Server, 128, server.c_str(), _TRUNCATE);
    Request.SetServerStatus.Connected = connected?TRUE:FALSE;
    
    try {
        Connect(5000);
        this->Request((PUCHAR)&Request, Request.Header.Size, NULL, 0, NULL);
        Disconnect();
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
    }
}

VOID CClientConn::SetTimeStamp(_In_ const FILETIME* ftPolicy, _In_ const FILETIME* ftUpdate) throw()
{
    RM_TRAYAPP_REQUEST  Request;

    memset(&Request, 0, sizeof(Request));
    Request.Header.Size = sizeof(Request);
    Request.Header.CtlCode = TRAYAPP_CTL_SET_TIMESTAMP;
    memcpy(&Request.SetTimeStamp.PolicyTimestamp, ftPolicy, sizeof(FILETIME));
    memcpy(&Request.SetTimeStamp.UpdateTimestamp, ftUpdate, sizeof(FILETIME));
    
    try {
        Connect(5000);
        this->Request((PUCHAR)&Request, Request.Header.Size, NULL, 0, NULL);
        Disconnect();
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
    }
}


//
//  class CSessions
//

CSessionServ::CSessionServ()
{
    ::InitializeCriticalSection(&_ActiveSessionsLock);
    ::InitializeCriticalSection(&_LoggedOnUsersLock);
}

CSessionServ::~CSessionServ()
{
    ::DeleteCriticalSection(&_LoggedOnUsersLock);
    ::DeleteCriticalSection(&_ActiveSessionsLock);
}

BOOL CSessionServ::Exists(_In_ ULONG dwSessionId) throw()
{
    BOOL bExists = FALSE;
    ::EnterCriticalSection(&_ActiveSessionsLock);
    for(std::list<std::shared_ptr<CActiveSession>>::iterator it=_ActiveSessions.begin(); it!=_ActiveSessions.end(); ++it) {
        if(dwSessionId == (*it)->GetSessionId()) {
            bExists = TRUE;
            break;
        }
    }
    ::LeaveCriticalSection(&_ActiveSessionsLock);
    return bExists;
}

std::shared_ptr<CActiveSession> CSessionServ::FindSession(_In_ ULONG dwSessionId) throw()
{
    std::shared_ptr<CActiveSession> spSession = NULL;
    ::EnterCriticalSection(&_ActiveSessionsLock);
    for(std::list<std::shared_ptr<CActiveSession>>::iterator it=_ActiveSessions.begin(); it!=_ActiveSessions.end(); ++it) {
        if(dwSessionId == (*it)->GetSessionId()) {
            spSession = (*it);
            break;
        }
    }
    ::LeaveCriticalSection(&_ActiveSessionsLock);
    return spSession;
}

VOID CSessionServ::AddSession(_In_ ULONG dwSessionId)
{
    CActiveSession*   pSession = NULL;
        
    pSession = new (std::nothrow) CActiveSession(dwSessionId, _nxPaths.GetTrayExe().c_str(), gpEngine->GetSessionGuid().c_str());
    if(NULL == pSession) {
        SetLastError(ERROR_OUTOFMEMORY);
        throw WIN32ERROR();
    }

    try {
        pSession->Start();
    }
    catch(const nudf::CException& e) {
        delete pSession;
        pSession = NULL;
        throw e;
    }


    if(NULL != pSession) {

        nudf::win::CUser user;
        BOOL             added = FALSE;
        pSession->GetUser(user);
        AddLoggedOnUser(user.GetSid().GetSidStr(), user.GetPrincipleName().empty()?user.GetName():user.GetPrincipleName(), &added);

        ::EnterCriticalSection(&_ActiveSessionsLock);
        _ActiveSessions.push_back(std::shared_ptr<CActiveSession>(pSession));
        ::LeaveCriticalSection(&_ActiveSessionsLock);
    }

    gpEngine->GetPolicyServ()->CreateSessionPolicySet(dwSessionId);
}

VOID CSessionServ::RemoveSession(_In_ ULONG dwSessionId)
{
    std::shared_ptr<CActiveSession> spSession;

    
    gpEngine->GetPolicyServ()->RemoveSessionPolicySet(dwSessionId);

    ::EnterCriticalSection(&_ActiveSessionsLock);
    for(std::list<std::shared_ptr<CActiveSession>>::iterator it=_ActiveSessions.begin(); it!=_ActiveSessions.end(); ++it) {
        if(dwSessionId == (*it)->GetSessionId()) {
            spSession = *it;
            _ActiveSessions.remove(*it);
            break;
        }
    }
    ::LeaveCriticalSection(&_ActiveSessionsLock);

    if(NULL == spSession) {
        SetLastError(ERROR_NOT_FOUND);
        throw WIN32ERROR();
    }
    
    spSession->Stop();
    spSession.reset();
}

VOID CSessionServ::NotifyClient(_In_ ULONG dwSessionId, _In_ LPCWSTR wzTitle, _In_ LPCWSTR wzNotification)
{
    std::vector<std::wstring> vPorts;

    ::EnterCriticalSection(&_ActiveSessionsLock);    
    for(std::list<std::shared_ptr<CActiveSession>>::iterator it=_ActiveSessions.begin(); it!=_ActiveSessions.end(); ++it) {
        if(-1 == dwSessionId) {
            vPorts.push_back((*it)->GetClientPort());
        }
        else {
            if(dwSessionId == (*it)->GetSessionId()) {
                vPorts.push_back((*it)->GetClientPort());
                break;
            }
        }
    }
    ::LeaveCriticalSection(&_ActiveSessionsLock);

    for(std::vector<std::wstring>::const_iterator it=vPorts.begin(); it!=vPorts.end(); ++it) {
        CClientConn cltconn((*it).c_str());
        cltconn.Notify(wzTitle, wzNotification);
    }
}

VOID CSessionServ::NotifyClientTimeStamp(_In_ const FILETIME* policyTime, _In_ const FILETIME* updateTime)
{
    ::EnterCriticalSection(&_ActiveSessionsLock);
    for(std::list<std::shared_ptr<CActiveSession>>::iterator it=_ActiveSessions.begin(); it!=_ActiveSessions.end(); ++it) {
        CClientConn cltconn((*it)->GetClientPort().c_str());
        cltconn.SetTimeStamp(policyTime, updateTime);
    }
    ::LeaveCriticalSection(&_ActiveSessionsLock);
}

VOID CSessionServ::NotifyClientServerStatus(_In_ const std::wstring& server, _In_ BOOL connected)
{
    ::EnterCriticalSection(&_ActiveSessionsLock);
    for(std::list<std::shared_ptr<CActiveSession>>::iterator it=_ActiveSessions.begin(); it!=_ActiveSessions.end(); ++it) {
        CClientConn cltconn((*it)->GetClientPort().c_str());
        cltconn.SetServerStatus(server, connected);
    }
    ::LeaveCriticalSection(&_ActiveSessionsLock);
}

VOID CSessionServ::Start()
{
    PWTS_SESSION_INFOW  pSessionInfo = NULL;
    DWORD dwCount = 0;
    
    InitializeLoggedOnUsersMap();

    if(!WTSEnumerateSessionsW(NULL, 0, 1, &pSessionInfo, &dwCount)) {
        return;
    }
    if(0 == dwCount) {
        if(NULL !=  pSessionInfo) WTSFreeMemory(pSessionInfo);
        return;
    }

    
    for(int i=0; i<(int)dwCount; i++) {
        if(pSessionInfo[i].State == WTSActive) {
            try {
                AddSession(pSessionInfo[i].SessionId);
            }
            catch(const nudf::CException& e) {
                UNREFERENCED_PARAMETER(e);
            }
        }
    }
}

VOID CSessionServ::Stop()
{
    std::shared_ptr<CActiveSession> spSession;

    ::EnterCriticalSection(&_ActiveSessionsLock);
    for(std::list<std::shared_ptr<CActiveSession>>::iterator it=_ActiveSessions.begin(); it!=_ActiveSessions.end(); ++it) {
        (*it)->Stop();
    }
    _ActiveSessions.clear();
    ::LeaveCriticalSection(&_ActiveSessionsLock);
}

VOID CSessionServ::AddLoggedOnUser(_In_ const std::wstring& sid, _In_ const std::wstring& name, _In_opt_ BOOL* added) throw()
{
    BOOL bToAdd = TRUE;
    std::wstring wsSid = sid;
    std::wstring wsName = name;

    std::transform(wsSid.begin(), wsSid.end(), wsSid.begin(), toupper);
    std::transform(wsName.begin(), wsName.end(), wsName.begin(), tolower);

    ::EnterCriticalSection(&_LoggedOnUsersLock);
    for(std::map<std::wstring, std::wstring>::const_iterator it=_LoggedOnUsers.begin(); it!= _LoggedOnUsers.end(); ++it) {
        if(0 == _wcsicmp(sid.c_str(), (*it).first.c_str())) {
            bToAdd = FALSE;
            break;
        }
    }
    if(NULL != added) {
        *added = bToAdd;
    }
    if(bToAdd) {
        _LoggedOnUsers[wsSid] = wsName;
    }
    ::LeaveCriticalSection(&_LoggedOnUsersLock);

    // Print:  It is the first time for this user to logon this machine
    if(bToAdd && NULL!=added) {
        LOGDBG(L"User 1st time logon this machine:");
        LOGDBG(L"    - %s, %s", wsName.c_str(), wsSid.c_str());
    }
}

VOID CSessionServ::GetLoggedOnUsers(_Out_ std::vector<std::pair<std::wstring, std::wstring>>& users) throw()
{
    ::EnterCriticalSection(&_LoggedOnUsersLock);
    for(std::map<std::wstring, std::wstring>::const_iterator it=_LoggedOnUsers.begin(); it!= _LoggedOnUsers.end(); ++it) {
        users.push_back(std::pair<std::wstring, std::wstring>((*it).first, (*it).second));
    }
    ::LeaveCriticalSection(&_LoggedOnUsersLock);
}

BOOL CSessionServ::ExistingUser(_In_ const std::wstring& sid) throw()
{
    BOOL bExist = FALSE;
    ::EnterCriticalSection(&_LoggedOnUsersLock);
    for(std::map<std::wstring, std::wstring>::const_iterator it=_LoggedOnUsers.begin(); it!= _LoggedOnUsers.end(); ++it) {
        if(0 == _wcsicmp(sid.c_str(), (*it).first.c_str())) {
            bExist = TRUE;
            break;
        }
    }
    ::LeaveCriticalSection(&_LoggedOnUsersLock);
    return bExist;
}

VOID CSessionServ::InitializeLoggedOnUsersMap() throw()
{
    static const std::wstring wsValidSidPrefix(L"S-1-5-21-");
    WCHAR wzKeyName[MAX_PATH] = {0};
    int i = 0;

    // HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProfileList
    nudf::win::CRegKey regprofile;
    if(!regprofile.Open(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList", KEY_READ)) {
        return;
    }

    //
    //  Initialize Map of Users who have logged on this machine
    //
    LOGDBG(L"List of Users who have logged on this machine:");
    while(ERROR_SUCCESS == RegEnumKeyW(regprofile, i++, wzKeyName, MAX_PATH)) {

        if(0 == _wcsnicmp(wzKeyName, wsValidSidPrefix.c_str(), wsValidSidPrefix.length())) {
            
            PSID pSid = NULL;

            if(!ConvertStringSidToSidW(wzKeyName, &pSid)) {
                continue;
            }

            nudf::win::CUser user;
            try {
                user.GetInfoBySid(pSid);
            }
            catch(const nudf::CException& e) {
                UNREFERENCED_PARAMETER(e);
                user.Clear();
            }
            LocalFree(pSid);
            pSid = NULL;

            if(user.IsValid()) {
                AddLoggedOnUser(wzKeyName, user.GetPrincipleName().empty()?user.GetName():user.GetPrincipleName(), NULL);
                LOGDBG(L"    - %s, %s", user.GetPrincipleName().empty()?user.GetName().c_str():user.GetPrincipleName().c_str(), wzKeyName);
            }
        }
        wzKeyName[0] = L'\0';
    }
}
