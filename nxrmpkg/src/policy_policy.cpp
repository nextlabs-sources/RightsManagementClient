

#include <Windows.h>
#include <atlbase.h>
#include <atlcomcli.h>

#include <string>
#include <set>

#include <nudf\exception.hpp>
#include <nudf\string.hpp>
#include <nudf\xmlparser.hpp>
#include <nudf\shared\rightsdef.h>

#include "policy_rule.hpp"
#include "policy_obligation.hpp"
#include "policy_policy.hpp"



using namespace nxrm::pkg::policy;


//
//  class CCondition
//
#define CONDITION_NODENAME          L"CONDITION"
#define CONDITION_TYPE_ATTRIBUTE    L"type"
#define CONDITION_EXCLUDE_ATTRIBUTE L"exclude"

#define POLICY_DEFAULT_NAME         L"Unnamed Policy"


CCondition::CCondition() : _exclude(false), _type(RULE_MAX)
{
}

CCondition::CCondition(RULETYPE type, bool exclude) : _exclude(exclude), _type(type)
{
}

CCondition::~CCondition()
{
}

CCondition& CCondition::operator = (const CCondition& cond)
{
    if(this != &cond) {
        _exclude = cond.IsExclude();
        _type = cond.GetType();
        _rules = cond.GetRules();
    }
    return *this;
}

void CCondition::Clear()
{
    _exclude = false;
    _type = RULE_MAX;
    _rules.clear();
}

bool CCondition::Match(_In_ RULETYPE type, _In_ const PROPERTYMAP& props) const throw()
{
    if(type != _type) {
        return false;
    }

    for(RULEOBJGROUP::const_iterator it=_rules.begin(); it!=_rules.end(); ++it) {
        if((*it).second->Match(props)) {
            // Match an exclude rule? then doesn't match this policy
            return _exclude ? false : true;
        }
    }

    // Not match any rule
    return _exclude ? true : false;
}

void CCondition::FromXml(_In_ IXMLDOMNode* node)
{
    HRESULT      hr = S_OK;
    std::wstring root_name;
    std::wstring exclude;
    std::wstring text;
    std::wstring type;
    std::vector<int> rules;

    if(NODE_ELEMENT != nudf::util::XmlUtil::GetNodeType(node)) {
        throw WIN32ERROR2(ERROR_DATATYPE_MISMATCH);
    }

    root_name = nudf::util::XmlUtil::GetNodeName(node);
    if(0 != _wcsicmp(root_name.c_str(), CONDITION_NODENAME)) {
        throw WIN32ERROR2(ERROR_INVALID_NAME);
    }

    Clear();
    
    if(!nudf::util::XmlUtil::GetNodeAttribute(node, CONDITION_TYPE_ATTRIBUTE, type)) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    if(0 == _wcsicmp(type.c_str(), L"usr")) {
        _type = RULE_USER;
    }
    else if(0 == _wcsicmp(type.c_str(), L"app")) {
        _type = RULE_APPLICATION;
    }
    else if(0 == _wcsicmp(type.c_str(), L"loc")) {
        _type = RULE_LOCATION;
    }
    else if(0 == _wcsicmp(type.c_str(), L"res")) {
        _type = RULE_RESOURCE;
    }
    else if(0 == _wcsicmp(type.c_str(), L"env")) {
        _type = RULE_ENV;
    }
    else {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }

    nudf::util::XmlUtil::GetNodeAttribute(node, CONDITION_EXCLUDE_ATTRIBUTE, exclude);
    if(exclude.empty() || !nudf::string::ToBoolean<wchar_t>(exclude, &_exclude)) {
        _exclude = false;
    }

    text = nudf::util::XmlUtil::GetNodeText(node);
    nudf::string::Split<wchar_t>(text, L',', rules);
    for(std::vector<int>::iterator it=rules.begin(); it!=rules.end(); ++it) {
        _rules.push_back(std::pair<unsigned int, CRule*>((unsigned int)(*it), NULL));
    }
}

void CCondition::ToXml(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* parent, _In_opt_ IXMLDOMNode** node)
{    
    if(_type >= RULE_MAX) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    
    try {

        HRESULT hr = S_OK;
        CComPtr<IXMLDOMElement> spCond;
        CComPtr<IXMLDOMNode> cond = NULL;
        std::wstring text;
        std::wstring type = (RULE_USER==_type) ? L"usr" : ((RULE_APPLICATION==_type) ? L"app" : ((RULE_LOCATION==_type) ? L"loc" : L"res"));
        std::wstring exclude = _exclude ? L"true" : L"false";

        if(RULE_USER == _type) {
            type = L"usr";
        }
        else if(RULE_APPLICATION == _type) {
            type = L"app";
        }
        else if(RULE_LOCATION == _type) {
            type = L"loc";
        }
        else if(RULE_RESOURCE == _type) {
            type = L"res";
        }
        else if(RULE_ENV == _type) {
            type = L"env";
        }
        else {
            assert(FALSE);
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        nudf::util::XmlUtil::CreateElement(doc, CONDITION_NODENAME, &spCond);
        nudf::util::XmlUtil::SetNodeAttribute(doc, spCond, CONDITION_TYPE_ATTRIBUTE, type);
        nudf::util::XmlUtil::SetNodeAttribute(doc, spCond, CONDITION_EXCLUDE_ATTRIBUTE, exclude);

        for(RULEOBJGROUP::iterator it=_rules.begin(); it!=_rules.end(); ++it) {
            std::wstring value;
            value = nudf::string::FromInt<wchar_t>((int)(*it).first);
            if(value.empty()) {
                continue;
            }
            if(!text.empty()) {text += L",";}
            text += value;
        }
        nudf::util::XmlUtil::SetNodeText(spCond, text);
        hr = parent->appendChild(spCond, &cond);
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }
        if(NULL != node) {
            *node = cond.Detach();
        }
    }
    catch(const nudf::CException& e) {
        throw e;
    }
}


