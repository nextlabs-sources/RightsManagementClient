

#ifndef __NXSERV_GLOBAL_HPP__
#define __NXSERV_GLOBAL_HPP__


#include <string>
#include <map>
#include <vector>
#include <memory>
#include <set>

#include <nudf\web\json.hpp>
#include <nudf\rwlock.hpp>

#include "servlog.hpp"
#include "session.hpp"
#include "secure.hpp"
#include "sys.hpp"
#include "asyncpipe.hpp"
#include "jobpool.hpp"
#include "agent.hpp"
#include "drvcore.hpp"
#include "drvflt.hpp"
#include "policy.hpp"
#include "cache.hpp"
#include "audit.hpp"
#include "rest.hpp"
#include "process_status.hpp"

namespace NX {


class global
{
public:
    enum SERV_STATUS {
        serv_stopped = 0,
        serv_running,
        serv_paused,
        serv_start_pending,
        serv_stop_pending
    };

    class serv_pipe_serv : public NX::async_pipe::server
    {
    public:
        serv_pipe_serv();
        virtual ~serv_pipe_serv();
        virtual void on_read(unsigned char* data, unsigned long* size, bool* write_response);
    };

    typedef std::map<std::wstring, std::pair<unsigned __int64, std::vector<unsigned __int64>>> CoreContextMapType;
    typedef std::pair<unsigned __int64, std::vector<unsigned __int64>> CoreContextType;

public:
    global();
    virtual ~global();

    bool init();
    bool listen();
    void run_jobs();
    void clear();

    void load_configurations(NX::web::json::value& classify_bundle, NX::web::json::value& keys_bundle);
    void generate_agent_key() noexcept;
    unsigned long time_since_last_heartbeat() noexcept;

    bool update_agent_info(const agent_info& info, bool force_update_heartbeat_time);
    bool update_classify_bundle(const std::wstring& v);
    bool update_keys_bundle(const std::wstring& v);
    bool update_agent_whitelist(const NX::web::json::value& v);
    bool update_policy_bundle(const std::wstring& v, const std::wstring& timestamp, bool force = false);

    void update_core_context(const std::wstring& module_path, unsigned __int64 module_checksum, const unsigned __int64* module_context, unsigned long count);
    CoreContextType query_core_context(const std::wstring& module_path);
    void save_core_context();
    void load_core_context();

    static std::vector<unsigned char> string_hash(const std::wstring& v) noexcept;
    static std::vector<unsigned char> json_hash(const NX::web::json::value& v) noexcept;
    static bool create_profile_folder(const std::wstring& sid, bool fail_if_exist=false, bool for_everyone=false);
    

    inline SERV_STATUS status() const noexcept { return _status; }
    inline bool connected() const noexcept { return _connected; }
    inline void set_connect_status(bool b) noexcept { _connected = b; }
    inline void change_status(SERV_STATUS status) { _status = status; }
    inline bool is_stopped() const noexcept { return (serv_stopped == _status); }
    inline bool is_paused() const noexcept { return (serv_paused == _status); }
    inline bool is_running() const noexcept { return (serv_running == _status); }
    inline bool is_start_pending() const noexcept { return (serv_start_pending == _status); }
    inline bool is_stop_pending() const noexcept { return (serv_stop_pending == _status); }
    inline bool is_pending() const noexcept { return (is_start_pending() || is_stop_pending()); }

    inline const std::wstring& dir_root() const noexcept {return _dir_root;}
    inline const std::wstring& dir_bin() const noexcept {return _dir_bin;}
    inline const std::wstring& dir_conf() const noexcept {return _dir_conf;}
    inline const std::wstring& dir_profiles() const noexcept {return _dir_profiles;}
    inline const std::wstring& dir_auditlogs() const noexcept {return _dir_auditlogs;}
    inline const std::wstring& dir_dbglogs() const noexcept {return _dir_dbglogs;}
    inline const std::wstring& dir_wintemp() const noexcept {return _dir_wintemp;}

    inline HMODULE res_module() const noexcept { return _resdll; }

    inline NX::drv::serv_drv& drv_core() noexcept { return _drv_core; }
    inline NX::drv::serv_flt& drv_flt() noexcept { return _drv_flt; }

    inline NX::rest_client& rest() noexcept { return _rest_client; }

    inline const NX::register_info& register_info() const noexcept { return _reg_info; }
    inline const NX::agent_info& agent_info() const noexcept { return _agent_info; }
    inline NX::agent_info& agent_info() noexcept { return _agent_info; }

