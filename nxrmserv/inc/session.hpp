
#ifndef __NXSERV_SESSION_HPP__
#define __NXSERV_SESSION_HPP__


#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <nudf\knowndir.hpp>

#include "nxlogdb.hpp"

#include "overlay.hpp"
#include "policy.hpp"
#include "profile.hpp"
#include "cache.hpp"


namespace NX {


    
class user
{
public:
    user(){}
    user(const user& other) :
        _id(other._id),
        _name(other._name),
        _principle_name(other._principle_name),
        _domain(other._domain)
    {
    }
    user(user&& other) :
        _id(std::move(other._id)),
        _name(std::move(other._name)),
        _principle_name(std::move(other._principle_name)),
        _domain(std::move(other._domain))
    {
    }
    user(const std::wstring& id, const std::wstring& name, const std::wstring& domain, const std::wstring& principle_name):_id(id),_name(name),_domain(domain),_principle_name(principle_name)
    {
        // _id MUST be upper case
        std::transform(_id.begin(), _id.end(), _id.begin(), toupper);
        // _name/_domain/_principle_name MUST be lower case
        std::transform(_name.begin(), _name.end(), _name.begin(), tolower);
        std::transform(_domain.begin(), _domain.end(), _domain.begin(), toupper);
        std::transform(_principle_name.begin(), _principle_name.end(), _principle_name.begin(), tolower);
    }
    virtual ~user(){}

    bool operator == (const user& other) const noexcept
    {
        if(this != &other) {
            return (_id == other.id());
        }
        else {
            return true;
        }
    }

    user& operator = (const user& other)
    {
        if(this != &other) {
            _id = other.id();
            _name = other.name();
            _principle_name = other.principle_name();
            _domain = other.domain();
        }
        return *this;
    }

    user& operator = (user&& other)
    {
        if(this != &other) {
            _id = std::move(other._id);
            _name = std::move(other._name);
            _principle_name = std::move(other._principle_name);
            _domain = std::move(other._domain);
        }
        return *this;
    }

    inline void clear() noexcept
    {
        _id.clear();
        _name.clear();
        _principle_name.clear();
        _domain.clear();
    }

    inline const std::wstring& id() const noexcept {return _id;}
    inline const std::wstring& name() const noexcept {return _name;}
    inline const std::wstring& principle_name() const noexcept {return _principle_name;}
    inline const std::wstring& best_name() const noexcept {return (_principle_name.empty() ? _name : _principle_name);}
    inline const std::wstring& domain() const noexcept {return _domain;}

    inline bool empty() const noexcept {return _name.empty();}
    inline bool is_local_user() const noexcept {return _domain.empty();}
    inline bool is_domain_user() const noexcept {return !is_local_user();}

    static NX::user get_user(PSID sid) noexcept;
    static NX::user get_token_user(void* token) noexcept;
    static NX::user get_process_user(unsigned long process_id) noexcept;
    static NX::user get_session_user(unsigned long session_id) noexcept;

private:
    std::wstring    _id;
    std::wstring    _name;
    std::wstring    _principle_name;
    std::wstring    _domain;
};


class session_app
{
public:
    session_app();
    session_app(unsigned long id);
    virtual ~session_app();

    void start();
    void stop();

    inline const std::wstring& app_port() const noexcept { return _port; }

    static void workthread(session_app* app, HANDLE h);

protected:
    void start_app();
    void kill_existing_app(bool safe);
    void shutdown_app();
    void wait();

private:
    unsigned long   _ssid;
    std::wstring    _port;
    std::thread     _worker;
    HANDLE          _hevts[2];
    bool            _stopping;
};

class session
{
public:
    session();
    session(unsigned long session_id);
    virtual ~session();

    inline bool empty() const noexcept {return !valid();}
    inline unsigned long id() const noexcept {return _id;}
    inline const NX::user& user() const noexcept {return _user;}
    inline const std::wstring& classify_group() const noexcept {return _classify_group;}
    inline const NX::profile& profile() const noexcept {return _profile;}
    inline bool valid() const noexcept {return (((unsigned long)-1) != _id);}
    inline bool logged_on() const noexcept { return _profile.logged_on(); }

    inline const nudf::win::CKnownDirs& session_dirs() const { return _session_dirs; };

    inline bool is_consol() const noexcept {return (0 == session_protocol());}
    inline bool is_rdp() const noexcept {return (2 == session_protocol());}

    inline bool is_dwm_enabled() const { return _is_dwm_enabled; }

    inline const NX::overlay::overlay_image& get_overlay_image() const { return _overlay_image; }

    NX::session& operator = (const NX::session& other) noexcept;
    unsigned short session_protocol() const noexcept;

    void start() noexcept;
    void stop() noexcept;

    // talk to tray-app
    void notify(const std::wstring& title, const std::wstring& msg);
    void set_policy_time(const std::wstring& time);
    void set_heartbeat_time(const std::wstring& time);
    void set_serv_status();
    void show_logon_ui();
    void send_update_response(bool updated);

    NX::web::json::value build_serv_status_response();
    NX::web::json::value build_serv_status_response2();

    // remote authn
    long logon(const std::wstring& name, const std::wstring& domain, const std::wstring& password) noexcept;
    void logoff() noexcept;

    // evaluate
    void set_session_policy_bundle(const NX::EVAL::policy_bundle& bundle) noexcept;
    void evaluate(NX::EVAL::eval_object& eval_obj) noexcept;
    void audit(const NX::EVAL::eval_object& eo);
    void audit(const std::wstring& operation_name, bool allowed, const std::wstring& resource_path, const std::wstring& app_name, const std::wstring& description);

    void set_dwm_enabled(bool enabled);
    bool check_dwm_status();

    void export_activity_log(const std::wstring& file);

protected:
    void create_overlay_image(const std::map<std::wstring, std::wstring>& parameter);

private:
    static void update_policy(NX::session* ss);
    static void update_user_attributes(NX::session* ss);
    static void session_app_monitor(NX::session* ss);

    void dump_db_info(const NXLOGDB::db_conf& conf);

private:
    bool            _ready;
    // win session information
    unsigned long   _id;
    NX::user        _user;
    std::wstring    _classify_group;
    std::wstring    _user_context;
    bool            _is_dwm_enabled;
    // profile information
    NX::profile    _profile;
    session_app    _ssapp;
    // evaluate
    std::vector<std::shared_ptr<NX::EVAL::policy>>  _bundle;
    rwlocker                                        _bundle_lock;

    nudf::win::CKnownDirs   _session_dirs;

    NX::overlay::overlay_image _overlay_image;

    NXLOGDB::nxlogdb    _audit_db;
};


class serv_session
{
public:
    typedef std::map<unsigned long, std::shared_ptr<NX::session>> SessionMapType;
    serv_session();
    ~serv_session();

    void clear() noexcept;
    bool empty() noexcept;
    void move(SessionMapType& ss);
    void copy(SessionMapType& ss);

    bool exists(unsigned long id) noexcept;
    std::shared_ptr<NX::session> add(unsigned long id, bool* existing) noexcept;
    std::shared_ptr<NX::session> del(unsigned long id) noexcept;
    std::shared_ptr<NX::session> get(unsigned long id) noexcept;

private:
    SessionMapType      _map;
    CRITICAL_SECTION    _lock;
};

}


#endif