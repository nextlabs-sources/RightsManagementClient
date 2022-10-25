
#include <Windows.h>
#include <assert.h>

#include <nudf\exception.hpp>
#include <nudf\shared\rightsdef.h>
#include <nudf\path.hpp>
#include <nudf\string.hpp>
#include <nudf\resutil.hpp>
#include <nudf\listentry.hpp>
#include <nudf\host.hpp>
#include <nudf\shared\enginectl.h>

#include <nudf\nxrmres.h>

#include "nxrmflt.h"
#include "nxrmfltman.h"

#include "engine.hpp"
#include "audit.hpp"
#include "propname.h"
#include "logserv.hpp"
#include "fltserv.hpp"


using namespace nxrm::engine;


typedef HANDLE (WINAPI* NXRMFLT_CREATE_MANAGER)(NXRMFLT_CALLBACK_NOTIFY, LOGAPI_LOG, LOGAPI_ACCEPT, const NXRM_KEY_BLOB*, ULONG, PVOID);
typedef ULONG (WINAPI* NXRMFLT_REPLY_MESSAGE)(HANDLE, PVOID, NXRMFLT_CHECK_RIGHTS_REPLY*);
typedef ULONG (WINAPI* NXRMFLT_START_FILTERING)(HANDLE);
typedef ULONG (WINAPI* NXRMFLT_STOP_FILTERING)(HANDLE);
typedef ULONG (WINAPI* NXRMFLT_CLOSE_MANAGER)(HANDLE);
typedef ULONG (WINAPI* NXRMFLT_SET_SAVEAS_FORECAST)(HANDLE,ULONG,const WCHAR*,const WCHAR*);
typedef ULONG (WINAPI* NXRMFLT_SET_POLICY_CHANGED)(HANDLE);
typedef ULONG (WINAPI* NXRMFLT_UPDATE_KEYCHAIN)(HANDLE, const NXRM_KEY_BLOB*, ULONG);

static ULONG WINAPI NotificationCallback(ULONG Type, PVOID Msg, ULONG Length, PVOID MsgCtx, PVOID UserCtx);

class CFltMan
{
public:
    CFltMan();
    ~CFltMan();

    VOID Load();
    VOID Unload();
    BOOL IsValid() const throw();

    HANDLE CreateManager(NXRMFLT_CALLBACK_NOTIFY CbNotify, LOGAPI_LOG CbDbgDump, LOGAPI_ACCEPT CbDbgDumpCheckLevel, const NXRM_KEY_BLOB* KeyBlob, ULONG KeyBlobSize, PVOID Context);
    ULONG ReplyMessage(HANDLE MgrHandle, PVOID Context, NXRMFLT_CHECK_RIGHTS_REPLY* Reply);
    ULONG StartFiltering(HANDLE MgrHandle);
    ULONG StopFiltering(HANDLE MgrHandle);
    ULONG CloseManager(HANDLE MgrHandle);
    ULONG SetSaveAsForecast(HANDLE MgrHandle, ULONG ProcessId, _In_opt_ const WCHAR* SrcFileName, _In_ const WCHAR* SaveAsFileName);
    ULONG SetPolicyChanged(HANDLE MgrHandle);
    ULONG UpdateKeyChain(HANDLE MgrHandle, const NXRM_KEY_BLOB* KeyBlob, ULONG KeyBlobSize);


private:
    HMODULE hMod;
    NXRMFLT_CREATE_MANAGER  FnCreateManager;
    NXRMFLT_REPLY_MESSAGE FnReplyMessage;
    NXRMFLT_START_FILTERING FnStartFiltering;
    NXRMFLT_STOP_FILTERING FnStopFiltering;
    NXRMFLT_CLOSE_MANAGER FnCloseManager;
    NXRMFLT_SET_SAVEAS_FORECAST FnSetSaveAsForecast;
    NXRMFLT_SET_POLICY_CHANGED FnSetPolicyChanged;
    NXRMFLT_UPDATE_KEYCHAIN FnUpdateKeyChain;
};


CFltMan fltMan;


//
//  class CFltServ
//
CFltServ::CFltServ() : _fltman(NULL)
{
    nudf::util::InitializeListHead(&_list);
    ::InitializeCriticalSection(&_listlock);
}

CFltServ::~CFltServ()
{
    ::DeleteCriticalSection(&_listlock);
}

void CFltServ::Start(const NXRM_KEY_BLOB* KeyBlob, ULONG KeyBlobSize)
{
    DWORD dwResult = 0;
    BOOL bThreadStarted = FALSE;

    _fltman = fltMan.CreateManager(NotificationCallback, RmEngineLog, RmEngineLogAccept, KeyBlob, KeyBlobSize, this);
    if(NULL == _fltman) {
        throw WIN32ERROR();
    }

    try {

        // Start worker threads
        nudf::util::thread::CThreadExPool<CFltWorkerThread>::Start(4, this);
        bThreadStarted = TRUE;
        
        // enable nxrmdrv manager
        dwResult = fltMan.StartFiltering(_fltman);
        if(0 != dwResult) {
            throw WIN32ERROR2(dwResult);
        }
    }
    catch(const nudf::CException& e) {
        Stop();
        throw e;
    }
}

