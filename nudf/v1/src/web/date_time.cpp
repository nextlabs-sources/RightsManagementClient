

#include <Windows.h>

#include <sstream>
#include <algorithm>

#include <nudf\web\conversions.hpp>
#include <nudf\web\error_category.hpp>
#include <nudf\web\date_time.hpp>


using namespace NX::utility;


static bool is_digit(wchar_t c)
{
    return (c>=0 && c<=9);
}



/// <summary>
/// Returns the current UTC date and time.
/// </summary>
datetime __cdecl datetime::utc_now()
{
    ULARGE_INTEGER largeInt;
    FILETIME fileTime;
    GetSystemTimeAsFileTime(&fileTime);

    largeInt.LowPart = fileTime.dwLowDateTime;
    largeInt.HighPart = fileTime.dwHighDateTime;

    return datetime(largeInt.QuadPart);
}

/// <summary>
/// Returns a string representation of the datetime. The string is formatted based on RFC 1123 or ISO 8601
/// </summary>
std::wstring datetime::to_string(date_format format) const
{
    int status;

    ULARGE_INTEGER largeInt;
    largeInt.QuadPart = m_interval;

    FILETIME ft;
    ft.dwHighDateTime = largeInt.HighPart;
    ft.dwLowDateTime = largeInt.LowPart;

    SYSTEMTIME systemTime;
    if (!FileTimeToSystemTime((const FILETIME *)&ft, &systemTime)) {
        throw NX::utility::create_system_error(GetLastError());
    }

    std::wostringstream outStream;

    if ( format == RFC_1123 ) {

#if _WIN32_WINNT < _WIN32_WINNT_VISTA
        WCHAR dateStr[18] = {0};
        status = GetDateFormat(LOCALE_INVARIANT, 0, &systemTime, L"ddd',' dd MMM yyyy", dateStr, sizeof(dateStr) / sizeof(TCHAR));
#else
        wchar_t dateStr[18] = {0};
        status = GetDateFormatEx(LOCALE_NAME_INVARIANT, 0, &systemTime, L"ddd',' dd MMM yyyy", dateStr, sizeof(dateStr) / sizeof(wchar_t), NULL);
#endif // _WIN32_WINNT < _WIN32_WINNT_VISTA
        if (status == 0) {
            throw NX::utility::create_system_error(GetLastError());
        }

#if _WIN32_WINNT < _WIN32_WINNT_VISTA
        WCHAR timeStr[10] = {0};
        status = GetTimeFormat(LOCALE_INVARIANT, TIME_NOTIMEMARKER | TIME_FORCE24HOURFORMAT, &systemTime, L"HH':'mm':'ss", timeStr, sizeof(timeStr) / sizeof(TCHAR));
#else
        wchar_t timeStr[10] = {0};
        status = GetTimeFormatEx(LOCALE_NAME_INVARIANT, TIME_NOTIMEMARKER | TIME_FORCE24HOURFORMAT, &systemTime, L"HH':'mm':'ss", timeStr, sizeof(timeStr) / sizeof(wchar_t));
#endif // _WIN32_WINNT < _WIN32_WINNT_VISTA
        if (status == 0) {
            throw NX::utility::create_system_error(GetLastError());
        }

        outStream << dateStr << L" " << timeStr << L" " << L"GMT";
    }
    else if ( format == ISO_8601 ) {

        const size_t buffSize = 64;
#if _WIN32_WINNT < _WIN32_WINNT_VISTA
        WCHAR dateStr[buffSize] = {0};
        status = GetDateFormat(LOCALE_INVARIANT, 0, &systemTime, L"yyyy-MM-dd", dateStr, buffSize);
#else
        wchar_t dateStr[buffSize] = {0};
        status = GetDateFormatEx(LOCALE_NAME_INVARIANT, 0, &systemTime, L"yyyy-MM-dd", dateStr, buffSize, NULL);
#endif // _WIN32_WINNT < _WIN32_WINNT_VISTA
        if (status == 0) {
            throw NX::utility::create_system_error(GetLastError());
        }

#if _WIN32_WINNT < _WIN32_WINNT_VISTA
        WCHAR timeStr[buffSize] = {0};
        status = GetTimeFormat(LOCALE_INVARIANT, TIME_NOTIMEMARKER | TIME_FORCE24HOURFORMAT, &systemTime, L"HH':'mm':'ss", timeStr, buffSize);
#else
        wchar_t timeStr[buffSize] = {0};
        status = GetTimeFormatEx(LOCALE_NAME_INVARIANT, TIME_NOTIMEMARKER | TIME_FORCE24HOURFORMAT, &systemTime, L"HH':'mm':'ss", timeStr, buffSize);
#endif // _WIN32_WINNT < _WIN32_WINNT_VISTA
        if (status == 0) {
            throw NX::utility::create_system_error(GetLastError());
        }

        outStream << dateStr << L"T" << timeStr;
        uint64_t frac_sec = largeInt.QuadPart % _secondTicks;
        if (frac_sec > 0) {
            // Append fractional second, which is a 7-digit value with no trailing zeros
            // This way, '1200' becomes '00012'
            wchar_t buf[9] = { 0 };
            swprintf_s(buf, sizeof(buf), L".%07ld", (long int)frac_sec);
            // trim trailing zeros
            for (int i = 7; buf[i] == L'0'; i--) buf[i] = L'\0';
            outStream << buf;
        }
        outStream << L"Z";
    }

    return outStream.str();
}

