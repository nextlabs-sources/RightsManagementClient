

#pragma once
#ifndef __NUDF_WINDOWS_UTILITY_HPP__
#define __NUDF_WINDOWS_UTILITY_HPP__

#include <Windows.h>

#include <string>
#include <vector>
#include <algorithm>

#include <nudf\bits.hpp>
#include <nudf\filesys.hpp>

namespace NX {
namespace win {


class os_version_basic
{
public:
    os_version_basic();
    virtual ~os_version_basic();

    enum {
        WindowsUnknown = 0,
        Windows2000,
        WindowsXP,
        WindowsXPPro64,
        WindowsServer2003,
        WindowsHomeServer,
        WindowsServer2003R2,
        WindowsVista,
        WindowsServer2008,
        WindowsServer2008R2,
        Windows7,
        WindowsServer2012,
        Windows8,
        WindowsServer2012R2,
        Windows8Point1,
        WindowsServer2016,
        Windows10
    };

    inline bool is_unknown() const { return (_os == WindowsUnknown); }

    inline unsigned long os_type() const { return _os; }
    inline unsigned long cpu_arch() const { return _cpu_arch; }
    inline unsigned long service_pack() const { return _service_pack; }
    inline unsigned long build_number() const { return _build_number; }
    inline unsigned short product_type() const { return _product_type; }
    inline const std::wstring& os_name() const { return _os_name; }
    inline const std::wstring& os_edition() const { return _os_edition; }
    inline const std::wstring& service_pack_name() const { return _service_pack_name; }

    bool is_processor_ARM() const;
    bool is_processor_x86() const;
    bool is_processor_x64() const;
    bool is_processor_IA64() const;

protected:
    virtual void initialize() = 0;
    
protected:
    unsigned long   _os;
    unsigned long   _cpu_arch;
    unsigned long   _service_pack;
    unsigned long   _build_number;
    unsigned short  _product_type;
    std::wstring    _os_name;
    std::wstring    _os_edition;
    std::wstring    _service_pack_name;
};

class os_version_old : public os_version_basic
{
public:
    os_version_old();
    virtual ~os_version_old();

protected:
    virtual void initialize();
};

class os_version_new : public os_version_basic
{
public:
    os_version_new();
    virtual ~os_version_new();

protected:
    virtual void initialize();

private:
    bool version_equal(unsigned long major, unsigned long minor);
    bool service_pack_equal(unsigned short major);
    bool is_workstation();
    bool is_domain_controller();
    bool check_product_suite(unsigned short suite);
    unsigned long get_build_number();
    std::wstring get_edition_name();
};

class language
{
public:
    language();
    explicit language(unsigned short lgid);
    explicit language(LCID lcid);
    virtual ~language();

    language& operator = (const language& other);

    bool empty() const { return (_lcid == 0); }
    void clear() { _lcid = 0; _name.clear(); }
    LCID id() const { return _lcid; }
    unsigned short lang_id() const { return LANGIDFROMLCID(_lcid); }
    unsigned short sort_id() const { return SORTIDFROMLCID(_lcid); }
    const std::wstring& name() const { return _name; }
    
private:
    void set_lcid(LCID lcid);
    void set_lgid(unsigned short lgid);

private:
    LCID _lcid;
    std::wstring _name;
};

class system_default_language : public language
{
public:
    system_default_language();
    ~system_default_language();
};

class user_default_language : public language
{
public:
    user_default_language();
    ~user_default_language();
};

namespace installation {
    
class software
{
public:
    software();
    software(const std::wstring& n, bool is_64bit, std::wstring ver, std::wstring pub, unsigned short lgid, const std::wstring& date, const std::wstring& dir);
    virtual ~software();
    software& operator = (const software& other);