void CFltServ::Stop() throw()
{
    if(_fltman != NULL) {
        fltMan.CloseManager(_fltman);
        _fltman = NULL;
    }

    nudf::util::thread::CThreadExPool<CFltWorkerThread>::Stop();
    CleanRequestQueue();
}

ULONG CFltServ::SetSaveAsForecast(ULONG ProcessId, _In_opt_ const WCHAR* SrcFileName, _In_ const WCHAR* SaveAsFileName)
{
    return fltMan.SetSaveAsForecast(_fltman, ProcessId, SrcFileName, SaveAsFileName);
}

ULONG CFltServ::SetPolicyChanged()
{
    return fltMan.SetPolicyChanged(_fltman);
}

ULONG CFltServ::UpdateKeyChain(const NXRM_KEY_BLOB* KeyBlob, ULONG KeyBlobSize)
{
    return fltMan.UpdateKeyChain(_fltman, KeyBlob, KeyBlobSize);
}

PFLTNOTIFYITEM CFltServ::PopRequest()
{
    PFLTNOTIFYITEM NotifyItem = NULL;

    ::EnterCriticalSection(&_listlock);
    if(!nudf::util::IsListEmpty(&_list)) {
        PLIST_ENTRY Entry = nudf::util::RemoveHeadList(&_list);
        if(NULL != Entry) {
            NotifyItem = CONTAINING_RECORD(Entry, FLTNOTIFYITEM, Link);
        }
    }
    ::LeaveCriticalSection(&_listlock);

    return NotifyItem;
}

VOID CFltServ::PushRequest(PFLTNOTIFYITEM NotifyItem)
{
    ::EnterCriticalSection(&_listlock);
    nudf::util::InsertTailList(&_list, &NotifyItem->Link);
    ::LeaveCriticalSection(&_listlock);
}

VOID CFltServ::CleanRequestQueue()
{
    ::EnterCriticalSection(&_listlock);
    while(!nudf::util::IsListEmpty(&_list)) {
        PLIST_ENTRY Entry = nudf::util::RemoveHeadList(&_list);
        PFLTNOTIFYITEM NotifyItem = CONTAINING_RECORD(Entry, FLTNOTIFYITEM, Link);
        if (NULL != NotifyItem->Notification) {
            free(NotifyItem->Notification); NotifyItem->Notification = NULL;
        }
        free(NotifyItem);
    }
    ::LeaveCriticalSection(&_listlock);
}

ULONG CFltServ::MakeCheckRightsRequest(_In_ PVOID Msg, _In_ ULONG Length, _In_ PVOID MsgCtx)
{
	NXRM_CHECK_RIGHTS_NOTIFICATION *pNotification = (NXRM_CHECK_RIGHTS_NOTIFICATION*)Msg;
    PFLTNOTIFYITEM NotifyItem = NULL;

    if (Length < sizeof(NXRM_CHECK_RIGHTS_NOTIFICATION)) {
        return ERROR_INVALID_PARAMETER;
    }

    NotifyItem = (PFLTNOTIFYITEM)malloc(sizeof(FLTNOTIFYITEM));
    if (!NotifyItem) {
        return ERROR_OUTOFMEMORY;
    }
    memset(NotifyItem, 0, sizeof(FLTNOTIFYITEM));

    NotifyItem->Type = NXRMFLT_MSG_TYPE_CHECK_RIGHTS;
    NotifyItem->Notification = malloc(Length);
    if (!NotifyItem->Notification) {
        free(NotifyItem); NotifyItem = NULL;
        return ERROR_OUTOFMEMORY;
    }
    memset(NotifyItem->Notification, 0, Length);

    memcpy(NotifyItem->Notification, pNotification, sizeof(NXRM_CHECK_RIGHTS_NOTIFICATION));
    NotifyItem->Context = MsgCtx;
    PushRequest(NotifyItem);
    SetJobEvent();
    return 0;
}

ULONG CFltServ::MakeBlockNotificationRequest(_In_ PVOID Msg, _In_ ULONG Length, _In_ PVOID MsgCtx)
{
	NXRM_BLOCK_NOTIFICATION *pNotification = (NXRM_BLOCK_NOTIFICATION*)Msg;
    PFLTNOTIFYITEM NotifyItem = NULL;

    if (Length < sizeof(NXRM_BLOCK_NOTIFICATION)) {
        return ERROR_INVALID_PARAMETER;
    }

    NotifyItem = (PFLTNOTIFYITEM)malloc(sizeof(FLTNOTIFYITEM));
    if (!NotifyItem) {
        return ERROR_OUTOFMEMORY;
    }
    memset(NotifyItem, 0, sizeof(FLTNOTIFYITEM));

    NotifyItem->Type = NXRMFLT_MSG_TYPE_BLOCK_NOTIFICATION;
    NotifyItem->Notification = malloc(Length);
    if (!NotifyItem->Notification) {
        free(NotifyItem); NotifyItem = NULL;
        return ERROR_OUTOFMEMORY;
    }
    memset(NotifyItem->Notification, 0, Length);

    memcpy(NotifyItem->Notification, pNotification, sizeof(NXRM_BLOCK_NOTIFICATION));
    NotifyItem->Context = MsgCtx;
    PushRequest(NotifyItem);
    SetJobEvent();
    return 0;
}

