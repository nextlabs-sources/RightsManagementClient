

#ifndef __NXRM_ENGINE_POLICYDB_H__
#define __NXRM_ENGINE_POLICYDB_H__

#include <nudf\exception.hpp>
#include <nudf\rwlock.hpp>
#include <nudf\listentry.hpp>
#include <nudf\pe.hpp>

#include <profile_policy.hpp>
#include <policy_policy.hpp>
#include <policy_property.hpp>

#include <string>
#include <vector>
#include <map>

namespace nxrm {
namespace engine {

    

class CResCacheRecord
{
public:
    CResCacheRecord()
    {
        _timestamp.QuadPart = 0;
    }
    CResCacheRecord(_In_ const std::wstring& path, _In_ ULARGE_INTEGER timestamp) : _path(path)
    {
        _timestamp.QuadPart = timestamp.QuadPart;
    }
    virtual ~CResCacheRecord()
    {
    }

    inline const nxrm::pkg::policy::PROPERTYMAP& GetProperties() const throw() {return _props;}
    inline nxrm::pkg::policy::PROPERTYMAP& GetProperties() throw() {return _props;}
    inline const ULARGE_INTEGER& GetTimestamp() const throw() {return _timestamp;}
    inline const std::wstring& GetPath() const throw() {return _path;}
    inline CResCacheRecord& operator = (const CResCacheRecord& data) throw()
    {
        if(this != &data) {
            _path = data.GetPath();
            _timestamp = data.GetTimestamp();
            _props = data.GetProperties();
        }
        return *this;
    }

private:
    std::wstring    _path;
    ULARGE_INTEGER  _timestamp;
    nxrm::pkg::policy::PROPERTYMAP _props;
};

class CResLruCache : public nudf::util::CLRUList<std::wstring, std::shared_ptr<CResCacheRecord>, 256>
{
public:
    CResLruCache() {}
    virtual ~CResLruCache() {}
};

class CAppLruCache : public nudf::util::CLRUList<std::wstring, nudf::win::CPEFile, 256>
{
public:
    CAppLruCache() {}
    virtual ~CAppLruCache() {}
};

class CObligationCache
{
public:
    CObligationCache()
    {
        ::InitializeCriticalSection(&_cs);
    }
    virtual ~CObligationCache()
    {
        ::DeleteCriticalSection(&_cs);
    }

    void Insert(_In_ ULONGLONG id, _In_ const nudf::util::CObligations& obs)
    {
        ::EnterCriticalSection(&_cs);
        while(_list.size() >= 256) {
            _list.pop_back();
        }
        _list.push_front(std::pair<ULONGLONG, nudf::util::CObligations>(id, obs));
        ::LeaveCriticalSection(&_cs);
    }

    bool Find(_In_ ULONGLONG id, _Out_ nudf::util::CObligations& obs)
    {
        bool find = false;
        ::EnterCriticalSection(&_cs);
        for(std::list<std::pair<ULONGLONG, nudf::util::CObligations>>::const_iterator it=_list.begin(); it!=_list.end(); ++it) {
            if((*it).first == id) {
                obs = (*it).second;
                find = true;
                break;
            }
        }
        ::LeaveCriticalSection(&_cs);
        return find;
    }

    void Clear() throw()
    {
        ::EnterCriticalSection(&_cs);
        _list.clear();
        ::LeaveCriticalSection(&_cs);
    }

private:
    std::list<std::pair<ULONGLONG, nudf::util::CObligations>>   _list;
    CRITICAL_SECTION _cs;

};


    
class CSessionPolicySet : public nxrm::pkg::CPolicySet
{
public:
    CSessionPolicySet()
        : nxrm::pkg::CPolicySet(nxrm::pkg::EVALAPP|nxrm::pkg::EVALRES), _sessionid((ULONG)-1)
    {
    }
    CSessionPolicySet(_In_ ULONG session_id, _In_ const nxrm::pkg::policy::PROPERTYMAP& user_props)
        : nxrm::pkg::CPolicySet(nxrm::pkg::EVALAPP|nxrm::pkg::EVALRES),
        _sessionid(session_id),
        _userprops(user_props)
    {
    }
    ~CSessionPolicySet()
    {
    }

    CSessionPolicySet& operator = (const CSessionPolicySet& set)
    {
        if(this != &set) {
            ((nxrm::pkg::CPolicySet&)(*this)) = (const nxrm::pkg::CPolicySet&)set;
            _sessionid = set.GetSessionId();
            _userprops = set.GetUserProperty();
            _clsprofilegroup = set.GetClassifyProfileGroup();
            _usercontext = set.GetUserContext();
            _locprops = set.GetLocationProperty();
            _obscache = set.GetObsCache();
        }
    }

