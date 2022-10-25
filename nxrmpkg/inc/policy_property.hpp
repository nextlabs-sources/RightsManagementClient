

#ifndef __NXRM_PKG_POLICY_PROPERTY_HPP__
#define __NXRM_PKG_POLICY_PROPERTY_HPP__

#include <unordered_map>

namespace nxrm {
namespace pkg {
namespace policy {

    
#define PROP_NODENAME           L"PROPERTY"
#define PROP_TYPE_ATTRIBUTE     L"type"
#define PROP_METHOD_ATTRIBUTE   L"method"
#define PROP_NAME_ATTRIBUTE     L"name"
#define PROP_VALUE_ATTRIBUTE    L"value"


typedef enum _PROPTYPE {
    PropString = 0,
    PropInt,
    PropUint,
    PropInt64,
    PropUint64,
    PropFloat,
    PropBool,
    PropDatetime,
    PropTypeMax
} PROPTYPE;

typedef enum _PROPMETHOD {
    NE = 0,
    EQ,
    LT,
    GT,
    LE,
    GE,
    PropMethodMax
} PROPMETHOD;

typedef union _PROPVALUE {
    int              l;
    unsigned int     ul;
    __int64          ll;
    unsigned __int64 ull;
    float            f;
    bool             b;
    FILETIME         ft;
} PROPVALUE;

class CProperty
{
public:
    CProperty();
    CProperty(const wchar_t* name, const wchar_t* value, PROPTYPE type=PropString);
    CProperty(const wchar_t* name, int value);
    CProperty(const wchar_t* name, unsigned int value);
    CProperty(const wchar_t* name, __int64 value);
    CProperty(const wchar_t* name, unsigned __int64 value);
    CProperty(const wchar_t* name, float value);
    CProperty(const wchar_t* name, bool value);
    CProperty(const wchar_t* name, const FILETIME& value);
    virtual ~CProperty();


    CProperty& operator = (const CProperty& prop) throw();
    bool operator == (const CProperty& prop) throw();

    inline PROPTYPE GetType() const throw() {return _type;}
    inline const std::wstring& GetName() const throw() {return _name;}
    inline const std::wstring& GetValue() const throw() {return _value;}
    inline int GetIntValue() const throw() {return _var.l;}
    inline unsigned int GetUintValue() const throw() {return _var.ul;}
    inline __int64 GetInt64Value() const throw() {return _var.ll;}
    inline unsigned __int64 GetUint64Value() const throw() {return _var.ull;}
    inline float GetFloatValue() const throw() {return _var.f;}
    inline bool GetBoolValue() const throw() {return _var.b;}
    inline FILETIME GetDatetimeValue() const throw() {return _var.ft;}

    bool SetType(PROPTYPE type) throw();
    bool SetName(const wchar_t* name) throw();
    bool SetValue(const wchar_t* value) throw();
    bool SetIntValue(int value) throw();
    bool SetUintValue(unsigned int value) throw();
    bool SetInt64Value(__int64 value) throw();
    bool SetUint64Value(unsigned __int64 value) throw();
    bool SetFloatValue(float value) throw();
    bool SetBoolValue(bool value) throw();
    bool SetDatetimeValue(const FILETIME& value) throw();


    void Clear();
        
    virtual void FromXml(_In_ IXMLDOMNode* node);
    virtual void ToXml(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* parent, _Out_opt_ IXMLDOMNode** node);

protected:
    PROPTYPE        _type;
    std::wstring    _name;
    std::wstring    _value;
    PROPVALUE       _var;
};

typedef std::unordered_multimap<std::wstring, CProperty> PROPERTYMAP;

class CPattern : public CProperty
{
public:
    CPattern();
    CPattern(const wchar_t* name, const wchar_t* value, PROPTYPE type=PropString, PROPMETHOD method=EQ);
    CPattern(const wchar_t* name, int value, PROPMETHOD method=EQ);
    CPattern(const wchar_t* name, unsigned int value, PROPMETHOD method=EQ);
    CPattern(const wchar_t* name, __int64 value, PROPMETHOD method=EQ);
    CPattern(const wchar_t* name, unsigned __int64 value, PROPMETHOD method=EQ);
    CPattern(const wchar_t* name, float value, PROPMETHOD method=EQ);
    CPattern(const wchar_t* name, bool value, PROPMETHOD method=EQ);
    CPattern(const wchar_t* name, const FILETIME& value, PROPMETHOD method=EQ);
    virtual ~CPattern();


    CPattern& operator = (const CPattern& prop) throw();
    bool Match(const CProperty& prop) const throw();
    bool Match(const wchar_t* value) const throw();
    bool Match(int value) const throw();
    bool Match(unsigned int value) const throw();
    bool Match(__int64 value) const throw();
    bool Match(unsigned __int64 value) const throw();
    bool Match(float value) const throw();
    bool Match(bool value) const throw();
    bool Match(const FILETIME& value) const throw();

    inline PROPMETHOD GetMethod() const throw() {return _method;}
    bool SetMethod(PROPMETHOD method) throw();

    void ToRegex() throw();

    virtual void FromXml(_In_ IXMLDOMNode* node);
    virtual void ToXml(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* parent, _Out_opt_ IXMLDOMNode** node);

private:
    PROPMETHOD      _method;
};

    

}   // namespace nxrm::pkg::policy
}   // namespace nxrm::pkg
}   // namespace nxrm


#endif