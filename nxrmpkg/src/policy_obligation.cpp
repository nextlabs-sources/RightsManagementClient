

#include <Windows.h>
#include <atlbase.h>
#include <atlcomcli.h>


#include <string>

#include <nudf\exception.hpp>
#include <nudf\string.hpp>
#include <nudf\xmlparser.hpp>

#include "policy_obligation.hpp"



using namespace nxrm::pkg::policy;


//
//  class CObligation
//
CPolicyObligation::CPolicyObligation() : nudf::util::CObligation()
{
}

CPolicyObligation::CPolicyObligation(USHORT id) : nudf::util::CObligation(id)
{
}

CPolicyObligation::~CPolicyObligation()
{
}

CPolicyObligation& CPolicyObligation::operator = (const CPolicyObligation& ob) throw()
{
    if(this != &ob) {
        ((nudf::util::CObligation&)(*this)) = (const nudf::util::CObligation&)ob;
    }
    return *this;
}

void CPolicyObligation::FromXml(IXMLDOMNode* node)
{
    HRESULT      hr = S_OK;
    std::wstring root_name;
    long         count = 0;
    std::wstring name;
    USHORT       id = 0;
    CComPtr<IXMLDOMNodeList> spNodeList;

    if(NODE_ELEMENT != nudf::util::XmlUtil::GetNodeType(node)) {
        throw WIN32ERROR2(ERROR_DATATYPE_MISMATCH);
    }

    root_name = nudf::util::XmlUtil::GetNodeName(node);
    if(0 != _wcsicmp(root_name.c_str(), OB_NODENAME)) {
        throw WIN32ERROR2(ERROR_INVALID_NAME);
    }

    this->Clear();

    // Make sure it is a valid obligation
    if(!nudf::util::XmlUtil::GetNodeAttribute(node, OB_NAME_ATTRIBUTE, name) || name.empty()) {
        return;
    }
    id = NameToId(name);
    if(0 == id) {
        return;
    }
    this->SetId(id);
    
    hr = node->get_childNodes(&spNodeList);
    if(SUCCEEDED(hr) && spNodeList!=NULL) {

        hr = spNodeList->get_length(&count);
        if(FAILED(hr)) {
            count = 0;
        }

        for(long i=0; i<count; i++) {

            CComPtr<IXMLDOMNode> spNode;
            std::wstring node_name;
            std::wstring param_name;
            std::wstring param_value;

            hr = spNodeList->get_item(i, &spNode);
            if(FAILED(hr) || spNode==NULL) {
                break;
            }
            if(NODE_ELEMENT != nudf::util::XmlUtil::GetNodeType(spNode)) {
                continue;
            }
            node_name = nudf::util::XmlUtil::GetNodeName(spNode);
            if(0 != _wcsicmp(node_name.c_str(), OB_PARAM_NODENAME)) {
                continue;
            }

            if(!nudf::util::XmlUtil::GetNodeAttribute(spNode, OB_PARAM_NAME_ATTRIBUTE, param_name) || param_name.empty()) {
                continue;
            }
            std::transform(param_name.begin(), param_name.end(), param_name.begin(), tolower);
            if(!nudf::util::XmlUtil::GetNodeAttribute(spNode, OB_PARAM_VALUE_ATTRIBUTE, param_value)) {
                continue;
            }

            this->GetParams()[param_name] = param_value;
        }
    }
}

void CPolicyObligation::ToXml(IXMLDOMDocument* doc, IXMLDOMNode* parent, IXMLDOMNode** node) const
{
    ToXml(*this, doc, parent, node);
}

void CPolicyObligation::ToXml(const nudf::util::CObligation& ob, IXMLDOMDocument* doc, IXMLDOMNode* parent, IXMLDOMNode** node)
{
    try {

        HRESULT hr = S_OK;
        CComPtr<IXMLDOMElement> spOb;
        CComPtr<IXMLDOMNode> spObNode;
        std::wstring name;

        if(!ob.IsValid()) {
            return;
        }

        nudf::util::XmlUtil::CreateElement(doc, OB_NODENAME, &spOb);
        nudf::util::XmlUtil::SetNodeAttribute(doc, spOb, OB_NAME_ATTRIBUTE, IdToName(ob.GetId()));

        const nudf::util::OBPARAMS& params = ob.GetParams();
        for(nudf::util::OBPARAMS::const_iterator it=params.begin(); it!=params.end(); ++it) {

            CComPtr<IXMLDOMNode> spParam;
            if((*it).first.empty() || (*it).second.empty()) {
                continue;
            }
            nudf::util::XmlUtil::AppendChildElement(doc, spOb, OB_PARAM_NODENAME, &spParam);
            nudf::util::XmlUtil::SetNodeAttribute(doc, spParam, L"name", (*it).first);
            nudf::util::XmlUtil::SetNodeAttribute(doc, spParam, L"value", (*it).second);
        }

        hr = parent->appendChild(spOb, &spObNode);
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }
        if(NULL != node) {
            *node = spObNode.Detach();
        }
    }
    catch(const nudf::CException& e) {
        throw e;
    }
}

USHORT CPolicyObligation::NameToId(_In_ const std::wstring& name)
{
    if(name == OB_NAME_OVERLAY) {
        return OB_ID_OVERLAY;
    }
    else if(name == OB_NAME_CLASSIFY) {
        return OB_ID_CLASSIFY;
    }
    else {
        return 0;
    }
}

LPCWSTR CPolicyObligation::IdToName(_In_ USHORT id)
{
    if(id == OB_ID_OVERLAY) {
        return OB_NAME_OVERLAY;
    }
    else if(id == OB_ID_CLASSIFY) {
        return OB_NAME_CLASSIFY;
    }
    else {
        return L"";
    }
}
