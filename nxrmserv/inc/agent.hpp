

#ifndef __NX_AGENT_HPP__
#define __NX_AGENT_HPP__


#include <string>
#include <regex>

#include <nudf\web\json.hpp>

#include "cache.hpp"


namespace NX {


class register_info
{
public:
    register_info() : _authn_type(L"local") {}
    ~register_info() {}

    inline const std::wstring& cert() const noexcept { return _cert; }
    inline const std::wstring& tenant_id() const noexcept { return _tenant_id; }
    inline const std::wstring& server() const noexcept { return _server; }
    inline const std::wstring& service_base() const noexcept { return _service_base; }
    inline const std::wstring& group_id() const noexcept { return _group_id; }
    inline const std::wstring& group_name() const noexcept { return _group_name; }
    inline const std::wstring& authn_type() const noexcept { return _authn_type; }
    inline bool external_authn() const noexcept { return (_authn_type == L"external"); }
    inline const std::vector<std::wstring>& authn_domains() const noexcept { return _authn_domains; }

    inline bool empty() const noexcept { return (_server.empty() || _service_base.empty()); }
    inline void clear() noexcept
    {
        _cert.clear();
        _tenant_id.clear();
        _server.clear();
        _service_base.clear();
        _group_id.clear();
        _group_name.clear();
        _authn_type = L"local";
        _authn_domains.clear();
    }

    register_info& operator = (const register_info& other)
    {
        if (this != &other) {
            _cert = other.cert();
            _tenant_id = other.tenant_id();
            _server = other.server();
            _service_base = other.service_base();
            _group_id = other.group_id();
            _group_name = other.group_name();
            _authn_type = other.authn_type();
            _authn_domains = other.authn_domains();
        }
        return *this;
    }

    void load(const std::wstring& file);

private:
    std::wstring    _tenant_id;
    std::wstring    _cert;
    std::wstring    _server;
    std::wstring    _service_base;
    std::wstring    _group_id;
    std::wstring    _group_name;
    std::wstring    _authn_type;
    std::vector<std::wstring>   _authn_domains;
};

class agent_info
{
public:
    agent_info();
    virtual ~agent_info();

    inline bool empty() const noexcept { return _id.empty(); }
    inline bool is_heartbeat_frequency_valid() const noexcept { return (_heartbeat_frequency > 0); }
    inline bool is_log_frequency_valid() const noexcept { return (_log_frequency > 0); }
    inline bool is_heartbeat_time_valid() const noexcept { return (0 != _heartbeat_time); }

    inline const std::wstring& id() const noexcept { return _id; }
    inline const std::wstring& cert() const noexcept { return _cert; }
    inline int heartbeat_frequency() const noexcept { return _heartbeat_frequency; }
    inline int log_frequency() const noexcept { return _log_frequency; }
    inline const std::wstring& agent_profile_name() const noexcept { return _agent_profile_name; }
    inline const std::wstring& agent_profile_time() const noexcept { return _agent_profile_time; }
    inline const std::wstring& comm_profile_name() const noexcept { return _comm_profile_name; }
    inline const std::wstring& comm_profile_time() const noexcept { return _comm_profile_time; }
    inline unsigned __int64 heartbeat_time() const noexcept { return _heartbeat_time; }
    std::wstring heartbeat_time_text() const noexcept;

    inline void set_id(const std::wstring& id) noexcept { _id = id; }
    inline void set_cert(const std::wstring& cert) noexcept { _cert = cert; }
    inline void set_heartbeat_frequency(int heartbeat_frequency) noexcept { _heartbeat_frequency = heartbeat_frequency; }
    inline void set_log_frequency(int log_frequency) noexcept { _log_frequency = log_frequency; }
    void set_heartbeat_time(const std::wstring& time) noexcept;
    void set_heartbeat_time(const FILETIME* ft = NULL) noexcept;
    inline void set_agent_profile_name(const std::wstring& agent_profile_name) noexcept
    {
        _agent_profile_name = agent_profile_name;
        std::transform(_agent_profile_name.begin(), _agent_profile_name.end(), _agent_profile_name.begin(), toupper);
    }
    inline void set_agent_profile_time(const std::wstring& agent_profile_time) noexcept
    {
        _agent_profile_time = agent_profile_time;
        std::transform(_agent_profile_time.begin(), _agent_profile_time.end(), _agent_profile_time.begin(), toupper);
    }
    inline void set_comm_profile_name(const std::wstring& comm_profile_name) noexcept
    {
        _comm_profile_name = comm_profile_name;
        std::transform(_comm_profile_name.begin(), _comm_profile_name.end(), _comm_profile_name.begin(), toupper);
    }
    inline void set_comm_profile_time(const std::wstring& comm_profile_time) noexcept
    {
        _comm_profile_time = comm_profile_time;
        std::transform(_comm_profile_time.begin(), _comm_profile_time.end(), _comm_profile_time.begin(), toupper);
    }

    agent_info& operator = (const agent_info& other) noexcept;
    bool operator == (const agent_info& other) noexcept;
    void clear() noexcept;
    void load(const std::wstring& file);
    void load_xml(const std::wstring& xml);
    void save(const std::wstring& file);

