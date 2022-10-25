

#include <Windows.h>
#include <assert.h>

#include <string>
#include <regex>

#include "pql_value.hpp"


using namespace pql;



//
//  class pql::value
//

// construct
pql::value::value() : _value(pql::details::make_unique<pql::details::_Null>()), _type(value_type::Null)
{
}

pql::value::value(int value) : _value(pql::details::make_unique<pql::details::_Number>(value)), _type(value_type::Number)
{
}

pql::value::value(unsigned int value) : _value(pql::details::make_unique<pql::details::_Number>(value)), _type(value_type::Number)
{
}

pql::value::value(__int64 value) : _value(pql::details::make_unique<pql::details::_Number>(value)), _type(value_type::Number)
{
}

pql::value::value(unsigned __int64 value) : _value(pql::details::make_unique<pql::details::_Number>(value)), _type(value_type::Number)
{
}

pql::value::value(double value) : _value(pql::details::make_unique<pql::details::_Number>(value)), _type(value_type::Number)
{
}

pql::value::value(bool value) : _value(pql::details::make_unique<pql::details::_Boolean>(value)), _type(value_type::Boolean)
{
}

pql::value::value(std::wstring value) : _value(pql::details::make_unique<pql::details::_String>(std::move(value))), _type(value_type::String)
{
}

pql::value::value(std::wstring value, bool has_escape_chars) : _value(pql::details::make_unique<pql::details::_String>(std::move(value), has_escape_chars)), _type(value_type::String)
{
}

pql::value::value(const wchar_t* value) : _value(pql::details::make_unique<pql::details::_String>(value)), _type(value_type::String)
{
}

pql::value::value(const wchar_t* value, bool has_escape_chars) : _value(pql::details::make_unique<pql::details::_String>(value, has_escape_chars)), _type(value_type::String)
{
}

pql::value::value(const pql::value & other) : _value(other._value->_copy_value()), _type(other._type)
{
}

pql::value::value(pql::value && other) throw() : _value(std::move(other._value)), _type(other._type)
{
}

pql::value& pql::value::operator=(const pql::value &other)
{
    if (this != &other) {
        _value = std::unique_ptr<details::_Value>(other._value->_copy_value());
        _type = other._type;
    }
    return *this;
}

pql::value& pql::value::operator=(pql::value &&other) throw()
{
    if (this != &other) {
        _value.swap(other._value);
        _type = other._type;
    }
    return *this;
}


// Static factories
pql::value __cdecl pql::value::null()
{
    return pql::value();
}

pql::value __cdecl pql::value::number(double v)
{
    return pql::value(v);
}

pql::value __cdecl pql::value::number(int v)
{
    return pql::value(v);
}

pql::value __cdecl pql::value::boolean(bool v)
{
    return pql::value(v);
}

pql::value __cdecl pql::value::string(std::wstring v)
{
    return pql::value(v);
}

pql::value __cdecl pql::value::string(std::wstring v, bool has_escape_chars)
{
    return pql::value(v, has_escape_chars);
}

std::wstring pql::value::serialize() const
{
    return _value->serialize();
}

double pql::value::as_double() const
{
    return _value->as_double();
}

int pql::value::as_integer() const
{
    return _value->as_integer();
}

pql::number pql::value::as_number() const
{
    return _value->as_number();
}

bool pql::value::as_bool() const
{
    return _value->as_bool();
}

std::wstring pql::value::as_string() const
{
    return _value->as_string();
}

pql::datetime pql::value::as_datetime() const
{
    return _value->as_datetime();
}


//
//  class pql::number
//
bool pql::number::is_int32() const
{
#pragma push_macro ("max")
#undef max
#pragma push_macro ("min")
#undef min
    switch (m_type)
    {
    case signed_type: return m_intval >= std::numeric_limits<int32_t>::min() && m_intval <= std::numeric_limits<int32_t>::max();
    case unsigned_type: return m_uintval <= std::numeric_limits<int32_t>::max();
    case double_type:
    default:
        return false;
    }
#pragma pop_macro ("min")
#pragma pop_macro ("max")
}

bool pql::number::is_uint32() const
{
#pragma push_macro ("max")
#undef max
    switch (m_type)
    {
    case signed_type: return m_intval >= 0 && m_intval <= std::numeric_limits<uint32_t>::max();
    case unsigned_type: return m_uintval <= std::numeric_limits<uint32_t>::max();
    case double_type:
    default:
        return false;
    }
#pragma pop_macro ("max")
}

bool pql::number::is_int64() const
{
#pragma push_macro ("max")
#undef max
    switch (m_type)
    {
    case signed_type: return true;
    case unsigned_type: return m_uintval <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max());
    case double_type:
    default:
        return false;
    }
