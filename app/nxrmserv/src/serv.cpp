

#include <Windows.h>
#include <assert.h>
#include <Wtsapi32.h>

#include <string>
#include <iostream>

#include <nudf\eh.hpp>
#include <nudf\winutil.hpp>
#include <nudf\string.hpp>
#include <nudf\dbglog.hpp>
#include <nudf\filesys.hpp>

#include "nxrmserv.hpp"
#include "global.hpp"
#include "serv.hpp"



DECLARE_GLOBAL_LOG_INSTANCE()
DECLARE_LOG_CALLBACKS()


rmserv::rmserv() : NX::win::service_instance(NXRMS_SERVICE_NAME)
{
}

rmserv::~rmserv()
{
}

void rmserv::on_start(int argc, const wchar_t** argv)
{
    //
    // the first thing is to initialize log system
    //
    init_log();
    GLOBAL.initialize();

    // Log initialize information
    log_init_info();

    // Start NXRM drivers
    //   nxrmdrv.sys
    //   nxrmflt.sys

    // Collect all the existing session id
    const std::vector<unsigned long>& existing_session_list = winsession_manager::find_existing_session();
    if (GLOBAL_LOG_ACCEPT(NX::dbg::LL_DEBUG)) {
        std::wstring str_existing_session;
        std::for_each(existing_session_list.begin(), existing_session_list.end(), [&](const unsigned long& session_id) {
            if (!str_existing_session.empty()) {
                str_existing_session += L", ";
            }
            str_existing_session += std::to_wstring((int)session_id);
        });
        LOGDEBUG(NX::string_formater(L"Existing Session: %s", str_existing_session.c_str()));
    }

    // Add existing sessions to session_manager
    std::for_each(existing_session_list.begin(), existing_session_list.end(), [&](const unsigned long& session_id) {
        _win_session_manager.add_session(session_id);
    });
}

void rmserv::on_stop() noexcept
{
    // Shutdown session manager
    _win_session_manager.clear();

    //
    // the last thing: shutdown log system
    //
    GLOBAL_LOG_SHUTDOWN();
}

void rmserv::on_pause()
{
}

void rmserv::on_resume()
{
}

void rmserv::on_preshutdown() noexcept
{
    _win_session_manager.clear();
}

void rmserv::on_shutdown() noexcept
{
}

long rmserv::on_session_logon(_In_ WTSSESSION_NOTIFICATION* wtsn) noexcept
{
    if (wtsn->dwSessionId != 0 && wtsn->dwSessionId != 0xFFFFFFFF) {
        _win_session_manager.add_session(wtsn->dwSessionId);
    }

    return 0;
}

long rmserv::on_session_logoff(_In_ WTSSESSION_NOTIFICATION* wtsn) noexcept
{
    if (wtsn->dwSessionId != 0 && wtsn->dwSessionId != 0xFFFFFFFF) {
        _win_session_manager.remove_session(wtsn->dwSessionId);
    }

    return 0;
}

void rmserv::init_log()
{
    const NX::fs::module_path image_path(NULL);
    const NX::fs::dos_filepath parent_dir(image_path.file_dir());
    std::wstring logfile_path = parent_dir.file_dir() + L"\\DebugDump.txt";
    GLOBAL_LOG_CREATE(logfile_path, NX::dbg::LL_DEBUG, 5242880/*5MB*/, 10);
}

