

#include <windows.h>


#include <set>

#include <boost\algorithm\string.hpp>
#include <boost\spirit\include\classic_core.hpp>
#include <boost\spirit\include\classic_parse_tree.hpp>

#include <nudf\eh.hpp>
#include <nudf\debug.hpp>
#include <nudf\string.hpp>
#include <nudf\time.hpp>
#include <nudf\conversion.hpp>
#include <nudf\shared\rightsdef.h>


#include "policy_impl.hpp"
#include "policy_parser.hpp"


using namespace NXPOLICY_IMPL;
using namespace BOOST_SPIRIT_CLASSIC_NS;


extern std::shared_ptr<condition_object> ConstConditionTrue;
extern std::shared_ptr<condition_object> ConstConditionFalse;


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
                    ch_p(L'\"') >> +(anychar_p - ch_p(L'\"'))
                    >> ch_p(L'\"')
                ];

            digit_literal =
                lexeme_d
                [
                    +(ch_p(L'0') | ch_p(L'1') | ch_p(L'2') | ch_p(L'3') | ch_p(L'4') | ch_p(L'5') | ch_p(L'6') | ch_p(L'7') | ch_p(L'8') | ch_p(L'9'))
                ];

            hex_literal =
                lexeme_d
                [
                    +(ch_p(L'0') | ch_p(L'1') | ch_p(L'2') | ch_p(L'3') | ch_p(L'4') | ch_p(L'5') | ch_p(L'6') | ch_p(L'7') | ch_p(L'8') | ch_p(L'9')
                    | ch_p(L'a') | ch_p(L'b') | ch_p(L'c') | ch_p(L'd') | ch_p(L'e') | ch_p(L'f')
                        | ch_p(L'A') | ch_p(L'B') | ch_p(L'C') | ch_p(L'D') | ch_p(L'E') | ch_p(L'F')
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
                    str_p(L" = ") | str_p(L" != ") | str_p(L" > ") | str_p(L" < ") | str_p(L" >= ") | str_p(L" <= ") | str_p(L" contains ") | str_p(L" has ")
                ];

            expr
                = term >> *(L"OR" >> term)
                ;

            term
                = factor >> *(L"AND" >> factor)
                ;

            factor
                = condition
                | L'(' >> expr >> L')'
                | L"NOT" >> factor
                ;

            condition = str_p(L"TRUE")
                | str_p(L"FALSE")
                | lexeme_d[+(anychar_p - L"AND" - L"OR" - L"NOT" - L'(' - L')' - L'\"') >> string_literal]
                | lexeme_d[+(anychar_p - L"AND" - L"OR" - L"NOT" - L'(' - L')' - L'\"')]
                ;

        }

        const boost::spirit::classic::rule<ScannerT> &start()
        {
            return expr;
        }
    };
};


static unsigned __int64 parse_current_time_identity(const std::wstring& s);
static unsigned long parse_current_time_time(const std::wstring& s);

class pql_parser
{
public:
    pql_parser(condition_map&  cmap) : _cmap(cmap) {}
    virtual ~pql_parser() {}

    std::shared_ptr<condition_object> parse(const std::wstring& s)
    {
        static const pql_grammar _parser;

        static std::wstring formatted_s(s);
        // parse PQL string
        auto info = pt_parse(formatted_s.c_str(), _parser, space_p);
        if (!info.full) {
            throw NX::exception(NX::string_formater("Parsing failed, stopped at: %s", info.stop));
        }

        // parse pql tree
        return parse_expression(info.trees.begin(), false);
    }

private:
    typedef wchar_t const*                                  iterator_t;
    typedef boost::spirit::classic::tree_match<iterator_t>  parse_tree_match_t;
    typedef parse_tree_match_t::const_tree_iterator         iter_t;

