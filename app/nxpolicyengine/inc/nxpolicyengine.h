

#pragma once
#ifndef __NX_POLICY_ENGINE_H__
#define __NX_POLICY_ENGINE_H__


#ifdef NXPOLICYENGINE_EXPORTS
#define NXPOLICYENGINE_API __declspec(dllexport)
#else
#ifdef NXPOLICYENGINE_STATIC
#define NXPOLICYENGINE_API
#else
#define NXPOLICYENGINE_API __declspec(dllimport)
#endif
#endif

#include <string>
#include <algorithm>
#include <vector>
#include <unordered_map>

namespace NXPOLICY {

typedef enum BUNDLE_FORMAT {
    PqlBundle = 0,
    JsonBundle,
    XmlBundle
} BUNDLE_FORMAT;

typedef enum VALUE_TYPE {
    VALUE_INVALID = 0,
    VALUE_INTEGER,
    VALUE_UNSIGNED_INTEGER,
    VALUE_FLOAT,
    VALUE_DATETIME,
    VALUE_DAYSECONDS,
    VALUE_STRING
} VALUE_TYPE;

typedef union _NUMBER_VALUE {
    __int64             _ll;
    unsigned __int64    _ull;
    double              _f;
} NUMBER_VALUE;

// This class is exported from the nxpolicyengine.dll
class property_value
{
public:
    property_value() : _type(VALUE_INVALID) {}
    explicit property_value(int v) : _type(VALUE_INTEGER)
    {
        _value.resize(sizeof(NUMBER_VALUE), 0);
        ((NUMBER_VALUE*)_value.data())->_ll = v;
    }
    explicit property_value(unsigned int v) : _type(VALUE_UNSIGNED_INTEGER)
    {
        _value.resize(sizeof(NUMBER_VALUE), 0);
        ((NUMBER_VALUE*)_value.data())->_ull = v;
    }
    explicit property_value(unsigned long v) : _type(VALUE_UNSIGNED_INTEGER)
    {
        _value.resize(sizeof(NUMBER_VALUE), 0);
        ((NUMBER_VALUE*)_value.data())->_ull = v;
    }
    explicit property_value(__int64 v, VALUE_TYPE type = VALUE_INTEGER) : _type(type)
    {
        _value.resize(sizeof(NUMBER_VALUE), 0);
        ((NUMBER_VALUE*)_value.data())->_ll = v;
        if (_type != VALUE_DATETIME && _type != VALUE_DAYSECONDS && _type != VALUE_INTEGER) {
            _type = VALUE_INTEGER;
        }
    }
    explicit property_value(unsigned __int64 v) : _type(VALUE_UNSIGNED_INTEGER)
    {
        _value.resize(sizeof(NUMBER_VALUE), 0);
        ((NUMBER_VALUE*)_value.data())->_ull = v;
    }
    explicit property_value(double v) : _type(VALUE_FLOAT)
    {
        _value.resize(sizeof(NUMBER_VALUE), 0);
        ((NUMBER_VALUE*)_value.data())->_f = v;
    }
    explicit property_value(bool v) : _type(VALUE_INTEGER)
    {
        _value.resize(sizeof(NUMBER_VALUE), 0);
        ((NUMBER_VALUE*)_value.data())->_ll = v ? 1 : 0;
    }
    explicit property_value(const std::wstring& v) : _type(VALUE_STRING)
    {
        if (!v.empty()) {
            _value.resize(v.length() * sizeof(wchar_t), 0);
            memcpy(_value.data(), v.c_str(), v.length() * sizeof(wchar_t));
        }
    }
    virtual ~property_value() {}

    property_value& operator = (const property_value& other)
    {
        if (this != &other) {
            _type = other.get_type();
            _value = other.get_value();
        }
        return *this;
    }

    virtual void clear()
    {
        _type = VALUE_INVALID;
        _value.clear();
    }

    inline bool empty() const { return (_type == VALUE_INVALID); }

    inline __int64 as_integer() const
    {
        if (VALUE_INTEGER == _type || VALUE_UNSIGNED_INTEGER == _type || VALUE_DATETIME == _type || VALUE_DAYSECONDS == _type) {
            return ((const NUMBER_VALUE*)_value.data())->_ll;
        }
        else if (VALUE_FLOAT == _type) {
            return (__int64)(((const NUMBER_VALUE*)_value.data())->_f);
        }
        else {
            throw std::exception("bad type");
        }
    }

    inline unsigned __int64 as_unsigned_integer() const
    {
        return (unsigned __int64)as_integer();
    }

    inline double as_double() const
    {
        if (VALUE_INTEGER == _type || VALUE_UNSIGNED_INTEGER == _type || VALUE_DATETIME == _type || VALUE_DAYSECONDS == _type) {
            return ((const NUMBER_VALUE*)_value.data())->_ll * 1.0;
        }
        else if (VALUE_FLOAT == _type) {
            return ((const NUMBER_VALUE*)_value.data())->_f;
        }
        else {
            throw std::exception("bad type");
        }
    }

