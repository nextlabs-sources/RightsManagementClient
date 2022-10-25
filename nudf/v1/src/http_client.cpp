

#include <Windows.h>
#include <assert.h>

#include <memory>
#include <algorithm>

#include <boost\algorithm\string.hpp>

#include <nudf\uri.hpp>
#include <nudf\convert.hpp>

#define NX_USE_WINHTTP_ERROR
#include <nudf\http_client.hpp>


using namespace NX;


const std::wstring http::methods::GET(L"GET");
const std::wstring http::methods::POST(L"POST");
const std::wstring http::methods::PUT(L"PUT");
const std::wstring http::methods::DEL(L"DELETE");
const std::wstring http::methods::HEAD(L"HEAD");
const std::wstring http::methods::OPTIONS(L"OPTIONS");
const std::wstring http::methods::TRCE(L"TRACE");
const std::wstring http::methods::CONNECT(L"CONNECT");
const std::wstring http::methods::MERGE(L"MERGE");
const std::wstring http::methods::PATCH(L"PATCH");

const std::wstring http::header_names::accept(L"Accept");
const std::wstring http::header_names::accept_charset(L"Accept-Charset");
const std::wstring http::header_names::accept_encoding(L"Accept-Encoding");
const std::wstring http::header_names::accept_language(L"Accept-Language");
const std::wstring http::header_names::accept_ranges(L"Accept-Ranges");
const std::wstring http::header_names::age(L"Age");
const std::wstring http::header_names::allow(L"Allow");
const std::wstring http::header_names::authorization(L"Authorization");
const std::wstring http::header_names::cache_control(L"Cache-Control");
const std::wstring http::header_names::connection(L"Connection");
const std::wstring http::header_names::content_encoding(L"Content-Encoding");
const std::wstring http::header_names::content_language(L"Content-Language");
const std::wstring http::header_names::content_length(L"Content-Length");
const std::wstring http::header_names::content_location(L"Content-Location");
const std::wstring http::header_names::content_md5(L"Content-MD5");
const std::wstring http::header_names::content_range(L"Content-Range");
const std::wstring http::header_names::content_type(L"Content-Type");
const std::wstring http::header_names::date(L"Date");
const std::wstring http::header_names::etag(L"ETag");
const std::wstring http::header_names::expect(L"Expect");
const std::wstring http::header_names::expires(L"Expires");
const std::wstring http::header_names::from(L"From");
const std::wstring http::header_names::host(L"Host");
const std::wstring http::header_names::if_match(L"If-Match");
const std::wstring http::header_names::if_modified_since(L"If-Modified-Since");
const std::wstring http::header_names::if_none_match(L"If-None-Match");
const std::wstring http::header_names::if_range(L"If-Range");
const std::wstring http::header_names::if_unmodified_since(L"If-Unmodified-Since");
const std::wstring http::header_names::last_modified(L"Last-Modified");
const std::wstring http::header_names::location(L"Location");
const std::wstring http::header_names::max_forwards(L"Max-Forwards");
const std::wstring http::header_names::pragma(L"Pragma");
const std::wstring http::header_names::proxy_authenticate(L"Proxy-Authenticate");
const std::wstring http::header_names::proxy_authorization(L"Proxy-Authorization");
const std::wstring http::header_names::range(L"Range");
const std::wstring http::header_names::referer(L"Referer");
const std::wstring http::header_names::retry_after(L"Retry-After");
const std::wstring http::header_names::server(L"Server");
const std::wstring http::header_names::te(L"TE");
const std::wstring http::header_names::trailer(L"Trailer");
const std::wstring http::header_names::transfer_encoding(L"Transfer-Encoding");
const std::wstring http::header_names::upgrade(L"Upgrade");
const std::wstring http::header_names::user_agent(L"User-Agent");
const std::wstring http::header_names::vary(L"Vary");
const std::wstring http::header_names::via(L"Via");
const std::wstring http::header_names::warning(L"Warning");
const std::wstring http::header_names::www_authenticate(L"WWW-Authenticate");


