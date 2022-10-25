

#include <Windows.h>

#include <iostream>
#include <sstream>
#include <algorithm>

#include <nudf\web\conversions.hpp>
#include <nudf\web\uri.hpp>



std::wstring NX::web::details::uri_components::join()
{
    // canonicalize components first

    // convert scheme to lowercase
    std::transform(m_scheme.begin(), m_scheme.end(), m_scheme.begin(), tolower);

    // convert host to lowercase
    std::transform(m_host.begin(), m_host.end(), m_host.begin(), tolower);

    // canonicalize the path to have a leading slash if it's a full uri
    if (!m_host.empty() && m_path.empty()) {
        m_path = L"/";
    }
    else if (!m_host.empty() && m_path[0] != L'/') {
        m_path.insert(m_path.begin(), 1, L'/');
    }

    std::wostringstream os;

    if (!m_scheme.empty()) {
        os << m_scheme << L':';
    }

    if (!m_host.empty()) {

        os << L"//" << m_host;
        if (m_port > 0) {
            os << L':' << m_port;
        }
    }

    if (!m_path.empty()) {

        // only add the leading slash when the host is present
        if (!m_host.empty() && m_path.front() != L'/') {
            os << L'/';
        }
        os << m_path;
    }

    if (!m_query.empty()) {
        os << L'?' << m_query;
    }

    if (!m_fragment.empty()) {
        os << L'#' << m_fragment;
    }

    return os.str();
}

using namespace NX::web::details;
static std::locale g_clocale("C"); // use the C local to force the ASCII definitions of isalpha and friends

bool uri_parser::validate(const std::wstring &encoded_string)
{
    const wchar_t *scheme_begin = nullptr;
    const wchar_t *scheme_end = nullptr;
    const wchar_t *uinfo_begin = nullptr;
    const wchar_t *uinfo_end = nullptr;
    const wchar_t *host_begin = nullptr;
    const wchar_t *host_end = nullptr;
    int port_ptr = 0;
    const wchar_t *path_begin = nullptr;
    const wchar_t *path_end = nullptr;
    const wchar_t *query_begin = nullptr;
    const wchar_t *query_end = nullptr;
    const wchar_t *fragment_begin = nullptr;
    const wchar_t *fragment_end = nullptr;

    // perform a parse, but don't copy out the data
    return inner_parse(
        encoded_string.c_str(),
        &scheme_begin,
        &scheme_end,
        &uinfo_begin,
        &uinfo_end,
        &host_begin,
        &host_end,
        &port_ptr,
        &path_begin,
        &path_end,
        &query_begin,
        &query_end,
        &fragment_begin,
        &fragment_end);
}

bool uri_parser::parse(const std::wstring &encoded_string, uri_components &components)
{
    const wchar_t *scheme_begin = nullptr;
    const wchar_t *scheme_end = nullptr;
    const wchar_t *host_begin = nullptr;
    const wchar_t *host_end = nullptr;
    const wchar_t *uinfo_begin = nullptr;
    const wchar_t *uinfo_end = nullptr;
    int port_ptr = 0;
    const wchar_t *path_begin = nullptr;
    const wchar_t *path_end = nullptr;
    const wchar_t *query_begin = nullptr;
    const wchar_t *query_end = nullptr;
    const wchar_t *fragment_begin = nullptr;
    const wchar_t *fragment_end = nullptr;

    if (inner_parse(
        encoded_string.c_str(),
        &scheme_begin,
        &scheme_end,
        &uinfo_begin,
        &uinfo_end,
        &host_begin,
        &host_end,
        &port_ptr,
        &path_begin,
        &path_end,
        &query_begin,
        &query_end,
        &fragment_begin,
        &fragment_end))
    {
        if (scheme_begin) {
            components.m_scheme.assign(scheme_begin, scheme_end);
            // convert scheme to lowercase
            std::transform(components.m_scheme.begin(), components.m_scheme.end(), components.m_scheme.begin(), tolower);
        }
        else {
            components.m_scheme.clear();
        }

        if (uinfo_begin) {
            components.m_user_info.assign(uinfo_begin, uinfo_end);
        }

        if (host_begin) {
            components.m_host.assign(host_begin, host_end);
            // convert host to lowercase
            std::transform(components.m_host.begin(), components.m_host.end(), components.m_host.begin(), tolower);
        }
        else {
            components.m_host.clear();
        }

        if (port_ptr) {
            components.m_port = port_ptr;
        }
        else {
            components.m_port = 0;
        }

        if (path_begin) {
            components.m_path.assign(path_begin, path_end);
        }
        else {
            // default path to begin with a slash for easy comparison
            components.m_path = L"/";
        }

        if (query_begin) {
            components.m_query.assign(query_begin, query_end);
        }
        else {
            components.m_query.clear();
        }

        if (fragment_begin) {
            components.m_fragment.assign(fragment_begin, fragment_end);
        }
        else {
            components.m_fragment.clear();
        }

        return true;
    }
    else {
        return false;
    }
}

