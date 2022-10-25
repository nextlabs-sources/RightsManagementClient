
#ifndef __NXSERV_POLICY_HPP__
#define __NXSERV_POLICY_HPP__


#include <string>
#include <map>
#include <vector>
#include <memory>
#include <algorithm>

#include <boost\algorithm\string.hpp>

namespace NX {
namespace EVAL {


namespace details {
struct pql_grammar;
class expression;
class condition;
}

class policy;
class obligation;


class value_object
{
public:
    enum value_type {
        value_string = 0,
        value_boolean,
        value_signed_integer,
        value_unsigned_integer,
        value_float,
        value_date
    };

public:
    value_object();
    explicit value_object(int v);
    explicit value_object(unsigned int v);
    explicit value_object(__int64 v);
    explicit value_object(unsigned __int64 v);
    explicit value_object(bool v);
    explicit value_object(float v);
    explicit value_object(double v);
    explicit value_object(const SYSTEMTIME& v, bool local = false);
    explicit value_object(FILETIME v, bool local = false);
    explicit value_object(const std::string& v, bool expandable = false);
    explicit value_object(const std::wstring& v, bool expandable = false);
    ~value_object();

    inline bool empty() const noexcept { return _s.empty(); }
    inline void clear() noexcept
    {
        _s.clear();
        _v.u = 0;
        _t = value_string;
        _expandable = false;
    }
    inline value_object& operator = (const value_object& other) noexcept
    {
        if (this != &other) {
            clear();
            _t = other.type();
            _s = other.serialize();
            _expandable = other.expandable();
            if (!other.is_string()) {
                if (other.is_float()) {
                    _v.b = other.as_double();
                }
                else {
                    if (other.is_signed_integer()) {
                        _v.n = other.as_int64();
                    }
                    else {
                        _v.u = other.as_uint64();
                    }
                }
            }
        }
        return *this;
    }

    bool operator == (const value_object& other) const noexcept;
    bool operator != (const value_object& other) const noexcept;
    bool operator > (const value_object& other) const noexcept;
    bool operator < (const value_object& other) const noexcept;
    bool operator >= (const value_object& other) const noexcept;
    bool operator <= (const value_object& other) const noexcept;
    bool has(const value_object& other) const noexcept;
    bool contains(const value_object& other) const noexcept;

    inline value_type type() const noexcept { return _t; }
    inline bool is_boolean() const noexcept { return (type() == value_boolean); }
    inline bool is_integer() const noexcept { return (is_signed_integer() || is_unsigned_integer()); }
    inline bool is_signed_integer() const noexcept { return (type() == value_signed_integer); }
    inline bool is_unsigned_integer() const noexcept { return (type() == value_unsigned_integer); }
    inline bool is_float() const noexcept { return (type() == value_float); }
    inline bool is_number() const noexcept { return (is_integer() || is_float()); }
    inline bool is_string() const noexcept { return (type() == value_string); }
    inline bool is_date() const noexcept { return (type() == value_date); }
    inline bool expandable() const noexcept { return _expandable; }


    int as_int() const;
    bool as_boolean() const;
    __int64 as_int64() const;
    unsigned int as_uint() const;
    unsigned __int64 as_uint64() const;
    float as_float() const;
    double as_double() const;
    FILETIME as_date(bool local = false) const;
    unsigned __int64 as_date_uint64(bool local = false) const;
    std::wstring serialize() const noexcept;

    enum escape_type {
        none_escape = 0,
        wildcard,
        nextlabs_wildcard
    };