ULONG CFltServ::MakeKeyErrorNotificationRequest(_In_ PVOID Msg, _In_ ULONG Length, _In_ PVOID MsgCtx)
{
	NXRM_KEYCHAIN_ERROR_NOTIFICATION *pNotification = (NXRM_KEYCHAIN_ERROR_NOTIFICATION*)Msg;
    PFLTNOTIFYITEM NotifyItem = NULL;

    if (Length < sizeof(NXRM_KEYCHAIN_ERROR_NOTIFICATION)) {
        return ERROR_INVALID_PARAMETER;
    }

    NotifyItem = (PFLTNOTIFYITEM)malloc(sizeof(FLTNOTIFYITEM));
    if (!NotifyItem) {
        return ERROR_OUTOFMEMORY;
    }
    memset(NotifyItem, 0, sizeof(FLTNOTIFYITEM));

    NotifyItem->Type = NXRMFLT_MSG_TYPE_KEYCHAIN_ERROR_NOTIFICATION;
    NotifyItem->Notification = malloc(Length);
    if (!NotifyItem->Notification) {
        free(NotifyItem); NotifyItem = NULL;
        return ERROR_OUTOFMEMORY;
    }
    memset(NotifyItem->Notification, 0, Length);

    memcpy(NotifyItem->Notification, pNotification, sizeof(NXRM_KEYCHAIN_ERROR_NOTIFICATION));
    NotifyItem->Context = MsgCtx;
    PushRequest(NotifyItem);
    SetJobEvent();
    return 0;
}

ULONG CFltServ::MakePurgeCacheRequest(_In_ PVOID Msg, _In_ ULONG Length, _In_ PVOID MsgCtx)
{
	PNXRM_PURGE_CACHE_NOTIFICATION pNotification = (PNXRM_PURGE_CACHE_NOTIFICATION)Msg;
    PFLTNOTIFYITEM NotifyItem = NULL;

    if (Length < sizeof(NXRM_PURGE_CACHE_NOTIFICATION)) {
        return ERROR_INVALID_PARAMETER;
    }

    NotifyItem = (PFLTNOTIFYITEM)malloc(sizeof(FLTNOTIFYITEM));
    if (!NotifyItem) {
        return ERROR_OUTOFMEMORY;
    }
    memset(NotifyItem, 0, sizeof(FLTNOTIFYITEM));

    NotifyItem->Type = NXRMFLT_MSG_TYPE_PURGE_CACHE_NOTIFICATION;
    NotifyItem->Notification = malloc(Length);
    if (!NotifyItem->Notification) {
        free(NotifyItem); NotifyItem = NULL;
        return ERROR_OUTOFMEMORY;
    }
    memset(NotifyItem->Notification, 0, Length);

    memcpy(NotifyItem->Notification, pNotification, sizeof(NXRM_PURGE_CACHE_NOTIFICATION));
    NotifyItem->Context = MsgCtx;
    PushRequest(NotifyItem);
    SetJobEvent();
    return 0;
}

