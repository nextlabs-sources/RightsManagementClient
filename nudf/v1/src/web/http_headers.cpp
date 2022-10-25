

#include <Windows.h>

#include <nudf\web\conversions.hpp>

#include <nudf\web\date_time.hpp>
#include <nudf\web\http_constant.hpp>
#include <nudf\web\http_headers.hpp>


using namespace NX::web;
using namespace NX::web::http;;



#define CRLF    L"\r\n"

std::wstring http_headers::content_type() const
{
    std::wstring result;
    match(http::header_names::content_type, result);
    return result;
}

void http_headers::set_content_type(std::wstring type)
{
    m_headers[http::header_names::content_type] = std::move(type);
}

std::wstring http_headers::cache_control() const
{
    std::wstring result;
    match(http::header_names::cache_control, result);
    return result;
}

void http_headers::set_cache_control(std::wstring control)
{
    add(http::header_names::cache_control, std::move(control));
}

std::wstring http_headers::date() const
{
    std::wstring result;
    match(http::header_names::date, result);
    return result;
}

void http_headers::set_date(const NX::utility::datetime& date)
{
    m_headers[http::header_names::date] = date.to_string(NX::utility::datetime::RFC_1123);
}

unsigned __int64 http_headers::content_length() const
{
    unsigned __int64 length = 0;
    match(http::header_names::content_length, length);
    return length;
}

void http_headers::set_content_length(unsigned __int64 length)
{
    m_headers[http::header_names::content_length] = NX::utility::conversions::print_string(length);
}