    static value_object parse(const std::wstring& v, escape_type escape = none_escape);
    static void remove_escape(std::wstring& s);
    static void wildcards_to_regex(std::wstring& s);
    static void nextlabs_wildcards_to_regex(std::wstring& s);
    static std::wstring remove_escape_copy(const std::wstring& s);
    static std::wstring wildcards_to_regex_copy(const std::wstring& s);
    static std::wstring nextlabs_wildcards_to_regex_copy(const std::wstring& s);
    
private:
    union {
        __int64             n;
        unsigned __int64    u;
        double              b;
    }               _v;
    std::wstring    _s;
    value_type      _t;
    bool            _expandable;
};

class pattern_object : public value_object
{
public:
    pattern_object() {}
    explicit pattern_object(int v) : value_object(v) {}
    explicit pattern_object(unsigned int v) : value_object(v) {}
    explicit pattern_object(__int64 v) : value_object(v) {}
    explicit pattern_object(unsigned __int64 v) : value_object(v) {}
    explicit pattern_object(bool v) : value_object(v) {}
    explicit pattern_object(float v) : value_object(v) {}
    explicit pattern_object(double v) : value_object(v) {}
    explicit pattern_object(const SYSTEMTIME& v, bool local = false) : value_object(v, local) {}
    explicit pattern_object(FILETIME v, bool local = false) : value_object(v, local) {}
    explicit pattern_object(const std::string& v) : value_object(v, true) {}
    explicit pattern_object(const std::wstring& v) : value_object(v, true) {}
    ~pattern_object() {}

private:
    bool operator == (const value_object& other) { return false; }
};

class attribute_multimap
{
public:
    typedef std::multimap<std::wstring, value_object>::iterator iterator;
    typedef std::multimap<std::wstring, value_object>::const_iterator const_iterator;
    typedef std::pair<std::wstring, value_object> value_type;;
    attribute_multimap() {}
    attribute_multimap(const std::wstring& name) : _name(name) {}
    virtual ~attribute_multimap() {}

    inline const std::wstring& name() const noexcept { return _name; }
    inline bool empty() const noexcept { return _map.empty(); }
    inline void clear() noexcept { _map.clear(); }

    inline iterator begin() noexcept { return _map.begin(); }
    inline const_iterator cbegin() const noexcept { return _map.cbegin(); }
    inline iterator end() noexcept { return _map.end(); }
    inline const_iterator cend() const noexcept { return _map.cend(); }

    virtual std::vector<value_object> get(const std::wstring& key) const noexcept;
    virtual value_object get_single(const std::wstring& key) const noexcept;
    virtual void insert(const std::wstring& key, const value_object& value) noexcept;

protected:
    std::multimap<std::wstring, value_object>   _map;

private:
    std::wstring    _name;
};

class attribute_map : public attribute_multimap
{
public:
    attribute_map() : attribute_multimap(){}
    attribute_map(const std::wstring& name) : attribute_multimap(name) {}
    virtual ~attribute_map() {}
    
    bool exists(const std::wstring& key) const noexcept;
    const value_object& at(const std::wstring& key) const;

    virtual void insert(const std::wstring& key, const value_object& value) noexcept;

private:
    virtual std::vector<value_object> get(const std::wstring& key) const noexcept { return std::vector<value_object>(); };
    virtual value_object get_single(const std::wstring& key) const noexcept { return value_object(); }
};

class rights
{
public:
    rights() : _rights(0) {}
    rights(const rights& r) : _rights(r.get()) {}
    rights(unsigned __int64 r) : _rights(r) {}
    rights(const std::wstring& rs) : _rights(0) { from_string(rs); }
    rights(const std::vector<std::wstring/*name*/>& rsv);
    virtual ~rights() {}

    inline unsigned __int64 get() const noexcept { return _rights; }
    inline operator unsigned __int64 () const noexcept { return _rights; }
    inline void clear() noexcept { _rights = 0; }
    inline bool empty() const noexcept { return (_rights == 0); }

    inline rights& operator = (const rights& other) noexcept { if(this != &other) _rights = other.get(); return *this; }
    inline rights& operator = (unsigned __int64 other) noexcept { _rights = other; return *this; }
    inline bool operator == (unsigned __int64 other) const noexcept { return (_rights == other); }
    inline rights operator + (unsigned __int64 other) const noexcept { return rights(_rights | other); }
    inline rights& operator += (unsigned __int64 other) noexcept { _rights |= other; return *this; }
    inline rights operator - (unsigned __int64 other) const noexcept { return rights(_rights & (~other)); }
    inline rights& operator -= (unsigned __int64 other) noexcept { _rights &= (~other); return *this; }

