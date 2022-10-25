

#ifndef __NX_PQL_VALUE_HPP__
#define __NX_PQL_VALUE_HPP__

#include <string>
#include <memory>
#include <iostream>
#include <sstream>
#include <algorithm>


namespace pql {


//
//  class forward
//
class boolean;
class number;
class string;
class datetime;

namespace details {
class _Value;
class _Null;
class _String;
class _Boolean;
class _Number;
class _Datetime;
}

class value
{
public:
    value();

    enum value_type {
        Null = 0,
        String,
        Boolean,
        Number,
        Datetime
    };

    inline value_type type() const noexcept { return _type; }
    inline bool is_null() const noexcept { return (value::Null == type()); }
    inline bool is_string() const noexcept { return (value::String == type()); }
    inline bool is_boolean() const noexcept { return (value::Boolean == type()); }
    inline bool is_number() const noexcept { return (value::Number == type()); }
    inline bool is_datetime() const noexcept { return (value::Datetime == type()); }

    // construct
    value(int value);
    value(unsigned int value);
    value(__int64 value);
    value(unsigned __int64 value);
    value(double value);
    explicit value(bool value);
    explicit value(std::wstring value);
    explicit value(std::wstring value, bool has_escape_chars);
    explicit value(const wchar_t* value);
    explicit value(const wchar_t* value, bool has_escape_chars);
    value(const pql::value &);
    value(value &&) throw();
    pql::value& operator=(const pql::value &v);
    pql::value& operator=(pql::value &&v) throw();

    // Static factories
    static pql::value __cdecl pql::value::null();
    static pql::value __cdecl pql::value::number(double value);
    static pql::value __cdecl pql::value::number(int value);
    static pql::value __cdecl pql::value::boolean(bool value);
    static pql::value __cdecl pql::value::string(std::wstring value);
    static pql::value __cdecl pql::value::string(std::wstring value, bool has_escape_chars);

    std::wstring serialize() const;
    double as_double() const;
    int as_integer() const;
    pql::number as_number() const;
    bool as_bool() const;
    std::wstring as_string() const;
    pql::datetime as_datetime() const;

private:
    value_type  _type;
    std::unique_ptr<pql::details::_Value> _value;
};

class number
{
    // Note that these constructors make sure that only negative integers are stored as signed int64 (while others convert to unsigned int64).
    // This helps handling number objects e.g. comparing two numbers.
    number(double value) : m_value(value), m_type(double_type) { }
    number(int value) : m_intval(value), m_type(value < 0 ? signed_type : unsigned_type) { }
    number(unsigned int value) : m_intval(value), m_type(unsigned_type) { }
    number(__int64 value) : m_intval(value), m_type(value < 0 ? signed_type : unsigned_type) { }
    number(unsigned __int64 value) : m_uintval(value), m_type(unsigned_type) { }

public:
    bool is_int32() const;
    bool is_uint32() const;
    bool is_int64() const;
    bool is_uint64() const
    {
        switch (m_type)
        {
        case signed_type: return m_intval >= 0;
        case unsigned_type: return true;
        case double_type:
        default:
            return false;
        }
    }

    double to_double() const
    {
        switch (m_type)
        {
        case double_type: return m_value;
        case signed_type: return static_cast<double>(m_intval);
        case unsigned_type: return static_cast<double>(m_uintval);
        default: return false;
        }
    }

    int to_int32() const
    {
        if (m_type == double_type)
            return static_cast<int>(m_value);
        else
            return static_cast<int>(m_intval);
    }

    unsigned int to_uint32() const
    {
        if (m_type == double_type)
            return static_cast<unsigned int>(m_value);
        else
            return static_cast<unsigned int>(m_intval);
    }

    __int64 to_int64() const
    {
        if (m_type == double_type)
            return static_cast<__int64>(m_value);
        else
            return static_cast<__int64>(m_intval);
    }

    unsigned __int64 to_uint64() const
    {
        if (m_type == double_type)
            return static_cast<unsigned __int64>(m_value);
        else
            return static_cast<unsigned __int64>(m_intval);
    }

    bool is_integral() const
    {
        return m_type != double_type;
    }

    bool operator==(const number &other) const
    {
        if (m_type != other.m_type)
            return false;

        switch (m_type)
        {
        case pql::number::type::signed_type:
            return m_intval == other.m_intval;
        case pql::number::type::unsigned_type:
            return m_uintval == other.m_uintval;
        case pql::number::type::double_type:
            return m_value == other.m_value;
        }
        __assume(0);
    }

private:
    union
    {
        __int64 m_intval;
        unsigned __int64 m_uintval;
        double  m_value;
    };