    std::shared_ptr<condition_object> parse_expression(iter_t const& i, bool opposite)
    {
        std::shared_ptr<condition_object>   sp;

        // first child points to a term, so call eval_term on it
        iter_t chi = i->children.begin();

        try {

            // assume it is a OR expression
            std::shared_ptr<condition_object> exp_node(condition_expression_object::create_condition(opposite, NXPOLICY_IMPL::OR));

            // parse left
            parse_term(chi, exp_node);


            for (++chi; chi != i->children.end(); ++chi) {

                // next node points to the operator.  The text of the operator is
                // stored in value (a vector<char>)
                std::wstring logic_op_str(chi->value.begin(), chi->value.end());
                std::transform(logic_op_str.begin(), logic_op_str.end(), logic_op_str.begin(), toupper);    // operator is always upper case

                if (logic_op_str != L"OR") {
                    assert(false);
                    throw NX::exception(NX::string_formater("bad condition syntax (expect OR)"));
                }

                // parse to right
                parse_term(++chi, exp_node);
            }

            if (!opposite && ((condition_expression_object*)exp_node.get())->get_children().size() == 1) {
                // Only one child
                sp = ((condition_expression_object*)exp_node.get())->get_children().front();
            }
            else {
                sp = exp_node;
            }
        }
        catch (std::exception& e) {
            UNREFERENCED_PARAMETER(e);
        }

        return sp;
    }

    void parse_term(iter_t const& i, std::shared_ptr<condition_object> parent)
    {
        std::shared_ptr<condition_object>   sp;
        iter_t chi = i->children.begin();

        // parse left
        sp = parse_factor(chi, false);
        if (sp != nullptr) {
            ((condition_expression_object*)parent.get())->insert_condition(sp);
        }

        for (++chi; chi != i->children.end(); ++chi) {

            std::wstring logic_op_str(chi->value.begin(), chi->value.end());
            std::transform(logic_op_str.begin(), logic_op_str.end(), logic_op_str.begin(), toupper);    // operator is always upper case

            if (logic_op_str != L"AND") {
                assert(false);
                throw NX::exception(NX::string_formater("bad condition syntax (expect AND)"));
            }

            // parse right
            // parent is an AND expression
            ((condition_expression_object*)parent.get())->set_relationship(NXPOLICY_IMPL::AND);

            sp = parse_factor(++chi, false);
            if (sp != nullptr) {
                ((condition_expression_object*)parent.get())->insert_condition(sp);
            }
        }
    }

    std::shared_ptr<condition_object> parse_factor(iter_t const& i, bool opposite)
    {
        iter_t chi = i->children.begin();
        if (chi->value.begin() == chi->value.end()) {
            // OKay, no child, this is a condition
            assert(!opposite);
            return parse_condition(chi);
        }

        std::wstring op(chi->value.begin(), chi->value.end());

        if (op == L"NOT") {
            assert(!opposite);
            return parse_factor(++chi, true);
        }

        if (op == L"(") {
            return parse_expression(++chi, opposite);
        }

        // This must be a single condition
        assert(!opposite);
        return parse_condition(chi);
    }

    void compose_condition(iter_t const& i, std::wstring& s)
    {
        for (iter_t & chi = i->children.begin(); chi != i->children.end(); ++chi) {
            s += std::wstring(chi->value.begin(), chi->value.end());
            if (chi->children.begin() != chi->children.end()) {
                compose_condition(chi, s);
            }
        }
    }

    std::shared_ptr<condition_object> parse_condition(iter_t const& i)
    {
        std::wstring  s;

        compose_condition(i, s);

        boost::algorithm::trim(s);
        std::shared_ptr<condition_object> sp;
        try {
            sp = _cmap.find(s);
            if (sp == nullptr) {
                sp = parse_condition(s);
                if (sp != nullptr) {
                    _cmap.insert(s, sp);
                }
            }
        }
        catch (const std::exception& e) {
            UNREFERENCED_PARAMETER(e);
            sp.reset();
        }
        return sp;
    }

    bool is_number(const std::wstring& s, bool* is_decimal)
    {
        *is_decimal = false;

        if (s.empty()) {
            return false;
        }
        for (int i = 0; i < (int)s.length(); i++) {
            if (L'.' == s[i]) {
                if (*is_decimal) {
                    return false;
                }
                *is_decimal = true;
            }
            else {
                if (!NX::utility::is_digit<wchar_t>(s[i])) {
                    return false;
                }
            }
        }
        return true;
    }