    virtual void Clear() throw()
    {
        nxrm::pkg::CPolicySet::Clear();
        SetMask(nxrm::pkg::EVALAPP|nxrm::pkg::EVALRES);
        _sessionid = (ULONG)-1;
        _clsprofilegroup.clear();
        _usercontext.clear();
        _userprops.clear();
        _locprops.clear();
        _obscache.Clear();
    }

    void Load(_In_ const nxrm::pkg::CPolicyProfile& profile);
    virtual bool OnPreEvalResource(_Inout_ nxrm::pkg::policy::PROPERTYMAP& resource) throw();
    virtual void OnPreEvalApplication(_Inout_ nxrm::pkg::policy::PROPERTYMAP& app) throw();

    inline ULONG GetSessionId() const throw() {return _sessionid;}
    inline const CObligationCache& GetObsCache() const throw() {return _obscache;}
    inline CObligationCache& GetObsCache()  throw() {return _obscache;}
    inline const std::wstring& GetUserContext() const throw() {return _usercontext;}
    inline const nxrm::pkg::policy::PROPERTYMAP& GetUserProperty() const throw() {return _userprops;}
    inline const nxrm::pkg::policy::PROPERTYMAP& GetLocationProperty() const throw() {return _locprops;}
    inline const std::wstring& GetClassifyProfileGroup() const throw() {return _clsprofilegroup;}


protected:
    void LoadResourceTags(_In_ const std::wstring& path, _In_ HANDLE handle, _Inout_ nxrm::pkg::policy::PROPERTYMAP& resource) throw();

private:
    ULONG   _sessionid;
    std::wstring _clsprofilegroup;
    std::wstring _usercontext;
    nxrm::pkg::policy::PROPERTYMAP _userprops;
    nxrm::pkg::policy::PROPERTYMAP _locprops;
    CObligationCache               _obscache;
};

typedef std::map<ULONG, std::shared_ptr<CSessionPolicySet>>     SESSIONPOLICYSET;
typedef std::shared_ptr<nxrm::pkg::CPolicyProfile>              POLICYBUNDLE;

class CPolicyServ
{
public:
    CPolicyServ();
    virtual ~CPolicyServ();
    
    void Load(_In_ const nudf::crypto::CAesKeyBlob& key);
    bool IsLoaded() throw();
    bool IsEmpty() throw();
    void Authorize(_In_ ULONG session_id,
                   _In_ nxrm::pkg::CEvalObject& object,
                   _Out_ nxrm::pkg::CEvalResult& result
                   ) throw();
    bool FindObs(_In_ ULONG session_id, _In_ ULONGLONG eval_id, _Out_ nudf::util::CObligations& obs);
    bool GetClassifyProfileGroup(_In_ ULONG session_id, _Out_ std::wstring& group) throw();

    bool CreateSessionPolicySet(_In_ ULONG session_id);
    void RemoveSessionPolicySet(_In_ ULONG session_id);

    bool FindCachedResource(_In_ const std::wstring& path, _Out_ std::shared_ptr<CResCacheRecord>& record);
    void CacheResource(_In_ const std::wstring& path, _In_ const std::shared_ptr<CResCacheRecord>& record);
    void RemoveResource(_In_ const std::wstring& path);
    bool FindCachedApplication(_In_ const std::wstring& path, _Out_ nudf::win::CPEFile& pe);
    void CacheApplication(_In_ const std::wstring& path, _In_ const nudf::win::CPEFile& pe);

    inline const FILETIME& GetPolicyTimestamp() const throw() {return _profile.GetTimestamp();}
    inline const std::wstring& GetStrPolicyTimestamp() const throw() {return _profile.GetStrTimestamp();}
    inline ULONGLONG GetPolicyTimestampUll() const throw() {return *((ULONGLONG*)(&_profile.GetTimestamp()));}
    inline void ResetTimestamp() throw() {_profile.ResetTimestamp();}
    inline const nxrm::pkg::CPolicyProfile& GetPolicyProfile() const throw() {return _profile;}

    bool InvalidateLoggedOnUserList(const std::vector<std::pair<std::wstring,std::wstring>>& sids) const throw();
    void UpdateLoggedOnUserCacheFile(const std::vector<std::pair<std::wstring,std::wstring>>& sids) throw();

protected:
    void RightsToNames(_In_ const ULONGLONG rights, _Out_ std::wstring& names);

private:
    nxrm::pkg::CPolicyProfile   _profile;
    SESSIONPOLICYSET            _sessionbudnle;
    nudf::util::CRwLock         _bundlelock;
    CResLruCache                _rescache;
    nudf::util::CRwLock         _rescachelock;
    CAppLruCache                _appcache;
    nudf::util::CRwLock         _appcachelock;
};


}   // namespace engine
}   // namespace nxrm


#endif  // __NXRM_ENGINE_POLICYDB_H__