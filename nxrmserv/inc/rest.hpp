

#ifndef __NXSERV_REST_HPP__
#define __NXSERV_REST_HPP__



#include <string>
#include <vector>
#include <memory>
#include <map>

#include <nudf\http_client.hpp>

namespace NX {

class rest_heartbeat_result;
class rest_authn_result;
class rest_checkupdate_result;

class rest_client
{
public:
    rest_client();
    ~rest_client();

    bool register_agent(NX::agent_info& _agent_info);
    bool request_heartbeat(rest_heartbeat_result& hbresult);
    bool send_acknowledge();
    bool request_authn(const std::wstring& user_name, const std::wstring& domain_name, const std::wstring& password, rest_authn_result& authnresult);
    bool send_audit_log(const std::string& s);
    bool request_eval(const NX::EVAL::eval_object& eo, NX::EVAL::eval_result* evalresult);
    bool check_update(rest_checkupdate_result& updateresult);

protected:
    bool init();
    void clear();
    std::wstring get_session_key();
    NX::http::client* create_client(const std::wstring& server_url);
    void send_request(const NX::http::basic_request* req, NX::http::basic_response* res);

private:
    rest_client(const rest_client& other) {}
    rest_client& operator = (const rest_client& other) { return *this; }

private:
    NX::http::client* _client;
    CRITICAL_SECTION _lock;
    std::wstring _service_base;
    std::wstring _cert;
    std::wstring _agent_name;
    bool _inactive;
};

class rest_heartbeat_result
{
public:
    rest_heartbeat_result() : _forced(false) {}
    ~rest_heartbeat_result() {}

    inline const agent_info& info() const noexcept { return _agent_info; }
    inline const NX::web::json::value& keys_bundle() const noexcept { return _keys_bundle; }
    inline const NX::web::json::value& classify_bundle() const noexcept { return _classify_bundle; }
    inline const NX::web::json::value& agent_whitelist() const noexcept { return _agent_whitelist; }
    inline const std::wstring& policy_bundle() const noexcept { return _policy_bundle; }
    inline const std::wstring& policy_bundle_time() const noexcept { return _policy_bundle_time; }

    inline bool force_update() const noexcept { return _forced; }

    bool set_result(const std::wstring& xml, bool forced);

private:
    bool                    _forced;
    agent_info              _agent_info;
    NX::web::json::value    _keys_bundle;
    NX::web::json::value    _classify_bundle;
    NX::web::json::value    _agent_whitelist;
    std::wstring            _policy_bundle;
    std::wstring            _policy_bundle_time;
};

class rest_authn_result
{
public:
    rest_authn_result() : _result(0), _expire_time({0, 0}) {}
    ~rest_authn_result() {}

    bool set_result(const std::wstring& body);
    void clear() noexcept;

    inline long result() const { return _result; }
    inline bool ok() const { return (200 == _result); }
    inline const std::wstring& user_id() const { return _user_id; }
    inline const std::wstring& user_name() const { return _user_name; }
    inline const std::map<std::wstring, std::wstring>& user_attributes() const { return _user_attrs; }
    inline const FILETIME& expire_time() const { return _expire_time; }
    inline bool has_expire_time() const { return (0 != _expire_time.dwHighDateTime || 0 != _expire_time.dwLowDateTime); }

    std::wstring error_message() const noexcept;

private:
    long                                 _result;
    std::map<std::wstring, std::wstring> _user_attrs;
    std::wstring                         _user_id;
    std::wstring                         _user_name;
    FILETIME                             _expire_time;
};

class rest_checkupdate_result
{
public:
    rest_checkupdate_result() {}
    virtual ~rest_checkupdate_result() {}

    bool set_result(const std::wstring& body);

    inline bool has_new_version() const noexcept { return !_new_version.empty(); }
    inline const std::wstring& new_version() const noexcept { return _new_version; }
    inline const std::wstring& download_url() const noexcept { return _download_url; }
    inline const std::wstring& checksum() const noexcept { return _checksum; }

private:
    std::wstring _new_version;
    std::wstring _download_url;
    std::wstring _checksum;
};


}   // namespace NX



#endif