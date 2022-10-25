

#ifndef __NXRM_PKG_POLICY_RIGHT_HPP__
#define __NXRM_PKG_POLICY_RIGHT_HPP__



namespace nxrm {
namespace pkg {
namespace policy {

    
#define RIGHT_NODENAME       L"RIGHT"
#define RIGHT_ID_ATTRIBUTE   L"id"
#define RIGHT_NAME_ATTRIBUTE L"name"
#define RIGHT_TYPE_ATTRIBUTE L"type"

class CRight
{
public:
    CRight();
    CRight(unsigned int id, const wchar_t* name, const wchar_t* display_name, bool custom);
    virtual ~CRight();

    CRight& operator = (const CRight& right) throw();

    inline unsigned int GetId() const throw() {return _id;}
    inline unsigned __int64 GetValue() const throw() {return _value;}
    inline const std::wstring& GetName() const throw() {return _name;}
    inline const std::wstring& GetDisplayName() const throw() {return _display_name;}
    inline bool IsCustom() const throw() {return _custom;}
    
    void FromXml(_In_ IXMLDOMNode* node);
    void ToXml(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* parent, _In_opt_ IXMLDOMNode** node);

private:
    unsigned int     _id;
    unsigned __int64 _value;
    std::wstring     _name;
    std::wstring     _display_name;
    bool             _custom;
};

    
}   // namespace nxrm::pkg::policy
}   // namespace nxrm::pkg
}   // namespace nxrm


#endif