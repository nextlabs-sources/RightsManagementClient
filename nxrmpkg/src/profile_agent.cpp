
#include <Windows.h>
#include <atlbase.h>
#include <atlcomcli.h>


#include <string>

#include <nudf\exception.hpp>
#include <nudf\string.hpp>
#include <nudf\xmlparser.hpp>

#include "profile_agent.hpp"



using namespace nxrm::pkg;



#define PROFILE_AGENT_NODENAME_AGENTID      L"AGENT_ID"
#define PROFILE_AGENT_NODENAME_AGENTTYPE    L"AGENT_TYPE"

CAgentProfile::CAgentProfile() : CProfile()
{
}

CAgentProfile::~CAgentProfile()
{
}

void CAgentProfile::FromXml(IXMLDOMNode* node)
{
    HRESULT      hr = S_OK;
    FILETIME     profile_timestamp = {0, 0};
    std::wstring root_name;
    std::wstring profile_id;
    std::wstring profile_name;
    std::wstring agent_id;
    std::wstring agent_type;
    CComPtr<IXMLDOMNode> spChildNode;

    if(NODE_ELEMENT != nudf::util::XmlUtil::GetNodeType(node)) {
        throw WIN32ERROR2(ERROR_DATATYPE_MISMATCH);
    }

    root_name = nudf::util::XmlUtil::GetNodeName(node);
    if(0 != _wcsicmp(root_name.c_str(), PROFILE_AGENT_NODENAME)) {
        throw WIN32ERROR2(ERROR_INVALID_NAME);
    }

    this->Clear();

    // Make sure it is a profile
    if(!nudf::util::XmlUtil::GetNodeAttribute(node, PROFILE_ATTRIBUTE_ID, profile_id) || profile_id.empty()) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    if(!nudf::util::XmlUtil::GetNodeAttribute(node, PROFILE_ATTRIBUTE_NAME, profile_name) || profile_name.empty()) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    if(!nudf::util::XmlUtil::GetNodeAttribute(node, PROFILE_ATTRIBUTE_TIMESTAMP, &profile_timestamp)) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }

    // Get Agent Id
    if(!nudf::util::XmlUtil::FindChildElement(node, PROFILE_AGENT_NODENAME_AGENTID, &spChildNode)) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    agent_id = nudf::util::XmlUtil::GetNodeText(spChildNode);
    if(agent_id.empty()) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    spChildNode.Release();

    // Get Agent Type
    if(!nudf::util::XmlUtil::FindChildElement(node, PROFILE_AGENT_NODENAME_AGENTTYPE, &spChildNode)) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    agent_type = nudf::util::XmlUtil::GetNodeText(spChildNode);
    if(agent_type.empty()) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    spChildNode.Release();

    this->SetId(profile_id);
    this->SetName(profile_name);
    this->SetTimestamp(profile_timestamp);
    this->SetAgentId(agent_id);
    this->SetAgentType(agent_type);
}

void CAgentProfile::ToXml(IXMLDOMDocument* doc, IXMLDOMNode* parent, IXMLDOMNode** node) const
{
    std::wstring node_name;
    
    if(0 == GetTimestamp().dwHighDateTime && 0 == GetTimestamp().dwLowDateTime) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    if(this->GetId().empty()) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    if(this->GetName().empty()) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    if(this->GetAgentId().empty()) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    if(this->GetAgentType().empty()) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    
    try {

        HRESULT hr = S_OK;
        CComPtr<IXMLDOMElement> pRoot = NULL;
        CComPtr<IXMLDOMNode> child = NULL;
        
        nudf::util::XmlUtil::CreateElement(doc, PROFILE_AGENT_NODENAME, &pRoot);
        nudf::util::XmlUtil::SetNodeAttribute(doc, pRoot, PROFILE_ATTRIBUTE_ID, this->GetId());
        nudf::util::XmlUtil::SetNodeAttribute(doc, pRoot, PROFILE_ATTRIBUTE_NAME, this->GetName());
        nudf::util::XmlUtil::SetNodeAttribute(doc, pRoot, PROFILE_ATTRIBUTE_TIMESTAMP, GetTimestamp(), true);
        nudf::util::XmlUtil::AppendChildElement(doc, pRoot, PROFILE_AGENT_NODENAME_AGENTID, &child);
        nudf::util::XmlUtil::SetNodeText(child, this->GetAgentId());
        child.Release();
        nudf::util::XmlUtil::AppendChildElement(doc, pRoot, PROFILE_AGENT_NODENAME_AGENTTYPE, &child);
        nudf::util::XmlUtil::SetNodeText(child, this->GetAgentType());
        child.Release();

        hr = parent->appendChild(pRoot, node);
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        } 
    }
    catch(const nudf::CException& e) {
        throw e;
    }
}