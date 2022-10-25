


#include <Windows.h>

#include "pql_parser.hpp"



using namespace pql;
using namespace pql::details;

using namespace BOOST_SPIRIT_CLASSIC_NS;


pql_parser::pql_parser()
{
}

pql_parser::~pql_parser()
{
}

std::shared_ptr<pql::details::_parent_node> pql_parser::parse_expression(iter_t const& i, pql::details::_parent_node* parent)
{
    // first child points to a term, so call eval_term on it
    iter_t chi = i->children.begin();

    if (parent != nullptr) {
        assert(parent->is_parent());
    }

    std::shared_ptr<pql::details::_parent_node> exp_node(new pql::details::_parent_node(parent));

    try {

        // parse left
        parse_term(chi, exp_node.get());
        for (++chi; chi != i->children.end(); ++chi) {

            // next node points to the operator.  The text of the operator is
            // stored in value (a vector<char>)
            std::string op(chi->value.begin(), chi->value.end());
            if (op != "OR") {
                assert(false);
                throw std::exception("parse_expression: expect operator is not OR");
            }

            // parse to right
            //std::cout << " OR ";
            exp_node->set_op_or();
            parse_term(++chi, exp_node.get());
        }

        if (NULL != parent) {
            parent->children().push_back(exp_node);
        }
    }
    catch (...) {
        exp_node.reset();
    }

    return exp_node;
}

void pql_parser::parse_term(iter_t const& i, pql::details::_parent_node* parent)
{
    iter_t chi = i->children.begin();

    // parse left
    parse_factor(chi, parent);
    for (++chi; chi != i->children.end(); ++chi) {

        std::string op(chi->value.begin(), chi->value.end());
        if (op != "AND") {
            assert(false);
            throw std::exception("parse_term: expect operator is not AND");
        }

        // parse right
        //std::cout << " AND ";
        parent->set_op_and();
        parse_factor(++chi, parent);
    }
}

pql::details::_basic_node* pql_parser::parse_factor(iter_t const& i, pql::details::_parent_node* parent)
{
    pql::details::_basic_node* factor_node;

    iter_t chi = i->children.begin();
    if (chi->value.begin() == chi->value.end()) {
        // Okay, this is a condition
        return parse_condition(chi, parent);
    }

    std::string op(chi->value.begin(), chi->value.end());

    if (op == "NOT") {
        //std::cout << "NOT ";
        factor_node = parse_factor(++chi, parent);
        factor_node->set_reverse(true);
        return factor_node;
    }

    if (op == "(") {
        //std::cout << "(";
        factor_node = parse_expression(++chi, parent).get();
        //std::cout << ")";
        return factor_node;
    }

    // This must be a single condition
    return parse_condition(chi, parent);
}

pql::details::_basic_node* pql_parser::parse_condition(iter_t const& i, pql::details::_parent_node* parent)
{
    std::string  cond;

    for (iter_t & chi = i->children.begin(); chi != i->children.end(); ++chi) {
        cond += std::string(chi->value.begin(), chi->value.end());
        if (chi->children.begin() != chi->children.end()) {
            compose_condition(chi, cond);
        }
    }

    boost::algorithm::trim(cond);
    //std::cout << cond;

    std::shared_ptr<details::_leaf_node> cond_node(new details::_leaf_node(parent, cond));
    parent->children().push_back(cond_node);

    return cond_node.get();
}

void pql_parser::compose_condition(iter_t const& i, std::string& s)
{
    for (iter_t & chi = i->children.begin(); chi != i->children.end(); ++chi) {
        s += std::string(chi->value.begin(), chi->value.end());
        if (chi->children.begin() != chi->children.end()) {
            compose_condition(chi, s);
        }
    }
}

void pql_parser::parse(const std::string& s, pql::pql_tree& tree)
{
    static const pql::details::_pql_grammar parser;

    // parse pql string
    _info = pt_parse(s.c_str(), parser, space_p);
    if (!_info.full) {
        std::string err("fail to parse pql, stop at: ");
        err += _info.stop;
        throw std::exception(err.c_str());
    }

    // parse pql tree
    //std::cout << "Policy is:" << std::endl;
    //std::shared_ptr<details::_node> tree_root = parse_expression(_info.trees.begin(), nullptr);
    tree.attach(parse_expression(_info.trees.begin(), nullptr));
}

//
//  class pql::pql_tree
//
bool pql::pql_tree::empty() const
{
    if (NULL == _root.get()) return true;
    return _root->empty();
}

std::string pql::pql_tree::serialize() const noexcept
{
    std::string s;
    _root->serialize(s);
    return s;
}

bool pql::pql_tree::evaluate() const noexcept
{
    if (empty()) {
        return true;
    }

    return _root->evaluate();
}

void pql::pql_tree::trim()
{
    if (!_root->empty() && _root->is_parent()) {

        // only have one child? then we don't need this layer
        while (_root->as_parent()->children().size() == 1) {
            details::_parent_node* parent = _root->parent();
            std::shared_ptr<details::_basic_node> new_root = _root->as_parent()->children()[0];
            new_root->set_parent(parent);
            _root = new_root;
        }
    }
}


//
//  class pql::details::_parent_node
//
bool pql::details::_parent_node::evaluate() const noexcept
{
    bool result = false;
    
    if (_children.empty()) {
        return is_reverse() ? false : true;
    }

    if (is_op_and()) {
        result = (_children.end() == std::find_if_not(_children.begin(), _children.end(), [&](std::shared_ptr<_basic_node> ch) -> bool {
            return ch->evaluate();
        }));
    }
    else if (is_op_or()) {
        result = (_children.end() == std::find_if(_children.begin(), _children.end(), [&](std::shared_ptr<_basic_node> ch) -> bool {
            return ch->evaluate();
        }));
    }
    else {
        result = _children[0]->evaluate();
    }

    return is_reverse() ? (!result) : result;
}

void pql::details::_parent_node::serialize(std::string& s) const noexcept
{
    if (_children.empty()) {
        return;
    }

    if (!is_root() && _children.size() > 1) {
        s += "(";
    }

    for (auto it = _children.begin(); it != _children.end(); ++it) {
        if (it != _children.begin()) {
            if (is_op_and()) {
                s += " AND ";
            }
            else {
                s += " OR ";
            }
        }
        if ((*it)->is_reverse()) {
            s += "NOT ";
        }
        (*it)->serialize(s);
    }

    if (!is_root() && _children.size() > 1) {
        s += ")";
    }
}


//
//  class pql::details::_leaf_node
//
bool pql::details::_leaf_node::evaluate() const noexcept
{
    return true;
}

void pql::details::_leaf_node::serialize(std::string& s) const noexcept
{
    if (is_reverse()) {
        s += "NOT ";
    }
    s += condition();
}
