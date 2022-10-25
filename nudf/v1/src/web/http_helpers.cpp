

#include <Windows.h>

#include <nudf\web\conversions.hpp>

#include <nudf\web\http_constant.hpp>
#include <nudf\web\http_helpers.hpp>


using namespace NX::web;
using namespace NX::web::http;
using namespace NX::web::http::details;



static bool is_content_type_one_of(const std::wstring *first, const std::wstring *last, const std::wstring &value)
{
    while (first != last) {
        if(0 == _wcsicmp((*first).c_str(), value.c_str())) {
            return true;
        }
        ++first;
    }
    return false;
}

bool NX::web::http::details::is_content_type_textual(const std::wstring &content_type)
{
    static const std::wstring textual_types[] = {
        mime_types::message_http,
        mime_types::application_json,
        mime_types::application_xml,
        mime_types::application_atom_xml,
        mime_types::application_http,
        mime_types::application_x_www_form_urlencoded
    };

    if(content_type.size() >= 4 && 0 == _wcsicmp(content_type.substr(0,4).c_str(), L"text")) {
        return true;
    }
    return (is_content_type_one_of(std::begin(textual_types), std::end(textual_types), content_type));
}

bool NX::web::http::details::is_content_type_json(const std::wstring &content_type)
{
    static const std::wstring json_types[] = {
        mime_types::application_json,
        mime_types::application_xjson,
        mime_types::text_json,
        mime_types::text_xjson,
        mime_types::text_javascript,
        mime_types::text_xjavascript,
        mime_types::application_javascript,
        mime_types::application_xjavascript
    };

    return (is_content_type_one_of(std::begin(json_types), std::end(json_types), content_type));
}

void NX::web::http::details::parse_content_type_and_charset(const std::wstring &content_type, std::wstring &content, std::wstring &charset)
{
    const size_t semi_colon_index = content_type.find_first_of(L";");

    // No charset specified.
    if(semi_colon_index == std::wstring::npos) {
        content = content_type;
        trim_whitespace(content);
        charset = get_default_charset(content);
        return;
    }

    // Split into content type and second part which could be charset.
    content = content_type.substr(0, semi_colon_index);
    trim_whitespace(content);
    std::wstring possible_charset = content_type.substr(semi_colon_index + 1);
    trim_whitespace(possible_charset);
    const size_t equals_index = possible_charset.find_first_of(L"=");

    // No charset specified.
    if(equals_index == std::wstring::npos) {
        charset = get_default_charset(content);
        return;
    }

    // Split and make sure 'charset'
    std::wstring charset_key = possible_charset.substr(0, equals_index);
    trim_whitespace(charset_key);
    if(0 != _wcsicmp(charset_key.c_str(), L"charset"))
    {
        charset = get_default_charset(content);
        return;
    }
    charset = possible_charset.substr(equals_index + 1);
    // Remove the redundant ';' at the end of charset.
    while (charset.back() == L';') {
        charset.pop_back();
    }
    trim_whitespace(charset);
    if (charset.front() == L'"' && charset.back() == L'"') {
        charset = charset.substr(1, charset.size() - 2);
        trim_whitespace(charset);
    }
}

std::wstring NX::web::http::details::get_default_charset(const std::wstring &content_type)
{
    // We are defaulting everything to Latin1 except JSON which is utf-8.
    if(is_content_type_json(content_type)) {
        return charset_types::utf8;
    }
    else {
        return charset_types::latin1;
    }
}

// Helper function to determine byte order mark.
enum endian_ness
{
    little_endian,
    big_endian,
    unknown
};
static endian_ness check_byte_order_mark(const std::wstring &str)
{
    if(str.empty()) {
        return unknown;
    }
    const unsigned char *src = (const unsigned char *)&str[0];

    // little endian
    if(src[0] == 0xFF && src[1] == 0xFE) {
        return little_endian;
    }

    // big endian
    else if(src[0] == 0xFE && src[1] == 0xFF) {
        return big_endian;
    }

    return unknown;
}

std::wstring NX::web::http::details::convert_utf16_to_string_t(std::wstring src)
{
    return NX::web::http::details::convert_utf16_to_utf16(std::move(src));
}