    std::shared_ptr<condition_object> parse_condition(const std::wstring& s)
    {
        if (0 == _wcsicmp(s.c_str(), L"TRUE")) {
            // const TRUE condition
            return ConstConditionTrue;
        }
        else if (0 == _wcsicmp(s.c_str(), L"FALSE")) {
            // const FALSE condition
            return ConstConditionFalse;
        }
        else {
            // Normal condition: NAME OP VALUE
            ;
        }

        // condition name MUST NOT have any space
        std::wstring::size_type pos = s.find_first_of(L' ');
        if (pos == std::wstring::npos) {
            throw NX::exception(NX::string_formater("bad condition syntax (missing space): %S", s.c_str()));
        }

        // parse property_name, cmp_op and cmp_value
        std::wstring property_name = s.substr(0, pos);
        // property name is always lower case
        std::transform(property_name.begin(), property_name.end(), property_name.begin(), tolower);

        std::wstring str_op = s.substr(pos + 1);
        boost::algorithm::trim(str_op);
        pos = str_op.find_first_of(L' ');
        if (pos == std::wstring::npos) {
            throw NX::exception(NX::string_formater("bad condition syntax (missing space): %S", s.c_str()));
        }
        std::wstring str_value = str_op.substr(pos + 1);
        boost::algorithm::trim(str_value);
        str_op = str_op.substr(0, pos);

        if (property_name.empty()) {
            throw NX::exception(NX::string_formater("bad condition syntax (empty name): %S", s.c_str()));
        }
        if (str_op.empty()) {
            throw NX::exception(NX::string_formater("bad condition syntax (empty operator): %S", s.c_str()));
        }
        if (str_op.empty()) {
            throw NX::exception(NX::string_formater("bad condition syntax (empty value): %S", s.c_str()));
        }

        NXPOLICY_IMPL::COMPARE_OPERATOR cmp_op = NXPOLICY_IMPL::EqualTo;
        bool    is_op_contains = false;
        // upper case operator string
        std::transform(str_op.begin(), str_op.end(), str_op.begin(), toupper);
        if (L"HAS" == str_op || L"=" == str_op) {
            cmp_op = NXPOLICY_IMPL::EqualTo;
        }
        else if (L"CONTAINS" == str_op) {
            cmp_op = NXPOLICY_IMPL::EqualTo;
            is_op_contains = true;
        }
        else if (L"!=" == str_op) {
            cmp_op = NXPOLICY_IMPL::NotEqualTo;
        }
        else if (L">" == str_op) {
            cmp_op = NXPOLICY_IMPL::GreaterThan;
        }
        else if (L">=" == str_op) {
            cmp_op = NXPOLICY_IMPL::GreaterThanOrEqualTo;
        }
        else if (L"<" == str_op) {
            cmp_op = NXPOLICY_IMPL::LessThan;
        }
        else if (L"<=" == str_op) {
            cmp_op = NXPOLICY_IMPL::LessThanOrEqualTo;
        }
        else {
            throw NX::exception(NX::string_formater("bad condition syntax (unknown operator): %S", s.c_str()));
        }

        std::shared_ptr<condition_object> sp;

        // parse value
        if (L'\"' == str_value[0]) {

            assert(L'\"' == str_value[str_value.length() - 1]);
            if (L'\"' != str_value[str_value.length() - 1]) {
                throw NX::exception(NX::string_formater("bad condition syntax (missing '\"'): %S", s.c_str()));
            }
            // remove '\"'
            str_value = str_value.substr(1);
            str_value = str_value.substr(0, str_value.length() - 1);
            boost::algorithm::trim(str_value);

            if (NX::time::is_iso_8601_time_string(str_value) || NX::time::is_iso_8601_zone_time_string(str_value)) {
                NX::time::datetime dtm(str_value);
                sp = std::shared_ptr<condition_object>(condition_value_object::create_condition(property_name, cmp_op, (__int64)dtm, NXPOLICY::VALUE_DATETIME));
            }
            else if (NX::time::is_rfc_1123_0_time_string(str_value) || NX::time::is_rfc_1123_1_time_string(str_value)) {
                NX::time::datetime_rfc1123 dtm;
                dtm.parse(str_value);
                sp = std::shared_ptr<condition_object>(condition_value_object::create_condition(property_name, cmp_op, (__int64)dtm, NXPOLICY::VALUE_DATETIME));
            }
            else if (NX::time::is_special_1_time_string(str_value)) {
                NX::time::datetime_special_1 dtm;
                dtm.parse(str_value);
                sp = std::shared_ptr<condition_object>(condition_value_object::create_condition(property_name, cmp_op, (__int64)dtm, NXPOLICY::VALUE_DATETIME));
            }
            else if (NX::time::is_time_only_string(str_value)) {

                int span_hours = 0, span_minutes = 0, span_seconds = 0;
                wchar_t span_time_flag[3] = { 0, 0, 0 };
                int count = swscanf_s(str_value.c_str(), L"%d:%d:%d %s", &span_hours, &span_minutes, &span_seconds, span_time_flag, 3);
                if (count >= 3) {
                    if (0 == _wcsicmp(span_time_flag, L"PM")) {
                        span_hours += 12;
                    }
                    span_seconds += (span_hours * 3600 + span_minutes * 60);
                }

                sp = std::shared_ptr<condition_object>(condition_value_object::create_condition(property_name, cmp_op, (__int64)span_seconds, NXPOLICY::VALUE_DAYSECONDS));
            }
            else {
                if (cmp_op != NXPOLICY_IMPL::EqualTo && cmp_op != NXPOLICY_IMPL::NotEqualTo) {
                    throw NX::exception(NX::string_formater("unexpected operator (string value must use operator '=' or '!='): %S", s.c_str()));
                }
                boost::algorithm::replace_all(str_value, L"\\\\", L"\\");
                sp = std::shared_ptr<condition_object>(condition_value_object::create_condition(property_name, cmp_op, str_value));
            }
        }
        else {

            if (0 == _wcsicmp(str_value.c_str(), L"null") || 0 == _wcsicmp(str_value.c_str(), L"nil")) {
                // empty
                sp = std::shared_ptr<condition_object>(condition_value_object::create_condition(property_name, cmp_op, L""));
            }
            else {
                // number
                if (boost::algorithm::istarts_with(str_value, L"0x")) {
                    // hex
                    const unsigned __int64 u = std::wcstoull(str_value.c_str(), nullptr, 16);
                    sp = std::shared_ptr<condition_object>(condition_value_object::create_condition(property_name, cmp_op, u));
                }
                else {
                    bool is_decimal = false;
                    if (is_number(s, &is_decimal)) {

                        if (is_decimal) {
                            // float
                            const double f = std::wcstod(str_value.c_str(), nullptr);
                            sp = std::shared_ptr<condition_object>(condition_value_object::create_condition(property_name, cmp_op, f));
                        }
                        else {
                            const __int64 n = std::wcstoll(str_value.c_str(), nullptr, 10);
                            sp = std::shared_ptr<condition_object>(condition_value_object::create_condition(property_name, cmp_op, n));
                        }
                    }
                    else {
                        sp = std::shared_ptr<condition_object>(condition_dynamic_value_object::create_condition(property_name, cmp_op, str_value));
                    }
                }
            }
        }

        return sp;
    }

private:
    condition_map&  _cmap;
};

