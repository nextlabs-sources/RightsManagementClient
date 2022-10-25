

#ifndef __NX_PQL_PARSER_HPP__
#define __NX_PQL_PARSER_HPP__

#include <string>
#include <iostream>
#include <sstream>
#include <vector>

#include <boost\algorithm\string.hpp>
#include <boost\spirit\include\classic_core.hpp>
#include <boost\spirit\include\classic_parse_tree.hpp>

//#include "pql_value.hpp"


namespace pql {


namespace details {
struct _pql_grammar;
class  _basic_node;
class  _parent_node;
class  _leaf_node;
}

class pql_tree;

class pql_parser
{
public:
    pql_parser();
    virtual ~pql_parser();

    typedef char const*                                     iterator_t;
    typedef boost::spirit::classic::tree_match<iterator_t>  parse_tree_match_t;
    typedef parse_tree_match_t::const_tree_iterator         iter_t;

    void parse(const std::string& s, pql_tree& tree);

private:
    std::shared_ptr<pql::details::_parent_node> parse_expression(iter_t const& i, details::_parent_node* parent);
    void parse_term(iter_t const& i, details::_parent_node* parent);
    details::_basic_node* parse_factor(iter_t const& i, details::_parent_node* parent);
    details::_basic_node* parse_condition(iter_t const& i, details::_parent_node* parent);
    void compose_condition(iter_t const& i, std::string& s);


private:
    boost::spirit::classic::tree_parse_info<> _info;
};

class pql_tree
{
public:
    pql_tree() {}
    virtual ~pql_tree() {}

    inline bool empty() const;
    std::string serialize() const noexcept;
    bool evaluate() const noexcept;

    void attach(std::shared_ptr<details::_basic_node>& root) noexcept { _root = root; trim(); }
    void attach(std::shared_ptr<details::_basic_node>&& root) noexcept { _root = root; trim(); }


private:
    void trim();

private:
    std::shared_ptr<details::_basic_node> _root;
};


namespace details {

struct _pql_grammar : public boost::spirit::classic::grammar<_pql_grammar>
{
    template <typename ScannerT>
    struct definition
    {

        boost::spirit::classic::rule<ScannerT> string_literal, number_literal, digit_literal, hex_literal;
        boost::spirit::classic::rule<ScannerT> expression, term, factor, condition, condition_left, condition_op, condition_right;

        definition(const _pql_grammar &self)
        {
            using namespace boost::spirit::classic;

            string_literal =
                lexeme_d
                [
                    ch_p('\"') >> +(anychar_p - ch_p('\"'))
                    >> ch_p('\"')
                ];

            digit_literal =
                lexeme_d
                [
                    +(ch_p('0') | ch_p('1') | ch_p('2') | ch_p('3') | ch_p('4') | ch_p('5') | ch_p('6') | ch_p('7') | ch_p('8') | ch_p('9'))
                ];

            hex_literal =
                lexeme_d
                [
                    +(ch_p('0') | ch_p('1') | ch_p('2') | ch_p('3') | ch_p('4') | ch_p('5') | ch_p('6') | ch_p('7') | ch_p('8') | ch_p('9')
                    | ch_p('a') | ch_p('b') | ch_p('c') | ch_p('d') | ch_p('e') | ch_p('f')
                        | ch_p('A') | ch_p('B') | ch_p('C') | ch_p('D') | ch_p('E') | ch_p('F')
                        )
                ];

            condition_left =
                lexeme_d
                [
                    +(anychar_p - ' ')
                ];

            condition_op =
                lexeme_d
                [
                    str_p(" = ") | str_p(" != ") | str_p(" > ") | str_p(" < ") | str_p(" >= ") | str_p(" <= ") | str_p(" contains ") | str_p(" has ")
                ];

            expression
                = term >> *("OR" >> term)
                ;

            term
                = factor >> *("AND" >> factor)
                ;

            factor
                = condition
                | '(' >> expression >> ')'
                | ("NOT" >> factor)
                ;

            condition = str_p("TRUE")
                | str_p("FALSE")
                | lexeme_d[+(anychar_p - " AND " - " OR " - " NOT " - '(' - ')' - '\"') >> string_literal]
                | lexeme_d[+(anychar_p - " AND " - " OR " - " NOT " - '(' - ')' - '\"')]
                ;

        }

        const boost::spirit::classic::rule<ScannerT> &start()
        {
            return expression;
        }
    };
};


class _basic_node
{
public:
    virtual ~_basic_node() {}

