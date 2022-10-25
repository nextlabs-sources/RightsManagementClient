

#ifndef __NXRM_PKG_POLICY_USER_HPP__
#define __NXRM_PKG_POLICY_USER_HPP__

#include <string>
#include <set>

namespace nxrm {
namespace pkg {
namespace policy {

    
#define USER_NODENAME           L"USERMAP"
#define USER_ID_ATTRIBUTE       L"id"
#define USER_TENANTID_ATTRIBUTE L"tenantid"
#define USER_CONTEXT_ATTRIBUTE  L"context"
#define UNKNOWN_USER_GROUP_NAME L"{UNKNOWN-USER}"

#define BUILTIN_NEXTLABS_TENANTID   L"anonymous"

class CUserGroups
{
public:
    CUserGroups();
    CUserGroups(const wchar_t* id, const std::set<unsigned int>& groups);
    CUserGroups(const wchar_t* id, const wchar_t* tenantid, const wchar_t* context, const std::set<unsigned int>& groups);
    virtual ~CUserGroups();

    CUserGroups& operator = (const CUserGroups& ug) throw();

    inline const std::wstring& GetId() const throw() {return _id;}
    inline const std::wstring& GetTenantId() const throw() {return _tenantid;}
    inline const std::wstring& GetContext() const throw() {return _context;}
    inline const std::set<unsigned int>& GetGroups() const throw() {return _groups;}
    inline bool IsUnknownUserGroups() const throw() {return _unknown;}
    
    void FromXml(_In_ IXMLDOMNode* node);
    void ToXml(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* parent, _In_opt_ IXMLDOMNode** node);

private:
    std::wstring    _id;
    std::wstring    _tenantid;
    std::wstring    _context;
    std::set<unsigned int> _groups;
    bool            _unknown;
};

    
}   // namespace nxrm::pkg::policy
}   // namespace nxrm::pkg
}   // namespace nxrm


#endif