    inline bool x64() const { return _x64; }
    inline const std::wstring name() const { return _name; }
    inline const std::wstring version() const { return _version; }
    inline const std::wstring publisher() const { return _publisher; }
    inline const win::language& lang() const { return _lang; }
    inline const std::wstring& install_date() const { return _install_date; }
    inline const std::wstring install_dir() const { return _install_dir; }

private:
    bool            _x64;
    std::wstring    _name;
    std::wstring    _version;
    std::wstring    _publisher;
    win::language   _lang;
    std::wstring    _install_date;
    std::wstring    _install_dir;
};

#define INSTALLED_SOFTWARE_32BIT  0x00000001
#define INSTALLED_SOFTWARE_64BIT  0x00000002
#define INSTALLED_SOFTWARE_ALL    0x00000003

std::vector<software> get_installed_software(unsigned long flags = INSTALLED_SOFTWARE_ALL);
std::vector<std::wstring> get_installed_kbs();

}   // namespace installation


namespace hardware {

typedef enum CPU_VENDER {
    CV_UNKNOWN = 0,
    CV_INTEL,
    CV_AMD
} CPU_VENDER;

class processor_information
{
    // forward declarations
    class cpu_id_data;

public:
    processor_information();
    ~processor_information();

public:
    // getters
    CPU_VENDER get_vendor() const { return _vendor; }
    const std::wstring& get_vendor_name(void) const throw() { return ((CV_INTEL == _vendor) ? L"GenuineIntel" : ((CV_AMD == _vendor) ? L"AuthenticAMD" : L"Others")); }
    const std::wstring& get_processor_brand(void) const throw() { return _brand; }

    inline unsigned long get_cores_count() const throw() { return _cores; }
    inline unsigned long get_logical_processors_count() const throw() { return _logical_processors; }

    inline bool is_vender_intel() const { return (CV_INTEL == _vendor); }
    inline bool is_vender_amd() const { return (CV_AMD == _vendor); }
    inline bool is_hyperthreads_on() const throw() { return _hyperthreads; }

    inline bool support_SSE3() const throw() { return f_1_ECX_[0]; }
    inline bool support_PCLMULQDQ() const throw() { return f_1_ECX_[1]; }
    inline bool support_MONITOR() const throw() { return f_1_ECX_[3]; }
    inline bool support_SSSE3() const throw() { return f_1_ECX_[9]; }
    inline bool support_FMA() const throw() { return f_1_ECX_[12]; }
    inline bool support_CMPXCHG16B() const throw() { return f_1_ECX_[13]; }
    inline bool support_SSE41() const throw() { return f_1_ECX_[19]; }
    inline bool support_SSE42() const throw() { return f_1_ECX_[20]; }
    inline bool support_MOVBE() const throw() { return f_1_ECX_[22]; }
    inline bool support_POPCNT() const throw() { return f_1_ECX_[23]; }
    inline bool support_AES() const throw() { return f_1_ECX_[25]; }
    inline bool support_XSAVE() const throw() { return f_1_ECX_[26]; }
    inline bool support_OSXSAVE() const throw() { return f_1_ECX_[27]; }
    inline bool support_AVX() const throw() { return f_1_ECX_[28]; }
    inline bool support_F16C() const throw() { return f_1_ECX_[29]; }
    inline bool support_RDRAND() const throw() { return f_1_ECX_[30]; }

    inline bool support_MSR() const throw() { return f_1_EDX_[5]; }
    inline bool support_CX8() const throw() { return f_1_EDX_[8]; }
    inline bool support_SEP() const throw() { return f_1_EDX_[11]; }
    inline bool support_CMOV() const throw() { return f_1_EDX_[15]; }
    inline bool support_CLFSH() const throw() { return f_1_EDX_[19]; }
    inline bool support_MMX() const throw() { return f_1_EDX_[23]; }
    inline bool support_FXSR() const throw() { return f_1_EDX_[24]; }
    inline bool support_SSE() const throw() { return f_1_EDX_[25]; }
    inline bool support_SSE2() const throw() { return f_1_EDX_[26]; }