//
//  class CPolicy
//
typedef std::pair<std::set<unsigned int>, std::set<unsigned int>> POLICYIDGROUP;

CPolicy::CPolicy() :
    _id((unsigned int)-1),
    _usergroup((unsigned int)-1),
    _rights(0)
{
}

CPolicy::CPolicy(unsigned int id, const wchar_t* name, unsigned int usergroup, unsigned __int64 rights) :
    _id(id),
    _usergroup(usergroup),
    _rights(rights),
    _name(name)
{
}

CPolicy::~CPolicy()
{
}

void CPolicy::SetRights(_In_ const std::wstring& actions)
{
    std::vector<std::wstring> vActions;
    
    _rights = 0;

    nudf::string::Split<wchar_t>(actions, L',', vActions);
    for(std::vector<std::wstring>::const_iterator it=vActions.begin(); it!=vActions.end(); ++it) {
        _rights += ActionToRights((*it).c_str());
    }
}

void CPolicy::GetRightActions(_Out_ std::wstring& actions)
{
    if(BUILTIN_RIGHT_VIEW == (BUILTIN_RIGHT_VIEW & _rights)) {
        if(!actions.empty()) actions += L",";
        actions += RIGHT_ACTION_VIEW;
    }
    if(BUILTIN_RIGHT_EDIT == (BUILTIN_RIGHT_EDIT & _rights)) {
        if(!actions.empty()) actions += L",";
        actions += RIGHT_ACTION_EDIT;
    }
    if(BUILTIN_RIGHT_PRINT == (BUILTIN_RIGHT_PRINT & _rights)) {
        if(!actions.empty()) actions += L",";
        actions += RIGHT_ACTION_PRINT;
    }
    if(BUILTIN_RIGHT_CLIPBOARD == (BUILTIN_RIGHT_CLIPBOARD & _rights)) {
        if(!actions.empty()) actions += L",";
        actions += RIGHT_ACTION_CLIPBOARD;
    }
    if(BUILTIN_RIGHT_SAVEAS == (BUILTIN_RIGHT_SAVEAS & _rights)) {
        if(!actions.empty()) actions += L",";
        actions += RIGHT_ACTION_SAVEAS;
    }
    if(BUILTIN_RIGHT_DECRYPT == (BUILTIN_RIGHT_DECRYPT & _rights)) {
        if(!actions.empty()) actions += L",";
        actions += RIGHT_ACTION_DECRYPT;
    }
    if(BUILTIN_RIGHT_SCREENCAP == (BUILTIN_RIGHT_SCREENCAP & _rights)) {
        if(!actions.empty()) actions += L",";
        actions += RIGHT_ACTION_SCREENCAP;
    }
    if(BUILTIN_RIGHT_SEND == (BUILTIN_RIGHT_SEND & _rights)) {
        if(!actions.empty()) actions += L",";
        actions += RIGHT_ACTION_SEND;
    }
    if(BUILTIN_RIGHT_CLASSIFY == (BUILTIN_RIGHT_CLASSIFY & _rights)) {
        if(!actions.empty()) actions += L",";
        actions += RIGHT_ACTION_CLASSIFY;
    }
}

