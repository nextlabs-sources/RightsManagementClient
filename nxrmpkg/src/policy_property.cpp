


#include <Windows.h>
#include <atlbase.h>
#include <atlcomcli.h>

#include <string>

#include <nudf\exception.hpp>
#include <nudf\string.hpp>
#include <nudf\xmlparser.hpp>

#include "policy_property.hpp"


using namespace nxrm::pkg::policy;

static const WCHAR* PROPTYPENAME[] = {
    L"string",
    L"int",
    L"uint",
    L"int64",
    L"uint64",
    L"float",
    L"bool",
    L"datetime",
    L"unknown"
};

static const WCHAR* PROPMETHODNAME[] = {
    L"NE",
    L"EQ",
    L"LT",
    L"GT",
    L"LE",
    L"GE",
    L"unknown"
};

//
//  class CProperty
//
CProperty::CProperty() : _type(PropString)
{
    _var.ull = 0;
}
CProperty::CProperty(const wchar_t* name, const wchar_t* value, PROPTYPE type) : _type(type), _name(name)
{
    switch(type)
    {
    case PropString: SetValue(value); break;
    default: break;
    }
    std::transform(_name.begin(), _name.end(), _name.begin(), tolower);
    std::transform(_value.begin(), _value.end(), _value.begin(), tolower);
}
CProperty::CProperty(const wchar_t* name, int value) : _type(PropInt), _name(name)
{
    _var.l = value;
    _value = nudf::string::FromInt<wchar_t>(value);
    std::transform(_name.begin(), _name.end(), _name.begin(), tolower);
    std::transform(_value.begin(), _value.end(), _value.begin(), tolower);
}
CProperty::CProperty(const wchar_t* name, unsigned int value) : _type(PropUint), _name(name)
{
    _var.ul = value;
    _value = nudf::string::FromUint<wchar_t>(value);
    std::transform(_name.begin(), _name.end(), _name.begin(), tolower);
    std::transform(_value.begin(), _value.end(), _value.begin(), tolower);
}
CProperty::CProperty(const wchar_t* name, __int64 value) : _type(PropInt64), _name(name)
{
    _var.ll = value;
    _value = nudf::string::FromInt64<wchar_t>(value);
    std::transform(_name.begin(), _name.end(), _name.begin(), tolower);
    std::transform(_value.begin(), _value.end(), _value.begin(), tolower);
}
CProperty::CProperty(const wchar_t* name, unsigned __int64 value) : _type(PropUint64), _name(name)
{
    _var.ull = value;
    _value = nudf::string::FromUint64<wchar_t>(value);
    std::transform(_name.begin(), _name.end(), _name.begin(), tolower);
    std::transform(_value.begin(), _value.end(), _value.begin(), tolower);
}
CProperty::CProperty(const wchar_t* name, float value) : _type(PropFloat), _name(name)
{
    _var.f = value;
    _value = nudf::string::FromFloat<wchar_t>(value);
    std::transform(_name.begin(), _name.end(), _name.begin(), tolower);
    std::transform(_value.begin(), _value.end(), _value.begin(), tolower);
}
CProperty::CProperty(const wchar_t* name, bool value) : _type(PropBool), _name(name)
{
    _var.b = value;
    _value = nudf::string::FromBoolean<wchar_t>(value);
    std::transform(_name.begin(), _name.end(), _name.begin(), tolower);
    std::transform(_value.begin(), _value.end(), _value.begin(), tolower);
}
CProperty::CProperty(const wchar_t* name, const FILETIME& value) : _type(PropDatetime), _name(name)
{
    _var.ft = value;
    _value = nudf::string::FromSystemTime<wchar_t>(&value);
    std::transform(_name.begin(), _name.end(), _name.begin(), tolower);
}
CProperty::~CProperty()
{
}
CProperty& CProperty::operator = (const CProperty& prop) throw()
{
    if(this != &prop) {
        _type = prop.GetType();
        _name = prop.GetName();
        _value = prop.GetValue();
        _var.ull = prop.GetUint64Value();
    }
    return *this;
}
bool CProperty::operator == (const CProperty& prop) throw()
{
    return (_type == prop.GetType()
        && _name == prop.GetName()
        && _value == prop.GetValue()
        && _var.ull == prop.GetInt64Value()
        );
}