    inline bool support_FSGSBASE() const throw() { return f_7_EBX_[0]; }
    inline bool support_BMI1() const throw() { return f_7_EBX_[3]; }
    inline bool support_HLE() const throw() { return is_vender_intel() && f_7_EBX_[4]; }
    inline bool support_AVX2() const throw() { return f_7_EBX_[5]; }
    inline bool support_BMI2() const throw() { return f_7_EBX_[8]; }
    inline bool support_ERMS() const throw() { return f_7_EBX_[9]; }
    inline bool support_INVPCID() const throw() { return f_7_EBX_[10]; }
    inline bool support_RTM() const throw() { return is_vender_intel() && f_7_EBX_[11]; }
    inline bool support_AVX512F() const throw() { return f_7_EBX_[16]; }
    inline bool support_RDSEED() const throw() { return f_7_EBX_[18]; }
    inline bool support_ADX() const throw() { return f_7_EBX_[19]; }
    inline bool support_AVX512PF() const throw() { return f_7_EBX_[26]; }
    inline bool support_AVX512ER() const throw() { return f_7_EBX_[27]; }
    inline bool support_AVX512CD() const throw() { return f_7_EBX_[28]; }
    inline bool support_SHA() const throw() { return f_7_EBX_[29]; }

    inline bool support_PREFETCHWT1() const throw() { return f_7_ECX_[0]; }

    inline bool support_LAHF() const throw() { return f_81_ECX_[0]; }
    inline bool support_LZCNT() const throw() { return is_vender_intel() && f_81_ECX_[5]; }
    inline bool support_ABM() const throw() { return is_vender_amd() && f_81_ECX_[5]; }
    inline bool support_SSE4a() const throw() { return is_vender_amd() && f_81_ECX_[6]; }
    inline bool support_XOP() const throw() { return is_vender_amd() && f_81_ECX_[11]; }
    inline bool support_TBM() const throw() { return is_vender_amd() && f_81_ECX_[21]; }

    inline bool support_SYSCALL() const throw() { return is_vender_intel() && f_81_EDX_[11]; }
    inline bool support_MMXEXT() const throw() { return is_vender_amd() && f_81_EDX_[22]; }
    inline bool support_RDTSCP() const throw() { return is_vender_intel() && f_81_EDX_[27]; }
    inline bool support_3DNOWEXT() const throw() { return is_vender_amd() && f_81_EDX_[30]; }
    inline bool support_3DNOW() const throw() { return is_vender_amd() && f_81_EDX_[31]; }

private:
    void load();

private:
    CPU_VENDER      _vendor;
    std::wstring    _brand;
    bool            _hyperthreads;
    unsigned long   _logical_processors;
    unsigned long   _cores;

    bits_map<32> f_1_ECX_;
    bits_map<32> f_1_EDX_;
    bits_map<32> f_7_EBX_;
    bits_map<32> f_7_ECX_;
    bits_map<32> f_81_ECX_;
    bits_map<32> f_81_EDX_;
    std::vector<cpu_id_data> _data;
    std::vector<cpu_id_data> _extdata;

    class cpu_id_data
    {
    public:
        cpu_id_data()
        {
            _data[0] = _data[1] = _data[2] = _data[3] = -1;
        }
        ~cpu_id_data()
        {
        }
        cpu_id_data& operator = (const cpu_id_data& cpudata)
        {
            memcpy(_data, cpudata._data, sizeof(_data));
        }

        inline int* data() throw() { return _data; }
        inline int operator [](int id) const { return ((id >= 0 && id < 4) ? _data[id] : -1); }

        int _data[4];
    };
};

class memory_information
{
public:
    memory_information();
    ~memory_information();