    static std::string decrypt(const std::wstring& file) noexcept;

private:
    std::wstring    _id;
    std::wstring    _cert;
    int             _heartbeat_frequency;
    int             _log_frequency;
    std::wstring    _agent_profile_name;
    std::wstring    _agent_profile_time;
    std::wstring    _comm_profile_name;
    std::wstring    _comm_profile_time;
    unsigned __int64 _heartbeat_time;
};

class key_info
{
public:
    key_info()
    {
    }
    key_info(const std::wstring& ring, const std::wstring& time, const std::wstring& id, const std::wstring& key) : _key_ring(ring), _key_time(time), _key_id(id), _key(key)
    {
        std::transform(_key_ring.begin(), _key_ring.end(), _key_ring.begin(), toupper);
        std::transform(_key_time.begin(), _key_time.end(), _key_time.begin(), toupper);
        std::transform(_key_id.begin(), _key_id.end(), _key_id.begin(), toupper);
        std::transform(_key.begin(), _key.end(), _key.begin(), toupper);
    }
    ~key_info()
    {
    }

    inline const std::wstring& key_ring() const noexcept { return _key_ring; }
    inline const std::wstring& key_time() const noexcept { return _key_time; }
    inline const std::wstring& key_id() const noexcept { return _key_id; }
    inline const std::wstring& key() const noexcept { return _key; }

    inline bool is_valid() const noexcept { return (!_key_ring.empty() && !_key_time.empty() && !_key_id.empty() && !_key.empty()); }
    void clear() noexcept
    {
        _key_ring.clear();
        _key_time.clear();
        _key_id.clear();
        _key.clear();
    }

    key_info& operator = (const key_info& other)
    {
        if (this != &other) {
            _key_ring = other.key_ring();
            _key_time = other.key_time();
            _key_id = other.key_id();
            _key = other.key();
        }
        return *this;
    }

    bool operator == (const key_info& other) {
        return (_key_ring == other.key_ring() &&
                _key_time == other.key_time() &&
                _key_id == other.key_id() &&
                _key == other.key());
    }
    
private:
    std::wstring    _key_ring;
    std::wstring    _key_time;
    std::wstring    _key_id;
    std::wstring    _key;
};

class external_user_info
{
public:
    typedef std::map<std::wstring/*sid*/, std::pair<std::wstring/*name*/, std::wstring/*domain*/>> ExternalUserMap;

    external_user_info();
    ~external_user_info();

    inline const ExternalUserMap& user_map() const noexcept { return _map; }
    inline bool empty() const noexcept { return _map.empty(); }
    inline void clear() noexcept { _map.clear(); }
    inline external_user_info& operator = (const external_user_info& other) noexcept
    {
        if (this != &other) {
            _map.clear();
            _map = other.user_map();
        }
        return *this;
    }

    void load();
    void save();
    void insert(const std::wstring& user_id, const std::wstring& user_name, const std::wstring& user_domain);
    std::pair<std::wstring, std::wstring> find_user(const std::wstring& user_id) const noexcept;
    bool user_exists(const std::wstring& user_id) const noexcept;
    std::vector<std::wstring> get_id_list() const noexcept;

private:
    ExternalUserMap     _map;
    mutable rwlocker    _lock;
    bool                _dirty;
};

class whitelist_info
{
private:
    class app_info
    {
    public:
        app_info() {}
        app_info(const std::wstring& name, const std::wstring& publisher);
        ~app_info() {}

        inline const std::wregex& name() const noexcept { return _name; }
        inline const std::wregex& publisher() const noexcept { return _publisher; }
        inline app_info& operator = (const app_info& other) noexcept
        {
            if (this != &other) {
                _name = other.name();
                _publisher = other.publisher();
            }
            return *this;
        }

        bool check(const std::wstring& image, const std::wstring& publisher) const noexcept;
        

    private:
        std::wregex _name;
        std::wregex _publisher;
    };

public:
    whitelist_info();
    whitelist_info(const NX::web::json::value& agent_whitelist);
    ~whitelist_info();

    inline const std::vector<std::pair<app_info, NX::EVAL::rights>>& app_list() const noexcept { return _app_list; }
    inline const std::wstring& ext_list() const noexcept { return _ext_list; }
    inline const std::vector<unsigned char> hash() const noexcept { return _hash; }
    inline bool empty() const noexcept { return _hash.empty(); }


    whitelist_info& operator = (const whitelist_info& other) noexcept;
    whitelist_info& operator = (const NX::web::json::value& v) noexcept;
    void clear() noexcept;
    void set(const NX::web::json::value& v) noexcept;

    NX::EVAL::rights check_app(const std::wstring& image, const std::wstring& publisher) const noexcept;


private:
    std::wstring    _ext_list;
    std::vector<std::pair<app_info, NX::EVAL::rights>>  _app_list;
    std::vector<unsigned char>  _hash;
    mutable rwlocker    _lock;
};

}



#endif