    inline bool has(unsigned __int64 right) const noexcept { return (right == (_rights & right)); }
    inline bool has_any(unsigned __int64 right) const noexcept { return (0 != (_rights & right)); }

    bool full_rights() const noexcept;
    void from_string(const std::wstring& rs) noexcept;
    void from_string_array(const std::vector<std::wstring>& v) noexcept;
    std::wstring serialize(const std::wstring& separator = std::wstring(L",")) const noexcept;


private:
    unsigned __int64    _rights;
};

class eval_result
{
public:
    eval_result(unsigned __int64 id) : _eval_id(id) {}
    virtual ~eval_result() {}

    inline unsigned __int64 id() const noexcept { return _eval_id; }
    inline const rights& grant_rights() const noexcept { return _grant_rights; }
    inline void grant_rights(unsigned __int64 r) noexcept { _grant_rights += r; }
    inline void grant_rights(const std::wstring& rs) noexcept { _grant_rights += rights(rs); }
    inline void grant_rights(const std::vector<std::wstring>& v) noexcept { _grant_rights += rights(v); }
    inline const rights& revoke_rights() const noexcept { return _revoke_rights; }
    inline void revoke_rights(unsigned __int64 r) noexcept { _revoke_rights += r; }
    inline void revoke_rights(const std::wstring& rs) noexcept { _revoke_rights += rights(rs); }
    inline void revoke_rights(const std::vector<std::wstring>& v) noexcept { _revoke_rights += rights(v); }
    inline rights final_rights() const noexcept { rights fr = _grant_rights; fr -= _revoke_rights; return fr; }

    inline const std::map<std::wstring, std::shared_ptr<policy>>& hit_policies() const noexcept { return _policies; }
    inline const std::map<std::wstring, std::shared_ptr<obligation>>& hit_obligations() const noexcept { return _obligations; }

    void hit_policy(std::shared_ptr<policy> p) noexcept;
    void hit_obligation(std::shared_ptr<obligation> p) noexcept;

protected:
    eval_result() : _eval_id(-1) {}

private:
    unsigned __int64 _eval_id;
    unsigned long _eval_mask;
    rights      _grant_rights;
    rights      _revoke_rights;
    std::map<std::wstring, std::shared_ptr<policy>> _policies;
    std::map<std::wstring, std::shared_ptr<obligation>> _obligations;
};

class eval_object
{
public:
    eval_object();
    eval_object(unsigned long mask);
    virtual ~eval_object();

    inline unsigned __int64 id() const { return _eval_id; }
    inline const std::wstring& user_context() const noexcept { return _user_context; }
    inline const attribute_multimap& attributes() const noexcept { return _attributes; }
    inline attribute_multimap& attributes() noexcept { return _attributes; }
    inline std::shared_ptr<eval_result> result() const noexcept { return _result; };

    inline void set_mask(unsigned long mask) noexcept { _mask = mask; }
    inline void set_user_context(const std::wstring& context) noexcept { _user_context = context; }


    inline bool eval_user() const noexcept { return (eval_mask_user == (eval_mask_user & _mask)); }
    inline bool eval_app() const noexcept { return (eval_mask_app == (eval_mask_app & _mask)); }
    inline bool eval_host() const noexcept { return (eval_mask_host == (eval_mask_host & _mask)); }
    inline bool eval_res() const noexcept { return (eval_mask_res == (eval_mask_res & _mask)); }
    inline bool eval_env() const noexcept { return (eval_mask_env == (eval_mask_env & _mask)); }

    
    static const unsigned long eval_mask_user;
    static const unsigned long eval_mask_app;
    static const unsigned long eval_mask_host;
    static const unsigned long eval_mask_res;
    static const unsigned long eval_mask_env;

private:
    unsigned __int64 generate_id();
    
private:
    unsigned __int64 _eval_id;
    unsigned long _mask;
    std::wstring       _user_context;
    attribute_multimap _attributes;
    std::shared_ptr<eval_result> _result;
};

class obligation
{
public:
    obligation();
    obligation(const std::wstring& name, const std::map<std::wstring, std::wstring>& parameters);
    ~obligation();

