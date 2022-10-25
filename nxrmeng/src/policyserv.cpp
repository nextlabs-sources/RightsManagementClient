

#include <Windows.h>

#include <nudf\exception.hpp>
#include <nudf\session.hpp>
#include <nudf\host.hpp>
#include <nudf\nxlutil.hpp>
#include <nudf\string.hpp>
#include <nudf\shared\rightsdef.h>

#include "nxrmeng.h"
#include "engine.hpp"
#include "propname.h"
#include "confmgr.hpp"
#include "policyserv.hpp"

using namespace nxrm::engine;

CPolicyServ::CPolicyServ()
{
}

CPolicyServ::~CPolicyServ()
{
    nudf::util::CRwExclusiveLocker lock(&_bundlelock);
    _sessionbudnle.clear();
}

void CPolicyServ::Load(_In_ const nudf::crypto::CAesKeyBlob& key)
{
    try {

        CPolicyBundle bundle;

        bundle.Load(key);
        LOGDBG(L"Temp Policy bundle file is loaded");

        if(bundle.GetTimestampUll() > _profile.GetTimestampUll()) {

            nudf::util::CRwExclusiveLocker lock(&_bundlelock);
            LOGDBG(L"Enter bundle lock area");

            // Reset bundle
            _profile.Clear();
            LOGDBG(L"Policy bundle has been changed, reset it");
            _profile.FromXml(bundle.GetXml());
            LOGDBG(L"Reset finished");

            // Reload all existing session
            for(SESSIONPOLICYSET::iterator it=_sessionbudnle.begin(); it!=_sessionbudnle.end(); ++it) {
                (*it).second->Load(_profile);
                LOGDBG(L"Reload session policy bundle (session: %d)", (*it).second->GetSessionId());
            }

            LOGDBG(L"Leave bundle lock area");
        }
    }
    catch(const nudf::CException& e) {
        throw e;
    }

}

bool CPolicyServ::IsLoaded() throw()
{
    nudf::util::CRwSharedLocker lock(&_bundlelock);
    return (0 != _profile.GetTimestampUll()) ? true : false;
}

bool CPolicyServ::IsEmpty() throw()
{
    nudf::util::CRwSharedLocker lock(&_bundlelock);
    return (_profile.GetPolicySet().size() == 0) ? true : false;
}

void CPolicyServ::RightsToNames(_In_ const ULONGLONG rights, _Out_ std::wstring& names)
{
    if(BUILTIN_RIGHT_VIEW == (BUILTIN_RIGHT_VIEW & rights)) {
        if(!names.empty()) names += L",";
        names += RIGHT_DISP_VIEW;
    }
    if(BUILTIN_RIGHT_EDIT == (BUILTIN_RIGHT_EDIT & rights)) {
        if(!names.empty()) names += L",";
        names += RIGHT_DISP_EDIT;
    }
    if(BUILTIN_RIGHT_PRINT == (BUILTIN_RIGHT_PRINT & rights)) {
        if(!names.empty()) names += L",";
        names += RIGHT_DISP_PRINT;
    }
    if(BUILTIN_RIGHT_CLIPBOARD == (BUILTIN_RIGHT_CLIPBOARD & rights)) {
        if(!names.empty()) names += L",";
        names += RIGHT_DISP_CLIPBOARD;
    }
    if(BUILTIN_RIGHT_SAVEAS == (BUILTIN_RIGHT_SAVEAS & rights)) {
        if(!names.empty()) names += L",";
        names += RIGHT_DISP_SAVEAS;
    }
    if(BUILTIN_RIGHT_DECRYPT == (BUILTIN_RIGHT_DECRYPT & rights)) {
        if(!names.empty()) names += L",";
        names += RIGHT_DISP_DECRYPT;
    }
    if(BUILTIN_RIGHT_SCREENCAP == (BUILTIN_RIGHT_SCREENCAP & rights)) {
        if(!names.empty()) names += L",";
        names += RIGHT_DISP_SCREENCAP;
    }
    if(BUILTIN_RIGHT_SEND == (BUILTIN_RIGHT_SEND & rights)) {
        if(!names.empty()) names += L",";
        names += RIGHT_DISP_SEND;
    }
    if(BUILTIN_RIGHT_CLASSIFY == (BUILTIN_RIGHT_CLASSIFY & rights)) {
        if(!names.empty()) names += L",";
        names += RIGHT_DISP_CLASSIFY;
    }
}

