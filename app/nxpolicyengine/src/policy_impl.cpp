

#include <windows.h>

#include <fstream>
#include <sstream>
#include <boost/algorithm/string.hpp>

#include <nudf\eh.hpp>
#include <nudf\debug.hpp>
#include <nudf\string.hpp>
#include <nudf\conversion.hpp>
#include <nudf\time.hpp>
#include <nudf\shared\rightsdef.h>

#include "policy_impl.hpp"
#include "policy_parser.hpp"


using namespace NXPOLICY;
using namespace NXPOLICY_IMPL;


std::shared_ptr<condition_object> ConstConditionTrue(condition_const_value_object::create_condition(true));
std::shared_ptr<condition_object> ConstConditionFalse(condition_const_value_object::create_condition(false));

static void remove_escape(std::wstring& s);
static void wildcards_to_regex(std::wstring& s);
static void nextlabs_wildcards_to_regex(std::wstring& s);

void remove_escape(std::wstring& s)
{
    // remove all escape
    boost::algorithm::replace_all(s, L"\\", L"\\\\");
    boost::algorithm::replace_all(s, L"^", L"\\^");
    boost::algorithm::replace_all(s, L".", L"\\.");
    boost::algorithm::replace_all(s, L"$", L"\\$");
    boost::algorithm::replace_all(s, L"|", L"\\|");
    boost::algorithm::replace_all(s, L"(", L"\\(");
    boost::algorithm::replace_all(s, L")", L"\\)");
    boost::algorithm::replace_all(s, L"[", L"\\[");
    boost::algorithm::replace_all(s, L"]", L"\\]");
    boost::algorithm::replace_all(s, L"*", L"\\*");
    boost::algorithm::replace_all(s, L"+", L"\\+");
    boost::algorithm::replace_all(s, L"?", L"\\?");
    boost::algorithm::replace_all(s, L"/", L"\\/");
}
void wildcards_to_regex(std::wstring& s)
{
    // remove all escape
    remove_escape(s);

    boost::algorithm::replace_all(s, L"\\*\\*", L".*");
    boost::algorithm::replace_all(s, L"\\*", L"[^\\\\]*");
    boost::algorithm::replace_all(s, L"\\?", L".");
}
void nextlabs_wildcards_to_regex(std::wstring& s)
{
    // remove all escape
    remove_escape(s);

    // convert wildcards
    boost::algorithm::replace_all(s, L"\\*\\*", L".*");
    boost::algorithm::replace_all(s, L"\\*", L"[^\\\\]*");

    // also convert NextLabs wildcards
    boost::algorithm::replace_all(s, L"\\?d", L"\\d");
    boost::algorithm::replace_all(s, L"\\?D", L"\\d*");
    boost::algorithm::replace_all(s, L"\\?a", L"\\w");
    boost::algorithm::replace_all(s, L"\\?A", L"\\w*");
    boost::algorithm::replace_all(s, L"\\?c", L"[^\\\\]");
    boost::algorithm::replace_all(s, L"\\?C", L"[^\\\\]*");
    boost::algorithm::replace_all(s, L"\\?s", L"\\s");
    boost::algorithm::replace_all(s, L"\\?S", L"\\s*");
    boost::algorithm::replace_all(s, L"\\?", L"\\.");
    boost::algorithm::replace_all(s, L"\\?\\?", L"\\?");

    boost::algorithm::replace_all(s, L"!d", L"\\D");
    boost::algorithm::replace_all(s, L"!D", L"\\D*");
    boost::algorithm::replace_all(s, L"!a", L"\\W");
    boost::algorithm::replace_all(s, L"!A", L"\\W*");
    boost::algorithm::replace_all(s, L"!c", L"\\S");
    boost::algorithm::replace_all(s, L"!s", L"\\S");
    boost::algorithm::replace_all(s, L"!S", L"\\S*");
    boost::algorithm::replace_all(s, L"!!", L"!");
}

//
//  class condition_value
//

condition_value::condition_value() : property_value()
{
}