VOID CFltServ::OnCheckRights(_In_ const NXRM_CHECK_RIGHTS_NOTIFICATION* request, _In_ PVOID context)
{
    nxrm::pkg::CEvalObject      object;
    nxrm::pkg::CEvalResult      result;
    NXRMFLT_CHECK_RIGHTS_REPLY  reply = {0};
    std::wstring                wsFile  = L"(";
    ULONG                       dwSessionId = -1;
    bool                        bIsExplorer = false;
    std::wstring                wsUserContext;
    
    static nudf::win::CHost host;
    
	reply.RightsMask = 0;
	reply.CustomRights = 0;
    reply.EvaluationId = result.GetUid();
    
    const WCHAR* pwzFileName = wcsrchr(request->FileName, L'\\');
    if(NULL == pwzFileName) pwzFileName = request->FileName;
    else pwzFileName++;
    wsFile += pwzFileName;
    wsFile += L")";

    try {
        
        CDrvProcessInfo         procinfo;

        //
        //  Unknown Session ?
        //
        if(!ProcessIdToSessionId(request->ProcessId, &dwSessionId) || ((ULONG)-1) == dwSessionId) {
            throw WIN32ERROR2(ERROR_NOT_FOUND);
        }

        //
        //  Get Application Information
        //
        if(!gpEngine->GetDrvServ()->QueryProcessInfo(request->ProcessId, procinfo)) {
            throw WIN32ERROR2(ERROR_NOT_FOUND);
        }
        object.InsertProperty(nxrm::pkg::policy::RULE_APPLICATION, APP_PROP_IMAGE_PATH, nxrm::pkg::policy::CProperty(APP_PROP_IMAGE_PATH, procinfo.GetImage().c_str()));
        if(!procinfo.GetPublisher().empty()) {
            object.InsertProperty(nxrm::pkg::policy::RULE_APPLICATION, APP_PROP_PUBLISHER, nxrm::pkg::policy::CProperty(APP_PROP_PUBLISHER, procinfo.GetPublisher().c_str()));
        }

        //
        //  Get Host Information
        //
        object.InsertProperty(nxrm::pkg::policy::RULE_LOCATION, LOC_PROP_HOST, nxrm::pkg::policy::CProperty(LOC_PROP_HOST,  host.GetHostName().c_str()));

        //
        //  Get Resource Information
        //
        object.InsertProperty(nxrm::pkg::policy::RULE_RESOURCE, RES_PROP_PATH, nxrm::pkg::policy::CProperty(RES_PROP_PATH,  request->FileName));

        //
        //  Set Environmental Information
        //
        FILETIME current_time = {0, 0};
        GetSystemTimeAsFileTime(&current_time);
        object.InsertProperty(nxrm::pkg::policy::RULE_ENV, ENV_PROP_DATETIME, nxrm::pkg::policy::CProperty(ENV_PROP_DATETIME, current_time));

        nudf::time::CTime cur_t(&current_time);
        nudf::time::CTime lhb_t(&gpEngine->GetLastHeartBeatTime());
        time_t seconds_cur = cur_t.ToSecondsSince1970Jan1st();
        time_t seconds_lhb = lhb_t.ToSecondsSince1970Jan1st();
        int    tm_diff = 0;
        if(seconds_cur > seconds_lhb) {
            tm_diff = (int)(seconds_cur - seconds_lhb);
        }
        object.InsertProperty(nxrm::pkg::policy::RULE_ENV, ENV_PROP_TSLH, nxrm::pkg::policy::CProperty(ENV_PROP_TSLH, tm_diff));

        //
        //  Check White List
        //
        const WCHAR* pwzAppName = wcsrchr(procinfo.GetImage().c_str(), L'\\');
        if(NULL == pwzAppName) {
            pwzAppName = procinfo.GetImage().c_str();
        }
        else {
            ++pwzAppName;
        }
        if(NULL != pwzAppName) {
            if(0 == _wcsicmp(pwzAppName, L"explorer.exe")) {
                bIsExplorer = true;
            }
            else {
                reply.RightsMask |= gpEngine->GetRmsAgentClassifyProfile().CheckAppInWhiteList(pwzAppName, procinfo.GetPublisher());
                if(0 != reply.RightsMask) {
                    // Found it in white list, add a policy
                    nxrm::pkg::policy::CPolicy policy(-1, L"BUILTIN-APP-WHITELIST", -1, reply.RightsMask);
                    result.AddPolicy(&policy);
                }
            }
        }

        if(0 != dwSessionId) {

            std::shared_ptr<CActiveSession> pSession = gpEngine->GetSessionServ()->FindSession(dwSessionId);
            int nIsRemote = 0;
            if(NULL != pSession) {
                nIsRemote = pSession->IsRdp() ? 1 : 0;
            }
            object.InsertProperty(nxrm::pkg::policy::RULE_ENV, ENV_PROP_REMOTEACCESS, nxrm::pkg::policy::CProperty(ENV_PROP_REMOTEACCESS, nIsRemote));

            // User Information
            const nudf::win::CUser& user = pSession->GetCurrentUser();
            object.InsertProperty(nxrm::pkg::policy::RULE_USER, USER_PROP_NAME, nxrm::pkg::policy::CProperty(USER_PROP_NAME, user.GetPrincipleName().empty()?user.GetName().c_str():user.GetPrincipleName().c_str()));
            object.InsertProperty(nxrm::pkg::policy::RULE_USER, USER_PROP_ID, nxrm::pkg::policy::CProperty(USER_PROP_ID, user.GetSid().GetSidStr().c_str()));

            // Get Context
            const nxrm::pkg::policy::CUserGroups* pGroup = gpEngine->GetPolicyServ()->GetPolicyProfile().GetUserGroup(user.GetSid().GetSidStr());
            if(NULL == pGroup) {
                pGroup = gpEngine->GetPolicyServ()->GetPolicyProfile().GetUnknownUserGroup();
                if(NULL != pGroup) {
                    wsUserContext = pGroup->GetContext();
                }
            }
            else {
                wsUserContext = pGroup->GetContext();
            }
            
            // Add User Context
            object.InsertProperty(nxrm::pkg::policy::RULE_USER, USER_PROP_CONTEXT, nxrm::pkg::policy::CProperty(USER_PROP_CONTEXT, wsUserContext.c_str()));

            // Evaluate
            gpEngine->GetPolicyServ()->Authorize(dwSessionId, object, result);
            reply.RightsMask |= result.GetRights();
        }
        else {
            const nxrm::pkg::policy::CUserGroups* pGroup = gpEngine->GetPolicyServ()->GetPolicyProfile().GetUnknownUserGroup();
            if(NULL != pGroup) {
                wsUserContext = pGroup->GetContext();
            }
            object.InsertProperty(nxrm::pkg::policy::RULE_ENV, ENV_PROP_REMOTEACCESS, nxrm::pkg::policy::CProperty(ENV_PROP_REMOTEACCESS, 0));
            object.InsertProperty(nxrm::pkg::policy::RULE_USER, USER_PROP_NAME, nxrm::pkg::policy::CProperty(USER_PROP_NAME, L"SYSTEM"));
            object.InsertProperty(nxrm::pkg::policy::RULE_USER, USER_PROP_ID, nxrm::pkg::policy::CProperty(USER_PROP_ID, L"S-1-5-18")); // Local System
            object.InsertProperty(nxrm::pkg::policy::RULE_USER, USER_PROP_CONTEXT, nxrm::pkg::policy::CProperty(USER_PROP_CONTEXT, wsUserContext.c_str()));
        }
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
    }

    //
    //  Add Audit Log
    //
    if(!bIsExplorer && 0 != reply.RightsMask) {
        
        CAuditItem      ai;
        nxrm::pkg::policy::PROPERTYMAP::const_iterator it;

        static int nAgentId = -1;
        if(-1 == nAgentId) {
            nudf::string::ToInt<wchar_t>(gpEngine->GetRmsAgentProfile().GetAgentId(), &nAgentId);
        }
        
        ai.Initialize(nAgentId);
        ai.SetType(L"Evaluation");
        ai.SetOperation(L"OPEN");
        ai.SetRights(reply.RightsMask);

        // Set Env Info
        for(it=object.GetProperty(nxrm::pkg::policy::RULE_ENV).begin(); it!=object.GetProperty(nxrm::pkg::policy::RULE_ENV).end(); ++it) {
            if((*it).first == ENV_PROP_TSLH) {
                ai.AddEnvVariable(L"SecondsSinceLastHeartbeat", (*it).second.GetValue());
            }
            else if((*it).first == ENV_PROP_REMOTEACCESS) {
                ai.AddEnvVariable(L"SessionType", (*it).second.GetIntValue() ? L"Remote" : L"Console");
            }
            else {
                ai.AddEnvVariable((*it).first, (*it).second.GetValue());
            }
        }

        // Set User Info
        ai.SetAuditUserContext(wsUserContext);
        for(it=object.GetProperty(nxrm::pkg::policy::RULE_USER).begin(); it!=object.GetProperty(nxrm::pkg::policy::RULE_USER).end(); ++it) {
            if((*it).first == USER_PROP_NAME) {
                ai.SetAuditUserName((*it).second.GetValue());
            }
            else if((*it).first == USER_PROP_ID) {
                ai.SetAuditUserSid((*it).second.GetValue());
            }
            else if((*it).first == USER_PROP_TENANTID) {
                ; // Ignore TenantId
            }
            else {
                ai.AddAuditUserAttributes((*it).first, (*it).second.GetValue());
            }
        }

        // Set Host Info
        for(it=object.GetProperty(nxrm::pkg::policy::RULE_LOCATION).begin(); it!=object.GetProperty(nxrm::pkg::policy::RULE_LOCATION).end(); ++it) {
            if((*it).first == LOC_PROP_HOST) {
                ai.SetAuditHostName((*it).second.GetValue());
            }
            else {
                ; // Nothing
            }
        }

        // Set App Info
        for(it=object.GetProperty(nxrm::pkg::policy::RULE_APPLICATION).begin(); it!=object.GetProperty(nxrm::pkg::policy::RULE_APPLICATION).end(); ++it) {
            if((*it).first == APP_PROP_IMAGE_PATH) {
                ai.SetAppImage((*it).second.GetValue());
            }
            else if((*it).first == APP_PROP_PUBLISHER) {
                ai.SetAppPublisher((*it).second.GetValue());
            }
            else {
                ; // NOTHING
            }
        }

        // Set Res Info
        for(it=object.GetProperty(nxrm::pkg::policy::RULE_RESOURCE).begin(); it!=object.GetProperty(nxrm::pkg::policy::RULE_RESOURCE).end(); ++it) {
            if((*it).first == RES_PROP_PATH) {
                ai.SetResPath((*it).second.GetValue());
            }
            else {
                ai.AddResTag((*it).first, (*it).second.GetValue());
            }
        }

        // Set policies
        const std::vector<nxrm::pkg::CEvalPolicyResult>& policies = result.GetPolicies();
        std::vector<nxrm::pkg::CEvalPolicyResult>::const_iterator itp;
        for(itp = policies.begin(); itp != policies.end(); ++itp) {
            ai.AddEvalPolicy((int)(*itp).GetId(), (*itp).GetName());            
        }

        // Insert
        gpEngine->Audit(ai);
    }

    //
    //  Notify
    //
    fltMan.ReplyMessage(_fltman, context, &reply);
}