void CPolicyServ::Authorize(_In_ ULONG session_id,
                            _In_ nxrm::pkg::CEvalObject& object,
                            _Out_ nxrm::pkg::CEvalResult& result
                            ) throw()
{    
    static nudf::win::CHost host;
    
    nudf::util::CRwSharedLocker lock(&_bundlelock);
    SESSIONPOLICYSET::iterator it = _sessionbudnle.find(session_id);
    if(it != _sessionbudnle.end()) {

        // Evaluaet
        (*it).second->Evaluate(object, result);
        
        // Insert Obligations to Cache
        if(!result.GetObligations().IsEmpty()) {
            nudf::util::CObligation obClassify;
            (*it).second->GetObsCache().Insert(result.GetUid(), result.GetObligations());
        }

        // Log Information
        if(_LogServ.AcceptLevel(LOGINFO)) {
            std::wstring wsRights;
            RightsToNames(result.GetRights(), wsRights);
            LOGINF(L"[Evaluation Request]");
            LOGINF(L"  Id: 0x%08X%08X", (ULONG)(result.GetUid() >>32), (ULONG)result.GetUid());
            LOGINF(L"  Result:");
            LOGINF(L"     Rights Granted = %s", wsRights.c_str());
        }
        if(_LogServ.AcceptLevel(LOGDEBUG)) {

            // Set user Information
            object.GetProperty(nxrm::pkg::policy::RULE_USER) = (*it).second->GetUserProperty();
            // Set host information
            object.GetProperty(nxrm::pkg::policy::RULE_LOCATION) = (*it).second->GetLocationProperty();

            LOGDBG(L"  User:");
            const nxrm::pkg::policy::PROPERTYMAP& userprops = object.GetProperty(nxrm::pkg::policy::RULE_USER);
            for(nxrm::pkg::policy::PROPERTYMAP::const_iterator it=userprops.begin(); it!=userprops.end(); ++it) {
                LOGDBG(L"      %s = %s", (*it).first.c_str(), (*it).second.GetValue().c_str());
            }
            LOGDBG(L"  Application:");
            const nxrm::pkg::policy::PROPERTYMAP& appprops = object.GetProperty(nxrm::pkg::policy::RULE_APPLICATION);
            for(nxrm::pkg::policy::PROPERTYMAP::const_iterator it=appprops.begin(); it!=appprops.end(); ++it) {
                LOGDBG(L"      %s = %s", (*it).first.c_str(), (*it).second.GetValue().c_str());
            }
            LOGDBG(L"  Location:");
            const nxrm::pkg::policy::PROPERTYMAP& locprops = object.GetProperty(nxrm::pkg::policy::RULE_LOCATION);
            for(nxrm::pkg::policy::PROPERTYMAP::const_iterator it=locprops.begin(); it!=locprops.end(); ++it) {
                LOGDBG(L"      %s = %s", (*it).first.c_str(), (*it).second.GetValue().c_str());
            }
            LOGDBG(L"  Resource:");
            const nxrm::pkg::policy::PROPERTYMAP& resprops = object.GetProperty(nxrm::pkg::policy::RULE_RESOURCE);
            for(nxrm::pkg::policy::PROPERTYMAP::const_iterator it=resprops.begin(); it!=resprops.end(); ++it) {
                LOGDBG(L"      %s = %s", (*it).first.c_str(), (*it).second.GetValue().c_str());
            }
            LOGDBG(L"  Obligations:");
            const nudf::util::OBS& obs = result.GetObligations().GetObligations();
            for(nudf::util::OBS::const_iterator it=obs.begin(); it!=obs.end(); ++it) {
                if(OB_ID_OVERLAY == (*it).first) {
                    LOGDBG(L"     - Overlay");
                }
                else if(OB_ID_CLASSIFY == (*it).first) {
                    LOGDBG(L"     - Classify");
                }
                else {
                    ; //
                }
            }
            LOGDBG(L"  Hit Policies:");
            const std::vector<nxrm::pkg::CEvalPolicyResult>& policies = result.GetPolicies();
            for(std::vector<nxrm::pkg::CEvalPolicyResult>::const_iterator it=policies.begin(); it!=policies.end(); ++it) {
                LOGDBG(L"      %04d: %s", (*it).GetId(), (*it).GetName().c_str());
            }

        }
    }
    else {
        LOGWARN(ERROR_NOT_FOUND, L"Session (%d) does not exist", session_id);
    }
}