    obligation& operator = (const obligation& ob) noexcept;
    std::wstring serialize() const noexcept;

    inline const std::wstring& name() const noexcept { return _name; }
    inline const std::map<std::wstring, std::wstring>& parameters() const noexcept { return _parameters; }
    inline const std::wstring& hash() const noexcept { return _hash; }
    inline bool empty() const noexcept { return _hash.empty(); }
    inline void clear() noexcept
    {
        _name.clear();
        _parameters.clear();
        _hash.clear();
    }

protected:
    std::wstring calc_hash();

private:
    std::wstring    _name;
    std::map<std::wstring, std::wstring> _parameters;
    std::wstring    _hash;
};

class policy_bundle
{
public:
    policy_bundle();
    virtual ~policy_bundle();

    inline const std::wstring& time() const noexcept { return _time; }
    inline const std::map<std::wstring, std::shared_ptr<policy>>& policy_map() const noexcept { return _policy_map; }
    inline std::map<std::wstring, std::shared_ptr<policy>>& policy_map() noexcept { return _policy_map; }
    inline const std::map<std::wstring, std::shared_ptr<details::condition>>& condition_map() const noexcept { return _condition_map; }
    inline const std::map<std::wstring, std::shared_ptr<obligation>>& obligation_map() const noexcept { return _obligation_map; }
    inline const std::map<std::wstring/*user.id*/, std::pair<std::wstring/*user context*/, std::vector<std::wstring>/*user group id*/>> user_group_map() const { return _user_group_map; }
    inline std::map<std::wstring/*user.id*/, std::pair<std::wstring/*user context*/, std::vector<std::wstring>/*user group id*/>> user_group_map() { return _user_group_map; }
    inline const std::pair<std::wstring/*user context*/, std::vector<std::wstring>/*user group id*/> unknown_user_group_map() const { return _unknown_user_group; }
    inline std::pair<std::wstring/*user context*/, std::vector<std::wstring>/*user group id*/> unknown_user_group_map() { return _unknown_user_group; }
    inline const std::pair<std::wstring/*user context*/, std::vector<std::wstring>/*user group id*/>& user_group(const std::wstring& user_id) const noexcept
    {
        auto pos = _user_group_map.find(user_id);
        return (pos == _user_group_map.end()) ? _unknown_user_group : (*pos).second;
    }

    std::wstring local_bundle_time() const noexcept;
    void clear() noexcept;
    bool empty() const noexcept;

    static policy_bundle load(const std::wstring& s);
    static policy_bundle load_pql_file(const std::wstring& file);
    
public:
    class parser
    {
    public:
        parser() {}
        virtual ~parser() {}
        virtual policy_bundle parse(const std::string& s /*utf8*/) = 0;
        virtual policy_bundle parse(const std::wstring& ws /*utf16*/) = 0;
    };
    class xml_parser : public policy_bundle::parser
    {
    public:
        xml_parser();
        virtual ~xml_parser();
        virtual policy_bundle parse(const std::string& s /*utf8*/);
        virtual policy_bundle parse(const std::wstring& ws /*utf16*/);

    protected:
        void add_policy_conditions(policy_bundle& bundle, details::expression* expr, const std::vector<std::vector<std::wstring>>& comp_map, bool exclude);

