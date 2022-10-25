

#include <Windows.h>
#include <assert.h>

#include <nudf\string.hpp>
#include <nudf\time.hpp>
#include <nudf\crypto.hpp>
#include <nudf\shared\rightsdef.h>
#include <nudf\web\json.hpp>

#include <boost\algorithm\string.hpp>

#include "nxrmserv.h"
#include "session.hpp"
#include "policy.hpp"


using namespace NX;
using namespace NX::EVAL;





//
//  class NX::EVAL::value_object
//

NX::EVAL::value_object::value_object() : _t(value_string), _expandable(false)
{
    _v.u = 0;
}
NX::EVAL::value_object::value_object(int v) : _t(value_signed_integer), _expandable(false)
{
    _v.n = (unsigned int)v;
    swprintf_s(nudf::string::tempstr<wchar_t>(_s, 128), 128, L"%d", v);
}
NX::EVAL::value_object::value_object(unsigned int v) : _t(value_unsigned_integer), _expandable(false)
{
    _v.u = v;
    swprintf_s(nudf::string::tempstr<wchar_t>(_s, 12), 12, L"0x%08X", v);
}
NX::EVAL::value_object::value_object(__int64 v) : _t(value_signed_integer), _expandable(false)
{
    _v.n = v;
    swprintf_s(nudf::string::tempstr<wchar_t>(_s, 128), 128, L"%I64d", v);
}
NX::EVAL::value_object::value_object(unsigned __int64 v) : _t(value_unsigned_integer), _expandable(false)
{
    _v.u = v;
    swprintf_s(nudf::string::tempstr<wchar_t>(_s, 20), 20, L"0x%08X`%08X", (unsigned int)(v >> 32), (unsigned int)v);
}
NX::EVAL::value_object::value_object(bool v) : _t(value_boolean), _s(v ? L"true" : L"false"), _expandable(false)
{
    _v.u = v ? 1 : 0;
}
NX::EVAL::value_object::value_object(float v) : _t(value_float), _expandable(false)
{
    _v.b = v;
    swprintf_s(nudf::string::tempstr<wchar_t>(_s, 128), 128, L"%.3f", v);
}
NX::EVAL::value_object::value_object(double v) : _t(value_float), _expandable(false)
{
    _v.b = v;
    swprintf_s(nudf::string::tempstr<wchar_t>(_s, 128), 128, L"%.5f", v);
}
NX::EVAL::value_object::value_object(const SYSTEMTIME& v, bool local/* = false*/) : _t(value_date), _expandable(false)
{
    FILETIME ft = { 0, 0 };
    SystemTimeToFileTime(&v, &ft);
    if (local) {
        FILETIME uft = { 0, 0 };
        LocalFileTimeToFileTime(&ft, &uft);
        ft = uft;
    }
    _v.u = ft.dwHighDateTime;
    _v.u <<= 32;
    _v.u |= ft.dwLowDateTime;
}
NX::EVAL::value_object::value_object(FILETIME v, bool local/* = false*/) : _t(value_date), _expandable(false)
{
    if (local) {
        FILETIME uft = { 0, 0 };
        LocalFileTimeToFileTime(&v, &uft);
        v = uft;
    }
    _v.u = v.dwHighDateTime;
    _v.u <<= 32;
    _v.u |= v.dwLowDateTime;

    SYSTEMTIME st = { 0 };
    FileTimeToSystemTime(&v, &st);
    swprintf_s(nudf::string::tempstr<wchar_t>(_s, 128), 128, L"%04d-%02d-%02dT%02d:%02d:%02dZ", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
}
NX::EVAL::value_object::value_object(const std::string& v, bool expandable /*= false*/) : _t(value_string), _s(NX::utility::conversions::utf8_to_utf16(v)), _expandable(expandable)
{
    std::transform(_s.begin(), _s.end(), _s.begin(), tolower);
}
NX::EVAL::value_object::value_object(const std::wstring& v, bool expandable /*= false*/) : _t(value_string), _s(v), _expandable(expandable)
{
    std::transform(_s.begin(), _s.end(), _s.begin(), tolower);
}
NX::EVAL::value_object::~value_object()
{
}
static FILETIME parse_nextlabs_date(const std::wstring& date)
{
    SYSTEMTIME  st = { 0 };
    FILETIME    ft = { 0, 0 };

    memset(&st, 0, sizeof(st));
    // Aug 27, 2015 4:15:39 PM
    if (boost::algorithm::istarts_with(date, L"Jan")) {
        st.wMonth = 1;
    }
    else if (boost::algorithm::istarts_with(date, L"Feb")) {
        st.wMonth = 2;
    }
    else if (boost::algorithm::istarts_with(date, L"Mar")) {
        st.wMonth = 3;
    }
    else if (boost::algorithm::istarts_with(date, L"Apr")) {
        st.wMonth = 4;
    }
    else if (boost::algorithm::istarts_with(date, L"May")) {
        st.wMonth = 5;
    }
    else if (boost::algorithm::istarts_with(date, L"Jun")) {
        st.wMonth = 6;
    }
    else if (boost::algorithm::istarts_with(date, L"Jul")) {
        st.wMonth = 7;
    }
    else if (boost::algorithm::istarts_with(date, L"Aug")) {
        st.wMonth = 8;
    }
    else if (boost::algorithm::istarts_with(date, L"Sep")) {
        st.wMonth = 9;
    }
    else if (boost::algorithm::istarts_with(date, L"Oct")) {
        st.wMonth = 10;
    }
    else if (boost::algorithm::istarts_with(date, L"Nov")) {
        st.wMonth = 11;
    }
    else if (boost::algorithm::istarts_with(date, L"Dec")) {
        st.wMonth = 12;
    }
    else {
        return ft;
    }

    bool is_pm = boost::algorithm::iends_with(date, L" PM");
    int year = 0, day = 0, hours = 0, minutes = 0, seconds = 0;
    std::wstring digit_part = date.substr(4);
    digit_part = digit_part.substr(0, digit_part.length() - 3);
    // 27, 2015 4:15:39
    swscanf_s(digit_part.c_str(), L"%d, %d %d:%d:%d", &day, &year, &hours, &minutes, &seconds);

    // validate
    if (day < 0 || day > 31) {
        return ft;
    }
    if (hours < 0 || (is_pm && hours > 12) || (!is_pm && hours > 23)) {
        return ft;
    }
    if (minutes < 0 || minutes > 59) {
        return ft;
    }

    st.wYear = year;
    st.wDay = day;
    st.wHour = is_pm ? (12 + hours%12) : hours;
    st.wMinute = minutes;
    st.wSecond = seconds;
    // Now we have a local time
    SystemTimeToFileTime(&st, &ft);
    // Convert it to UTC time
    LocalFileTimeToFileTime(&ft, &ft);

    return ft;
}
NX::EVAL::value_object NX::EVAL::value_object::parse(const std::wstring& v, escape_type escape)
{
    static const std::wregex reg_quota = std::wregex(L"^\".*\"$", std::regex_constants::icase);
    static const std::wregex reg_integer = std::wregex(L"^[+-]?[0-9]+$", std::regex_constants::icase);
    static const std::wregex reg_hex = std::wregex(L"^0x[0-9a-fA-F]+$", std::regex_constants::icase);
    static const std::wregex reg_float = std::wregex(L"^[+-]?[0-9]?\\.[0-9]+$", std::regex_constants::icase);
    static const std::wregex reg_exponent = std::wregex(L"^\\d?\\.\\d+e[+-]\\d+$", std::regex_constants::icase);
    // Aug 27, 2015 4:15:39 PM
    static const std::wregex reg_date_nextlabs = std::wregex(L"^(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)\\s\\d{1,2},\\s\\d{4}\\s\\d{1,2}:\\d{1,2}:\\d{1,2}(\\s[AaPp]M){0,1}$", std::regex_constants::icase);
    static const std::wregex reg_date_rfc1123 = std::wregex(L"^(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)\\s\\d+,\\s\\d{4}\\s\\d{1,2}:\\d{1,2}:\\d{1,2}(\\s[AaPp]M){0,1}$", std::regex_constants::icase);
    static const std::wregex reg_date_iso8601 = std::wregex(L"^\\d{4}[ -]\\d{1,2}[ -]\\d{1,2}[ T]\\d{1,2}:\\d{1,2}:\\d{1,2}(\\.\\d{1,3}){0,1}[Z]?$", std::regex_constants::icase);
    static const std::wregex reg_true = std::wregex(L"^true|yes$", std::regex_constants::icase);
    static const std::wregex reg_false = std::wregex(L"^false|no$", std::regex_constants::icase);
    static const std::wregex reg_expandable = std::wregex(L"^(user\\.|application\\.|resource\\.fso\\.|host\\.|environment\\.)[^\\.]*$", std::regex_constants::icase);

    // the value with quota, it must be a string type
    if (std::regex_match(v, reg_quota)) {
        std::wstring ws = v;
        boost::algorithm::trim(ws);
        while (boost::algorithm::starts_with(ws, L"\"")) ws = ws.substr(1);
        while (boost::algorithm::ends_with(ws, L"\"")) ws = ws.substr(0, ws.length() - 1);
        boost::algorithm::trim_copy(ws);

        // is expandable?
        if (std::regex_match(ws, reg_expandable)) {
            // don't make any change
            // engine will replace this value with the one in attributes map
            // dynamically when evaluation happen
            return value_object(ws, true);
        }

        // need to remov escape
        if (escape != none_escape) {
            (escape == nextlabs_wildcard) ? value_object::nextlabs_wildcards_to_regex(ws) : value_object::wildcards_to_regex(ws);
            return value_object(ws);
        }
        return NX::EVAL::value_object(NX::EVAL::value_object::nextlabs_wildcards_to_regex_copy(ws), false);
    }

    // boolean
    if (std::regex_match(v, reg_true)) {
        return NX::EVAL::value_object(true);
    }
    if (std::regex_match(v, reg_false)) {
        return NX::EVAL::value_object(false);
    }

    // integer
    if (std::regex_match(v, reg_integer)) {
        wchar_t* end = nullptr;
        __int64 n = std::wcstoll(v.c_str(), &end, 0);
        return NX::EVAL::value_object(n);
    }

    // hex
    if (std::regex_match(v, reg_hex)) {
        wchar_t* end = nullptr;
        unsigned __int64 u = std::wcstoull(v.c_str(), &end, 16);
        return NX::EVAL::value_object(u);
    }

    // float or exponent
    if (std::regex_match(v, reg_float) || std::regex_match(v, reg_exponent)) {
        wchar_t* end = nullptr;
        double b = std::wcstod(v.c_str(), &end);
        return NX::EVAL::value_object(b);
    }

    // float
    if (std::regex_match(v, reg_float)) {
        wchar_t* end = nullptr;
        double b = std::wcstod(v.c_str(), &end);
        return NX::EVAL::value_object(b);
    }

    // nextlabs date
    if (std::regex_match(v, reg_date_nextlabs)) {
        // Aug 27, 2015 4:15:39 PM
        FILETIME ft = { 0, 0 };
        ft = parse_nextlabs_date(v);
        if (0 == ft.dwHighDateTime && 0 == ft.dwLowDateTime) {
            // bad value
            LOGERR(ERROR_INVALID_DATA, L"value_object: bad nextlabs date %s", v.c_str())
        }
        return NX::EVAL::value_object(ft, false);
    }

    // Not match any of them?
    // treat as a string
    // is expandable?
    if (std::regex_match(v, reg_expandable)) {
        // don't make any change
        // engine will replace this value with the one in attributes map
        // dynamically when evaluation happen
        return value_object(v, true);
    }
    if (escape != none_escape) {
        return value_object((escape == nextlabs_wildcard) ? value_object::nextlabs_wildcards_to_regex_copy(v) : value_object::wildcards_to_regex_copy(v));
    }
    return NX::EVAL::value_object(NX::EVAL::value_object::nextlabs_wildcards_to_regex_copy(v), false);
}
void NX::EVAL::value_object::remove_escape(std::wstring& s)
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
void NX::EVAL::value_object::wildcards_to_regex(std::wstring& s)
{
    // remove all escape
    remove_escape(s);

    boost::algorithm::replace_all(s, L"\\*\\*", L".*");
    boost::algorithm::replace_all(s, L"\\*", L"[^\\\\]*");
    boost::algorithm::replace_all(s, L"\\?", L".");
}
void NX::EVAL::value_object::nextlabs_wildcards_to_regex(std::wstring& s)
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
std::wstring NX::EVAL::value_object::wildcards_to_regex_copy(const std::wstring& s)
{
    std::wstring ws(s);
    NX::EVAL::value_object::wildcards_to_regex(ws);
    return std::move(ws);
}
std::wstring NX::EVAL::value_object::nextlabs_wildcards_to_regex_copy(const std::wstring& s)
{
    std::wstring ws(s);
    NX::EVAL::value_object::nextlabs_wildcards_to_regex(ws);
    return std::move(ws);
}
std::wstring NX::EVAL::value_object::remove_escape_copy(const std::wstring& s)
{
    std::wstring ws(s);
    NX::EVAL::value_object::remove_escape(ws);
    return std::move(ws);
}
bool NX::EVAL::value_object::as_boolean() const
{
    if (is_string() || is_float()) {
        throw std::exception("not a boolean value");
    }
    return (0 == _v.u) ? false : true;
}
int NX::EVAL::value_object::as_int() const
{
    if (is_string()) {
        throw std::exception("not a int value");
    }
    return static_cast<int>(_v.n);
}
__int64 NX::EVAL::value_object::as_int64() const
{
    if (is_string()) {
        throw std::exception("not a int64 value");
    }
    return static_cast<__int64>(_v.n);
}
unsigned int NX::EVAL::value_object::as_uint() const
{
    if (is_string()) {
        throw std::exception("not a unsigned int value");
    }
    return static_cast<unsigned int>(_v.u);
}
unsigned __int64 NX::EVAL::value_object::as_uint64() const
{
    if (is_string()) {
        throw std::exception("not a unsigned int64 value");
    }
    return _v.u;
}
float NX::EVAL::value_object::as_float() const
{
    if (is_string()) {
        throw std::exception("not a float value");
    }
    return static_cast<float>(_v.b);
}
double NX::EVAL::value_object::as_double() const
{
    if (is_string()) {
        throw std::exception("not a float value");
    }
    return _v.b;
}
FILETIME NX::EVAL::value_object::as_date(bool local/* = false*/) const
{
    if (is_string()) {
        throw std::exception("not a float value");
    }
    FILETIME ft = { 0, 0 };
    ft.dwHighDateTime = (unsigned long)(_v.u >> 32);
    ft.dwLowDateTime = (unsigned long)_v.u;
    if (local) {
        // need to convert utc date-time to local date-time
        FILETIME lft = { 0, 0 };
        FileTimeToLocalFileTime(&ft, &lft);
        ft = lft;
    }
    return ft;
}
unsigned __int64 NX::EVAL::value_object::as_date_uint64(bool local/* = false*/) const
{
    if (is_string()) {
        throw std::exception("not a float value");
    }
    return _v.u;
}
std::wstring NX::EVAL::value_object::serialize() const noexcept
{
    return _s;
}

static const double float_precision = 0.0000001;
bool NX::EVAL::value_object::operator == (const value_object& other) const noexcept
{
    bool result = false;

    assert(!expandable());
    assert(!other.expandable());

    if (type() != other.type()) {
        return result;
    }

    switch (type())
    {
    case value_boolean:
        result = (as_boolean() == other.as_boolean());
        break;
    case value_signed_integer:
        result = (as_int64() == other.as_int64());
        break;
    case value_unsigned_integer:
        result = (as_uint64() == other.as_uint64());
        break;
    case value_float:
        result = (fabs(as_double() - other.as_double()) < float_precision);
        break;
    case value_date:
        result = (as_date_uint64() == other.as_date_uint64());
        break;
    case value_string:
        try {
            std::wregex rgx(other.serialize(), std::regex_constants::icase);
            result = std::regex_match(_s, rgx);
        }
        catch (std::exception& e) {
            UNREFERENCED_PARAMETER(e);
            result = false;
        }
        break;
    default:
        result = false;
        break;
    }

    return result;
}
bool NX::EVAL::value_object::operator != (const value_object& other) const noexcept
{
    return (((*this) == other) ? false : true);
}
bool NX::EVAL::value_object::operator > (const value_object& other) const noexcept
{
    bool result = false;

    if (type() != other.type()) {
        return result;
    }

    switch (type())
    {
    case value_boolean:
    case value_signed_integer:
        result = (as_int64() > other.as_int64());
        break;
    case value_unsigned_integer:
        result = (as_uint64() > other.as_uint64());
        break;
    case value_float:
        result = ((*this) == other) ? false : (as_double() > other.as_double());
        break;
    case value_date:
        result = (as_date_uint64() > other.as_date_uint64());
        break;
    case value_string:
        result = (_wcsicmp(_s.c_str(), other.serialize().c_str()) > 0) ? true : false;
        break;
    default:
        result = false;
        break;
    }

    return result;
}
bool NX::EVAL::value_object::operator < (const value_object& other) const noexcept
{
    bool result = false;

    if (type() != other.type()) {
        return result;
    }

    switch (type())
    {
    case value_boolean:
    case value_signed_integer:
        result = (as_int64() < other.as_int64());
        break;
    case value_unsigned_integer:
        result = (as_uint64() < other.as_uint64());
        break;
    case value_float:
        result = ((*this) == other) ? false : (as_double() < other.as_double());
        break;
    case value_date:
        result = (as_date_uint64() < other.as_date_uint64());
        break;
    case value_string:
        result = (_wcsicmp(_s.c_str(), other.serialize().c_str()) < 0) ? true : false;
        break;
    default:
        result = false;
        break;
    }

    return result;
}
bool NX::EVAL::value_object::operator >= (const value_object& other) const noexcept
{
    return ((*this) < other) ? false : true;
}
bool NX::EVAL::value_object::operator <= (const value_object& other) const noexcept
{
    return ((*this) > other) ? false : true;
}
bool NX::EVAL::value_object::has(const value_object& other) const noexcept
{
    if (type() != value_string || other.type() != value_string) {
        return false;
    }
    return (_wcsicmp(_s.c_str(), other.serialize().c_str()) == 0) ? true : false;
}
bool NX::EVAL::value_object::contains(const value_object& other) const noexcept
{
    if (type() != value_string || other.type() != value_string) {
        return false;
    }
    return boost::algorithm::icontains(_s, other.serialize());
}


//
//  class NX::EVAL::attribute_multimap
//
std::vector<value_object> attribute_multimap::get(const std::wstring& key) const noexcept
{
    std::vector<value_object> result;
    auto range = _map.equal_range(key);
    if (range.first == _map.end()) {
        return result;
    }
    std::for_each(range.first, range.second, [&](const std::pair<std::wstring, value_object>& item) {
        result.push_back(item.second);
    });
    return std::move(result);
}

value_object attribute_multimap::get_single(const std::wstring& key) const noexcept
{
    auto range = _map.equal_range(key);
    if (range.first == _map.end()) {
        return value_object();
    }
    return (*(range.first)).second;
}

void attribute_multimap::insert(const std::wstring& key, const value_object& value) noexcept
{
    std::wstring k(key);
    std::transform(k.begin(), k.end(), k.begin(), tolower);
    _map.insert(std::pair<std::wstring, value_object>(k, value));
}

//
//  class NX::EVAL::attribute_map
//
bool attribute_map::exists(const std::wstring& key) const noexcept
{
    auto range = _map.equal_range(key);
    return (range.first == _map.end()) ? false : true;
}
const value_object& attribute_map::at(const std::wstring& key) const
{
    auto range = _map.equal_range(key);
    if (range.first == _map.end()) {
        throw std::exception("element not exists");
    }
    return (*range.first).second;
}
void attribute_map::insert(const std::wstring& key, const value_object& value) noexcept
{
    std::wstring k(key);
    std::transform(k.begin(), k.end(), k.begin(), tolower);
    auto pos = _map.find(k);
    if (pos == _map.end()) {
        attribute_multimap::insert(k, value);
    }
    else {
        (*pos).second = value;
    }
}


//
//  class NX::EVAL::rights
//
NX::EVAL::rights::rights(const std::vector<std::wstring/*name*/>& rsv) : _rights(0)
{
    from_string_array(rsv);
}

bool NX::EVAL::rights::full_rights() const noexcept
{
    return has(BUILTIN_RIGHT_ALL);
}

void NX::EVAL::rights::from_string(const std::wstring& rs) noexcept
{
    std::vector<std::wstring> v;
    nudf::string::Split<wchar_t>(rs, L',', v);
    from_string_array(v);
}

void NX::EVAL::rights::from_string_array(const std::vector<std::wstring>& v) noexcept
{
    _rights = 0;
    std::for_each(v.begin(), v.end(), [&](const std::wstring& s) {
        if (0 == _wcsicmp(s.c_str(), L"RIGHT_VIEW") || 0 == _wcsicmp(s.c_str(), L"VIEW")) {
            _rights |= BUILTIN_RIGHT_VIEW;
        }
        else if (0 == _wcsicmp(s.c_str(), L"RIGHT_EDIT") || 0 == _wcsicmp(s.c_str(), L"EDIT")) {
            _rights |= BUILTIN_RIGHT_EDIT;
        }
        else if (0 == _wcsicmp(s.c_str(), L"RIGHT_PRINT") || 0 == _wcsicmp(s.c_str(), L"PRINT")) {
            _rights |= BUILTIN_RIGHT_PRINT;
        }
        else if (0 == _wcsicmp(s.c_str(), L"RIGHT_CLIPBOARD") || 0 == _wcsicmp(s.c_str(), L"CLIPBOARD") || 0 == _wcsicmp(s.c_str(), L"Access Clipboard")) {
            _rights |= BUILTIN_RIGHT_CLIPBOARD;
        }
        else if (0 == _wcsicmp(s.c_str(), L"RIGHT_SAVEAS") || 0 == _wcsicmp(s.c_str(), L"SAVEAS") || 0 == _wcsicmp(s.c_str(), L"SAVE AS")) {
            _rights |= BUILTIN_RIGHT_SAVEAS;
        }
        else if (0 == _wcsicmp(s.c_str(), L"RIGHT_DECRYPT") || 0 == _wcsicmp(s.c_str(), L"DECRYPT")) {
            _rights |= BUILTIN_RIGHT_DECRYPT;
        }
        else if (0 == _wcsicmp(s.c_str(), L"RIGHT_SCREENCAP") || 0 == _wcsicmp(s.c_str(), L"SCREENCAP") || 0 == _wcsicmp(s.c_str(), L"Capture Screen") || 0 == _wcsicmp(s.c_str(), L"Screen Capture")) {
            _rights |= BUILTIN_RIGHT_SCREENCAP;
        }
        else if (0 == _wcsicmp(s.c_str(), L"RIGHT_SEND") || 0 == _wcsicmp(s.c_str(), L"SEND")) {
            _rights |= BUILTIN_RIGHT_SEND;
        }
        else if (0 == _wcsicmp(s.c_str(), L"RIGHT_CLASSIFY") || 0 == _wcsicmp(s.c_str(), L"CLASSIFY")) {
            _rights |= BUILTIN_RIGHT_CLASSIFY;
        }
        else {
            ; // ignore unknown right
        }
    });
}

std::wstring NX::EVAL::rights::serialize(const std::wstring& separator) const noexcept
{
    std::wstring s;

    if (has(BUILTIN_RIGHT_VIEW)) {
        s += L"View";
    }

    if (has(BUILTIN_RIGHT_EDIT)) {
        if (!s.empty()) s += separator;
        s += L"Edit";
    }
    if (has(BUILTIN_RIGHT_PRINT)) {
        if (!s.empty()) s += separator;
        s += L"Print";
    }
    if (has(BUILTIN_RIGHT_CLIPBOARD)) {
        if (!s.empty()) s += separator;
        s += L"Access Clipboard";
    }
    if (has(BUILTIN_RIGHT_SAVEAS)) {
        if (!s.empty()) s += separator;
        s += L"Save As";
    }
    if (has(BUILTIN_RIGHT_DECRYPT)) {
        if (!s.empty()) s += separator;
        s += L"Decrypt";
    }
    if (has(BUILTIN_RIGHT_SCREENCAP)) {
        if (!s.empty()) s += separator;
        s += L"Screen Capture";
    }
    if (has(BUILTIN_RIGHT_SEND)) {
        if (!s.empty()) s += separator;
        s += L"Send";
    }
    if (has(BUILTIN_RIGHT_CLASSIFY)) {
        if (!s.empty()) s += separator;
        s += L"Classify";
    }

    return std::move(s);
}



//
//  class NX::eval_object
//
const unsigned long eval_object::eval_mask_user   = 0x00000001;
const unsigned long eval_object::eval_mask_app    = 0x00000002;
const unsigned long eval_object::eval_mask_host   = 0x00000004;
const unsigned long eval_object::eval_mask_res    = 0x00000008;
const unsigned long eval_object::eval_mask_env    = 0x00000010;


eval_object::eval_object() : _mask(eval_mask_user|eval_mask_app| eval_mask_host|eval_mask_res|eval_mask_env)
{
    generate_id();
    _result = std::shared_ptr<eval_result>(new eval_result(_eval_id));
}

eval_object::eval_object(unsigned long mask) : _mask(mask)
{
    generate_id();
    _result = std::shared_ptr<eval_result>(new eval_result(_eval_id));
}

eval_object::~eval_object()
{
}

unsigned __int64 eval_object::generate_id()
{
    static volatile LONGLONG NextUniqueId = 1;
    if (NextUniqueId >= 0xFFFFFFFFFFFFFFF) {    // 1,152,921,504,606,846,975
        _eval_id = (ULONGLONG)InterlockedExchange64(&NextUniqueId, 1);
    }
    else {
        _eval_id = InterlockedCompareExchange64(&NextUniqueId, NextUniqueId + 1, NextUniqueId);
    }

    return _eval_id;
}


//
//  class NX::EVAL::details::obligation
//
NX::EVAL::obligation::obligation()
{
}

NX::EVAL::obligation::obligation(const std::wstring& name, const std::map<std::wstring, std::wstring>& parameters) : _name(name), _parameters(parameters)
{
    if (_name.empty()) {
        clear();
        return;
    }
    std::transform(_name.begin(), _name.end(), _name.begin(), tolower);
    _hash = calc_hash();
}

NX::EVAL::obligation::~obligation()
{
}

NX::EVAL::obligation& NX::EVAL::obligation::operator = (const NX::EVAL::obligation& other) noexcept
{
    if (this != &other) {
        _name = other.name();
        _parameters = other.parameters();
        _hash = other.hash();
    }
    return *this;
}

std::wstring NX::EVAL::obligation::serialize() const noexcept
{
    std::wstring s;
    NX::web::json::value v = NX::web::json::value::object();
    v[L"name"] = NX::web::json::value::string(_name);
    v[L"parameters"] = NX::web::json::value::object();
    std::for_each(_parameters.begin(), _parameters.end(), [&](const std::pair<std::wstring, std::wstring>& item) {
        v[L"parameters"][item.first] = NX::web::json::value::string(item.second);
    });
    s = v.serialize();
    return std::move(s);
}

std::wstring NX::EVAL::obligation::calc_hash()
{
    std::wstring s = serialize();
    std::vector<unsigned char> hash;
    nudf::crypto::ToSha1(s.c_str(), (unsigned long)(s.length()*sizeof(wchar_t)), hash);
    return nudf::string::FromBytes<wchar_t>(&hash[0], (unsigned long)hash.size());
}


//
//  class NX::policy
//

void NX::EVAL::eval_result::hit_policy(std::shared_ptr<policy> p) noexcept
{
    _policies[p->id()] = p;
    if (p->to_grant()) {
        _grant_rights += p->rights();
    }
    else {
        _revoke_rights += p->rights();
    }
    std::for_each(p->obligations().begin(), p->obligations().end(), [&](auto ob) {
        _obligations[ob->name()] = ob;
    });
}

void NX::EVAL::eval_result::hit_obligation(std::shared_ptr<obligation> p) noexcept
{
    _obligations[p->name()] = p;
}


//
//  class NX::policy
//

policy::policy() : _operator(grant)
{
}

policy::policy(const std::wstring& id, const std::wstring& name, const std::vector<std::wstring>& rsv) : _id(id), _name(name), _rights(rsv), _operator(grant)
{
    std::transform(_id.begin(), _id.end(), _id.begin(), tolower);
}

policy::~policy()
{
}

bool policy::evaluate(eval_object& eobj) noexcept
{
    std::vector<std::wstring> ignore_list;

    LOGDMP(L" ");
    LOGDMP(L"EVALUATE POLICY (%s - %s)", id().c_str(), name().c_str());

    if (!eobj.eval_app()) {
        ignore_list.push_back(L"application.");
        LOGDMP(L"  - ignore application attributes");
    }
    if (!eobj.eval_user()) {
        ignore_list.push_back(L"user.");
        LOGDMP(L"  - ignore user attributes");
    }
    if (!eobj.eval_host()) {
        ignore_list.push_back(L"host.");
        LOGDMP(L"  - ignore host attributes");
    }
    
    // always ignore user.group
    LOGDMP(L"  - ignore user group");
    ignore_list.push_back(L"user.group");
    if (eobj.eval_user() || eobj.eval_app() || eobj.eval_host()) {
        LOGDMP(L"  >>> check subject expression");
        if (!subject_expr()->evaluate(eobj.attributes(), ignore_list)) {
            return false;
        }
    }
    if (eobj.eval_res()) {
        LOGDMP(L"  >>> check resource expression");
        if (!resource_expr()->evaluate(eobj.attributes())) {
            return false;
        }
    }
    if (eobj.eval_env()) {
        LOGDMP(L"  >>> check environment expression");
        if (!env_expr()->evaluate(eobj.attributes())) {
            return false;
        }
    }

    return true;
}


//
//  class NX::policy_bundle
//

policy_bundle::policy_bundle()
{
}

policy_bundle::~policy_bundle()
{
}

std::wstring policy_bundle::local_bundle_time() const noexcept
{
    if (_time.empty()) {
        return _time;
    }

    // 
    FILETIME ft = { 0, 0 };
    bool utc = false;
    if (!nudf::string::ToSystemTime(_time, &ft, &utc)) {
        return _time;
    }
    nudf::time::CTime ct(&ft);
    SYSTEMTIME st = { 0 };
    if (utc) {
        ct.ToLocalTime(NULL);
    }
    ct.ToSystemTime(&st);
    std::wstring s;
    swprintf_s(nudf::string::tempstr<wchar_t>(s, 128), 128, L"%04d-%02d-%02d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    return s;
}

void policy_bundle::clear() noexcept
{
    _time.clear();
    _policy_map.clear();
    _condition_map.clear();
    _obligation_map.clear();
    _user_group_map.clear();
    _unknown_user_group.first.clear();
    _unknown_user_group.second.clear();
}

bool policy_bundle::empty() const noexcept
{
    return _policy_map.empty();
}

policy_bundle policy_bundle::load(const std::wstring& s)
{
    std::shared_ptr<parser> sp;

    if (boost::algorithm::istarts_with(s, L"Deployment Bundle:")) {
        // pql
        sp = std::shared_ptr<parser>(new policy_bundle::pql_parser());
    }
    else if (boost::algorithm::istarts_with(s, L"{")) {
        // json
        sp = std::shared_ptr<parser>(new policy_bundle::json_parser());
    }
    else if (boost::algorithm::istarts_with(s, L"<POLICYBUNDLE")) {
        // xml
        sp = std::shared_ptr<parser>(new policy_bundle::xml_parser());
    }
    if (sp == nullptr) {
        throw std::exception("fail to create parser object");
    }
    return sp->parse(s);
}

policy_bundle policy_bundle::load_pql_file(const std::wstring& file)
{
    throw std::exception("format not support");
}


//
//  class NX::details::expression
//


//
//  class NX::details::expression::expression_node
//
bool details::expression::expression_node::evaluate(const attribute_multimap& attributes, const std::vector<std::wstring>& ignore_list, bool check_exist_only/* = false*/) const noexcept
{
    bool result = false;

    if (_children.empty()) {
        return is_reverse() ? false : true;
    }

    std::wstring ws_expr = serialize();

    bool found = false;
    if (is_logic_and()) {
        for (auto it = _children.begin(); it != _children.end(); ++it) {
            if (!(*it)->evaluate(attributes, ignore_list, check_exist_only)) {
                // any thing doesn't match, result = false
                found = true;
                break;
            }
        }
        result = found ? false : true;
        //result = (_children.end() == std::find_if_not(_children.begin(), _children.end(), [&](std::shared_ptr<node> ch) -> bool {
        //    return ch->evaluate(attributes, ignore_list, check_exist_only);
        //}));
    }
    else if (is_logic_or()) {
        //result = (_children.end() == std::find_if(_children.begin(), _children.end(), [&](std::shared_ptr<node> ch) -> bool {
        //    return ch->evaluate(attributes, ignore_list, check_exist_only);
        //}));
        for (auto it = _children.begin(); it != _children.end(); ++it) {
            if ((*it)->evaluate(attributes, ignore_list, check_exist_only)) {
                // any thing doesn't match, result = false
                found = true;
                break;
            }
        }
        result = found ? true : false;
    }
    else {
        // unknown logic? only check first one
        assert(false);
        result = _children[0]->evaluate(attributes, ignore_list, check_exist_only);
    }

    result = is_reverse() ? (!result) : result;
    LOGDMP(L"[EvalDump] Result: %s, Expr: %s", result ? L"true" : L"false", ws_expr.c_str());
    return result;
}

std::wstring details::expression::expression_node::serialize() const noexcept
{
    std::wstring s;

    if (_children.empty()) {
        return s;
    }

    if (is_reverse()) {
        s += L"NOT ";
    }

    if (!is_root() && _children.size() > 1) {
        s += L"(";
    }

    for (auto it = _children.begin(); it != _children.end(); ++it) {
        if (it != _children.begin()) {
            if (is_logic_and()) {
                s += L" AND ";
            }
            else {
                s += L" OR ";
            }
        }
        //if ((*it)->is_reverse()) {
        //    s += L"NOT ";
        //}
        s += (*it)->serialize();
    }

    if (!is_root() && _children.size() > 1) {
        s += L")";
    }

    return std::move(s);
}


//
//  class NX::details::expression::condition_node
//
details::expression::condition_node::condition_node(expression_node* parent, const std::string& s, bool reverse) : node(parent, reverse), _condition(std::shared_ptr<condition>(new condition(s)))
{
}
details::expression::condition_node::condition_node(expression_node* parent, const std::wstring& s, bool reverse) : node(parent, reverse), _condition(std::shared_ptr<condition>(new condition(s)))
{
}
bool details::expression::condition_node::evaluate(const attribute_multimap& attributes, const std::vector<std::wstring>& ignore_list, bool check_exist_only/* = false*/) const noexcept
{
    bool result = false;

    if (_condition == nullptr) {
        return is_reverse() ? false : true;
    }

    std::wstring ws_condition = serialize();

    // we ignore user.group because the bundle has been generated base on user group already
    if (0 == _wcsicmp(_condition->name().c_str(), L"user.group")) {
        LOGDMP(L"[EvalDump] Result: ignored (true), Expr: %s", ws_condition.c_str());
        return true;
    }
    
    if (ignore_list.end() != std::find_if(ignore_list.begin(), ignore_list.end(), [&](const std::wstring& name) -> bool {
        return boost::algorithm::istarts_with(_condition->name(), name);
    })) {
        return true;
    }

    std::vector<value_object> vs = attributes.get(_condition->name());
    
    if (vs.empty()) {
        // not exist
        result = check_exist_only ? true : false;
        LOGDMP(L"[EvalDump] Result: not exist (%s), Condition: %s", result ? L"true - check exist only" : L"false", ws_condition.c_str());
    }
    else {

        if (_condition->value().expandable()) {
            // condition is something like "user.name = $(resource.fso.owner)"
            // then we need to create a new vlaue list
            std::vector<value_object> vexpanded = attributes.get(_condition->value().serialize());
            for (auto cit = vexpanded.begin(); cit != vexpanded.end(); ++cit) {
                // build a new condition with expanded value
                assert(!(*cit).expandable());
                condition tc(_condition->name(), (*cit).serialize(), _condition->op());
                for (auto it = vs.begin(); it != vs.end(); ++it) {
                    result = tc.evaluate(*it);
                    if (result) {
                        // match
                        break;
                    }
                }
                std::wstring ws_expanded_cond = tc.serialize();
                LOGDMP(L"[EvalDump]     Result: %s, Expanded Condition: %s", result ? L"true" : L"false", ws_expanded_cond.c_str());
                if (result) {
                    // match
                    break;
                }
            }
            if (vexpanded.empty()) {
                LOGDMP(L"[EvalDump]     Fail to expanded value: %s", _condition->value().serialize().c_str());
            }
            LOGDMP(L"[EvalDump] Result: %s, Condition: %s", result ? L"true" : L"false", ws_condition.c_str());
        }
        else {
            // not expandable, don't need to create temp condition
            for (auto it = vs.begin(); it != vs.end(); ++it) {
                result = get_condition()->evaluate(*it);
                if (result) {
                    // match
                    break;
                }
            }
            LOGDMP(L"[EvalDump] Result: %s, Condition: %s", result ? L"true" : L"false", ws_condition.c_str());
        }
    }

    return is_reverse() ? (!result) : result;
}

std::wstring details::expression::condition_node::serialize() const noexcept
{
    std::wstring s;

    if (is_reverse()) {
        s += L"NOT ";
    }
    s += get_condition()->serialize();

    return std::move(s);
}



//
//  class NX::details::condition
//
const std::wstring NX::EVAL::details::condition::op_eq(L"=");
const std::wstring NX::EVAL::details::condition::op_ne(L"!=");
const std::wstring NX::EVAL::details::condition::op_gt(L">");
const std::wstring NX::EVAL::details::condition::op_lt(L"<");
const std::wstring NX::EVAL::details::condition::op_ge(L">=");
const std::wstring NX::EVAL::details::condition::op_le(L"<=");
const std::wstring NX::EVAL::details::condition::op_has(L"HAS");
const std::wstring NX::EVAL::details::condition::op_contains(L"CONTAINS");
NX::EVAL::details::condition::condition() : _op(EQ)
{
}
NX::EVAL::details::condition::condition(const std::string& s) : _op(EQ)
{
    std::wstring ws = NX::utility::conversions::utf8_to_utf16(s);
    parse(ws);
    
}
NX::EVAL::details::condition::condition(const std::wstring& s) : _op(EQ)
{
    parse(s);
}
NX::EVAL::details::condition::condition(const std::wstring& name, const std::wstring& value, const std::wstring& op) : _op(EQ)
{
    std::wstring n(name);
    std::wstring v(value);
    std::wstring o(op);
    std::transform(n.begin(), n.end(), n.begin(), tolower);
    std::transform(v.begin(), v.end(), v.begin(), tolower);
    std::transform(o.begin(), o.end(), o.begin(), toupper);

    if (n.empty() || v.empty()) {
        return;
    }
    _op = string_to_operator(o);
    _name = n;
    try {
        boost::algorithm::replace_all(v, L"\\\\", L"\\");
        _value = value_object::parse(v, value_object::nextlabs_wildcard);
        calc_hash();
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        clear();
    }
}
NX::EVAL::details::condition::condition(const std::wstring& name, const std::wstring& value, condition_operator op)
{
    std::wstring n(name);
    std::wstring v(value);
    std::transform(n.begin(), n.end(), n.begin(), tolower);
    std::transform(v.begin(), v.end(), v.begin(), tolower);

    if (n.empty() || v.empty()) {
        return;
    }
    _name = n;
    _op = op;
    try {
        boost::algorithm::replace_all(v, L"\\\\", L"\\");
        _value = value_object::parse(v, value_object::nextlabs_wildcard);
        calc_hash();
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        clear();
    }
}
NX::EVAL::details::condition::~condition()
{
}
NX::EVAL::details::condition::condition_operator NX::EVAL::details::condition::string_to_operator(const std::wstring& s) noexcept
{
    if (s == op_eq || s == L"EQ") {
       return EQ;
    }
    else if (s == op_ne || s == L"NE") {
        return NE;
    }
    else if (s == op_gt || s == L"GT") {
        return GT;
    }
    else if (s == op_lt || s == L"LT") {
        return LT;
    }
    else if (s == op_ge || s == L"GE") {
        return GE;
    }
    else if (s == op_le || s == L"LE") {
        return LE;
    }
    else if (s == op_has) {
        return HAS;
    }
    else if (s == op_contains) {
        return CONTAINS;
    }
    else {
        return EQ;
    }
}
const std::wstring& NX::EVAL::details::condition::op_string() const noexcept
{
    switch (_op)
    {
    case EQ: return op_eq;
    case NE: return op_ne;
    case GT: return op_gt;
    case LT: return op_lt;
    case GE: return op_ge;
    case LE: return op_le;
    case HAS: return op_has;
    case CONTAINS: return op_contains;
    default: break;
    }
    // should never reach here
    assert(false);
    __assume(0);
}
bool NX::EVAL::details::condition::evaluate(const value_object& v) const noexcept
{
    bool result = false;
    switch (_op)
    {
    case EQ: return (v == value());
    case NE: return (v != value());
    case GT: return (v > value());
    case LT: return (v < value());
    case GE: return (v >= value());
    case LE: return (v <= value());
    case HAS: return (v.has(value()));
    case CONTAINS: return (v.contains(value()));
    default: break;
    }
    return true;
}
std::wstring NX::EVAL::details::condition::serialize() const noexcept
{
    std::wstring s(_name + L" " + op_string() + L" \"" + _value.serialize() + L"\"");
    if (_value.is_string()) {
        s = _name + L" " + op_string() + L" \"" + _value.serialize() + L"\"";
    }
    else {
        s = _name + L" " + op_string() + _value.serialize();
    }
    return std::move(s);
}
void NX::EVAL::details::condition::calc_hash()
{
    std::wstring s = serialize();
    _hash.clear();
    if (!s.empty()) {
        std::vector<unsigned char> h;
        if (!nudf::crypto::ToSha256(s.c_str(), (unsigned long)(s.length()*sizeof(wchar_t)), h)) {
            throw std::exception("fail to calculate hash");
        }
        assert(h.size() > 0);
        _hash = nudf::string::FromBytes<wchar_t>(&h[0], (unsigned long)h.size());
    }
}
void NX::EVAL::details::condition::parse(std::wstring s)
{
    std::wstring::size_type pos;
    std::wstring            value_str;
    static const std::wstring op_is_not(L"is not ");
    static const std::wstring op_is_not2(L"isnot ");

    // get name
    boost::algorithm::trim_left(s);
    pos = s.find(L' ');
    if (pos == std::wstring::npos) {
        throw std::exception("invalid condition format");
    }
    _name = s.substr(0, pos);
    std::transform(_name.begin(), _name.end(), _name.begin(), tolower);
    s = s.substr(pos + 1);
    boost::algorithm::trim_left(s);
    // get operator
    pos = s.find(L' ');
    if (pos == std::wstring::npos) {
        throw std::exception("invalid condition format");
    }
    std::wstring op = s.substr(0, pos);
    if (op == op_eq) {
        _op = EQ;
    }
    else if (op == op_ne) {
        _op = NE;
    }
    else if (op == op_gt) {
        _op = GT;
    }
    else if (op == op_lt) {
        _op = LT;
    }
    else if (op == op_ge) {
        _op = GE;
    }
    else if (op == op_le) {
        _op = LE;
    }
    else if (0 == _wcsicmp(op.c_str(), op_has.c_str())) {
        _op = HAS;
    }
    else if (0 == _wcsicmp(op.c_str(), op_contains.c_str())) {
        _op = CONTAINS;
    }
    else {
        throw std::exception("unknown condition operator");
    }
    // get value
    value_str = s.substr(pos + 1);
    boost::algorithm::trim(value_str);   /// remove L' '
    while (boost::algorithm::starts_with(value_str, L"\"")) value_str = value_str.substr(1);
    while (boost::algorithm::ends_with(value_str, L"\"")) value_str = value_str.substr(0, value_str.length()-1);
    if (value_str.empty()) {
        throw std::exception("value is empty");
    }
    // convert value to proper type
    _value = value_object::parse(value_str, value_object::nextlabs_wildcard);

    calc_hash();
}

