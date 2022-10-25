

#include <Windows.h>
#include <atlbase.h>
#include <atlcomcli.h>

#include <string>

#include <nudf\exception.hpp>
#include <nudf\string.hpp>
#include <nudf\xmlparser.hpp>
#include <nudf\shared\rightsdef.h>

#include "profile_policy.hpp"


using namespace nxrm::pkg;
using namespace nxrm::pkg::policy;

#define POLICYSET_NODENAME      L"POLICYSET"
#define USERGROUPMAP_NODENAME   L"USERGROUPMAP"
#define RIGHTSET_NODENAME       L"RIGHTSET"
#define COMPONENTS_NODENAME     L"COMPONENTS"
#define USERSET_NODENAME        L"USERS"
#define APPLICATIONSET_NODENAME L"APPLICATIONS"
#define LOCATIONSET_NODENAME    L"LOCATIONS"
#define RESOURCESET_NODENAME    L"RESOURCES"
#define ENVSET_NODENAME         L"ENVS"




//
//  class CPolicyProfile
//
CPolicyProfile::CPolicyProfile() : CProfile(), _regexOn(false)
{
}

CPolicyProfile::~CPolicyProfile()
{
}

CRule* CPolicyProfile::FindRuleById(_In_ RULETYPE type, _In_ unsigned int id) throw()
{
    for(RULEGROUP::iterator it=_rules[type].begin(); it!=_rules[type].end(); ++it) {
        if((*it)->GetId() == id) {
            return (*it).get();
        }
    }
    return NULL;
}

void CPolicyProfile::FromXml(const std::wstring& xml)
{
    nudf::util::CXmlDocument doc;

    try {

        CComPtr<IXMLDOMElement> spRoot;

        doc.LoadFromXml(xml.c_str());
        if(!doc.GetDocRoot(&spRoot)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        FromXml(spRoot);
    }
    catch(const nudf::CException& e) {
        throw e;
    }
}

void CPolicyProfile::FromXml(IXMLDOMNode* node)
{
    HRESULT      hr = S_OK;
    std::wstring root_name;
    std::wstring profile_id;
    std::wstring profile_name;
    std::wstring profile_stimestamp;
    FILETIME     profile_timestamp;
    CComPtr<IXMLDOMNode>    spPolicyNode;
    CComPtr<IXMLDOMNode>    spUserGroupNode;
    CComPtr<IXMLDOMNode>    spCompNode;
    CComPtr<IXMLDOMNode>    spUserRuleNode;
    CComPtr<IXMLDOMNode>    spAppRuleNode;
    CComPtr<IXMLDOMNode>    spLocRuleNode;
    CComPtr<IXMLDOMNode>    spResRuleNode;
    CComPtr<IXMLDOMNode>    spEnvRuleNode;

    if(NODE_ELEMENT != nudf::util::XmlUtil::GetNodeType(node)) {
        throw WIN32ERROR2(ERROR_DATATYPE_MISMATCH);
    }

    root_name = nudf::util::XmlUtil::GetNodeName(node);
    if(0 != _wcsicmp(root_name.c_str(), PROFILE_POLICY_NODENAME)) {
        throw WIN32ERROR2(ERROR_INVALID_NAME);
    }
    
    // Make sure it is a profile
    if(!nudf::util::XmlUtil::GetNodeAttribute(node, PROFILE_ATTRIBUTE_TIMESTAMP, profile_stimestamp) || profile_stimestamp.empty()) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    if(!nudf::util::XmlUtil::GetNodeAttribute(node, PROFILE_ATTRIBUTE_TIMESTAMP, &profile_timestamp)) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }


    // Make sure all the sub-nodes exist
    if(!nudf::util::XmlUtil::FindChildElement(node, POLICYSET_NODENAME, &spPolicyNode) || NULL==spPolicyNode) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    if(!nudf::util::XmlUtil::FindChildElement(node, USERGROUPMAP_NODENAME, &spUserGroupNode) || NULL==spUserGroupNode) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    if(!nudf::util::XmlUtil::FindChildElement(node, COMPONENTS_NODENAME, &spCompNode) || NULL==spCompNode) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    if(!nudf::util::XmlUtil::FindChildElement(spCompNode, USERSET_NODENAME, &spUserRuleNode) || NULL==spUserRuleNode) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    if(!nudf::util::XmlUtil::FindChildElement(spCompNode, APPLICATIONSET_NODENAME, &spAppRuleNode) || NULL==spAppRuleNode) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    if(!nudf::util::XmlUtil::FindChildElement(spCompNode, LOCATIONSET_NODENAME, &spLocRuleNode) || NULL==spLocRuleNode) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    if(!nudf::util::XmlUtil::FindChildElement(spCompNode, RESOURCESET_NODENAME, &spResRuleNode) || NULL==spResRuleNode) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    if(!nudf::util::XmlUtil::FindChildElement(spCompNode, ENVSET_NODENAME, &spEnvRuleNode) || NULL==spEnvRuleNode) {
        ; // Do Nothing
    }

    this->Clear();
    SetTimestamp(profile_timestamp);
    SetStrTimestamp(profile_stimestamp);

    //
    // Load
    //
    
    // User Group Map
    LoadUserGroupMap(spUserGroupNode);
    // User Rule Collection
    LoadUserSet(spUserRuleNode);
    // Application Rule Collection
    LoadAppSet(spAppRuleNode);
    // Location Rule Collection
    LoadLocSet(spLocRuleNode);
    // Resource Rule Collection
    LoadResSet(spResRuleNode);
    // Environment Rule Collection
    if(NULL != spEnvRuleNode) {
        LoadEnvSet(spEnvRuleNode);
    }
    // Policy Collection
    LoadPolicySet(spPolicyNode);

    ToRegex();
}