//unsigned __int64 parse_current_time_identity(const std::wstring& s)
//{
//    // format is like "Apr 12, 2016 1:24:00 PM"
//    wchar_t month_name[4] = { 0, 0, 0, 0 };
//    wchar_t flag[3] = { 0, 0, 0 };
//    int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
//
//    if (7 != swscanf_s(s.c_str(), L"%3c %d, %d %d:%d:%d %2c", month_name, 3, &day, &year, &hour, &minute, &second, flag, 2)) {
//        return 0;
//    }
//
//    if (0 == _wcsicmp(month_name, L"Jan")) {
//        month = 1;
//    }
//    else if (0 == _wcsicmp(month_name, L"Feb")) {
//        month = 2;
//    }
//    else if (0 == _wcsicmp(month_name, L"Mar")) {
//        month = 3;
//    }
//    else if (0 == _wcsicmp(month_name, L"Apr")) {
//        month = 4;
//    }
//    else if (0 == _wcsicmp(month_name, L"May")) {
//        month = 5;
//    }
//    else if (0 == _wcsicmp(month_name, L"Jun")) {
//        month = 6;
//    }
//    else if (0 == _wcsicmp(month_name, L"Jul")) {
//        month = 7;
//    }
//    else if (0 == _wcsicmp(month_name, L"Aug")) {
//        month = 8;
//    }
//    else if (0 == _wcsicmp(month_name, L"Sep")) {
//        month = 9;
//    }
//    else if (0 == _wcsicmp(month_name, L"Oct")) {
//        month = 10;
//    }
//    else if (0 == _wcsicmp(month_name, L"Nov")) {
//        month = 11;
//    }
//    else if (0 == _wcsicmp(month_name, L"Dec")) {
//        month = 11;
//    }
//    else {
//        return 0;
//    }
//
//    if (0 == _wcsicmp(flag, L"PM") && hour < 12) {
//        hour += 12;
//    }
//
//    SYSTEMTIME  st = { 0 };
//    FILETIME local_ft = { 0, 0 };
//    FILETIME gmt_ft = { 0, 0 };
//    st.wYear = year;
//    st.wMonth = month;
//    st.wDay = day;
//    st.wHour = hour;
//    st.wMinute = minute;
//    st.wSecond = second;
//    st.wMilliseconds = 0;
//    SystemTimeToFileTime(&st, &local_ft);
//    LocalFileTimeToFileTime(&local_ft, &gmt_ft);
//
//    unsigned __int64 ut = gmt_ft.dwHighDateTime;
//    ut <<= 32;
//    ut += gmt_ft.dwLowDateTime;
//
//    return ut;
//}
//
//unsigned long parse_current_time_time(const std::wstring& s)
//{
//    // format is like "1:24:00 PM"
//    wchar_t flag[3] = { 0, 0, 0 };
//    unsigned long hour = 0, minute = 0, second = 0;
//
//    if (4 != swscanf_s(s.c_str(), L"%d:%d:%d %2c", &hour, &minute, &second, flag, 2)) {
//        return -1;
//    }
//    if (hour > 23 || minute > 59 || second > 59) {
//        return -1;
//    }
//    if (0 == _wcsicmp(flag, L"PM") && hour < 12) {
//        hour += 12;
//    }
//
//    return ((hour * 3600) + minute * 60 + second);
//}

