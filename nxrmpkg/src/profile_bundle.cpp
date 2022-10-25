

#include <Windows.h>
#include <atlbase.h>
#include <atlcomcli.h>

#include <string>

#include <nudf\exception.hpp>
#include <nudf\string.hpp>
#include <nudf\xmlparser.hpp>

#include "profile_bundle.hpp"


using namespace nxrm::pkg;
using namespace nxrm::pkg::policy;


#define PROFILE_ID_NODENAME                 L"PROFILE_ID"
#define PROFILE_CERT_NODENAME               L"PROFILE_CERT"
#define AGENT_ID_NODENAME                   L"AGENT_ID"
#define AGENT_TYPE_NODENAME                 L"AGENT_TYPE"

#define PROFILE_ISDEFAULT_ATTRIBUTE         L"default"
#define PROFILE_CREATETIME_ATTRIBUTE        L"create-time"
#define PROFILE_MODIFYTIME_ATTRIBUTE        L"last-modify-time"

#define AGENTPROFILE_NODENAME               L"AGENTPROFILE"
#define AGENTPROFILE_ID_NODENAME            L"ID"
#define AGENTPROFILE_NAME_NODENAME          L"NAME"

#define COMMPROFILE_NODENAME                L"COMMPROFILE"
#define COMMPROFILE_ID_NODENAME             L"ID"
#define COMMPROFILE_NAME_NODENAME           L"NAME"
#define COMMPROFILE_DABSLOCATION_NODENAME   L"DABS_LOCATION"
#define COMMPROFILE_HEARTBEAT_FREQ_NODENAME L"HEARTBEAT_INTERVAL"
#define COMMPROFILE_LOG_FREQ_NODENAME       L"LOG_INTERVAL"
#define COMMPROFILE_LOG_SIZE_NODENAME       L"LOG_SIZELIMIT"




//
//  class CBundlePackage
//

CProfilePackage::CProfilePackage() : CPackage()
{
}

CProfilePackage::~CProfilePackage()
{
    Close();
}

void CProfilePackage::LoadFromFile(const wchar_t* file)
{
    CPackage::LoadFromFile(file);
    LoadBundle();
}

void CProfilePackage::LoadFromXml(const wchar_t* xml)
{
    CPackage::LoadFromXml(xml);
    LoadBundle();
}

void CProfilePackage::Create(_In_ const wchar_t* issuer)
{
    CPackage::Create(issuer, PROFILEBUNDLE_DATA_TYPE);

    try {
        CComPtr<IXMLDOMNode> spComponents;
        AppendChildComment(_datanode, L"Profile Bundle Root", NULL);
        AppendChildElement(_datanode, PROFILEBUNDLE_NODENAME, &_bundle);
    }
    catch(const nudf::CException& e) {
        Close();
        throw e;
    }
}

void CProfilePackage::Close()
{
    _bundle.Release();
    CPackage::Close();
}

void CProfilePackage::LoadBundle()
{
    // Get bundle root
    if(!FindChildElement(_datanode, PROFILEBUNDLE_NODENAME, &_bundle)) {
        Close();
        throw WIN32ERROR2(ERROR_FILE_INVALID);
    }
}




//
//  class CProfileBundle
//
CProfileBundle::CProfileBundle() : CProfile()
{
    SetName(PROFILEBUNDLE_NODENAME);
}

CProfileBundle::~CProfileBundle()
{
}

void CProfileBundle::FromXml(IXMLDOMNode* node)
{
    HRESULT      hr = S_OK;
    std::wstring root_name;
    std::wstring profile_id;
    CComPtr<IXMLDOMNode>    spAgentNode;
    CComPtr<IXMLDOMNode>    spCommNode;
    //CComPtr<IXMLDOMNode>    spClassifyNode;
    CComPtr<IXMLDOMNode>    spKeyNode;
    CComPtr<IXMLDOMNode>    spPolicyNode;

    if(NODE_ELEMENT != nudf::util::XmlUtil::GetNodeType(node)) {
        throw WIN32ERROR2(ERROR_DATATYPE_MISMATCH);
    }

    root_name = nudf::util::XmlUtil::GetNodeName(node);
    if(0 != _wcsicmp(root_name.c_str(), PROFILEBUNDLE_NODENAME)) {
        throw WIN32ERROR2(ERROR_INVALID_NAME);
    }

    this->Clear();

    // Make sure it is a valid profile
    if(!nudf::util::XmlUtil::GetNodeAttribute(node, PROFILE_ATTRIBUTE_ID, profile_id) || profile_id.empty()) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }

    // Make sure all the sub profile nodes exist
    if(!nudf::util::XmlUtil::FindChildElement(node, PROFILE_AGENT_NODENAME, &spAgentNode)) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    if(!nudf::util::XmlUtil::FindChildElement(node, PROFILE_COMM_NODENAME, &spCommNode)) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    //if(!nudf::util::XmlUtil::FindChildElement(node, PROFILE_CLASSIFY_NODENAME, &spClassifyNode)) {
    //    throw WIN32ERROR2(ERROR_NOT_FOUND);
    //}
    if(!nudf::util::XmlUtil::FindChildElement(node, PROFILE_KEY_NODENAME, &spKeyNode)) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    if(!nudf::util::XmlUtil::FindChildElement(node, PROFILE_POLICY_NODENAME, &spPolicyNode)) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }

    // Load all sub-profiles
    this->SetId(profile_id);
    _agentProfile.FromXml(spAgentNode);
    _commProfile.FromXml(spCommNode);
    //_classifyProfile.FromXml(spClassifyNode);
    _keyProfile.FromXml(spKeyNode);
    _policyProfile.FromXml(spPolicyNode);
}