void CPolicyProfile::ToRegex()
{
    // Convert pattern string to regex string
    for(int i=0; i<RULE_MAX; i++) {
        for(RULEGROUP::iterator it=(GetRulesSet()[i]).begin(); it!=(GetRulesSet()[i]).end(); ++it) {
            std::vector<CPattern>& patterns = (*it)->GetPatterns();
            for(std::vector<CPattern>::iterator itp=patterns.begin(); itp!=patterns.end(); ++itp) {
                (*itp).ToRegex();
            }
        }
    }
}

void CPolicyProfile::Clear() throw()
{
    CProfile::Clear();
    _policyset.clear();
    _usermap.clear();
    for(int i=0; i<RULE_MAX; i++) {
        _rules[i].clear();
    }
}

bool CPolicyProfile::GetGroupIdSet(_In_ const std::wstring& id, _In_ const std::wstring& tenantid, _Out_  std::set<unsigned int>& groups, _Out_ std::wstring& context) const throw()
 {
     if(0 == _usermap.size()) {
         return false;
     }

     std::shared_ptr<CUserGroups> spUnknownGroup;

     for(USERGROUP::const_iterator it=_usermap.begin(); it!=_usermap.end(); ++it) {
         if(id == (*it)->GetId() && tenantid == (*it)->GetTenantId()) {
             groups = (*it)->GetGroups();
             context = (*it)->GetContext();
             return true;
         }
         if((*it)->IsUnknownUserGroups()) {
             spUnknownGroup = (*it);
         }
     }

     // Not found
     if(spUnknownGroup.get() != NULL) {
         groups = spUnknownGroup->GetGroups();
         context = spUnknownGroup->GetContext();
     }
     return false;
 }


void CPolicyProfile::LoadPolicySet(IXMLDOMNode* node)
{
    HRESULT hr = S_OK;
    CComPtr<IXMLDOMNodeList>    spList;
    long count = 0;
    
    std::wstring root_name = nudf::util::XmlUtil::GetNodeName(node);
    if(0 != _wcsicmp(root_name.c_str(), POLICYSET_NODENAME)) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    
    hr = node->get_childNodes(&spList);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    hr = spList->get_length(&count);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    for(long i=0; i<count; i++) {

        CComPtr<IXMLDOMNode>    spNode;
        std::wstring            node_name;

        hr = spList->get_item(i, &spNode);
        if(FAILED(hr)) {
            break;
        }

        if(NODE_ELEMENT != nudf::util::XmlUtil::GetNodeType(spNode)) {
            continue;
        }
        node_name = nudf::util::XmlUtil::GetNodeName(spNode);
        if(0 != _wcsicmp(node_name.c_str(), POLICY_NODENAME)) {
            continue;
        }

        std::shared_ptr<CPolicy> policy(new CPolicy);        
        if(policy == NULL) {
            throw WIN32ERROR2(ERROR_OUTOFMEMORY);
        }
        policy->FromXml(spNode);

        CONDITIONS* conds = policy->GetConditions();
        for(int i=0; i<RULE_MAX; i++) {
            for(int j=0; j<(int)conds[i].size(); j++) {
                for(RULEOBJGROUP::iterator it=conds[i][j].GetRules().begin(); it!=conds[i][j].GetRules().end(); ++it) {
                    (*it).second = FindRuleById((RULETYPE)i, (*it).first);
                    if(NULL == (*it).second) {
                        (*it).first = (UINT)-1;
                    }
                }
                conds[i][j].GetRules().remove_if([](const RULEOBJ& rule){return (((UINT)-1) == rule.first);});
            }
        }

        _policyset.push_back(policy);
    }
}

