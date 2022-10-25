

#pragma once
#ifndef __NUDF_HTTP_CLIENT_HPP__
#define __NUDF_HTTP_CLIENT_HPP__

#include <winhttp.h>
#include <string>
#include <vector>
#include <map>
//#include <function>

#include <nudf\security.hpp>
#include <nudf\uri.hpp>

namespace NX {

namespace http {

    
namespace message_direction
{
    enum direction {
        upload,
        download
    };
}

//typedef std::function<void(message_direction::direction, unsigned __int64)> progress_handler;

class methods
{
public:
    const static std::wstring GET;
    const static std::wstring POST;
    const static std::wstring PUT;
    const static std::wstring DEL;
    const static std::wstring HEAD;
    const static std::wstring OPTIONS;
    const static std::wstring TRCE;
    const static std::wstring CONNECT;
    const static std::wstring MERGE;
    const static std::wstring PATCH;
};

class status_codes
{
public:
    const static unsigned short Continue               = 100;
    const static unsigned short SwitchingProtocols     = 101;
    const static unsigned short OK                     = 200;
    const static unsigned short Created                = 201;
    const static unsigned short Accepted               = 202;
    const static unsigned short NonAuthInfo            = 203;
    const static unsigned short NoContent              = 204;
    const static unsigned short ResetContent           = 205;
    const static unsigned short PartialContent         = 206;
    const static unsigned short MultipleChoices        = 300;
    const static unsigned short MovedPermanently       = 301;
    const static unsigned short Found                  = 302;
    const static unsigned short SeeOther               = 303;
    const static unsigned short NotModified            = 304;
    const static unsigned short UseProxy               = 305;
    const static unsigned short TemporaryRedirect      = 307;
    const static unsigned short BadRequest             = 400;
    const static unsigned short Unauthorized           = 401;
    const static unsigned short PaymentRequired        = 402;
    const static unsigned short Forbidden              = 403;
    const static unsigned short NotFound               = 404;
    const static unsigned short MethodNotAllowed       = 405;
    const static unsigned short NotAcceptable          = 406;
    const static unsigned short ProxyAuthRequired      = 407;
    const static unsigned short RequestTimeout         = 408;
    const static unsigned short Conflict               = 409;
    const static unsigned short Gone                   = 410;
    const static unsigned short LengthRequired         = 411;
    const static unsigned short PreconditionFailed     = 412;
    const static unsigned short RequestEntityTooLarge  = 413;
    const static unsigned short RequestUriTooLarge     = 414;
    const static unsigned short UnsupportedMediaType   = 415;
    const static unsigned short RangeNotSatisfiable    = 416;
    const static unsigned short ExpectationFailed      = 417;
    const static unsigned short InternalError          = 500;
    const static unsigned short NotImplemented         = 501;
    const static unsigned short BadGateway             = 502;
    const static unsigned short ServiceUnavailable     = 503;
    const static unsigned short GatewayTimeout         = 504;
    const static unsigned short HttpVersionNotSupported= 505;
};

struct http_status_to_phrase
{
    unsigned short  id;
    std::wstring    phrase;
};

class header_names
{
public:
    const static std::wstring accept;
    const static std::wstring accept_charset;
    const static std::wstring accept_encoding;
    const static std::wstring accept_language;
    const static std::wstring accept_ranges;
    const static std::wstring age;
    const static std::wstring allow;
    const static std::wstring authorization;
    const static std::wstring cache_control;
    const static std::wstring connection;
    const static std::wstring content_encoding;
    const static std::wstring content_language;
    const static std::wstring content_length;
    const static std::wstring content_location;
    const static std::wstring content_md5;
    const static std::wstring content_range;
    const static std::wstring content_type;
    const static std::wstring date;
    const static std::wstring etag;
    const static std::wstring expect;
    const static std::wstring expires;
    const static std::wstring from;
    const static std::wstring host;
    const static std::wstring if_match;
    const static std::wstring if_modified_since;
    const static std::wstring if_none_match;
    const static std::wstring if_range;
    const static std::wstring if_unmodified_since;
    const static std::wstring last_modified;
    const static std::wstring location;
    const static std::wstring max_forwards;
    const static std::wstring pragma;
    const static std::wstring proxy_authenticate;
    const static std::wstring proxy_authorization;
    const static std::wstring range;
    const static std::wstring referer;
    const static std::wstring retry_after;
    const static std::wstring server;
    const static std::wstring te;
    const static std::wstring trailer;
    const static std::wstring transfer_encoding;
    const static std::wstring upgrade;
    const static std::wstring user_agent;
    const static std::wstring vary;
    const static std::wstring via;
    const static std::wstring warning;
    const static std::wstring www_authenticate;
};


class mime_types
{
public:
    const static std::wstring application_atom_xml;
    const static std::wstring application_http;
    const static std::wstring application_javascript;
    const static std::wstring application_json;
    const static std::wstring application_xjson;
    const static std::wstring application_octetstream;
    const static std::wstring application_x_www_form_urlencoded;
    const static std::wstring application_xjavascript;
    const static std::wstring application_xml;
    const static std::wstring message_http;
    const static std::wstring text;
    const static std::wstring text_javascript;
    const static std::wstring text_json;
    const static std::wstring text_plain;
    const static std::wstring text_plain_utf16;
    const static std::wstring text_plain_utf16le;
    const static std::wstring text_plain_utf8;
    const static std::wstring text_xjavascript;
    const static std::wstring text_xjson;
};

class charset_types
{
public:
    const static std::wstring ascii;
    const static std::wstring usascii;
    const static std::wstring latin1;
    const static std::wstring utf8;
    const static std::wstring utf16;
    const static std::wstring utf16le;
    const static std::wstring utf16be;
};

typedef std::wstring oauth1_method;
class oauth1_methods
{
public:
    const static std::wstring hmac_sha1;
    const static std::wstring plaintext;
};

class oauth1_strings
{
public:
    const static std::wstring callback;
    const static std::wstring callback_confirmed;
    const static std::wstring consumer_key;
    const static std::wstring nonce;
    const static std::wstring realm;
    const static std::wstring signature;
    const static std::wstring signature_method;
    const static std::wstring timestamp;
    const static std::wstring token;
    const static std::wstring token_secret;
    const static std::wstring verifier;
    const static std::wstring version;
};

class oauth2_strings
{
public:
    const static std::wstring access_token;
    const static std::wstring authorization_code;
    const static std::wstring bearer;
    const static std::wstring client_id;
    const static std::wstring client_secret;
    const static std::wstring code;
    const static std::wstring expires_in;
    const static std::wstring grant_type;
    const static std::wstring redirect_uri;
    const static std::wstring refresh_token;
    const static std::wstring response_type;
    const static std::wstring scope;
    const static std::wstring state;
    const static std::wstring token;
    const static std::wstring token_type;
};

static const size_t max_cache_size = 16384; // 16 KB

class credential
{
public:
    credential() {}
    credential(const std::wstring& name, const std::wstring& password) : _name(name), _password(password) {}
    ~credential() {}

