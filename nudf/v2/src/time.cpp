

#include <Windows.h>
#include <assert.h>

#include <regex>

#include <boost/algorithm/string.hpp>

#include <nudf\eh.hpp>
#include <nudf\time.hpp>
#include <nudf\string.hpp>

using namespace NX;


namespace time_detail {

    static const LPCWSTR utf16_month_names[12] = { L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun", L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec" };
    static const LPCWSTR utf16_weekday_names[7] = { L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat" };
    static const LPCSTR utf8_month_names[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    static const LPCSTR utf8_weekday_names[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

    static unsigned short name_to_month(const wchar_t* name)
    {
        for (int i = 0; i < 12; i++) {
            if (0 == _wcsicmp(name, utf16_month_names[i])) {
                return (i + 1);
            }
        }
        return 0;
    }
    static unsigned short name_to_month(const char* name)
    {
        for (int i = 0; i < 12; i++) {
            if (0 == _stricmp(name, utf8_month_names[i])) {
                return (i + 1);
            }
        }
        return 0;
    }
    static unsigned short name_to_weekday(const wchar_t* name)
    {
        for (int i = 0; i < 7; i++) {
            if (0 == _wcsicmp(name, utf16_weekday_names[i])) {
                return i;
            }
        }
        return 7;
    }
    static unsigned short name_to_weekday(const char* name)
    {
        for (int i = 0; i < 7; i++) {
            if (0 == _stricmp(name, utf8_weekday_names[i])) {
                return i;
            }
        }
        return 7;
    }
}


__int64 time::filetime_to_time(const FILETIME* t)
{
    __int64 n = t->dwHighDateTime;
    n <<= 32;
    n += t->dwLowDateTime;
    return n;
}

__int64 time::systemtime_to_time(const SYSTEMTIME* t)
{
    FILETIME ft = { 0, 0 };
    SystemTimeToFileTime(t, &ft);
    return filetime_to_time(&ft);
}

bool time::is_iso_8601_time_string(const std::wstring& s)
{
    // ISO 8601
    //  2016-4-14T10:22:45
    //  2016-4-14 10:22:45
    //  2016-4-14T10:22:45Z
    //  2016-4-14 10:22:45Z
    //  2016-4-14 10:22:45.312Z
    // Pattern
    //  ^\d{4}-\d{1,2}-\d{1,2}[ tT]\d{1,2}:\d{1,2}:\d{1,2}(\.\d{1,3})?[zZ]?$
    static const std::wregex pattern(L"^\\d{4}-\\d{1,2}-\\d{1,2}[ T]\\d{1,2}:\\d{1,2}:\\d{1,2}(\\.\\d{1,3})?[Z]?$", std::regex_constants::icase);
    bool result = false;
    try {
        result = std::regex_match(s, pattern);
    }
    catch (...) {
        result = false;
    }
    return result;    
}

bool time::is_iso_8601_zone_time_string(const std::wstring& s)
{
    // ISO 8601 with time zone
    //  2016-4-14T10:22:45-07:00
    //  2016-4-14 10:22:45.312-07:00
    // Pattern
    //  ^\d{4}-\d{1,2}-\d{1,2}[ tT]\d{1,2}:\d{1,2}:\d{1,2}(\.\d{1,3})?[+-]\d{1,2}:\d{1,2}$
    static const std::wregex pattern(L"^\\d{4}-\\d{1,2}-\\d{1,2}[ T]\\d{1,2}:\\d{1,2}:\\d{1,2}(\\.\\d{1,3})?[+-]\\d{1,2}:\\d{1,2}$", std::regex_constants::icase);
    bool result = false;
    try {
        result = std::regex_match(s, pattern);
    }
    catch (...) {
        result = false;
    }
    return result;
}

bool time::is_rfc_1123_0_time_string(const std::wstring& s)
{
    // RFC 1123
    //  "Fri, 05 Feb 2016 19:13:26 GMT"
    // Pattern
    //  ^((Sun|Mon|Tue|Wed|Thu[r]?|Fri|Sat), )?\d{1,2} (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) \d{4} \d{1,2}:\d{1,2}:\d{1,2}( (AM|PM|GMT))?$
    static const std::wregex pattern(L"^((Sun|Mon|Tue|Wed|Thu[r]?|Fri|Sat), )?\\d{1,2} (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) \\d{4} \\d{1,2}:\\d{1,2}:\\d{1,2}( (AM|PM|GMT))?$", std::regex_constants::icase);
    bool result = false;
    try {
        result = std::regex_match(s, pattern);
    }
    catch (...) {
        result = false;
    }
    return result;
}

bool time::is_rfc_1123_1_time_string(const std::wstring& s)
{
    // RFC 1123
    //  "2/5/2016 11:13:26 AM"
    //  "2/5/2016 11:13:26 GMT"
    // Pattern
    //  ^\d{1,2}\/\d{1,2}\/\d{4} \d{1,2}:\d{1,2}:\d{1,2}( (AM|PM|GMT))?$
    static const std::wregex pattern(L"^\\d{1,2}\\/\\d{1,2}\\/\\d{4} \\d{1,2}:\\d{1,2}:\\d{1,2}( (AM|PM|GMT))?$", std::regex_constants::icase);
    bool result = false;
    try {
        result = std::regex_match(s, pattern);
    }
    catch (...) {
        result = false;
    }
    return result;
}

bool time::is_special_1_time_string(const std::wstring& s)
{
    // Special NextLabs Time
    //  "Feb 2, 2016 7:13:26 PM"
    //  "Feb 2, 2016 19:13:26"
    // Pattern
    //  ^(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) \d{1,2}, \d{4} \d{1,2}:\d{1,2}:\d{1,2}( [AP]M)?$
    static const std::wregex pattern(L"^(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) \\d{1,2}, \\d{4} \\d{1,2}:\\d{1,2}:\\d{1,2}( [AP]M)?$", std::regex_constants::icase);
    bool result = false;
    try {
        result = std::regex_match(s, pattern);
    }
    catch (...) {
        result = false;
    }
    return result;
}

bool time::is_time_only_string(const std::wstring& s)
{
    // Special NextLabs Time
    //  7:13:26 PM
    //  19:13:26
    // Pattern
    //  ^\d{1,2}:\d{1,2}:\d{1,2}( [AP]M)?$
    static const std::wregex pattern(L"^\\d{1,2}:\\d{1,2}:\\d{1,2}( [AP]M)?$", std::regex_constants::icase);
    bool result = false;
    try {
        result = std::regex_match(s, pattern);
    }
    catch (...) {
        result = false;
    }
    return result;
}



//
//  class time::datetime
//

time::span::span() : _span(0)
{
}

time::span::span(const time::span& s) : _span((__int64)s)
{
}

time::span::span(const std::wstring& s) : _span(0)
{
    if (time::is_time_only_string(s)) {
        int hours = 0, minutes = 0, seconds = 0;
        int count = swscanf_s(s.c_str(), L"%d:%d:%d", &hours, &minutes, &seconds);
        if (count == 3) {
            seconds += (hours * 3600 + minutes * 60);
            _span = ((__int64)seconds) * 10000000LL;
        }
    }
}

time::span::span(__int64 s) : _span(s)
{
}

time::span::~span()
{
}

time::span time::span::seconds_span(__int64 seconds)
{
    return time::span(seconds * 10000000LL);
}

time::span time::span::minutes_span(__int64 minutes)
{
    return time::span::seconds_span(minutes * 60);
}

time::span time::span::hours_span(__int64 hours)
{
    return time::span::minutes_span(hours * 60);
}

time::span time::span::days_span(__int64 days)
{
    return time::span::hours_span(days * 24);
}

__int64 time::span::to_seconds() const
{
    return (_span / 10000000LL);
}


//
//  class time::timezone
//

time::timezone::timezone() : _bias(0), _name(L"UTC")
{
}

time::timezone::timezone(long zone_bias, const std::wstring& zone_name) : _bias(zone_bias), _name(zone_name)
{
}

time::timezone::timezone(const std::wstring& zone_bias, const std::wstring& zone_name) : _bias(0), _name(L"UTC")
{
    try {
        _bias = parse(zone_bias);
        _name = zone_name;
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
}

time::timezone::~timezone()
{
}

std::wstring time::timezone::serialize_bias() const
{
    std::wstring s;
    const long minutes = abs(_bias) % 60;
    const long hours = abs(_bias) / 60;
    swprintf_s(NX::string_buffer<wchar_t>(s, 16), 16, L"%c%02d:%02d", (_bias > 0) ? L'-' : L'+', hours, minutes);
    return std::move(s);
}

time::timezone time::timezone::current_timezone()
{
    TIME_ZONE_INFORMATION zone_info = { 0 };
    DWORD status = GetTimeZoneInformation(&zone_info);
    switch (status)
    {
    case TIME_ZONE_ID_DAYLIGHT:
        return time::timezone(zone_info.Bias + zone_info.DaylightBias, zone_info.DaylightName);
    case TIME_ZONE_ID_STANDARD:
        return time::timezone(zone_info.Bias + zone_info.StandardBias, zone_info.StandardName);
    case TIME_ZONE_ID_UNKNOWN:
    default:
        return time::timezone(zone_info.Bias, zone_info.StandardName);
    }
}

long time::timezone::parse(const std::wstring& zone_bias)
{
    long hours = 0;
    long minutes = 0;

    if (zone_bias.empty()) {
        throw NX::exception("empty timezone string", ERROR_INVALID_PARAMETER);
    }

    if (L'+' == zone_bias[0]) {
        if (2 != swscanf_s(zone_bias.c_str() + 1, L"%d:%d", &hours, &minutes)) {
            throw NX::exception("invalid timezone format", ERROR_INVALID_PARAMETER);
        }
        minutes += (60 * hours);
        minutes = 0 - minutes;
    }
    else if (L'-' == zone_bias[0]) {
        if (2 != swscanf_s(zone_bias.c_str() + 1, L"%d:%d", &hours, &minutes)) {
            throw NX::exception("invalid timezone format", ERROR_INVALID_PARAMETER);
        }
        minutes += (60 * hours);
    }
    else {
        throw NX::exception("invalid timezone format: missing sign", ERROR_INVALID_PARAMETER);
    }
    return minutes;
}

//
//  class time::datetime
//
time::datetime::datetime() : _t(0)
{
}

time::datetime::datetime(const datetime& other) : _t((__int64)other)
{
}

time::datetime::datetime(__int64 t) : _t(t)
{
}

time::datetime::datetime(const FILETIME* t) : _t(filetime_to_time(t))
{
}

time::datetime::datetime(const SYSTEMTIME* t) : _t(systemtime_to_time(t))
{
}

time::datetime::datetime(const std::string& s) : _t(0)
{
    std::wstring ws(s.begin(), s.end());
    parse(ws);
}

time::datetime::datetime(const std::wstring& s) : _t(0)
{
    parse(s);
}

time::datetime::~datetime()
{
}

time::datetime& time::datetime::operator = (const time::datetime& other)
{
    if (this != &other) {
        _t = (__int64)other;
    }
    return *this;
}

time::datetime& time::datetime::operator += (const time::span& other)
{
    _t += (__int64)other;
    return *this;
}

time::datetime& time::datetime::operator -= (const time::span& other)
{
    _t -= (__int64)other;
    return *this;
}

time::datetime& time::datetime::add(const time::span& other)
{
    _t += (__int64)other;
    return *this;
}

time::datetime& time::datetime::sub(const time::span& other)
{
    _t -= (__int64)other;
    return *this;
}

time::span time::datetime::operator - (const time::datetime& other) const
{
    return time::span(_t - (__int64)other);
}

bool time::datetime::operator == (const time::datetime& other) const
{
    return (_t == ((__int64)other));
}

bool time::datetime::operator > (const time::datetime& other) const
{
    return (_t > ((__int64)other));
}

bool time::datetime::operator >= (const time::datetime& other) const
{
    return (_t >= ((__int64)other));
}

bool time::datetime::operator < (const time::datetime& other) const
{
    return (_t < ((__int64)other));
}

bool time::datetime::operator <= (const time::datetime& other) const
{
    return (_t <= ((__int64)other));
}

static void ull_to_ft(unsigned __int64 ull, FILETIME* ft)
{
    ft->dwHighDateTime = (unsigned long)(ull >> 32);
    ft->dwLowDateTime = (unsigned long)(ull & 0x00000000FFFFFFFFULL);
}

void time::datetime::from_filetime(const FILETIME* ft, bool local_time)
{
    if (local_time) {
        FILETIME uft = { 0, 0 };
        LocalFileTimeToFileTime(ft, &uft);
        _t = time::filetime_to_time(&uft);
    }
    else {
        _t = time::filetime_to_time(ft);
    }
}

void time::datetime::from_systemtime(const SYSTEMTIME* st, bool local_time)
{
    FILETIME ft = { 0, 0 };
    SystemTimeToFileTime(st, &ft);
    from_filetime(&ft, local_time);
}

void time::datetime::to_filetime(FILETIME* ft, bool local_time) const
{
    if (local_time) {
        FILETIME uft = { 0, 0 };
        ull_to_ft((unsigned __int64)_t, &uft);
        FileTimeToLocalFileTime(&uft, ft);
    }
    else {
        ull_to_ft((unsigned __int64)_t, ft);
    }
}

void time::datetime::to_systemtime(SYSTEMTIME* st, bool local_time) const
{
    FILETIME ft = { 0, 0 };
    to_filetime(&ft, local_time);
    memset(st, 0, sizeof(SYSTEMTIME));
    FileTimeToSystemTime(&ft, st);
}

void time::datetime::parse(const std::wstring& s)
{
    // ISO 8601
    //    [With Milliseconds]
    //      "2015-16-02T13:36:21.243"       - Local time using current time zone
    //      "2015-16-02T13:36:21.243Z"      - UTC time
    //      "2015-16-02T13:36:21.243-08:00" - Local time using specified time zone
    //    [Without Milliseconds]
    //      "2015-16-02T13:36:21"           - Local time using current time zone
    //      "2015-16-02T13:36:21Z"          - UTC time
    //      "2015-16-02T13:36:21-08:00"     - Local time using specified time zone

    SYSTEMTIME st = { 0 };
    memset(&st, 0, sizeof(SYSTEMTIME));

    int year = 0, month = 0, day = 0, hours = 0, minutes = 0, seconds = 0, milliseconds = 0;
    bool local_time = true;

    if (time::is_iso_8601_time_string(s)) {
        //  "2015-16-02T13:36:21.243"       - Local time using current time zone
        //  "2015-16-02T13:36:21.243Z"      - UTC time
        //  "2015-16-02T13:36:21"           - Local time using current time zone
        //  "2015-16-02T13:36:21Z"          - UTC time
        wchar_t separator[2] = { 0, 0 };
        std::wstring time_str(s);
        if (boost::algorithm::iends_with(time_str, L"Z")) {
            time_str = time_str.substr(0, time_str.length() - 1);
            local_time = false;
        }

        auto pos = time_str.find_last_of(L'.');
        if (pos != std::wstring::npos) {
            std::wstring milliseconds_str = time_str.substr(pos + 1);
            time_str = time_str.substr(0, pos);
            milliseconds = std::wcstol(milliseconds_str.c_str(), nullptr, 10);
        }

        if (7 != swscanf_s(s.c_str(), L"%d-%d-%d%c%d:%d:%d", &year, &month, &day, separator, 1, &hours, &minutes, &seconds)) {
            throw NX::exception("mismatch ISO-8601 format", ERROR_INVALID_PARAMETER);
        }

        st.wYear = year;
        st.wMonth = month;
        st.wDay = day;
        st.wHour = hours;
        st.wMinute = minutes;
        st.wSecond = seconds;
        st.wMilliseconds = milliseconds;
        from_systemtime(&st, local_time);
    }
    else if (time::is_iso_8601_zone_time_string(s)) {
        //  "2015-16-02T13:36:21.243-08:00" - Local time using specified time zone
        //  "2015-16-02T13:36:21-08:00"     - Local time using specified time zone
        wchar_t separator[2] = { 0, 0 };
        std::wstring time_str(s);
        std::wstring zone_str(s);

        // get time zone
        auto pos = time_str.find_first_of(L'+');
        if (pos != std::wstring::npos) {
            zone_str = time_str.substr(pos);
            time_str = time_str.substr(0, pos);
        }
        else {
            pos = time_str.find_last_of(L'-');
            if (pos != std::wstring::npos) {
                throw NX::exception("mismatch ISO-8601 format", ERROR_INVALID_PARAMETER);
            }
            zone_str = time_str.substr(pos);
            time_str = time_str.substr(0, pos);
        }

        time::timezone zone(zone_str, L"");

        // get milliseconds if it exists
        pos = time_str.find_last_of(L'.');
        if (pos != std::wstring::npos) {
            std::wstring milliseconds_str = time_str.substr(pos + 1);
            time_str = time_str.substr(0, pos);
            milliseconds = std::wcstol(milliseconds_str.c_str(), nullptr, 10);
        }

        if (7 != swscanf_s(s.c_str(), L"%d-%d-%d%c%d:%d:%d", &year, &month, &day, separator, 1, &hours, &minutes, &seconds)) {
            throw NX::exception("mismatch ISO-8601 format", ERROR_INVALID_PARAMETER);
        }

        st.wYear = year;
        st.wMonth = month;
        st.wDay = day;
        st.wHour = hours;
        st.wMinute = minutes;
        st.wSecond = seconds;
        st.wMilliseconds = milliseconds;
        from_systemtime(&st, false);    // treat as UTC time, and adjust time zone manually
        time::span zone_adjust = time::span::minutes_span(zone.bias());
        _t += (__int64)zone_adjust;
    }
    else {
        throw NX::exception("mismatch ISO-8601 format", ERROR_INVALID_PARAMETER);
    }
}

std::wstring time::datetime::serialize(bool local_time, bool with_milliseconds)
{
    std::wstring s;
    SYSTEMTIME st = { 0 };
    to_systemtime(&st, local_time);
    if (local_time) {
        const time::timezone& zone = time::timezone::current_timezone();
        if (with_milliseconds) {
            swprintf_s(NX::string_buffer<wchar_t>(s, 64), 64, L"%04d-%02d-%02dT%02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
        }
        else {
            swprintf_s(NX::string_buffer<wchar_t>(s, 64), 64, L"%04d-%02d-%02dT%02d:%02d:%02d.%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
        }
        s += zone.serialize_bias();
    }
    else {
        if (with_milliseconds) {
            swprintf_s(NX::string_buffer<wchar_t>(s, 64), 64, L"%04d-%02d-%02dT%02d:%02d:%02dZ", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
        }
        else {
            swprintf_s(NX::string_buffer<wchar_t>(s, 64), 64, L"%04d-%02d-%02dT%02d:%02d:%02d.%03dZ", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
        }
    }
    return std::move(s);
}

void time::datetime_rfc1123::parse(const std::wstring& s)
{
    // RFC 1123
    SYSTEMTIME st = { 0 };
    wchar_t week_day_name[4] = { 0, 0, 0, 0 };
    wchar_t month_name[4] = { 0, 0, 0, 0 };
    wchar_t flag_name[16] = { 0 };
    int year = 0, month = 0, day = 0, hours = 0, minutes = 0, seconds = 0, milliseconds = 0;
    bool local_time = true;

    memset(&st, 0, sizeof(st));

    if (is_rfc_1123_0_time_string(s)) {
        //  "Fri, 05 Feb 2016 19:13:26 GMT"
        int count = swscanf_s(s.c_str(), L"%3c, %d %3c %d %d:%d:%d %s", week_day_name, 3, &day, &month_name, 3, &year, &hours, &minutes, &seconds, flag_name, 16);
        if (count < 7) {
            throw NX::exception("mismatch RFC-1123 format", ERROR_INVALID_PARAMETER);
        }
        if (time_detail::name_to_weekday(week_day_name) > 6) {
            throw NX::exception("mismatch RFC-1123 format", ERROR_INVALID_PARAMETER);
        }
        month = time_detail::name_to_month(month_name);
        if (0 == month) {
            throw NX::exception("mismatch RFC-1123 format", ERROR_INVALID_PARAMETER);
        }
        if (0 == _wcsicmp(flag_name, L"PM") && hours < 12) {
            hours += 12;
        }
        else if (0 == _wcsicmp(flag_name, L"GMT") || 0 == _wcsicmp(flag_name, L"UTC")) {
            local_time = false;
        }
        else {
            ; // ignore
        }
    }
    else if (is_rfc_1123_1_time_string(s)) {
        //  "2/5/2016 11:13:26 AM"
        int count = swscanf_s(s.c_str(), L"%d/%d/%d %d:%d:%d %s", &month, &day, &year, &hours, &minutes, &seconds, flag_name, 16);
        if (count < 6) {
            throw NX::exception("mismatch RFC-1123 format", ERROR_INVALID_PARAMETER);
        }
        if (0 == _wcsicmp(flag_name, L"PM") && hours < 12) {
            hours += 12;
        }
        else if (0 == _wcsicmp(flag_name, L"GMT") || 0 == _wcsicmp(flag_name, L"UTC")) {
            local_time = false;
        }
        else {
            ; // ignore
        }
    }
    else {
        throw NX::exception("mismatch RFC-1123 format", ERROR_INVALID_PARAMETER);
    }

    st.wYear = year;
    st.wMonth = month;
    st.wDay = day;
    st.wHour = hours;
    st.wMinute = minutes;
    st.wSecond = seconds;
    st.wMilliseconds = milliseconds;
    from_systemtime(&st, local_time);
}

std::wstring time::datetime_rfc1123::serialize(bool local_time, bool with_milliseconds)
{
    std::wstring s;
    SYSTEMTIME st = { 0 };

    UNREFERENCED_PARAMETER(with_milliseconds);

    to_systemtime(&st, local_time);

    if (local_time) {
        bool is_pm = false;
        if (st.wHour > 11) {
            is_pm = true;
            if (st.wHour > 12) {
                st.wHour = (st.wHour % 12);
            }
        }
        s = NX::string_formater(L"%s, %02d %s %04d %02d:%02d:%02d %s",
                                time_detail::utf8_weekday_names[st.wDayOfWeek],
                                st.wDay, time_detail::utf16_month_names[st.wMonth - 1], st.wYear,
                                st.wHour, st.wMinute, st.wSecond, (is_pm ? L"PM" : L"AM")
                                );
    }
    else {
        s = NX::string_formater(L"%s, %02d %s %04d %02d:%02d:%02d GMT",
                                time_detail::utf8_weekday_names[st.wDayOfWeek],
                                st.wDay, time_detail::utf16_month_names[st.wMonth - 1], st.wYear,
                                st.wHour, st.wMinute, st.wSecond
                                );
    }

    return std::move(s);
}

void time::datetime_special_1::parse(const std::wstring& s)
{
    // Special NextLabs Time
    //  "Feb 2, 2016 7:13:26 PM"

    wchar_t month_name[4] = { 0, 0, 0, 0 };
    wchar_t flag[3] = { 0, 0, 0 };
    int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;

    if (7 != swscanf_s(s.c_str(), L"%3c %d, %d %d:%d:%d %2c", month_name, 3, &day, &year, &hour, &minute, &second, flag, 2)) {
        throw NX::exception("mismatch Special-1 format", ERROR_INVALID_PARAMETER);
    }
    if (0 == _wcsicmp(flag, L"PM") && hour < 12) {
        hour += 12;
    }

    SYSTEMTIME st = { 0 };
    st.wYear = year;
    st.wMonth = time_detail::name_to_month(month_name);
    if (0 == st.wMonth) {
        throw NX::exception("mismatch Special-1 format", ERROR_INVALID_PARAMETER);
    }
    st.wDay = day;
    st.wHour = hour;
    st.wMinute = minute;
    st.wSecond = second;
    from_systemtime(&st, true);
}

std::wstring time::datetime_special_1::serialize(bool local_time, bool with_milliseconds)
{
    // Special NextLabs Time
    //  "Feb 2, 2016 7:13:26 PM"
    UNREFERENCED_PARAMETER(with_milliseconds);

    SYSTEMTIME st = { 0 };
    bool is_pm = false;

    to_systemtime(&st, local_time);
    if (st.wHour > 11) {
        is_pm = true;
        if (st.wHour > 12) {
            st.wHour = (st.wHour % 12);
        }
    }

    return NX::string_formater( L"%s %d, %d %02d:%02d:%02d %s",
                                time_detail::utf16_month_names[st.wMonth - 1],
                                st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond,
                                is_pm ? L"PM" : L"AM");
}