void CPolicyProfile::LoadUserGroupMap(IXMLDOMNode* node)
{
    HRESULT hr = S_OK;
    CComPtr<IXMLDOMNodeList>    spList;
    long count = 0;
    
    static const std::wstring wsSidPrefix(L"S-1-5");
    
    std::wstring root_name = nudf::util::XmlUtil::GetNodeName(node);
    if(0 != _wcsicmp(root_name.c_str(), USERGROUPMAP_NODENAME)) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    
    hr = node->get_childNodes(&spList);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    hr = spList->get_length(&count);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    for(long i=0; i<count; i++) {

        CComPtr<IXMLDOMNode>    spNode;
        std::wstring            node_name;
        std::wstring            node_id;

        hr = spList->get_item(i, &spNode);
        if(FAILED(hr)) {
            break;
        }

        if(NODE_ELEMENT != nudf::util::XmlUtil::GetNodeType(spNode)) {
            continue;
        }
        node_name = nudf::util::XmlUtil::GetNodeName(spNode);
        if(0 != _wcsicmp(node_name.c_str(), USER_NODENAME)) {
            continue;
        }

        if(!nudf::util::XmlUtil::GetNodeAttribute(spNode, USER_ID_ATTRIBUTE, node_id) || node_id.length() == 0) {
            continue;
        }
        if(0 != _wcsicmp(node_id.c_str(), UNKNOWN_USER_GROUP_NAME)) {
            if(0 != _wcsnicmp(node_id.c_str(), wsSidPrefix.c_str(), wsSidPrefix.length())) {
                // NOT a SID
                continue;
            }
        }

        std::shared_ptr<CUserGroups> user(new CUserGroups);        
        if(user == NULL) {
            throw WIN32ERROR2(ERROR_OUTOFMEMORY);
        }
        user->FromXml(spNode);
        if(user->IsUnknownUserGroups()) {
            _usermap.insert(_usermap.begin(), user);    // unknown map must be first one
        }
        else {
            _usermap.push_back(user);
        }
    }
}

void CPolicyProfile::LoadUserSet(IXMLDOMNode* node)
{
    HRESULT hr = S_OK;
    CComPtr<IXMLDOMNodeList>    spList;
    long count = 0;
    
    std::wstring root_name = nudf::util::XmlUtil::GetNodeName(node);
    if(0 != _wcsicmp(root_name.c_str(), USERSET_NODENAME)) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    
    hr = node->get_childNodes(&spList);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    hr = spList->get_length(&count);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    for(long i=0; i<count; i++) {

        CComPtr<IXMLDOMNode>    spNode;
        std::wstring            node_name;

        hr = spList->get_item(i, &spNode);
        if(FAILED(hr)) {
            break;
        }

        if(NODE_ELEMENT != nudf::util::XmlUtil::GetNodeType(spNode)) {
            continue;
        }
        node_name = nudf::util::XmlUtil::GetNodeName(spNode);
        if(0 != _wcsicmp(node_name.c_str(), RULE_USR_NODENAME)) {
            continue;
        }

        std::shared_ptr<CRule> rule(new CRule);        
        if(rule == NULL) {
            throw WIN32ERROR2(ERROR_OUTOFMEMORY);
        }
        rule->FromXml(spNode);
        _rules[RULE_USER].push_back(rule);
    }
}