    credential& operator = (const credential& other)
    {
        if (this != &other) {
            _name = other.name();
            _password.encrypt(other.password());
        }
        return *this;
    }

    inline bool empty() const { return _name.empty(); }
    inline void clear() { _name.clear(); _password.clear(); }
    inline const std::wstring& name() const { return _name; }
    inline std::wstring password() const { return _password.decrypt_string(); }

private:
    std::wstring _name;
    secure::wsstring _password;
};

class basic_request
{
public:
    basic_request();
    basic_request(const std::wstring& request_method,
        const std::wstring& request_path,
        const std::vector<std::pair<std::wstring, std::wstring>>& request_headers,
        const std::vector<LPCWSTR>& accepted_types = std::vector<LPCWSTR>());
    virtual ~basic_request();
    
    inline bool empty() const { return _method.empty(); }
    inline const std::wstring& method() const { return _method; }
    inline const std::wstring& path() const { return _path; }
    inline const LPCWSTR* accepted_types() const { return (_accepted_types.empty() ? WINHTTP_DEFAULT_ACCEPT_TYPES : _accepted_types.data()); }
    inline const std::vector<std::pair<std::wstring, std::wstring>>& headers() const { return _headers; }

    typedef enum HEADER_ADD_METHOD {
        HEADER_NEW = 0,
        HEADER_REPLACE,
        HEADER_MERGE,
        HEADER_MERGE_WITH_COMMA,
        HEADER_MERGE_WITH_SEMICOLON
    } HEADER_ADD_METHOD;