VOID CFltServ::OnBlockNotification(_In_ const NXRM_BLOCK_NOTIFICATION* request, _In_ PVOID context)
{
    DWORD dwSessionId = -1;
    std::wstring wsTitle;
    std::wstring wsInfo;
    std::wstring wsFile;
    std::wstring wsOperation;

    UNREFERENCED_PARAMETER(context);


    const WCHAR* pwzFileName = wcsrchr(request->FileName, L'\\');
    if(NULL == pwzFileName) pwzFileName = request->FileName;
    else pwzFileName++;
    wsFile = pwzFileName;


    if(!ProcessIdToSessionId(request->ProcessId, &dwSessionId) || ((ULONG)-1) == dwSessionId) {
        return;
    }

    switch(request->Reason)
    {
    case nxrmfltDeniedWritesOpen:
        wsTitle = nudf::util::res::LoadMessage(gpEngine->GetResModule(), IDS_NOTIFY_READONLY_MODE_TITLE, 256, LANG_NEUTRAL, L"Open in ReadOnly Mode");
        wsInfo = nudf::util::res::LoadMessageEx(gpEngine->GetResModule(), IDS_NOTIFY_READONLY_MODE, 1024, LANG_NEUTRAL, L"The file (%s) will be opened in read only mode because you don't have write permission", wsFile.c_str());
        break;
    case nxrmfltDeniedSaveAsOpen:
        wsTitle = nudf::util::res::LoadMessage(gpEngine->GetResModule(), IDS_NOTIFY_OPERATION_DENIED_TITLE, 256, LANG_NEUTRAL, L"Operation Denied");
        wsOperation = nudf::util::res::LoadMessage(gpEngine->GetResModule(), IDS_NOTIFY_OPERATION_OVERWRITE, 256, LANG_NEUTRAL, L"overwrite");
        wsInfo = nudf::util::res::LoadMessageEx(gpEngine->GetResModule(), IDS_NOTIFY_OPERATION_DENIED, 1024, LANG_NEUTRAL, L"You don't have permission to %s file %s", wsOperation.c_str(), wsFile.c_str());
        break;
    case nxrmfltSaveAsToUnprotectedVolume:
        wsTitle = nudf::util::res::LoadMessage(gpEngine->GetResModule(), IDS_NOTIFY_OPERATION_DENIED_TITLE, 256, LANG_NEUTRAL, L"Operation Denied");
        wsInfo = nudf::util::res::LoadMessageEx(gpEngine->GetResModule(), IDS_NOTIFY_OPERATION_SAVEAS_DENIED, 1024, LANG_NEUTRAL, L"File (%s) cannot be saved to unprotected location", wsFile.c_str());
        break;
    default:
        wsTitle = nudf::util::res::LoadMessage(gpEngine->GetResModule(), IDS_NOTIFY_OPERATION_DENIED_TITLE, 256, LANG_NEUTRAL, L"Operation Denied");
        wsOperation = nudf::util::res::LoadMessage(gpEngine->GetResModule(), IDS_NOTIFY_OPERATION_DEFAULT, 256, LANG_NEUTRAL, L"operate");
        wsInfo = nudf::util::res::LoadMessageEx(gpEngine->GetResModule(), IDS_NOTIFY_OPERATION_DENIED, 1024, LANG_NEUTRAL, L"You don't have permission to %s file %s", wsOperation.c_str(), wsFile.c_str());
        break;
    }
    
    RmNotify(dwSessionId, wsTitle.c_str(), wsInfo.c_str());
}