bool CProperty::SetType(PROPTYPE type) throw()
{
    if(type<0 || type>=PropTypeMax) {
        return false;
    }
    _type = type;
    return true;
}
bool CProperty::SetName(const wchar_t* name) throw()
{
    if(NULL==name || L'\0'==name[0]) {
        return false;
    }
    _name = name;
    std::transform(_value.begin(), _value.end(), _value.begin(), tolower);
    return true;
}
bool CProperty::SetValue(const wchar_t* value) throw()
{
    if(NULL==value || L'\0'==value[0]) {
        return false;
    }
    _value = value;
    std::transform(_value.begin(), _value.end(), _value.begin(), tolower);
    _type = PropString;
    _var.ull = 0;
    return true;
}
bool CProperty::SetIntValue(int value) throw()
{
    _value = nudf::string::FromInt<wchar_t>(value);
    std::transform(_value.begin(), _value.end(), _value.begin(), tolower);
    _var.l = value;
    _type = PropInt;
    return true;
}
bool CProperty::SetUintValue(unsigned int value) throw()
{
    _value = nudf::string::FromUint<wchar_t>(value);
    std::transform(_value.begin(), _value.end(), _value.begin(), tolower);
    _var.ul = value;
    _type = PropUint;
    return true;
}
bool CProperty::SetInt64Value(__int64 value) throw()
{
    _value = nudf::string::FromInt64<wchar_t>(value);
    std::transform(_value.begin(), _value.end(), _value.begin(), tolower);
    _var.ll = value;
    _type = PropInt64;
    return true;
}
bool CProperty::SetUint64Value(unsigned __int64 value) throw()
{
    _value = nudf::string::FromUint64<wchar_t>(value);
    std::transform(_value.begin(), _value.end(), _value.begin(), tolower);
    _var.ull = value;
    _type = PropUint64;
    return true;
}
bool CProperty::SetFloatValue(float value) throw()
{
    _value = nudf::string::FromFloat<wchar_t>(value);
    std::transform(_value.begin(), _value.end(), _value.begin(), tolower);
    _var.f = value;
    _type = PropFloat;
    return true;
}
bool CProperty::SetBoolValue(bool value) throw()
{
    _value = nudf::string::FromBoolean<wchar_t>(value);
    std::transform(_value.begin(), _value.end(), _value.begin(), tolower);
    _var.b = value;
    _type = PropBool;
    return true;
}
bool CProperty::SetDatetimeValue(const FILETIME& value) throw()
{
    _value = nudf::string::FromSystemTime<wchar_t>(&value);
    _var.ft = value;
    _type = PropDatetime;
    return true;
}

void CProperty::Clear()
{
    _type = PropTypeMax;
    _name = L"";
    _value= L"";
    _var.ull = 0;
}

