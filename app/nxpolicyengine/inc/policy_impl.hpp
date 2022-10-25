

#pragma once
#ifndef __NX_POLICY_ENGINE_IMPL_HPP__
#define __NX_POLICY_ENGINE_IMPL_HPP__

#include <regex>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <memory>
#include <fstream>
#include <unordered_map>

#include <boost/noncopyable.hpp>

#include "nxpolicyengine.h"

namespace NXPOLICY_IMPL {

class condition_value : public NXPOLICY::property_value
{
public:
    condition_value();
    explicit condition_value(int v);
    explicit condition_value(unsigned int v);
    explicit condition_value(unsigned long v);
    explicit condition_value(__int64 v, NXPOLICY::VALUE_TYPE type = NXPOLICY::VALUE_INTEGER);
    explicit condition_value(unsigned __int64 v);
    explicit condition_value(double v);
    explicit condition_value(bool v);
    explicit condition_value(const std::wstring& v);
    virtual ~condition_value();

    virtual void clear();
    condition_value& operator = (const condition_value& other);
    std::wstring serialize() const;

    inline const std::wregex& get_regex() const { return _regex; }
    
    bool same_as(const condition_value& other) const noexcept;
    bool operator > (const property_value& other) const noexcept;
    bool operator >= (const property_value& other) const noexcept;
    bool operator < (const property_value& other) const noexcept;
    bool operator <= (const property_value& other) const noexcept;
    bool operator == (const property_value& other) const noexcept;

protected:
    inline int double_compare(double x, double y) const
    {
        double maxXY = max(std::fabs(x), std::fabs(y));
        return (std::fabs(x - y) <= std::numeric_limits<double>::epsilon()*maxXY) ? 0 : (x > y ? 1 : -1);
    }

    inline int integer_compare(__int64 x, __int64 y) const
    {
        return (x == y) ? 0 : (x > y ? 1 : -1);
    }

    inline int unsigned_integer_compare(unsigned __int64 x, unsigned __int64 y) const
    {
        return (x == y) ? 0 : (x > y ? 1 : -1);
    }

    int compare_number(const property_value& other) const;

private:
    std::wregex _regex;
};

typedef enum LOGIC_OPERATOR {
    NIL = 0,
    AND,
    OR
} LOGIC_OPERATOR;

class condition_object
{
public:
    condition_object() {}
    virtual ~condition_object() {}


    virtual bool evaluate(const NXPOLICY::property_repository& properties) const = 0;
    virtual std::wstring serialize() const = 0;

    virtual bool is_expression() const = 0;
    virtual bool is_const_value() const = 0;
    virtual bool is_dynamic_value() const = 0;

private:
    // copy is not allowed
    condition_object& operator = (const condition_object& other) { return *this; }
};

class condition_map
{
public:
    condition_map();
    virtual ~condition_map();

    void insert(const std::wstring& s, const std::shared_ptr<condition_object>& sp);
    void remove(const std::wstring& s);
    std::shared_ptr<condition_object> find(const std::wstring& s);

    condition_map& operator = (const condition_map& other);

    inline bool empty() const { return _map.empty(); }
    inline void clear() { _map.clear(); }
    inline const std::map<std::wstring, std::shared_ptr<condition_object>>& get_map() const { return _map; }

private:
    std::map<std::wstring, std::shared_ptr<condition_object>>   _map;
};


class condition_expression_object : public condition_object
{
public:
    virtual ~condition_expression_object();

    static condition_expression_object* create_condition(bool opposite, LOGIC_OPERATOR relationship);

    virtual bool evaluate(const NXPOLICY::property_repository& properties) const;
    virtual std::wstring serialize() const;
    virtual bool is_expression() const { return true; }
    virtual bool is_const_value() const { return false; }
    virtual bool is_dynamic_value() const { return false; }

    inline LOGIC_OPERATOR get_relationship() const { return _relationship; }
    inline const std::list<std::shared_ptr<condition_object>>& get_children() const { return _children; }
    inline std::list<std::shared_ptr<condition_object>>& get_children() { return _children; }
    inline void set_relationship(LOGIC_OPERATOR op) { _relationship = op; }
    inline void insert_condition(std::shared_ptr<condition_object> sp) { _children.push_back(sp); }
    inline bool is_and() const { return (NXPOLICY_IMPL::AND == _relationship); }
    inline bool is_or() const { return (NXPOLICY_IMPL::OR == _relationship); }
    inline bool is_nil() const { return (NXPOLICY_IMPL::NIL == _relationship); }


private:
    inline bool is_opposite() const { return _opposite; }

private:
    condition_expression_object();
    condition_expression_object(bool opposite, LOGIC_OPERATOR relationship);

private:
    std::list<std::shared_ptr<condition_object>> _children;
    LOGIC_OPERATOR  _relationship;
    bool _opposite;
};

typedef enum COMPARE_OPERATOR {
    EqualTo = 0,
    NotEqualTo,
    GreaterThan,
    GreaterThanOrEqualTo,
    LessThan,
    LessThanOrEqualTo
} COMPARE_OPERATOR;

class condition_value_object : public condition_object
{
public:
    virtual ~condition_value_object() {}