    private:
        std::map<std::vector<unsigned char>, std::shared_ptr<details::condition>> _condition_map;
        std::map<std::wstring/*sid*/, std::pair<std::wstring/*context*/, std::vector<std::wstring/*group_id*/>>> _user_group_map;
        std::map<std::wstring/*id*/, std::vector<std::shared_ptr<details::condition>>> _user_props_map;
        std::map<std::wstring/*id*/, std::vector<std::shared_ptr<details::condition>>> _app_props_map;
        std::map<std::wstring/*id*/, std::vector<std::shared_ptr<details::condition>>> _res_props_map;
        std::map<std::wstring/*id*/, std::vector<std::shared_ptr<details::condition>>> _loc_props_map;
        std::map<std::wstring/*id*/, std::vector<std::shared_ptr<details::condition>>> _env_props_map;
    };
    class json_parser : public policy_bundle::parser
    {
    public:
        json_parser();
        virtual ~json_parser();
        virtual policy_bundle parse(const std::string& s /*utf8*/);
        virtual policy_bundle parse(const std::wstring& ws /*utf16*/);
    };
#ifdef ENABLE_PQL_PARSER
    class pql_parser : public policy_bundle::parser
    {
    public:
        pql_parser();
        virtual ~pql_parser();
        virtual policy_bundle parse(const std::string& s /*utf8*/);
        virtual policy_bundle parse(const std::wstring& ws /*utf16*/);
    };
#endif

private:
    std::wstring    _time;
    std::map<std::wstring/*id*/, std::shared_ptr<policy>> _policy_map;
    std::map<std::wstring/*hash*/, std::shared_ptr<details::condition>> _condition_map;
    std::map<std::wstring/*hash*/, std::shared_ptr<obligation>> _obligation_map;
    std::map<std::wstring/*user.id*/, std::pair<std::wstring/*user context*/,std::vector<std::wstring>/*user group id*/>> _user_group_map;
    std::pair<std::wstring/*user context*/, std::vector<std::wstring>/*user group id*/>   _unknown_user_group;

    friend class xml_parser;
    friend class json_parser;
    friend class pql_parser;
};


namespace details {
    

class expression
{
public:
    class node;
    class expression_node;
    class condition_node;

    expression() {}
    expression(std::shared_ptr<expression_node> p) : _root(p) {}
    ~expression() {}

    inline bool empty() const
    {
        return (NULL == _root.get()) ? true : _root->empty();
    }

    inline std::shared_ptr<expression_node> root() const noexcept { return _root; }

    bool evaluate(const attribute_multimap& attributes, const std::vector<std::wstring>& ignore_list=std::vector<std::wstring>(),  bool match_if_empty=true, bool check_exist_only = false) const noexcept
    {
        return empty() ? match_if_empty : _root->evaluate(attributes, ignore_list, check_exist_only);
    }
    
    std::wstring serialize() const noexcept
    {
        return std::move(_root->serialize());
    }

    expression& operator = (const expression& other) {
        if (this != &other) {
            _root = other.root();
        }
        return *this;
    }

    inline void clear() {
        _root.reset();
    }

    // void FuncT(const condition_node* p)
    template<class _FuncT>
    void traverse_expr(_FuncT _func)
    {
        if (NULL != _root) {
            traverse(_root.get(), _func);
        }
    }

    template<class _FuncT>
    void traverse(node* p, _FuncT& _func)
    {
        if (p->is_condition()) {
            return _func(p->as_condition());
        }

        std::for_each(p->as_expression()->children().begin(), p->as_expression()->children().end(), [&](std::shared_ptr<node> sp) {
            traverse(sp.get(), _func);
        });
    }

    void attach(std::shared_ptr<expression_node> p) { _root = p; }
    std::shared_ptr<expression_node> detach() {
        std::shared_ptr<expression_node> p = _root;
        _root.reset();
        return p;
    }

    static expression parse_pql(const std::wstring& ws);
    static expression parse_pql(const std::string& s);


public:
    class node
    {
    public:
        virtual ~node() {}

        inline bool is_reverse() const noexcept { return _reverse; }
        inline void set_reverse(bool b) noexcept { _reverse = b; }
        inline void set_parent(expression_node* parent) noexcept { _parent = parent; }
        inline expression_node* parent() noexcept { return _parent; }
        inline const expression_node* parent() const noexcept { return _parent; }
        inline bool is_root() const noexcept { return (nullptr == parent()); }

        virtual void clear() noexcept
        {
            _reverse = false;
            _parent = nullptr;
        }

