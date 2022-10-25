

#include <Windows.h>
#include <stdio.h>
#include <assert.h>
#include <Wtsapi32.h>
#include <Userenv.h>
#include <Sddl.h>
#define SECURITY_WIN32
#include <security.h>
#include <Sspi.h>

#pragma warning(push)
#pragma warning(disable: 4091)
#include <imagehlp.h>
#pragma warning(pop)
#include <cstdio>
#include <wchar.h>
#include <psapi.h>
#include <stdlib.h>

#include <string>
#include <vector>
#include <algorithm>

#include <nudf\string.hpp>
#include <nudf\encoding.hpp>
#include <nudf\crypto.hpp>
#include <nudf\shared\keydef.h>
#include <nudf\web\json.hpp>

#include <rapidxml\rapidxml.hpp>
#include <rapidxml\rapidxml_utils.hpp>
#include <rapidxml\rapidxml_print.hpp>


#include "nxrmserv.h"
#include "sconfig.hpp"
#include "debug.hpp"
#include "servlog.hpp"


using namespace NX;



static NX::web::json::value init_request(const unsigned char* data, unsigned long size) noexcept;
static NX::web::json::value on_request_echo(int code, int session_id, const NX::web::json::value& request) noexcept;
static NX::web::json::value on_request_logon(int code, int session_id, const NX::web::json::value& request) noexcept;
static NX::web::json::value on_request_logoff(int code, int session_id, const NX::web::json::value& request) noexcept;
static NX::web::json::value on_request_set_aero_status(int code, int session_id, const NX::web::json::value& request) noexcept;
static NX::web::json::value on_request_query_status(int code, int session_id, const NX::web::json::value& request) noexcept;
static NX::web::json::value on_request_update(int code, int session_id, const NX::web::json::value& request) noexcept;
static NX::web::json::value on_enable_debug(int code, int session_id, const NX::web::json::value& request) noexcept;
static NX::web::json::value on_disable_debug(int code, int session_id, const NX::web::json::value& request) noexcept;
static NX::web::json::value on_collect_debug(int code, int session_id, const NX::web::json::value& request) noexcept;
static NX::web::json::value on_query_default_authn_info(int code, int session_id, const NX::web::json::value& request) noexcept;



static bool __stdcall DbLogAccept(unsigned long level)
{
    return GLOBAL.log().AcceptLevel((LOGLEVEL)level) ? true : false;
}

static void __stdcall DbLogPrint(unsigned long level, const wchar_t* message)
{
    UNREFERENCED_PARAMETER(level);
    GLOBAL.log().Push(message);
}


//
//  class global
//

NX::global::serv_pipe_serv::serv_pipe_serv() : NX::async_pipe::server(4096UL, 3000UL)
{
}

NX::global::serv_pipe_serv::~serv_pipe_serv()
{
}

