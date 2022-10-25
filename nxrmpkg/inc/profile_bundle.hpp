

#ifndef __NXRM_PKG_PROFILE_BUNDLE_HPP__
#define __NXRM_PKG_PROFILE_BUNDLE_HPP__

#include <atlbase.h>
#include <atlcomcli.h>
#include <string>
#include <vector>
#include <memory>

#include "package.hpp"
#include "profile_agent.hpp"
#include "profile_comm.hpp"
#include "profile_classify.hpp"
#include "profile_key.hpp"
#include "profile_policy.hpp"


namespace nxrm {
namespace pkg {
    
#define PROFILEBUNDLE_NODENAME  L"PROFILE"
#define PROFILEBUNDLE_DATA_TYPE L"ProfileBundle"


  
/*
FILE:
    profile.xml
CONTENT:
    <?xml version="1.0"?>
    <NXPACKAGE>
        <SIGNATURE>
            Base64(Signature)
        </SIGNATURE>
        <DATA data-type="PROFILE" issue-time="2015-03-23T19:46:11Z" issuer="www.nextlabs.com">
            <PROFILE id="226828FE-62A0-4753-9966-32BA5B5875DE" name="Default Profile" timestamp="2015-03-23T19:46:11Z">
                <AGENT_PROFILE id="1" name="Default Agent Profile" timestamp="2015-01-12T15:48:31Z">
                    (Agent Profile)
                </AGENT_PROFILE>
                <COMM_PROFILE id="1" name="Default Communication Profile" timestamp="2015-01-12T15:48:31Z">
                    (Communication Profile)
                </COMM_PROFILE>
                <CLASSIFY_PROFILE>
                    (Classification Profile)
                </CLASSIFY_PROFILE>
                <KEY_PROFILE>
                    (Key Profile)
                </KEY_PROFILE>
                <POLICY_PROFILE>
                    (Policy Profile)
                </POLICY_PROFILE>
            </PROFILE>
        </DATA>
    </NXPACKAGE>
*/

class CProfilePackage : public CPackage
{
public:
    CProfilePackage();
    virtual ~CProfilePackage();

    virtual void LoadFromFile(const wchar_t* file);
    virtual void LoadFromXml(const wchar_t* xml);
    virtual void Create(_In_ const wchar_t* issuer);
    virtual void Close();


    inline IXMLDOMNode* GetBundleRoot() {return _bundle;}
        
protected:
    void LoadBundle();

private:
    CComPtr<IXMLDOMNode>    _bundle;
};
 
class CProfileBundle : public CProfile
{
public:
    CProfileBundle();
    virtual ~CProfileBundle();
    
    void FromXml(IXMLDOMNode* node);
    void ToXml(IXMLDOMDocument* doc, IXMLDOMNode* parent, IXMLDOMNode** node) const;

    void FromFile(const std::wstring& file, nudf::crypto::CRsaPubKeyBlob& key);
    void ToFile(const std::wstring& file, nudf::crypto::CLegacyRsaPriKeyBlob& key) const;

    void UpdateProfileId();

    void Clear() throw();

    inline const CAgentProfile& GetAgentProfile() const throw() {return _agentProfile;}
    inline CAgentProfile& GetAgentProfile() throw() {return _agentProfile;}
    inline const CCommProfile& GetCommProfile() const throw() {return _commProfile;}
    inline CCommProfile& GetCommProfile() throw() {return _commProfile;}
    inline const CClassifyProfile& GetClassifyProfile() const throw() {return _classifyProfile;}
    inline CClassifyProfile& GetClassifyProfile() throw() {return _classifyProfile;}
    inline const CKeyProfile& GetKeyProfile() const throw() {return _keyProfile;}
    inline CKeyProfile& GetKeyProfile() throw() {return _keyProfile;}
    inline const CPolicyProfile& GetPolicyProfile() const throw() {return _policyProfile;}
    inline CPolicyProfile& GetPolicyProfile() throw() {return _policyProfile;}
    
    CProfileBundle& operator = (const CProfileBundle& bundle) throw()
    {
        if(this != &bundle) {
            CProfile::operator=(bundle);
            _agentProfile = bundle.GetAgentProfile();
            _commProfile = bundle.GetCommProfile();
            _classifyProfile = bundle.GetClassifyProfile();
            _keyProfile = bundle.GetKeyProfile();
            _policyProfile = bundle.GetPolicyProfile();
        }
        return *this;
    }
    

private:
    CAgentProfile    _agentProfile;
    CCommProfile     _commProfile;
    CClassifyProfile _classifyProfile;
    CKeyProfile      _keyProfile;
    CPolicyProfile   _policyProfile;
};

    
}   // namespace nxrm::pkg
}   // namespace nxrm


#endif  // __NXRM_PKG_PROFILE_BUNDLE_HPP__