    static condition_value_object* create_condition(const std::wstring& name, COMPARE_OPERATOR op, int v);
    static condition_value_object* create_condition(const std::wstring& name, COMPARE_OPERATOR op, unsigned int v);
    static condition_value_object* create_condition(const std::wstring& name, COMPARE_OPERATOR op, unsigned long v);
    static condition_value_object* create_condition(const std::wstring& name, COMPARE_OPERATOR op, __int64 v, NXPOLICY::VALUE_TYPE type = NXPOLICY::VALUE_INTEGER);
    static condition_value_object* create_condition(const std::wstring& name, COMPARE_OPERATOR op, unsigned __int64 v);
    static condition_value_object* create_condition(const std::wstring& name, COMPARE_OPERATOR op, double v);
    static condition_value_object* create_condition(const std::wstring& name, COMPARE_OPERATOR op, bool v);
    static condition_value_object* create_condition(const std::wstring& name, COMPARE_OPERATOR op, const std::wstring& v);

    virtual bool evaluate(const NXPOLICY::property_repository& properties) const;
    virtual std::wstring serialize() const;
    virtual bool is_expression() const { return false; }
    virtual bool is_const_value() const { return false; }
    virtual bool is_dynamic_value() const { return false; }


    bool is_same(condition_value_object& other) const noexcept
    {
        return (get_property_name() == other.get_property_name() && get_cmp_operator() == other.get_cmp_operator() && get_cmp_value().same_as(other.get_cmp_value()));
    }

    inline const std::wstring& get_property_name() const { return _property_name; }
    inline COMPARE_OPERATOR get_cmp_operator() const { return _cmp_operator; }
    inline const condition_value& get_cmp_value() const { return _cmp_value; }

private:
    condition_value_object();
    explicit condition_value_object(const std::wstring& name, COMPARE_OPERATOR op, int v);
    explicit condition_value_object(const std::wstring& name, COMPARE_OPERATOR op, unsigned int v);
    explicit condition_value_object(const std::wstring& name, COMPARE_OPERATOR op, unsigned long v);
    explicit condition_value_object(const std::wstring& name, COMPARE_OPERATOR op, __int64 v, NXPOLICY::VALUE_TYPE type = NXPOLICY::VALUE_INTEGER);
    explicit condition_value_object(const std::wstring& name, COMPARE_OPERATOR op, unsigned __int64 v);
    explicit condition_value_object(const std::wstring& name, COMPARE_OPERATOR op, double v);
    explicit condition_value_object(const std::wstring& name, COMPARE_OPERATOR op, bool v);
    explicit condition_value_object(const std::wstring& name, COMPARE_OPERATOR op, const std::wstring& v);

    bool compare(const NXPOLICY::property_value& other) const noexcept;

private:
    std::wstring        _property_name;
    COMPARE_OPERATOR    _cmp_operator;
    condition_value     _cmp_value;
};

class condition_const_value_object : public condition_object
{
public:
    virtual ~condition_const_value_object() {}

    static condition_const_value_object* create_condition(bool v);

    virtual bool evaluate(const NXPOLICY::property_repository& properties) const;
    virtual std::wstring serialize() const;
    virtual bool is_expression() const { return false; }
    virtual bool is_const_value() const { return true; }
    virtual bool is_dynamic_value() const { return false; }

private:
    condition_const_value_object();
    explicit condition_const_value_object(bool v);

private:
    const bool _const_value;
};

class condition_dynamic_value_object : public condition_object
{
public:
    virtual ~condition_dynamic_value_object() {}

    static condition_dynamic_value_object* create_condition(const std::wstring& property_name, COMPARE_OPERATOR op, const std::wstring& cmp_property_name);

    virtual bool evaluate(const NXPOLICY::property_repository& properties) const;
    virtual std::wstring serialize() const;
    virtual bool is_expression() const { return false; }
    virtual bool is_const_value() const { return false; }
    virtual bool is_dynamic_value() const { return true; }