void NX::global::serv_pipe_serv::on_read(unsigned char* data, unsigned long* size, bool* write_response)
{
    NX::web::json::value response = NX::web::json::value::object();
    NX::web::json::value request = init_request(data, *size);

    static const std::string sussess_response_default("{\"result\": 0}");
    static const std::string error_response_response_too_long("{\"result\": 1, \"info\": \"response is too long\"}");
    
    try {

        if (!request.is_object()) {
            response[L"result"] = NX::web::json::value::number(-1);
            response[L"message"] = NX::web::json::value::string(L"invalid request format");
            throw std::exception("bad request format");
        }
        if (!request.has_field(L"code")) {
            response[L"result"] = NX::web::json::value::number(-2);
            response[L"message"] = NX::web::json::value::string(L"request code not found");
            throw std::exception("request code not found");
        }
        if (!request[L"code"].is_number()) {
            response[L"result"] = NX::web::json::value::number(-3);
            response[L"message"] = NX::web::json::value::string(L"request code is not a number");
            throw std::exception("request code is not a number");
        }
        if (!request.has_field(L"session")) {
            response[L"result"] = NX::web::json::value::number(-4);
            response[L"message"] = NX::web::json::value::string(L"request session id not found");
            throw std::exception("request code not found");
        }
        if (!request[L"session"].is_number()) {
            response[L"result"] = NX::web::json::value::number(-5);
            response[L"message"] = NX::web::json::value::string(L"request session id is not a number");
            throw std::exception("request code is not a number");
        }

        int code = request[L"code"].as_integer();
        int session_id = request[L"session"].as_integer();
        switch (code)
        {
        case 0:     // echo test
            response = on_request_echo(code, session_id, request);
            break;
        case 1:     // logon
            response = on_request_logon(code, session_id, request);
            break;
        case 2:     // logoff
            response = on_request_logoff(code, session_id, request);
            break;
        case 3:     // query status
            response = on_request_query_status(code, session_id, request);
            break;
        case 4:     // update --> trigger heartbeat
            response = on_request_update(code, session_id, request);
            break;
        case 5:     // disable debug
            response = on_disable_debug(code, session_id, request);
            break;
        case 6:     // enable debug
            response = on_enable_debug(code, session_id, request);
            break;
        case 7:     // collect log
            response = on_collect_debug(code, session_id, request);
            break;
        case 8:     // query default logon information
            response = on_query_default_authn_info(code, session_id, request);
            break;
        case 9:     // set dwm composition (aero) status
            response = on_request_set_aero_status(code, session_id, request);
            break;
        default:
            response[L"result"] = NX::web::json::value::number(-4);
            response[L"message"] = NX::web::json::value::string(L"unknown request code");
            LOGWARN(0, L"serv_pipe: unknown request code (%d)", code);
            break;
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
    catch (NX::structured_exception& e) {
        std::wstring exception_msg = e.exception_message();
        LOGASSERT(exception_msg.c_str());
        e.dump(); // NOTHING
        e.raise();
    }

    std::wstring ws = response.serialize();
    std::string s = NX::utility::conversions::utf16_to_utf8(ws);
    if (s.length() > (buffer_size() - 1)) {
        int response_code = response[L"result"].as_integer();
        response = NX::web::json::value::object();
        response[L"result"] = NX::web::json::value::number(response_code);
        response[L"message"] = NX::web::json::value::string(L"--- response is too long ---");
        LOGASSERT(L"response is too long: %d", s.length());
        ws = response.serialize();
        s = NX::utility::conversions::utf16_to_utf8(ws);
    }

    // set response data
    memcpy(data, s.c_str(), s.length() + 1);
    *size = (unsigned long)s.length() + 1;
    *write_response = true;
}


NX::global::global() : _connected(false), _policy_need_force_update(false), _requestor_session_id(0), _resdll(NULL), _stop_evt(NULL), _start_time(0), _status(serv_stopped)
{
    FILETIME _gmt_time = { 0, 0 };
    FILETIME _local_time = { 0, 0 };
    GetSystemTimeAsFileTime(&_gmt_time);
    FileTimeToLocalFileTime(&_gmt_time, &_local_time);
    _start_time = _local_time.dwHighDateTime;
    _start_time <<= 32;
    _start_time += _local_time.dwLowDateTime;

    _stop_evt = ::CreateEventW(NULL, TRUE, FALSE, NULL);
    ::InitializeCriticalSection(&_core_context_map_lock);
}

NX::global::~global()
{
    clear();
    if (NULL != _stop_evt) {
        CloseHandle(_stop_evt);
    }
    ::DeleteCriticalSection(&_core_context_map_lock);
}

bool NX::global::init()
{
    // Get module information
    _mod = NX::module(NULL, true);

    // Get product information
#ifdef _AMD64_
    _prod = NX::product(L"NextLabs Rights Management", arch_amd64);
#else
    _prod = NX::product(L"NextLabs Rights Management", arch_i386);
#endif

    // Initialize paths
    //    -> bin
    _dir_bin =  _prod.install_root() + L"\\bin";
    if(0 == _wcsicmp(_mod.parent_dir().c_str(), _prod.install_root().c_str())) {
        // this app is in the install location
        _dir_root = _prod.install_root();
    }
    else {
        // this app is not in the install location
        _dir_bin = _mod.parent_dir();
        std::wstring bin_name;
        NX::module::parse_filepath(_dir_bin, _dir_root, bin_name);
    }

    //    -> conf
    _dir_conf = _dir_root + L"\\conf";
    ::CreateDirectoryW(_dir_conf.c_str(), NULL);

    //    -> profiles
    _dir_profiles = _dir_root + L"\\profiles";
    ::CreateDirectoryW(_dir_profiles.c_str(), NULL);

    //    -> audit logs
    _dir_auditlogs = _dir_root + L"\\log";
    ::CreateDirectoryW(_dir_auditlogs.c_str(), NULL);

    //    -> dbg logs
    _dir_dbglogs = _dir_root;
    ::CreateDirectoryW(_dir_dbglogs.c_str(), NULL);

    //    -> win-temp
    _dir_wintemp = L"C:\\Windows\\Temp";
    ::CreateDirectoryW(_dir_wintemp.c_str(), NULL);

    //    -> init resource dll
    std::wstring resdll_path = _dir_bin + L"\\nxrmres.dll";
    _resdll = ::LoadLibraryW(resdll_path.c_str());

    // Initialize SSPI
    initialize_sspi();

    // Initialize logging system
    _log.init();
    try {
        _log.Start();
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        return false;
    }

    // try to load register info
    try {
        std::wstring register_info_file = GLOBAL.dir_conf() + L"\\register.xml";
        _reg_info.load(register_info_file);
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        _reg_info.clear();
    }

    // initialize audit log db
    NXLOGDB::SetDbgLogRoutine(DbLogAccept, DbLogPrint);
    return true;
}

bool NX::global::update_agent_info(const NX::agent_info& info, bool force_update_heartbeat_time)
{
    bool changed = false;

    if (info.is_heartbeat_frequency_valid() && _agent_info.heartbeat_frequency() != info.heartbeat_frequency()) {
        changed = true;
        _agent_info.set_heartbeat_frequency(info.heartbeat_frequency());

		std::shared_ptr<NX::job> spJobHB = jobs().get(NX::job::jn_heartbeat);
		if (NULL != spJobHB) {
			spJobHB->reset_repeat_interval(agent_info().heartbeat_frequency() * 1000);
		}
    }
    if (info.is_log_frequency_valid() && _agent_info.log_frequency() != info.log_frequency()) {
        changed = true;
        _agent_info.set_log_frequency(info.log_frequency());

		std::shared_ptr<NX::job> spJobAudit = jobs().get(NX::job::jn_audit);
		if (NULL != spJobAudit) {
			spJobAudit->reset_repeat_interval(agent_info().log_frequency() * 1000);
		}
    }
    if(!info.agent_profile_name().empty() && _agent_info.agent_profile_name() != info.agent_profile_name()) {
        changed = true;
        _agent_info.set_agent_profile_name(info.agent_profile_name());
    }
    if(!info.agent_profile_time().empty() && _agent_info.agent_profile_time() != info.agent_profile_time()) {
        changed = true;
        _agent_info.set_agent_profile_time(info.agent_profile_time());
    }
    if(!info.comm_profile_name().empty() && _agent_info.comm_profile_name() != info.comm_profile_name()) {
        changed = true;
        _agent_info.set_comm_profile_name(info.comm_profile_name());
    }
    if(!info.comm_profile_time().empty() && _agent_info.comm_profile_time() != info.comm_profile_time()) {
        changed = true;
        _agent_info.set_comm_profile_time(info.comm_profile_time());
    }
    if(!info.cert().empty() && _agent_info.cert() != info.cert()) {
        changed = true;
        _agent_info.set_cert(info.cert());
    }
    if (changed || force_update_heartbeat_time) {
        changed = true;
        _agent_info.set_heartbeat_time(NULL);
    }

    if (changed) {
        // Save agent info to local file
        std::wstring file = GLOBAL.dir_conf() + L"\\agent_info.sjs";
        _agent_info.save(file);
    }

    return changed;
}

bool NX::global::update_classify_bundle(const std::wstring& v)
{
    bool changed = false;

    std::vector<unsigned char> hash = NX::global::string_hash(v);
    if (hash != classify_budnle_hash()) {
        std::string s = NX::utility::conversions::utf16_to_utf8(v);
        std::wstring file = GLOBAL.dir_conf() + L"\\agent_classify.sjs";
        NX::sconfig::save(file, s, GLOBAL.agent_key().decrypt());
        _classify_budnle_hash = hash;
        changed = true;
    }

    return changed;
}

bool NX::global::update_keys_bundle(const std::wstring& v)
{
    bool changed = false;

    std::vector<unsigned char> hash = NX::global::string_hash(v);
    if (hash != keys_budnle_hash()) {

        changed = true;
        std::vector<unsigned char> keys_blob = generate_keys_budnle(v);
        if (!keys_blob.empty()) {

            // save new keys bundle to file
            std::string s = NX::utility::conversions::utf16_to_utf8(v);
            std::wstring file = GLOBAL.dir_conf() + L"\\agent_keys.sjs";
            NX::sconfig::save(file, s, GLOBAL.agent_key().decrypt());
            _keys_budnle_hash = hash;

            // update keys in driver
            if (!_drv_flt.started()) {
                // we need to start it
                LOGSYSINF(L"[SERVER] Connecting to filter driver ...");
                if (_drv_flt.start(&keys_blob[0], (unsigned long)keys_blob.size())) {
#ifdef _DEBUG
                    GLOBAL._nxl_keys_blob = keys_blob;
#endif
                    LOGSYSINF(L"[SERVER] Connected");
                }
                else {
                    LOGASSERT(L"[SERVER] Failed (%d)", GetLastError());
                }
            }
            else {
                // we need to update keys
                LOGSYSINF(L"[SERVER] Updating keys in filter driver ...");
                unsigned long result = 0;
                if (0 == (result = _drv_flt.set_keys(&keys_blob[0], (unsigned long)keys_blob.size()))) {
                    LOGSYSINF(L"[SERVER] Updated");
                }
                else {
                    LOGASSERT(L"[SERVER] Failed (%d)", result);
                }
            }
        }
    }

    return changed;
}

bool NX::global::update_agent_whitelist(const NX::web::json::value& v)
{
    bool changed = false;

    std::vector<unsigned char> hash = NX::global::json_hash(v);
    if (hash != whitelist_info().hash()) {
        std::wstring ws = v.serialize();
        std::string s = NX::utility::conversions::utf16_to_utf8(ws);
        std::wstring file = GLOBAL.dir_conf() + L"\\agent_whitelist.sjs";
        NX::sconfig::save(file, s, GLOBAL.agent_key().decrypt());
        _whitelist_info.set(v);
        changed = true;
    }

    return changed;

}

static bool clean_bundle(const std::wstring& v, std::wstring& bundle)
{
    static const std::wstring sid_prefix(L"S-1-5");
    bool result = false;

    try {

        rapidxml::xml_document<wchar_t> doc;
        doc.parse<0>((wchar_t*)v.c_str());
        
        rapidxml::xml_node<wchar_t>* root_node = doc.first_node();
        if(NULL == root_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        rapidxml::xml_node<wchar_t>* usermap_node = root_node->first_node(L"USERGROUPMAP", 0, false);
        if(NULL == usermap_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        rapidxml::xml_node<wchar_t>* user_node = usermap_node->first_node(L"USERMAP", 0, false);
        while (user_node) {
            rapidxml::xml_node<wchar_t>* current_node = user_node;
            user_node = user_node->next_sibling(L"USERMAP", 0, false);

            rapidxml::xml_attribute<wchar_t>* current_node_id = current_node->first_attribute(L"id", 0, false);
            std::wstring usernode_id = (NULL == current_node_id) ? L"" : current_node_id->value();
            if (0 == _wcsicmp(usernode_id.c_str(), L"{UNKNOWN-USER}") || 0 == _wcsnicmp(usernode_id.c_str(), sid_prefix.c_str(), sid_prefix.length())) {
                continue;
            }
            usermap_node->remove_node(current_node);
        }

        rapidxml::print(std::back_inserter(bundle), doc);
        result = true;
    }
    catch (const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        result = false;
        bundle = v;
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        result = false;
        bundle = v;
    }

    return result;
}

bool NX::global::update_policy_bundle(const std::wstring& v, const std::wstring& timestamp, bool force)
{
    bool changed = false;

    if (force || (!timestamp.empty() && timestamp != policy_bundle().time())) {

        std::wstring clean_bundle_xml;
        std::string  s;
        std::wstring file;

        // save original data
        s = NX::utility::conversions::utf16_to_utf8(v);
        file = GLOBAL.dir_conf() + L"\\agent_policy_orig.sjs";
        NX::sconfig::save(file, s, GLOBAL.agent_key().decrypt());

        // save cleaned bundle
        clean_bundle(v, clean_bundle_xml);
        s = NX::utility::conversions::utf16_to_utf8(clean_bundle_xml);
        file = GLOBAL.dir_conf() + L"\\agent_policy.sjs";
        NX::sconfig::save(file, s, GLOBAL.agent_key().decrypt());

        _policy_bundle = NX::EVAL::policy_bundle::load(clean_bundle_xml);
        changed = true;
        LOGDBG(L"Policy updated (%s) %s", timestamp.c_str(), force ? L"(forced)" : L"(normal)");
    }

    return changed;
}

void NX::global::update_core_context(const std::wstring& module_path, unsigned __int64 module_checksum, const unsigned __int64* module_context, unsigned long count)
{
    std::wstring key(module_path);
    std::transform(key.begin(), key.end(), key.begin(), tolower);
    if (boost::algorithm::istarts_with(key, L"\\??\\")) {
        key = key.substr(4);
    }
    NX::global::CoreContextType context;
    context.first = module_checksum;
    if (NULL != module_context && 0 != count) {
        context.second.insert(context.second.end(), module_context, module_context + count);
    }
    ::EnterCriticalSection(&_core_context_map_lock);
    _core_context_map[key] = context;
    ::LeaveCriticalSection(&_core_context_map_lock);

    // save it
    save_core_context();
}

NX::global::CoreContextType NX::global::query_core_context(const std::wstring& module_path)
{
    NX::global::CoreContextType context;
    context.first = 0;  // init
    std::wstring key(module_path);
    std::transform(key.begin(), key.end(), key.begin(), tolower);
    if (boost::algorithm::istarts_with(key, L"\\??\\")) {
        key = key.substr(4);
    }
    ::EnterCriticalSection(&_core_context_map_lock);
    auto pos = _core_context_map.find(key);
    if (pos != _core_context_map.end()) {
        context = (*pos).second;
    }
    ::LeaveCriticalSection(&_core_context_map_lock);

    return context;
}

void NX::global::save_core_context()
{
    try {

        std::wstring file = GLOBAL.dir_conf() + L"\\core_context.sjs";

        NX::web::json::value v = NX::web::json::value::object();
        try {
            ::EnterCriticalSection(&_core_context_map_lock);
            std::for_each(_core_context_map.begin(), _core_context_map.end(), [&](const std::pair<std::wstring, CoreContextType>& it) {

                if (it.second.second.empty()) {
                    return;
                }

                NX::web::json::value mod = NX::web::json::value::object();
                mod[L"checksum"] = NX::web::json::value::number(it.second.first);
                NX::web::json::value pointers = NX::web::json::value::array(it.second.second.size());
                int i = 0;
                std::for_each(it.second.second.begin(), it.second.second.end(), [&](const unsigned __int64 pt) {
                    pointers[i++] = NX::web::json::value::number(pt);
                });
                mod[L"pointers"] = pointers;

                v[it.first] = mod;
            });
        }
        catch (std::exception& e) {
            UNREFERENCED_PARAMETER(e);
        }
        ::LeaveCriticalSection(&_core_context_map_lock);
        std::wstring ws = v.serialize();
        std::string s = NX::utility::conversions::utf16_to_utf8(ws);
        NX::sconfig::save(file, s, agent_key().decrypt());
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
}

void NX::global::load_core_context()
{
    try {

        std::wstring file = GLOBAL.dir_conf() + L"\\core_context.sjs";
        std::string s = NX::sconfig::load(file, agent_key().decrypt());
        std::wstring ws = NX::utility::conversions::utf8_to_utf16(s);
        NX::web::json::value v = NX::web::json::value::parse(ws);
        if (v.is_object()) {
            const NX::web::json::object& modules = v.as_object();
            std::for_each(modules.begin(), modules.end(), [&](const std::pair<std::wstring, NX::web::json::value>& it) {
                std::wstring module_path = it.first;
                if (it.second.has_field(L"checksum") && it.second.has_field(L"pointers")) {
                    unsigned __int64 module_checksum = it.second.at(L"checksum").as_number().to_uint64();
                    const NX::web::json::array& module_pointers = it.second.at(L"pointers").as_array();
                    if (module_pointers.size() != 0) {
                        ::EnterCriticalSection(&_core_context_map_lock);
                        try {
                            CoreContextType context;
                            context.first = module_checksum;
                            std::for_each(module_pointers.begin(), module_pointers.end(), [&](const NX::web::json::value& v) {
                                if (v.is_number()) {
                                    context.second.push_back(v.as_number().to_uint64());
                                }
                            });
                            _core_context_map[it.first] = context;
                        }
                        catch (std::exception& e) {
                            UNREFERENCED_PARAMETER(e);
                        }
                        ::LeaveCriticalSection(&_core_context_map_lock);
                    }
                }
            });
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
}

void NX::global::generate_agent_key() noexcept
{
    assert(!_agent_info.empty());
    if (_agent_info.empty()) {
        return;
    }

    static const std::vector<unsigned char> magic_key({
        0x1C, 0x2B, 0x33, 0x48, 0x56, 0x21, 0x1A, 0xCC,
        0x1C, 0x2B, 0x33, 0x48, 0x56, 0x21, 0x1A, 0xCC,
        0x1C, 0x2B, 0x33, 0x48, 0x56, 0x21, 0x1A, 0xCC,
        0x1C, 0x2B, 0x33, 0x48, 0x56, 0x21, 0x1A, 0xCC
    });


    std::vector<unsigned char> hash;
    if (!nudf::crypto::ToSha256(_agent_info.id().c_str(), (unsigned long)(_agent_info.id().length()*sizeof(wchar_t)), hash)) {
        return;
    }

    assert(32 == hash.size());
    nudf::crypto::CAesKeyBlob keyblob;
    keyblob.SetKey(&magic_key[0], (unsigned long)magic_key.size());
    if (!nudf::crypto::AesEncrypt(keyblob, &hash[0], (unsigned long)hash.size(), 0)) {
        return;
    }

    _agent_key = NX::secure_mem(hash);
}

unsigned long NX::global::time_since_last_heartbeat() noexcept
{
    if (agent_info().empty() || !agent_info().is_heartbeat_time_valid()) {
        return (unsigned long)-1;
    }
    nudf::time::CTime ct_hb(agent_info().heartbeat_time());
    nudf::time::CTime ct_now;
    ct_now.Now();
    return (unsigned long)(ct_now.ToSecondsSince1970Jan1st() - ct_hb.ToSecondsSince1970Jan1st());
}

std::wstring NX::global::create_listen_port() noexcept
{
    std::wstring ws;
    std::wstring seed;
    LARGE_INTEGER li = { 0, 0 };
    swprintf_s(nudf::string::tempstr<wchar_t>(seed, 64), 64, L"%I64d", _start_time);
    li.QuadPart = (LONGLONG)nudf::crypto::ToCrc64(0, seed.c_str(), (ULONG)(seed.length()*sizeof(wchar_t)));
    swprintf_s(nudf::string::tempstr<wchar_t>(ws, 32), 32, L"NXRM_SERV_IPC_%08X%08X", li.HighPart, li.LowPart);
    LOGDBG(L"serv ipc port generated (seed: %s, port: %s)", seed.c_str(), ws.c_str())
    return std::move(ws);
}

void NX::global::initialize_sspi() noexcept
{
    PSecurityFunctionTableW pSft = InitSecurityInterfaceW();
    if (NULL != pSft) {
        ULONG           cPackages = 0;
        PSecPkgInfoW    pPackageInfo = NULL;
        SECURITY_STATUS status = EnumerateSecurityPackagesW(&cPackages, &pPackageInfo);
        if (SEC_E_OK == status && NULL != pPackageInfo) {
            (VOID)FreeContextBuffer(pPackageInfo);
        }
    }
}

namespace {
class nx_key_blob
{
public:
    nx_key_blob(const std::wstring& ring, const std::wstring& id, const std::wstring& key, const std::wstring& time) : _time(0)
    {
        std::string        vring(ring.begin(), ring.end());
        std::vector<UCHAR> vid;
        std::vector<UCHAR> vkey;
        FILETIME           ftime = { 0, 0 };

        if (!nudf::util::encoding::Base64Decode<wchar_t>(id, vid) || vid.size() != 32) {
            return;
        }
        if (vid.size() != 32) {
            return;
        }
        if (!nudf::util::encoding::Base64Decode<wchar_t>(key, vkey)) {
            return;
        }
        // Only support AES 256 in this release
        if (32 != vkey.size() && 16 != vkey.size()) {
            return;
        }
        if (!nudf::string::ToSystemTime<wchar_t>(time, &ftime, NULL)) {
            return;
        }

        nudf::time::CTime timestamp(&ftime);
        _time = (long)timestamp.ToSecondsSince1970Jan1st();

        // good, now we have all the data, pu them to blob
        _blob.resize(sizeof(NXRM_KEY_BLOB), 0);
        data()->KeKeyId.Algorithm = (32 == vkey.size()) ? NXRM_ALGORITHM_AES256 : NXRM_ALGORITHM_AES128;
        data()->KeKeyId.IdSize = (ULONG)(8 + vid.size() + 4);
        memcpy(&data()->KeKeyId.Id[0], vring.c_str(), min(8, vring.length()));
        memcpy(&data()->KeKeyId.Id[8], &vid[0], vid.size());
        memcpy(&data()->KeKeyId.Id[8+ vid.size()], &_time, sizeof(long));
        memcpy(data()->Key, &vkey[0], vkey.size());
    }
    ~nx_key_blob() {}

    nx_key_blob& operator = (const nx_key_blob& other) noexcept
    {
        if (this != &other) {
            _blob = other.blob();
            _time = other.time();
        }
        return *this;
    }

    inline bool empty() const noexcept { return _blob.empty(); }
    inline const std::vector<unsigned char>& blob() const noexcept { return _blob; }
    inline long time() const noexcept { return _time; }
    inline const NXRM_KEY_BLOB* data() const noexcept { return empty() ? NULL : ((const NXRM_KEY_BLOB*)(&_blob[0])); }

    inline unsigned long algorithm() const noexcept { return empty() ? 0 : data()->KeKeyId.Algorithm; }
    inline bool is_algorithm_aes_128() const noexcept { return (!empty() && data()->KeKeyId.Algorithm == NXRM_ALGORITHM_AES128); }
    inline bool is_algorithm_aes_256() const noexcept { return (!empty() && data()->KeKeyId.Algorithm == NXRM_ALGORITHM_AES256); }
    inline NXRM_KEY_BLOB* data() noexcept { return empty() ? NULL : ((NXRM_KEY_BLOB*)(&_blob[0])); }

protected:
    nx_key_blob() {}

private:
    std::vector<unsigned char> _blob;
    long _time;
};
}
std::vector<unsigned char> NX::global::generate_keys_budnle(const std::wstring& s) const noexcept
{
    std::vector<unsigned char>  keys_blob;

    if (!s.empty()) {
        try {
            NX::web::json::value v = NX::web::json::value::parse(s);
            keys_blob = generate_keys_budnle(v);
        }
        catch (std::exception& e) {
            UNREFERENCED_PARAMETER(e);
        }
    }

    return std::move(keys_blob);
}

std::vector<unsigned char> NX::global::generate_keys_budnle(const NX::web::json::value& v) const noexcept
{
    std::vector<unsigned char>  keys_blob;

    try {

        std::vector<nx_key_blob>  nx_key_blobs;

        if (v.is_null()) {
            throw std::exception("empty json object");
        }
        if (!v.is_array()) {
            throw std::exception("invalid json object");
        }
        if (v.size() == 0) {
            throw std::exception("empty json object");
        }
        int key_index = 0;
        std::for_each(v.as_array().begin(), v.as_array().end(), [&](const NX::web::json::value& key_obj) {
            // sanity check
            if (!key_obj.is_object()) {
                return;
            }
            if (!key_obj.has_field(L"ring")) {
                return;
            }
            if (!key_obj.has_field(L"id")) {
                return;
            }
            if (!key_obj.has_field(L"time")) {
                return;
            }
            if (!key_obj.has_field(L"key")) {
                return;
            }

            try {
                nx_key_blob blob(key_obj.at(L"ring").as_string(), key_obj.at(L"id").as_string(), key_obj.at(L"key").as_string(), key_obj.at(L"time").as_string());
                if (!blob.empty()) {
                    if (blob.is_algorithm_aes_256()) {
                        nx_key_blobs.push_back(blob);
                        // Debug
#ifdef _DEBUG
                        std::wstring ws_key_id = nudf::string::FromBytes<wchar_t>(blob.data()->KeKeyId.Id, blob.data()->KeKeyId.IdSize);
                        std::wstring ws_key_data = nudf::string::FromBytes<wchar_t>(blob.data()->Key, blob.is_algorithm_aes_256() ? 32 : 16);
                        LOGDBG(L"Key #%d\r\n\t - Id: %s\r\n\t - value: %s", key_index++, ws_key_id.c_str(), ws_key_data.c_str());
#endif
                    }
                    else {
                        LOGWARN(ERROR_INVALID_DATA, L"key id algorithm is not aes 256 (algorithm = %d) (%s, %s, %s)", blob.algorithm(), key_obj.at(L"ring").as_string().c_str(), key_obj.at(L"id").as_string().c_str(), key_obj.at(L"time").as_string().c_str());
                    }
                }
                else {
                    LOGWARN(ERROR_INVALID_DATA, L"empty key id blob (%s, %s, %s)", key_obj.at(L"ring").as_string().c_str(), key_obj.at(L"id").as_string().c_str(), key_obj.at(L"time").as_string().c_str());
                }
            }
            catch (std::exception& e) {
                UNREFERENCED_PARAMETER(e);
            }
        });

        if (nx_key_blobs.empty()) {
            LOGERR(ERROR_INVALID_DATA, L"No valid key in this key bundle");
            throw std::exception("no key exists");
        }

        // let the latest key be the first one
        std::sort(nx_key_blobs.begin(), nx_key_blobs.end(), [&](const nx_key_blob& left, const nx_key_blob& right) -> bool {
            return (left.time() > right.time());
        });

        // copy all the key blobs to one trunck
        keys_blob.resize(sizeof(NXRM_KEY_BLOB) * nx_key_blobs.size());
        NXRM_KEY_BLOB* p = (NXRM_KEY_BLOB*)(&keys_blob[0]);
        std::for_each(nx_key_blobs.begin(), nx_key_blobs.end(), [&](const nx_key_blob& b) {
            assert(b.blob().size() == sizeof(NXRM_KEY_BLOB));
            memcpy(p++, b.data(), sizeof(NXRM_KEY_BLOB));
        });
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        keys_blob.clear();
    }

    return std::move(keys_blob);
}

bool NX::global::listen()
{
    // Start serv pipe
    try {
        const std::wstring port = create_listen_port();
        _pipe_serv.listen(port);
    }
    catch(const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        return false;
    }

    return true;
}

void NX::global::run_jobs()
{
    if (_agent_info.empty()) {
        //    - register
        _job_pool.add(std::shared_ptr<NX::job>(new NX::job_register()));
    }
    else {
        // load core context only if registration is ready
        load_core_context();
        //    - heartbeat
        _job_pool.add(std::shared_ptr<NX::job>(new NX::job_heartbeat(GLOBAL.agent_info().heartbeat_frequency() * 1000)));
        Sleep(3000);
        //    - audit
        audit_serv().start(GLOBAL.dir_auditlogs());
        _job_pool.add(std::shared_ptr<NX::job>(new NX::job_audit(GLOBAL.agent_info().log_frequency() * 1000)));
        //    - checkupdate
        _job_pool.add(std::shared_ptr<NX::job>(new NX::job_checkupdate()));
    }
}

void NX::global::clear()
{
    _pipe_serv.shutdown();
    _log.Stop();
}

void NX::global::load_configurations(NX::web::json::value& classify_bundle, NX::web::json::value& keys_bundle)
{
    // load agent profile
    try {
        std::wstring agent_info_file = GLOBAL.dir_conf() + L"\\agent_info.sjs";
        _agent_info.load(agent_info_file);
        generate_agent_key();
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        _agent_info.clear();
    }

    // load external users who had logged on this machine
    external_users().load();

    // if agent_info is empty, don't need to continue
    if (_agent_info.empty()) {
        return;
    }

    assert(!_agent_key.empty());

    // load classify profile
    try {
        std::wstring classify_file = GLOBAL.dir_conf() + L"\\agent_classify.sjs";
        std::string s = NX::sconfig::load(classify_file, _agent_key.decrypt());
        if (s.empty()) {
            throw std::exception("empty classify file");
        }
        std::wstring ws = NX::utility::conversions::utf8_to_utf16(s);
        NX::web::json::value v = NX::web::json::value::parse(ws);
        if (v.is_null()) {
            throw std::exception("empty classify profile");
        }

        // set classify
        classify_bundle = v;
        _classify_budnle_hash = global::json_hash(v);
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        LOGWARN(ERROR_NOT_FOUND, L"agent_configuration: invalid/empty classify profile");
    }

    // load keys profile
    try {
        std::wstring keys_file = GLOBAL.dir_conf() + L"\\agent_keys.sjs";
        std::string s = NX::sconfig::load(keys_file, _agent_key.decrypt());
        if (s.empty()) {
            throw std::exception("empty keys file");
        }
        std::wstring ws = NX::utility::conversions::utf8_to_utf16(s);
        NX::web::json::value v = NX::web::json::value::parse(ws);
        if (v.is_null()) {
            throw std::exception("empty keys");
        }

        // set keys
        keys_bundle = v;
        _keys_budnle_hash = global::json_hash(v);
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        LOGWARN(ERROR_NOT_FOUND, L"agent_configuration: invalid/empty key profile");
    }

    // load agent-whitelist profile
    try {
        std::wstring app_whitelist_file = GLOBAL.dir_conf() + L"\\agent_whitelist.sjs";
        std::string s = NX::sconfig::load(app_whitelist_file, _agent_key.decrypt());
        if (s.empty()) {
            throw std::exception("empty app whitelist file");
        }
        std::wstring ws = NX::utility::conversions::utf8_to_utf16(s);
        NX::web::json::value v = NX::web::json::value::parse(ws);
        if (v.is_null()) {
            throw std::exception("empty app whitelist");
        }

        // set app-whitelist
        _whitelist_info.set(v);
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        LOGWARN(ERROR_NOT_FOUND, L"agent_configuration: invalid/empty whitelist profile");
    }

    // load policy profile
    try {
        std::wstring policy_file = GLOBAL.dir_conf() + L"\\agent_policy.sjs";
        std::string s = NX::sconfig::load(policy_file, _agent_key.decrypt());
        if (s.empty()) {
            throw std::exception("empty policy file");
        }
        std::wstring ws = NX::utility::conversions::utf8_to_utf16(s);

        // load timestamp
        try {

            rapidxml::xml_document<wchar_t> doc;
            doc.parse<0>((wchar_t*)ws.c_str());

            rapidxml::xml_node<wchar_t>* root_node = doc.first_node(L"POLICYBUNDLE", 0, false);
            if (NULL == root_node) {
                throw WIN32ERROR2(ERROR_INVALID_DATA);
            }

            std::wstring ws_policy_xml;
            rapidxml::print(std::back_inserter(ws_policy_xml), doc);
            _policy_bundle = NX::EVAL::policy_bundle::load(ws_policy_xml);
            if (_policy_bundle.empty()) {
                LOGWARN(ERROR_INVALID_DATA, L"policy budnle on disk is empty");
            }
            else {
                NX::serv_session::SessionMapType ss;
                serv_session().copy(ss);
                std::for_each(ss.begin(), ss.end(), [&](auto s) {
                    s.second->set_session_policy_bundle(_policy_bundle);
                });
            }
        }
        catch (const nudf::CException& e) {
            _policy_bundle.clear();
            throw std::exception(e.what());
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        LOGWARN(ERROR_NOT_FOUND, L"agent_configuration: invalid/empty policy profile");
    }
}

std::vector<unsigned char> NX::global::string_hash(const std::wstring& v) noexcept
{
    std::vector<unsigned char> hash;
    nudf::crypto::ToSha1(v.c_str(), (unsigned long)(v.length()*sizeof(wchar_t)), hash);
    return std::move(hash);
}

std::vector<unsigned char> NX::global::json_hash(const NX::web::json::value& v) noexcept
{
    std::wstring ws = v.serialize();
    return NX::global::string_hash(ws);
}

bool NX::global::create_profile_folder(const std::wstring& sid, bool fail_if_exist, bool for_everyone)
{
    std::wstring dir_profile = GLOBAL.dir_profiles() + L"\\" + sid;

    // make sure profile folder exists
    if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(dir_profile.c_str())) {
        NX::security_attribute  sa;
        sa.add_acl(sid, GENERIC_ALL, TRUSTEE_IS_USER, SET_ACCESS, SUB_CONTAINERS_AND_OBJECTS_INHERIT);
        sa.add_acl_for_wellknown_group2(SECURITY_NT_AUTHORITY, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, GENERIC_ALL, SUB_CONTAINERS_AND_OBJECTS_INHERIT);
        if (for_everyone) {
            sa.add_acl_for_wellknown_group(SECURITY_WORLD_SID_AUTHORITY, SECURITY_WORLD_RID, GENERIC_READ|GENERIC_WRITE, SUB_CONTAINERS_AND_OBJECTS_INHERIT);
        }
        sa.generate();
        return ::CreateDirectoryW(dir_profile.c_str(), sa) ? true : false;
    }

    return fail_if_exist ? false : true;;
}


//
//
//

static NX::web::json::value init_request(const unsigned char* data, unsigned long size) noexcept
{
    NX::web::json::value request;
    try {
        std::string sreq((char*)data);
        std::wstring wsreq = NX::utility::conversions::utf8_to_utf16(sreq);
        request = NX::web::json::value::parse(wsreq);
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        request = NX::web::json::value();
    }

    return request;
}

static NX::web::json::value on_request_echo(int code, int session_id, const NX::web::json::value& request) noexcept
{
    NX::web::json::value response = NX::web::json::value::object();
    try {
        if (request.has_field(L"message")) {
            response[L"message"] = request.at(L"message");
        }
        else {
            response[L"message"] = NX::web::json::value::string(L"empty");
        }
        response[L"result"] = NX::web::json::value::number(0);
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        response[L"result"] = NX::web::json::value::number(-1);
        response[L"message"] = NX::web::json::value::string(L"invalid request format");
    }

    return response;
}

static NX::web::json::value on_request_logon(int code, int session_id, const NX::web::json::value& request) noexcept
{
    NX::web::json::value response = NX::web::json::value::object();

    if (!request.has_field(L"user")) {
        response[L"result"] = NX::web::json::value::number(ERROR_INVALID_DATA);
        response[L"message"] = NX::web::json::value::string(L"invalid request data (user not exist)");
        return response;
    }
    if (!request.has_field(L"domain")) {
        response[L"result"] = NX::web::json::value::number(ERROR_INVALID_DATA);
        response[L"message"] = NX::web::json::value::string(L"invalid request data (domain not exist)");
        return response;
    }
    if (!request.has_field(L"password")) {
        response[L"result"] = NX::web::json::value::number(ERROR_INVALID_DATA);
        response[L"message"] = NX::web::json::value::string(L"invalid request data (password not exist)");
        return response;
    }

#ifdef _DEBUG
    if (!request.at(L"user").is_string()) {
        response[L"result"] = NX::web::json::value::number(ERROR_INVALID_DATA);
        response[L"message"] = NX::web::json::value::string(L"invalid request data (user is not string)");
        return response;
    }
    if (!request.at(L"domain").is_string()) {
        response[L"result"] = NX::web::json::value::number(ERROR_INVALID_DATA);
        response[L"message"] = NX::web::json::value::string(L"invalid request data (domain is not string)");
        return response;
    }
    if (!request.at(L"password").is_string()) {
        response[L"result"] = NX::web::json::value::number(ERROR_INVALID_DATA);
        response[L"message"] = NX::web::json::value::string(L"invalid request data (password is not string)");
        return response;
    }

    if (request.at(L"user").as_string().empty()) {
        response[L"result"] = NX::web::json::value::number(ERROR_INVALID_DATA);
        response[L"message"] = NX::web::json::value::string(L"invalid request data (user is empty string)");
        return response;
    }
    if (request.at(L"password").as_string().empty()) {
        response[L"result"] = NX::web::json::value::number(ERROR_INVALID_DATA);
        response[L"message"] = NX::web::json::value::string(L"invalid request data (password is empty string)");
        return response;
    }
#endif

    std::shared_ptr<NX::session> sp = GLOBAL.serv_session().get(session_id);
    if (sp == NULL) {
        response[L"result"] = NX::web::json::value::number(ERROR_INVALID_DATA);
        response[L"message"] = NX::web::json::value::string(L"invalid request data (session not found)");
        return response;
    }

    try {
        // log on
        std::wstring logon_user = request.at(L"user").as_string();
        std::wstring logon_domain = request.at(L"domain").as_string();
        std::wstring logon_password = request.at(L"password").as_string();
        if (logon_domain.empty()) {
            if (!GLOBAL.register_info().authn_domains().empty()) {
                logon_domain = GLOBAL.register_info().authn_domains()[0];
            }
        }
        long result = sp->logon(logon_user, logon_domain, logon_password);
        response[L"result"] = NX::web::json::value::number(result);
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        response[L"result"] = NX::web::json::value::number(-1);
        response[L"message"] = NX::web::json::value::string(L"invalid request format");
    }

    return response;
}

static NX::web::json::value on_request_logoff(int code, int session_id, const NX::web::json::value& request) noexcept
{
    NX::web::json::value response = NX::web::json::value::object();

    std::shared_ptr<NX::session> sp = GLOBAL.serv_session().get(session_id);
    if (sp == NULL) {
        response[L"result"] = NX::web::json::value::number(ERROR_INVALID_DATA);
        response[L"message"] = NX::web::json::value::string(L"invalid request data (session not found)");
        return response;
    }

    sp->logoff();
    response[L"result"] = NX::web::json::value::number(0);
    return response;
}

static NX::web::json::value on_request_set_aero_status(int code, int session_id, const NX::web::json::value& request) noexcept
{
    NX::web::json::value response = NX::web::json::value::object();

    if (!request.has_field(L"aero")) {
        response[L"result"] = NX::web::json::value::number(ERROR_INVALID_DATA);
        response[L"message"] = NX::web::json::value::string(L"invalid request data (session not found)");
        return response;
    }

    if (!request.at(L"aero").is_boolean()) {
        response[L"result"] = NX::web::json::value::number(ERROR_INVALID_DATA);
        response[L"message"] = NX::web::json::value::string(L"invalid request data (session not found)");
        return response;
    }

    const bool enabled = request.at(L"aero").as_bool();

    std::shared_ptr<NX::session> sp = GLOBAL.serv_session().get(session_id);
    if (sp == NULL) {
        response[L"result"] = NX::web::json::value::number(ERROR_INVALID_DATA);
        response[L"message"] = NX::web::json::value::string(L"invalid request data (session not found)");
        return response;
    }

    sp->set_dwm_enabled(enabled);
    response[L"result"] = NX::web::json::value::number(0);
    return response;
}

static NX::web::json::value on_request_query_status(int code, int session_id, const NX::web::json::value& request) noexcept
{
    NX::web::json::value response = NX::web::json::value::object();

    std::shared_ptr<NX::session> sp = GLOBAL.serv_session().get(session_id);
    if (sp == NULL) {
        response[L"result"] = NX::web::json::value::number(ERROR_INVALID_DATA);
        response[L"message"] = NX::web::json::value::string(L"invalid request data (session not found)");
        return response;
    }

    return sp->build_serv_status_response();
}

static NX::web::json::value on_request_update(int code, int session_id, const NX::web::json::value& request) noexcept
{
    NX::web::json::value response = NX::web::json::value::object();
    std::shared_ptr<NX::job> sp = GLOBAL.jobs().get(NX::job::jn_heartbeat);
    if (sp != NULL) {
        GLOBAL.set_requestor_session_id(session_id);
        sp->trigger();
        response[L"result"] = NX::web::json::value::number(ERROR_SUCCESS);
    }
    else {
        response[L"result"] = NX::web::json::value::number(ERROR_NOT_FOUND);
        response[L"message"] = NX::web::json::value::string(L"heart-beat job not exists");
    }
    return response;
}

static NX::web::json::value on_enable_debug(int code, int session_id, const NX::web::json::value& request) noexcept
{
    LOGINF(L"log level is set to DEBUG (debug is enabled)");
    NX::web::json::value response = NX::web::json::value::object();
    GLOBAL.log().SetAcceptLevel(LOGDEBUG);
    response[L"result"] = NX::web::json::value::number(ERROR_SUCCESS);
    return response;
}

static NX::web::json::value on_disable_debug(int code, int session_id, const NX::web::json::value& request) noexcept
{
    LOGINF(L"log level is set to WARNING (debug is disabled)");
    NX::web::json::value response = NX::web::json::value::object();
    GLOBAL.log().SetAcceptLevel(LOGWARNING);
    response[L"result"] = NX::web::json::value::number(ERROR_SUCCESS);
    return response;
}

static NX::web::json::value on_collect_debug(int code, int session_id, const NX::web::json::value& request) noexcept
{
    NX::web::json::value response = NX::web::json::value::object();

    try {
        NX::debug_object dbg(session_id);
        if (!dbg.create()) {
            response[L"result"] = NX::web::json::value::number(1);
        }
        else {
            response[L"result"] = NX::web::json::value::number(ERROR_SUCCESS);
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        response[L"result"] = NX::web::json::value::number(1);
    }

    return response;
}

static NX::web::json::value on_query_default_authn_info(int code, int session_id, const NX::web::json::value& request) noexcept
{
    NX::web::json::value response = NX::web::json::value::object();

    try {

        std::shared_ptr<NX::session> sp = GLOBAL.serv_session().get(session_id);
        if (sp == NULL) {
            response[L"result"] = NX::web::json::value::number(ERROR_NOT_FOUND);
            throw std::exception("session not exists");
        }

        std::wstring user_name;
        std::wstring user_domain;

        user_domain = sp->profile().token().user_domain();
        if (user_domain.empty()) {
            if (!GLOBAL.register_info().authn_domains().empty()) {
                user_domain = GLOBAL.register_info().authn_domains()[0];
            }
            if (user_domain.empty()) {
                user_domain = sp->user().domain().empty() ? GLOBAL.host().name() : sp->user().domain();
            }
        }

        user_name = sp->profile().token().user_name();
        if (user_name.empty()) {
            user_name = sp->user().name();
        }

        response[L"result"] = NX::web::json::value::number(ERROR_SUCCESS);
        response[L"user_name"] = NX::web::json::value::string(user_name);
        response[L"user_domain"] = NX::web::json::value::string(user_domain);
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        response[L"result"] = NX::web::json::value::number(1);
    }

    return response;
}