    inline bool is_reverse() const noexcept { return _reverse; }
    inline void set_reverse(bool b) noexcept { _reverse = b; }
    inline void set_parent(_parent_node* parent) noexcept { _parent = parent; }
    inline _parent_node* parent() noexcept { return _parent; }
    inline const _parent_node* parent() const noexcept { return _parent; }
    inline bool is_root() const noexcept { return (nullptr == parent()); }

    virtual void clear() noexcept
    {
        _reverse = false;
        _parent = nullptr;
    }

    virtual bool empty() const = 0;
    virtual bool is_parent() const = 0;
    virtual bool is_leaf() const = 0;

    virtual _parent_node* as_parent() = 0;
    virtual const _parent_node* as_parent() const = 0;
    virtual _leaf_node* as_leaf() = 0;
    virtual const _leaf_node* as_leaf() const = 0;

    virtual bool evaluate() const noexcept = 0;
    virtual void serialize(std::string& s) const noexcept = 0;

protected:
    _basic_node() : _reverse(false), _parent(nullptr) {}
    _basic_node(_parent_node* parent, bool reverse) : _reverse(reverse), _parent(parent) {}

private:
    bool    _reverse;
    _parent_node* _parent;
};

class _parent_node : public _basic_node
{
public:
    enum logic_operator {
        op_unknown = 0,
        op_and,
        op_or
    };

    typedef std::shared_ptr<_basic_node>  ChildType;
    typedef std::vector<ChildType>  ChildArray;

    _parent_node() : _basic_node(nullptr, false), _operator(op_unknown)
    {
    }
    _parent_node(_parent_node* parent, logic_operator op=op_unknown, bool reverse=false) : _basic_node(parent, reverse), _operator(op)
    {
    }
    virtual ~_parent_node()
    {
    }

    virtual void clear() noexcept
    {
        _operator = op_unknown;
        _children.clear();
        _basic_node::clear();
    }

    virtual bool empty() const
    {
        const ChildArray& chi = children();
        // no any child?
        if (chi.empty()) {
            return true;
        }
        // If cannot find any child which is not empty, it mean this node is empty
        return (chi.end() != std::find_if_not(chi.begin(), chi.end(), [&](std::shared_ptr<_basic_node> n)->bool {
            // find any which is not empty
            return !n->empty();
        }));
    }


    virtual bool evaluate() const noexcept;
    virtual void serialize(std::string& s) const noexcept;

    inline const ChildArray& children() const noexcept { return _children; }
    inline ChildArray& children() noexcept { return _children; }

    virtual bool is_parent() const { return true; }
    virtual bool is_leaf() const { return false; }

    virtual _parent_node* as_parent() { return this; }
    virtual const _parent_node* as_parent() const { return this; }
    virtual _leaf_node* as_leaf() { throw std::exception("not a leaf node"); }
    virtual const _leaf_node* as_leaf() const { throw std::exception("not a leaf node"); }

    inline logic_operator get_operator() const noexcept { return _operator; }
    inline void set_operator(logic_operator op) noexcept { _operator = op; }
    inline void set_op_and() noexcept { _operator = op_and; }
    inline void set_op_or() noexcept { _operator = op_or; }
    inline bool is_op_and() const noexcept { return (op_and == _operator); }
    inline bool is_op_or() const noexcept { return (op_or == _operator); }
    inline bool is_op_unknown() const noexcept { return (op_unknown == _operator); }

private:
    logic_operator  _operator;
    ChildArray      _children;
};

class _leaf_node : public _basic_node
{
public:
    _leaf_node() : _basic_node(nullptr, false)
    {
    }
    _leaf_node(_parent_node* parent, const std::string& condition, bool reverse=false) : _basic_node(parent, reverse), _condition(condition)
    {
    }
    virtual ~_leaf_node()
    {
    }

    virtual bool empty() const { return _condition.empty(); }
    virtual void clear() noexcept
    {
        _condition.clear();
        _basic_node::clear();
    }

    virtual bool evaluate() const noexcept;
    virtual void serialize(std::string& s) const noexcept;

    virtual _parent_node* as_parent() { throw std::exception("not a parent node"); }
    virtual const _parent_node* as_parent() const { throw std::exception("not a parent node"); }
    virtual _leaf_node* as_leaf() { return this; }
    virtual const _leaf_node* as_leaf() const { return this; }

    virtual bool is_parent() const { return false; }
    virtual bool is_leaf() const { return true; }

    inline const std::string& condition() const noexcept { return _condition; }
    inline void set_condition(const std::string& cond) noexcept { _condition = cond; }

private:
    std::string _condition;
};


}   // namespace pql::details

}   // namespace pql




#endif