static const std::wstring element_prefix_title(L"ID ");
static const std::wstring element_prefix_attribute(L"ATTRIBUTE ");
static const std::wstring element_prefix_resource(L"FOR ");
static const std::wstring element_prefix_recipient(L"TO ");
static const std::wstring element_prefix_subject(L"BY ");
static const std::wstring element_prefix_action(L"DO ");
static const std::wstring element_prefix_environment(L"WHERE ");
static const std::wstring element_prefix_allow_obligation(L"ON allow ");
static const std::wstring element_prefix_deny_obligation(L"ON deny ");

template<typename CharType>
class pql_bundle_parser
{
public:
    pql_bundle_parser(std::basic_istream<CharType>& s) : _stream(s) {}
    virtual ~pql_bundle_parser() {}

    std::shared_ptr<policy_bundle> parse(const NXPOLICY::property_repository& repo = NXPOLICY::property_repository())
    {
        break_down();
        if (!repo.get_repo().empty()) {
            filter(repo);
        }

        return std::shared_ptr<policy_bundle>(new policy_bundle(get_policy_map()));
    }

    inline const std::map<std::wstring, std::shared_ptr<policy_object>>& get_policy_map() const { return _policy_map; }
    inline const condition_map& get_condition_map() const { return _cmap; }

private:
    void break_down()
    {
        std::wstring line;

        while (!_stream.eof()) {

            if (0 == _wcsicmp(line.c_str(), L"Deployment entities:")) {
                // start of policies map
                parse_policy_map(_stream, line);
            }
            else if (0 == _wcsicmp(line.c_str(), L"Subject keys:")) {
                // start of subject keys
                parse_subject_key_map(_stream, line);
            }
            else if (0 == _wcsicmp(line.c_str(), L"Subject-Group Mappings:")) {
                // start of subject keys
                parse_subject_group_map(_stream, line);
            }
            else if (0 == _wcsicmp(line.c_str(), L"Subject-Policy Mappings:")) {
                // start of subject keys
                //parse_subject_policy_map(_stream, line);
                line = get_line(_stream);
            }
            else if (0 == _wcsicmp(line.c_str(), L"Action-Policy Mappings:")) {
                // start of subject keys
                line = get_line(_stream);
            }
            else {
                // Unknown
                line = get_line(_stream);
            }
        }
    }