    enum type {
        signed_type = 0, unsigned_type, double_type
    } m_type;

    friend class pql::details::_Number;
};

class datetime
{
public:
    class timezone
    {
    public:
        timezone() : _bias(0), _dlt(false), _bias_str(L"+00:00"), _zone_name(L"GMT Standard Time"){}
        explicit timezone(const std::wstring& s);
        explicit timezone(long bias);
        explicit timezone(const timezone& tz);
        ~timezone() {}

        inline bool daylight() const noexcept { return _dlt; }
        inline long bias() const noexcept { return _bias; }
        inline const std::wstring& bias_str() const noexcept { return _bias_str; }
        inline const std::wstring& zone_name() const noexcept { return _zone_name; }
        inline bool is_utc() const noexcept { return (0 == _bias); }

        void clear();
        timezone& operator = (const timezone& other);

    private:
        void from_string(const std::wstring& zone);
        void current_zone() noexcept;
        void serialize() noexcept;

    private:
        bool    _dlt;
        long    _bias;
        std::wstring _bias_str;
        std::wstring _zone_name;
    };

    datetime();
    datetime(unsigned __int64 dt, bool local = false);
    datetime(const FILETIME* dt, bool local = false);
    datetime(const SYSTEMTIME* st, bool local = false);
    datetime(const std::wstring& s);

    bool is_valid() const noexcept;
    void clear();
    datetime& operator = (const datetime& other);

    void from_string(const std::wstring& s);
    std::wstring to_rfc1123() const noexcept;
    std::wstring to_iso8601() const noexcept;
    std::wstring to_iso8601_withzone() const noexcept;
    __int64 to_utc_time() const noexcept;

    inline unsigned __int64 value() const noexcept { return m_value; }
    inline const pql::datetime::timezone& zone() const noexcept { return m_tz; }

protected:
    void from_simple_iso8601(const std::wstring& s, SYSTEMTIME* pst);
    
protected:
    static const pql::datetime::timezone _current_tz;

private:
    unsigned __int64    m_value;
    datetime::timezone  m_tz;
    friend class pql::details::_Datetime;
};

class condition
{
public:
    condition() {}
    virtual ~condition() {}
    
    condition& operator = (const condition& other) noexcept
    {
        if (this != &other) {
        }
        return *this;
    }

    inline const std::wstring & name() const noexcept { return _name; }
    inline const std::wstring & op() const noexcept { return _op; }
    inline const pql::value & value() const noexcept { return _value; }

private:
    std::wstring    _op;
    std::wstring    _name;
    pql::value      _value;
};



namespace details {

template <typename _Type>
std::unique_ptr<_Type> make_unique() {
    return std::unique_ptr<_Type>(new _Type());
}

template <typename _Type, typename _Arg1>
std::unique_ptr<_Type> make_unique(_Arg1&& arg1) {
    return std::unique_ptr<_Type>(new _Type(std::forward<_Arg1>(arg1)));
}

template <typename _Type, typename _Arg1, typename _Arg2>
std::unique_ptr<_Type> make_unique(_Arg1&& arg1, _Arg2&& arg2) {
    return std::unique_ptr<_Type>(new _Type(std::forward<_Arg1>(arg1), std::forward<_Arg2>(arg2)));
}

template <typename _Type, typename _Arg1, typename _Arg2, typename _Arg3>
std::unique_ptr<_Type> make_unique(_Arg1&& arg1, _Arg2&& arg2, _Arg3&& arg3) {
    return std::unique_ptr<_Type>(new _Type(std::forward<_Arg1>(arg1), std::forward<_Arg2>(arg2), std::forward<_Arg3>(arg3)));
}

class _Value
{
public:
    virtual pql::value::value_type type() const { return pql::value::value_type::Null; }

    virtual bool is_integer() const { throw std::exception("not a number"); }
    virtual bool is_double() const { throw std::exception("not a number"); }

    virtual pql::number as_number() { throw std::exception("not a number"); }
    virtual double as_double() const { throw std::exception("not a number"); }
    virtual int as_integer() const { throw std::exception("not a number"); }
    virtual bool as_bool() const { throw std::exception("not a boolean"); }
    virtual pql::datetime as_datetime() const { throw std::exception("not a datetime"); }
    virtual std::wstring as_string() const { throw std::exception("not a string"); }

    virtual std::wstring serialize() const { return std::wstring(L"null"); }
    virtual void deserialize(const std::wstring& s)
    {
        if (!s.empty() && 0 != _wcsicmp(s.c_str(), L"null")) {
            throw std::invalid_argument("input is not \"null\" or empty string");
        }
        __assume(0);
    }