    inline int get_load() const { return (int)_status.dwMemoryLoad; }
    inline __int64 get_physical_total() /*in MB*/ const { return (_status.ullTotalPhys / 1048576 /*1024*1024*/); }
    inline __int64 get_physical_available() /*in MB*/  const { return (_status.ullAvailPhys / 1048576 /*1024*1024*/); }
    inline __int64 get_page_total() /*in MB*/  const { return (_status.ullTotalPageFile / 1048576 /*1024*1024*/); }
    inline __int64 get_page_available() /*in MB*/  const { return (_status.ullAvailPageFile / 1048576 /*1024*1024*/); }
    inline __int64 get_virtual_total() /*in MB*/  const { return (_status.ullTotalVirtual / 1048576 /*1024*1024*/); }
    inline __int64 get_virtual_available() /*in MB*/  const { return (_status.ullAvailVirtual / 1048576 /*1024*1024*/); }

private:
    MEMORYSTATUSEX  _status;
};

class network_adapter_information
{
public:
    network_adapter_information();
    network_adapter_information(const std::wstring& adapter_name,
        const std::wstring& friendly_name,
        const std::wstring& description,
        const std::wstring& physical_address,
        unsigned long if_type,
        unsigned long oper_status,
        bool ipv4_enabled,
        bool ipv6_enabled,
        const std::vector<std::wstring>& ipv4_addresses,
        const std::vector<std::wstring>& ipv6_addresses
        );
    network_adapter_information(const void* adapter_data);
    ~network_adapter_information();

    inline bool empty() const { return _adapter_name.empty(); }
    inline const std::wstring& get_adapter_name() const { return _adapter_name; }
    inline const std::wstring& get_friendly_name() const { return _friendly_name; }
    inline const std::wstring& get_description() const { return _description; }
    inline const std::wstring& get_mac_address() const { return _physical_address; }
    inline unsigned long get_if_type() const { return _if_type; }
    inline unsigned long get_oper_status() const { return _oper_status; }
    inline bool is_ipv4_enabled() const { return _ipv4_enabled; }
    inline bool is_ipv6_enabled() const { return _ipv6_enabled; }
    inline const std::vector<std::wstring>& get_ipv4_addresses() const { return _ipv4_addresses; }
    inline const std::vector<std::wstring>& get_ipv6_addresses() const { return _ipv6_addresses; }

    network_adapter_information& operator = (const network_adapter_information& other);
    void clear();

    std::wstring get_if_type_name() const;
    std::wstring get_oper_status_name() const;

    bool is_ethernet_adapter() const;
    bool is_ppp_adapter() const;
    bool is_80211_adapter() const;
    bool is_1394_adapter() const;
    bool is_network_adapter() const;

    bool is_active() const;
    bool is_connected() const;

    
protected:
    void load(const void* adapter_data);

private:
    std::wstring        _adapter_name;
    std::wstring        _friendly_name;
    std::wstring        _description;
    std::wstring        _physical_address;
    unsigned long       _if_type;
    unsigned long       _oper_status;
    bool                _ipv4_enabled;
    bool                _ipv6_enabled;
    std::vector<std::wstring>   _ipv4_addresses;
    std::vector<std::wstring>   _ipv6_addresses;
};

std::vector<network_adapter_information> get_all_adapters();
std::vector<network_adapter_information> get_all_network_adapters();
std::vector<network_adapter_information> get_active_network_adapters();

}   // namespace hardware


class file_version
{
public:
    file_version();
    file_version(const std::wstring& file);
    ~file_version();

    inline const std::wstring& get_company_name() const { return _company_name; }
    inline const std::wstring& get_product_name() const { return _product_name; }
    inline const std::wstring& get_product_version_string() const { return _product_version_string; }
    inline const std::wstring& get_file_name() const { return _file_name; }
    inline const std::wstring& get_file_description() const { return _file_description; }
    inline const std::wstring& get_file_version_string() const { return _file_version_string; }

    inline ULARGE_INTEGER get_product_version() const { return _product_version; }
    inline unsigned short get_product_version_major() const { return ((unsigned short)(_product_version.HighPart >> 16)); }
    inline unsigned short get_product_version_minor() const { return ((unsigned short)_product_version.HighPart); }
    inline unsigned short get_product_version_build() const { return ((unsigned short)(_product_version.LowPart >> 16)); }
    inline unsigned short get_product_version_patch() const { return ((unsigned short)_product_version.LowPart); }