const std::wstring http::mime_types::application_atom_xml(L"application/atom+xml");
const std::wstring http::mime_types::application_http(L"application/http");
const std::wstring http::mime_types::application_javascript(L"application/javascript");
const std::wstring http::mime_types::application_json(L"application/json");
const std::wstring http::mime_types::application_xjson(L"application/x-json");
const std::wstring http::mime_types::application_octetstream(L"application/octet-stream");
const std::wstring http::mime_types::application_x_www_form_urlencoded(L"application/x-www-form-urlencoded");
const std::wstring http::mime_types::application_xjavascript(L"application/x-javascript");
const std::wstring http::mime_types::application_xml(L"application/xml");
const std::wstring http::mime_types::message_http(L"message/http");
const std::wstring http::mime_types::text(L"text");
const std::wstring http::mime_types::text_javascript(L"text/javascript");
const std::wstring http::mime_types::text_json(L"text/json");
const std::wstring http::mime_types::text_csv(L"text/csv");
const std::wstring http::mime_types::text_plain(L"text/plain");
const std::wstring http::mime_types::text_plain_utf16(L"text/plain; charset=utf-16");
const std::wstring http::mime_types::text_plain_utf16le(L"text/plain; charset=utf-16le");
const std::wstring http::mime_types::text_plain_utf8(L"text/plain; charset=utf-8");
const std::wstring http::mime_types::text_xjavascript(L"text/x-javascript");
const std::wstring http::mime_types::text_xjson(L"text/x-json");


const std::wstring http::charset_types::ascii(L"ascii");
const std::wstring http::charset_types::usascii(L"us-ascii");
const std::wstring http::charset_types::latin1(L"iso-8859-1");
const std::wstring http::charset_types::utf8(L"utf-8");
const std::wstring http::charset_types::utf16(L"utf-16");
const std::wstring http::charset_types::utf16le(L"utf-16le");
const std::wstring http::charset_types::utf16be(L"utf-16be");


const std::wstring http::oauth1_methods::hmac_sha1(L"HMAC-SHA1");
const std::wstring http::oauth1_methods::plaintext(L"PLAINTEXT");


const std::wstring http::oauth1_strings::callback(L"oauth_callback");
const std::wstring http::oauth1_strings::callback_confirmed(L"oauth_callback_confirmed");
const std::wstring http::oauth1_strings::consumer_key(L"oauth_consumer_key");
const std::wstring http::oauth1_strings::nonce(L"oauth_nonce");
const std::wstring http::oauth1_strings::realm(L"realm"); // NOTE: No "oauth_" prefix.
const std::wstring http::oauth1_strings::signature(L"oauth_signature");
const std::wstring http::oauth1_strings::signature_method(L"oauth_signature_method");
const std::wstring http::oauth1_strings::timestamp(L"oauth_timestamp");
const std::wstring http::oauth1_strings::token(L"oauth_token");
const std::wstring http::oauth1_strings::token_secret(L"oauth_token_secret");
const std::wstring http::oauth1_strings::verifier(L"oauth_verifier");
const std::wstring http::oauth1_strings::version(L"oauth_version");


const std::wstring http::oauth2_strings::access_token(L"access_token");
const std::wstring http::oauth2_strings::authorization_code(L"authorization_code");
const std::wstring http::oauth2_strings::bearer(L"bearer");
const std::wstring http::oauth2_strings::client_id(L"client_id");
const std::wstring http::oauth2_strings::client_secret(L"client_secret");
const std::wstring http::oauth2_strings::code(L"code");
const std::wstring http::oauth2_strings::expires_in(L"expires_in");
const std::wstring http::oauth2_strings::grant_type(L"grant_type");
const std::wstring http::oauth2_strings::redirect_uri(L"redirect_uri");
const std::wstring http::oauth2_strings::refresh_token(L"refresh_token");
const std::wstring http::oauth2_strings::response_type(L"response_type");
const std::wstring http::oauth2_strings::scope(L"scope");
const std::wstring http::oauth2_strings::state(L"state");
const std::wstring http::oauth2_strings::token(L"token");
const std::wstring http::oauth2_strings::token_type(L"token_type");



namespace http_detail {
class request_context
{
public:
    virtual ~request_context();
    static request_context* create_context(const http::client* c, const http::basic_request* req, http::basic_response* res);
    
    unsigned short winhttp_get_status_code();
    std::wstring winhttp_get_reason_phrase();
    std::vector<std::pair<std::wstring, std::wstring>> winhttp_get_headers();
    
protected:
    request_context();
    request_context(HINTERNET h, const http::basic_request* req, http::basic_response* res);