condition_value::condition_value(int v) : property_value(v)
{
}
condition_value::condition_value(unsigned int v) : property_value(v)
{
}
condition_value::condition_value(unsigned long v) : property_value(v)
{
}
condition_value::condition_value(__int64 v, NXPOLICY::VALUE_TYPE type) : property_value(v, type)
{
}
condition_value::condition_value(unsigned __int64 v) : property_value(v)
{
}
condition_value::condition_value(double v) : property_value(v)
{
}
condition_value::condition_value(bool v) : property_value(v)
{
}
condition_value::condition_value(const std::wstring& v) : property_value(v)
{
    if (!get_value().empty()) {
        const wchar_t* s = (const wchar_t*)get_value().data();
        int len = (int)(get_value().size() / sizeof(wchar_t));
        std::wstring regex_str(s, s + len);
        nextlabs_wildcards_to_regex(regex_str);
        try {
            _regex = std::wregex(regex_str, std::regex_constants::icase);
        }
        catch (const std::exception& e) {
            UNREFERENCED_PARAMETER(e);
            _regex = std::wregex();
        }
    }
}
condition_value::~condition_value()
{
    clear();
}

condition_value& condition_value::operator = (const condition_value& other)
{
    if (this != &other) {
        property_value::operator=(other);
        _regex = other.get_regex();
    }
    return *this;
}

void condition_value::clear()
{
    property_value::clear();
    _regex = std::wregex();
}

std::wstring condition_value::serialize() const
{
    std::wstring s;
    wchar_t buf[MAX_PATH] = { 0 };
    switch (get_type())
    {
    case VALUE_INTEGER:
        s = NX::string_formater(L"%I64d", as_integer());
        break;
    case VALUE_UNSIGNED_INTEGER:
        s = NX::string_formater(L"0x%I64X", as_unsigned_integer());
        break;
    case VALUE_FLOAT:
        s = NX::string_formater(L"%.4f", as_double());
        break;
    case VALUE_DATETIME:
        {
            NX::time::datetime_special_1 t(as_integer());
            s = L"\"";
            s += t.serialize(true, false);
            s += L"\"";
        }
        break;
    case VALUE_DAYSECONDS:
        {
            int seconds = (int)as_integer();
            int hours = seconds / 3600;
            seconds = seconds % 3600;
            int minutes = seconds / 60;
            seconds = seconds % 60;
            bool is_pm = false;
            if (hours > 11) {
                is_pm = true;
                if (hours > 12) {
                    hours = hours % 12;
                }
            }
            s = L"\"";
            s += NX::string_formater(L"%d:%d:%d %s", hours, minutes, seconds, is_pm ? L"PM" : L"AM");
            s += L"\"";
        }
        break;
    case VALUE_STRING:
        s = L"\"";
        s += as_string();
        s += L"\"";
        break;
    default:
        s = L"Nil";
        break;
    }

    return std::move(s);
}

bool condition_value::same_as(const condition_value& other) const noexcept
{
    if (this == &other) {
        return true;
    }
    return (get_type() == other.get_type() && get_value() == other.get_value());
}

bool condition_value::operator > (const property_value& other) const noexcept
{
    bool result = false;

    try {
        result = (0 < compare_number(other));
    }
    catch (...) {
        result = false;
    }

    return result;
}

bool condition_value::operator >= (const property_value& other) const noexcept
{
    bool result = false;

    try {
        result = (0 <= compare_number(other));
    }
    catch (...) {
        result = false;
    }

    return result;
}

bool condition_value::operator < (const property_value& other) const noexcept
{
    bool result = false;

    try {
        result = (0 > compare_number(other));
    }
    catch (...) {
        result = false;
    }

    return result;
}

bool condition_value::operator <= (const property_value& other) const noexcept
{
    bool result = false;

    try {
        result = (0 >= compare_number(other));
    }
    catch (...) {
        result = false;
    }

    return result;
}

bool condition_value::operator == (const property_value& other) const noexcept
{
    bool result = false;

    try {
        if (get_type() == NXPOLICY::VALUE_STRING) {
            result = std::regex_match(other.as_string(), get_regex());
        }
        else {
            result = (0 > compare_number(other));
        }
    }
    catch (...) {
        result = false;
    }

    return result;
}