    inline ULARGE_INTEGER get_file_version() const { return _file_version; }
    inline unsigned short get_file_version_major() const { return ((unsigned short)(_file_version.HighPart >> 16)); }
    inline unsigned short get_file_version_minor() const { return ((unsigned short)_file_version.HighPart); }
    inline unsigned short get_file_version_build() const { return ((unsigned short)(_file_version.LowPart >> 16)); }
    inline unsigned short get_file_version_patch() const { return ((unsigned short)_file_version.LowPart); }

    inline FILETIME get_file_time() const { return _file_time; }
    inline unsigned long get_file_flags() const { return _file_flags; }
    inline unsigned long get_file_os() const { return _file_os; }
    inline unsigned long get_file_type() const { return _file_type; }
    inline unsigned long get_file_subtype() const { return _file_subtype; }

    // flags
    inline bool is_flag_debug_on() const { return (VS_FF_DEBUG == (VS_FF_DEBUG & _file_flags)); }
    inline bool is_flag_patched_on() const { return (VS_FF_PATCHED == (VS_FF_PATCHED & _file_flags)); }
    inline bool is_flag_prerelease_on() const { return (VS_FF_PRERELEASE == (VS_FF_PRERELEASE & _file_flags)); }
    inline bool is_flag_private_build_on() const { return (VS_FF_PRIVATEBUILD == (VS_FF_PRIVATEBUILD & _file_flags)); }
    inline bool is_flag_special_build_on() const { return (VS_FF_SPECIALBUILD == (VS_FF_SPECIALBUILD & _file_flags)); }

    // os
    inline bool is_os_unknown() const { return (VOS_UNKNOWN == _file_os); }
    inline bool is_os_dos() const { return (VOS_DOS == (VOS_DOS & _file_os)); }
    inline bool is_os_dos_win16() const { return (VOS_DOS_WINDOWS16 == _file_os); }
    inline bool is_os_dos_win32() const { return (VOS_DOS_WINDOWS32 == _file_os); }
    inline bool is_os_nt() const { return (VOS_NT == (VOS_NT & _file_os)); }
    inline bool is_os_nt_win32() const { return (VOS_NT_WINDOWS32 == _file_os); }
    inline bool is_os_win16() const { return (VOS__WINDOWS16 == (VOS__WINDOWS16 & _file_os)); }
    inline bool is_os_win32() const { return (VOS__WINDOWS32 == (VOS__WINDOWS32 & _file_os)); }
    inline bool is_os_os216() const { return (VOS_OS216 == (VOS_OS216 & _file_os)); }
    inline bool is_os_os232() const { return (VOS_OS232 == (VOS_OS232 & _file_os)); }
    inline bool is_os_pm16() const { return (VOS__PM16 == (VOS__PM16 & _file_os)); }
    inline bool is_os_pm32() const { return (VOS__PM32 == (VOS__PM32 & _file_os)); }
    inline bool is_os_os216_pm16() const { return (VOS_OS216_PM16 == _file_os); }
    inline bool is_os_os232_pm32() const { return (VOS_OS232_PM32 == _file_os); }

    // file type
    inline bool is_unknown_type() const { return (VFT_UNKNOWN == _file_type); }
    inline bool is_application() const { return (VFT_APP == _file_type); }
    inline bool is_dll() const { return (VFT_DLL == _file_type); }
    inline bool is_driver() const { return (VFT_DRV == _file_type); }
    inline bool is_font() const { return (VFT_FONT == _file_type); }
    inline bool is_static_lib() const { return (VFT_STATIC_LIB == _file_type); }
    inline bool is_vxd() const { return (VFT_VXD == _file_type); }

