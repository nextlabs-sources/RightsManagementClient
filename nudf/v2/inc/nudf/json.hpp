

#pragma once
#ifndef __NUDF_JSON_HPP__
#define __NUDF_JSON_HPP__


#include <string>
#include <vector>
#include <memory>

namespace NX {


class json_null;
class json_boolean;
class json_number;
class json_string;
class json_array;
class json_object;


class json_value
{
public:
    virtual ~json_value() {}

    typedef enum ValueType {
        ValueNull = 0,
        ValueBoolean,
        ValueNumber,
        ValueString,
        ValueArray,
        ValueObject
    } ValueType;

    static std::shared_ptr<json_value> parse(const std::wstring& s);
    static std::shared_ptr<json_value> parse(const std::string& s);
    static std::shared_ptr<json_value> create_null();
    static std::shared_ptr<json_value> create_boolean(bool v);
    static std::shared_ptr<json_value> create_number(int v);
    static std::shared_ptr<json_value> create_number(unsigned int v);
    static std::shared_ptr<json_value> create_number(__int64 v);
    static std::shared_ptr<json_value> create_number(unsigned __int64 v);
    static std::shared_ptr<json_value> create_number(float v);
    static std::shared_ptr<json_value> create_number(double v);
    static std::shared_ptr<json_value> create_string(const std::wstring& s);
    static std::shared_ptr<json_value> create_string(const std::string& s);
    static std::shared_ptr<json_value> create_array();
    static std::shared_ptr<json_value> create_object();

    virtual bool is_null() const { return false; }
    virtual bool is_boolean() const { return false; }
    virtual bool is_number() const { return false; }
    virtual bool is_string() const { return false; }
    virtual bool is_array() const { return false; }
    virtual bool is_object() const { return false; }

    virtual json_null& as_null() { throw std::exception("not json_null"); }
    virtual json_boolean& as_boolean() { throw std::exception("not json_boolean"); }
    virtual json_string& as_string() { throw std::exception("not json_string"); }
    virtual json_number& as_number() { throw std::exception("not json_number"); }
    virtual json_array& as_array() { throw std::exception("not json_array"); }
    virtual json_object& as_object() { throw std::exception("not json_object"); }
    
    virtual std::wstring serialize() const { return std::wstring(L"{}"); }

protected:
    json_value() : _type(ValueNull) {}
    json_value(ValueType type) : _type(type) {}

private:
    // not copyable
    json_value& operator = (const json_value& other) { return *this; }

private:
    ValueType       _type;
};

class json_null : public json_value
{
public:
    json_null();
    virtual ~json_null();

    virtual bool is_null() const { return true; }
    virtual json_null& as_null() { return *this; }        
    virtual std::wstring serialize() const;

private:
    // not copyable
    json_null& operator = (const json_null& other) { return *this; }

private:
    bool  _boolean;
};

class json_boolean : public json_value
{
public:
    json_boolean();
    json_boolean(bool v);
    virtual ~json_boolean();


    virtual bool is_boolean() const { return true; }
    virtual json_boolean& as_boolean() { return *this; }

    operator bool() const { return _boolean; }
    json_boolean& operator = (bool b) { _boolean = b; return *this; }
    
    virtual std::wstring serialize() const;

private:
    // not copyable
    json_boolean& operator = (const json_boolean& other) { return *this; }

private:
    bool  _boolean;
};

class json_number : public json_value
{
public:
    json_number();
    json_number(int v);
    json_number(long v);
    json_number(unsigned int v);
    json_number(unsigned long v);
    json_number(__int64 v);
    json_number(unsigned __int64 v);
    json_number(float v);
    json_number(double v);
    virtual ~json_number();


    virtual bool is_number() const { return true; }
    virtual json_number& as_number() { return *this; }

    inline bool is_integer() const { return !_float; }
    inline bool is_float() const { return _float; }

    int as_int() const;
    long as_long() const;
    unsigned int as_uint() const;
    __int64 as_int64() const;
    unsigned __int64 as_uint64() const;
    double as_float() const;

    virtual std::wstring serialize() const;

private:
    // not copyable
    json_number& operator = (const json_number& other) { return *this; }

private:
    bool  _float;
    union {
        int              _n;
        long             _l;
        __int64          _ll;
        unsigned int     _u;
        unsigned __int64 _ull;
        double           _f;
    } DUMMYUNIONNAME;
};

class json_string : public json_value
{
public:
    json_string();
    json_string(const std::wstring& s);
    virtual ~json_string();

    virtual bool is_string() const { return true; }
    virtual json_string& as_string() { return *this; }

    inline const std::wstring& get_string() const { return _s; }
    inline void set_string(const std::wstring& s) { _s = s; }

    virtual size_t length() const { return _s.length(); }
    virtual bool empty() const { return _s.empty(); }
    virtual void clear() { _s.clear(); }

    virtual std::wstring serialize() const;

private:
    // not copyable
    json_string& operator = (const json_string& other) { return *this; }

private:
    std::wstring    _s;
};

class json_array : public json_value
{
public:
    json_array();
    virtual ~json_array();

    virtual bool is_array() const { return true; }
    virtual json_array& as_array() { return *this; }

    virtual size_t size() const { return _array.size(); }
    virtual bool empty() const { return _array.empty(); }
    virtual void clear() { _array.clear(); }

    std::vector<std::shared_ptr<json_value>>::iterator begin() { return _array.begin(); }
    std::vector<std::shared_ptr<json_value>>::iterator end() { return _array.end(); }
    std::vector<std::shared_ptr<json_value>>::const_iterator cbegin() const { return _array.cbegin(); }
    std::vector<std::shared_ptr<json_value>>::const_iterator cend() const { return _array.cend(); }

    virtual void push_back(std::shared_ptr<json_value>& v) { _array.push_back(v); }

    virtual void remove(size_t index);
    virtual std::shared_ptr<json_value>& operator [](size_t index);
    virtual const std::shared_ptr<json_value>& operator [](size_t index) const;

    virtual std::wstring serialize() const;

private:
    // not copyable
    json_array& operator = (const json_array& other) { return *this; }

private:
    std::vector<std::shared_ptr<json_value>> _array;
};

class json_object : public json_value
{
public:
    json_object();
    virtual ~json_object();

    virtual bool is_object() const { return true; }
    virtual json_object& as_object() { return *this; }

    virtual size_t size() const { return _object.size(); }
    virtual bool empty() const { return _object.empty(); }
    virtual void clear() { _object.clear(); }

    std::vector<std::pair<std::wstring, std::shared_ptr<json_value>>>::iterator begin() { return _object.begin(); }
    std::vector<std::pair<std::wstring, std::shared_ptr<json_value>>>::iterator end() { return _object.end(); }
    std::vector<std::pair<std::wstring, std::shared_ptr<json_value>>>::const_iterator cbegin() const { return _object.cbegin(); }
    std::vector<std::pair<std::wstring, std::shared_ptr<json_value>>>::const_iterator cend() const { return _object.cend(); }
    
    std::shared_ptr<json_value>& operator [](const std::wstring& key);
    std::vector<std::pair<std::wstring, std::shared_ptr<json_value>>>::iterator find(const std::wstring& key);
    std::vector<std::pair<std::wstring, std::shared_ptr<json_value>>>::const_iterator find(const std::wstring& key) const;
    void remove(const std::wstring& key);

    virtual std::wstring serialize() const;

private:
    // not copyable
    json_object& operator = (const json_object& other) { return *this; }

private:
    std::vector<std::pair<std::wstring, std::shared_ptr<json_value>>> _object;
};


}


#endif