void rmserv::log_init_info()
{
    NX::win::system_default_language sys_lang;
    NX::win::hardware::memory_information mem_info;
    const std::vector<NX::fs::drive>& drives = NX::fs::get_logic_drives();
    const std::vector<NX::win::hardware::network_adapter_information>& adapters = NX::win::hardware::get_all_network_adapters();


    LOGINFO(L" ");
    LOGINFO(L" ");
    LOGINFO(L" ");
    LOGINFO(L"***********************************************************");
    LOGINFO(L"*                 NextLabs Rights Management              *");
    LOGINFO(L"***********************************************************");
    LOGINFO(L" ");
    LOGINFO(NX::string_formater(L"Product Information"));
    LOGINFO(NX::string_formater(L"    - Name:      %s", GLOBAL.get_product_name().c_str()));
    LOGINFO(NX::string_formater(L"    - Company:   %s", GLOBAL.get_company_name().c_str()));
    LOGINFO(NX::string_formater(L"    - Version:   %s", GLOBAL.get_product_version_string().c_str()));
    LOGINFO(NX::string_formater(L"    - Directory: %s", GLOBAL.get_product_dir().c_str()));
    LOGINFO(NX::string_formater(L"OS Information"));
    LOGINFO(NX::string_formater(L"    - Name:      %s %s %s (Build %d)", GLOBAL.get_os_version().os_name().c_str(), GLOBAL.get_os_version().os_edition().c_str(), GLOBAL.get_os_version().is_processor_x64() ? L"64 Bits" : L"32 Bits", GLOBAL.get_os_version().build_number()));
    LOGINFO(NX::string_formater(L"    - Language:  %s", sys_lang.name().c_str()));
    LOGINFO(NX::string_formater(L"Hardware Information"));
    LOGINFO(NX::string_formater(L"    - CPU:       %s", GLOBAL.get_cpu_brand().c_str()));
    LOGINFO(NX::string_formater(L"    - Memory:    %.1f GB", (mem_info.get_physical_total() / 1024.0)));
    LOGINFO(NX::string_formater(L"    - Drives"));
    for (int i = 0; i < (int)drives.size(); i++) {
        LOGINFO(NX::string_formater(L"      #%d: %c:", i, drives[i].drive_letter()));
        LOGINFO(NX::string_formater(L"         > Type: %s", drives[i].type_name().c_str()));
        LOGINFO(NX::string_formater(L"         > Dos Name: %s", drives[i].dos_name().c_str()));
        LOGINFO(NX::string_formater(L"         > NT Name: %s", drives[i].nt_name().c_str()));
        if (drives[i].is_fixed() || drives[i].is_removable() || drives[i].is_ramdisk()) {
            const NX::fs::drive::space& drive_space = drives[i].get_space();
            if (0 != drive_space.total_bytes()) {
                LOGINFO(NX::string_formater(L"         > Total space: %d MB", (int)(drive_space.total_bytes() / 1048576)));
                LOGINFO(NX::string_formater(L"         > Free space: %d MB (%.1f%%)", (int)(drive_space.available_free_bytes() / 1048576), 100 * ((float)drive_space.available_free_bytes() / (float)drive_space.total_bytes())));
            }
            else {
                if (drives[i].is_removable()) {
                    LOGINFO(L"         > No media");
                }
            }
        }

    }

    LOGINFO(NX::string_formater(L"    - Network Adapters"));
    for (int i = 0; i < (int)adapters.size(); i++) {
        const std::wstring& if_type_name = adapters[i].get_if_type_name();
        const std::wstring& oper_status_name = adapters[i].get_oper_status_name();
        LOGINFO(NX::string_formater(L"      #%d: %s", i, adapters[i].get_adapter_name().c_str()));
        LOGINFO(NX::string_formater(L"         > Friendly name: %s", adapters[i].get_friendly_name().c_str()));
        LOGINFO(NX::string_formater(L"         > Description: %s", adapters[i].get_description().c_str()));
        LOGINFO(NX::string_formater(L"         > IfType: %s", if_type_name.c_str()));
        LOGINFO(NX::string_formater(L"         > OperStatus: %s", oper_status_name.c_str()));
        LOGINFO(NX::string_formater(L"         > MAC: %s", adapters[i].get_mac_address().c_str()));
        if (adapters[i].is_connected()) {
            LOGINFO(NX::string_formater(L"         > IPv4: %s", adapters[i].get_ipv4_addresses().empty() ? L"" : adapters[i].get_ipv4_addresses()[0].c_str()));
            LOGINFO(NX::string_formater(L"         > IPv6: %s", adapters[i].get_ipv6_addresses().empty() ? L"" : adapters[i].get_ipv6_addresses()[0].c_str()));
        }
    }
}


//
//  class rmserv_conf
//
rmserv_conf::rmserv_conf() : _delay_seconds(0), _log_level(NX::dbg::LL_DEBUG), _log_size(RMS_DEFAULT_LOGSIZE)
{
}

rmserv_conf::rmserv_conf(const std::wstring& key_path) : _delay_seconds(0), _log_level(NX::dbg::LL_DEBUG), _log_size(RMS_DEFAULT_LOGSIZE)
{
    load(key_path);
}

rmserv_conf::rmserv_conf(unsigned long log_level, unsigned long log_size, unsigned long delay_time)
    : _delay_seconds(delay_time), _log_level(log_level), _log_size((0 == log_size) ? RMS_DEFAULT_LOGSIZE : log_size)
{
}

rmserv_conf::~rmserv_conf()
{
}

rmserv_conf& rmserv_conf::operator = (const rmserv_conf& other)
{
    if (this != &other) {
        _delay_seconds = other.get_delay_seconds();
        _log_level = other.get_log_level();
        _log_size = other.get_log_size();
    }
    return *this;
}

void rmserv_conf::load(const std::wstring& key_path) noexcept
{
    try {

        NX::win::reg_local_machine rgk;

        rgk.open(key_path.empty() ? NXRMS_SERVICE_KEY_PARAMETER : key_path, NX::win::reg_key::reg_wow64_64, true);

        try {
            rgk.read_value(RMS_CONF_DELAY_SECONDS, &_delay_seconds);
        }
        catch (std::exception& e) {
            UNREFERENCED_PARAMETER(e);
            _delay_seconds = 0;
        }

        try {
            rgk.read_value(RMS_CONF_LOG_LEVEL, &_log_level);
        }
        catch (std::exception& e) {
            UNREFERENCED_PARAMETER(e);
            _log_level = NX::dbg::LL_DEBUG;
        }

        try {
            rgk.read_value(RMS_CONF_LOG_SIZE, &_log_size);
        }
        catch (std::exception& e) {
            UNREFERENCED_PARAMETER(e);
            _log_size = 0;
        }
        if (0 == _log_size) {
            _log_size = RMS_DEFAULT_LOGSIZE;
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
}

void rmserv_conf::apply(const std::wstring& key_path)
{
    try {

        unsigned long ul = 0;
        NX::win::reg_local_machine rgk;

        rgk.create(key_path.empty() ? NXRMS_SERVICE_KEY_PARAMETER : key_path, NX::win::reg_key::reg_wow64_64);

        rgk.set_value(RMS_CONF_DELAY_SECONDS, _delay_seconds);
        rgk.set_value(RMS_CONF_LOG_LEVEL, _log_level);
        rgk.set_value(RMS_CONF_LOG_SIZE, _log_size);
    }
    catch (std::exception& e) {
        throw e;
    }
}