    // sub-type
    inline bool is_printer_driver() const { return (is_driver() && VFT2_DRV_PRINTER == _file_subtype); }
    inline bool is_communication_driver() const { return (is_driver() && VFT2_DRV_COMM == _file_subtype); }
    inline bool is_keyboard_driver() const { return (is_driver() && VFT2_DRV_KEYBOARD == _file_subtype); }
    inline bool is_language_driver() const { return (is_driver() && VFT2_DRV_LANGUAGE == _file_subtype); }
    inline bool is_display_driver() const { return (is_driver() && VFT2_DRV_DISPLAY == _file_subtype); }
    inline bool is_mouse_driver() const { return (is_driver() && VFT2_DRV_MOUSE == _file_subtype); }
    inline bool is_network_driver() const { return (is_driver() && VFT2_DRV_NETWORK == _file_subtype); }
    inline bool is_system_driver() const { return (is_driver() && VFT2_DRV_SYSTEM == _file_subtype); }
    inline bool is_sound_driver() const { return (is_driver() && VFT2_DRV_SOUND == _file_subtype); }
    inline bool is_versioned_printer_driver() const { return (is_driver() && VFT2_DRV_VERSIONED_PRINTER == _file_subtype); }
    inline bool is_raster_font() const { return (is_font() && VFT2_FONT_RASTER == _file_subtype); }
    inline bool is_truetype_font() const { return (is_font() && VFT2_FONT_TRUETYPE == _file_subtype); }
    inline bool is_vector_font() const { return (is_font() && VFT2_FONT_VECTOR == _file_subtype); }

    inline bool empty() const { return (0 == _file_type); }

    void clear();
    file_version& operator = (const file_version& other);
    bool operator == (const file_version& other) const;

protected:
    void load(const std::wstring& file);
    std::wstring load_string(void* data, const std::wstring& name, unsigned short language, unsigned short codepage);

private:
    std::wstring _company_name;
    std::wstring _product_name;
    std::wstring _product_version_string;
    std::wstring _file_name;
    std::wstring _file_description;
    std::wstring _file_version_string;
    ULARGE_INTEGER  _product_version;
    ULARGE_INTEGER  _file_version;
    FILETIME        _file_time;
    unsigned long   _file_flags;
    unsigned long   _file_os;
    unsigned long   _file_type;
    unsigned long   _file_subtype;
};


class pe_file
{
public:
    pe_file();
    pe_file(const std::wstring& file);
    virtual ~pe_file();

    void load(const std::wstring& file);
    void clear();
    pe_file& operator = (const pe_file& other);

    inline bool empty() const { return (0 == _machine); }
    inline const std::wstring& get_image_publisher() const { return _image_publisher; }
    inline unsigned short get_machine_code() const { return _machine; }
    inline unsigned short get_characteristics() const { return _characteristics; }
    inline unsigned short get_subsystem() const { return _subsystem; }
    inline unsigned long get_image_checksum() const { return _image_checksum; }
    inline unsigned __int64 get_image_base() const { return _image_base; }
    inline unsigned long get_base_of_code() const { return _base_of_code; }
    inline unsigned long get_address_of_entry() const { return _address_of_entry; }
    inline const NX::win::file_version& get_file_version() const { return _file_version; }

    inline bool is_x86_image() const { return (_machine == IMAGE_FILE_MACHINE_I386); }
    inline bool is_x64_image() const { return (_machine == IMAGE_FILE_MACHINE_AMD64); }
    inline bool is_ia64_image() const { return (_machine == IMAGE_FILE_MACHINE_IA64); }

    inline bool is_exe() const { return (_characteristics == IMAGE_FILE_EXECUTABLE_IMAGE); }
    inline bool is_dll() const { return (_characteristics == IMAGE_FILE_DLL); }
    inline bool is_driver() const { return (_characteristics == IMAGE_FILE_SYSTEM); }

    inline bool is_subsystem_win32_gui() const { return (_subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI); }

protected:
    bool load_pe_header(const std::wstring& file);
    void load_signature(const std::wstring& file);

private:
    std::wstring     _image_publisher;
    unsigned short   _machine;
    unsigned short   _characteristics;
    unsigned short   _subsystem;
    unsigned long    _image_checksum;
    unsigned __int64 _image_base;
    unsigned long    _base_of_code;
    unsigned long    _address_of_entry;
    NX::win::file_version   _file_version;
};

class reg_key
{
public:
    reg_key();
    virtual ~reg_key();

    typedef enum reg_position {
        reg_default = 0,
        reg_wow64_32 = 1,
        reg_wow64_64 = 2
    } reg_position;

