

#ifndef __NXRM_PKG_POLICY_PROFILE_HPP__
#define __NXRM_PKG_POLICY_PROFILE_HPP__

#include <atlbase.h>
#include <atlcomcli.h>
#include <string>
#include <vector>
#include <memory>

#include <nudf\crypto.hpp>
#include <nudf\shared\rightsdef.h>

#include "package.hpp"
#include "profile_basic.hpp"
#include "policy_policy.hpp"
#include "policy_rights.hpp"
#include "policy_user.hpp"
#include "policy_rule.hpp"


namespace nxrm {
namespace pkg {
    
#define PROFILE_POLICY_NODENAME         L"POLICYBUNDLE"

typedef std::vector<std::shared_ptr<policy::CPolicy>>       POLICYGROUP;
typedef std::vector<std::shared_ptr<policy::CUserGroups>>   USERGROUP;
typedef std::vector<std::shared_ptr<policy::CRule>>         RULEGROUP;


class CBundlePackage : public CPackage
{
public:
    CBundlePackage();
    virtual ~CBundlePackage();

    virtual void LoadFromFile(const wchar_t* file);
    virtual void LoadFromXml(const wchar_t* xml);
    virtual void Create(const wchar_t* issuer);
    virtual void Close();

    inline IXMLDOMNode* GetPolicySetNode() throw() {return _policynode.p;}
    inline IXMLDOMNode* GetUserMapNode() throw() {return _usernode.p;}
    inline IXMLDOMNode* GetUsrRuleSetNode() throw() {return _usrnode.p;}
    inline IXMLDOMNode* GetAppRuleSetNode() throw() {return _appnode.p;}
    inline IXMLDOMNode* GetLocRuleSetNode() throw() {return _locnode.p;}
    inline IXMLDOMNode* GetResRuleSetNode() throw() {return _resnode.p;}
    inline IXMLDOMNode* GetEnvRuleSetNode() throw() {return _envnode.p;}

protected:
    void LoadBundle();

private:
    CComPtr<IXMLDOMNode>    _bundle;
    CComPtr<IXMLDOMNode>    _policynode;
    CComPtr<IXMLDOMNode>    _usernode;
    CComPtr<IXMLDOMNode>    _usrnode;
    CComPtr<IXMLDOMNode>    _appnode;
    CComPtr<IXMLDOMNode>    _locnode;
    CComPtr<IXMLDOMNode>    _resnode;
    CComPtr<IXMLDOMNode>    _envnode;
};

class CPolicyProfile : public CProfile
{
public:
    CPolicyProfile();
    virtual ~CPolicyProfile();
    
    virtual void FromXml(const std::wstring& xml);
    virtual void FromXml(IXMLDOMNode* node);

    virtual void Clear() throw();
    
    inline const POLICYGROUP& GetPolicySet() const throw() {return _policyset;}
    inline const USERGROUP& GetUsersMap() const throw() {return _usermap;}
    inline const RULEGROUP* GetRulesSet() const throw() {return _rules;}
    inline const std::wstring& GetStrTimestamp() const throw() {return _stimestamp;}
    inline void SetStrTimestamp(const std::wstring& stimestamp) throw() {_stimestamp=stimestamp;}    
    virtual void ResetTimestamp() throw()
    {
        _stimestamp.clear();
        CProfile::ResetTimestamp();
    }

    bool GetGroupIdSet(_In_ const std::wstring& id, _In_ const std::wstring& tenantid, _Out_  std::set<unsigned int>& groups, _Out_ std::wstring& context) const throw();

    const nxrm::pkg::policy::CUserGroups* GetUnknownUserGroup() const throw()
    {
        for(USERGROUP::const_iterator it=_usermap.begin(); it!=_usermap.end(); ++it) {
            if((*it)->IsUnknownUserGroups()) {
                return (*it).get();
            }
        }
        // Not found
        return NULL;
    }

    const nxrm::pkg::policy::CUserGroups* GetUserGroup(_In_ const std::wstring& id) const throw()
    {
        if(id.empty()) {
            return GetUnknownUserGroup();
        }

        for(USERGROUP::const_iterator it=_usermap.begin(); it!=_usermap.end(); ++it) {
            if(0 == _wcsicmp(id.c_str(), (*it)->GetId().c_str())) {
                return (*it).get();
            }
        }

        // Not found
        return NULL;
    }
    
    
private:
    void ToRegex();
    void LoadPolicySet(IXMLDOMNode* node);
    void LoadUserGroupMap(IXMLDOMNode* node);
    void LoadUserSet(IXMLDOMNode* node);
    void LoadAppSet(IXMLDOMNode* node);
    void LoadLocSet(IXMLDOMNode* node);
    void LoadResSet(IXMLDOMNode* node);
    void LoadEnvSet(IXMLDOMNode* node);

    inline RULEGROUP* GetRulesSet() throw() {return _rules;}
    policy::CRule* FindRuleById(_In_ policy::RULETYPE type, _In_ unsigned int id) throw();

private:    
    std::wstring    _stimestamp;
    bool        _regexOn;
    POLICYGROUP _policyset;
    USERGROUP   _usermap;
    RULEGROUP   _rules[policy::RULE_MAX];
};


typedef enum _EVALMASK {
    EVALUSER = 0x00000001,
    EVALAPP  = 0x00000002,
    EVALLOC  = 0x00000004,
    EVALRES  = 0x00000008
} EVALMASK;

class CEvalObject
{
public:
    CEvalObject() {}
    virtual ~CEvalObject() {}