    void filter(const NXPOLICY::property_repository& repo)
    {
        for (auto it = _policy_map.begin(); it != _policy_map.end(); ) {
            if (!it->second->get_subject_condition()->evaluate(repo)) {
                _policy_map.erase(it++);
            }
            else {
                ++it;
            }
        }
    }

    std::wstring get_line(std::basic_istream<CharType>& fs)
    {
        std::basic_string<CharType> s;
        std::getline(fs, s);
        boost::algorithm::trim(s);
        if (s.empty()) {
            return std::wstring();
        }
        return std::move(NX::conversion::to_utf16(s));
    }

    unsigned __int64 parse_rights(const std::wstring& s)
    {
        unsigned __int64 result = 0;
        std::vector<std::wstring> right_names;
        std::wstring ls(s);
        boost::algorithm::replace_all(ls, L"OR", L" ");
        boost::algorithm::replace_all(ls, L"(", L" ");
        boost::algorithm::replace_all(ls, L")", L" ");
        boost::algorithm::trim(ls);
        std::wstring::size_type pos = 0;

        do {

            pos = ls.find(L' ');
            if (pos == std::wstring::npos) {
                right_names.push_back(ls);
                ls.clear();
            }
            else {
                std::wstring action_str = ls.substr(0, pos);
                if (!action_str.empty()) {
                    right_names.push_back(action_str);
                }
                ls = ls.substr(pos + 1);
                boost::algorithm::trim_left(ls);
            }

        } while (pos != std::wstring::npos && !ls.empty());

        std::for_each(right_names.begin(), right_names.end(), [&](const std::wstring& name) {
            result |= ActionToRights(name.c_str());
        });

        return result;
    }

    void parse_policy_title(const std::wstring& s, std::wstring& id, std::wstring& descritpion)
    {
        auto pos = s.find(L' ');
        if (pos == std::wstring::npos) {
            return;
        }

        id = s.substr(0, pos);
        if (id.empty()) {
            return;
        }
        std::transform(id.begin(), id.end(), id.begin(), tolower);

        pos = s.find(L'\"');
        descritpion = s.substr(pos + 1);
        if (boost::algorithm::ends_with(descritpion, L"\"")) {
            descritpion = descritpion.substr(0, descritpion.length() - 1);
        }
    }