        // pure virtual methods
        virtual bool empty() const noexcept = 0;
        virtual bool is_expression() const noexcept = 0;
        virtual bool is_condition() const noexcept = 0;

        virtual expression_node* as_expression() = 0;
        virtual const expression_node* as_expression() const = 0;
        virtual condition_node* as_condition() = 0;
        virtual const condition_node* as_condition() const = 0;

        virtual bool evaluate(const attribute_multimap& attributes, const std::vector<std::wstring>& ignore_list = std::vector<std::wstring>(), bool check_exist_only = false) const noexcept = 0;
        virtual std::wstring serialize() const noexcept = 0;

    protected:
        node() : _reverse(false), _parent(nullptr) {}
        node(expression_node* parent, bool reverse) : _reverse(reverse), _parent(parent) {}

    private:
        bool _reverse;
        expression_node* _parent;
    };

    class expression_node : public node
    {
    public:
        typedef std::shared_ptr<node>   ChildType;
        typedef std::vector<ChildType>  ChildArray;
        enum logic_operator {
            unknown = 0,
            op_and,
            op_or
        };

        expression_node() : node(nullptr, false), _operator(unknown)
        {
        }
        expression_node(expression_node* parent, logic_operator op = unknown, bool reverse = false) : node(parent, reverse), _operator(op)
        {
        }
        ~expression_node()
        {
        }

        inline const ChildArray& children() const noexcept { return _children; }
        inline ChildArray& children() noexcept { return _children; }
        inline logic_operator get_operator() const noexcept { return _operator; }
        inline void set_operator(logic_operator op) noexcept { _operator = op; }
        inline void set_logic_and() noexcept { _operator = op_and; }
        inline void set_logic_or() noexcept { _operator = op_or; }
        inline bool is_logic_and() const noexcept { return (op_and == _operator); }
        inline bool is_logic_or() const noexcept { return (op_or == _operator); }
        inline bool is_logic_unknown() const noexcept { return (unknown == _operator); }

        virtual void clear() noexcept
        {
            _operator = unknown;
            _children.clear();
            node::clear();
        }

        // implement pure virtual methods
        virtual bool empty() const noexcept
        {
            const ChildArray& chi = children();
            // no any child?
            if (chi.empty()) {
                return true;
            }
            // If cannot find any child which is not empty, it mean this node is empty
            return (chi.end() != std::find_if_not(chi.begin(), chi.end(), [&](std::shared_ptr<node> n)->bool {
                // find any which is not empty
                return !n->empty();
            }));
        }

        virtual bool is_expression() const noexcept { return true; }
        virtual bool is_condition() const noexcept { return false; }

        virtual expression_node* as_expression() { return this; }
        virtual const expression_node* as_expression() const { return this; }
        virtual condition_node* as_condition() { throw std::exception("not a condition node"); }
        virtual const condition_node* as_condition() const { throw std::exception("not a condition node"); }

        virtual bool evaluate(const attribute_multimap& attributes, const std::vector<std::wstring>& ignore_list = std::vector<std::wstring>(), bool check_exist_only = false) const noexcept;
        virtual std::wstring serialize() const noexcept;


    private:
        logic_operator  _operator;
        ChildArray      _children;
    };

    class condition_node : public node
    {
    public:
        condition_node() : node(nullptr, false)
        {
        }
        condition_node(expression_node* parent, std::shared_ptr<condition> p, bool reverse = false) : node(parent, reverse), _condition(p)
        {
        }
        condition_node(expression_node* parent, const std::string& s, bool reverse = false);
        condition_node(expression_node* parent, const std::wstring& s, bool reverse = false);
        virtual ~condition_node() {}
        
        inline std::shared_ptr<condition> get_condition() const noexcept { return _condition; }
        inline void set_condition(std::shared_ptr<condition> cond) noexcept { _condition = cond; }

        virtual void clear() noexcept
        {
            _condition.reset();
            node::clear();
        }

        // implement pure virtual methods
        virtual bool empty() const noexcept
        {
            return (_condition == nullptr);
        }