    inline const std::wstring& get_property_name() const { return _property_name; }
    inline COMPARE_OPERATOR get_cmp_operator() const { return _cmp_operator; }
    inline const std::wstring& get_cmp_property_name() const { return _cmp_property_name; }

private:
    condition_dynamic_value_object();
    explicit condition_dynamic_value_object(const std::wstring& property_name, COMPARE_OPERATOR op, const std::wstring& cmp_property_name);

private:
    std::wstring        _property_name;
    COMPARE_OPERATOR    _cmp_operator;
    std::wstring _cmp_property_name;
};

class obligation_object : boost::noncopyable
{
public:
    obligation_object();
    obligation_object(const std::wstring& id, const std::map<std::wstring, std::wstring>& parameters);
    virtual ~obligation_object();

    bool same_as(std::shared_ptr<obligation_object>& other) const;

    inline const std::wstring& get_id() const { return _id; }
    inline const std::map<std::wstring, std::wstring>& get_parameters() const { return _parameters; }

private:
    std::wstring    _id;
    std::map<std::wstring, std::wstring> _parameters;
};

class policy_object : boost::noncopyable
{
public:
    typedef enum POLICY_ACTION {
        ACTION_UNKNOWN = 0,
        REVOKE_RIGHTS,
        GRANT_RIGHTS
    } POLICY_ACTION;
    typedef enum POLICY_STRING_TYPE {
        STR_UNKNOWN = 0,
        STR_TITLE,
        STR_ATTRIBUTE,
        STR_RESOURCE,
        STR_RIGHTS,
        STR_RECIPIENT,
        STR_SUBJECT,
        STR_ENVIRONMENT,
        STR_ACTION,
        STR_OBLIGATION
    } POLICY_STRING_TYPE;


    policy_object();
    policy_object(const std::wstring& id,
        const std::wstring& description,
        POLICY_ACTION action,
        unsigned __int64 rights,
        std::shared_ptr<condition_object> subject_condition,
        std::shared_ptr<condition_object> resource_condition,
        std::shared_ptr<condition_object> environment_condition
        );
    policy_object(POLICY_ACTION action, unsigned __int64 rights, const std::map<POLICY_STRING_TYPE, std::wstring>& elements);
    virtual ~policy_object();

    std::wstring serialize() const;
    unsigned __int64 evaluate(const NXPOLICY::property_repository& repo) const;

    inline const std::wstring& get_id() const { return _id; }
    inline const std::wstring& get_description() const { return _description; }
    inline POLICY_ACTION get_action() const { return _action; }
    inline unsigned __int64 get_rights() const { return _rights; }
    inline std::shared_ptr<condition_object> get_subject_condition() const { return _subject_condition; }
    inline std::shared_ptr<condition_object> get_resource_condition() const { return _resource_condition; }
    inline std::shared_ptr<condition_object> get_environment_condition() const { return _environment_condition; }


private:
    void parse(const std::map<POLICY_STRING_TYPE, std::wstring>& policy_strings);
    void parse_title(const std::wstring& s);
    void parse_subject_condition(const std::wstring& s);
    void parse_resource_condition(const std::wstring& s);
    void parse_environment_condition(const std::wstring& s);
    void parse_rights(const std::wstring& s);
    void parse_obligation(const std::wstring& s);

    std::wstring serialize_action() const;
    std::wstring serialize_rights() const;
    std::wstring serialize_subject_condition() const;
    std::wstring serialize_resource_condition() const;
    std::wstring serialize_environment_condition() const;

private:
    POLICY_ACTION       _action;
    const unsigned __int64 _rights;
    std::wstring        _id;
    std::wstring        _description;
    std::shared_ptr<condition_object>   _subject_condition;     // user, application and host
    std::shared_ptr<condition_object>   _resource_condition;    // resource condition
    std::shared_ptr<condition_object>   _environment_condition; // resource condition
};

class policy_bundle : boost::noncopyable
{
public:
    policy_bundle();
    policy_bundle(const std::map<std::wstring, std::shared_ptr<policy_object>>& p) : _policy_map(p) {}
    virtual ~policy_bundle();

    void load(const std::wstring& s);   // default is in JSON format
    std::wstring serialize() const; // serialize to JSON format

    inline const std::map<std::wstring, std::shared_ptr<policy_object>>& get_policy_map() const { return _policy_map; }

private:
    std::map<std::wstring, std::shared_ptr<policy_object>>  _policy_map;
};

}


#endif

