

#ifndef __NXRM_PKG_POLICY_POLICY_HPP__
#define __NXRM_PKG_POLICY_POLICY_HPP__

#include <string>
#include <vector>
#include <set>
#include <list>

#include <nudf\shared\obutil.h>

#include "policy_rule.hpp"
#include "policy_obligation.hpp"

namespace nxrm {
namespace pkg {
namespace policy {
    
#define POLICY_NODENAME                 L"POLICY"
#define POLICY_ID_ATTRIBUTE             L"id"
#define POLICY_NAME_ATTRIBUTE           L"name"
#define POLICY_USERGROUP_ATTRIBUTE      L"usergroup"
#define POLICY_RIGHTS_ATTRIBUTE         L"rights"
#define POLICY_CUSTOM_RIGHTS_ATTRIBUTE  L"custom_rights"


typedef std::pair<unsigned int, CRule*>                 RULEOBJ;
typedef std::list<std::pair<unsigned int, CRule*>>      RULEOBJGROUP;


class CCondition
{
public:
    CCondition();
    CCondition(RULETYPE type, bool exclude);
    virtual ~CCondition();

    inline bool IsExclude() const throw() {return _exclude;}
    inline RULETYPE GetType() const throw() {return _type;}
    inline const RULEOBJGROUP& GetRules() const throw() {return _rules;}
    inline RULEOBJGROUP& GetRules() throw() {return _rules;}

    CCondition& operator = (const CCondition& cond);
    void Clear();
    
    bool Match(_In_ RULETYPE type, _In_ const PROPERTYMAP& props) const throw();
    
    void FromXml(_In_ IXMLDOMNode* node);
    void ToXml(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* parent, _In_opt_ IXMLDOMNode** node);

private:
    bool                    _exclude;
    RULETYPE                _type;
    RULEOBJGROUP            _rules;
};


typedef std::vector<CCondition>     CONDITIONS;

class CPolicy
{
public:
    CPolicy();
    CPolicy(unsigned int id, const wchar_t* name, unsigned int usergroup, unsigned __int64 rights);
    virtual ~CPolicy();
    
    inline void SetId(unsigned int id) throw() {_id = id;}
    inline unsigned int GetId() const throw() {return _id;}
    inline unsigned int GetUserGroup() const throw() {return _usergroup;}
    inline unsigned __int64 GetRights() const throw() {return _rights;}
    inline const std::wstring& GetName() const throw() {return _name;}
    inline const nudf::util::CObligations& GetObligations() const throw() {return _obs;}
    inline nudf::util::CObligations& GetObligations() throw() {return _obs;}
    inline const CONDITIONS* GetConditions() const throw() {return _conds;}
    inline CONDITIONS* GetConditions() throw() {return _conds;}
        
    void FromXml(_In_ IXMLDOMNode* node);

    bool MatchUser(_In_ const PROPERTYMAP& props) const throw();
    bool MatchApplication(_In_ const PROPERTYMAP& props) const throw();
    bool MatchLocation(_In_ const PROPERTYMAP& props) const throw();
    bool MatchResource(_In_ const PROPERTYMAP& props) const throw();
    bool MatchEnvironment(_In_ const PROPERTYMAP& props) const throw();

protected:
    bool Match(_In_ RULETYPE type, _In_ const PROPERTYMAP& props) const throw();
    void SetRights(_In_ const std::wstring& actions);
    void GetRightActions(_Out_ std::wstring& actions);
    void GetRightDisplayNames(_Out_ std::wstring& names);
    
private:
    unsigned int        _id;
    unsigned int        _usergroup;
    unsigned __int64    _rights;
    std::wstring        _name;
    CONDITIONS          _conds[RULE_MAX];
    nudf::util::CObligations _obs;
};

    
}   // namespace nxrm::pkg::policy
}   // namespace nxrm::pkg
}   // namespace nxrm


#endif