int condition_value::compare_number(const property_value& other) const
{
    int result = 0;

    if (empty() || other.empty()) {
        throw std::exception("bad args");
    }

    if (get_type() == NXPOLICY::VALUE_INTEGER || get_type() == NXPOLICY::VALUE_DATETIME || get_type() == VALUE_DAYSECONDS) {
        result = integer_compare(as_integer(), other.as_integer());
    }
    else if (get_type() == NXPOLICY::VALUE_UNSIGNED_INTEGER) {
        result = integer_compare(as_unsigned_integer(), other.as_unsigned_integer());
    }
    else if (get_type() == NXPOLICY::VALUE_FLOAT) {
        result = double_compare(as_double(), other.as_double());
    }
    else {
        throw std::exception("bad type");
    }

    return result;
}


//
//  class condition_map
//



condition_map::condition_map()
{
}

condition_map::~condition_map()
{
}

void condition_map::insert(const std::wstring& s, const std::shared_ptr<condition_object>& sp)
{
    if (sp != nullptr) {
        std::wstring ls(s);
        std::transform(ls.begin(), ls.end(), ls.begin(), tolower);
        _map[ls] = sp;
    }
}

void condition_map::remove(const std::wstring& s)
{
    std::wstring ls(s);
    std::transform(ls.begin(), ls.end(), ls.begin(), tolower);
    _map.erase(ls);
}

std::shared_ptr<condition_object> condition_map::find(const std::wstring& s)
{
    std::wstring ls(s);
    std::transform(ls.begin(), ls.end(), ls.begin(), tolower);
    auto it = _map.find(ls);
    return (it != _map.end()) ? (*it).second : std::shared_ptr<condition_object>(nullptr);
}

condition_map& condition_map::operator = (const condition_map& other)
{
    if (this != &other) {
        _map = other.get_map();
    }
    return *this;
}


//
//  class condition_expression_object
//


condition_expression_object::condition_expression_object() : condition_object(), _relationship(NXPOLICY_IMPL::NIL)
{
}
condition_expression_object::condition_expression_object(bool opposite, LOGIC_OPERATOR relationship) : condition_object(), _opposite(opposite), _relationship(relationship)
{
}
condition_expression_object::~condition_expression_object()
{
}

condition_expression_object* condition_expression_object::create_condition(bool opposite, LOGIC_OPERATOR relationship)
{
    return new condition_expression_object(opposite, relationship);
}

bool condition_expression_object::evaluate(const NXPOLICY::property_repository& properties) const
{
    bool result = true;
    auto pos = std::find_if_not(_children.begin(), _children.end(), [&](const std::shared_ptr<condition_object>& sp) -> bool {
        return sp->evaluate(properties);
    });
    return (pos == _children.end()) ? true : false;
}

std::wstring condition_expression_object::serialize() const
{
    const std::wstring logic_op(is_and() ? L"AND" : L"OR");
    std::wstring s;

    if (is_opposite()) {
        s = L"NOT (";
    }
    else {
        s = L"(";
    }

    std::for_each(_children.begin(), _children.end(), [&](const std::shared_ptr<condition_object>& sp) {
        if (s.length() != 0 && L'(' != s[s.length() - 1]) {
            s += L" ";
            s += logic_op;
            s += L" ";
        }
        s += sp->serialize();
    });

    s += L")";
    return std::move(s);
}


//
//  class condition_value_object
//

condition_value_object::condition_value_object() : condition_object()
{
}

