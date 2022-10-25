


#include <Windows.h>
#include <atlbase.h>
#include <atlcomcli.h>

#include <string>

#include <nudf\exception.hpp>
#include <nudf\string.hpp>
#include <nudf\xmlparser.hpp>

#include "policy_rights.hpp"


using namespace nxrm::pkg::policy;

#define RIGHTS_TYPE_BUILTIN L"builtin"
#define RIGHTS_TYPE_CUSTOM  L"custom"

//
//  class CUserGroupMap
//
CRight::CRight() : _id(0xFFFFFFFF), _value(0xFFFFFFFFFFFFFFFF), _custom(false)
{
}

CRight::CRight(unsigned int id, const wchar_t* name, const wchar_t* display_name, bool custom) : _id(0xFFFFFFFF), _value(0xFFFFFFFFFFFFFFFF), _custom(false)
{
    if(id <= 63) {
        _id = id;
        _value = (0 == id) ? 0x0000000000000001ULL : ((0x0000000000000001ULL) << id);
        _name = name;
        _display_name = display_name;
        _custom = custom;
    }
}

CRight::~CRight()
{
}

CRight& CRight::operator = (const CRight& right) throw()
{
    if(this != &right) {
        _id = right.GetId();
        _value = right.GetValue();
        _name = right.GetName();
        _display_name = right.GetDisplayName();
        _custom = right.IsCustom();
    }
    return *this;
}

void CRight::FromXml(_In_ IXMLDOMNode* node)
{
    HRESULT      hr = S_OK;
    std::wstring root_name;
    std::wstring type;

    if(NODE_ELEMENT != nudf::util::XmlUtil::GetNodeType(node)) {
        throw WIN32ERROR2(ERROR_DATATYPE_MISMATCH);
    }

    root_name = nudf::util::XmlUtil::GetNodeName(node);
    if(0 != _wcsicmp(root_name.c_str(), RIGHT_NODENAME)) {
        throw WIN32ERROR2(ERROR_INVALID_NAME);
    }
    
    if(!nudf::util::XmlUtil::GetNodeAttribute(node, RIGHT_ID_ATTRIBUTE, (int*)(&_id))) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    if(_id > 63) {
        _id = (unsigned int)-1;
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    else {
        _value = (0 == _id) ? 0x0000000000000001ULL : ((0x0000000000000001ULL) << _id);
    }
    if(!nudf::util::XmlUtil::GetNodeAttribute(node, RIGHT_NAME_ATTRIBUTE, _name) || _name.empty()) {
        _id = (unsigned int)-1;
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    if(!nudf::util::XmlUtil::GetNodeAttribute(node, RIGHT_TYPE_ATTRIBUTE, type)) {
        _custom = false;
    }
    else {
        _custom = (0 == _wcsicmp(type.c_str(), RIGHTS_TYPE_BUILTIN)) ? false : true;
    }

    _display_name = nudf::util::XmlUtil::GetNodeText(node);
    if(_display_name.empty()) {
        _display_name = _name;
    }
}

void CRight::ToXml(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* parent, _In_opt_ IXMLDOMNode** node)
{
    HRESULT hr = S_OK;


    if(_id == (UINT)-1) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    if(_name.empty()) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }

    try {

        CComPtr<IXMLDOMElement> pRoot = NULL;
        CComPtr<IXMLDOMNode> child = NULL;
        
        nudf::util::XmlUtil::CreateElement(doc, RIGHT_NODENAME, &pRoot);
        nudf::util::XmlUtil::SetNodeAttribute(doc, pRoot, RIGHT_ID_ATTRIBUTE, (int)_id);
        nudf::util::XmlUtil::SetNodeAttribute(doc, pRoot, RIGHT_NAME_ATTRIBUTE, _name);
        nudf::util::XmlUtil::SetNodeAttribute(doc, pRoot, RIGHT_TYPE_ATTRIBUTE, _custom?RIGHTS_TYPE_CUSTOM:RIGHTS_TYPE_BUILTIN);
        nudf::util::XmlUtil::SetNodeText(pRoot, _display_name.empty() ? _name : _display_name);

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