    void complete_request() noexcept;

    size_t winhttp_query_header_length(unsigned long info_level);

    void on_async_request_error(const WINHTTP_ASYNC_RESULT* result);
    void on_async_send_complete();
    void on_async_write_complete(unsigned long bytes_written);
    void on_async_header_available();
    void on_async_data_available(unsigned long bytes_available);
    void on_async_read_complete(unsigned long bytes_read);

private:
    HINTERNET       _request_handle;
    HANDLE          _complete_event;
    const http::basic_request* _request;
    http::basic_response*      _response;
    std::vector<unsigned char> _read_buf;

    friend class NX::http::client;
};
}

using namespace http_detail;

//
//  http client
//
http::client::client() : _session_handle(NULL), _conn_handle(NULL), _secure(false), _async(false), _timeout(DEFAULT_TIMEOUT_SECONDS)
{
}

// http://username:password@example.com:123
// https://username:password@example.com:123
http::client::client(const std::wstring& url, bool async_call, unsigned long timeout) : _session_handle(NULL), _conn_handle(NULL), _secure(false), _async(async_call), _timeout(timeout)
{
    NX::uri server_uri(url);
    if (!server_uri.empty()) {
        _secure = (0 == _wcsicmp(L"https", server_uri.scheme().c_str())) ? true : false;
        _host = server_uri.host();
        _port = server_uri.port();
        if (_port == 0) {
            _port = secure() ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
        }
    }
}

http::client::~client()
{
    close();
}

void http::client::open()
{
    unsigned long   access_type = WINHTTP_ACCESS_TYPE_NO_PROXY;
    const wchar_t*  proxy_name = WINHTTP_NO_PROXY_NAME;
    bool            callback_is_set = false;

    try {

        // Open session.
        if (nullptr == _session_handle) {

            _session_handle = ::WinHttpOpen(NULL, access_type, proxy_name, WINHTTP_NO_PROXY_BYPASS, async() ? WINHTTP_FLAG_ASYNC : 0);
            if (!_session_handle) {
                throw std::exception("WinHttpOpen failed");
            }

            // Set timeouts.
#ifdef _DEBUG
            const int milliseconds = 1000 * 20; // 20 seconds
#else
            const int milliseconds = 1000 * 3;  // 3 seconds
#endif
            if (!WinHttpSetTimeouts(_session_handle, milliseconds, milliseconds, milliseconds, milliseconds)) {
                throw std::exception("WinHttpSetTimeouts failed");
            }

            if (secure()) {
                const unsigned long protos = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
                if (!::WinHttpSetOption(_session_handle, WINHTTP_OPTION_SECURE_PROTOCOLS, (LPVOID)&protos, sizeof(protos))) {
                    throw std::exception("WinHttpSetOption failed");
                }
            }

            if (async()) {
                // Register asynchronous callback.
                if (WINHTTP_INVALID_STATUS_CALLBACK == WinHttpSetStatusCallback(_session_handle, http::client::completion_callback, WINHTTP_CALLBACK_FLAG_ALL_COMPLETIONS | WINHTTP_CALLBACK_FLAG_HANDLES, 0)) {
                    throw std::exception("WinHttpSetStatusCallback failed");
                }
                callback_is_set = true;
            }

            connect(false);
        }
    }
    catch (const std::exception& e) {

        if (_session_handle != nullptr) {
            // Unregister the callback.
            if (callback_is_set) {
                WinHttpSetStatusCallback(_session_handle, nullptr, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, NULL);
            }
            WinHttpCloseHandle(_session_handle);
            _session_handle = nullptr;
        }
        throw e;
    }
}

void http::client::close()
{
    if (_session_handle != nullptr) {

        disconnect();

        // Unregister the callback.
        if (secure()) {
            WinHttpSetStatusCallback(_session_handle, nullptr, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, NULL);
        }

        // close session handle
        WinHttpCloseHandle(_session_handle);
        _session_handle = nullptr;
    }
}

void http::client::connect(bool reconnect)
{
    assert(nullptr != _session_handle);

    if (nullptr != _conn_handle) {
        if (!reconnect) {
            return;
        }
        WinHttpCloseHandle(_conn_handle);
        _conn_handle = nullptr;
    }

    assert(nullptr == _conn_handle);
    _conn_handle = WinHttpConnect(_session_handle, _host.c_str(), (INTERNET_PORT)_port, 0);
    if (_conn_handle == nullptr) {
        throw std::exception("WinHttpConnect failed");
    }
}

