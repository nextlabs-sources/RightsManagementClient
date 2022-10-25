

#include <Windows.h>

#include <string>

#include <nudf\exception.hpp>
#include <nudf\string.hpp>
#include <nudf\xmlparser.hpp>

#include "package.hpp"


using namespace nudf::util;
using namespace nxrm::pkg;

CPackage::CPackage() : CXmlDocument()
{
}

CPackage::~CPackage()
{
    Close();
}

void CPackage::LoadFromFile(const wchar_t* file)
{
    CComPtr<IXMLDOMElement> spRoot;
    std::wstring            root_name;

    CXmlDocument::LoadFromFile(file);

    if(!GetDocRoot(&spRoot)) {
        Close();
        throw WIN32ERROR2(ERROR_FILE_INVALID);
    }

    root_name = GetNodeName(spRoot);
    if(0 != _wcsicmp(root_name.c_str(), ROOT_NODE_NAME)) {
        Close();
        throw WIN32ERROR2(ERROR_FILE_INVALID);
    }

    if(!FindChildElement(spRoot, SIG_NODE_NAME, &_signode)) {
        Close();
        throw WIN32ERROR2(ERROR_FILE_INVALID);
    }
    if(!FindChildElement(spRoot, DATA_NODE_NAME, &_datanode)) {
        Close();
        throw WIN32ERROR2(ERROR_FILE_INVALID);
    }

    if(!GetNodeAttribute(_datanode, DATA_ATTRIBUTE_ISSUER, _issuer) || _issuer.empty()) {
        Close();
        throw WIN32ERROR2(ERROR_FILE_INVALID);
    }
    if(!GetNodeAttribute(_datanode, DATA_ATTRIBUTE_ISSUETIME, &_issuetime)) {
        Close();
        throw WIN32ERROR2(ERROR_FILE_INVALID);
    }
    if(!GetNodeAttribute(_datanode, DATA_ATTRIBUTE_DATATYPE, _datatype) || _datatype.empty()) {
        Close();
        throw WIN32ERROR2(ERROR_FILE_INVALID);
    }
}

void CPackage::LoadFromXml(const wchar_t* xml)
{
    CComPtr<IXMLDOMElement> spRoot;
    std::wstring            root_name;
    
    CXmlDocument::LoadFromXml(xml);
    
    if(!GetDocRoot(&spRoot)) {
        Close();
        throw WIN32ERROR2(ERROR_FILE_INVALID);
    }

    root_name = GetNodeName(spRoot);
    if(0 != _wcsicmp(root_name.c_str(), ROOT_NODE_NAME)) {
        Close();
        throw WIN32ERROR2(ERROR_FILE_INVALID);
    }

    if(!FindChildElement(spRoot, SIG_NODE_NAME, &_signode)) {
        Close();
        throw WIN32ERROR2(ERROR_FILE_INVALID);
    }
    if(!FindChildElement(spRoot, DATA_NODE_NAME, &_datanode)) {
        Close();
        throw WIN32ERROR2(ERROR_FILE_INVALID);
    }

    if(!GetNodeAttribute(_datanode, DATA_ATTRIBUTE_ISSUER, _issuer) || _issuer.empty()) {
        Close();
        throw WIN32ERROR2(ERROR_FILE_INVALID);
    }
    if(!GetNodeAttribute(_datanode, DATA_ATTRIBUTE_ISSUETIME, &_issuetime)) {
        Close();
        throw WIN32ERROR2(ERROR_FILE_INVALID);
    }
    if(!GetNodeAttribute(_datanode, DATA_ATTRIBUTE_DATATYPE, _datatype) || _datatype.empty()) {
        Close();
        throw WIN32ERROR2(ERROR_FILE_INVALID);
    }
}

void CPackage::Create(const wchar_t* issuer, const wchar_t* datatype)
{
    HRESULT hr = S_OK;
    CComPtr<IXMLDOMNode> spRoot;
    CComPtr<IXMLDOMNode> spSignature;
    CComPtr<IXMLDOMNode> spData;
    SYSTEMTIME st = {0};
    bool result = false;

    CXmlDocument::Create();

    try {
        // Creater Root
        AppendChildElement(ROOT_NODE_NAME, &spRoot);
        // Create Signature Element
        AppendChildElement(spRoot, SIG_NODE_NAME, &_signode);
        // Create Data Element
        AppendChildElement(spRoot, DATA_NODE_NAME, &_datanode);

        GetSystemTime(&st);

        SetNodeText(_signode, L"N/A");
        SetNodeAttribute(_datanode, DATA_ATTRIBUTE_ISSUER, issuer);
        SetNodeAttribute(_datanode, DATA_ATTRIBUTE_ISSUETIME, &st);
        SetNodeAttribute(_datanode, DATA_ATTRIBUTE_DATATYPE, datatype);
    }
    catch(const nudf::CException& e) {
        throw e;
    }
}

void CPackage::Create()
{
    throw WIN32ERROR2(ERROR_INVALID_FUNCTION);
}

void CPackage::SignPackage(_In_ const nudf::crypto::CLegacyRsaPriKeyBlob& key)
{
    HRESULT hr = S_OK;
    CComBSTR bstrXml;

    hr = _datanode->get_xml(&bstrXml);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    std::vector<UCHAR> signature;
    hr = nudf::crypto::RsaSign(key, bstrXml.m_str, bstrXml.ByteLength(), signature);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    std::wstring wssig = nudf::string::FromBytes<wchar_t>(&signature[0], (ULONG)signature.size());
    SetNodeText(_signode, wssig);
}

void CPackage::VerifySignatrue(_In_ const nudf::crypto::CRsaPubKeyBlob& key) const
{
    HRESULT hr = S_OK;
    CComBSTR bstrXml;
    std::wstring wsSig;

    hr = _datanode->get_xml(&bstrXml);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    wsSig = ((nudf::util::CXmlDocument*)this)->GetNodeText(_signode);
    if(wsSig.empty() || wsSig == L"N/A") {
        throw WIN32ERROR2(HRESULT_FROM_WIN32(ERROR_NOT_FOUND));
    }

    std::vector<UCHAR> signature;
    nudf::string::ToBytes<wchar_t>(wsSig, signature);


    hr = nudf::crypto::RsaVerifySignature(key, bstrXml.m_str, bstrXml.ByteLength(), &signature[0], (ULONG)signature.size());
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }
}

void CPackage::Close()
{
    _signode.Release();
    _datanode.Release();
    _issuer = L"";
    _datatype = L"";
    memset(&_issuetime, 0, sizeof(_issuetime));
    CXmlDocument::Close();
}