bool uri_parser::inner_parse( const wchar_t *encoded,
                  const wchar_t **scheme_begin, const wchar_t **scheme_end,
                  const wchar_t **uinfo_begin, const wchar_t **uinfo_end,
                  const wchar_t **host_begin, const wchar_t **host_end,
                  _Out_ int *port,
                  const wchar_t **path_begin, const wchar_t **path_end,
                  const wchar_t **query_begin, const wchar_t **query_end,
                  const wchar_t **fragment_begin, const wchar_t **fragment_end)
{
    *scheme_begin = nullptr;
    *scheme_end = nullptr;
    *uinfo_begin = nullptr;
    *uinfo_end = nullptr;
    *host_begin = nullptr;
    *host_end = nullptr;
    *port = 0;
    *path_begin = nullptr;
    *path_end = nullptr;
    *query_begin = nullptr;
    *query_end = nullptr;
    *fragment_begin = nullptr;
    *fragment_end = nullptr;

    const wchar_t *p = encoded;

    // IMPORTANT -- A uri may either be an absolute uri, or an relative-reference
    // Absolute: 'http://host.com'
    // Relative-Reference: '//:host.com', '/path1/path2?query', './path1:path2'
    // A Relative-Reference can be disambiguated by parsing for a ':' before the first slash

    bool is_relative_reference = true;
    const wchar_t *p2 = p;
    for (;*p2 != L'/' && *p2 != L'\0'; p2++) {
        if (*p2 == L':') {
            // found a colon, the first portion is a scheme
            is_relative_reference = false;
            break;
        }
    }

    if (!is_relative_reference) {
        // the first character of a scheme must be a letter
        if (!isalpha(*p)) {
            return false;
        }

        // start parsing the scheme, it's always delimited by a colon (must be present)
        *scheme_begin = p++;
        for (;*p != ':'; p++) {
            if (!is_scheme_character(*p)) {
                return false;
            }
        }
        *scheme_end = p;

        // skip over the colon
        p++;
    }

    // if we see two slashes next, then we're going to parse the authority portion
    // later on we'll break up the authority into the port and host
    const wchar_t *authority_begin = nullptr;
    const wchar_t *authority_end = nullptr;
    if (*p == L'/' && p[1] == L'/') {
        // skip over the slashes
        p += 2;
        authority_begin = p;

        // the authority is delimited by a slash (resource), question-mark (query) or octothorpe (fragment)
        // or by EOS. The authority could be empty ('file:///C:\file_name.txt')
        for (;*p != L'/' && *p != L'?' && *p != L'#' && *p != L'\0'; p++) {
            // We're NOT currently supporting IPv6, IPvFuture or username/password in authority
            if (!is_authority_character(*p)) {
                return false;
            }
        }
        authority_end = p;

        // now lets see if we have a port specified -- by working back from the end
        if (authority_begin != authority_end) {
            // the port is made up of all digits
            const wchar_t *port_begin = authority_end - 1;
            for (;isdigit(*port_begin) && port_begin != authority_begin; port_begin--)
            { }

            if (*port_begin == L':') {
                // has a port
                *host_begin = authority_begin;
                *host_end = port_begin;

                //skip the colon
                port_begin++;

                std::wistringstream port_str(std::wstring(port_begin, authority_end));
                int port_num;
                port_str >> port_num;
                *port = port_num;
            }
            else {
                // no port
                *host_begin = authority_begin;
                *host_end = authority_end;
            }

            // look for a user_info component
            const wchar_t *u_end = *host_begin;
            for (;is_user_info_character(*u_end) && u_end != *host_end; u_end++)
            { }

            if (*u_end == L'@') {
                *host_begin = u_end+1;
                *uinfo_begin = authority_begin;
                *uinfo_end = u_end;
            }
            else {
                uinfo_end = uinfo_begin = nullptr;
            }
        }
    }

    // if we see a path character or a slash, then the
    // if we see a slash, or any other legal path character, parse the path next
    if (*p == L'/' || is_path_character(*p)) {

        *path_begin = p;

        // the path is delimited by a question-mark (query) or octothorpe (fragment) or by EOS
        for (;*p != L'?' && *p != L'#' && *p != L'\0'; p++) {
            if (!is_path_character(*p)) {
                return false;
            }
        }
        *path_end = p;
    }

    // if we see a ?, then the query is next
    if (*p == L'?') {
        // skip over the question mark
        p++;
        *query_begin = p;

        // the query is delimited by a '#' (fragment) or EOS
        for (;*p != L'#' && *p != L'\0'; p++) {
            if (!is_query_character(*p)) {
                return false;
            }
        }
        *query_end = p;
    }

    // if we see a #, then the fragment is next
    if (*p == L'#') {
        // skip over the hash mark
        p++;
        *fragment_begin = p;

        // the fragment is delimited by EOS
        for (;*p != L'\0'; p++) {
            if (!is_fragment_character(*p)) {
                return false;
            }
        }
        *fragment_end = p;
    }

    return true;
}


