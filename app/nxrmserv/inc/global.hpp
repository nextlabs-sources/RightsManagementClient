
#pragma once
#ifndef __NXRMSERV_GLOBAL_HPP__
#define __NXRMSERV_GLOBAL_HPP__


#include <string>
#include <map>

#include <nudf\dbglog.hpp>
#include <nudf\winutil.hpp>

#include "process.hpp"

class nxrm_global
{
public:
    nxrm_global();
    ~nxrm_global();

    //
    //  inline functions
    //
    inline const NX::win::file_version& get_file_version() const { return _file_version; }
    inline const std::wstring& get_company_name() const { return _file_version.get_company_name(); }
    inline const std::wstring& get_product_name() const { return _file_version.get_product_name(); }
    inline const std::wstring& get_product_version_string() const { return _file_version.get_product_version_string(); }
    inline const std::wstring& get_file_name() const { return _file_version.get_file_name(); }
    inline const std::wstring& get_file_version_string() const { return _file_version.get_file_version_string(); }

    // dirs
    inline const std::wstring& get_product_dir() const { return _product_dir; }
    inline const std::wstring& get_bin_dir() const { return _bin_dir; }
#ifdef _AMD64_
    inline const std::wstring& get_bin32_dir() const { return _bin32_dir; }
#endif
    inline const std::wstring& get_working_dir() const { return _working_dir; }
    inline const std::wstring& get_config_dir() const { return _config_dir; }
    inline const std::wstring& get_profiles_dir() const { return _profiles_dir; }

    // host
    inline const NX::win::host& get_host() const { return _host; }
    inline const std::wstring& get_host_name() const { return _host.in_domain() ? _host.fqdn_name() : _host.dns_host_name(); }
    inline bool in_domain() const { return _host.in_domain(); }

    // cpu
    inline const NX::win::hardware::processor_information& get_cpu_info() const { return _cpu_info; }
    inline const std::wstring& get_cpu_vender() const { return _cpu_info.get_vendor_name(); }
    inline const std::wstring& get_cpu_brand() const { return _cpu_info.get_processor_brand(); }
    inline unsigned long get_cpu_cores() const { return _cpu_info.get_cores_count(); }
    inline unsigned long get_cpu_logical_processors() const { return _cpu_info.get_logical_processors_count(); }

    // os
    inline const NX::win::os_version_new& get_os_version() const { return _os_version; }
    inline const std::wstring& get_os_name() const { return _os_version.os_name(); }

    // rights management data
    inline process_cache get_process_cache() { return _process_cache; }

    //
    // initialize & cleanup
    //
    void initialize();
    void clear();

protected:
    void init_dirs();

private:
    // no copy is allowed
    nxrm_global& operator = (const nxrm_global& other) { return *this; }

private:
    NX::win::file_version _file_version;
    NX::win::hardware::processor_information _cpu_info;

    std::wstring    _module_path;
    std::wstring    _product_dir;
    std::wstring    _bin_dir;
#ifdef _AMD64_
    std::wstring    _bin32_dir;
#endif
    std::wstring    _working_dir;
    std::wstring    _config_dir;
    std::wstring    _profiles_dir;

    // hardware & software information
    NX::win::host   _host;
    NX::win::os_version_new _os_version;

    process_cache   _process_cache;
};

extern nxrm_global GLOBAL;

BOOL __stdcall GlobalLogAccept(_In_ ULONG Level);
LONG __stdcall GlobalLogWrite(_In_ LPCWSTR Info);


#endif