condition_value_object::condition_value_object(const std::wstring& name, COMPARE_OPERATOR op, int v) : condition_object(), _property_name(name), _cmp_value(condition_value(v)), _cmp_operator(op)
{
    std::transform(_property_name.begin(), _property_name.end(), _property_name.begin(), tolower);
}
condition_value_object::condition_value_object(const std::wstring& name, COMPARE_OPERATOR op, unsigned int v) : condition_object(), _property_name(name), _cmp_value(condition_value(v)), _cmp_operator(op)
{
    std::transform(_property_name.begin(), _property_name.end(), _property_name.begin(), tolower);
}
condition_value_object::condition_value_object(const std::wstring& name, COMPARE_OPERATOR op, unsigned long v) : condition_object(), _property_name(name), _cmp_value(condition_value(v)), _cmp_operator(op)
{
    std::transform(_property_name.begin(), _property_name.end(), _property_name.begin(), tolower);
}
condition_value_object::condition_value_object(const std::wstring& name, COMPARE_OPERATOR op, __int64 v, NXPOLICY::VALUE_TYPE type) : condition_object(), _property_name(name), _cmp_value(condition_value(v, type)), _cmp_operator(op)
{
    std::transform(_property_name.begin(), _property_name.end(), _property_name.begin(), tolower);
}
condition_value_object::condition_value_object(const std::wstring& name, COMPARE_OPERATOR op, unsigned __int64 v) : condition_object(), _property_name(name), _cmp_value(condition_value(v)), _cmp_operator(op)
{
    std::transform(_property_name.begin(), _property_name.end(), _property_name.begin(), tolower);
}
condition_value_object::condition_value_object(const std::wstring& name, COMPARE_OPERATOR op, double v) : condition_object(), _property_name(name), _cmp_value(condition_value(v)), _cmp_operator(op)
{
    std::transform(_property_name.begin(), _property_name.end(), _property_name.begin(), tolower);
}
condition_value_object::condition_value_object(const std::wstring& name, COMPARE_OPERATOR op, bool v) : condition_object(), _property_name(name), _cmp_value(condition_value(v)), _cmp_operator(op)
{
    std::transform(_property_name.begin(), _property_name.end(), _property_name.begin(), tolower);
}
condition_value_object::condition_value_object(const std::wstring& name, COMPARE_OPERATOR op, const std::wstring& v) : condition_object(), _property_name(name), _cmp_value(condition_value(v)), _cmp_operator(op)
{
    std::transform(_property_name.begin(), _property_name.end(), _property_name.begin(), tolower);
}

condition_value_object* condition_value_object::create_condition(const std::wstring& name, COMPARE_OPERATOR op, int v)
{
    return new condition_value_object(name, op, v);
}
condition_value_object* condition_value_object::create_condition(const std::wstring& name, COMPARE_OPERATOR op, unsigned int v)
{
    return new condition_value_object(name, op, v);
}
condition_value_object* condition_value_object::create_condition(const std::wstring& name, COMPARE_OPERATOR op, unsigned long v)
{
    return new condition_value_object(name, op, v);
}
condition_value_object* condition_value_object::create_condition(const std::wstring& name, COMPARE_OPERATOR op, __int64 v, NXPOLICY::VALUE_TYPE type)
{
    return new condition_value_object(name, op, v, type);
}
condition_value_object* condition_value_object::create_condition(const std::wstring& name, COMPARE_OPERATOR op, unsigned __int64 v)
{
    return new condition_value_object(name, op, v);
}
condition_value_object* condition_value_object::create_condition(const std::wstring& name, COMPARE_OPERATOR op, double v)
{
    return new condition_value_object(name, op, v);
}
condition_value_object* condition_value_object::create_condition(const std::wstring& name, COMPARE_OPERATOR op, bool v)
{
    return new condition_value_object(name, op, v);
}
condition_value_object* condition_value_object::create_condition(const std::wstring& name, COMPARE_OPERATOR op, const std::wstring& v)
{
    return new condition_value_object(name, op, v);
}

bool condition_value_object::evaluate(const NXPOLICY::property_repository& properties) const
{
    NXPOLICY::property_repository::CONST_RANGE range = properties.find(get_property_name());

    if (range.first == range.second) {
        // not found
        return properties.ignore_nonexist() ? true : false;
    }

    for (property_repository::CONST_ITERATOR it = range.first; it != range.second; ++it) {
        if (compare((*it).second)) {
           return true;
        }
    }

    return false;
}