void http::client::disconnect()
{
    if (nullptr != _conn_handle) {
        WinHttpCloseHandle(_conn_handle);
        _conn_handle = nullptr;
    }
}

void http::client::send_request(const basic_request* req, basic_response* res)
{

    try {

        std::shared_ptr<request_context> context = nullptr;

        if (!connected()) {
            open();
        }

        context = std::shared_ptr<request_context>(http_detail::request_context::create_context(this, req, res));
        assert(context != nullptr);

        if (async()) {

            // send asynchronously
            if (!WinHttpSendRequest(context->_request_handle,
                                    WINHTTP_NO_ADDITIONAL_HEADERS,
                                    0,
                                    nullptr,
                                    0,
                                    (DWORD)req->get_data_length(),
                                    (DWORD_PTR)context.get())) {
                throw std::exception("WinHttpSendRequest (async) failed");
            }

            unsigned long wait_result = ::WaitForSingleObject(context->_complete_event, _timeout * 1000);
            if (WAIT_OBJECT_0 != wait_result) {
                DWORD wait_error = 0;
                switch (wait_result)
                {
                case WAIT_ABANDONED:
                    wait_error = ERROR_CANCELLED;
                    break;
                case WAIT_TIMEOUT:
                    wait_error = ERROR_TIMEOUT;
                    break;
                case WAIT_FAILED:
                default:
                    wait_error = GetLastError();
                    break;
                }
                throw std::exception("WaitForSingleObject (wait for WinHttpSendRequest async) failed");
            }
        }
        else {

            send_request_sync(res, context.get());
        }

        // finished
        context.reset();
    }
    catch (const std::exception& e) {
        throw e;
    }
}

void http::client::send_request_sync(basic_response* res, void* context)
{
    request_context* rc = reinterpret_cast<request_context*>(context);

    // send synchronized
    if (!WinHttpSendRequest(rc->_request_handle,
                            WINHTTP_NO_ADDITIONAL_HEADERS,
                            0,
                            nullptr,
                            0,
                            (DWORD)rc->_request->get_data_length(),
                            0)) {
        throw std::exception("WinHttpSendRequest (sync) failed");
    }

    if (rc->_request->get_data_length() > 0) {

        // send data
        do {

            size_t bytes_read = http::max_cache_size;
            unsigned long bytes_written = 0;

            const unsigned char* input = rc->_request->get_data(bytes_read);
            if(input== nullptr || 0 == bytes_read) {
                break;
            }
            if (!WinHttpWriteData(rc->_request_handle, input, (unsigned long)bytes_read, &bytes_written)) {
                throw std::exception("WinHttpWriteData failed");
            }
        } while (true);
    }

    // now try to get response
    if (!WinHttpReceiveResponse(rc->_request_handle, nullptr)) {
        throw std::exception("WinHttpReceiveResponse failed");
    }

    // get header
    res->set_status_code(rc->winhttp_get_status_code());
    res->set_reason_phrase(rc->winhttp_get_reason_phrase());
    res->set_headers(rc->winhttp_get_headers());

    if (0 == _wcsicmp(rc->_request->method().c_str(), http::methods::HEAD.c_str())) {
        // finished
        return;
    }

    // read response data
    while(true) {

        unsigned long bytes_to_read = 0;
        unsigned long bytes_read = 0;
        std::vector<unsigned char> output;


        if (!WinHttpQueryDataAvailable(rc->_request_handle, &bytes_to_read)) {
            throw std::exception("WinHttpQueryDataAvailable failed");
        }

        // No more data.
        if (0 == bytes_to_read) {
            break;
        }

        output.resize(bytes_to_read + 1, 0);
        if (!WinHttpReadData(rc->_request_handle, (LPVOID)output.data(), bytes_to_read, &bytes_read)) {
            throw std::exception("WinHttpReadData failed");
        }

        if (bytes_read == 0) {
            // no data is read
            break;
        }
        
        res->write_data(output.data(), bytes_read);
    }

    // all the data has been written
    res->complete_write();
}

