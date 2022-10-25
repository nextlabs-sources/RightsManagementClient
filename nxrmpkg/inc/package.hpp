

#ifndef __NXRM_PKG_HPP__
#define __NXRM_PKG_HPP__

#include <atlbase.h>
#include <atlcomcli.h>
#include <string>
#include <nudf\xmlparser.hpp>
#include <nudf\crypto.hpp>

namespace nxrm {
namespace pkg {

    

#define ROOT_NODE_NAME              L"NXPACKAGE"
#define SIG_NODE_NAME               L"SIGNATURE"
#define DATA_NODE_NAME              L"DATA"
#define DATA_ATTRIBUTE_ISSUER       L"issuer"
#define DATA_ATTRIBUTE_ISSUETIME    L"issue-time"
#define DATA_ATTRIBUTE_DATATYPE     L"data-type"

class CPackage : public nudf::util::CXmlDocument
{
public:
    CPackage();
    virtual ~CPackage();
    
    virtual void Create(const wchar_t* issuer, const wchar_t* datatype);
    virtual void LoadFromFile(_In_ LPCWSTR file);
    virtual void LoadFromXml(_In_ LPCWSTR xml);
    virtual void Close() throw();

    virtual void SignPackage(_In_ const nudf::crypto::CLegacyRsaPriKeyBlob& key);
    virtual void VerifySignatrue(_In_ const nudf::crypto::CRsaPubKeyBlob& key) const;


    inline const std::wstring& GetIssuer() const throw() {return _issuer;}
    inline const std::wstring& GetDataType() const throw() {return _datatype;}
    inline const SYSTEMTIME* GetIssueTime() const throw() {return &_issuetime;}

protected:
    virtual void Create();
    
protected:
    std::wstring      _issuer;
    SYSTEMTIME        _issuetime;
    std::wstring      _datatype;

protected:
    CComPtr<IXMLDOMNode> _signode;
    CComPtr<IXMLDOMNode> _datanode;
};

    
}   // namespace nxrm::pkg
}   // namespace nxrm


#endif