    std::shared_ptr<policy_object> parse_policy(std::basic_istream<CharType>& fs, const std::wstring& title_str, std::wstring& exist_str)
    {
        std::shared_ptr<policy_object> sp;

        if (!boost::algorithm::istarts_with(title_str, element_prefix_title)) {
            return sp;
        }
        
        // policy data
        policy_object::POLICY_ACTION policy_action = policy_object::ACTION_UNKNOWN;
        unsigned __int64 policy_rights = 0;
        std::wstring    policy_id;
        std::wstring    policy_attribute;
        std::wstring    policy_description;
        std::shared_ptr<condition_object> subject_condition;
        std::shared_ptr<condition_object> resource_condition;
        std::shared_ptr<condition_object> environment_condition;

        parse_policy_title(title_str.substr(element_prefix_title.length()), policy_id, policy_description);
        if (policy_id.empty()) {
            return sp;
        }
        
        while (!fs.eof()) {

            const std::wstring& s = get_line(fs);

            if (boost::algorithm::istarts_with(s, element_prefix_attribute)) {
                // Policy general attributes
                policy_attribute = s.substr(element_prefix_attribute.length());
            }
            else if (boost::algorithm::istarts_with(s, element_prefix_resource)) {
                // Policy resource conditions
                resource_condition = parse_pql(s.substr(element_prefix_resource.length()), _cmap);
            }
            else if (boost::algorithm::istarts_with(s, L"BY ")) {
                // Policy subject conditions
                subject_condition = parse_pql(s.substr(element_prefix_subject.length()), _cmap);
            }
            else if (boost::algorithm::istarts_with(s, element_prefix_environment)) {
                // Policy subject conditions
                environment_condition = parse_pql(s.substr(element_prefix_environment.length()), _cmap);
            }
            else if (boost::algorithm::istarts_with(s, element_prefix_action)) {
                // Policy action
                const std::wstring& action_name = s.substr(element_prefix_action.length());
                if (0 == _wcsicmp(action_name.c_str(), L"allow")) {
                    policy_action = policy_object::GRANT_RIGHTS;
                }
                else if (0 == _wcsicmp(action_name.c_str(), L"deny")) {
                    policy_action = policy_object::REVOKE_RIGHTS;
                }
                else {
                    policy_action = policy_object::ACTION_UNKNOWN;
                }
            }
            else if (boost::algorithm::istarts_with(s, L"ON ")) {

                if (boost::algorithm::istarts_with(s, element_prefix_allow_obligation)) {
                    // Allow obligation
                    ;
                }
                else if (boost::algorithm::istarts_with(s, element_prefix_deny_obligation)) {
                    // Deny obligation
                    //   --> Ignore
                }
                else {
                    // Actions
                    //   --> Parse Rights
                    policy_rights = parse_rights(s.substr(3));
                }
            }
            else if (boost::algorithm::istarts_with(s, L"TO ")) {
                // Policy recipient conditions, ignore
                ;
            }
            else {
                // Unknown
                exist_str = s;
                break;
            }
        }

        if (policy_object::ACTION_UNKNOWN != policy_action && 0 != policy_rights && 0 == _wcsicmp(policy_attribute.c_str(), L"DOCUMENT_POLICY")) {
            // ignore invalid policy
            sp = std::shared_ptr<policy_object>(new policy_object(policy_id, policy_description, policy_action, policy_rights, subject_condition, resource_condition, environment_condition));
        }
        return sp;
    }

    void parse_policy_map(std::basic_istream<CharType>& fs, std::wstring& exit_str)
    {
        std::wstring s = get_line(fs);
        while (boost::algorithm::istarts_with(s, element_prefix_title)) {
            std::shared_ptr<policy_object> sp = parse_policy(fs, s, s);
            if (sp != nullptr) {
                _policy_map[sp->get_id()] = sp;
            }
        }
    }

    void parse_subject_policy_map(std::basic_istream<CharType>& fs, std::wstring& exit_str)
    {
    }

    void parse_subject_key_map(std::basic_istream<CharType>& fs, std::wstring& exit_str)
    {
        std::wstring line;

        while (!_stream.eof()) {

            const std::wstring& line = get_line(_stream);

            std::wstring s;
            std::wstring subject_str;
            std::wstring subject_type;
            std::wstring subject_key_id;

            auto pos = line.find(L'(');
            if (pos == std::wstring::npos) {
                exit_str = line;
                break;
            }

            subject_str = line.substr(0, pos);
            s = line.substr(pos + 1);
            boost::algorithm::trim(subject_str);
            
            pos = s.find(L')');
            if (pos == std::wstring::npos) {
                exit_str = line;
                break;
            }
            subject_type = s.substr(0, pos);
            s = s.substr(pos + 1);
            boost::algorithm::trim(subject_type);

            pos = s.find(L"->");
            if (pos == std::wstring::npos) {
                exit_str = line;
                break;
            }
            subject_key_id = s.substr(pos + 2);
            boost::algorithm::trim(subject_key_id);
            if (!subject_key_id.empty()) {
                if (0 == _wcsicmp(subject_type.c_str(), L"user-windowsSid")) {
                    _user_id_subject_map[subject_str] = std::wcstoll(subject_key_id.c_str(), nullptr, 10);
                }
                else if (0 == _wcsicmp(subject_type.c_str(), L"email")) {
                    _email_subject_map[subject_str] = std::wcstoll(subject_key_id.c_str(), nullptr, 10);
                }
                else {
                    // unknown type, ignore it
                    ;
                }
            }
        }
    }

