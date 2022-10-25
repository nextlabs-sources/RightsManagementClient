

#ifndef __NXRM_PKG_POLICY_OBLIGATION_HPP__
#define __NXRM_PKG_POLICY_OBLIGATION_HPP__

#include <nudf\shared\obutil.h>

namespace nxrm {
namespace pkg {
namespace policy {

#define OB_NODENAME              L"OBLIGATION"
#define OB_PARAM_NODENAME        L"PARAM"
#define OB_NAME_ATTRIBUTE        L"name"
#define OB_PARAM_NAME_ATTRIBUTE  L"name"
#define OB_PARAM_VALUE_ATTRIBUTE L"value"


class CPolicyObligation : public nudf::util::CObligation
{
public:
    CPolicyObligation();
    CPolicyObligation(USHORT id);
    virtual ~CPolicyObligation();

    CPolicyObligation& operator = (const CPolicyObligation& ob) throw();

    void FromXml(IXMLDOMNode* node);
    void ToXml(IXMLDOMDocument* doc, IXMLDOMNode* parent, IXMLDOMNode** node) const;


    static void ToXml(const nudf::util::CObligation& ob, IXMLDOMDocument* doc, IXMLDOMNode* parent, IXMLDOMNode** node);
    static USHORT NameToId(_In_ const std::wstring& name);
    static LPCWSTR IdToName(_In_ USHORT id);
};

    
}   // namespace nxrm::pkg::policy
}   // namespace nxrm::pkg
}   // namespace nxrm


#endif