    virtual void create(HKEY root, const std::wstring& path, reg_position pos);
    virtual void open(HKEY root, const std::wstring& path, reg_position pos, bool read_only);
    void close();

    static void remove(HKEY root, const std::wstring& path);
    static bool exist(HKEY root, const std::wstring& path) noexcept;

    inline bool opened() const { return (NULL != _h); }
    inline operator HKEY() const { return _h; }

#define MAX_KEY_NAME    255
#define MAX_VALUE_NAME  16383

    // _Fn ==> void Fn(const wchar_t* name)
    template<class _Fn>
    inline void enum_sub_keys(_Fn& func)
    {
        int index = 0;

        while (true) {
            // according to MSDN, key name's max length is 255
            // so MAX_PATH is enough
            std::vector<wchar_t> name_buf;
            name_buf.resize(MAX_KEY_NAME + 1, 0);
            unsigned long name_length = MAX_KEY_NAME + 1;
            if (ERROR_SUCCESS != RegEnumKeyW(_h, index++, name_buf.data(), name_length)) {
                break;
            }
            func(name_buf.data());
        }
    }

    // _Fn ==> void Fn(const wchar_t* value_name, unsigned long value_type, unsigned long value_size)
    template<class _Fn>
    inline void enum_sub_values(_Fn& func)
    {
        int index = 0;
        // Although MSDN says that
        // Value name's size limit is 16,383 characters
        // We only support MAX_PATH here to handle most of the normal case
        std::vector<wchar_t> value_name_buf(MAX_VALUE_NAME + 1, 0);

        while (true) {

            unsigned long value_type = 0;
            unsigned name_length = MAX_VALUE_NAME + 1;
            unsigned data_length = 0;

            unsigned long result = RegEnumValue(_h, index, value_name_buf.data(), &name_length, NULL, &value_type, NULL, &data_length);
            if (ERROR_NO_MORE_ITEMS == result) {
                break;
            }
            if (ERROR_SUCCESS != result) {
                if (ERROR_MORE_DATA != result) {
                    break;
                }
            }

            ++index;
            func(name_buf.data(), value_type, data_length);
        }
    }

    std::wstring read_default_value();
    void set_default_value(const std::wstring& value);
    
    void read_value(const std::wstring& name, unsigned long* value);
    void read_value(const std::wstring& name, unsigned __int64* value);
    void read_value(const std::wstring& name, std::wstring& value);
    void read_value(const std::wstring& name, std::vector<std::wstring>& value);
    void read_value(const std::wstring& name, std::vector<unsigned char>& value);

    void set_value(const std::wstring& name, unsigned long value);
    void set_value(const std::wstring& name, unsigned __int64 value);
    void set_value(const std::wstring& name, const std::wstring& value, bool expandable = false);
    void set_value(const std::wstring& name, const std::vector<std::wstring>& value);
    void set_value(const std::wstring& name, const std::vector<unsigned char>& value);

protected:
    std::wstring expand_env_string(const std::wstring& s);
    std::vector<std::wstring> expand_multi_strings(const wchar_t* s);
    std::vector<wchar_t> create_multi_strings_buffer(const std::vector<std::wstring>& strings);
    unsigned long convert_endian(unsigned long u);

    std::vector<unsigned char> internal_read_value(const std::wstring& name, unsigned long* value_type);
    void internal_set_value(const std::wstring& name, unsigned long value_type, const void* value, unsigned long size);

private:
    // No copy allowed
    reg_key& operator = (const reg_key& other) { return *this; }

private:
    HKEY    _h;
};

template <HKEY root>
class reg_root_key : public reg_key
{
public:
    reg_root_key() : reg_key() {}
    virtual ~reg_root_key() {}