//
//  class uri_builder
//
NX::web::uri_builder &NX::web::uri_builder::append_path(const std::wstring &path, bool is_encode)
{
    if(path.empty() || path == L"/") {
        return *this;
    }

    auto encoded_path = is_encode ? uri::encode_uri(path, uri::components::path) : path;
    auto thisPath = this->path();
    if(thisPath.empty() || thisPath == L"/") {
        if(encoded_path.front() != L'/') {
            set_path(L"/" + encoded_path);
        }
        else {
            set_path(encoded_path);
        }
    }
    else if(thisPath.back() == L'/' && encoded_path.front() == L'/') {
        thisPath.pop_back();
        set_path(thisPath + encoded_path);
    }
    else if(thisPath.back() != L'/' && encoded_path.front() != L'/') {
        set_path(thisPath + L"/" + encoded_path);
    }
    else {
        // Only one slash.
        set_path(thisPath + encoded_path);
    }
    return *this;
}

NX::web::uri_builder &NX::web::uri_builder::append_query(const std::wstring &query, bool is_encode)
{
    if(query.empty()) {
        return *this;
    }

    auto encoded_query = is_encode ? uri::encode_uri(query, uri::components::query) : query;
    auto thisQuery = this->query();
    if (thisQuery.empty()) {
        this->set_query(encoded_query);
    }
    else if(thisQuery.back() == L'&' && encoded_query.front() == L'&') {
        thisQuery.pop_back();
        this->set_query(thisQuery + encoded_query);
    }
    else if(thisQuery.back() != L'&' && encoded_query.front() != L'&') {
        this->set_query(thisQuery + L"&" + encoded_query);
    }
    else {
        // Only one ampersand.
        this->set_query(thisQuery + encoded_query);
    }
    return *this;
}

NX::web::uri_builder &NX::web::uri_builder::append(const web::uri &relative_uri)
{
    append_path(relative_uri.path());
    append_query(relative_uri.query());
    this->set_fragment(this->fragment() + relative_uri.fragment());
    return *this;
}

std::wstring NX::web::uri_builder::to_string()
{
    return to_uri().to_string();
}

NX::web::uri NX::web::uri_builder::to_uri()
{
    return NX::web::uri(m_uri.join());
}

bool NX::web::uri_builder::is_valid()
{
    return NX::web::uri::validate(m_uri.join());
}