bool __cdecl datetime::system_type_to_datetime(void* pvsysTime, uint64_t seconds, datetime * pdt)
{
    SYSTEMTIME* psysTime = (SYSTEMTIME*)pvsysTime;
    FILETIME fileTime;

    if (SystemTimeToFileTime(psysTime, &fileTime))
    {
        ULARGE_INTEGER largeInt;
        largeInt.LowPart = fileTime.dwLowDateTime;
        largeInt.HighPart = fileTime.dwHighDateTime;

        // Add hundredths of nanoseconds
        largeInt.QuadPart += seconds;

        *pdt = datetime(largeInt.QuadPart);
        return true;
    }
    return false;
}

// Take a string that represents a fractional second and return the number of ticks
// This is equivalent to doing atof on the string and multiplying by 10000000,
// but does not lose precision
uint64_t timeticks_from_second(std::wstring::iterator begin, std::wstring::iterator end)
{
    int size = (int)(end - begin);
    _ASSERTE(begin[0] == L'.');
    uint64_t ufrac_second = 0;
    for (int i = 1; i <= 7; ++i) {
        ufrac_second *= 10;
        int add = i < size ? begin[i] - L'0' : 0;
        ufrac_second += add;
    }
    return ufrac_second;
}

void extract_fractional_second(const std::wstring& dateString, std::wstring& resultString, uint64_t& ufrac_second)
{
    resultString = dateString;
    // First, the string must be strictly longer than 2 characters, and the trailing character must be 'Z'
    if (resultString.size() > 2 && resultString[resultString.size() - 1] == L'Z')
    {
        // Second, find the last non-digit by scanning the string backwards
        auto last_non_digit = std::find_if_not(resultString.rbegin() + 1, resultString.rend(), is_digit);
        if (last_non_digit < resultString.rend() - 1)
        {
            // Finally, make sure the last non-digit is a dot:
            auto last_dot = last_non_digit.base() - 1;
            if (*last_dot == L'.') {
                // Got it! Now extract the fractional second
                auto last_before_Z = std::end(resultString) - 1;
                ufrac_second = timeticks_from_second(last_dot, last_before_Z);
                // And erase it from the string
                resultString.erase(last_dot, last_before_Z);
            }
        }
    }
}

