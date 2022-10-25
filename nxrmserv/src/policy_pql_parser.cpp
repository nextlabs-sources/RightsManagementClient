
#ifdef ENABLE_PQL_PARSER


#include <Windows.h>

#include <set>
#include <boost\algorithm\string.hpp>
#include <boost\spirit\include\classic_core.hpp>
#include <boost\spirit\include\classic_parse_tree.hpp>

#include <nudf\web\json.hpp>
#include <nudf\exception.hpp>
#include <nudf\string.hpp>

#include <rapidxml\rapidxml.hpp>

#include "nxrmserv.h"
#include "policy.hpp"


using namespace NX;
using namespace NX::EVAL;
using namespace BOOST_SPIRIT_CLASSIC_NS;



struct pql_grammar : public boost::spirit::classic::grammar<pql_grammar>
{
    template <typename ScannerT>
    struct definition
    {

        boost::spirit::classic::rule<ScannerT> string_literal, number_literal, digit_literal, hex_literal;
        boost::spirit::classic::rule<ScannerT> expr, term, factor, condition, condition_left, condition_op, condition_right;

        definition(const pql_grammar &self)
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

            expr
                = term >> *("OR" >> term)
                ;

            term
                = factor >> *("AND" >> factor)
                ;

            factor
                = condition
                | '(' >> expr >> ')'
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
            return expr;
        }
    };
};


class expr_parser
{
public:
    expr_parser() {}
    virtual ~expr_parser() {}

    details::expression parse(const std::string& s)
    {
        static const pql_grammar _parser;

        // parse pql string
        _info = pt_parse(s.c_str(), _parser, space_p);
        if (!_info.full) {
            std::string err("fail to parse pql, stop at: ");
            err += _info.stop;
            throw std::exception(err.c_str());
        }

        // parse pql tree
        return details::expression(parse_expression(_info.trees.begin(), nullptr));
    }
    details::expression operator()(const std::string& s)
    {
        return parse(s);
    }

private:
    typedef char const*                                     iterator_t;
    typedef boost::spirit::classic::tree_match<iterator_t>  parse_tree_match_t;
    typedef parse_tree_match_t::const_tree_iterator         iter_t;

    std::shared_ptr<details::expression::expression_node> parse_expression(iter_t const& i, details::expression::expression_node* parent)
    {
        // first child points to a term, so call eval_term on it
        iter_t chi = i->children.begin();

        if (parent != nullptr) {
            assert(parent->is_expression());
        }

        std::shared_ptr<details::expression::expression_node> exp_node(new details::expression::expression_node(parent));

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
                exp_node->set_logic_or();
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

    void parse_term(iter_t const& i, details::expression::expression_node* parent)
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
            parent->set_logic_and();
            parse_factor(++chi, parent);
        }
    }

    details::expression::node* parse_factor(iter_t const& i, details::expression::expression_node* parent)
    {
        details::expression::node* factor_node;

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

    details::expression::node* parse_condition(iter_t const& i, details::expression::expression_node* parent)
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

        std::shared_ptr<details::expression::condition_node> cond_node(new details::expression::condition_node(parent, cond));
        parent->children().push_back(cond_node);

        return cond_node.get();
    }

    void compose_condition(iter_t const& i, std::string& s)
    {
        for (iter_t & chi = i->children.begin(); chi != i->children.end(); ++chi) {
            s += std::string(chi->value.begin(), chi->value.end());
            if (chi->children.begin() != chi->children.end()) {
                compose_condition(chi, s);
            }
        }
    }

private:
    boost::spirit::classic::tree_parse_info<> _info;
};


//
//  class details::expression
//
details::expression details::expression::parse_pql(const std::wstring& ws)
{
    std::string s = NX::utility::conversions::utf16_to_utf8(ws);
    return parse_pql(s);
}

details::expression details::expression::parse_pql(const std::string& s)
{
    expr_parser parser;
    return parser.parse(s);
}


//
//  class policy_bundle::pql_parser
//
policy_bundle::pql_parser::pql_parser() : policy_bundle::parser()
{
}

policy_bundle::pql_parser::~pql_parser()
{
}

policy_bundle policy_bundle::pql_parser::parse(const std::string& s /*utf8*/)
{
    std::wstring ws = NX::utility::conversions::utf8_to_utf16(s);
    return parse(ws);
}

policy_bundle policy_bundle::pql_parser::parse(const std::wstring& ws /*utf16*/)
{
    policy_bundle bundle;
    return bundle;
}



#endif  // #ifdef ENABLE_PQL_PARSER