VOID CFltServ::OnKeyErrorNotification(_In_ const NXRM_KEYCHAIN_ERROR_NOTIFICATION* request, _In_ PVOID context)
{
    std::wstring wsTitle;
    std::wstring wsInfo;
    std::wstring wsFile;
    std::wstring wsKeyId = nudf::string::FromBytes<wchar_t>(request->KeyId.Id, request->KeyId.IdSize);

    UNREFERENCED_PARAMETER(context);
        
    const WCHAR* pwzFileName = wcsrchr(request->FileName, L'\\');
    if(NULL == pwzFileName) pwzFileName = request->FileName;
    else pwzFileName++;
    wsFile = pwzFileName;

    
    wsTitle = nudf::util::res::LoadMessage(gpEngine->GetResModule(), IDS_MSG_KEY_ERROR_TITLE, 256, LANG_NEUTRAL, L"Key Error");
    wsInfo = nudf::util::res::LoadMessageEx(gpEngine->GetResModule(), IDS_MSG_UNRECOGNIZED_KEY_ID, 1024, LANG_NEUTRAL, L"File (%s) has an unrecognized key id: %s", wsFile.c_str(), wsKeyId.c_str());    
    
    RmNotify(request->SessionId, wsTitle.c_str(), wsInfo.c_str());

    LOGWARN(ERROR_LOCAL_USER_SESSION_KEY, L"KeyError: %s", wsInfo.c_str());
}