void CPolicyProfile::LoadAppSet(IXMLDOMNode* node)
{
    HRESULT hr = S_OK;
    CComPtr<IXMLDOMNodeList>    spList;
    long count = 0;
    
    std::wstring root_name = nudf::util::XmlUtil::GetNodeName(node);
    if(0 != _wcsicmp(root_name.c_str(), APPLICATIONSET_NODENAME)) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    
    hr = node->get_childNodes(&spList);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    hr = spList->get_length(&count);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    for(long i=0; i<count; i++) {

        CComPtr<IXMLDOMNode>    spNode;
        std::wstring            node_name;

        hr = spList->get_item(i, &spNode);
        if(FAILED(hr)) {
            break;
        }

        if(NODE_ELEMENT != nudf::util::XmlUtil::GetNodeType(spNode)) {
            continue;
        }
        node_name = nudf::util::XmlUtil::GetNodeName(spNode);
        if(0 != _wcsicmp(node_name.c_str(), RULE_APP_NODENAME)) {
            continue;
        }

        std::shared_ptr<CRule> rule(new CRule);        
        if(rule == NULL) {
            throw WIN32ERROR2(ERROR_OUTOFMEMORY);
        }
        rule->FromXml(spNode);
        _rules[RULE_APPLICATION].push_back(rule);
    }
}

void CPolicyProfile::LoadLocSet(IXMLDOMNode* node)
{
    HRESULT hr = S_OK;
    CComPtr<IXMLDOMNodeList>    spList;
    long count = 0;
    
    std::wstring root_name = nudf::util::XmlUtil::GetNodeName(node);
    if(0 != _wcsicmp(root_name.c_str(), LOCATIONSET_NODENAME)) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }

    hr = node->get_childNodes(&spList);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    hr = spList->get_length(&count);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    for(long i=0; i<count; i++) {

        CComPtr<IXMLDOMNode>    spNode;
        std::wstring            node_name;

        hr = spList->get_item(i, &spNode);
        if(FAILED(hr)) {
            break;
        }

        if(NODE_ELEMENT != nudf::util::XmlUtil::GetNodeType(spNode)) {
            continue;
        }
        node_name = nudf::util::XmlUtil::GetNodeName(spNode);
        if(0 != _wcsicmp(node_name.c_str(), RULE_LOC_NODENAME)) {
            continue;
        }

        std::shared_ptr<CRule> rule(new CRule);        
        if(rule == NULL) {
            throw WIN32ERROR2(ERROR_OUTOFMEMORY);
        }
        rule->FromXml(spNode);
        _rules[RULE_LOCATION].push_back(rule);
    }
}

void CPolicyProfile::LoadResSet(IXMLDOMNode* node)
{
    HRESULT hr = S_OK;
    CComPtr<IXMLDOMNodeList>    spList;
    long count = 0;
    
    std::wstring root_name = nudf::util::XmlUtil::GetNodeName(node);
    if(0 != _wcsicmp(root_name.c_str(), RESOURCESET_NODENAME)) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    
    hr = node->get_childNodes(&spList);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    hr = spList->get_length(&count);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    for(long i=0; i<count; i++) {

        CComPtr<IXMLDOMNode>    spNode;
        std::wstring            node_name;

        hr = spList->get_item(i, &spNode);
        if(FAILED(hr)) {
            break;
        }

        if(NODE_ELEMENT != nudf::util::XmlUtil::GetNodeType(spNode)) {
            continue;
        }
        node_name = nudf::util::XmlUtil::GetNodeName(spNode);
        if(0 != _wcsicmp(node_name.c_str(), RULE_RES_NODENAME)) {
            continue;
        }

        std::shared_ptr<CRule> rule(new CRule);        
        if(rule == NULL) {
            throw WIN32ERROR2(ERROR_OUTOFMEMORY);
        }
        rule->FromXml(spNode);
        _rules[RULE_RESOURCE].push_back(rule);
    }
}

void CPolicyProfile::LoadEnvSet(IXMLDOMNode* node)
{
    HRESULT hr = S_OK;
    CComPtr<IXMLDOMNodeList>    spList;
    long count = 0;
    
    std::wstring root_name = nudf::util::XmlUtil::GetNodeName(node);
    if(0 != _wcsicmp(root_name.c_str(), ENVSET_NODENAME)) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    
    hr = node->get_childNodes(&spList);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    hr = spList->get_length(&count);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    for(long i=0; i<count; i++) {

        CComPtr<IXMLDOMNode>    spNode;
        std::wstring            node_name;

        hr = spList->get_item(i, &spNode);
        if(FAILED(hr)) {
            break;
        }

        if(NODE_ELEMENT != nudf::util::XmlUtil::GetNodeType(spNode)) {
            continue;
        }
        node_name = nudf::util::XmlUtil::GetNodeName(spNode);
        if(0 != _wcsicmp(node_name.c_str(), RULE_ENV_NODENAME)) {
            continue;
        }

        std::shared_ptr<CRule> rule(new CRule);        
        if(rule == NULL) {
            throw WIN32ERROR2(ERROR_OUTOFMEMORY);
        }
        rule->FromXml(spNode);
        _rules[RULE_ENV].push_back(rule);
    }
}