void CProfileBundle::ToXml(IXMLDOMDocument* doc, IXMLDOMNode* parent, IXMLDOMNode** node) const
{
    try {

        HRESULT hr = S_OK;
        CComPtr<IXMLDOMElement> pRoot = NULL;
        
        nudf::util::XmlUtil::CreateElement(doc, PROFILE_AGENT_NODENAME, &pRoot);
        nudf::util::XmlUtil::SetNodeAttribute(doc, pRoot, PROFILE_ATTRIBUTE_ID, this->GetId());
        _agentProfile.ToXml(doc, pRoot, NULL);
        _commProfile.ToXml(doc, pRoot, NULL);
        //_classifyProfile.ToXml(doc, pRoot, NULL);
        _keyProfile.ToXml(doc, pRoot, NULL);
        _policyProfile.ToXml(doc, pRoot, NULL);
        
        hr = parent->appendChild(pRoot, node);
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        } 
    }
    catch(const nudf::CException& e) {
        throw e;
    }
}

void CProfileBundle::FromFile(const std::wstring& file, nudf::crypto::CRsaPubKeyBlob& key)
{
    CProfilePackage pkg;
    pkg.LoadFromFile(file.c_str());
    FromXml(pkg.GetBundleRoot());
    pkg.VerifySignatrue(key);
}

void CProfileBundle::ToFile(const std::wstring& file, nudf::crypto::CLegacyRsaPriKeyBlob& key) const
{
    CProfilePackage pkg;
    pkg.Create(L"NextLabs, Inc.");
    pkg.AppendChildComment(pkg.GetBundleRoot(), L"Agent Profile", NULL);
    _agentProfile.ToXml(pkg.GetDoc(), pkg.GetBundleRoot(), NULL);
    pkg.AppendChildComment(pkg.GetBundleRoot(), L"Communication Profile", NULL);
    _commProfile.ToXml(pkg.GetDoc(), pkg.GetBundleRoot(), NULL);
    //pkg.AppendChildComment(pkg.GetBundleRoot(), L"Classification Profile", NULL);
    //_classifyProfile.ToXml(pkg.GetDoc(), pkg.GetBundleRoot(), NULL);
    pkg.AppendChildComment(pkg.GetBundleRoot(), L"Key Profile", NULL);
    _keyProfile.ToXml(pkg.GetDoc(), pkg.GetBundleRoot(), NULL);
    pkg.AppendChildComment(pkg.GetBundleRoot(), L"Policy Profile", NULL);
    _policyProfile.ToXml(pkg.GetDoc(), pkg.GetBundleRoot(), NULL);
    pkg.SetNodeAttribute(pkg.GetBundleRoot(), PROFILE_ATTRIBUTE_ID, this->GetId());
    pkg.SignPackage(key);
    pkg.SaveToFile(file.c_str());
}

void CProfileBundle::UpdateProfileId()
{
    std::vector<UCHAR> md5;
    std::wstring combine = _agentProfile.GetAgentId() + _agentProfile.GetAgentType() + _commProfile.GetServHost();
    if(!nudf::crypto::ToMd5(combine.c_str(), (ULONG)combine.length()*2, md5) || md5.size() < 16) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    WCHAR sguid[128] = {0};
    const GUID* guid = (const GUID*)(&md5[0]);
    swprintf_s(sguid, 128, L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", guid->Data1, guid->Data2, guid->Data3, guid->Data4[0], guid->Data4[1],
        guid->Data4[2], guid->Data4[3], guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
    this->SetId(sguid);
}

void CProfileBundle::Clear() throw()
{
    CProfile::Clear();
    _agentProfile.Clear();
    _commProfile.Clear();
    _classifyProfile.Clear();
    _keyProfile.Clear();
    _policyProfile.Clear();
}