VOID CFltServ::OnPurgeCacheNotification(_In_ const NXRM_PURGE_CACHE_NOTIFICATION* request, _In_ PVOID context)
{
    UNREFERENCED_PARAMETER(context);
    std::wstring wsFile(request->FileName);
    if(!wsFile.empty()) {
        std::transform(wsFile.begin(), wsFile.end(), wsFile.begin(), tolower);
        gpEngine->GetPolicyServ()->RemoveResource(wsFile);
    }
}

VOID CFltServ::OnReplyMessage(_In_ PFLTNOTIFYITEM NotifyItem)
{
    switch(NotifyItem->Type)
    {
    case NXRMFLT_MSG_TYPE_CHECK_RIGHTS:
        OnCheckRights((const NXRM_CHECK_RIGHTS_NOTIFICATION*)NotifyItem->Notification, NotifyItem->Context);
        break;
    case NXRMFLT_MSG_TYPE_BLOCK_NOTIFICATION:
        OnBlockNotification((const NXRM_BLOCK_NOTIFICATION*)NotifyItem->Notification, NotifyItem->Context);
        break;
    case NXRMFLT_MSG_TYPE_KEYCHAIN_ERROR_NOTIFICATION:
        OnKeyErrorNotification((const NXRM_KEYCHAIN_ERROR_NOTIFICATION*)NotifyItem->Notification, NotifyItem->Context);
        break;
    case NXRMFLT_MSG_TYPE_PURGE_CACHE_NOTIFICATION:
        OnPurgeCacheNotification((const NXRM_PURGE_CACHE_NOTIFICATION*)NotifyItem->Notification, NotifyItem->Context);
        break;
    default:
        break;
    }
}


//
//  class CDrvWorkerThread
//
CFltWorkerThread::CFltWorkerThread()
{
}

CFltWorkerThread::~CFltWorkerThread()
{
}

DWORD CFltWorkerThread::OnStart(_In_opt_ PVOID Context)
{
    return 0L;
}

DWORD CFltWorkerThread::OnRunning(_In_opt_ PVOID Context)
{
    CFltServ* serv = (CFltServ*)Context;
    PFLTNOTIFYITEM NotifyItem = NULL;

    assert(NULL != Context);

    NotifyItem = serv->PopRequest();
    if(NULL == NotifyItem) {
        // No more request, resetthe signal
        serv->ResetJobEvent();
        return 0;
    }

    serv->OnReplyMessage(NotifyItem);

    if(NULL != NotifyItem) {
        if (NULL != NotifyItem->Notification) {
            free(NotifyItem->Notification); NotifyItem->Notification = NULL;
        }
        free(NotifyItem);
        NotifyItem = NULL;
    }

    return 0;
}


ULONG WINAPI NotificationCallback(ULONG Type, PVOID Msg, ULONG Length, PVOID MsgCtx, PVOID UserCtx)
{
	NXRM_CHECK_RIGHTS_NOTIFICATION *pNotification = (NXRM_CHECK_RIGHTS_NOTIFICATION*)Msg;
    CFltServ* fltserv = (CFltServ*)UserCtx;
    PFLTNOTIFYITEM NotifyItem = NULL;

    ULONG lResult = 0;

    switch(Type)
    {
    case NXRMFLT_MSG_TYPE_CHECK_RIGHTS:
        lResult = fltserv->MakeCheckRightsRequest(Msg, Length, MsgCtx);
        break;
    case NXRMFLT_MSG_TYPE_BLOCK_NOTIFICATION:
        lResult = fltserv->MakeBlockNotificationRequest(Msg, Length, MsgCtx);
        break;
    case NXRMFLT_MSG_TYPE_KEYCHAIN_ERROR_NOTIFICATION:
        lResult = fltserv->MakeKeyErrorNotificationRequest(Msg, Length, MsgCtx);
        break;
    case NXRMFLT_MSG_TYPE_PURGE_CACHE_NOTIFICATION:
        lResult = fltserv->MakePurgeCacheRequest(Msg, Length, MsgCtx);
        break;
    default:
        lResult = ERROR_INVALID_PARAMETER;
        break;
    }

    return lResult;
}

//
//  class CFltMan
//
CFltMan::CFltMan() : hMod(NULL),
    FnCreateManager(NULL),
    FnCloseManager(NULL),
    FnReplyMessage(NULL),
    FnStartFiltering(NULL),
    FnStopFiltering(NULL),
    FnSetSaveAsForecast(NULL),
    FnSetPolicyChanged(NULL),
    FnUpdateKeyChain(NULL)
{
    Load();
}

CFltMan::~CFltMan()
{
    Unload();
}

