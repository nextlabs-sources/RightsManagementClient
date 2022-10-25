
#ifndef __NXRM_ENGINE_SESSIONS_H__
#define __NXRM_ENGINE_SESSIONS_H__

#include <nudf\session.hpp>
#include <nudf\pipe.hpp>
#include <nudf\knowndir.hpp>
#include <string>
#include <list>
#include <memory>

#include "confmgr.hpp"

namespace nxrm {
namespace engine {


class CSessionKnownDirs : public nudf::win::CKnownDirs
{
public:
    CSessionKnownDirs();
    virtual ~CSessionKnownDirs();

    virtual void Load(_In_ HANDLE hToken) throw();
    virtual void Clear();

    inline const std::wstring& GetNxLocalAppDataDir() const throw() {return _dirNxLocalAppData;}
    inline const std::wstring& GetNxLocalConfDir() const throw() {return _dirNxLocalConf;}
    inline const std::wstring& GetNxLocalCacheDir() const throw() {return _dirNxLocalCache;}
    
private:
    std::wstring    _dirNxLocalAppData;     // %LocalAppData%\NextLabs
    std::wstring    _dirNxLocalConf;        // %LocalAppData%\NextLabs\conf
    std::wstring    _dirNxLocalCache;       // %LocalAppData%\NextLabs\cache
};

class CActiveSession : public nudf::win::CSession
{
public:
    CActiveSession();
    CActiveSession(_In_ ULONG dwSessionId);
    CActiveSession(_In_ ULONG dwSessionId, _In_ LPCWSTR wzPath, _In_opt_ LPCWSTR wzArgs);
    virtual ~CActiveSession();

    VOID Start();
    VOID Stop() throw();

    VOID ClientGuard() throw();
    VOID StartClientApp();

    inline VOID SetClientPath(_In_ LPCWSTR wzPath) throw() {_ClientPath = wzPath;}
    inline VOID SetClientArgs(_In_ LPCWSTR wzArgs) throw() {_ClientArgs = wzArgs;}
    inline const std::wstring& GetClientPort() const throw() {return _ClientPort;}
    inline const CSessionKnownDirs& GetKnownDirs() const throw() {return _dirs;}
    inline const std::wstring& GetClassifyXmlFile() const throw() {return _clsConf.GetTemp();}
    inline const nudf::win::CUser& GetCurrentUser() const throw() {return _User;}


protected:
    void KillExistingClient();

private:
    nudf::win::CUser    _User;
    CSessionKnownDirs   _dirs;
    std::wstring        _ClientPath;
    std::wstring        _ClientArgs;
    std::wstring        _ClientPort;
    BOOL                _ShuttingDown;
    HANDLE              _hClientProcess;
    HANDLE              _hClientGuardThread;
    CClsConfFile        _clsConf;
};

class CClientConn : public nudf::ipc::CPipeClient
{
public:
    CClientConn();
    CClientConn(_In_ LPCWSTR wzClientPort);
    virtual ~CClientConn();

    VOID Quit() throw();
    VOID Notify(_In_ const std::wstring& title, _In_ const std::wstring& info) throw();
    VOID SetServerStatus(_In_ const std::wstring& server, _In_ BOOL connected) throw();
    VOID SetTimeStamp(_In_ const FILETIME* ftPolicy, _In_ const FILETIME* ftUpdate) throw();
};


class CSessionServ
{
public:
    CSessionServ();
    virtual ~CSessionServ();

    VOID Start();
    VOID Stop();

    BOOL Exists(_In_ ULONG dwSessionId) throw();
    std::shared_ptr<CActiveSession> FindSession(_In_ ULONG dwSessionId) throw();
    VOID AddSession(_In_ ULONG dwSessionId);
    VOID RemoveSession(_In_ ULONG dwSessionId);

    VOID NotifyClient(_In_ ULONG dwSessionId, _In_ LPCWSTR wzTitle, _In_ LPCWSTR wzMsg);
    VOID NotifyClientTimeStamp(_In_ const FILETIME* policyTime, _In_ const FILETIME* updateTime);
    VOID NotifyClientServerStatus(_In_ const std::wstring& server, _In_ BOOL connected);

    VOID AddLoggedOnUser(_In_ const std::wstring& sid, _In_ const std::wstring& name, _In_opt_ BOOL* added) throw();
    BOOL ExistingUser(_In_ const std::wstring& sid) throw();
    VOID GetLoggedOnUsers(_Out_ std::vector<std::pair<std::wstring, std::wstring>>& users) throw();

private:
    VOID InitializeLoggedOnUsersMap() throw();


private:    
    std::list<std::shared_ptr<CActiveSession>>  _ActiveSessions;
    CRITICAL_SECTION                            _ActiveSessionsLock;
    std::map<std::wstring, std::wstring>        _LoggedOnUsers;
    CRITICAL_SECTION                            _LoggedOnUsersLock;
};
    
}   // namespace engine
}   // namespace nxrm


#endif  // #ifndef __NXRM_ENGINE_SESSIONS_H__