    inline const policy::PROPERTYMAP& GetProperty(_In_ policy::RULETYPE type) const throw() {return _props[type];}
    inline policy::PROPERTYMAP& GetProperty(_In_ policy::RULETYPE type) throw() {return _props[type];}
    inline const std::set<unsigned int>& GetUserGroups() const throw() {return _groups;}
    inline std::set<unsigned int>& GetUserGroups() throw() {return _groups;}
    inline void SetUserGroups(_In_ const std::set<unsigned int>& groups) throw() {_groups = groups;}
    inline void Clear() throw() {for(int i=0; i<policy::RULE_MAX; i++) _props[i].clear();}
    inline bool MatchGroup(_In_ UINT groupid) const throw()
    {
        if((UINT)-1 == groupid) {
            return true;
        }
        for(std::set<unsigned int>::const_iterator it=_groups.begin(); it!=_groups.end(); ++it) {
            if((*it) == groupid) {
                return true;
            }
        }
        return false;
    }

    inline void InsertProperty(_In_ policy::RULETYPE type, const std::wstring& key, const nxrm::pkg::policy::CProperty& prop)
    {
        std::wstring lowerkey = key;
        std::transform(lowerkey.begin(), lowerkey.end(), lowerkey.begin(), tolower);
        _props[type].insert(std::pair<std::wstring,nxrm::pkg::policy::CProperty>(lowerkey, prop));
    }

private:
    std::set<unsigned int> _groups;
    policy::PROPERTYMAP _props[policy::RULE_MAX];
};

class CEvalPolicyResult
{
public:
    CEvalPolicyResult() : _id((UINT)-1), _rights(0)
    {
    }
    CEvalPolicyResult(UINT id, const std::wstring& name, ULONGLONG rights) : _id(id), _name(name), _rights(rights)
    {
    }
    virtual ~CEvalPolicyResult()
    {
    }

    inline UINT GetId() const throw() {return _id;}
    inline const std::wstring& GetName() const throw() {return _name;}
    inline ULONGLONG GetRights() const throw() {return _rights;}
    inline CEvalPolicyResult& operator = (const CEvalPolicyResult& result) throw()
    {
        if(this != &result) {
            _id = result.GetId();
            _name = result.GetName();
            _rights = result.GetRights();
        }
        return *this;
    }


private:
    UINT         _id;
    std::wstring _name;
    ULONGLONG    _rights;
};

class CEvalResult
{
public:
    CEvalResult() : _uid(0), _rights(0) {GenerateUid();}
    virtual ~CEvalResult() {}
    
    inline ULONGLONG GetUid() const throw() {return _uid;}
    inline ULONGLONG GetRights() const throw() {return _rights;}
    inline const std::vector<CEvalPolicyResult>& GetPolicies() const throw() {return _policies;}
    inline const nudf::util::CObligations& GetObligations() const throw() {return _obs;}
    inline CEvalResult& operator = (const CEvalResult& result) throw()
    {
        if(this != &result) {
            _uid = result.GetUid();
            _rights = result.GetRights();
            _policies = result.GetPolicies();
            _obs = result.GetObligations();
        }
        return *this;
    }
    inline void AddPolicy(const policy::CPolicy* policy) throw()
    {
        _rights |= policy->GetRights();
        _policies.push_back(CEvalPolicyResult(policy->GetId(), policy->GetName(), policy->GetRights()));
        _obs.Merge(policy->GetObligations());
    }

    inline void Clear() throw()
    {
        _rights = 0;
        _policies.clear();
        _obs.Clear();
    }

    inline void SetNotCacheFlag() throw() {_rights |= RIGHTS_NOT_CACHE;}

private:
    void GenerateUid() throw();

private:
    ULONGLONG   _rights;
    std::vector<CEvalPolicyResult> _policies;
    nudf::util::CObligations _obs;
    ULONGLONG   _uid;
};

class CPolicySet
{
public:
    CPolicySet() : _mask(EVALUSER|EVALAPP|EVALLOC|EVALRES)
    {
    }

    CPolicySet(_In_ ULONG mask) : _mask(mask)
    {
    }
    virtual ~CPolicySet()
    {
    }

    inline const std::list<policy::CPolicy*>& GetPolicySet() const throw() {return _policyset;}
    inline std::list<policy::CPolicy*>& GetPolicySet() throw() {return _policyset;}
    inline ULONG GetMask() const throw() {return _mask;}
    inline void SetMask(_In_ ULONG mask) throw() {_mask = mask;}
    
    virtual void Clear() throw()
    {
        _policyset.clear();
        _mask = EVALUSER|EVALAPP|EVALLOC|EVALRES;
    }

    void Evaluate(_Inout_ CEvalObject& object, _Out_ CEvalResult& result) throw();
    CPolicySet& operator = (const CPolicySet& policyset) throw();

    // Sometime we need to delay getting properties only when there IS related policy
    virtual void OnPreEvalUser(_Inout_ policy::PROPERTYMAP& user) throw() {}
    virtual void OnPreEvalApplication(_Inout_ policy::PROPERTYMAP& app) throw() {}
    virtual void OnPreEvalLocation(_Inout_ policy::PROPERTYMAP& location) throw() {}
    virtual bool OnPreEvalResource(_Inout_ policy::PROPERTYMAP& resource) throw() {return true;}

private:
    std::list<policy::CPolicy*> _policyset;
    ULONG               _mask;
};

    
}   // namespace nxrm::pkg
}   // namespace nxrm


#endif  // __NXRM_PKG_POLICY_PROFILE_HPP__