void CProperty::FromXml(_In_ IXMLDOMNode* node)
{
    HRESULT      hr = S_OK;
    std::wstring root_name;
    std::wstring type;

    if(NODE_ELEMENT != nudf::util::XmlUtil::GetNodeType(node)) {
        throw WIN32ERROR2(ERROR_DATATYPE_MISMATCH);
    }

    root_name = nudf::util::XmlUtil::GetNodeName(node);
    if(0 != _wcsicmp(root_name.c_str(), PROP_NODENAME)) {
        throw WIN32ERROR2(ERROR_INVALID_NAME);
    }

    if(!nudf::util::XmlUtil::GetNodeAttribute(node, PROP_VALUE_ATTRIBUTE, _value) || _value.length() == 0) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    if(!nudf::util::XmlUtil::GetNodeAttribute(node, PROP_NAME_ATTRIBUTE, _name) || _name.length() == 0) {
        _value = L"";
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    std::transform(_name.begin(), _name.end(), _name.begin(), tolower);

    if(!nudf::util::XmlUtil::GetNodeAttribute(node, PROP_TYPE_ATTRIBUTE, type)) {
        type = PROPTYPENAME[PropString];
    }

    // Handle special PC environmental attributes
    if(0 == _wcsicmp(_name.c_str(), L"CURRENT_TIME.identity")) {
        type = PROPTYPENAME[PropDatetime];
    }
    if(0 == _wcsicmp(_name.c_str(), L"ENVIRONMENT.REMOTE_ACCESS")) {
        type = PROPTYPENAME[PropInt];
    }
    if(0 == _wcsicmp(_name.c_str(), L"ENVIRONMENT.TIME_SINCE_LAST_HEARTBEAT")) {
        type = PROPTYPENAME[PropInt];
    }

    if(0 == _wcsicmp(PROPTYPENAME[PropInt], type.c_str())) {
        if(!nudf::string::ToInt<wchar_t>(_value, &_var.l)) {
            Clear();
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _type = PropInt;
    }
    else if(0 == _wcsicmp(PROPTYPENAME[PropUint], type.c_str())) {
        if(!nudf::string::ToUint<wchar_t>(_value, &_var.ul)) {
            Clear();
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _type = PropUint;
    }
    else if(0 == _wcsicmp(PROPTYPENAME[PropInt64], type.c_str())) {
        if(!nudf::string::ToInt64<wchar_t>(_value, &_var.ll)) {
            Clear();
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _type = PropInt64;
    }
    else if(0 == _wcsicmp(PROPTYPENAME[PropUint64], type.c_str())) {
        if(!nudf::string::ToUint64<wchar_t>(_value, &_var.ull)) {
            Clear();
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _type = PropUint64;
    }
    else if(0 == _wcsicmp(PROPTYPENAME[PropFloat], type.c_str())) {
        if(!nudf::string::ToFloat<wchar_t>(_value, &_var.f)) {
            Clear();
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _type = PropFloat;
    }
    else if(0 == _wcsicmp(PROPTYPENAME[PropBool], type.c_str())) {
        if(!nudf::string::ToBoolean<wchar_t>(_value, &_var.b)) {
            Clear();
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _type = PropBool;
    }
    else if(0 == _wcsicmp(PROPTYPENAME[PropDatetime], type.c_str())) {
        bool utc = false;
        if(!nudf::string::ToSystemTime<wchar_t>(_value, &_var.ft, &utc)) {
            Clear();
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        if(!utc) {
            // Convert to UTC time
            FILETIME  ft = {0, 0};
#ifdef _DEBUG
            std::wstring wsLocalTime = nudf::string::FromSystemTime<wchar_t>(&_var.ft);
#endif
            LocalFileTimeToFileTime(&_var.ft, &ft);
            _var.ft = ft;
#ifdef _DEBUG
            std::wstring wsSystemTime = nudf::string::FromSystemTime<wchar_t>(&ft);
#endif
        }
        _type = PropDatetime;
    }
    else {
        _type = PropString;
    }
}

void CProperty::ToXml(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* parent, _Out_opt_ IXMLDOMNode** node)
{
    if(_type == PropTypeMax || _name.empty() || _value.empty()) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }

    try {

        HRESULT hr = S_OK;
        CComPtr<IXMLDOMElement> spProp;
        CComPtr<IXMLDOMNode> prop;
        std::wstring value;
        std::wstring type = PROPTYPENAME[_type];

        nudf::util::XmlUtil::CreateElement(doc, PROP_NODENAME, &spProp);

        nudf::util::XmlUtil::SetNodeAttribute(doc, spProp, PROP_TYPE_ATTRIBUTE, type);
        nudf::util::XmlUtil::SetNodeAttribute(doc, spProp, PROP_NAME_ATTRIBUTE, _name);
        nudf::util::XmlUtil::SetNodeAttribute(doc, spProp, PROP_VALUE_ATTRIBUTE, _value);

        hr = parent->appendChild(spProp, &prop);
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }
        if(NULL != node) {
            *node = prop.Detach();
        }
    }
    catch(const nudf::CException& e) {
        throw e;
    }
}


//
//  class CPattern
//
CPattern::CPattern() : CProperty(), _method(EQ)
{
}
CPattern::CPattern(const wchar_t* name, const wchar_t* value, PROPTYPE type, PROPMETHOD method) : CProperty(name, value, type), _method(method)
{
}
CPattern::CPattern(const wchar_t* name, int value, PROPMETHOD method) : CProperty(name, value), _method(method)
{
}
CPattern::CPattern(const wchar_t* name, unsigned int value, PROPMETHOD method) : CProperty(name, value), _method(method)
{
}
CPattern::CPattern(const wchar_t* name, __int64 value, PROPMETHOD method) : CProperty(name, value), _method(method)
{
}
CPattern::CPattern(const wchar_t* name, unsigned __int64 value, PROPMETHOD method) : CProperty(name, value), _method(method)
{
}
CPattern::CPattern(const wchar_t* name, float value, PROPMETHOD method) : CProperty(name, value), _method(method)
{
}
CPattern::CPattern(const wchar_t* name, bool value, PROPMETHOD method) : CProperty(name, value), _method(method)
{
}
CPattern::CPattern(const wchar_t* name, const FILETIME& value, PROPMETHOD method) : CProperty(name, value), _method(method)
{
}
CPattern::~CPattern()
{
}

CPattern& CPattern::operator = (const CPattern& prop) throw()
{
    if(this != &prop) {
        switch(prop.GetType())
        {
        case PropString: SetValue(prop.GetValue().c_str()); break;
        case PropInt: SetIntValue(prop.GetIntValue()); break;
        case PropUint: SetUintValue(prop.GetUintValue()); break;
        case PropInt64: SetInt64Value(prop.GetInt64Value()); break;
        case PropUint64: SetUint64Value(prop.GetUint64Value()); break;
        case PropFloat: SetFloatValue(prop.GetFloatValue()); break;
        case PropBool: SetBoolValue(prop.GetBoolValue()); break;
        case PropDatetime: SetDatetimeValue(prop.GetDatetimeValue()); break;
        default: break;
        }
        _method = prop.GetMethod();
    }
    return *this;
}

bool CPattern::SetMethod(PROPMETHOD method) throw()
{
    if(method<0 || method>=PropMethodMax) {
        return false;
    }
    _method = method;
    return true;
}
void CPattern::ToRegex() throw()
{
    if(PropString==_type) {
        _value = nudf::util::regex::WildcardsToRegexEx<wchar_t>(_value);
    }
}

void CPattern::FromXml(_In_ IXMLDOMNode* node)
{
    std::wstring method;
    CProperty::FromXml(node);
    nudf::util::XmlUtil::GetNodeAttribute(node, PROP_METHOD_ATTRIBUTE, method);
    
    // This is to resolve RMS' bug which pass wrong method
    if(0 == _wcsicmp(_name.c_str(), L"ENVIRONMENT.TIME_SINCE_LAST_HEARTBEAT")) {
        _method = LT;
        return;
    }

    for(int i=NE; i<PropMethodMax; i++) {
        if(0 == _wcsicmp(method.c_str(), PROPMETHODNAME[i])) {
            _method = (PROPMETHOD)i;
            return;
        }
    }
    _method = EQ;
}

void CPattern::ToXml(_In_ IXMLDOMDocument* doc, _In_ IXMLDOMNode* parent, _Out_opt_ IXMLDOMNode** node)
{
    CComPtr<IXMLDOMNode> pattern = NULL;

    CProperty::ToXml(doc, parent, &pattern);
    
    try {
        std::wstring method = PROPMETHODNAME[_method];
        nudf::util::XmlUtil::SetNodeAttribute(doc, pattern, PROP_METHOD_ATTRIBUTE, method);
    }
    catch(...) {
        ; // NOTHING
    }

    if(NULL != node) {
        *node = pattern.Detach();
    }
}

bool CPattern::Match(const CProperty& prop) const throw()
{
    bool result = false;

    if(GetType() != prop.GetType()) {
        return false;
    }
    if(GetName() != prop.GetName()) {
        return false;
    }

    switch(prop.GetType())
    {
    case PropString:
        result = Match(prop.GetValue().c_str()); break;
    case PropInt: result = Match(prop.GetIntValue()); break;
    case PropUint: result = Match(prop.GetUintValue()); break;
    case PropInt64: result = Match(prop.GetInt64Value()); break;
    case PropUint64: result = Match(prop.GetUint64Value()); break;
    case PropFloat: result = Match(prop.GetFloatValue()); break;
    case PropBool: result = Match(prop.GetBoolValue()); break;
    case PropDatetime: result = Match(prop.GetDatetimeValue()); break;
    default: break;
    }

    return result;
}

bool CPattern::Match(const wchar_t* value) const throw()
{
    bool result = false;

    if(GetType() != PropString) {
        return false;
    }

    try {
        result = nudf::util::regex::Match<wchar_t>(value, GetValue(), true);
    }
    catch(...) {
        result = false;
    }

    return result;
}
bool CPattern::Match(int value) const throw()
{
    if(GetType() != PropInt) {
        return false;
    }
    switch(_method)
    {
    case NE: return (value != GetIntValue());
    case EQ: return (value == GetIntValue());
    case LT: return (value < GetIntValue());
    case GT: return (value > GetIntValue());
    case LE: return (value <= GetIntValue());
    case GE: return (value >= GetIntValue());
    default:
        break;
    }

    return false;
}
bool CPattern::Match(unsigned int value) const throw()
{
    if(GetType() != PropUint) {
        return false;
    }
    switch(_method)
    {
    case NE: return (value != GetUintValue());
    case EQ: return (value == GetUintValue());
    case LT: return (value < GetUintValue());
    case GT: return (value > GetUintValue());
    case LE: return (value <= GetUintValue());
    case GE: return (value >= GetUintValue());
    default:
        break;
    }

    return false;
}
bool CPattern::Match(__int64 value) const throw()
{
    if(GetType() != PropInt64) {
        return false;
    }
    switch(_method)
    {
    case NE: return (value != GetInt64Value());
    case EQ: return (value == GetInt64Value());
    case LT: return (value < GetInt64Value());
    case GT: return (value > GetInt64Value());
    case LE: return (value <= GetInt64Value());
    case GE: return (value >= GetInt64Value());
    default:
        break;
    }

    return false;
}
bool CPattern::Match(unsigned __int64 value) const throw()
{
    if(GetType() != PropUint64) {
        return false;
    }
    switch(_method)
    {
    case NE: return (value != GetUint64Value());
    case EQ: return (value == GetUint64Value());
    case LT: return (value < GetUint64Value());
    case GT: return (value > GetUint64Value());
    case LE: return (value <= GetUint64Value());
    case GE: return (value >= GetUint64Value());
    default:
        break;
    }

    return false;
}
bool CPattern::Match(float value) const throw()
{
    if(GetType() != PropFloat) {
        return false;
    }

    const float precision = 0.00001f;
    float delta = value - GetFloatValue();
    switch(_method)
    {
    case NE: return (abs(delta) > precision);
    case EQ: return (abs(delta) < precision);
    case LT: return (delta < 0 && abs(delta) > precision);
    case GT: return (delta > 0 && abs(delta) > precision);
    case LE: return (delta < 0);
    case GE: return (delta > 0);
    default:
        break;
    }

    return false;
}
bool CPattern::Match(bool value) const throw()
{
    if(GetType() != PropBool) {
        return false;
    }
    return (EQ == _method) ? (GetBoolValue() == value) : (GetBoolValue() != value);
}
bool CPattern::Match(const FILETIME& value) const throw()
{
    FILETIME ft = {0, 0};
    if(GetType() != PropDatetime) {
        return false;
    }
    ft = GetDatetimeValue();
    switch(_method)
    {
    case NE: return (((PULARGE_INTEGER)(&value))->QuadPart != ((PULARGE_INTEGER)(&ft))->QuadPart);
    case EQ: return (((PULARGE_INTEGER)(&value))->QuadPart == ((PULARGE_INTEGER)(&ft))->QuadPart);
    case LT: return (((PULARGE_INTEGER)(&value))->QuadPart < ((PULARGE_INTEGER)(&ft))->QuadPart);
    case GT: return (((PULARGE_INTEGER)(&value))->QuadPart > ((PULARGE_INTEGER)(&ft))->QuadPart);
    case LE: return (((PULARGE_INTEGER)(&value))->QuadPart <= ((PULARGE_INTEGER)(&ft))->QuadPart);
    case GE: return (((PULARGE_INTEGER)(&value))->QuadPart >= ((PULARGE_INTEGER)(&ft))->QuadPart);
    default:
        break;
    }
    return false;
}