        virtual bool is_expression() const noexcept { return false; }
        virtual bool is_condition() const noexcept { return true; }

        virtual expression_node* as_expression() { throw std::exception("not a expression node"); }
        virtual const expression_node* as_expression() const { throw std::exception("not a expression node"); }
        virtual condition_node* as_condition() { return this; }
        virtual const condition_node* as_condition() const { return this; }

        virtual bool evaluate(const attribute_multimap& attributes, const std::vector<std::wstring>& ignore_list = std::vector<std::wstring>(), bool check_exist_only = false) const noexcept;
        virtual std::wstring serialize() const noexcept;

    private:
        std::shared_ptr<condition> _condition;
    };

protected:

private:
    std::shared_ptr<expression_node> _root;

    friend class policy_bundle;
    friend class policy_bundle::xml_parser;
};

class condition
{
public:
    
    enum condition_operator {
        EQ = 0,
        NE,
        GT,
        LT,
        GE,
        LE,
        HAS,
        CONTAINS
    };

    condition();
    condition(const std::string& s);
    condition(const std::wstring& s);
    condition(const std::wstring& name, const std::wstring& value, const std::wstring& op);
    condition(const std::wstring& name, const std::wstring& value, condition_operator op);
    virtual ~condition();



    static const std::wstring op_eq;
    static const std::wstring op_ne;
    static const std::wstring op_gt;
    static const std::wstring op_lt;
    static const std::wstring op_ge;
    static const std::wstring op_le;
    static const std::wstring op_has;
    static const std::wstring op_contains;
    static condition_operator string_to_operator(const std::wstring& s) noexcept;

    inline const std::wstring& name() const noexcept { return _name; }
    inline const value_object& value() const noexcept { return _value; }
    inline condition_operator op() const noexcept { return _op; }
    inline const std::wstring& hash() const noexcept { return _hash; }
    inline bool empty() const noexcept { return (_name.empty() || _value.empty()); }
    inline void clear() noexcept
    {
        _name.clear();
        _value.clear();
        _op = EQ;
    }

    const std::wstring& op_string() const noexcept;
    bool evaluate(const value_object& v) const noexcept;
    std::wstring serialize() const noexcept;

protected:
    void parse(std::wstring s);
    void calc_hash();

private:
    std::wstring        _name;
    value_object        _value;
    condition_operator  _op;
    std::wstring        _hash;
};

}   // namespace NX::EVAL::details


class policy
{
    enum rights_operator {
        revoke = 0,
        grant
    };
public:
    policy();
    policy(const std::wstring& id, const std::wstring& name, const std::vector<std::wstring>& rsv);
    virtual ~policy();

    inline bool to_grant() const noexcept { return (grant == _operator); }
    inline bool to_revoke() const noexcept { return (revoke == _operator); }
    inline const std::wstring& id() const noexcept { return _id; }
    inline const std::wstring& name() const noexcept { return _name; }
    inline const EVAL::rights& rights() const noexcept { return _rights; }
    inline const std::vector<std::shared_ptr<obligation>>& obligations() const noexcept { return _obligations; }
    inline std::vector<std::shared_ptr<obligation>>& obligations() noexcept { return _obligations; }

    inline details::expression* subject_expr() noexcept { return &_subject_expr; }
    inline details::expression* resource_expr() noexcept { return &_res_expr; }
    inline details::expression* env_expr() noexcept { return &_env_expr; }

    bool evaluate(eval_object& eobj) noexcept;

protected:

private:
    std::wstring    _id;
    std::wstring    _name;
    EVAL::rights    _rights;
    rights_operator _operator;
    details::expression _subject_expr;
    details::expression _res_expr;
    details::expression _env_expr;
    std::vector<std::shared_ptr<obligation>> _obligations;

    friend class policy_bundle::xml_parser;
    friend class policy_bundle::json_parser;
    friend class policy_bundle::pql_parser;
};


}   // namespace NX::EVAL
}   // namespace NX


#endif