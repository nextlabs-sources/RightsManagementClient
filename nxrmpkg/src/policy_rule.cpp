


#include <Windows.h>
#include <atlbase.h>
#include <atlcomcli.h>

#include <string>

#include <nudf\exception.hpp>
#include <nudf\string.hpp>
#include <nudf\xmlparser.hpp>

#include "policy_rule.hpp"
#include "policy_property.hpp"


using namespace nxrm::pkg::policy;


//
//  class CRule
//
CRule::CRule() : _id(-1), _type(RULE_MAX)
{
}

CRule::CRule(_In_ unsigned int id, _In_ RULETYPE type, _In_ const std::vector<CPattern>& patterns, _In_opt_ const wchar_t* comment) : _id(id), _type(type), _patterns(patterns), _comment(comment?comment:L"")
{
}

CRule::~CRule()
{
}

CRule& CRule::operator = (const CRule& rule) throw()
{
    if(this != &rule) {
        _id = rule.GetId();
        _patterns = rule.GetPatterns();
        _comment = rule.GetComment();
    }
    return *this;
}

bool CRule::Match(const std::unordered_multimap<std::wstring, CProperty>& props)
{
    for(std::vector<CPattern>::const_iterator it=_patterns.begin(); it!=_patterns.end(); ++it) {
        // Check Patterns
        std::pair<PROPERTYMAP::const_iterator,PROPERTYMAP::const_iterator> range = props.equal_range((*it).GetName());
        bool bFindMatch = false;
        for(PROPERTYMAP::const_iterator itprop=range.first; itprop!=range.second; ++itprop) {
            // Match an INCLUDE rule? then match this policy
            if((*it).Match((*itprop).second)) {
                bFindMatch = true;
                break;
            }
        }
        if(!bFindMatch) {
            return false;
        }
    }

    // For all the patterns, we can find a match in the properties
    return true;
}

void CRule::FromXml(_In_ IXMLDOMNode* node)
{
    HRESULT      hr = S_OK;
    std::wstring root_name;
    CComPtr<IXMLDOMNodeList> spNodeList;
    long count = 0;

    if(NODE_ELEMENT != nudf::util::XmlUtil::GetNodeType(node)) {
        throw WIN32ERROR2(ERROR_DATATYPE_MISMATCH);
    }

    root_name = nudf::util::XmlUtil::GetNodeName(node);
    if(0 == _wcsicmp(root_name.c_str(), RULE_USR_NODENAME)) {
        _type = RULE_USER;
    }
    else if(0 == _wcsicmp(root_name.c_str(), RULE_APP_NODENAME)) {
        _type = RULE_APPLICATION;
    }
    else if(0 == _wcsicmp(root_name.c_str(), RULE_LOC_NODENAME)) {
        _type = RULE_LOCATION;
    }
    else if(0 == _wcsicmp(root_name.c_str(), RULE_RES_NODENAME)) {
        _type = RULE_RESOURCE;
    }
    else if(0 == _wcsicmp(root_name.c_str(), RULE_ENV_NODENAME)) {
        _type = RULE_ENV;
    }
    else {
        throw WIN32ERROR2(ERROR_INVALID_NAME);
    }
    
    if(!nudf::util::XmlUtil::GetNodeAttribute(node, RULE_ID_ATTRIBUTE, (int*)(&_id))) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    
    hr = node->get_childNodes(&spNodeList);
    if(FAILED(hr) || spNodeList==NULL) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }

    hr = spNodeList->get_length(&count);
    if(FAILED(hr)) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }

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
        if(0 != _wcsicmp(node_name.c_str(), PROP_NODENAME)) {
            continue;
        }

        try {
            CPattern pattern;
            pattern.FromXml(spNode);
            _patterns.push_back(pattern);
        }
        catch(const nudf::CException& e) {
            UNREFERENCED_PARAMETER(e); // NOTHING
        }
    }
}

void CRule::ToXml(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* parent, _In_opt_ IXMLDOMNode** node)
{
    std::wstring node_name;
    
    if(_id == (ULONG)-1) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    if(_patterns.empty()) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    if(_type == RULE_USER) {
        node_name = RULE_USR_NODENAME;
    }
    else if(_type == RULE_APPLICATION) {
        node_name = RULE_APP_NODENAME;
    }
    else if(_type == RULE_LOCATION) {
        node_name = RULE_LOC_NODENAME;
    }
    else if(_type == RULE_RESOURCE) {
        node_name = RULE_RES_NODENAME;
    }
    else if(_type == RULE_ENV) {
        node_name = RULE_ENV_NODENAME;
    }
    else {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    
    try {

        HRESULT hr = S_OK;
        CComPtr<IXMLDOMElement> pRoot = NULL;
        CComPtr<IXMLDOMNode> child = NULL;
        
        nudf::util::XmlUtil::CreateElement(doc, node_name.c_str(), &pRoot);
        nudf::util::XmlUtil::SetNodeAttribute(doc, pRoot, RULE_ID_ATTRIBUTE, (int)_id);

        for(std::vector<CPattern>::iterator it=_patterns.begin(); it!=_patterns.end(); ++it) {
            (*it).ToXml(doc, pRoot, NULL);
        }
        
        if(!_comment.empty()) {
            nudf::util::XmlUtil::AppendChildComment(doc, parent, _comment.c_str(), NULL);
        }

        hr = parent->appendChild(pRoot, node);
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }
        if(NULL != node) {
            *node = child.Detach();
        }    
    }
    catch(const nudf::CException& e) {
        throw e;
    }
}

void CRule::Clear() throw()
{
    _id = (unsigned int)-1;;
    _type = RULE_MAX;
    _comment = L"";
    _patterns.clear();
}