static const wchar_t* operator_to_name(COMPARE_OPERATOR op)
{
    switch (op)
    {
    case NXPOLICY_IMPL::EqualTo:
        return L"=";
    case NXPOLICY_IMPL::NotEqualTo:
        return L"!=";
    case NXPOLICY_IMPL::GreaterThan:
        return L">";
    case NXPOLICY_IMPL::GreaterThanOrEqualTo:
        return L">=";
    case NXPOLICY_IMPL::LessThan:
        return L"<";
    case NXPOLICY_IMPL::LessThanOrEqualTo:
        return L"<=";
    default:
        break;
    }
    return L"";
}

std::wstring condition_value_object::serialize() const
{
    std::wstring s;

    s += get_property_name();
    s += L" ";
    s += operator_to_name(_cmp_operator);
    s += L" ";
    s += _cmp_value.serialize();

    return std::move(s);
}


bool condition_value_object::compare(const NXPOLICY::property_value& other) const noexcept
{
    bool result = false;

    switch (_cmp_operator)
    {
    case EqualTo:               // other == this
        result = (_cmp_value == other);
        break;
    case NotEqualTo:            // other != this
        result = !(_cmp_value == other);
        break;
    case GreaterThan:           // other > this
        result = (_cmp_value < other);
        break;
    case GreaterThanOrEqualTo:  // other >= this
        result = (_cmp_value <= other);
        break;
    case LessThan:              // other < this
        result = (_cmp_value > other);
        break;
    case LessThanOrEqualTo:     // other <= this
        result = (_cmp_value >= other);
        break;
    default:
        break;
    }

    return result;
}

condition_const_value_object::condition_const_value_object() : condition_object(), _const_value(true)
{
}

condition_const_value_object::condition_const_value_object(bool v) : condition_object(), _const_value(v)
{
}

condition_const_value_object* condition_const_value_object::create_condition(bool v)
{
    return new condition_const_value_object(v);
}

bool condition_const_value_object::evaluate(const NXPOLICY::property_repository& properties) const
{
    return _const_value;
}

std::wstring condition_const_value_object::serialize() const
{
    return std::wstring(_const_value ? L"TRUE" : L"FALSE");
}


condition_dynamic_value_object::condition_dynamic_value_object() : condition_object()
{
}

condition_dynamic_value_object::condition_dynamic_value_object(const std::wstring& property_name, COMPARE_OPERATOR op, const std::wstring& cmp_property_name) : condition_object(), _property_name(property_name), _cmp_operator(op), _cmp_property_name(cmp_property_name)
{
    // property name is always lower case
    std::transform(_property_name.begin(), _property_name.end(), _property_name.begin(), tolower);
    std::transform(_cmp_property_name.begin(), _cmp_property_name.end(), _cmp_property_name.begin(), tolower);
}

condition_dynamic_value_object* condition_dynamic_value_object::create_condition(const std::wstring& property_name, COMPARE_OPERATOR op, const std::wstring& cmp_property_name)
{
    return new condition_dynamic_value_object(property_name, op, cmp_property_name);
}

bool condition_dynamic_value_object::evaluate(const NXPOLICY::property_repository& properties) const
{
    NXPOLICY::property_repository::CONST_RANGE range_cmp_prop = properties.find(get_cmp_property_name());

    if (range_cmp_prop.first == range_cmp_prop.second) {
        // not found
        return properties.ignore_nonexist() ? true : false;
    }

    bool result = false;

    for (property_repository::CONST_ITERATOR it = range_cmp_prop.first; it != range_cmp_prop.second; ++it) {

        std::shared_ptr<condition_value_object> sp;
        NXPOLICY::VALUE_TYPE vtype = (*it).second.get_type();
        switch (vtype)
        {
        case VALUE_INTEGER:
        case VALUE_DATETIME:
        case VALUE_DAYSECONDS:
            sp = std::shared_ptr<condition_value_object>(condition_value_object::create_condition(get_property_name(), get_cmp_operator(), (*it).second.as_integer()));
            break;
        case VALUE_UNSIGNED_INTEGER:
            sp = std::shared_ptr<condition_value_object>(condition_value_object::create_condition(get_property_name(), get_cmp_operator(), (*it).second.as_unsigned_integer()));
            break;
        case VALUE_FLOAT:
            sp = std::shared_ptr<condition_value_object>(condition_value_object::create_condition(get_property_name(), get_cmp_operator(), (*it).second.as_double()));
            break;
        case VALUE_STRING:
            sp = std::shared_ptr<condition_value_object>(condition_value_object::create_condition(get_property_name(), get_cmp_operator(), (*it).second.as_string()));
            break;
        default:
            break;
        }

        if (sp != nullptr) {
            result = sp->evaluate(properties);
        }

        if (result) {
            break;
        }
    }

    return result;
}