void CPolicy::GetRightDisplayNames(_Out_ std::wstring& names)
{
    if(BUILTIN_RIGHT_VIEW == (BUILTIN_RIGHT_VIEW & _rights)) {
        if(!names.empty()) names += L",";
        names += RIGHT_DISP_VIEW;
    }
    if(BUILTIN_RIGHT_EDIT == (BUILTIN_RIGHT_EDIT & _rights)) {
        if(!names.empty()) names += L",";
        names += RIGHT_DISP_EDIT;
    }
    if(BUILTIN_RIGHT_PRINT == (BUILTIN_RIGHT_PRINT & _rights)) {
        if(!names.empty()) names += L",";
        names += RIGHT_DISP_PRINT;
    }
    if(BUILTIN_RIGHT_CLIPBOARD == (BUILTIN_RIGHT_CLIPBOARD & _rights)) {
        if(!names.empty()) names += L",";
        names += RIGHT_DISP_CLIPBOARD;
    }
    if(BUILTIN_RIGHT_SAVEAS == (BUILTIN_RIGHT_SAVEAS & _rights)) {
        if(!names.empty()) names += L",";
        names += RIGHT_DISP_SAVEAS;
    }
    if(BUILTIN_RIGHT_DECRYPT == (BUILTIN_RIGHT_DECRYPT & _rights)) {
        if(!names.empty()) names += L",";
        names += RIGHT_DISP_DECRYPT;
    }
    if(BUILTIN_RIGHT_SCREENCAP == (BUILTIN_RIGHT_SCREENCAP & _rights)) {
        if(!names.empty()) names += L",";
        names += RIGHT_DISP_SCREENCAP;
    }
    if(BUILTIN_RIGHT_SEND == (BUILTIN_RIGHT_SEND & _rights)) {
        if(!names.empty()) names += L",";
        names += RIGHT_DISP_SEND;
    }
    if(BUILTIN_RIGHT_CLASSIFY == (BUILTIN_RIGHT_CLASSIFY & _rights)) {
        if(!names.empty()) names += L",";
        names += RIGHT_DISP_CLASSIFY;
    }
}

void CPolicy::FromXml(IXMLDOMNode* node)
{
    HRESULT      hr = S_OK;
    std::wstring root_name;
    CComPtr<IXMLDOMNodeList> spNodeList;
    long count = 0;
    POLICYIDGROUP idgroup[RULE_MAX];

    if(NODE_ELEMENT != nudf::util::XmlUtil::GetNodeType(node)) {
        throw WIN32ERROR2(ERROR_DATATYPE_MISMATCH);
    }

    root_name = nudf::util::XmlUtil::GetNodeName(node);
    if(0 != _wcsicmp(root_name.c_str(), POLICY_NODENAME)) {
        throw WIN32ERROR2(ERROR_INVALID_NAME);
    }
    
    if(!nudf::util::XmlUtil::GetNodeAttribute(node, POLICY_ID_ATTRIBUTE, (int*)(&_id))) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    
    if(!nudf::util::XmlUtil::GetNodeAttribute(node, POLICY_NAME_ATTRIBUTE, _name)) {
        _name = POLICY_DEFAULT_NAME;
    }
    
    if(!nudf::util::XmlUtil::GetNodeAttribute(node, POLICY_USERGROUP_ATTRIBUTE, (int*)(&_usergroup))) {
        _usergroup = (UINT)-1;
    }
    
    std::wstring actions;
    if(!nudf::util::XmlUtil::GetNodeAttribute(node, POLICY_RIGHTS_ATTRIBUTE, actions)) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    SetRights(actions);
    
    hr = node->get_childNodes(&spNodeList);
    if(FAILED(hr) || spNodeList==NULL) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }

    hr = spNodeList->get_length(&count);
    if(FAILED(hr)) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }

    // Get all the conditions and obligations
    for(long i=0; i<count; i++) {

        CComPtr<IXMLDOMNode> spNode;
        std::wstring node_name;

        hr = spNodeList->get_item(i, &spNode);
        if(FAILED(hr) || spNode==NULL) {
            break;
        }
        if(NODE_ELEMENT != nudf::util::XmlUtil::GetNodeType(spNode)) {
            continue;
        }
        node_name = nudf::util::XmlUtil::GetNodeName(spNode);
        if(0 == _wcsicmp(node_name.c_str(), CONDITION_NODENAME)) {
            CCondition cond;
            cond.FromXml(spNode);
            assert(cond.GetType() < RULE_MAX);
            _conds[cond.GetType()].push_back(cond);
        }
        else if(0 == _wcsicmp(node_name.c_str(), OB_NODENAME)) {
            CPolicyObligation ob;
            ob.FromXml(spNode);
            if(ob.IsValid()) {
                _obs.GetObligations()[ob.GetId()] = ob;
            }
        }
        else {
            continue;
        }
    }
}

bool CPolicy::Match(_In_ RULETYPE type, _In_ const PROPERTYMAP& props) const throw()
{
    if(type >= RULE_MAX) {
        assert(false);
        return false;
    }

    for(int i=0; i<(int)_conds[type].size(); i++) {
        if(!_conds[type][i].Match(type, props)) {
            return false;
        }
    }

    // Not match any rule
    return true;
}

bool CPolicy::MatchUser(_In_ const PROPERTYMAP& props) const throw()
{
    return Match(RULE_USER, props);
}

bool CPolicy::MatchApplication(_In_ const PROPERTYMAP& props) const throw()
{
    return Match(RULE_APPLICATION, props);
}

bool CPolicy::MatchLocation(_In_ const PROPERTYMAP& props) const throw()
{
    return Match(RULE_LOCATION, props);
}

bool CPolicy::MatchResource(_In_ const PROPERTYMAP& props) const throw()
{
    return Match(RULE_RESOURCE, props);
}

bool CPolicy::MatchEnvironment(_In_ const PROPERTYMAP& props) const throw()
{
    return Match(RULE_ENV, props);
}