    inline std::wstring as_string() const
    {
        if (VALUE_STRING == _type) {
            if (_value.empty()) {
                return std::wstring();
            }
            else {
                const wchar_t* s = (const wchar_t*)_value.data();
                size_t len = _value.size() / sizeof(wchar_t);
                return std::wstring(s, s + len);
            }
        }
        else {
            throw std::exception("bad type");
        }
    }

    inline VALUE_TYPE get_type() const { return _type; }
    inline const std::vector<unsigned char>& get_value() const { return _value; }

    inline bool is_integer() const { return (VALUE_INTEGER == _type); }
    inline bool is_unsigned_integer() const { return (VALUE_UNSIGNED_INTEGER == _type); }
    inline bool is_date_time() const { return (VALUE_DATETIME == _type); }
    inline bool is_day_seconds() const { return (VALUE_DAYSECONDS == _type); }
    inline bool is_string() const { return (VALUE_STRING == _type); }

private:
    VALUE_TYPE _type;
    std::vector<unsigned char> _value;
};

class property_object
{
public:
    property_object() {}
    explicit property_object(const std::wstring& name, int v) : _value(v), _name(name)
    {
        std::transform(_name.begin(), _name.end(), _name.begin(), tolower);
    }
    explicit property_object(const std::wstring& name, unsigned int v) : _value(v), _name(name)
    {
        std::transform(_name.begin(), _name.end(), _name.begin(), tolower);
    }
    explicit property_object(const std::wstring& name, unsigned long v) : _value(v), _name(name)
    {
        std::transform(_name.begin(), _name.end(), _name.begin(), tolower);
    }
    explicit property_object(const std::wstring& name, __int64 v) : _value(v), _name(name)
    {
        std::transform(_name.begin(), _name.end(), _name.begin(), tolower);
    }
    explicit property_object(const std::wstring& name, unsigned __int64 v) : _value(v), _name(name)
    {
        std::transform(_name.begin(), _name.end(), _name.begin(), tolower);
    }
    explicit property_object(const std::wstring& name, double v) : _value(v), _name(name)
    {
        std::transform(_name.begin(), _name.end(), _name.begin(), tolower);
    }
    explicit property_object(const std::wstring& name, bool v) : _value(v), _name(name)
    {
        std::transform(_name.begin(), _name.end(), _name.begin(), tolower);
    }
    explicit property_object(const std::wstring& name, const std::wstring& v) : _value(v), _name(name)
    {
        std::transform(_name.begin(), _name.end(), _name.begin(), tolower);
    }
    virtual ~property_object() {}

    inline const std::wstring& get_name() const { return _name; }
    inline const property_value& get_value() const { return _value; }
    inline bool empty() const { return _name.empty(); }
    inline property_object& operator = (const property_object& other)
    {
        if (this != &other) {
            _name = other.get_name();
            _value = other.get_value();
        }
        return *this;
    }

private:
    std::wstring    _name;
    property_value  _value;
};

class property_repository
{
public:
    property_repository() : _ignore_nonexist(false)
    {
    }

    property_repository(bool b) : _ignore_nonexist(b)
    {
    }

    virtual ~property_repository()
    {
    }

    typedef std::unordered_map<std::wstring, property_value>::const_iterator CONST_ITERATOR;
    typedef std::unordered_map<std::wstring, property_value>::iterator ITERATOR;
    typedef std::pair<CONST_ITERATOR, CONST_ITERATOR> CONST_RANGE;
    typedef std::pair<ITERATOR, ITERATOR> RANGE;

    inline bool ignore_nonexist()  const { return _ignore_nonexist; }
    inline const std::unordered_map<std::wstring, property_value>& get_repo() const { return _repo; }
    inline std::unordered_map<std::wstring, property_value>& get_repo() { return _repo; }


    inline void insert_value(const std::wstring& name, const property_value& value)
    {
        std::wstring low_name(name);
        std::transform(low_name.begin(), low_name.end(), low_name.begin(), tolower);
        _repo.insert(std::pair<std::wstring, property_value>(low_name, value));
    }

    inline CONST_RANGE find(const std::wstring& name) const
    {
        std::wstring low_name(name);
        std::transform(low_name.begin(), low_name.end(), low_name.begin(), tolower);
        return _repo.equal_range(low_name);
    }

    inline RANGE find(const std::wstring& name)
    {
        std::wstring low_name(name);
        std::transform(low_name.begin(), low_name.end(), low_name.begin(), tolower);
        return _repo.equal_range(low_name);
    }

private:
    std::unordered_map<std::wstring, property_value>    _repo;
    bool    _ignore_nonexist;
};

//NXPOLICYENGINE_API bool evaluate(const condition_expression& express, const std::vector<std::pair<std::wstring, std::wstring>>& conditions);


// This class is exported from the nxpolicyengine.dll
class NXPOLICYENGINE_API policy_engine
{
public:
    policy_engine();
    virtual ~policy_engine();

	// TODO: add your methods here.
};


}

#endif