std::wstring condition_dynamic_value_object::serialize() const
{
    std::wstring s;

    s += get_property_name();
    s += L" ";
    s += operator_to_name(_cmp_operator);
    s += L" ";
    s += _cmp_property_name;

    return std::move(s);
}

//
//  class policy_object
//
/*
    {
        "id": 100,
        "description": "Test policy",
        "path": "gye/test group/test policy",
        "condition": {
            "subject": "user.GROUP has 2 AND ((application.path = \"**\\Excel.exe\" AND application.publisher = \"Microsoft Corporation\") OR (application.path = \"**\\AcroRd32.exe\" AND application.publisher = \"Adobe Systems, Incorporated\"))",
            "resource": "(resource.fso.course = \"2300 Conductor Etch Process\" AND resource.fso.year = \"2015\")",
            "environment": ""
        },
        "rights": {
            "action": "grant",    // or "revoke"
            "value": 0x0000001
        },
        "obligation": {
            "overlay": {
            },
            "classify": {
            }
        }
    }
*/

//
//  class obligation_object
//

obligation_object::obligation_object()
{
}

obligation_object::obligation_object(const std::wstring& id, const std::map<std::wstring, std::wstring>& parameters) : _id(id), _parameters(parameters)
{
}

obligation_object::~obligation_object()
{
}

bool obligation_object::same_as(std::shared_ptr<obligation_object>& other) const
{
    return (get_id() == other->get_id() && get_parameters() == other->get_parameters());
}


//
//
//

policy_object::policy_object() : _rights(0)
{
}

policy_object::policy_object(const std::wstring& id,
    const std::wstring& description,
    POLICY_ACTION action,
    unsigned __int64 rights,
    std::shared_ptr<condition_object> subject_condition,
    std::shared_ptr<condition_object> resource_condition,
    std::shared_ptr<condition_object> environment_condition
    ) : _id(id),
        _description(description),
        _action(action),
        _rights(rights),
        _subject_condition(subject_condition),
        _resource_condition(resource_condition),
        _environment_condition(environment_condition)
{
}

policy_object::policy_object(POLICY_ACTION action, unsigned __int64 rights, const std::map<POLICY_STRING_TYPE, std::wstring>& elements) : _action(action), _rights(rights)
{
    parse(elements);
}

policy_object::~policy_object()
{
}

std::wstring policy_object::serialize() const
{
    std::wstringstream wss(L"");

    wss << L"{";


    wss << L"\"ID\": \"" << get_id() << L"\",";
    wss << L"\"DESCRIPTION\": \"" << get_description() << L"\",";
    wss << L"\"ACTION\": \"" << serialize_action() << L"\",";
    wss << L"\"RIGHTS\": \"" << serialize_rights() << L"\",";
    wss << L"\"CONDITION\": {";
    wss << L"\"subject\": \"" << serialize_subject_condition() << L"\",";
    wss << L"\"resource\": \"" << serialize_resource_condition() << L"\",";
    wss << L"\"environment\": \"" << serialize_environment_condition() << L"\"";
    wss << L"}";

    wss << L"}";

    return std::move(wss.str());
}

std::wstring policy_object::serialize_action() const
{
    return (GRANT_RIGHTS == get_action()) ? L"Grant" : L"Revoke";
}