    void add_header(const std::wstring& name, const std::wstring& value, HEADER_ADD_METHOD add_method);
    void remove_header(const std::wstring& name);
    
    virtual size_t get_data_length() const = 0;
    virtual std::vector<unsigned char> get_data(size_t length) const = 0;

private:
    // No copy/move allowed
    basic_request& operator = (const basic_request& other) { return *this; }
    basic_request& operator = (basic_request&& other) { return *this; }

private:
    std::wstring    _method;
    std::wstring    _path;
    std::vector<std::pair<std::wstring, std::wstring>>  _headers;
    std::vector<LPCWSTR> _accepted_types;
};

class basic_response
{
public:
    basic_response() : _status_code(http::status_codes::NotFound) {}
    virtual ~basic_response() {}

    virtual void write_data(const unsigned char* data, const size_t length) = 0;
    virtual void complete_write() = 0;

    inline void set_status_code(unsigned short code) { _status_code = code; }
    inline void set_reason_phrase(const std::wstring& phrase) { _reason_phrase = phrase; }
    inline void set_headers(const std::vector<std::pair<std::wstring, std::wstring>>& header_list) { _headers = header_list; }

    inline unsigned short status_code() { return _status_code; }
    inline const std::wstring& reason_phrase() { return _reason_phrase; }
    inline const std::vector<std::pair<std::wstring, std::wstring>>& headers() { return _headers; }

private:
    // No copy/move allowed
    basic_response& operator = (const basic_response& other) { return *this; }
    basic_response& operator = (basic_response&& other) { return *this; }

private:
    unsigned short  _status_code;
    std::wstring    _reason_phrase;
    std::vector<std::pair<std::wstring, std::wstring>> _headers;
};

class string_request : public basic_request
{
public:
    string_request(const std::wstring& request_method,
        const std::wstring& request_path,
        const std::vector<std::pair<std::wstring, std::wstring>>& request_headers,
        const std::string& s);
    string_request(const std::wstring& request_method,
        const std::wstring& request_path,
        const std::vector<std::pair<std::wstring, std::wstring>>& request_headers, 
        const std::wstring& s);
    virtual ~string_request();

    virtual size_t get_data_length() const;
    virtual std::vector<unsigned char> get_data(size_t length) const;

private:
    std::string _body;
    mutable std::string::size_type _pos;
};

class string_response : public basic_response
{
public:
    string_response();
    virtual ~string_response();
    
    virtual void write_data(const unsigned char* data, const size_t length);
    virtual void complete_write();

    inline const std::string& body() const { return _body; }
    std::wstring to_utf16() const;

private:
    std::string _body;
};

#ifdef _DEBUG
#define DEFAULT_TIMEOUT_SECONDS 20
#else
#define DEFAULT_TIMEOUT_SECONDS 3
#endif

class client
{
public:
    client();
    // http://username:password@example.com:123
    // https://username:password@example.com:123
    client(const std::wstring& url, bool async_call = false, unsigned long timeout = DEFAULT_TIMEOUT_SECONDS);
    virtual ~client();

    inline HINTERNET conn_handle() const { return _conn_handle; }
    inline bool sync() const { return !_async; }
    inline bool async() const { return _async; }
    inline bool secure() const { return _secure; }


    void send_request(const basic_request* req, basic_response* res);

    // Callback used with WinHTTP to listen for async completions.
    static void CALLBACK completion_callback(HANDLE request_handle, ULONG_PTR context, unsigned long status_code, void* status_info, unsigned long status_info_length);

protected:
    void open();
    void close();
    void connect(bool reconnect);
    void disconnect();

    void send_request_sync(basic_response* res, void* context);
    void send_request_async(basic_response* res, void* context);

    inline HINTERNET session_handle() const { return _session_handle; }
    inline bool opened() const { return (nullptr != _session_handle); }
    inline bool connected() const { return (nullptr != _session_handle && nullptr != _conn_handle); }
    
private:
    HINTERNET       _session_handle;
    HINTERNET       _conn_handle;
    bool            _async;
    bool            _secure;
    unsigned long   _timeout;
    credential      _cred;
    std::wstring    _host;
    unsigned short  _port;
};




}

}


#endif