//
//  class CPolicySet
//

void CPolicySet::Evaluate(_Inout_ CEvalObject& object, _Out_ CEvalResult& result) throw()
{
    bool onEvalUserExecuted = false;
    bool onEvalAppExecuted = false;
    bool onEvalLocExecuted = false;
    bool onEvalResExecuted = false;
    bool isExplorer = false;

    PROPERTYMAP::const_iterator it_path = object.GetProperty(nxrm::pkg::policy::RULE_APPLICATION).find(L"path");
    if(it_path!=object.GetProperty(nxrm::pkg::policy::RULE_APPLICATION).end()) {
        std::wstring path = (*it_path).second.GetValue();
        const wchar_t* appname = wcsrchr(path.c_str(), L'\\');
        if(NULL != appname && 0 == _wcsicmp(appname, L"\\explorer.exe")) {
            isExplorer = true;
        }
    }

    for(std::list<CPolicy*>::iterator it=_policyset.begin(); it!=_policyset.end(); ++it) {

        // User Evaluation Mask is Set
        if(0 != (_mask & EVALUSER)) {
            if(!onEvalUserExecuted) {
                OnPreEvalUser(object.GetProperty(RULE_USER));
                onEvalUserExecuted = true;
            }
            // Check User Group
            if(!object.MatchGroup((*it)->GetUserGroup())) {
                continue;
            }
            if(!(*it)->MatchUser(object.GetProperty(RULE_USER))) {
                continue;
            }
        }

        // Application Evaluation Mask is Set
        if(!isExplorer && 0 != (_mask & EVALAPP)) {
            if(!onEvalAppExecuted) {
                OnPreEvalApplication(object.GetProperty(RULE_APPLICATION));
                onEvalAppExecuted = true;
            }

            if(!(*it)->MatchApplication(object.GetProperty(RULE_APPLICATION))) {
                continue;
            }
        }

        // Location Evaluation Mask is Set
        if(0 != (_mask & EVALLOC)) {
            if(!onEvalLocExecuted) {
                OnPreEvalLocation(object.GetProperty(RULE_LOCATION));
                onEvalLocExecuted = true;
            }
            if(!(*it)->MatchLocation(object.GetProperty(RULE_LOCATION))) {
                continue;
            }
        }        

        // Check Environment Rules
        if(!(*it)->MatchEnvironment(object.GetProperty(RULE_ENV))) {
            continue;
        }

        // Resource Evaluation Mask is Set
        if(0 != (_mask & EVALRES)) {
            if(!onEvalResExecuted) {
                if(!OnPreEvalResource(object.GetProperty(RULE_RESOURCE))) {
                    result.SetNotCacheFlag();
                }
                onEvalResExecuted = true;
            }
            if(!(*it)->MatchResource(object.GetProperty(RULE_RESOURCE))) {
                continue;
            }
        }

        result.AddPolicy(*it);
    }

    if(0 == (result.GetRights() & BUILTIN_RIGHT_VIEW)) {
        result.Clear();
    }
}

CPolicySet& CPolicySet::operator = (const CPolicySet& policyset) throw()
{
    if(this != &policyset) {
        _mask = policyset.GetMask();
        _policyset = policyset.GetPolicySet();
    }
    return *this;
}

//
//  class CEvalResult
//
void CEvalResult::GenerateUid() throw()
{
    static volatile LONGLONG NextUniqueId = 1;
    if(NextUniqueId >= 0xFFFFFFFFFFFFFFF) {
        _uid = (ULONGLONG)InterlockedExchange64(&NextUniqueId, 1);
    }
    else {
        _uid = InterlockedCompareExchange64(&NextUniqueId, NextUniqueId+1, NextUniqueId);
    }
}