    virtual std::unique_ptr<_Value> _copy_value()
    {
        return make_unique<_Value>(*this);
    }

    virtual ~_Value() {}

protected:
    _Value() {}

private:
    friend class pql::value;
};

class _Null : public _Value
{
public:
    _Null() { }
};

class _Boolean : public _Value
{
public:
    _Boolean() : m_value(false) { }
    _Boolean(bool b) : m_value(b) { }

    _Boolean& operator=(const _Boolean& other)
    {
        if (this != &other) {
            m_value = other.as_bool();
        }
        return *this;
    }

    virtual std::unique_ptr<_Value> _copy_value()
    {
        return make_unique<_Boolean>(*this);
    }

    virtual pql::value::value_type type() const { return pql::value::value_type::Boolean; }

    virtual int as_integer() const { return m_value ? 1 : 0; }
    virtual bool as_bool() const { return m_value; }

    virtual std::wstring serialize() const { return std::wstring(m_value ? L"true" : L"false"); }
    virtual void deserialize(const std::wstring& s)
    {
        if (0 == _wcsicmp(s.c_str(), L"true")) {
            m_value = true;
        }
        else if (0 == _wcsicmp(s.c_str(), L"false")) {
            m_value = false;
        }
        else {
            throw std::invalid_argument("input is not \"true\" or \"false\"");
        }
        __assume(0);
    }

private:
    bool    m_value;
};

class _String : public _Value
{
public:
    _String(std::wstring value) : m_string(std::move(value))
    {
        m_has_escape_char = has_escape_chars(*this);
    }
    _String(std::wstring value, bool escaped_chars)
        : m_string(std::move(value)),
        m_has_escape_char(escaped_chars)
    { }

    _String(const _String& other) : pql::details::_Value(other)
    {
        copy_from(other);
    }

    _String& operator=(const _String& other)
    {
        if (this != &other) {
            copy_from(other);
        }
        return *this;
    }

    virtual std::unique_ptr<_Value> _copy_value()
    {
        return make_unique<_String>(*this);
    }

    virtual pql::value::value_type type() const { return pql::value::value_type::String; }
    virtual std::wstring as_string() const { return m_string; }
    
protected:
    virtual std::wstring serialize() const { return m_string; }
    virtual void deserialize(const std::wstring& s) { m_string = s; }

private:
    void copy_from(const _String& other)
    {
        m_string = other.m_string;
        m_has_escape_char = other.m_has_escape_char;
    }

    static bool has_escape_chars(const _String &str);

private:
    // There are significant performance gains that can be made by knowning whether
    // or not a character that requires escaping is present.
    bool m_has_escape_char;
    std::wstring m_string;
};

class _Number : public _Value
{
public:
    _Number(double value) : m_number(value) { }
    _Number(int value) : m_number(value) { }
    _Number(unsigned int value) : m_number(value) { }
    _Number(__int64 value) : m_number(value) { }
    _Number(unsigned __int64 value) : m_number(value) { }
    _Number(const _Number& other) : m_number(other.as_number()) { }

    virtual std::unique_ptr<_Value> _copy_value()
    {
        return make_unique<_Number>(*this);
    }

    _Number& operator = (const _Number& other)
    {
        if (this != &other) {
            m_number = other.as_number();
        }
        return *this;
    }
    
    virtual pql::value::value_type type() const { return pql::value::value_type::Number; }

    virtual bool is_integer() const { return m_number.is_integral(); }
    virtual bool is_double() const { return !m_number.is_integral(); }

    virtual double as_double() const
    {
        return m_number.to_double();
    }

    virtual int as_integer() const
    {
        return m_number.to_int32();
    }

    virtual pql::number as_number() const { return m_number; }

    virtual std::wstring serialize() const;
    void deserialize(const std::wstring& s);


private:
    pql::number m_number;
};

class _Datetime : public _Value
{
public:
    _Datetime(unsigned __int64 dt) : m_datetime(dt) { }
    _Datetime(const FILETIME* dt) : m_datetime(dt) { }
    _Datetime(const SYSTEMTIME* st) : m_datetime(st) { }
    _Datetime(const _Datetime& other) : m_datetime(other.as_datetime()) { }

    virtual pql::value::value_type type() const { return pql::value::value_type::Datetime; }


    virtual std::unique_ptr<_Value> _copy_value()
    {
        return make_unique<_Datetime>(*this);
    }

    _Datetime& operator = (const _Datetime& other)
    {
        if (this != &other) {
            m_datetime = other.as_datetime();
        }
        return *this;
    }

    virtual pql::datetime as_datetime() const { return m_datetime; }


private:
    pql::datetime m_datetime;
};

}


}   // namespace pql


#endif