std::wstring policy_object::serialize_rights() const
{
    std::wstring s;

    if (0 != (BUILTIN_RIGHT_VIEW & get_rights())) {
        if (!s.empty()) s += L", ";
        s += RIGHT_DISP_VIEW;
    }
    if (0 != (BUILTIN_RIGHT_EDIT & get_rights())) {
        if (!s.empty()) s += L", ";
        s += RIGHT_DISP_EDIT;
    }
    if (0 != (BUILTIN_RIGHT_PRINT & get_rights())) {
        if (!s.empty()) s += L", ";
        s += RIGHT_DISP_PRINT;
    }
    if (0 != (BUILTIN_RIGHT_CLIPBOARD & get_rights())) {
        if (!s.empty()) s += L", ";
        s += RIGHT_DISP_CLIPBOARD;
    }
    if (0 != (BUILTIN_RIGHT_SAVEAS & get_rights())) {
        if (!s.empty()) s += L", ";
        s += RIGHT_DISP_SAVEAS;
    }
    if (0 != (BUILTIN_RIGHT_DECRYPT & get_rights())) {
        if (!s.empty()) s += L", ";
        s += RIGHT_DISP_DECRYPT;
    }
    if (0 != (BUILTIN_RIGHT_SCREENCAP & get_rights())) {
        if (!s.empty()) s += L", ";
        s += RIGHT_DISP_SCREENCAP;
    }
    if (0 != (BUILTIN_RIGHT_SEND & get_rights())) {
        if (!s.empty()) s += L", ";
        s += RIGHT_DISP_SEND;
    }
    if (0 != (BUILTIN_RIGHT_CLASSIFY & get_rights())) {
        if (!s.empty()) s += L", ";
        s += RIGHT_DISP_CLASSIFY;
    }

    return std::move(s);
}

std::wstring policy_object::serialize_subject_condition() const
{
    std::wstring s = (get_subject_condition() == nullptr) ? L"" : get_subject_condition()->serialize();
    boost::algorithm::replace_all(s, L"\\", L"\\\\");
    boost::algorithm::replace_all(s, L"\"", L"\\\"");
    return std::move(s);
}

std::wstring policy_object::serialize_resource_condition() const
{
    std::wstring s = (get_resource_condition() == nullptr) ? L"" : get_resource_condition()->serialize();
    boost::algorithm::replace_all(s, L"\\", L"\\\\");
    boost::algorithm::replace_all(s, L"\"", L"\\\"");
    return std::move(s);
}

std::wstring policy_object::serialize_environment_condition() const
{
    std::wstring s = (get_environment_condition() == nullptr) ? L"" : get_environment_condition()->serialize();
    boost::algorithm::replace_all(s, L"\\", L"\\\\");
    boost::algorithm::replace_all(s, L"\"", L"\\\"");
    return std::move(s);
}

unsigned __int64 policy_object::evaluate(const NXPOLICY::property_repository& repo) const
{
    unsigned __int64 result = 0;

    try {

        if (get_subject_condition()->evaluate(repo) && get_resource_condition()->evaluate(repo) && get_environment_condition()->evaluate(repo)) {
            result = _rights;
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        result = 0;
    }

    return result;
}

void policy_object::parse(const std::map<policy_object::POLICY_STRING_TYPE, std::wstring>& elements)
{
    // parse title
    // parse attributes
    // parse subject condition
    // parse resource condition
    // parse environment condition
    // parse obligation
}

void policy_object::parse_title(const std::wstring& s)
{
}

void policy_object::parse_subject_condition(const std::wstring& s)
{
}

void policy_object::parse_resource_condition(const std::wstring& s)
{
}

void policy_object::parse_environment_condition(const std::wstring& s)
{
}

void policy_object::parse_rights(const std::wstring& s)
{
}

void policy_object::parse_obligation(const std::wstring& s)
{
}




//
//  class policy_bundle
//

policy_bundle::policy_bundle()
{
}

policy_bundle::~policy_bundle()
{
}

// default is in JSON format
void policy_bundle::load(const std::wstring& s)
{
}

// serialize to JSON format
std::wstring policy_bundle::serialize() const
{
    std::wstringstream wss(L"");
    bool first = true;

    wss << L"{";
    std::for_each(_policy_map.begin(), _policy_map.end(), [&](const std::pair<std::wstring, std::shared_ptr<policy_object>>& item) {
        if (!first) {
            wss << L",";
        }
        else {
            first = false;
        }
        wss << L"\"" << item.first << L"\":" << item.second->serialize();
    });
    wss << L"}";

    return std::move(wss.str());
}