#pragma pop_macro ("max")
}

bool pql::details::_String::has_escape_chars(const pql::details::_String &str)
{
    static const auto escapes = L"\"\\\b\f\r\n\t";
    return (str.m_string.find_first_of(escapes) != std::wstring::npos);
}

std::wstring pql::details::_Number::serialize() const
{
    std::wstring stream;

    if (m_number.m_type != number::type::double_type) {

        // #digits + 1 to avoid loss + 1 for the sign + 1 for null terminator.
        const size_t tempSize = std::numeric_limits<uint64_t>::digits10 + 3;
        wchar_t tempBuffer[tempSize];

        if (m_number.m_type == number::type::signed_type)
            _i64tow_s(m_number.m_intval, tempBuffer, tempSize, 10);
        else
            _ui64tow_s(m_number.m_uintval, tempBuffer, tempSize, 10);

        stream.append(tempBuffer, wcsnlen_s(tempBuffer, tempSize));
    }
    else {

        // #digits + 2 to avoid loss + 1 for the sign + 1 for decimal point + 5 for exponent (e+xxx) + 1 for null terminator
        const size_t tempSize = std::numeric_limits<double>::digits10 + 10;
        wchar_t tempBuffer[tempSize];
        const int numChars = swprintf_s(
            tempBuffer,
            tempSize,
            L"%.*g",
            std::numeric_limits<double>::digits10 + 2,
            m_number.m_value);
        stream.append(tempBuffer, numChars);
    }

    return stream;
}

void pql::details::_Number::deserialize(const std::wstring& s)
{
}



//
//  class pql::datetime
//
const pql::datetime::timezone pql::datetime::_current_tz(std::wstring(L""));


pql::datetime::datetime() : m_value(0)
{
}

pql::datetime::datetime(unsigned __int64 dt, bool local) : m_value(dt)
{
    if (local) {
        m_tz = pql::datetime::_current_tz;
    }
}

pql::datetime::datetime(const FILETIME* dt, bool local) : m_value(((const ULARGE_INTEGER*)dt)->QuadPart)
{
    if (local) {
        m_tz = pql::datetime::_current_tz;
    }
}

pql::datetime::datetime(const SYSTEMTIME* st, bool local) : m_value(0)
{
    FILETIME ft = { 0, 0 };
    SystemTimeToFileTime(st, &ft);
    m_value = ((const ULARGE_INTEGER*)(&ft))->QuadPart;
}

pql::datetime::datetime(const std::wstring& s)
{
    try {
        from_string(s);
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        clear();
    }
}

void pql::datetime::from_simple_iso8601(const std::wstring& s, SYSTEMTIME* pst)
{
    const wchar_t * fmt_iso8601 = L"%04hd-%02hd-%02hd%c%2hd:%2hd:%2hd.%03hd";
    wchar_t sep = L'\0';
    memset(pst, 0, sizeof(SYSTEMTIME));
    int n = swscanf_s(s.c_str(), fmt_iso8601,
                      &pst->wYear, &pst->wMonth, &pst->wDay,
                      &sep,
                      &pst->wHour, &pst->wMinute, &pst->wSecond,
                      &pst->wMilliseconds);
    assert(sep == L' ' || sep == L'T');
}

