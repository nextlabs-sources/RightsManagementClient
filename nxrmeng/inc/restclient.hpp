

#ifndef __NX_REST_CLIENT_HPP
#define __NX_REST_CLIENT_HPP


#include <string>
#include <memory>
#include <map>

#include <nudf\web\http_client.hpp>

namespace NX {
namespace rest {

namespace details {

class session_key
{
public:
    session_key();
    ~session_key();

    inline size_t size() const { return _key.size(); }
    inline const unsigned char* key() const { return _key.data(); }
    inline const std::wstring& base64_key() const { return _base64_key; }

private:
    std::vector<unsigned char>  _key;
    std::wstring                _base64_key;
};

class request
{
public:
    request();
    virtual ~request();

    inline const std::wstring& service_name() const {return _service_name;}
    inline const std::wstring& server() const {return _rms_server;}
    inline void set_service_name(const std::wstring& name) {_service_name += name;}

    inline const NX::web::http::http_request& get_request() const { return _request; }
    inline NX::web::http::http_request& get_request() { return _request; }

    void set_data(const std::string& s) noexcept { _data = s; }
    void set_data(const std::wstring& s) noexcept;

protected:
    std::wstring    _rms_server;
    std::wstring    _service_name;
    NX::rest::details::session_key _session_key;
    NX::web::http::http_request _request;

private:
    std::string     _data;
};

class request2 : public request
{
public:
    request2();
    virtual ~request2();

    const std::wstring& agent_id() const { return _agent_id; }

private:
    std::wstring    _agent_id;
};

class response
{
public:
    response();
    virtual ~response();

    virtual void set_response(const std::wstring& body) { _data = body; }
    virtual void clear() noexcept { _data.clear(); }

    inline const std::wstring& data() const noexcept { return _data; }
    inline bool empty() const noexcept { return _data.empty(); }

private:
    std::wstring _data;
};
class client
{
public:
    client();
    virtual ~client();
    virtual void request(const NX::rest::details::request* req, NX::rest::details::response* res) noexcept;

protected:
    NX::web::http::client::http_client_config   _config;
    std::shared_ptr<NX::web::http::client::http_client> _client;
};
}   // namespace NX::rest::details


class rmclient : public NX::rest::details::client
{
public:
    rmclient();
    rmclient(const std::wstring& server);
    virtual ~rmclient();
};

class request_register : public NX::rest::details::request
{
public:
    request_register();
    virtual ~request_register();
};

class response_register : public NX::rest::details::response
{
public:
    response_register() {}
    virtual ~response_register() {}
};

class request_heartbeat : public NX::rest::details::request2
{
public:
    request_heartbeat() {}
    virtual ~request_heartbeat() {}
};

class response_heartbeat : public NX::rest::details::response
{
public:
    response_heartbeat() {}
    virtual ~response_heartbeat() {}
};

class request_audit : public NX::rest::details::request2
{
public:
    request_audit() {}
    virtual ~request_audit() {}
};

class response_audit : public NX::rest::details::response
{
public:
    response_audit() {}
    virtual ~response_audit() {}
};

class request_authn : public NX::rest::details::request2
{
public:
    request_authn();
    request_authn(const std::wstring& user_name, const std::wstring& domain_name, const std::wstring& password);
    virtual ~request_authn();
};

class response_authn : public NX::rest::details::response
{
public:
    response_authn();
    virtual ~response_authn();

    virtual void set_response(const std::wstring& body);
    virtual void clear() noexcept;

    inline long result() const { return _result; }
    inline const std::wstring& user_id() const { return _user_id; }
    inline const std::map<std::wstring, std::wstring>& user_attributes() const { return _user_attrs; }

private:
    long                                 _result;
    std::map<std::wstring, std::wstring> _user_attrs;
    std::wstring                         _user_id;
};

}   // namespace NX::rest
}   // namespace NX



#endif