std::string NX::web::http::details::convert_utf16_to_utf8(std::wstring src)
{
    const endian_ness endian = check_byte_order_mark(src);
    switch(endian)
    {
    case little_endian:
        return NX::web::http::details::convert_utf16le_to_utf8(std::move(src), true);
    case big_endian:
        return NX::web::http::details::convert_utf16be_to_utf8(std::move(src), true);
    case unknown:
        // unknown defaults to big endian.
        return NX::web::http::details::convert_utf16be_to_utf8(std::move(src), false);
    }
    __assume(0);
}

std::wstring NX::web::http::details::convert_utf16_to_utf16(std::wstring src)
{
    const endian_ness endian = check_byte_order_mark(src);
    switch(endian)
    {
    case little_endian:
        src.erase(0, 1);
        return std::move(src);
    case big_endian:
        return convert_utf16be_to_utf16le(std::move(src), true);
    case unknown:
        // unknown defaults to big endian.
        return convert_utf16be_to_utf16le(std::move(src), false);
    }
    __assume(0);
}

std::string NX::web::http::details::convert_utf16le_to_utf8(std::wstring src, bool erase_bom)
{
    if(erase_bom && !src.empty())
    {
        src.erase(0, 1);
    }
    return NX::utility::conversions::utf16_to_utf8(std::move(src));
}

std::wstring NX::web::http::details::convert_utf16le_to_string_t(std::wstring src, bool erase_bom)
{
    if(erase_bom && !src.empty())
    {
        src.erase(0, 1);
    }

    return std::move(src);
}

// Helper function to change endian ness from big endian to little endian
static std::wstring big_endian_to_little_endian(std::wstring src, bool erase_bom)
{
    if(erase_bom && !src.empty()) {
        src.erase(0, 1);
    }
    if(src.empty()) {
        return std::move(src);
    }

    const size_t size = src.size();
    for(size_t i = 0; i < size; ++i) {
        wchar_t ch = src[i];
        src[i] = static_cast<wchar_t>(ch << 8);
        src[i] = static_cast<wchar_t>(src[i] | ch >> 8);
    }

    return std::move(src);
}

std::wstring NX::web::http::details::convert_utf16be_to_string_t(std::wstring src, bool erase_bom)
{
    return NX::web::http::details::convert_utf16be_to_utf16le(std::move(src), erase_bom);
}

std::string NX::web::http::details::convert_utf16be_to_utf8(std::wstring src, bool erase_bom)
{
    return NX::utility::conversions::utf16_to_utf8(big_endian_to_little_endian(std::move(src), erase_bom));
}

std::wstring NX::web::http::details::convert_utf16be_to_utf16le(std::wstring src, bool erase_bom)
{
    return big_endian_to_little_endian(std::move(src), erase_bom);
}

void NX::web::http::details::ltrim_whitespace(std::wstring &str)
{
    size_t index;
    for(index = 0; index < str.size() && isspace(str[index]); ++index);
    str.erase(0, index);
}
void NX::web::http::details::rtrim_whitespace(std::wstring &str)
{
    size_t index;
    for(index = str.size(); index > 0 && isspace(str[index - 1]); --index);
    str.erase(index);
}
void NX::web::http::details::trim_whitespace(std::wstring &str)
{
    NX::web::http::details::ltrim_whitespace(str);
    NX::web::http::details::rtrim_whitespace(str);
}

size_t NX::web::http::details::chunked_encoding::add_chunked_delimiters(_Out_writes_ (buffer_size) uint8_t *data, _In_ size_t buffer_size, size_t bytes_read)
{
    size_t offset = 0;

    if(buffer_size < bytes_read + NX::web::http::details::chunked_encoding::additional_encoding_space) {
        throw std::exception("Insufficient buffer size.");
    }

    if ( bytes_read == 0 ) {
        offset = 7;
        data[7] = '0';
        data[8] = '\r';  data[9] = '\n'; // The end of the size.
        data[10] = '\r'; data[11] = '\n'; // The end of the message.
    }
    else {
        char buffer[9];
        sprintf_s(buffer, sizeof(buffer), "%8IX", bytes_read);
        memcpy(&data[0], buffer, 8);
        while (data[offset] == ' ') ++offset;
        data[8] = '\r'; data[9] = '\n'; // The end of the size.
        data[10+bytes_read] = '\r'; data[11+bytes_read] = '\n'; // The end of the chunk.
    }

    return offset;
}