VOID CFltMan::Load()
{
    nudf::win::CModulePath mod(NULL);
    std::wstring wsDll = mod.GetParentDir();
    if(!wsDll.empty()) {
        wsDll += L"\\";
    }
    wsDll += L"nxrmfltman.dll";

    hMod = ::LoadLibraryW(wsDll.c_str());
    if(NULL != hMod) {
        // Refer to nxrmfltman.def to get ordinal
        FnCreateManager = (NXRMFLT_CREATE_MANAGER)GetProcAddress(hMod, MAKEINTRESOURCEA(1));
        FnReplyMessage = (NXRMFLT_REPLY_MESSAGE)GetProcAddress(hMod, MAKEINTRESOURCEA(2));
        FnStopFiltering = (NXRMFLT_STOP_FILTERING)GetProcAddress(hMod, MAKEINTRESOURCEA(3));
        FnStartFiltering = (NXRMFLT_START_FILTERING)GetProcAddress(hMod, MAKEINTRESOURCEA(4));
        FnCloseManager = (NXRMFLT_CLOSE_MANAGER)GetProcAddress(hMod, MAKEINTRESOURCEA(5));
        FnSetSaveAsForecast = (NXRMFLT_SET_SAVEAS_FORECAST)GetProcAddress(hMod, MAKEINTRESOURCEA(6));
        FnSetPolicyChanged = (NXRMFLT_SET_POLICY_CHANGED)GetProcAddress(hMod, MAKEINTRESOURCEA(7));
        FnUpdateKeyChain = (NXRMFLT_UPDATE_KEYCHAIN)GetProcAddress(hMod, MAKEINTRESOURCEA(8));
    }
}

VOID CFltMan::Unload()
{
    FnCreateManager = NULL;
    FnReplyMessage = NULL;
    FnStartFiltering = NULL;
    FnStopFiltering = NULL;
    FnCloseManager = NULL;
    FnSetSaveAsForecast = NULL;
    FnSetPolicyChanged = NULL;
    FnUpdateKeyChain = NULL;
    if(NULL != hMod) {
        FreeLibrary(hMod);
    }
}

BOOL CFltMan::IsValid() const throw()
{
    return (FnCreateManager != NULL &&
            FnReplyMessage != NULL &&
            FnStartFiltering != NULL &&
            FnStopFiltering != NULL &&
            FnCloseManager != NULL &&
            FnSetSaveAsForecast != NULL &&
            FnSetPolicyChanged != NULL &&
            FnUpdateKeyChain != NULL
            );
}

HANDLE CFltMan::CreateManager(NXRMFLT_CALLBACK_NOTIFY CbNotify, LOGAPI_LOG CbDbgDump, LOGAPI_ACCEPT CbDbgDumpCheckLevel, const NXRM_KEY_BLOB* KeyBlob, ULONG KeyBlobSize, PVOID Context)
{
    if(NULL == FnCreateManager) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return NULL;
    }
    return FnCreateManager(CbNotify, CbDbgDump, CbDbgDumpCheckLevel, KeyBlob, KeyBlobSize, Context);
}

ULONG CFltMan::ReplyMessage(HANDLE MgrHandle, PVOID Context, NXRMFLT_CHECK_RIGHTS_REPLY* Reply)
{
    if(NULL == FnReplyMessage) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return ERROR_INVALID_FUNCTION;
    }
    return FnReplyMessage(MgrHandle, Context, Reply);
}

ULONG CFltMan::StartFiltering(HANDLE MgrHandle)
{
    if(NULL == FnStartFiltering) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return ERROR_INVALID_FUNCTION;
    }
    return FnStartFiltering(MgrHandle);
}

ULONG CFltMan::StopFiltering(HANDLE MgrHandle)
{
    if(NULL == FnStopFiltering) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return ERROR_INVALID_FUNCTION;
    }
    return FnStopFiltering(MgrHandle);
}

ULONG CFltMan::CloseManager(HANDLE MgrHandle)
{
    if(NULL == FnCloseManager) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return ERROR_INVALID_FUNCTION;
    }
    return FnCloseManager(MgrHandle);
}

ULONG CFltMan::SetSaveAsForecast(HANDLE MgrHandle, ULONG ProcessId, _In_opt_ const WCHAR* SrcFileName, _In_ const WCHAR* SaveAsFileName)
{
    if(NULL == FnSetSaveAsForecast) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return ERROR_INVALID_FUNCTION;
    }
    return FnSetSaveAsForecast(MgrHandle, ProcessId, SrcFileName, SaveAsFileName);
}

ULONG CFltMan::SetPolicyChanged(HANDLE MgrHandle)
{
    if(NULL == FnSetPolicyChanged) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return ERROR_INVALID_FUNCTION;
    }
    return FnSetPolicyChanged(MgrHandle);
}

ULONG CFltMan::UpdateKeyChain(HANDLE MgrHandle, const NXRM_KEY_BLOB* KeyBlob, ULONG KeyBlobSize)
{
    if(NULL == FnUpdateKeyChain) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return ERROR_INVALID_FUNCTION;
    }
    return FnUpdateKeyChain(MgrHandle, KeyBlob, KeyBlobSize);
}