    virtual void create(const std::wstring& path, reg_position pos)
    {
        reg_key::create(root, path, pos);
    }
    virtual void open(const std::wstring& path, reg_position pos, bool read_only)
    {
        reg_key::open(root, path, pos, read_only);
    }

private:
    virtual void create(HKEY root, const std::wstring& path, reg_position pos) {}
    virtual void open(HKEY root, const std::wstring& path, reg_position pos, bool read_only) {}
};

typedef reg_root_key<HKEY_LOCAL_MACHINE>    reg_local_machine;
typedef reg_root_key<HKEY_CURRENT_USER>     reg_current_user;


class sid
{
public:
    sid();
    explicit sid(PSID p);
    sid(const std::wstring& s);
    ~sid();

    static sid create(PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority,
                      BYTE nSubAuthorityCount,
                      DWORD dwSubAuthority0,
                      DWORD dwSubAuthority1,
                      DWORD dwSubAuthority2,
                      DWORD dwSubAuthority3,
                      DWORD dwSubAuthority4,
                      DWORD dwSubAuthority5,
                      DWORD dwSubAuthority6,
                      DWORD dwSubAuthority7);

    static const SID_IDENTIFIER_AUTHORITY null_authority;
    static const SID_IDENTIFIER_AUTHORITY world_authority;
    static const SID_IDENTIFIER_AUTHORITY nt_authority;

    static std::wstring serialize(PSID psid);

    static bool is_null_auth(PSID psid);
    static bool is_world_auth(PSID psid);
    static bool is_nt_auth(PSID psid);
    static bool is_everyone_sid(PSID psid);
    static bool is_nt_local_system_sid(PSID psid);
    static bool is_nt_local_service_sid(PSID psid);
    static bool is_nt_network_service_sid(PSID psid);
    static bool is_nt_domain_sid(PSID psid);
    static bool is_nt_builtin_sid(PSID psid);

    inline operator PSID() const { return _sid; }
    inline bool empty() const { return (NULL == _sid); }

    void clear();
    unsigned long length() const;
    sid& operator = (const sid& other);
    sid& operator = (PSID other);
    std::wstring serialize() const;
    bool operator == (const sid& other) const;
    bool operator == (PSID other) const;

private:
    PSID    _sid;
};

class host
{
public:
    host();
    ~host();

    inline const std::wstring& fqdn_name() const { return _fqdn; }
    inline const std::wstring& dns_host_name() const { return _host; }
    inline const std::wstring& dns_domain_name() const { return _domain; }
    inline bool empty() const { return _fqdn.empty(); }
    inline bool in_domain() const { return !_domain.empty(); }

    void clear();
    host& operator = (const host& other);
    bool operator == (const host& other) const;
    bool operator == (const std::wstring& other) const;

private:
    std::wstring    _host;
    std::wstring    _domain;
    std::wstring    _fqdn;
};

std::wstring sam_compatiple_name_to_principle_name(const std::wstring& name);
std::wstring principle_name_to_sam_compatiple_name(const std::wstring& name);
std::wstring get_object_name(PSID psid);

class user_or_group
{
public:
    user_or_group();
    user_or_group(const std::wstring& uid, const std::wstring& uname);
    virtual ~user_or_group();

    inline const std::wstring& id() const { return _id; }
    inline const std::wstring& name() const { return _name; }

    user_or_group& operator = (const user_or_group& other);
    bool operator == (const user_or_group& other) const;
        
private:
    std::wstring    _id;    // user sid
    std::wstring    _name;  // principle name: user@domain
};

class token
{
public:
    token();
    token(HANDLE h);
    virtual ~token();

    inline operator HANDLE () const { return _h; }
    inline bool empty() const { return (NULL == _h); }
    inline void clear() { if (!empty()) { CloseHandle(_h); _h = NULL; } }

    user_or_group get_user() const;
    std::vector<user_or_group> get_user_groups() const;

private:
    // Copy is not allowed
    token& operator = (const token& tk) { return *this; }

protected:
    HANDLE _h;
};

class process_token : public token
{
public:
    process_token();
    process_token(unsigned long process_id);
    virtual ~process_token();
};

class session_token : public token
{
public:
    session_token();
    session_token(unsigned long session_id);
    virtual ~session_token();
};




}
}



#endif