    inline const NX::external_user_info& external_users() const noexcept { return _external_users; }
    inline NX::external_user_info& external_users() noexcept { return _external_users; }

    inline const std::vector<unsigned char>& keys_budnle_hash() noexcept { return _keys_budnle_hash; }
    inline const std::vector<unsigned char>& classify_budnle_hash() noexcept { return _classify_budnle_hash; }

    inline const NX::product& product() const noexcept {return _prod;}
    inline const NX::module& module() const noexcept {return _mod;}
    inline const NX::host& host() const noexcept {return _host;}
    inline NX::log& log() noexcept {return _log;}
    inline global::serv_pipe_serv& serv_pipe() noexcept {return _pipe_serv;}
    inline NX::serv_session& serv_session() noexcept {return _session_serv;}
    inline NX::serv_audit& audit_serv() noexcept {return _audit_serv;}
    inline const NX::EVAL::policy_bundle& policy_bundle() const noexcept { return _policy_bundle; }
    inline const whitelist_info& whitelist_info() const noexcept { return _whitelist_info; }

    inline bool policy_need_force_update() const noexcept { return _policy_need_force_update; }
    inline void set_policy_force_update_flag() noexcept { _policy_need_force_update = true; }
    inline void reset_policy_force_update_flag() noexcept { _policy_need_force_update = false; }

    inline unsigned long get_requestor_session_id() const noexcept { return _requestor_session_id; }
    inline void set_requestor_session_id(unsigned long id) noexcept { _requestor_session_id = id; }

    inline const NX::secure_mem& agent_key() const noexcept { return _agent_key; }
    inline NX::secure_mem& agent_key() noexcept { return _agent_key; }

    inline __int64 start_time() const noexcept { return _start_time; }

    inline NX::job_pool& jobs() noexcept { return _job_pool; }

    std::vector<unsigned char> generate_keys_budnle(const std::wstring& s) const noexcept;
    std::vector<unsigned char> generate_keys_budnle(const NX::web::json::value& v) const noexcept;

    inline NX::process_map& process_cache() noexcept { return _process_cache; }
    inline NX::cache_map<unsigned __int64, std::shared_ptr<NX::EVAL::eval_result>, 512, 300>& eval_cache() noexcept { return _eval_cache; }

#ifdef _DEBUG
    std::vector<unsigned char>  _nxl_keys_blob;
#endif

private:
    std::wstring create_listen_port() noexcept;
    void initialize_sspi() noexcept;

private:
    // status
    SERV_STATUS         _status;
    bool                _connected;

    // object
    NX::product         _prod;
    NX::module          _mod;
    NX::log             _log;
    serv_pipe_serv      _pipe_serv;
    NX::serv_session    _session_serv;
    NX::serv_audit      _audit_serv;
    NX::host            _host;
    NX::drv::serv_drv   _drv_core;
    NX::drv::serv_flt   _drv_flt;
    NX::rest_client     _rest_client;

    CoreContextMapType  _core_context_map;
    CRITICAL_SECTION    _core_context_map_lock;
    
    NX::register_info   _reg_info;
    NX::agent_info      _agent_info;
    NX::external_user_info  _external_users;
    secure_mem          _agent_key;

    bool                    _policy_need_force_update;
    NX::EVAL::policy_bundle _policy_bundle;
    NX::whitelist_info      _whitelist_info;

    __int64             _start_time;
    HANDLE              _stop_evt;

    HMODULE             _resdll;


    // hash or timestamp of bundles
    std::vector<unsigned char>  _keys_budnle_hash;
    std::vector<unsigned char>  _classify_budnle_hash;
    
    // caches
    NX::cache_map<unsigned __int64, std::shared_ptr<NX::EVAL::eval_result>, 512, 300>   _eval_cache;

    NX::process_map _process_cache;


    // paths
    std::wstring    _dir_root;
    std::wstring    _dir_bin;
    std::wstring    _dir_conf;
    std::wstring    _dir_profiles;
    std::wstring    _dir_auditlogs;
    std::wstring    _dir_dbglogs;
    std::wstring    _dir_wintemp;

    //jobs
    job_pool        _job_pool;
    std::thread     _register_thread;
    std::thread     _heartbeat_thread;
    std::thread     _upgrade_thread;
    std::thread     _audit_thread;

    // client request
    unsigned long   _requestor_session_id;
};


}   // namespace NX


#endif