void http::client::send_request_async(basic_response* res, void* context)
{
    request_context* rc = reinterpret_cast<request_context*>(context);

    // send asynchronously
    if (!WinHttpSendRequest(rc->_request_handle,
                            WINHTTP_NO_ADDITIONAL_HEADERS,
                            0,
                            nullptr,
                            0,
                            (DWORD)rc->_request->get_data_length(),
                            (DWORD_PTR)context)) {
        throw std::exception("WinHttpSendRequest (async) failed");
    }
}

// Callback used with WinHTTP to listen for async completions.
void CALLBACK http::client::completion_callback(HANDLE request_handle, ULONG_PTR context, unsigned long status_code, void* status_info, unsigned long status_info_length)
{
    UNREFERENCED_PARAMETER(status_info_length);

    if (status_code == WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING)
        return;

    request_context* rc = reinterpret_cast<request_context*>(context);
    if (rc == nullptr) {
        return;
    }

    try {

        switch (status_code)
        {
        case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
            rc->on_async_request_error(reinterpret_cast<const WINHTTP_ASYNC_RESULT*>(status_info));
            break;

        case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
            rc->on_async_send_complete();
            break;

        case WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE:
            assert(status_info_length == sizeof(unsigned long));
            rc->on_async_write_complete(*((unsigned long*)status_info));
            break;

        case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
            rc->on_async_header_available();
            break;
            {
                break;
            }
        case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE:
            assert(status_info_length == sizeof(unsigned long));
            rc->on_async_data_available(status_info_length);
            break;

        case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
            rc->on_async_read_complete(status_info_length);
            break;

        default:
            break;
        }
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        rc->complete_request();
    }
}


//
//  class basic_request
//

http::basic_request::basic_request()
{
}

http::basic_request::basic_request(const std::wstring& request_method,
    const std::wstring& request_path,
    const std::vector<std::pair<std::wstring, std::wstring>>& request_headers,
    const std::vector<LPCWSTR>& accepted_types)
    : _method(request_method), _path(request_path), _headers(request_headers), _accepted_types(accepted_types)
{
    if (!_accepted_types.empty()) {
        if (NULL != _accepted_types[_accepted_types.size() - 1]) {
            _accepted_types.push_back(NULL);    // must end with NULL
        }
    }
}

http::basic_request::~basic_request()
{
}

void http::basic_request::add_header(const std::wstring& name, const std::wstring& value, HEADER_ADD_METHOD add_method)
{
    auto pos = std::find_if(_headers.begin(), _headers.end(), [&](const std::pair<std::wstring, std::wstring>& header) -> bool {
        return (0 == _wcsicmp(name.c_str(), header.first.c_str()));
    });
    if (pos == _headers.end()) {
        _headers.push_back(std::pair<std::wstring, std::wstring>(name, value));
    }
    else {
        switch (add_method)
        {
        case HEADER_NEW:
            break;
        case HEADER_MERGE:
            if (!(*pos).second.empty()) {
                (*pos).second += L" ";
            }
            (*pos).second += value;
            break;
        case HEADER_MERGE_WITH_COMMA:
            if (!(*pos).second.empty()) {
                (*pos).second += L", ";
            }
            (*pos).second += value;
            break;
        case HEADER_MERGE_WITH_SEMICOLON:
            if (!(*pos).second.empty()) {
                (*pos).second += L"; ";
            }
            (*pos).second += value;
            break;
        case HEADER_REPLACE:
        default:
            (*pos).second = value;
            break;
        }
        if (add_method) {
            if (!(*pos).second.empty()) {
                (*pos).second += L"; ";
            }
            (*pos).second += value;
        }
        else {
            (*pos).second = value;
        }
    }
}

void http::basic_request::remove_header(const std::wstring& name)
{
    auto pos = std::find_if(_headers.begin(), _headers.end(), [&](const std::pair<std::wstring, std::wstring>& header) -> bool {
        return (0 == _wcsicmp(name.c_str(), header.first.c_str()));
    });
    if (pos != _headers.end()) {
        _headers.erase(pos);
    }
}


//
//  class raw_request
//

http::raw_request::raw_request(const std::wstring& request_method,
    const std::wstring& request_path,
    const std::vector<std::pair<std::wstring, std::wstring>>& request_headers,
    const std::vector<LPCWSTR>& accepted_types,
    const std::vector<unsigned char>& body) : basic_request(request_method, request_path, request_headers, accepted_types), _body(body), _pos(0)
{
}

