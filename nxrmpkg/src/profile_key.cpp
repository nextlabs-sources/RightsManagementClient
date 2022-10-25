
#include <Windows.h>
#include <atlbase.h>
#include <atlcomcli.h>


#include <string>

#include <nudf\exception.hpp>
#include <nudf\string.hpp>
#include <nudf\xmlparser.hpp>
#include <nudf\crypto.hpp>

#include "profile_key.hpp"



using namespace nxrm::pkg;



#define PROFILE_AGENT_NODENAME_KEY      L"KEY"
#define PROFILE_KEY_ATTRIBUTE_CHECKSUM  L"checksum"
#define PROFILE_KEY_ATTRIBUTE_KEYID     L"id"

#define KEYID_SIZE      20
#define KEY128_SIZE     16
#define KEY256_SIZE     32

CKeyProfile::CKeyProfile() : CProfile(), _checksum(0)
{
}

CKeyProfile::~CKeyProfile()
{
}

bool CKeyProfile::InsertKey(const std::vector<UCHAR>& id, const std::vector<UCHAR>& key) throw()
{
    if(id.size() != KEYID_SIZE) {
        return false;
    }
    if(key.size() != KEY256_SIZE && key.size() != KEY128_SIZE) {
        return false;
    }
    _keys.push_back(KEYPAIR(id, key));
    return true;
}

bool CKeyProfile::InsertKey(const std::wstring& sid, const std::wstring& skey) throw()
{
    std::vector<UCHAR> id;
    std::vector<UCHAR> key;
    if(!nudf::string::ToBytes<wchar_t>(sid, id) || id.empty()) {
        return false;
    }
    if(!nudf::string::ToBytes<wchar_t>(skey, key) || key.empty()) {
        return false;
    }
    return InsertKey(id, key);
}

ULONGLONG CKeyProfile::UpdateChecksum() throw()
{
    _checksum = 0;
    for(KEYSARRAY::const_iterator it=_keys.begin(); it!=_keys.end(); ++it) {
        _checksum = nudf::crypto::ToCrc64(_checksum, &((*it).first[0]), (ULONG)(*it).first.size());
        _checksum = nudf::crypto::ToCrc64(_checksum, &((*it).second[0]), (ULONG)(*it).second.size());
    }
    return _checksum;
}

void CKeyProfile::FromXml(IXMLDOMNode* node)
{
    HRESULT      hr = S_OK;
    std::wstring root_name;
    ULONGLONG    checksum = 0;
    CComPtr<IXMLDOMNodeList> spNodeList;

    if(NODE_ELEMENT != nudf::util::XmlUtil::GetNodeType(node)) {
        throw WIN32ERROR2(ERROR_DATATYPE_MISMATCH);
    }

    root_name = nudf::util::XmlUtil::GetNodeName(node);
    if(0 != _wcsicmp(root_name.c_str(), PROFILE_KEY_NODENAME)) {
        throw WIN32ERROR2(ERROR_INVALID_NAME);
    }

    this->Clear();

    // Make sure the checksum exist
    if(!nudf::util::XmlUtil::GetNodeAttribute(node, PROFILE_KEY_ATTRIBUTE_CHECKSUM, &checksum)) {
        throw WIN32ERROR2(ERROR_NOT_FOUND);
    }
    
    hr = node->get_childNodes(&spNodeList);
    if(SUCCEEDED(hr) && spNodeList!=NULL) {

        long count = 0;

        hr = spNodeList->get_length(&count);
        if(FAILED(hr)) {
            count = 0;
        }

        for(long i=0; i<count; i++) {

            CComPtr<IXMLDOMNode> spNode;
            std::wstring node_name;
            std::wstring skey_id;
            std::wstring skey_data;

            hr = spNodeList->get_item(i, &spNode);
            if(FAILED(hr) || spNode==NULL) {
                break;
            }
            if(NODE_ELEMENT != nudf::util::XmlUtil::GetNodeType(spNode)) {
                continue;
            }
            node_name = nudf::util::XmlUtil::GetNodeName(spNode);
            if(0 != _wcsicmp(node_name.c_str(), PROFILE_AGENT_NODENAME_KEY)) {
                continue;
            }

            if(!nudf::util::XmlUtil::GetNodeAttribute(spNode, PROFILE_KEY_ATTRIBUTE_KEYID, skey_id) || skey_id.empty()) {
                continue;
            }
            skey_data = nudf::util::XmlUtil::GetNodeText(spNode);
            if(skey_data.empty()) {
                continue;
            }

            std::vector<UCHAR> key_id;
            std::vector<UCHAR> key_data;
            if(!nudf::string::ToBytes<wchar_t>(skey_id, key_id) || key_id.size()!=KEYID_SIZE || !nudf::string::ToBytes<wchar_t>(skey_data, key_data)) {
                continue;
            }
            if(key_id.size()!=KEYID_SIZE || (key_data.size()!=KEY128_SIZE && key_data.size()!=KEY256_SIZE)) {
                continue;
            }

            // Add this key
            _keys.push_back(KEYPAIR(key_id, key_data));
        }
    }
}

void CKeyProfile::ToXml(IXMLDOMDocument* doc, IXMLDOMNode* parent, IXMLDOMNode** node) const
{
    std::wstring node_name;
    
    try {

        HRESULT hr = S_OK;
        CComPtr<IXMLDOMElement> pRoot = NULL;
        
        nudf::util::XmlUtil::CreateElement(doc, PROFILE_KEY_NODENAME, &pRoot);
        nudf::util::XmlUtil::SetNodeAttribute(doc, pRoot, PROFILE_KEY_ATTRIBUTE_CHECKSUM, _checksum);
        for(KEYSARRAY::const_iterator it=_keys.begin(); it != _keys.end(); ++it) {
            
            CComPtr<IXMLDOMNode> child = NULL;
            std::wstring skey_id;
            std::wstring skey_data;

            skey_id = nudf::string::FromBytes<wchar_t>(&((*it).first[0]), (ULONG)(*it).first.size());
            skey_data = nudf::string::FromBytes<wchar_t>(&((*it).second[0]), (ULONG)(*it).second.size());
            nudf::util::XmlUtil::AppendChildElement(doc, pRoot, PROFILE_AGENT_NODENAME_KEY, &child);
            nudf::util::XmlUtil::SetNodeAttribute(doc, child, PROFILE_KEY_ATTRIBUTE_KEYID, skey_id);
            nudf::util::XmlUtil::SetNodeText(child, skey_data);
        }

        hr = parent->appendChild(pRoot, node);
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        } 
    }
    catch(const nudf::CException& e) {
        throw e;
    }
}