//
//  class uri
//
NX::web::uri::uri(const std::wstring &uri_string)
{
    if (!details::uri_parser::parse(uri_string, m_components))
    {
        throw uri_exception("provided uri is invalid: " + NX::utility::conversions::to_utf8string(uri_string));
    }
    m_uri = m_components.join();
}

NX::web::uri::uri(const wchar_t *uri_string): m_uri(uri_string)
{
    if (!details::uri_parser::parse(uri_string, m_components))
    {
        throw uri_exception("provided uri is invalid: " + NX::utility::conversions::to_utf8string(uri_string));
    }
    m_uri = m_components.join();
}

std::wstring NX::web::uri::encode_impl(const std::wstring &raw, const std::function<bool(int)>& should_encode)
{
    const wchar_t * const hex = L"0123456789ABCDEF";
    std::wstring encoded;
    std::string utf8raw = NX::utility::conversions::to_utf8string(raw);
    for (auto iter = utf8raw.begin(); iter != utf8raw.end(); ++iter)
    {
        // for utf8 encoded string, char ASCII can be greater than 127.
        int ch = static_cast<unsigned char>(*iter);
        // ch should be same under both utf8 and utf16.
        if(should_encode(ch))
        {
            encoded.push_back(L'%');
            encoded.push_back(hex[(ch >> 4) & 0xF]);
            encoded.push_back(hex[ch & 0xF]);
        }
        else
        {
            // ASCII don't need to be encoded, which should be same on both utf8 and utf16.
            encoded.push_back((wchar_t)ch);
        }
    }
    return encoded;
}

/// </summary>
/// Encodes a string by converting all characters except for RFC 3986 unreserved characters to their
/// hexadecimal representation.
/// </summary>
std::wstring NX::web::uri::encode_data_string(const std::wstring &raw)
{
    return NX::web::uri::encode_impl(raw, [](int ch) -> bool
    {
        return !uri_parser::is_unreserved(ch);
    });
}

std::wstring NX::web::uri::encode_uri(const std::wstring &raw, NX::web::uri::components::component component)
{
    // Note: we also encode the '+' character because some non-standard implementations
    // encode the space character as a '+' instead of %20. To better interoperate we encode
    // '+' to avoid any confusion and be mistaken as a space.
    switch(component)
    {
    case components::user_info:
        return NX::web::uri::encode_impl(raw, [](int ch) -> bool
        {
            return !uri_parser::is_user_info_character(ch)
                || ch == '%' || ch == '+';
        });
    case components::host:
        return uri::encode_impl(raw, [](int ch) -> bool
        {
            // No encoding of ASCII characters in host name (RFC 3986 3.2.2)
            return ch > 127;
        });
    case components::path:
        return uri::encode_impl(raw, [](int ch) -> bool
        {
            return !uri_parser::is_path_character(ch)
                || ch == '%' || ch == '+';
        });
    case components::query:
        return uri::encode_impl(raw, [](int ch) -> bool
        {
            return !uri_parser::is_query_character(ch)
                || ch == '%' || ch == '+';
        });
    case components::fragment:
        return uri::encode_impl(raw, [](int ch) -> bool
        {
            return !uri_parser::is_fragment_character(ch)
                || ch == '%' || ch == '+';
        });
    case components::full_uri:
    default:
        return uri::encode_impl(raw, [](int ch) -> bool
        {
            return !uri_parser::is_unreserved(ch) && !uri_parser::is_reserved(ch);
        });
    };
}

/// <summary>
/// Helper function to convert a hex character digit to a decimal character value.
/// Throws an exception if not a valid hex digit.
/// </summary>
static int hex_char_digit_to_decimal_char(int hex)
{
    int decimal;
    if(hex >= '0' && hex <= '9')
    {
        decimal = hex - '0';
    }
    else if(hex >= 'A' && hex <= 'F')
    {
        decimal = 10 + (hex - 'A');
    }
    else if(hex >= 'a' && hex <= 'f')
    {
        decimal = 10 + (hex - 'a');
    }
    else
    {
        throw NX::web::uri_exception("Invalid hexidecimal digit");
    }
    return decimal;
}