http::raw_request::~raw_request()
{
}

size_t http::raw_request::get_data_length() const
{
    return _body.size();
}

const unsigned char* http::raw_request::get_data(size_t& length) const
{
    const unsigned char* p = nullptr;
    const size_t in_size = length;

    if (_pos < get_data_length()) {

        const size_t bytes_left = get_data_length() - _pos;
        if (length > bytes_left) {
            length = bytes_left;
        }

        if (length > 0) {
            p = _body.data() + _pos;
            _pos += length;
        }
    }
    else {
        length = 0;
    }

    return p;
}


//
//  class string_request
//

http::string_request::string_request(const std::wstring& request_method,
    const std::wstring& request_path,
    const std::vector<std::pair<std::wstring, std::wstring>>& request_headers,
    const std::vector<LPCWSTR>& accepted_types,
    const std::string& s) : basic_request(request_method, request_path, request_headers, accepted_types), _body(s), _pos(0)
{
}

http::string_request::string_request(const std::wstring& request_method,
    const std::wstring& request_path,
    const std::vector<std::pair<std::wstring, std::wstring>>& request_headers,
    const std::vector<LPCWSTR>& accepted_types,
    const std::wstring& s) : basic_request(request_method, request_path, request_headers, accepted_types), _body(nudf::util::convert::Utf16ToUtf8(s)), _pos(0)
{
}

http::string_request::~string_request()
{
}

size_t http::string_request::get_data_length() const
{
    return _body.length();
}

const unsigned char* http::string_request::get_data(size_t& length) const
{
    const unsigned char* p = nullptr;
    const size_t in_size = length;

    if (_pos < get_data_length()) {

        const size_t bytes_left = get_data_length() - _pos;
        if (length > bytes_left) {
            length = bytes_left;
        }

        if (length > 0) {
            p = (const unsigned char*)_body.c_str() + _pos;
            _pos += length;
        }
    }
    else {
        length = 0;
    }

    return p;
}


//
//  class string_response
//

http::string_response::string_response() : http::basic_response()
{
}

http::string_response::~string_response()
{
}

void http::string_response::write_data(const unsigned char* data, const size_t length)
{
    std::for_each(data, data + length, [&](const unsigned char& c) {
        _body.push_back(c);
    });
}

void http::string_response::complete_write()
{
}

std::wstring http::string_response::to_utf16() const
{
    return std::move(nudf::util::convert::Utf8ToUtf16(_body));
}


//
//  http request context
//

http_detail::request_context::request_context() : _request_handle(nullptr), _complete_event(::CreateEventW(nullptr, TRUE, FALSE, nullptr)), _request(nullptr), _response(nullptr)
{
}

http_detail::request_context::request_context(HINTERNET h, const http::basic_request* req, http::basic_response* res) : _request_handle(h), _complete_event(::CreateEventW(nullptr, TRUE, FALSE, nullptr)),_request(req), _response(res)
{
}

http_detail::request_context::~request_context()
{
    if (nullptr != _request_handle) {
        WinHttpCloseHandle(_request_handle);
        _request_handle = nullptr;
    }
    if (nullptr != _complete_event) {
        CloseHandle(_complete_event);
        _complete_event = nullptr;
    }
}

request_context* http_detail::request_context::create_context(const http::client* c, const http::basic_request* req, http::basic_response* res)
{
    request_context* p = nullptr;
    HINTERNET        rh = nullptr;

    try {

        rh = WinHttpOpenRequest(c->conn_handle(),
                                req->method().c_str(),
                                req->path().c_str(),
                                nullptr,
                                WINHTTP_NO_REFERER,
                                (LPCWSTR*)req->accepted_types(),
                                WINHTTP_FLAG_ESCAPE_DISABLE | (c->secure() ? WINHTTP_FLAG_SECURE : 0));
        if (nullptr == rh) {
            throw std::exception("WinHttpOpenRequest failed");
        }

        static const unsigned long data = SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID | SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;
        if (!WinHttpSetOption(rh, WINHTTP_OPTION_SECURITY_FLAGS, (LPVOID)&data, sizeof(data))) {
            throw std::exception("WinHttpSetOption failed");
        }

        if (!req->headers().empty()) {

            std::for_each(req->headers().begin(), req->headers().end(), [&](const std::pair<std::wstring, std::wstring>& item) {
                std::wstring header(item.first);
                header += L": ";
                header += item.second;
                if (!WinHttpAddRequestHeaders(rh, header.c_str(), static_cast<DWORD>(header.length()), WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE)) {
                    throw std::exception("WinHttpAddRequestHeaders failed");
                }
            });
        }

        p = new request_context(rh, req, res);
        rh = nullptr;   // transfer owner
    }
    catch (const std::exception& e) {

        if (nullptr != rh) {
            WinHttpCloseHandle(rh);
            rh = nullptr;
        }

        throw e;
    }

    assert(nullptr != p);
    return p;
}