bool CPolicyServ::FindObs(_In_ ULONG session_id, _In_ ULONGLONG eval_id, _Out_ nudf::util::CObligations& obs)
{
    bool find = false;
    nudf::util::CRwSharedLocker lock(&_bundlelock);
    SESSIONPOLICYSET::iterator it = _sessionbudnle.find(session_id);
    if(it != _sessionbudnle.end()) {
        find = (*it).second->GetObsCache().Find(eval_id, obs);
    }
    return find;
}

bool CPolicyServ::GetClassifyProfileGroup(_In_ ULONG session_id, _Out_ std::wstring& group) throw()
{
    bool find = false;
    nudf::util::CRwSharedLocker lock(&_bundlelock);
    SESSIONPOLICYSET::iterator it = _sessionbudnle.find(session_id);
    if(it != _sessionbudnle.end()) {
        group = (*it).second->GetClassifyProfileGroup();
        find = true;
    }
    return find;
}

bool CPolicyServ::CreateSessionPolicySet(_In_ ULONG session_id)
{
    nudf::win::CSession session(session_id);

    if(!session.IsValid()) {
        return false;
    }

    nudf::win::CUser user;
    if(!session.GetUser(user)) {
        return false;
    }

    nxrm::pkg::policy::PROPERTYMAP user_props;
    user_props.insert(std::pair<std::wstring,nxrm::pkg::policy::CProperty>(USER_PROP_ID, nxrm::pkg::policy::CProperty(USER_PROP_ID, user.GetSid().GetSidStr().c_str())));
    user_props.insert(std::pair<std::wstring,nxrm::pkg::policy::CProperty>(USER_PROP_NAME, nxrm::pkg::policy::CProperty(USER_PROP_NAME, user.GetPrincipleName().empty()?user.GetName().c_str():user.GetPrincipleName().c_str())));
    user_props.insert(std::pair<std::wstring,nxrm::pkg::policy::CProperty>(USER_PROP_TENANTID, nxrm::pkg::policy::CProperty(USER_PROP_TENANTID, BUILTIN_NEXTLABS_TENANTID)));

    std::shared_ptr<CSessionPolicySet> spSet(new CSessionPolicySet(session_id, user_props));
    spSet->Load(_profile);
    nudf::util::CRwExclusiveLocker lock(&_bundlelock);
    _sessionbudnle[session_id] = spSet;
    return true;
}

void CPolicyServ::RemoveSessionPolicySet(_In_ ULONG session_id)
{
    nudf::util::CRwExclusiveLocker lock(&_bundlelock);
    SESSIONPOLICYSET::iterator it = _sessionbudnle.find(session_id);
    if(it != _sessionbudnle.end()) {
        _sessionbudnle.erase(it);
    }
}

bool CPolicyServ::FindCachedResource(_In_ const std::wstring& path, _Out_ std::shared_ptr<CResCacheRecord>& record)
{
    nudf::util::CRwSharedLocker lock(&_rescachelock);
    return _rescache.Find(path, record);
}

void CPolicyServ::CacheResource(_In_ const std::wstring& path, _In_ const std::shared_ptr<CResCacheRecord>& record)
{
    nudf::util::CRwExclusiveLocker lock(&_rescachelock);
    _rescache.Insert(path, record);
}

void CPolicyServ::RemoveResource(_In_ const std::wstring& path)
{
    nudf::util::CRwExclusiveLocker lock(&_rescachelock);
    _rescache.Remove(path);
}

bool CPolicyServ::FindCachedApplication(_In_ const std::wstring& path, _Out_ nudf::win::CPEFile& pe)
{
    nudf::util::CRwSharedLocker lock(&_appcachelock);
    return _appcache.Find(path, pe);
}

void CPolicyServ::CacheApplication(_In_ const std::wstring& path, _In_ const nudf::win::CPEFile& pe)
{
    nudf::util::CRwExclusiveLocker lock(&_appcachelock);
    _appcache.Insert(path, pe);
}