    void parse_subject_group_map(std::basic_istream<CharType>& fs, std::wstring& exit_str)
    {
        while (!_stream.eof()) {

            const std::wstring& line = get_line(_stream);

            auto pos = line.find(L' ');
            if (pos == std::wstring::npos) {
                exit_str = line;
                break;
            }

            std::wstring subject_key_id = line.substr(0, pos);
            if (subject_key_id.empty()) {
                exit_str = line;
                break;
            }
            std::transform(subject_key_id.begin(), subject_key_id.end(), subject_key_id.begin(), tolower);

            pos = line.find(L'{');
            if (pos == std::wstring::npos) {
                exit_str = line;
                break;
            }

            std::wstring group_list_str = line.substr(pos + 1);
            if (group_list_str.empty()) {
                exit_str = line;
                break;
            }
            if (group_list_str[group_list_str.length() - 1] != L'}') {
                exit_str = line;
                break;
            }
            group_list_str = line.substr(0, group_list_str.length() - 1);
            boost::algorithm::trim(group_list_str);

            std::set<__int64> subject_group_set;
            while (!group_list_str.empty()) {
                std::wstring group_id;
                pos = group_list_str.find(L',');
                if (pos == std::wstring::npos) {
                    group_id = group_list_str;
                    group_list_str.clear();
                }
                else {
                    group_id = group_list_str.substr(0, pos);
                    group_list_str = group_list_str.substr(pos + 1);
                }
                boost::algorithm::trim(group_id);
                if (!group_id.empty()) {
                    std::transform(group_id.begin(), group_id.end(), group_id.begin(), tolower);
                    subject_group_set.insert(std::wcstoll(group_id.c_str(), nullptr, 10));
                }
            }
            std::vector<__int64> subject_group_list;
            std::for_each(subject_group_set.begin(), subject_group_set.end(), [&](__int64 n) {
                subject_group_list.push_back(n);
            });

            _subject_group_map[subject_key_id] = subject_group_list;
        }
    }

private:

private:
    std::basic_istream<CharType>&   _stream;
    std::map<std::wstring, __int64>                 _user_id_subject_map;
    std::map<std::wstring, __int64>                 _email_subject_map;
    std::map<std::wstring, std::vector<__int64>>    _subject_group_map;
    std::map<std::wstring, std::shared_ptr<policy_object>>  _policy_map;
    condition_map  _cmap;
};


//
//
//
std::shared_ptr<condition_object> NXPOLICY_IMPL::parse_pql(const std::wstring& s, condition_map& cmap)
{
    std::shared_ptr<condition_object> sp;
    try {
        pql_parser parser(cmap);
        sp = parser.parse(s);
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
    return sp;
}

std::shared_ptr<policy_bundle> NXPOLICY_IMPL::parse_pql_bundle_file(const std::wstring& file)
{
    std::shared_ptr<policy_bundle> sp;
    std::ifstream fs;

    fs.open(file);
    if (fs.is_open() && fs.good()) {
        try {
            pql_bundle_parser<char> parser(fs);
            sp = parser.parse();
        }
        catch (std::exception& e) {
            UNREFERENCED_PARAMETER(e);
        }
    }

    return sp;
}

std::shared_ptr<policy_bundle> NXPOLICY_IMPL::parse_pql_bundle_string(const std::wstring& file)
{
    return std::shared_ptr<policy_bundle>();
}