/// <summary>
/// Returns a string representation of the datetime. The string is formatted based on RFC 1123 or ISO 8601
/// </summary>
datetime __cdecl datetime::from_string(const std::wstring& dateString, date_format format)
{
    // avoid floating point math to preserve precision
    uint64_t ufrac_second = 0;
    datetime result;

    if ( format == RFC_1123 ) {

        SYSTEMTIME sysTime = {0};

        std::wstring month(3, L'\0');
        std::wstring unused(3, L'\0');

        const wchar_t * formatString = L"%3c, %2d %3c %4d %2d:%2d:%2d %3c";
        auto n = swscanf_s(dateString.c_str(), formatString,
            unused.data(), unused.size(),
            &sysTime.wDay,
            month.data(), month.size(),
            &sysTime.wYear,
            &sysTime.wHour,
            &sysTime.wMinute,
            &sysTime.wSecond,
            unused.data(), unused.size());

        if (n == 8) {

            std::wstring monthnames[12] = {L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun", L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec"};
            auto loc = std::find_if(monthnames, monthnames+12, [&month](const std::wstring& m) { return m == month;});

            if (loc != monthnames+12) {
                sysTime.wMonth = (short) ((loc - monthnames) + 1);
                if (system_type_to_datetime(&sysTime, ufrac_second, &result)) {
                    return result;
                }
            }
        }
    }
    else if ( format == ISO_8601 ) {

        // Unlike FILETIME, SYSTEMTIME does not have enough precision to hold seconds in 100 nanosecond
        // increments. Therefore, start with seconds and milliseconds set to 0, then add them separately

        // Try to extract the fractional second from the timestamp
        std::wstring input;
        extract_fractional_second(dateString, input, ufrac_second);
        {
            SYSTEMTIME sysTime = { 0 };
            const wchar_t * formatString = L"%4d-%2d-%2dT%2d:%2d:%2dZ";
            auto n = swscanf_s(input.c_str(), formatString,
                &sysTime.wYear,
                &sysTime.wMonth,
                &sysTime.wDay,
                &sysTime.wHour,
                &sysTime.wMinute,
                &sysTime.wSecond);

            if (n == 3 || n == 6) {
                if (system_type_to_datetime(&sysTime, ufrac_second, &result)) {
                    return result;
                }
            }
        }
        {
            SYSTEMTIME sysTime = {0};
            DWORD date = 0;

            const wchar_t * formatString = L"%8dT%2d:%2d:%2dZ";
            auto n = swscanf_s(input.c_str(), formatString,
                &date,
                &sysTime.wHour,
                &sysTime.wMinute,
                &sysTime.wSecond);

            if (n == 1 || n == 4) {
                sysTime.wDay = date % 100;
                date /= 100;
                sysTime.wMonth = date % 100;
                date /= 100;
                sysTime.wYear = (WORD)date;

                if (system_type_to_datetime(&sysTime, ufrac_second, &result)) {
                    return result;
                }
            }
        }
        {
            SYSTEMTIME sysTime = {0};
            GetSystemTime(&sysTime);    // Fill date portion with today's information
            sysTime.wSecond = 0;
            sysTime.wMilliseconds = 0;

            const wchar_t * formatString = L"%2d:%2d:%2dZ";
            auto n = swscanf_s(input.c_str(), formatString,
                &sysTime.wHour,
                &sysTime.wMinute,
                &sysTime.wSecond);

            if (n == 3) {
                if (system_type_to_datetime(&sysTime, ufrac_second, &result)) {
                    return result;
                }
            }
        }
    }

    return datetime();
}





/// <summary>
/// Converts a timespan/interval in seconds to xml duration string as specified by
/// http://www.w3.org/TR/xmlschema-2/#duration
/// </summary>
std::wstring __cdecl timespan::seconds_to_xml_duration(NX::utility::seconds durationSecs)
{
    auto numSecs = durationSecs.count();

    // Find the number of minutes
    auto numMins =  numSecs / 60;
    if (numMins > 0) {
        numSecs = numSecs % 60;
    }

    // Hours
    auto numHours = numMins / 60;
    if (numHours > 0) {
        numMins = numMins % 60;
    }

    // Days
    auto numDays = numHours / 24;
    if (numDays > 0) {
        numHours = numHours % 24;
    }

    // The format is:
    // PdaysDThoursHminutesMsecondsS
    std::wostringstream oss;

    oss << L"P";
    if (numDays > 0) {
        oss << numDays << L"D";
    }

    oss << L"T";

    if (numHours > 0) {
        oss << numHours << L"H";
    }

    if (numMins > 0) {
        oss << numMins << L"M";
    }

    if (numSecs > 0) {
        oss << numSecs << L"S";
    }

    return oss.str();
}

/// <summary>
/// Converts an xml duration to timespan/interval in seconds
/// http://www.w3.org/TR/xmlschema-2/#duration
/// </summary>
NX::utility::seconds __cdecl timespan::xml_duration_to_seconds(std::wstring timespanString)
{
    // The format is:
    // PnDTnHnMnS
    // if n == 0 then the field could be omitted
    // The final S could be omitted

    int64_t numSecs = 0;

    std::wistringstream is(timespanString);
    auto eof = std::char_traits<wchar_t>::eof();

    std::basic_istream<wchar_t>::int_type c;
    c = is.get(); // P

    while (c != eof) {

        int val = 0;
        c = is.get();

        while (is_digit((wchar_t)c)) {
            val = val * 10 + (c - L'0');
            c = is.get();

            if (c == '.') {
                // decimal point is not handled
                do { c = is.get(); } while(is_digit((wchar_t)c));
            }
        }

        if (c == L'D') numSecs += val * 24 * 3600; // days
        if (c == L'H') numSecs += val * 3600; // Hours
        if (c == L'M') numSecs += val * 60; // Minutes
        if (c == L'S' || c == eof) {
            numSecs += val; // seconds
            break;
        }
    }

    return utility::seconds(numSecs);
}