


#include <Windows.h>
#include <atlbase.h>
#include <atlcomcli.h>

#include <string>

#include <nudf\exception.hpp>
#include <nudf\string.hpp>
#include <nudf\xmlparser.hpp>

#include "policy_user.hpp"



using namespace nxrm::pkg::policy;


CUserGroups::CUserGroups() : _unknown(false)
{
}

CUserGroups::CUserGroups(const wchar_t* id, const std::set<unsigned int>& groups) : _unknown(false)
{
    _id = id;
    _groups = groups;
    _unknown = (0 == _wcsicmp(_id.c_str(), UNKNOWN_USER_GROUP_NAME)) ? true : false;
}

CUserGroups::CUserGroups(const wchar_t* id, const wchar_t* tenantid, const wchar_t* context, const std::set<unsigned int>& groups) : _unknown(false)
{
    _id = id;
    _tenantid = tenantid?tenantid:L"";
    _context = context?context:L"";
    _groups = groups;
    _unknown = (0 == _wcsicmp(_id.c_str(), UNKNOWN_USER_GROUP_NAME)) ? true : false;
}

CUserGroups::~CUserGroups()
{
}

CUserGroups& CUserGroups::operator = (const CUserGroups& ug) throw()
{
    if(this != &ug) {
        _id = ug.GetId();
        _tenantid = ug.GetTenantId();
        _context = ug.GetContext();
        _groups = ug.GetGroups();
        _unknown = ug.IsUnknownUserGroups();
    }
    return *this;
}

void CUserGroups::FromXml(_In_ IXMLDOMNode* node)
{
    HRESULT      hr = S_OK;
    std::wstring root_name;
    std::wstring group_ids;
    std::vector<int> ids;

    if(NODE_ELEMENT != nudf::util::XmlUtil::GetNodeType(node)) {
        throw WIN32ERROR2(ERROR_DATATYPE_MISMATCH);
    }

    root_name = nudf::util::XmlUtil::GetNodeName(node);
    if(0 != _wcsicmp(root_name.c_str(), USER_NODENAME)) {
        throw WIN32ERROR2(ERROR_INVALID_NAME);
    }

    if(!nudf::util::XmlUtil::GetNodeAttribute(node, USER_ID_ATTRIBUTE, _id) || _id.length() == 0) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    _unknown = (0 == _wcsicmp(_id.c_str(), UNKNOWN_USER_GROUP_NAME)) ? true : false;

    if(!nudf::util::XmlUtil::GetNodeAttribute(node, USER_TENANTID_ATTRIBUTE, _tenantid) || _tenantid.length() == 0) {
        _tenantid = BUILTIN_NEXTLABS_TENANTID;
    }

    nudf::util::XmlUtil::GetNodeAttribute(node, USER_CONTEXT_ATTRIBUTE, _context);

    std::transform(_id.begin(), _id.end(), _id.begin(), tolower);
    std::transform(_tenantid.begin(), _tenantid.end(), _tenantid.begin(), tolower);
    std::transform(_context.begin(), _context.end(), _context.begin(), tolower);

    group_ids = nudf::util::XmlUtil::GetNodeText(node);
    if(group_ids.length() == 0) {
        return;
    }

    nudf::string::Split<wchar_t>(group_ids, L',', ids);
    _groups.clear();
    for(std::vector<int>::iterator it=ids.begin(); it!=ids.end(); ++it) {
        _groups.insert((unsigned int)(*it));
    }
}

void CUserGroups::ToXml(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* parent, _In_opt_ IXMLDOMNode** node)
{
    std::wstring text;

    if(_id.empty()) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }

    for(std::set<unsigned int>::iterator it=_groups.begin(); it!=_groups.end(); ++it) {
        std::wstring v = nudf::string::FromInt<wchar_t>((int)(*it));
        if(!v.empty()) {
            if(!text.empty()) text += L",";
            text += v;
        }
    }

    if(text.empty()) {
        text = L" ";
    }

    try {

        HRESULT hr = S_OK;
        CComPtr<IXMLDOMElement> pRoot;
        CComPtr<IXMLDOMNode> child;

        nudf::util::XmlUtil::CreateElement(doc, USER_NODENAME, &pRoot);

        nudf::util::XmlUtil::SetNodeAttribute(doc, pRoot, USER_ID_ATTRIBUTE, _id);
        nudf::util::XmlUtil::SetNodeAttribute(doc, pRoot, USER_TENANTID_ATTRIBUTE, _tenantid);
        if(!_context.empty()) {
            nudf::util::XmlUtil::SetNodeAttribute(doc, pRoot, USER_CONTEXT_ATTRIBUTE, _context);
        }
        nudf::util::XmlUtil::SetNodeText(pRoot, text);
        
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