std::wstring NX::web::uri::decode(const std::wstring &encoded)
{
    std::string utf8raw;
    for(auto iter = encoded.begin(); iter != encoded.end(); ++iter) {
        if(*iter == L'%') {
            if(++iter == encoded.end()) {
                throw NX::web::uri_exception("Invalid URI string, two hexidecimal digits must follow '%'");
            }
            int decimal_value = hex_char_digit_to_decimal_char(static_cast<int>(*iter)) << 4;
            if(++iter == encoded.end()) {
                throw NX::web::uri_exception("Invalid URI string, two hexidecimal digits must follow '%'");
            }
            decimal_value += hex_char_digit_to_decimal_char(static_cast<int>(*iter));

            utf8raw.push_back(static_cast<char>(decimal_value));
        }
        else {
            // encoded string has to be ASCII.
            utf8raw.push_back(reinterpret_cast<const char &>(*iter));
        }
    }
    return NX::utility::conversions::utf8_to_utf16(utf8raw);
}

std::vector<std::wstring> NX::web::uri::split_path(const std::wstring &path)
{
    std::vector<std::wstring> results;
    std::wistringstream iss(path);
    std::wstring s;

    while (std::getline(iss, s, L'/')) {
        if (!s.empty()) {
            results.push_back(s);
        }
    }

    return results;
}

std::map<std::wstring, std::wstring> NX::web::uri::split_query(const std::wstring &query)
{
    std::map<std::wstring, std::wstring> results;

    // Split into key value pairs separated by '&'.
    size_t prev_amp_index = 0;
    while(prev_amp_index != std::wstring::npos)
    {
        size_t amp_index = query.find_first_of(L'&', prev_amp_index);
        if (amp_index == std::wstring::npos)
            amp_index = query.find_first_of(L';', prev_amp_index);

        std::wstring key_value_pair = query.substr(
            prev_amp_index,
            amp_index == std::wstring::npos ? query.size() - prev_amp_index : amp_index - prev_amp_index);
        prev_amp_index = amp_index == std::wstring::npos ? std::wstring::npos : amp_index + 1;

        size_t equals_index = key_value_pair.find_first_of(L'=');
        if(equals_index == std::wstring::npos) {
            continue;
        }
        else if (equals_index == 0) {
            std::wstring value(key_value_pair.begin() + equals_index + 1, key_value_pair.end());
            results[L""] = value;
        }
        else {
            std::wstring key(key_value_pair.begin(), key_value_pair.begin() + equals_index);
            std::wstring value(key_value_pair.begin() + equals_index + 1, key_value_pair.end());
            results[key] = value;
        }
    }

    return results;
}

bool NX::web::uri::validate(const std::wstring &uri_string)
{
    return uri_parser::validate(uri_string);
}

NX::web::uri NX::web::uri::authority() const
{
        return uri_builder().set_scheme(this->scheme()).set_host(this->host()).set_port(this->port()).set_user_info(this->user_info()).to_uri();
}

NX::web::uri NX::web::uri::resource() const
{
        return uri_builder().set_path(this->path()).set_query(this->query()).set_fragment(this->fragment()).to_uri();
}

bool NX::web::uri::operator == (const uri &other) const
{
    // Each individual URI component must be decoded before performing comparison.
    // TFS # 375865

    if (this->is_empty() && other.is_empty())
    {
        return true;
    }
    else if (this->is_empty() || other.is_empty())
    {
        return false;
    }
    else if (this->scheme() != other.scheme())
    {
        // scheme is canonicalized to lowercase
        return false;
    }
    else if(uri::decode(this->user_info()) != uri::decode(other.user_info()))
    {
        return false;
    }
    else if (uri::decode(this->host()) != uri::decode(other.host()))
    {
        // host is canonicalized to lowercase
        return false;
    }
    else if (this->port() != other.port())
    {
        return false;
    }
    else if (uri::decode(this->path()) != uri::decode(other.path()))
    {
        return false;
    }
    else if (uri::decode(this->query()) != uri::decode(other.query()))
    {
        return false;
    }
    else if (uri::decode(this->fragment()) != uri::decode(other.fragment()))
    {
        return false;
    }

    return true;
}