void pql::datetime::from_string(const std::wstring& s)
{
    static const std::wregex pt_iso8601(L"^\\d{4}-\\d{1,2}-\\d{1,2}[ T]\\d{1,2}:\\d{1,2}:\\d{1,2}(\\.\\d{1,3})?(([+-]\\d{1,2}:\\d{1,2})|[Z])?$", std::regex_constants::icase);
    static const std::wregex pt_rfc1123(L"^(Mon|Tue|Wed|Thu|Fri|Sat|Sun){1},[ ]\\d{1,2}[ ](Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)[ ]\\d{4}[ ]\\d{1,2}:\\d{1,2}:\\d{1,2}[ ](UT|GMT|EST|EDT|CST|CDT|MST|MDT|PST|PDT|AM|PM)$", std::regex_constants::icase);

    SYSTEMTIME   st;
    memset(&st, 0, sizeof(st));

    if (std::regex_match(s, pt_iso8601)) {

        std::wstring stime;
        std::wstring::size_type pos = s.find_first_of(L"+-");

        if (s.c_str()[s.length() - 1] == 'Z' || s.c_str()[s.length() - 1] == 'Z') {
            assert(pos == std::wstring::npos);
            if (pos == std::wstring::npos) {
                throw std::invalid_argument("invalid iso8601 time: zone and 'Z' exists at the same time");
            }
            // remove last 'Z'
            stime = s.substr(0, s.length() - 1);
            from_simple_iso8601(stime, &st);
            m_tz.clear();
        }
        else {
            std::wstring::size_type pos = s.find_first_of(L"+-");
            if (pos != std::wstring::npos) {
                std::wstring tz;
                tz = s.substr(pos);
                stime = s.substr(0, pos);
                from_simple_iso8601(stime, &st);
                m_tz = timezone(tz);
            }
            else {
                // no time zone? use current zone
                from_simple_iso8601(s, &st);
                m_tz = timezone(std::wstring());
            }
        }
    }
    else if (std::regex_match(s, pt_rfc1123)) {
        const wchar_t * fmt_rfc1123 = L"%3c, %2hd %3c %4hd %2hd:%2hd:%2hd %3c";
        std::wstring month(3, L'\0');
        std::wstring weekday(3, L'\0');
        std::wstring zone(3, L'\0');
        int n = swscanf_s(s.c_str(), fmt_rfc1123,
                          weekday.data(), weekday.size(),
                          &st.wDay,
                          month.data(), month.size(),
                          &st.wYear,
                          &st.wHour,
                          &st.wMinute,
                          &st.wSecond,
                          zone.data(), zone.size());
        if (8 == n) {
            // month
            static const std::wstring monthnames[12] = { L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun", L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec" };
            auto mon_loc = std::find_if(monthnames, monthnames + 12, [&month](const std::wstring& m) { return m == month; });
            if (mon_loc == (monthnames + 12)) {
                throw std::invalid_argument("incorrect rfc1123 datetime month name");
            }
            st.wMonth = (short)((mon_loc - monthnames) + 1);
            // day of week
            static const std::wstring weekdaynames[7] = { L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat" };
            auto wd_loc = std::find_if(weekdaynames, weekdaynames + 7, [&weekday](const std::wstring& wd) { return wd == weekday; });
            if (wd_loc == (weekdaynames + 7)) {
                throw std::invalid_argument("incorrect rfc1123 datetime weekday name");
            }
            st.wDayOfWeek =(short)(mon_loc - weekdaynames);

            if (0 == _wcsicmp(zone.c_str(), L"AM")) {
                // local time
                if (12 == st.wHour) {
                    st.wHour = st.wHour%12;
                }
            }
            else if (0 == _wcsicmp(zone.c_str(), L"PM")) {
                // local time
                if (12 > st.wHour) {
                    st.wHour += 12;
                }
            }
            else if (0 == _wcsicmp(zone.c_str(), L"UT") || 0 == _wcsicmp(zone.c_str(), L"GMT")) {
                // UTC time
                m_tz.clear();
            }
            else if (0 == _wcsicmp(zone.c_str(), L"EST")) {
                m_tz = timezone(L"-05:00");
            }
            else if (0 == _wcsicmp(zone.c_str(), L"EDT")) {
                m_tz = timezone(L"-04:00");
            }
            else if (0 == _wcsicmp(zone.c_str(), L"CST")) {
                m_tz = timezone(L"-06:00");
            }
            else if (0 == _wcsicmp(zone.c_str(), L"CDT")) {
                m_tz = timezone(L"-05:00");
            }
            else if (0 == _wcsicmp(zone.c_str(), L"MST")) {
                m_tz = timezone(L"-07:00");
            }
            else if (0 == _wcsicmp(zone.c_str(), L"MDT")) {
                m_tz = timezone(L"-06:00");
            }
            else if (0 == _wcsicmp(zone.c_str(), L"PST")) {
                m_tz = timezone(L"-08:00");
            }
            else if (0 == _wcsicmp(zone.c_str(), L"PDT")) {
                m_tz = timezone(L"-07:00");
            }
            else {
                // unknown time zone
                m_tz = timezone(L"");   // Use local zone
            }
        }
    }
    else {
        throw std::invalid_argument("incorrect datetime format");
    }

    FILETIME ft = { 0, 0 };
    SystemTimeToFileTime(&st, &ft);
    m_value = ft.dwHighDateTime;
    m_value <<= 32;
    m_value |= ft.dwLowDateTime;
}

__int64 pql::datetime::to_utc_time() const noexcept
{
    if (m_tz.is_utc()) {
        return m_value;
    }
    else {
        static const __int64 ratio_to_second = 10000000;
        static const __int64 ratio_to_minute = 600000000;
        __int64 t = (__int64)m_value;
        t += ratio_to_minute * m_tz.bias();
        return t;
    }
    __assume(0);
}

std::wstring pql::datetime::to_rfc1123() const noexcept
{
    static const std::wstring monthnames[12] = { L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun", L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec" };
    static const std::wstring weekdaynames[7] = { L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat" };
    FILETIME ft = { 0, 0 };
    SYSTEMTIME st = { 0 };
    std::vector<wchar_t> buf;

    buf.resize(256, 0);
    memset(&st, 0, sizeof(st));
    // To UTC time
    __int64 t = to_utc_time();
    ft.dwHighDateTime = (unsigned long)(t >> 32);
    ft.dwLowDateTime = (unsigned long)t;
    FileTimeToSystemTime(&ft, &st);

    swprintf_s(&buf[0], 256, L"%s, %02d %s %04d %02d:%02d:%02d GMT", weekdaynames[st.wDayOfWeek].c_str(), st.wDay, monthnames[st.wMonth-1].c_str(), st.wYear, st.wHour, st.wMinute, st.wSecond);
    return std::wstring(&buf[0]);
}

std::wstring pql::datetime::to_iso8601() const noexcept
{
    std::wstring stime;
    FILETIME ft = { 0, 0 };
    SYSTEMTIME st = { 0 };
    std::vector<wchar_t> buf;
    buf.resize(256, 0);
    memset(&st, 0, sizeof(st));
    ft.dwHighDateTime = (unsigned long)(m_value >> 32);
    ft.dwLowDateTime = (unsigned long)m_value;
    FileTimeToSystemTime(&ft, &st);
    swprintf_s(&buf[0], 256, L"%04d-%-2d-%02dT%02d:%02d:%02d.%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    stime = &buf[0];
    if (m_tz.is_utc()) {
        stime += L"Z";
    }

    return stime;
}

std::wstring pql::datetime::to_iso8601_withzone() const noexcept
{
    std::wstring stime = to_iso8601();
    if (stime.c_str()[stime.length() - 1] == L'Z') {
        // utc
        stime = stime.substr(0, stime.length() - 1);
        stime += L"+00:00";
    }
    else {
        stime += m_tz.bias_str();
    }

    return stime;
}

bool pql::datetime::is_valid() const noexcept
{
    return (0 != m_value);
}

void pql::datetime::clear()
{
    m_value = 0;
    m_tz.clear();
}

datetime& pql::datetime::operator = (const datetime& other)
{
    if (this != &other) {
        m_value = other.value();
        m_tz = other.zone();
    }
    return *this;
}

pql::datetime::timezone::timezone(const std::wstring& zone) : _bias(0), _dlt(false)
{
    from_string(zone);
}

pql::datetime::timezone::timezone(long bias) : _bias(bias), _dlt(false)
{
    serialize();
}

pql::datetime::timezone::timezone(const timezone& tz)
{
    _dlt = tz.daylight();
    _bias = tz.bias();
    _bias_str = tz.bias_str();
    _zone_name = tz.zone_name();
}

void pql::datetime::timezone::current_zone() noexcept
{
    TIME_ZONE_INFORMATION ti = { 0 };
    memset(&ti, 0, sizeof(ti));
    DWORD status = GetTimeZoneInformation(&ti);
    if (TIME_ZONE_ID_DAYLIGHT == status) {
        _dlt = true;
        _bias = ti.Bias + ti.DaylightBias;
        _zone_name = ti.DaylightName;
    }
    else if (TIME_ZONE_ID_STANDARD == status) {
        _dlt = false;
        _bias = ti.Bias + ti.StandardBias;
        _zone_name = ti.StandardName;
    }
    else {
        _dlt = false;
        _bias = ti.Bias;
        _zone_name = ti.StandardName;
    }
}

void pql::datetime::timezone::from_string(const std::wstring& s)
{
    // [+|-]HH:MM
    if (s.empty()) {
        current_zone();
    }

    const wchar_t* p = s.c_str();
    if (s.length() != 6 || p[3] != L':' || (p[0] != L'+' && p[0] != L'-')) {
        throw std::invalid_argument("invalid time zone string");
    }

    int h = 0, m = 0;

    swscanf_s(p+1/*ignore the sign*/, L"%02d:%02d", &h, &m);
    _bias = (h * 60 + m) * ((L'+' == p[0]) ? 1 : (-1));
}

void pql::datetime::timezone::serialize() noexcept
{
    wchar_t s[16] = { 0 };
    long lbias = (_bias >= 0) ? _bias : (0 - _bias);
    swprintf_s(s, 16, L"%c%02d:%02d", (_bias >= 0) ? L'+' : L'-', (_bias / 60), (_bias % 60));
    _bias_str = s;
}

void pql::datetime::timezone::clear()
{
    _bias = 0;
    _bias_str = L"+00:00";
    _dlt = false;
    _zone_name.clear();
}

pql::datetime::timezone& pql::datetime::timezone::operator = (const pql::datetime::timezone& other)
{
    if (this != &other) {
        _bias = other.bias();
        _bias_str = other.bias_str();
        _dlt = other.daylight();
        _zone_name = other.zone_name();
    }
    return *this;
}