void http_detail::request_context::complete_request() noexcept
{
    if (nullptr != _request_handle) {
        WinHttpCloseHandle(_request_handle);
        _request_handle = nullptr;
    }

    _response->complete_write();

    SetEvent(_complete_event);
}

size_t http_detail::request_context::winhttp_query_header_length(unsigned long info_level)
{
    unsigned long length = 0;
    if (!WinHttpQueryHeaders(_request_handle,
                             info_level,
                             WINHTTP_HEADER_NAME_BY_INDEX,
                             WINHTTP_NO_OUTPUT_BUFFER,
                             &length,
                             WINHTTP_NO_HEADER_INDEX)) {
        if (ERROR_INSUFFICIENT_BUFFER != GetLastError()) {
            throw std::exception("WinHttpQueryHeaders failed");
        }
    }
    return length;
}

unsigned short http_detail::request_context::winhttp_get_status_code()
{
    unsigned long length = (unsigned long)winhttp_query_header_length(WINHTTP_QUERY_STATUS_CODE);
    std::wstring buffer;
    buffer.resize(length);
    if (!WinHttpQueryHeaders(_request_handle, WINHTTP_QUERY_STATUS_CODE, WINHTTP_HEADER_NAME_BY_INDEX, &buffer[0], &length, WINHTTP_NO_HEADER_INDEX)) {
        throw std::exception("WinHttpQueryHeaders failed");
    }
    return (unsigned short)_wtoi(buffer.c_str());
}

std::wstring http_detail::request_context::winhttp_get_reason_phrase()
{
    std::wstring phrase;
    unsigned long length = 0;

    length = (unsigned long)winhttp_query_header_length(WINHTTP_QUERY_STATUS_TEXT);
    phrase.resize(length);
    if (!WinHttpQueryHeaders(_request_handle, WINHTTP_QUERY_STATUS_TEXT, WINHTTP_HEADER_NAME_BY_INDEX, &phrase[0], &length, WINHTTP_NO_HEADER_INDEX)) {
        throw std::exception("WinHttpQueryHeaders failed");
    }
    // WinHTTP reports back the wrong length, trim any null characters.
    boost::algorithm::trim(phrase);
    return phrase;
}

std::vector<std::pair<std::wstring, std::wstring>> http_detail::request_context::winhttp_get_headers()
{
    std::vector<std::pair<std::wstring, std::wstring>> header_list;

    unsigned short response_code = 0;

    // First need to query to see what the headers size is.
    unsigned long header_buffer_length = 0;
    header_buffer_length = (unsigned long)winhttp_query_header_length(WINHTTP_QUERY_RAW_HEADERS_CRLF);

    // Now allocate buffer for headers and query for them.
    std::vector<wchar_t> header_raw_buffer;
    header_raw_buffer.resize((header_buffer_length + 1) / 2 + 1, 0);
    if (!WinHttpQueryHeaders(_request_handle,
                             WINHTTP_QUERY_RAW_HEADERS_CRLF,
                             WINHTTP_HEADER_NAME_BY_INDEX,
                             header_raw_buffer.data(),
                             &header_buffer_length,
                             WINHTTP_NO_HEADER_INDEX)) {
        throw std::exception("WinHttpQueryHeaders failed");
    }

    if (header_buffer_length == 0) {
        return std::move(header_list);
    }

    const wchar_t* p = header_raw_buffer.data();
    while (p != nullptr && *p != L'\r' && *p != L'\n') {

        std::wstring line;
        const wchar_t* pe = wcschr(p, L'\n');


        if (pe == nullptr) {
            line = p;
            p = nullptr;
        }
        else {
            line = std::wstring(p, (L'\r' == *(pe - 1)) ? (pe - 1) : pe);
            p = pe + 1;
        }

        const size_t pos = line.find(L":");
        if (pos == std::wstring::npos) {
            continue;
        }

        std::wstring key = line.substr(0, pos);
        std::wstring value = line.substr(pos + 1);
        boost::algorithm::trim(key);
        boost::algorithm::trim(value);
        header_list.push_back(std::pair<std::wstring, std::wstring>(key, value));
    }

    return std::move(header_list);
}