bool CPolicyServ::InvalidateLoggedOnUserList(const std::vector<std::pair<std::wstring,std::wstring>>& sids) const throw()
{
    const nxrm::pkg::USERGROUP& usermap = _profile.GetUsersMap();
    for(std::vector<std::pair<std::wstring,std::wstring>>::const_iterator it=sids.begin(); it!=sids.end(); ++it) {
        bool found = false;
        for(nxrm::pkg::USERGROUP::const_iterator mit=usermap.begin(); mit!=usermap.end(); ++mit) {
            if(0 == _wcsicmp((*it).first.c_str(), (*mit)->GetId().c_str())) {
                found = true;
                break;
            }
        }
        if(!found) {
            std::wstring wsfile = _nxPaths.GetCacheDir();
            wsfile += L"\\";
            wsfile += (*it).first;
            if(INVALID_FILE_ATTRIBUTES == GetFileAttributesW(wsfile.c_str())) {
                // SID is not in the map and is not on disk
                return false;
            }
        }
    }
    return true;
}

void CPolicyServ::UpdateLoggedOnUserCacheFile(const std::vector<std::pair<std::wstring,std::wstring>>& sids) throw()
{
    for(std::vector<std::pair<std::wstring,std::wstring>>::const_iterator it=sids.begin(); it!=sids.end(); ++it) {

        std::wstring wsfile = _nxPaths.GetCacheDir();
        wsfile += L"\\";
        wsfile += (*it).first;

        HANDLE h = ::CreateFileW(wsfile.c_str(), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if(INVALID_HANDLE_VALUE != h) {
            CloseHandle(h);
            h = INVALID_HANDLE_VALUE;
        }
    }
}

//
//  class CSessionPolicySet
//

void CSessionPolicySet::Load(_In_ const nxrm::pkg::CPolicyProfile& profile)
{
    const nxrm::pkg::POLICYGROUP& policygroup = profile.GetPolicySet();

    // Clear existing policies
    this->GetPolicySet().clear();

    // Get user groups
    std::set<unsigned int> groups;
    nxrm::pkg::policy::PROPERTYMAP::const_iterator it_userid = _userprops.find(USER_PROP_ID);
    nxrm::pkg::policy::PROPERTYMAP::const_iterator it_tenantid = _userprops.find(USER_PROP_TENANTID);
    profile.GetGroupIdSet((_userprops.end()==it_userid) ? L"" : (*it_userid).second.GetValue(),
                          (_userprops.end()==it_tenantid) ? L"" : (*it_tenantid).second.GetValue(), groups, _usercontext);

    nxrm::pkg::policy::PROPERTYMAP::iterator it_userctx = _userprops.find(USER_PROP_CONTEXT);
    if(_userprops.end()==it_userctx) {
        _userprops.insert(std::pair<std::wstring,nxrm::pkg::policy::CProperty>(USER_PROP_CONTEXT, nxrm::pkg::policy::CProperty(USER_PROP_CONTEXT, _usercontext.c_str())));
    }
    else {
        (*it_userctx).second.SetValue(_usercontext.c_str());
    }


    // Get host name
    static nudf::win::CHost host;
    _locprops.clear();
    _locprops.insert(std::pair<std::wstring,nxrm::pkg::policy::CProperty>(LOC_PROP_HOST, nxrm::pkg::policy::CProperty(LOC_PROP_HOST, host.GetHostName().c_str())));
    _locprops.insert(std::pair<std::wstring,nxrm::pkg::policy::CProperty>(LOC_PROP_LOCALE, nxrm::pkg::policy::CProperty(LOC_PROP_LOCALE, host.GetLocaleName().c_str())));

    // Get all policies match current user
    for(nxrm::pkg::POLICYGROUP::const_iterator it=policygroup.begin(); it!=policygroup.end(); ++it) {

        // Match the group?
        if((UINT)-1 != (*it)->GetUserGroup()) {
            if(groups.end() == groups.find((*it)->GetUserGroup())) {
                continue;
            }
        }

        // Match LOCATION rules?
        if(!(*it)->MatchLocation(_locprops)) {
            continue;
        }

        // Match USER rules?
        if(!(*it)->MatchUser(_userprops)) {
            continue;
        }

        // Add it
        GetPolicySet().push_back((*it).get());

        // Check Classify Obligation
        nudf::util::CObligation ob;
        if((*it)->GetObligations().Find(OB_ID_CLASSIFY, ob)) {
            std::wstring wsGroup = ob.GetParams()[L"group"];
            if(!wsGroup.empty()) {
                if(0 == _wcsicmp(wsGroup.c_str(), L"Default")) {
                    if(_clsprofilegroup.empty()) {
                        _clsprofilegroup = wsGroup;
                    }
                }
                else {
                    _clsprofilegroup = wsGroup;
                }
            }
        }
    }
}

void CSessionPolicySet::OnPreEvalApplication(_Inout_ nxrm::pkg::policy::PROPERTYMAP& app) throw()
{
    std::wstring    image;
    nxrm::pkg::policy::PROPERTYMAP::const_iterator it = app.find(APP_PROP_IMAGE_PATH);
    if(it == app.end()) {
        return;
    }
    image = (*it).second.GetValue();

    nudf::win::CPEFile pe;

    if(!gpEngine->GetPolicyServ()->FindCachedApplication(image, pe)) {
        pe.Load(image.c_str());
        if(pe.IsValid()) {
            gpEngine->GetPolicyServ()->CacheApplication(image, pe);
        }        
    }
    if(!pe.IsValid()) {
        return;
    }

    // Add signature information
    if(pe.GetCert().IsValid()) {
        if(!pe.GetCert().GetSubject().empty()) {
            app.insert(std::pair<std::wstring,nxrm::pkg::policy::CProperty>(APP_PROP_PUBLISHER, nxrm::pkg::policy::CProperty(APP_PROP_PUBLISHER, pe.GetCert().GetSubject().c_str())));
        }
        if(!pe.GetCert().GetIssuer().empty()) {
            app.insert(std::pair<std::wstring,nxrm::pkg::policy::CProperty>(APP_PROP_CERT_ISSUER, nxrm::pkg::policy::CProperty(APP_PROP_CERT_ISSUER, pe.GetCert().GetIssuer().c_str())));
        }
        if(!pe.GetCert().GetThumbprint().empty()) {
            std::wstring thumbprint = nudf::string::FromBytes<wchar_t>(&pe.GetCert().GetThumbprint()[0], (ULONG)pe.GetCert().GetThumbprint().size());
            app.insert(std::pair<std::wstring,nxrm::pkg::policy::CProperty>(APP_PROP_CERT_THUMBPRINT, nxrm::pkg::policy::CProperty(APP_PROP_CERT_THUMBPRINT, thumbprint.c_str())));
        }
        if(!pe.GetCert().GetValidFromDate().wYear != 0) {
            std::wstring validfrom = nudf::string::FromSystemTime<wchar_t>(&pe.GetCert().GetValidFromDate(), true);
            app.insert(std::pair<std::wstring,nxrm::pkg::policy::CProperty>(APP_PROP_CERT_VALIDFROM, nxrm::pkg::policy::CProperty(APP_PROP_CERT_VALIDFROM, validfrom.c_str())));
        }
        if(!pe.GetCert().GetValidThruDate().wYear != 0) {
            std::wstring validthru = nudf::string::FromSystemTime<wchar_t>(&pe.GetCert().GetValidThruDate(), true);
            app.insert(std::pair<std::wstring,nxrm::pkg::policy::CProperty>(APP_PROP_CERT_VALIDTHRU, nxrm::pkg::policy::CProperty(APP_PROP_CERT_VALIDTHRU, validthru.c_str())));
        }
    }    
}

bool CSessionPolicySet::OnPreEvalResource(_Inout_ nxrm::pkg::policy::PROPERTYMAP& resource) throw()
{
    HANDLE          hFile = INVALID_HANDLE_VALUE;
    std::wstring    path;
    DWORD           dwError = 0;
    bool            impersonated = false;
    nxrm::pkg::policy::PROPERTYMAP::const_iterator it = resource.find(RES_PROP_PATH);
    if(it == resource.end()) {
        return false;
    }
    path = (*it).second.GetValue();


    hFile = ::CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(INVALID_HANDLE_VALUE == hFile) {

        dwError = GetLastError();
        if(ERROR_ACCESS_DENIED != dwError) {
            // Log and Exit
            LOGWARN(dwError, L"Fail to open target file: %s", path.c_str());
            return false;
        }

        // Impersonation is required
        HANDLE  hToken = NULL;

        
        if(!WTSQueryUserToken(_sessionid, &hToken)) {
            // Fail to get current session token
            // log & Exit
            LOGWARN(GetLastError(), L"Fail to get session token");
            return false;
        }

        if(!ImpersonateLoggedOnUser(hToken)) {
            // Fail to impersonate current user
            // log & Exit
            LOGWARN(GetLastError(), L"Fail to impersonate logon user");
            CloseHandle(hToken);
            return false;
        }

        // Set impersonate flag
        impersonated = true;
        CloseHandle(hToken);

        // try OPEN again
        hFile = ::CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if(INVALID_HANDLE_VALUE == hFile) {
            // Still cannot open target file
            // log & Exit
            LOGWARN(GetLastError(), L"Fail to open target file: %s", path.c_str());
            RevertToSelf();
            return false;
        }
    }
    
    LoadResourceTags(path, hFile, resource);
    CloseHandle(hFile);
    hFile = INVALID_HANDLE_VALUE;

    if(impersonated) {
        RevertToSelf();
    }

    return true;
}

void CSessionPolicySet::LoadResourceTags(_In_ const std::wstring& path, _In_ HANDLE handle, _Inout_ nxrm::pkg::policy::PROPERTYMAP& resource) throw()
{
    ULARGE_INTEGER  timestamp = {0, 0};
    FILETIME        modifiedtime = {0, 0};
    std::shared_ptr<nxrm::engine::CResCacheRecord> spRecord;

    if(!GetFileTime(handle, NULL, NULL, &modifiedtime)) {
        // Fail to get last modified time
        // log & Exit
        LOGWARN(GetLastError(), L"Fail to get last modified time of file: %s", path.c_str());
        return;
    }

    timestamp.HighPart = modifiedtime.dwHighDateTime;
    timestamp.LowPart  = modifiedtime.dwLowDateTime;

    // Find in cache
    if(gpEngine->GetPolicyServ()->FindCachedResource(path, spRecord)) {
        // Found an record, and its timestamp equals to input resource
        if(timestamp.QuadPart == spRecord->GetTimestamp().QuadPart) {
            // Copy & Exit
            for(nxrm::pkg::policy::PROPERTYMAP::const_iterator cit=spRecord->GetProperties().begin(); cit != spRecord->GetProperties().end(); ++cit) {
                resource.insert(std::pair<std::wstring,nxrm::pkg::policy::CProperty>((*cit).first, (*cit).second));
            }
            return;
        }
    }


    // Load NXL header
    std::vector<std::pair<std::wstring,std::wstring>> tags;
    nudf::util::nxl::NXLSTATUS ns = nudf::util::nxl::NXL_UNKNOWN;
    try {
        nudf::util::nxl::CFile nf(path.c_str());
        nf.Open(&ns);
        nf.GetNTags(tags);
    }
    catch(const nudf::CException& e) {
        LOGDBG(L"Fail to load tags from file (%s): %s", nudf::util::nxl::NxlStatus2Message(ns), path.c_str());
        LOGDBG(L"  ==> error:%d, function: %S, file: %S, line: %d", e.GetCode(), e.GetFunction(), e.GetFile(), e.GetLine());
        tags.clear();
    }


    // Create a new record
    spRecord.reset(new CResCacheRecord(path, timestamp));

    // Copy Tags
    for(std::vector<std::pair<std::wstring,std::wstring>>::iterator it=tags.begin(); it!=tags.end(); ++it) {
        // Add tags
        std::transform((*it).first.begin(), (*it).first.end(), (*it).first.begin(), tolower);
        std::transform((*it).second.begin(), (*it).second.end(), (*it).second.begin(), tolower);
        resource.insert(std::pair<std::wstring,nxrm::pkg::policy::CProperty>((*it).first, nxrm::pkg::policy::CProperty((*it).first.c_str(), (*it).second.c_str())));
        spRecord->GetProperties().insert(std::pair<std::wstring,nxrm::pkg::policy::CProperty>((*it).first, nxrm::pkg::policy::CProperty((*it).first.c_str(), (*it).second.c_str())));
    }

    // Finally, cache this record
    gpEngine->GetPolicyServ()->CacheResource(path, spRecord);
}