
#include <Windows.h>
#include <atlbase.h>
#include <atlcomcli.h>


#include <string>

#include <nudf\exception.hpp>
#include <nudf\string.hpp>
#include <nudf\xmlparser.hpp>

#include "profile_comm.hpp"



using namespace nxrm::pkg;



#define PROFILE_COMM_NODENAME_SERVHOST      L"SERVER_HOST"
#define PROFILE_COMM_NODENAME_SERVPORT      L"SERVER_PORT"
#define PROFILE_COMM_NODENAME_SERVPATH      L"SERVER_PATH"
#define PROFILE_COMM_NODENAME_HEARTBEAT     L"HEARTBEAT_INTERVAL"
#define PROFILE_COMM_NODENAME_LOG           L"LOG_INTERVAL"
#define PROFILE_COMM_NODENAME_LOGSIZE       L"LOG_SIZE"


CCommProfile::CCommProfile() : CProfile(), _servPort(0),_hbInterval(0), _logInterval(0), _logSize(0)
{
}

CCommProfile::~CCommProfile()
{
}

void CCommProfile::FromXml(IXMLDOMNode* node)
{
    HRESULT      hr = S_OK;
    std::wstring root_name;
    std::wstring node_text;
    std::wstring profile_id;
    std::wstring profile_name;
    std::wstring server_host;
    USHORT       server_port;
    std::wstring server_path;
    FILETIME     profile_timestamp;
    __int64      heartbeat_interval = 0;
    __int64      log_interval = 0;
    __int64      log_size = 0;
    CComPtr<IXMLDOMNode> spChildNode;

    if(NODE_ELEMENT != nudf::util::XmlUtil::GetNodeType(node)) {
        throw WIN32ERROR2(ERROR_DATATYPE_MISMATCH);
    }

    root_name = nudf::util::XmlUtil::GetNodeName(node);
    if(0 != _wcsicmp(root_name.c_str(), PROFILE_COMM_NODENAME)) {
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

    // Get Server Host
    if(!nudf::util::XmlUtil::FindChildElement(node, PROFILE_COMM_NODENAME_SERVHOST, &spChildNode)) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    server_host = nudf::util::XmlUtil::GetNodeText(spChildNode);
    if(server_host.empty()) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    spChildNode.Release();

    // Get Server Port
    if(!nudf::util::XmlUtil::FindChildElement(node, PROFILE_COMM_NODENAME_SERVPORT, &spChildNode)) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    node_text = nudf::util::XmlUtil::GetNodeText(spChildNode);
    if(node_text.empty()) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    spChildNode.Release();
    int v = 0;
    if(!nudf::string::ToInt<wchar_t>(node_text, &v)) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    server_port = (USHORT)v;

    // Get Server Path
    if(!nudf::util::XmlUtil::FindChildElement(node, PROFILE_COMM_NODENAME_SERVPATH, &spChildNode)) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    server_path = nudf::util::XmlUtil::GetNodeText(spChildNode);
    if(server_path.empty()) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    spChildNode.Release();

    // Get HeartBeat Interval
    if(!nudf::util::XmlUtil::FindChildElement(node, PROFILE_COMM_NODENAME_HEARTBEAT, &spChildNode)) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    node_text = nudf::util::XmlUtil::GetNodeText(spChildNode);
    if(node_text.empty()) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    spChildNode.Release();
    if(!nudf::string::ToInt64<wchar_t>(node_text, &heartbeat_interval)) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }

    // Get Log Interval
    if(!nudf::util::XmlUtil::FindChildElement(node, PROFILE_COMM_NODENAME_LOG, &spChildNode)) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    node_text = nudf::util::XmlUtil::GetNodeText(spChildNode);
    if(node_text.empty()) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    spChildNode.Release();
    if(!nudf::string::ToInt64<wchar_t>(node_text, &log_interval)) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }

    // Get Log Size
    if(!nudf::util::XmlUtil::FindChildElement(node, PROFILE_COMM_NODENAME_LOGSIZE, &spChildNode)) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    node_text = nudf::util::XmlUtil::GetNodeText(spChildNode);
    if(node_text.empty()) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    spChildNode.Release();
    if(!nudf::string::ToInt64<wchar_t>(node_text, &log_size)) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }

    this->SetId(profile_id);
    this->SetName(profile_name);
    this->SetTimestamp(profile_timestamp);
    this->SetServHost(server_host);
    this->SetServPath(server_path);
    this->SetServPort(server_port);
    this->SetHeartBeatInterval(heartbeat_interval);
    this->SetLogInterval(log_interval);
    this->SetLogSize(log_size);
}

void CCommProfile::ToXml(IXMLDOMDocument* doc, IXMLDOMNode* parent, IXMLDOMNode** node) const
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
    if(this->GetServHost().empty()) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    if(this->GetServPath().empty()) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    
    try {

        HRESULT hr = S_OK;
        CComPtr<IXMLDOMElement> pRoot = NULL;
        CComPtr<IXMLDOMNode> child = NULL;
        
        nudf::util::XmlUtil::CreateElement(doc, PROFILE_COMM_NODENAME, &pRoot);
        nudf::util::XmlUtil::SetNodeAttribute(doc, pRoot, PROFILE_ATTRIBUTE_ID, this->GetId());
        nudf::util::XmlUtil::SetNodeAttribute(doc, pRoot, PROFILE_ATTRIBUTE_NAME, this->GetName());
        nudf::util::XmlUtil::SetNodeAttribute(doc, pRoot, PROFILE_ATTRIBUTE_TIMESTAMP, GetTimestamp(), true);
        nudf::util::XmlUtil::AppendChildElement(doc, pRoot, PROFILE_COMM_NODENAME_SERVHOST, &child);
        nudf::util::XmlUtil::SetNodeText(child, this->GetServHost());
        child.Release();
        nudf::util::XmlUtil::AppendChildElement(doc, pRoot, PROFILE_COMM_NODENAME_SERVPATH, &child);
        nudf::util::XmlUtil::SetNodeText(child, this->GetServPath());
        child.Release();
        nudf::util::XmlUtil::AppendChildElement(doc, pRoot, PROFILE_COMM_NODENAME_SERVPORT, &child);
        nudf::util::XmlUtil::SetNodeText(child, nudf::string::FromInt<wchar_t>((int)this->GetServPort()));
        child.Release();
        nudf::util::XmlUtil::AppendChildElement(doc, pRoot, PROFILE_COMM_NODENAME_HEARTBEAT, &child);
        nudf::util::XmlUtil::SetNodeText(child, nudf::string::FromInt64<wchar_t>((__int64)this->GetHeartBeatInterval()));
        child.Release();
        nudf::util::XmlUtil::AppendChildElement(doc, pRoot, PROFILE_COMM_NODENAME_LOG, &child);
        nudf::util::XmlUtil::SetNodeText(child, nudf::string::FromInt64<wchar_t>((__int64)this->GetLogInterval()));
        child.Release();
        nudf::util::XmlUtil::AppendChildElement(doc, pRoot, PROFILE_COMM_NODENAME_LOGSIZE, &child);
        nudf::util::XmlUtil::SetNodeText(child, nudf::string::FromInt64<wchar_t>((__int64)this->GetLogSize()));
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