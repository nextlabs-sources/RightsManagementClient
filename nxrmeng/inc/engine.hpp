
#ifndef __NXRM_ENGINE_HPP__
#define __NXRM_ENGINE_HPP__

#include <nudf\shared\logdef.h>
#include <nudf\shared\engineenv.h>
#include <nudf\log.hpp>
#include "ipcserv.hpp"
#include "sessionserv.hpp"
#include "policyserv.hpp"
#include "logserv.hpp"
#include "drvserv.hpp"
#include "fltserv.hpp"
#include "confmgr.hpp"
#include "rmsutil.hpp"
#include "timerserv.hpp"
#include "audit.hpp"

#include "profile_bundle.hpp"



extern BOOL WINAPI RmEngineLogAccept(_In_ ULONG Level);
extern LONG WINAPI RmEngineLog(_In_ LPCWSTR Info);
extern LONG WINAPI RmNotify(_In_ ULONG dwSessionId, _In_ LPCWSTR wzTitle, _In_ LPCWSTR wzNotification);

namespace nxrm {
namespace engine {

typedef enum ENGINE_STATUS {
    EngineStopped = 0,
    EnginePaused,
    EngineRunning,
    EngineStartPending,
    EngineStopPending,
    EnginePausePending,
    EngineContinuePending
} ENGINE_STATUS;

extern CLogServ _LogServ;

class CEngine
{
public:
    CEngine();
    virtual ~CEngine();

    virtual VOID Initialize(_In_ LPCWSTR wzWorkingRoot);
    virtual VOID Cleanup();

    virtual VOID Start();
    virtual VOID Stop();
    virtual VOID Pause();
    virtual VOID Continue();

    VOID Audit(const CAuditItem& ai) throw();
    VOID DumpAudit() throw();
    VOID UploadAudit() throw();

    inline ULONG GetEngineState() const throw() {return _Status;}
    inline BOOL IsConnectedToServer() const throw() {return _Connected;}
    inline BOOL IsRunning() const throw() {return (_Status == EngineRunning);}
    inline BOOL IsStopped() const throw() {return (_Status == EngineStopped);}
    inline BOOL IsPaused() const throw() {return (_Status == EnginePaused);}
    inline BOOL IsStartPending() const throw() {return (_Status == EngineStartPending);}
    inline BOOL IsStopPending() const throw() {return (_Status == EngineStopPending);}
    inline BOOL IsPausePending() const throw() {return (_Status == EnginePausePending);}
    inline BOOL IsContinuePending() const throw() {return (_Status == EngineContinuePending);}
    inline BOOL IsPending() const throw() {return ( _Status == EngineStartPending ||
                                                    _Status == EngineStopPending ||
                                                    _Status == EnginePausePending ||
                                                    _Status == EngineContinuePending );}

    inline const nudf::crypto::CAesKeyBlob& GetSessionKey() const throw() {return _sessionKey;}
    inline const std::wstring GetSessionGuid() const throw() {return _sessionGuid;}
    inline CIpcServ* GetIpcServ() throw() {return &_IpcServ;}
    inline CSessionServ* GetSessionServ() throw() {return &_SessionServ;}
    inline CPolicyServ* GetPolicyServ() throw() {return &_PolicyServ;}
    inline CDrvServ* GetDrvServ() throw() {return &_DrvServ;}
    inline CFltServ* GetFltServ() throw() {return &_FltServ;}
    inline nudf::util::log::CFileLog* GetLogServ() throw() {return &_LogServ;}
    inline const FILETIME& GetLastHeartBeatTime() throw() {return _lastHeartBeatTime;}

    inline CRmsAgentProfile& GetRmsAgentProfile() throw() {return _agentProfile;}
    inline CRmsAgentClassify& GetRmsAgentClassifyProfile() throw() {return _classifyProfile;}

    inline HMODULE GetResModule() throw() {return _ResDll;}
    
    BOOL DecryptPolicyBundle(const std::wstring& file, bool orig=false);

    // Session Events
    VOID OnSessionLogon(_In_ ULONG dwSessionId);
    VOID OnSessionLogoff(_In_ ULONG dwSessionId);
    VOID OnSessionConn(_In_ ULONG dwSessionId);
    VOID OnSessionDisconn(_In_ ULONG dwSessionId);
    VOID OnSessionRemoteConn(_In_ ULONG dwSessionId);
    VOID OnSessionRemoteDisconn(_In_ ULONG dwSessionId);

    // Timer handler
    VOID OnTimerRegister();
    VOID OnTimerHeartBeat();
    VOID OnTimerLog();
    VOID OnTimerCheckUpdate();
    VOID TriggerRegister() {_TimerRegister.Trigger();}
    VOID TriggerHeartBeat() {_TimerHeartBeat.Trigger();}

private:
    BOOL CheckFolderExistence(_In_ const std::wstring& path);
    BOOL CheckFileExistence(_In_ const std::wstring& path, _Out_opt_ PDWORD attributes=NULL);
    BOOL EnvValidate();
    VOID CleanSidCache();
    VOID Register();
    VOID HeartBeat(_Out_ BOOL* bKeyChanged, _Out_ BOOL* bPolicyChanged);
    VOID SoftwareUpdate();
    BOOL GetHeartBeatTime(_Out_ FILETIME* timestamp);
    BOOL NeedHeartBeat();
    BOOL SetHeartBeatTime(_In_opt_ const FILETIME* timestamp);

    BOOL InterStartEngine();
    VOID InterStopEngine();

    VOID InitializeSSPI();

    // 
    void ReadServerXml(_Out_ std::wstring server, _Out_ PUSHORT port);

private:
    BOOL                        _Active;
    ENGINE_STATUS               _Status;
    BOOL                        _Connected;
    nudf::crypto::CAesKeyBlob   _sessionKey;
    std::wstring                _sessionGuid;
    CRITICAL_SECTION            _LogOnLock;

    CRmsAgentProfile            _agentProfile;
    CRmsAgentClassify           _classifyProfile;
    CKeyBundle                  _keyBundle;
    FILETIME                    _lastHeartBeatTime;

    // Build-in Server
    CIpcServ                    _IpcServ;
    CSessionServ                _SessionServ;
    CPolicyServ                 _PolicyServ;
    CDrvServ                    _DrvServ;
    CFltServ                    _FltServ;
    CAuditObject                _AuditObject;
    // Resource DLL
    HMODULE                     _ResDll;
    // Timers
    CRegisterTimer              _TimerRegister;
    CHeartBeatTimer             _TimerHeartBeat;
    CLogTimer                   _TimerLog;
    CCheckUpdateTimer           _TimerCheckUpdate;
};

extern nxrm::engine::CEngine* gpEngine;

    
}   // namespace engine
}   // namespace nxrm


#endif  // #ifndef __NXRM_ENGINE_EVENTS_H__