void http_detail::request_context::on_async_request_error(const WINHTTP_ASYNC_RESULT* result)
{
    switch (result->dwResult)
    {
    case API_RECEIVE_RESPONSE:
        throw std::exception("on_async_request_error: WinHttpReceiveResponse");
    case API_QUERY_DATA_AVAILABLE:
        throw std::exception("WinHttpQueryDataAvaliable");
    case API_READ_DATA:
        throw std::exception("on_async_request_error: WinHttpReadData");
    case API_WRITE_DATA:
        throw std::exception("on_async_request_error: WinHttpWriteData");
    case API_SEND_REQUEST:
        throw std::exception("on_async_request_error: WinHttpSendRequest");
    default:
        throw std::exception("on_async_request_error: Unknown WinHTTP Function");
    }
}

void http_detail::request_context::on_async_send_complete()
{

    if (_request->get_data_length() > 0) {

        // send data
        size_t bytes_read = http::max_cache_size;
        unsigned long bytes_written = 0;

        const unsigned char* input = _request->get_data(bytes_read);
        if (input == nullptr || 0 == bytes_read) {
            throw std::exception("on_async_send_complete: invalid data");
        }

        if (!WinHttpWriteData(_request_handle, input, (unsigned long)bytes_read, &bytes_written)) {
            throw std::exception("on_async_send_complete: WinHttpWriteData failed");
        }
    }
    else {
        on_async_write_complete(0);
    }
}

void http_detail::request_context::on_async_write_complete(unsigned long bytes_written)
{
    if (0 != bytes_written) {

        // maybe there are left data
        size_t bytes_read = http::max_cache_size;

        const unsigned char* input = _request->get_data(bytes_read);
        if (input == nullptr || 0 == bytes_read) {
            bytes_written = 0;
        }

        if (0 != bytes_written) {
            if (!WinHttpWriteData(_request_handle, input, (unsigned long)bytes_read, &bytes_written)) {
                throw std::exception("on_async_write_complete: WinHttpWriteData failed");
            }
        }
    }

    // final
    if (0 == bytes_written) {
        // all the data has been sent
        if (!WinHttpReceiveResponse(_request_handle, nullptr)) {
            throw std::exception("on_async_write_complete: WinHttpReceiveResponse failed");
        }
    }
}

void http_detail::request_context::on_async_header_available()
{
    _response->set_status_code(winhttp_get_status_code());
    _response->set_reason_phrase(winhttp_get_reason_phrase());
    _response->set_headers(winhttp_get_headers());

    if (0 == _wcsicmp(_request->method().c_str(), http::methods::HEAD.c_str())) {
        // done
        complete_request();
        return;
    }

    if (!WinHttpQueryDataAvailable(_request_handle, nullptr)) {
        throw std::exception("on_async_header_available: WinHttpQueryDataAvailable failed");
    }
}

void http_detail::request_context::on_async_data_available(unsigned long bytes_available)
{
    // No more data.
    if (0 == bytes_available) {
        complete_request();
        return;
    }

    _read_buf.resize(bytes_available + 1, 0);
    if (!WinHttpReadData(_request_handle, (LPVOID)_read_buf.data(), bytes_available, nullptr)) {
        throw std::exception("on_async_data_available: WinHttpReadData failed");
    }
}

void http_detail::request_context::on_async_read_complete(unsigned long bytes_read)
{
    // No more data.
    if (0 == bytes_read) {
        complete_request();
        return;
    }

    // write data to response
    _response->write_data(_read_buf.data(), bytes_read);

    _read_buf.resize(http::max_cache_size, 0);
    if (!WinHttpReadData(_request_handle, (LPVOID)_read_buf.data(), (unsigned long)http::max_cache_size, nullptr)) {
        throw std::exception("on_async_read_complete: WinHttpReadData failed");
    }
}
