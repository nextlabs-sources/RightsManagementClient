

#ifndef __NXRM_PKG_POLICY_RULE_HPP__
#define __NXRM_PKG_POLICY_RULE_HPP__

#include "policy_property.hpp"

namespace nxrm {
namespace pkg {
namespace policy {

    
#define RULE_USR_NODENAME   L"USER"
#define RULE_APP_NODENAME   L"APPLICATION"
#define RULE_LOC_NODENAME   L"LOCATION"
#define RULE_RES_NODENAME   L"RESOURCE"
#define RULE_ENV_NODENAME   L"ENV"
#define RULE_ID_ATTRIBUTE   L"id"


typedef enum _RULETYPE {
    RULE_USER = 0,
    RULE_APPLICATION = 1,
    RULE_LOCATION = 2,
    RULE_RESOURCE = 3,
    RULE_ENV = 4,
    RULE_MAX
} RULETYPE;

class CRule
{
public:
    CRule();
    CRule(_In_ unsigned int id, _In_ RULETYPE type, _In_ const std::vector<CPattern>& props, _In_opt_ const wchar_t* comment);
    virtual ~CRule();

    inline void SetId(unsigned int id) throw() {_id = id;}
    inline unsigned int GetId() const throw() {return _id;}
    inline unsigned int GetType() const throw() {return _type;}
    inline const std::wstring& GetComment() const throw() {return _comment;}
    inline const std::vector<CPattern>& GetPatterns() const throw() {return _patterns;}
    inline std::vector<CPattern>& GetPatterns() throw() {return _patterns;}

    CRule& operator = (const CRule& rule) throw();
    bool Match(const std::unordered_multimap<std::wstring, CProperty>& props);
    
    void FromXml(_In_ IXMLDOMNode* node);
    void ToXml(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* parent, _In_opt_ IXMLDOMNode** node);

    void Clear() throw();

private:
    unsigned int _id;
    RULETYPE _type;
    std::vector<CPattern>  _patterns;
    std::wstring    _comment;
};

    
}   // namespace nxrm::pkg::policy
}   // namespace nxrm::pkg
}   // namespace nxrm


#endif