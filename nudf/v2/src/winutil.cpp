

#include <winsock2.h>
#include <Windows.h>
#include <assert.h>
#include <iphlpapi.h>
#include <Ws2tcpip.h>
#include <Wtsapi32.h>
#define SECURITY_WIN32
#include <security.h>
#include <Sddl.h>

#include <versionhelpers.h>

#include <set>
#include <boost/algorithm/string.hpp>

#include <nudf\eh.hpp>
#include <nudf\debug.hpp>
#include <nudf\string.hpp>
#include <nudf\filesys.hpp>
#include <nudf\crypto.hpp>

#include <nudf\winutil.hpp>


using namespace NX;
using namespace NX::win;


os_version_basic::os_version_basic() : _os(WindowsUnknown), _os_name(L"Unknown Windows"), _product_type(0), _cpu_arch(PROCESSOR_ARCHITECTURE_UNKNOWN), _service_pack(0), _build_number(0)
{
}

os_version_basic::~os_version_basic()
{
}

bool os_version_basic::is_processor_ARM() const
{
    return (_cpu_arch == PROCESSOR_ARCHITECTURE_ARM);
}

bool os_version_basic::is_processor_x86() const
{
    return (_cpu_arch == PROCESSOR_ARCHITECTURE_INTEL);
}

bool os_version_basic::is_processor_x64() const
{
    return (_cpu_arch == PROCESSOR_ARCHITECTURE_AMD64);
}

bool os_version_basic::is_processor_IA64() const
{
    return (_cpu_arch == PROCESSOR_ARCHITECTURE_IA64);
}

os_version_old::os_version_old() : os_version_basic()
{
    initialize();
}

os_version_old::~os_version_old()
{
}

void os_version_old::initialize()
{
    OSVERSIONINFOEXW    osvi;
    SYSTEM_INFO         sysinf;

    memset(&sysinf, 0, sizeof(SYSTEM_INFO));
    GetNativeSystemInfo(&sysinf);

    memset(&osvi, 0, sizeof(OSVERSIONINFOEXW));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
#pragma warning(push)
#pragma warning(disable: 4996)
#pragma prefast(suppress: 28159, "Consider using 'IsWindows*' instead of 'GetVersionExW'.Reason : Deprecated.Use VerifyVersionInfo* or IsWindows* macros from VersionHelpers.")
    GetVersionExW((LPOSVERSIONINFO)&osvi);
#pragma warning(pop)

    // get default
    _product_type = osvi.wProductType;
    _cpu_arch = sysinf.wProcessorArchitecture;
    _service_pack = osvi.wServicePackMajor;
    _service_pack_name = osvi.szCSDVersion;
    _build_number = osvi.dwBuildNumber;

    if (osvi.dwMajorVersion == 5) {
        switch (osvi.dwMinorVersion)
        {
        case 0:
            _os = Windows2000;
            _os_name = L"Windows 2000";
            break;
        case 1:
            _os = WindowsXP;
            _os_name = L"Windows XP";
            break;
        case 2:
            if (_product_type == VER_NT_WORKSTATION) {
                _os = WindowsXPPro64;
                _os_name = L"Windows XP Professional x64 Edition";
            }
            else {
                if (GetSystemMetrics(SM_SERVERR2) == 0) {
                    if (osvi.wSuiteMask & VER_SUITE_WH_SERVER) {
                        _os = WindowsHomeServer;
                        _os_name = L"Windows Home Server";
                    }
                    else {
                        _os = WindowsServer2003;
                        _os_name = L"Windows Server 2003";
                    }
                }
                else {
                    _os = WindowsServer2003R2;
                    _os_name = L"Windows Server 2003 R2";
                }
            }
            break;
        default:
            break;
        }
    }
    else if (osvi.dwMajorVersion == 6) {
        switch (osvi.dwMinorVersion)
        {
        case 0:
            if (_product_type == VER_NT_WORKSTATION) {
                _os = WindowsVista;
                _os_name = L"Windows Vista";
            }
            else {
                _os = WindowsServer2008;
                _os_name = L"Windows Server 2008";
            }
            break;
        case 1:
            if (_product_type == VER_NT_WORKSTATION) {
                _os = Windows7;
                _os_name = L"Windows 7";
            }
            else {
                _os = WindowsServer2008R2;
                _os_name = L"Windows Server 2008 R2";
            }
            break;
        case 2:
            if (_product_type == VER_NT_WORKSTATION) {
                _os = Windows8;
                _os_name = L"Windows 8";
            }
            else {
                _os = WindowsServer2012;
                _os_name = L"Windows Server 2012";
            }
            break;
        case 3:
            if (_product_type == VER_NT_WORKSTATION) {
                _os = Windows8Point1;
                _os_name = L"Windows 8.1";
            }
            else {
                _os = WindowsServer2012R2;
                _os_name = L"Windows Server 2012 R2";
            }
            break;
        default:
            break;
        }
    }
    else if (osvi.dwMajorVersion == 10) {
        switch (osvi.dwMinorVersion)
        {
        case 0:
            if (_product_type == VER_NT_WORKSTATION) {
                _os = Windows10;
                _os_name = L"Windows 10";
            }
            else {
                _os = WindowsServer2016;
                _os_name = L"Windows Server 2016";
            }
            break;
        default:
            break;
        }
    }
    else {
        ; // Unknown
    }
}


os_version_new::os_version_new() : os_version_basic()
{
    initialize();
}

os_version_new::~os_version_new()
{
}

void os_version_new::initialize()
{
    SYSTEM_INFO         sysinf;

    memset(&sysinf, 0, sizeof(SYSTEM_INFO));
    GetNativeSystemInfo(&sysinf);

    // Get default
    _product_type = is_workstation() ? VER_NT_WORKSTATION : (is_domain_controller() ? VER_NT_DOMAIN_CONTROLLER : VER_NT_SERVER);
    _cpu_arch = sysinf.wProcessorArchitecture;
    _build_number = get_build_number();
    _os_edition = get_edition_name();

    if (service_pack_equal(5)) {
        _service_pack = 5;
        _service_pack_name = L"Service Pack 5";
    }
    else if (service_pack_equal(4)) {
        _service_pack = 4;
        _service_pack_name = L"Service Pack 4";
    }
    else if (service_pack_equal(3)) {
        _service_pack = 3;
        _service_pack_name = L"Service Pack 3";
    }
    else if (service_pack_equal(2)) {
        _service_pack = 2;
        _service_pack_name = L"Service Pack 2";
    }
    else if (service_pack_equal(1)) {
        _service_pack = 1;
        _service_pack_name = L"Service Pack 1";
    }
    else {
        ; // No service pack
    }

    if (version_equal(5, 0)) {
        _os = Windows2000;
        _os_name = L"Windows 2000";
    }
    else if (version_equal(5, 1)) {
        _os = WindowsXP;
        _os_name = L"Windows XP";
    }
    else if (version_equal(5, 2)) {
        if (_product_type == VER_NT_WORKSTATION) {
            _os = WindowsXPPro64;
            _os_name = L"Windows XP Professional x64 Edition";
        }
        else {
            if (GetSystemMetrics(SM_SERVERR2) == 0) {
                if (check_product_suite(VER_SUITE_WH_SERVER)) {
                    _os = WindowsHomeServer;
                    _os_name = L"Windows Home Server";
                }
                else {
                    _os = WindowsServer2003;
                    _os_name = L"Windows Server 2003";
                }
            }
            else {
                _os = WindowsServer2003R2;
                _os_name = L"Windows Server 2003 R2";
            }
        }
    }
    else if (version_equal(6, 0)) {
        if (_product_type == VER_NT_WORKSTATION) {
            _os = WindowsVista;
            _os_name = L"Windows Vista";
        }
        else {
            _os = WindowsServer2008;
            _os_name = L"Windows Server 2008";
        }
    }
    else if (version_equal(6, 1)) {
        if (_product_type == VER_NT_WORKSTATION) {
            _os = Windows7;
            _os_name = L"Windows 7";
        }
        else {
            _os = WindowsServer2008R2;
            _os_name = L"Windows Server 2008 R2";
        }
    }
    else if (version_equal(6, 2)) {
        if (_product_type == VER_NT_WORKSTATION) {
            _os = Windows8;
            _os_name = L"Windows 8";
        }
        else {
            _os = WindowsServer2012;
            _os_name = L"Windows Server 2012";
        }
    }
    else if (version_equal(6, 3)) {
        if (_product_type == VER_NT_WORKSTATION) {
            _os = Windows8Point1;
            _os_name = L"Windows 8.1";
        }
        else {
            _os = WindowsServer2012R2;
            _os_name = L"Windows Server 2012 R2";
        }
    }
    else if (version_equal(10, 0)) {
        if (_product_type == VER_NT_WORKSTATION) {
            _os = Windows10;
            _os_name = L"Windows 10";
        }
        else {
            _os = WindowsServer2016;
            _os_name = L"Windows Server 2016";
        }
    }
    else {
        ERROR_OLD_WIN_VERSION
        ; // Unknown
    }
}

bool os_version_new::version_equal(unsigned long major, unsigned long minor)
{
    OSVERSIONINFOEXW osvi;
    DWORDLONG        dwlConditionMask = 0;
    // Initialize the OSVERSIONINFOEX structure.
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXW));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    osvi.dwMajorVersion = major;
    osvi.dwMinorVersion = minor;
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_EQUAL);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_EQUAL);
    return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask) ? true : false;
}

bool os_version_new::service_pack_equal(unsigned short major)
{
    OSVERSIONINFOEXW osvi;
    DWORDLONG        dwlConditionMask = 0;
    // Initialize the OSVERSIONINFOEX structure.
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXW));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    osvi.wServicePackMajor = major;
    VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMAJOR, VER_EQUAL);
    return VerifyVersionInfoW(&osvi, VER_SERVICEPACKMAJOR, dwlConditionMask) ? true : false;
}

bool os_version_new::check_product_suite(unsigned short suite)
{
    OSVERSIONINFOEXW osvi;
    DWORDLONG        dwlConditionMask = 0;
    // Initialize the OSVERSIONINFOEX structure.
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXW));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    osvi.wSuiteMask = suite;
    VER_SET_CONDITION(dwlConditionMask, VER_SUITENAME, VER_AND);
    return VerifyVersionInfoW(&osvi, VER_SUITENAME, dwlConditionMask) ? true : false;
}

bool os_version_new::is_workstation()
{
    OSVERSIONINFOEXW osvi;
    DWORDLONG        dwlConditionMask = 0;
    // Initialize the OSVERSIONINFOEX structure.
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXW));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    osvi.wProductType = VER_NT_WORKSTATION;
    VER_SET_CONDITION(dwlConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);
    return VerifyVersionInfoW(&osvi, VER_PRODUCT_TYPE, dwlConditionMask) ? true : false;
}

bool os_version_new::is_domain_controller()
{
    OSVERSIONINFOEXW osvi;
    DWORDLONG        dwlConditionMask = 0;
    // Initialize the OSVERSIONINFOEX structure.
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXW));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    osvi.wProductType = VER_NT_DOMAIN_CONTROLLER;
    VER_SET_CONDITION(dwlConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);
    return VerifyVersionInfoW(&osvi, VER_PRODUCT_TYPE, dwlConditionMask) ? true : false;
}

unsigned long os_version_new::get_build_number()
{
    unsigned long build_number = 0;
    try {
        reg_local_machine rlm;
        std::wstring    str_build_number;
        rlm.open(L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", reg_key::reg_wow64_64, true);
        rlm.read_value(L"CurrentBuildNumber", str_build_number);
        rlm.close();
        build_number = std::stoul(str_build_number);
    }
    catch (const NX::exception& e) {
        UNREFERENCED_PARAMETER(e);
        build_number = 0;
    }
    return build_number;
}

std::wstring os_version_new::get_edition_name()
{
    std::wstring    edition_name;
    try {
        reg_local_machine rlm;
        rlm.open(L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", reg_key::reg_wow64_64, true);
        rlm.read_value(L"EditionID", edition_name);
        rlm.close();
    }
    catch (const NX::exception& e) {
        UNREFERENCED_PARAMETER(e);
        edition_name.clear();
    }
    return std::move(edition_name);
}


language::language()
{
}

language::language(unsigned short lgid)
{
    set_lgid(lgid);
}

language::language(LCID lcid)
{
    set_lcid(lcid);
}

language::~language()
{
}

language& language::operator = (const language& other)
{
    if (this != &other) {
        _lcid = other.id();
        _name = other.name();
    }
    return *this;
}

void language::set_lcid(LCID lcid)
{
    clear();
    if (0 != LCIDToLocaleName(lcid, NX::string_buffer<wchar_t>(_name, LOCALE_NAME_MAX_LENGTH), LOCALE_NAME_MAX_LENGTH, 0)) {
        _lcid = LOWORD(lcid);
    }
}

void language::set_lgid(unsigned short lgid)
{
    set_lcid(MAKELCID(lgid, SORT_DEFAULT));
}

system_default_language::system_default_language() : language(GetSystemDefaultLCID())
{
}

system_default_language::~system_default_language()
{
}

user_default_language::user_default_language() : language(GetUserDefaultLCID())
{
}

user_default_language::~user_default_language()
{
}


installation::software::software()
{
}

installation::software::software(const std::wstring& n, bool is_64bit, std::wstring ver, std::wstring pub, unsigned short lgid, const std::wstring& date, const std::wstring& dir)
    : _name(n), _x64(is_64bit), _version(ver), _publisher(pub), _lang(lgid), _install_date(date), _install_dir(dir)
{
}

installation::software::~software()
{
}

installation::software& installation::software::operator = (const installation::software& other)
{
    if (this != &other) {
        _x64 = other.x64();
        _name = other.name();
        _version = other.version();
        _publisher = other.publisher();
        _lang = other.lang();
        _install_date = other.install_date();
        _install_dir = other.install_dir();
    }
    return *this;
}

static void inter_get_installed_software(std::vector<installation::software>& items, bool x64)
{
    // HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall
    try {

        reg_key rk;
        reg_key::reg_position rpos = x64 ? reg_key::reg_wow64_64 : reg_key::reg_wow64_32;
        rk.open(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", rpos, true);
        rk.enum_sub_keys([&](const wchar_t* name) {
            try {
                reg_key rk_software;
                rk_software.open(rk, name, rpos, true);

                std::wstring software_name;
                std::wstring software_version;
                std::wstring software_publisher;
                unsigned long software_lang_id;
                std::wstring software_install_date;
                std::wstring software_install_dir;

                rk_software.read_value(L"DisplayName", software_name);
                rk_software.read_value(L"DisplayVersion", software_version);
                rk_software.read_value(L"Publisher", software_publisher);
                rk_software.read_value(L"Language", &software_lang_id);
                rk_software.read_value(L"InstallDate", software_install_date);
                rk_software.read_value(L"InstallLocation", software_install_dir);

                items.push_back(installation::software(software_name, x64, software_version, software_publisher, (unsigned short)software_lang_id, software_install_date, software_install_dir));
            }
            catch (const std::exception& e) {
                UNREFERENCED_PARAMETER(e);
            }
        });
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
}

std::vector<installation::software> installation::get_installed_software(unsigned long flags)
{
    std::vector<installation::software> installed_software;
    if (flags & INSTALLED_SOFTWARE_64BIT) {
        inter_get_installed_software(installed_software, true);     // 64 bit
    }
    if (flags & INSTALLED_SOFTWARE_32BIT) {
        inter_get_installed_software(installed_software, false);    // 32 bit
    }
    return installed_software;
}

std::vector<std::wstring> installation::get_installed_kbs()
{
    std::vector<std::wstring> kbs;
    std::set<std::wstring> kbs_set;

    try {

        reg_key rk;
        rk.open(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Component Based Servicing\\Packages", reg_key::reg_wow64_64, true);
        rk.enum_sub_keys([&](const wchar_t* name) {

            if (name == NULL || name[0] == 0) {
                return;
            }

            std::wstring sub_key_name(name);
            // Package_1_for_KB2894852~31bf3856ad364e35~amd64~~6.3.2.0
            if (!boost::algorithm::istarts_with(sub_key_name, L"Package_")) {
                return;
            }

            std::transform(sub_key_name.begin(), sub_key_name.end(), sub_key_name.begin(), toupper);
            auto pos = sub_key_name.find(L"_FOR_KB");
            if (pos == std::wstring::npos) {
                return;
            }

            // Get KB start: KB2894852~31bf3856ad364e35~amd64~~6.3.2.0
            sub_key_name = sub_key_name.substr(pos + 5);
            pos = sub_key_name.find_first_of(L"~_- .");
            if (pos != std::wstring::npos) {
                sub_key_name = sub_key_name.substr(0, pos); // KB2894852
            }

            // add KB name
            kbs_set.insert(sub_key_name);
        });

        std::for_each(kbs_set.begin(), kbs_set.end(), [&](const std::wstring& name) {
            kbs.push_back(name);
        });
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }

    return std::move(kbs);
}





//
//  class file_version::
//

file_version::file_version() : _product_version({ 0, 0 }), _file_version({ 0, 0 }), _file_time({ 0, 0 }), _file_flags(0), _file_os(VOS_UNKNOWN), _file_type(0), _file_subtype(0)
{
}

file_version::file_version(const std::wstring& file) : _product_version({ 0, 0 }), _file_version({ 0, 0 }), _file_time({ 0, 0 }), _file_flags(0), _file_os(VOS_UNKNOWN), _file_type(0), _file_subtype(0)
{
    if (file.empty()) {
        NX::fs::module_path mod_path(NULL);
        load(mod_path.path());
    }
    else {
        load(file);
    }
}

file_version::~file_version()
{
}

void file_version::clear()
{
    _company_name.clear();
    _product_name.clear();
    _product_version_string.clear();
    _file_name.clear();
    _file_description.clear();
    _file_version_string.clear();
    _product_version = { 0, 0 };
    _file_version = { 0, 0 };
    _file_time = { 0, 0 };
    _file_flags = 0;
    _file_os = 0;
    _file_type = 0;
    _file_subtype = 0;
}

file_version& file_version::operator = (const file_version& other)
{
    if (this != &other) {
        _company_name = other.get_company_name();
        _product_name = other.get_product_name();
        _product_version_string = other.get_product_version_string();
        _file_name = other.get_file_name();
        _file_description = other.get_file_description();
        _file_version_string = other.get_file_version_string();
        _product_version = other.get_product_version();
        _file_time = other.get_file_time();
        _file_version = other.get_file_version();
        _file_flags = other.get_file_flags();
        _file_os = other.get_file_os();
        _file_type = other.get_file_type();
        _file_subtype = other.get_file_subtype();
    }
    return *this;
}

bool file_version::operator == (const file_version& other) const
{
    return (_file_time.dwHighDateTime == other.get_file_time().dwHighDateTime
        && _file_time.dwLowDateTime == other.get_file_time().dwLowDateTime
        && _product_version.QuadPart == other.get_product_version().QuadPart
        && _file_version.QuadPart == other.get_file_version().QuadPart
        && _file_flags == other.get_file_flags()
        && _file_os == other.get_file_os()
        && _file_type == other.get_file_type()
        && _file_subtype == other.get_file_subtype()
        && _product_name == other.get_product_name()
        && _company_name == other.get_company_name()
        && _file_name == other.get_file_name()
        );
}

void file_version::load(const std::wstring& file)
{
    typedef struct LANGANDCODEPAGE {
        WORD wLanguage;
        WORD wCodePage;
    } LANGANDCODEPAGE, *LPLANGANDCODEPAGE;


    try {

        const unsigned long version_info_size = GetFileVersionInfoSizeW(file.c_str(), NULL);
        if (0 == version_info_size) {
            throw std::exception("fail to get version size");
        }

        std::vector<unsigned char> info;
        info.resize(version_info_size, 0);
        if (!GetFileVersionInfoW(file.c_str(), 0, version_info_size, info.data())) {
            throw std::exception("fail to get version data");
        }

        VS_FIXEDFILEINFO* fixed_info = NULL;
        unsigned int data_size = 0;

        if (!VerQueryValueW(info.data(), L"\\", (LPVOID*)&fixed_info, &data_size)) {
            throw std::exception("fail to get fixed file info");
        }
        _product_version.HighPart = fixed_info->dwProductVersionMS;
        _product_version.LowPart = fixed_info->dwProductVersionLS;
        _file_version.HighPart = fixed_info->dwFileVersionMS;
        _file_version.LowPart = fixed_info->dwFileVersionLS;
        _file_time.dwHighDateTime = fixed_info->dwFileDateMS;
        _file_time.dwLowDateTime = fixed_info->dwFileDateLS;
        _file_flags = fixed_info->dwFileFlags;
        _file_os = fixed_info->dwFileOS;
        _file_type = fixed_info->dwFileType;
        _file_subtype = fixed_info->dwFileSubtype;

        // get language & codepage
        LPLANGANDCODEPAGE Translate;
        if (!VerQueryValueW(info.data(), L"\\VarFileInfo\\Translation", (LPVOID*)&Translate, &data_size)) {
            throw std::exception("fail to get translate info");
        }
        if (NULL == Translate || data_size < sizeof(LANGANDCODEPAGE)) {
            throw std::exception("empty translate info");
        }

        // now get all the string information in default language
        //    -> CompanyName
        _company_name = load_string(info.data(), L"CompanyName", Translate[0].wLanguage, Translate[0].wCodePage);
        //    -> ProductName
        _product_name = load_string(info.data(), L"ProductName", Translate[0].wLanguage, Translate[0].wCodePage);
        //    -> OriginalFilename
        _file_name = load_string(info.data(), L"OriginalFilename", Translate[0].wLanguage, Translate[0].wCodePage);
        //    -> FileDescription
        _file_description = load_string(info.data(), L"FileDescription", Translate[0].wLanguage, Translate[0].wCodePage);
        //    -> ProductVersion
        _product_version_string = load_string(info.data(), L"ProductVersion", Translate[0].wLanguage, Translate[0].wCodePage);
        //    -> FileVersion
        _file_version_string = load_string(info.data(), L"FileVersion", Translate[0].wLanguage, Translate[0].wCodePage);
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        clear();
    }
}

std::wstring file_version::load_string(void* data, const std::wstring& name, unsigned short language, unsigned short codepage)
{
    unsigned int cch = 0;
    wchar_t* str = NULL;
    const std::wstring resource_name = NX::string_formater(L"\\StringFileInfo\\%04x%04x\\%s", language, codepage, name.c_str());
    if (!VerQueryValueW(data, resource_name.c_str(), (LPVOID*)&str, &cch)) {
        return std::wstring();
    }
    if (0 == cch || NULL == str) {
        return std::wstring();
    }    
    return std::wstring(str, str + cch);
}


//
//
//

NX::win::hardware::processor_information::processor_information() : _hyperthreads(false), _cores(0), _logical_processors(0)
{
    load();
}

NX::win::hardware::processor_information::~processor_information()
{
}

void NX::win::hardware::processor_information::load()
{
    cpu_id_data cpui;

    // Calling __cpuid with 0x0 as the function_id argument
    // gets the number of the highest valid function ID.
    __cpuid(cpui.data(), 0);
    const int cpu_id_count = cpui[0];

    for (int i = 0; i <= cpu_id_count; ++i) {
        __cpuidex(cpui.data(), i, 0);
        _data.push_back(cpui);
    }

    // Capture vendor string
    char vendor_name[0x20];
    memset(vendor_name, 0, sizeof(vendor_name));
    *reinterpret_cast<int*>(vendor_name) = _data[0][1];
    *reinterpret_cast<int*>(vendor_name + 4) = _data[0][3];
    *reinterpret_cast<int*>(vendor_name + 8) = _data[0][2];
    if (0 == _stricmp(vendor_name, "GenuineIntel")) {
        _vendor = CV_INTEL;
    }
    else if (0 == _stricmp(vendor_name, "AuthenticAMD")) {
        _vendor = CV_AMD;
    }
    else {
        _vendor = CV_UNKNOWN;
    }

    // logical processors
    _logical_processors = (_data[1][1] >> 16) & 0xFF;
    _cores = _logical_processors;
    const unsigned long cpu_features = _data[1][3];


    // load bitset with flags for function 0x00000001
    if (cpu_id_count >= 1) {
        f_1_ECX_ = _data[1][2];
        f_1_EDX_ = _data[1][3];
    }

    // load bitset with flags for function 0x00000007
    if (cpu_id_count >= 7) {
        f_7_EBX_ = _data[7][1];
        f_7_ECX_ = _data[7][2];
    }

    // Calling __cpuid with 0x80000000 as the function_id argument
    // gets the number of the highest valid extended ID.
    __cpuid(cpui.data(), 0x80000000);
    const int cpu_extra_id_count = cpui[0];

    char brand_name[0x40];
    memset(brand_name, 0, sizeof(brand_name));

    for (int i = 0x80000000; i <= cpu_extra_id_count; ++i) {
        __cpuidex(cpui.data(), i, 0);
        _extdata.push_back(cpui);
    }

    // load bitset with flags for function 0x80000001
    if (cpu_extra_id_count >= 0x80000001) {
        f_81_ECX_ = _extdata[1][2];
        f_81_EDX_ = _extdata[1][3];
    }

    // Interpret CPU brand string if reported
    if (cpu_extra_id_count >= 0x80000004) {
        memcpy(brand_name, _extdata[2].data(), sizeof(cpui));
        memcpy(brand_name + 16, _extdata[3].data(), sizeof(cpui));
        memcpy(brand_name + 32, _extdata[4].data(), sizeof(cpui));
        _brand = NX::conversion::utf8_to_utf16(brand_name);
    }

    // Get cores        
    if (is_vender_intel()) {
        _cores = ((_data[4][0] >> 26) & 0x3f) + 1;
    }
    else if (is_vender_amd()) {
        _cores = ((unsigned)(_extdata[8][2] & 0xff)) + 1;
    }

    // is hyperthreads enabled
    _hyperthreads = ((cpu_features & (1 << 28)) && (_cores < _logical_processors));
}


//
//
//

NX::win::hardware::memory_information::memory_information()
{
    memset(&_status, 0, sizeof(_status));
    _status.dwLength = sizeof(_status);
    GlobalMemoryStatusEx(&_status);
}

NX::win::hardware::memory_information::~memory_information()
{
}


//
//
//

NX::win::hardware::network_adapter_information::network_adapter_information() : _if_type(0), _oper_status(0), _ipv4_enabled(false), _ipv6_enabled(false)
{
}

NX::win::hardware::network_adapter_information::network_adapter_information(const std::wstring& adapter_name,
    const std::wstring& friendly_name,
    const std::wstring& description,
    const std::wstring& physical_address,
    unsigned long if_type,
    unsigned long oper_status,
    bool ipv4_enabled,
    bool ipv6_enabled,
    const std::vector<std::wstring>& ipv4_addresses,
    const std::vector<std::wstring>& ipv6_addresses
    ) : _adapter_name(adapter_name),
    _friendly_name(friendly_name),
    _description(description),
    _physical_address(physical_address),
    _if_type(if_type),
    _oper_status(oper_status),
    _ipv4_enabled(ipv4_enabled),
    _ipv6_enabled(ipv6_enabled),
    _ipv4_addresses(ipv4_addresses),
    _ipv6_addresses(ipv6_addresses)
{
}

NX::win::hardware::network_adapter_information::network_adapter_information(const void* adapter_data)
{
    load(adapter_data);
}

NX::win::hardware::network_adapter_information::~network_adapter_information()
{
}

NX::win::hardware::network_adapter_information& NX::win::hardware::network_adapter_information::operator = (const NX::win::hardware::network_adapter_information& other)
{
    if (this != &other) {
        _adapter_name = other.get_adapter_name();
        _friendly_name = other.get_friendly_name();
        _description = other.get_description();
        _physical_address = other.get_mac_address();
        _if_type = other.get_if_type();
        _oper_status = other.get_oper_status();
        _ipv4_enabled = other.is_ipv4_enabled();
        _ipv6_enabled = other.is_ipv6_enabled();
        _ipv4_addresses = other.get_ipv4_addresses();
        _ipv6_addresses = other.get_ipv6_addresses();
    }
    return *this;
}

void NX::win::hardware::network_adapter_information::clear()
{
    _adapter_name.clear();
    _friendly_name.clear();
    _description.clear();
    _physical_address.clear();
    _if_type = 0;
    _oper_status = 0;
    _ipv4_enabled = false;
    _ipv6_enabled = false;
    _ipv4_addresses.clear();
    _ipv6_addresses.clear();
}

typedef PWSTR(NTAPI* FpRtlIpv4AddressToString)(_In_ const IN_ADDR *Addr, _Out_ PWSTR S);
typedef PWSTR(NTAPI* FpRtlIpv6AddressToString)(_In_ const IN6_ADDR *Addr, _Out_ PWSTR S);

class RtlIpAddressToStringW
{
public:
    RtlIpAddressToStringW() : _fp_ipv4(NULL), _fp_ipv6(NULL)
    {
        HMODULE mod = GetModuleHandleW(L"ntdll.dll");
        if (NULL != mod) {
            _fp_ipv4 = (FpRtlIpv4AddressToString)GetProcAddress(mod, "RtlIpv4AddressToStringW");
            _fp_ipv6 = (FpRtlIpv6AddressToString)GetProcAddress(mod, "RtlIpv6AddressToStringW");
        }
    }

    ~RtlIpAddressToStringW() {}

    PWSTR operator () (_In_ const IN_ADDR *Addr, _Out_ PWSTR S)
    {
        return (NULL != _fp_ipv4) ? _fp_ipv4(Addr, S) : NULL;
    }

    PWSTR operator () (_In_ const IN6_ADDR *Addr, _Out_ PWSTR S)
    {
        return (NULL != _fp_ipv6) ? _fp_ipv6(Addr, S) : NULL;
    }

private:
    FpRtlIpv4AddressToString    _fp_ipv4;
    FpRtlIpv6AddressToString    _fp_ipv6;
};

static bool is_ipv6_link_local_or_special_use(const std::wstring& ipv6_address)
{
    if (0 == ipv6_address.find(L"fe")) {
        wchar_t c = ipv6_address[2];
        if (c == '8' || c == '9' || c == 'a' || c == 'b') {
            // local link
            return true;
        }
    }
    else if (0 == ipv6_address.find(L"2001:0:")) {
        // special use
        return true;
    }
    else {
        ; // nothing
    }

    return false;
}

void NX::win::hardware::network_adapter_information::load(const void* adapter_data)
{
    PIP_ADAPTER_ADDRESSES pAdapter = (PIP_ADAPTER_ADDRESSES)adapter_data;

    // adapter name
    _adapter_name = NX::conversion::ansi_to_utf16(pAdapter->AdapterName);

    // adapter friendly name
    if (NULL != pAdapter->FriendlyName) {
        _friendly_name = pAdapter->FriendlyName;
    }

    // adapter description
    if (NULL != pAdapter->Description) {
        _description = pAdapter->Description;
    }

    // adapter mac address
    if (NULL != pAdapter->PhysicalAddress && 0 != pAdapter->PhysicalAddressLength) {
        std::for_each(pAdapter->PhysicalAddress, pAdapter->PhysicalAddress + pAdapter->PhysicalAddressLength, [&](const BYTE v) {
            if (!_physical_address.empty()) _physical_address += L"-";
            _physical_address += NX::string_formater(L"%02X", v);
        });
    }

    // adapter IfType and OperStatus
    _if_type = pAdapter->IfType;
    _oper_status = pAdapter->OperStatus;
    // ipv4/ipv6 status
    _ipv4_enabled = pAdapter->Ipv4Enabled;
    _ipv6_enabled = pAdapter->Ipv6Enabled;

    // ip addresses
    static RtlIpAddressToStringW ip_conv;
    PIP_ADAPTER_UNICAST_ADDRESS ip_address = pAdapter->FirstUnicastAddress;
    while (NULL != ip_address) {
        if (ip_address->Address.lpSockaddr->sa_family == AF_INET) {
            SOCKADDR_IN* ipv4 = reinterpret_cast<SOCKADDR_IN*>(ip_address->Address.lpSockaddr);
            wchar_t str_buffer[INET_ADDRSTRLEN] = { 0 };
            ip_conv(&(ipv4->sin_addr), str_buffer);
            if (L'\0' != str_buffer[0]) {
                _ipv4_addresses.push_back(std::wstring(str_buffer));
            }
        }
        else if (ip_address->Address.lpSockaddr->sa_family == AF_INET6) {
            SOCKADDR_IN6* ipv6 = reinterpret_cast<SOCKADDR_IN6*>(ip_address->Address.lpSockaddr);
            wchar_t str_buffer[INET6_ADDRSTRLEN] = { 0 };
            ip_conv(&(ipv6->sin6_addr), str_buffer);
            std::wstring ws_ip(str_buffer);
            if (!ws_ip.empty() && !is_ipv6_link_local_or_special_use(ws_ip)) {
                _ipv6_addresses.push_back(std::wstring(ws_ip));
            }
        }
        else {
            ; // unknown
        }

        ip_address = ip_address->Next;
    }
}


std::wstring NX::win::hardware::network_adapter_information::get_if_type_name() const
{
    std::wstring s;

    switch (_if_type)
    {
    case IF_TYPE_OTHER:
        s = L"Others";
        break;
    case IF_TYPE_ETHERNET_CSMACD:
        s = L"Ethernet network interface";
        break;
    case IF_TYPE_ISO88025_TOKENRING:
        s = L"Token ring network interface";
        break;
    case IF_TYPE_PPP:
        s = L"PPP network interface";
        break;
    case IF_TYPE_SOFTWARE_LOOPBACK:
        s = L"Software loopback network interface";
        break;
    case IF_TYPE_ATM:
        s = L"ATM network interface";
        break;
    case IF_TYPE_IEEE80211:
        s = L"IEEE 802.11 wireless network interface";
        break;
    case IF_TYPE_TUNNEL:
        s = L"Tunnel type encapsulation network interface";
        break;
    case IF_TYPE_IEEE1394:
        s = L"IEEE 1394 (Firewire) high performance serial bus network interface";
        break;
    default:
        s = L"Unknown";
        break;
    }

    return std::move(s);
}

std::wstring NX::win::hardware::network_adapter_information::get_oper_status_name() const
{
    std::wstring s;

    switch (_oper_status)
    {
    case IfOperStatusUp:
        s = L"Up";
        break;
    case IfOperStatusDown:
        s = L"Down";
        break;
    case IfOperStatusTesting:
        s = L"Testing Mode";
        break;
    case IfOperStatusDormant:
        s = L"Pending";
        break;
    case IfOperStatusNotPresent:
        s = L"Down (Component not present)";
        break;
    case IfOperStatusLowerLayerDown:
        s = L"Down (Lower layer interface is down)";
        break;
    case IfOperStatusUnknown:
    default:
        s = L"Unknown";
        break;
    }

    return std::move(s);
}

bool NX::win::hardware::network_adapter_information::is_ethernet_adapter() const
{
    return (IF_TYPE_ETHERNET_CSMACD == get_if_type());
}

bool NX::win::hardware::network_adapter_information::is_ppp_adapter() const
{
    return (IF_TYPE_PPP == get_if_type());
}

bool NX::win::hardware::network_adapter_information::is_80211_adapter() const
{
    return (IF_TYPE_IEEE80211 == get_if_type());
}

bool NX::win::hardware::network_adapter_information::is_1394_adapter() const
{
    return (IF_TYPE_IEEE1394 == get_if_type());
}

bool NX::win::hardware::network_adapter_information::is_network_adapter() const
{
    return (is_ethernet_adapter() || is_ppp_adapter() || is_80211_adapter());
}

bool NX::win::hardware::network_adapter_information::is_active() const
{
    return (IfOperStatusUp == get_oper_status());
}

bool NX::win::hardware::network_adapter_information::is_connected() const
{
    return (IfOperStatusUp == get_oper_status());
}

std::vector<NX::win::hardware::network_adapter_information> NX::win::hardware::get_all_adapters()
{
    std::vector<NX::win::hardware::network_adapter_information> adapters;
    std::vector<unsigned char> buf;
    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    PIP_ADAPTER_ADDRESSES pCurAddress = NULL;
    ULONG dwSize = sizeof(IP_ADAPTER_ADDRESSES);
    ULONG dwRetVal = 0;

    buf.resize(dwSize, 0);
    pAddresses = (PIP_ADAPTER_ADDRESSES)buf.data();
    memset(pAddresses, 0, dwSize);

    dwRetVal = GetAdaptersAddresses(AF_UNSPEC, 0, NULL, pAddresses, &dwSize);
    if (ERROR_SUCCESS != dwRetVal) {
        if (ERROR_BUFFER_OVERFLOW != dwRetVal) {
            return std::move(adapters);
        }

        dwSize += sizeof(IP_ADAPTER_ADDRESSES);
        buf.resize(dwSize, 0);
        pAddresses = (PIP_ADAPTER_ADDRESSES)buf.data();
        memset(pAddresses, 0, dwSize);
    }

    if (NULL == pAddresses) {
        return std::move(adapters);
    }

    dwRetVal = GetAdaptersAddresses(AF_UNSPEC, 0, NULL, pAddresses, &dwSize);
    if (ERROR_SUCCESS != dwRetVal) {
        return std::move(adapters);
    }

    pCurAddress = pAddresses;
    do {
        if (IF_TYPE_SOFTWARE_LOOPBACK == pCurAddress->IfType) {
            continue;
        }
        adapters.push_back(NX::win::hardware::network_adapter_information(pCurAddress));
    } while (NULL != (pCurAddress = pCurAddress->Next));

    return std::move(adapters);
}

std::vector<NX::win::hardware::network_adapter_information> NX::win::hardware::get_all_network_adapters()
{
    std::vector<NX::win::hardware::network_adapter_information> active_adapters;
    const std::vector<NX::win::hardware::network_adapter_information>& all_adapters = NX::win::hardware::get_all_adapters();
    std::for_each(all_adapters.begin(), all_adapters.end(), [&](const NX::win::hardware::network_adapter_information& adapter) {
        if (adapter.is_network_adapter()) {
            active_adapters.push_back(adapter);
        }
    });
    return std::move(active_adapters);
}

std::vector<NX::win::hardware::network_adapter_information> NX::win::hardware::get_active_network_adapters()
{
    std::vector<NX::win::hardware::network_adapter_information> active_adapters;
    const std::vector<NX::win::hardware::network_adapter_information>& all_adapters = NX::win::hardware::get_all_adapters();
    std::for_each(all_adapters.begin(), all_adapters.end(), [&](const NX::win::hardware::network_adapter_information& adapter) {
        if (adapter.is_network_adapter() && adapter.is_connected()) {
            active_adapters.push_back(adapter);
        }
    });
    return std::move(active_adapters);
}

//
//  class pe_file
//

pe_file::pe_file() : _machine(0), _characteristics(0), _subsystem(0), _image_checksum(0), _image_base(0), _base_of_code(0), _address_of_entry(0)
{
}

pe_file::pe_file(const std::wstring& file) : _machine(0), _characteristics(0), _subsystem(0), _image_checksum(0), _image_base(0), _base_of_code(0), _address_of_entry(0)
{
    load(file);
}

pe_file::~pe_file()
{
}

void pe_file::load(const std::wstring& file)
{
    if (!load_pe_header(file)) {
        return;
    }

    load_signature(file);
    _file_version = file_version(file);
}

void pe_file::clear()
{
    _image_publisher.clear();
    _machine = 0;
    _characteristics = 0;
    _subsystem = 0;
    _image_checksum = 0;
    _image_base = 0;
    _base_of_code = 0;
    _address_of_entry = 0;
    _file_version.clear();
}

pe_file& pe_file::operator = (const pe_file& other)
{
    if (this != &other) {
        _image_publisher = other.get_image_publisher();
        _machine = other.get_machine_code();
        _characteristics = other.get_characteristics();
        _subsystem = other.get_subsystem();
        _image_checksum = other.get_image_checksum();
        _image_base = other.get_image_base();
        _base_of_code = other.get_base_of_code();
        _address_of_entry = other.get_address_of_entry();
        _file_version = other.get_file_version();
    }
    return *this;
}

bool pe_file::load_pe_header(const std::wstring& file)
{
    bool result = false;
    HANDLE h = INVALID_HANDLE_VALUE;
    HANDLE hmap = NULL;
    LPVOID p = NULL;

    h = CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (h == INVALID_HANDLE_VALUE) {
        return false;
    }

    do {

        IMAGE_DOS_HEADER dos_header = { 0 };
        IMAGE_NT_HEADERS nt_headers = { 0 };
        unsigned long bytes_read = 0;

        memset(&dos_header, 0, sizeof(dos_header));
        memset(&nt_headers, 0, sizeof(nt_headers));

        if (!ReadFile(h, &dos_header, (unsigned long)sizeof(dos_header), &bytes_read, NULL)) {
            break;
        }
        if (bytes_read != (unsigned long)sizeof(dos_header)) {
            break;
        }
        if (IMAGE_DOS_SIGNATURE != dos_header.e_magic) {
            break;
        }

        if (INVALID_SET_FILE_POINTER == SetFilePointer(h, dos_header.e_lfanew, NULL, FILE_BEGIN)) {
            break;
        }
        if (!ReadFile(h, &nt_headers, (unsigned long)sizeof(nt_headers), &bytes_read, NULL)) {
            break;
        }
        CloseHandle(h); h = INVALID_HANDLE_VALUE;
        if (bytes_read != (unsigned long)sizeof(nt_headers)) {
            break;
        }
        if (0x00004550 != nt_headers.Signature) {
            break;
        }

        // Good
        result = true;
        _machine = nt_headers.FileHeader.Machine;
        _characteristics = nt_headers.FileHeader.Characteristics;

        _image_checksum = nt_headers.OptionalHeader.CheckSum;
        _image_base = nt_headers.OptionalHeader.ImageBase;
        _base_of_code = nt_headers.OptionalHeader.BaseOfCode;
        _address_of_entry = nt_headers.OptionalHeader.AddressOfEntryPoint;
        _subsystem = nt_headers.OptionalHeader.Subsystem;

    } while (false);

    // clean up
    if (INVALID_HANDLE_VALUE != h) {
        CloseHandle(h); h = INVALID_HANDLE_VALUE;
    }

    return result;
}

void pe_file::load_signature(const std::wstring& file)
{
    DWORD dwEncoding = 0, dwContentType = 0, dwFormatType = 0;
    HCERTSTORE hStore = NULL;
    HCRYPTMSG hMsg = NULL;

    do {

        if (!CryptQueryObject(CERT_QUERY_OBJECT_FILE,
            file.c_str(),
            CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,
            CERT_QUERY_FORMAT_FLAG_BINARY,
            0,
            &dwEncoding,
            &dwContentType,
            &dwFormatType,
            &hStore,
            &hMsg,
            NULL))
        {
            break;;
        }

        std::vector<unsigned char> signer_data;
        PCMSG_SIGNER_INFO signer_info = NULL;
        unsigned long signer_info_size = 0;
        CERT_INFO cert_info = { 0 };

        if (!CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, 0, NULL, &signer_info_size)) {
            break;
        }

        // Allocate memory for signer information.
        signer_data.resize(signer_info_size, 0);
        signer_info = reinterpret_cast<PCMSG_SIGNER_INFO>(signer_data.data());

        // Get Signer Information.
        if (!CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, 0, (PVOID)signer_info, &signer_info_size)) {
            break;
        }

        memset(&cert_info, 0, sizeof(cert_info));
        cert_info.Issuer = signer_info->Issuer;
        cert_info.SerialNumber = signer_info->SerialNumber;
        PCCERT_CONTEXT pcontext = CertFindCertificateInStore(hStore, dwEncoding, 0, CERT_FIND_SUBJECT_CERT, (PVOID)&cert_info, NULL);
        if (NULL == pcontext) {
            break;
        }

        NX::certificate::context cert_context;
        cert_context.attach(pcontext); pcontext = NULL;
        _image_publisher = cert_context.get_subject_name();

    } while (false);

    // clean
    if (NULL != hMsg) {
        CryptMsgClose(hMsg);
        hMsg = NULL;
    }
    if (NULL != hStore) {
        CertCloseStore(hStore, 0);
        hStore = NULL;
    }
}


//
//
//

reg_key::reg_key() : _h(NULL)
{
}

reg_key::~reg_key()
{
    close();
}

void reg_key::create(HKEY root, const std::wstring& path, reg_position pos)
{
    unsigned long disposition = 0;
    unsigned long desired_access = KEY_ALL_ACCESS;
    switch (pos)
    {
    case reg_wow64_32:
        desired_access |= KEY_WOW64_32KEY;
        break;
    case reg_wow64_64:
        desired_access |= KEY_WOW64_64KEY;
        break;
    case reg_default:
    default:
        break;
    }
    long result = ::RegCreateKeyExW(root, path.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, desired_access, NULL, &_h, &disposition);
    if (ERROR_SUCCESS != result) {
        _h = NULL;
        throw NX::exception(WIN32_ERROR_MSG(result, "win_registry::create, RegCreateKeyExW"));
    }
}

void reg_key::open(HKEY root, const std::wstring& path, reg_position pos, bool read_only)
{
    unsigned long desired_access = read_only ? KEY_READ : (KEY_READ | KEY_WRITE);
    switch (pos)
    {
    case reg_wow64_32:
        desired_access |= KEY_WOW64_32KEY;
        break;
    case reg_wow64_64:
        desired_access |= KEY_WOW64_64KEY;
        break;
    default:
        break;
    }
    long result = ::RegOpenKeyExW(root, path.c_str(), 0, desired_access, &_h);
    if (ERROR_SUCCESS != result) {
        _h = NULL;
        throw NX::exception(WIN32_ERROR_MSG(result, "win_registry::open, RegOpenKeyExW"));
    }
}

void reg_key::close()
{
    if (NULL != _h) {
        RegCloseKey(_h);
        _h = NULL;
    }
}

void reg_key::remove(HKEY root, const std::wstring& path)
{
    long result = RegDeleteKey(root, path.c_str());
    if (ERROR_SUCCESS != result) {
        throw NX::exception(WIN32_ERROR_MSG(result, "win_registry::remove, RegDeleteKey"));
    }
}

bool reg_key::exist(HKEY root, const std::wstring& path) noexcept
{
    HKEY h = NULL;
    long result = ::RegOpenKeyExW(root, path.c_str(), 0, KEY_READ, &h);
    if (ERROR_SUCCESS == result) {
        RegCloseKey(h);
        return true;
    }
    else {
        return (ERROR_FILE_NOT_FOUND == result) ? false : true;
    }
}


std::wstring reg_key::read_default_value()
{
    std::wstring value;
    read_value(L"", value);
    return std::move(value);
}

void reg_key::read_value(const std::wstring& name, unsigned long* value)
{
    unsigned long value_type = 0;
    const std::vector<unsigned char>& buf = internal_read_value(name, &value_type);
    switch (value_type)
    {
    case REG_DWORD:
    case REG_QWORD:
        assert(buf.size() >= 4);
        *value = *((unsigned long*)buf.data());
        break;
    case REG_DWORD_BIG_ENDIAN:
        assert(buf.size() >= 4);
        *value = convert_endian(*((unsigned long*)buf.data()));
        break;
    default:
        throw NX::exception(WIN32_ERROR_MSG2(ERROR_INVALID_DATATYPE));
        break;
    }
}

void reg_key::read_value(const std::wstring& name, unsigned __int64* value)
{
    unsigned long value_type = 0;
    const std::vector<unsigned char>& buf = internal_read_value(name, &value_type);
    switch (value_type)
    {
    case REG_DWORD:
        assert(buf.size() >= 4);
        *value = *((unsigned long*)buf.data());
        break;
    case REG_QWORD:
        assert(buf.size() >= 4);
        *value = *((unsigned __int64*)buf.data());
        break;
    case REG_DWORD_BIG_ENDIAN:
        assert(buf.size() >= 4);
        *value = (unsigned __int64)convert_endian(*((unsigned long*)buf.data()));
        break;
    default:
        throw NX::exception(WIN32_ERROR_MSG2(ERROR_INVALID_DATATYPE));
        break;
    }
}

void reg_key::read_value(const std::wstring& name, std::wstring& value)
{
    unsigned long value_type = 0;
    const std::vector<unsigned char>& buf = internal_read_value(name, &value_type);
    switch (value_type)
    {
    case REG_SZ:
        value = buf.empty() ? std::wstring(L"") : ((const wchar_t*)buf.data());
        break;
    case REG_EXPAND_SZ:
        value = buf.empty() ? std::wstring(L"") : expand_env_string((const wchar_t*)buf.data());
        break;
    case REG_MULTI_SZ:
        {
            const std::vector<std::wstring>& strings = expand_multi_strings((const wchar_t*)buf.data());
            value = strings.empty() ? std::wstring(L"") : strings[0];
        }
        break;
    default:
        throw NX::exception(WIN32_ERROR_MSG2(ERROR_INVALID_DATATYPE));
        break;
    }
}

void reg_key::read_value(const std::wstring& name, std::vector<std::wstring>& value)
{
    unsigned long value_type = 0;
    const std::vector<unsigned char>& buf = internal_read_value(name, &value_type);
    switch (value_type)
    {
    case REG_SZ:
        if (!buf.empty()) {
            value.push_back((const wchar_t*)buf.data());
        }
        break;
    case REG_EXPAND_SZ:
        if (!buf.empty()) {
            value.push_back(expand_env_string((const wchar_t*)buf.data()));
        }
        break;
    case REG_MULTI_SZ:
        value = expand_multi_strings((const wchar_t*)buf.data());
        break;
    default:
        throw NX::exception(WIN32_ERROR_MSG2(ERROR_INVALID_DATATYPE));
        break;
    }
}

void reg_key::read_value(const std::wstring& name, std::vector<unsigned char>& value)
{
    unsigned long value_type = 0;
    value = internal_read_value(name, &value_type);
}


void reg_key::set_default_value(const std::wstring& value)
{
    set_value(L"", value, false);
}

void reg_key::set_value(const std::wstring& name, unsigned long value)
{
    internal_set_value(name, REG_DWORD, &value, sizeof(unsigned long));
}

void reg_key::set_value(const std::wstring& name, unsigned __int64 value)
{
    internal_set_value(name, REG_QWORD, &value, sizeof(unsigned __int64));
}

void reg_key::set_value(const std::wstring& name, const std::wstring& value, bool expandable)
{
    unsigned long size = (unsigned long)(sizeof(wchar_t) * (value.length() + 1));
    internal_set_value(name, expandable ? REG_SZ : REG_EXPAND_SZ, value.c_str(), size);
}

void reg_key::set_value(const std::wstring& name, const std::vector<std::wstring>& value)
{
    const std::vector<wchar_t>& buf = create_multi_strings_buffer(value);
    unsigned long size = (unsigned long)(sizeof(wchar_t) * buf.size());
    internal_set_value(name, REG_MULTI_SZ, buf.data(), size);
}

void reg_key::set_value(const std::wstring& name, const std::vector<unsigned char>& value)
{
    internal_set_value(name, REG_BINARY, value.data(), (unsigned long)value.size());
}

std::wstring reg_key::expand_env_string(const std::wstring& s)
{
    static const size_t max_info_buf_size = 32767;
    std::wstring output;
    std::vector<wchar_t> buf;
    buf.resize(max_info_buf_size, 0);
    if (0 != ExpandEnvironmentStrings(s.c_str(), buf.data(), max_info_buf_size)) {
        output = buf.data();
    }
    return std::move(output);
}

std::vector<std::wstring> reg_key::expand_multi_strings(const wchar_t* s)
{
    std::vector<std::wstring> strings;
    while (0 != s[0]) {
        std::wstring ws(s);
        assert(ws.length() > 0);
        s += ws.length() + 1;
        strings.push_back(ws);
    }
    return std::move(strings);
}

std::vector<wchar_t> reg_key::create_multi_strings_buffer(const std::vector<std::wstring>& strings)
{
    std::vector<wchar_t> buf;
    std::for_each(strings.begin(), strings.end(), [&](const std::wstring& s) {
        std::for_each(s.begin(), s.end(), [&](const wchar_t& c) {
            buf.push_back(c);
        });
        buf.push_back(L'\0');
    });
    buf.push_back(L'\0');
    return std::move(buf);
}

unsigned long reg_key::convert_endian(unsigned long u)
{
    return (((u >> 24) & 0xFF) | ((u << 24) & 0xFF000000) | ((u >> 8) & 0xFF00) | ((u << 8) & 0xFF0000));
}

std::vector<unsigned char> reg_key::internal_read_value(const std::wstring& name, unsigned long* value_type)
{
    long result = 0;
    std::vector<unsigned char> buf;
    unsigned long value_size = 1;

    buf.resize(1, 0);
    value_size = 1;

    result = ::RegQueryValueExW(_h, name.empty() ? NULL : name.c_str(), NULL, value_type, (LPBYTE)buf.data(), &value_size);
    if (ERROR_SUCCESS == result) {
        // succeed
        if (0 == value_size) buf.clear();
        return std::move(buf);
    }

    // failed, and error is not ERROR_MORE_DATA
    if (ERROR_MORE_DATA != result) {
        throw NX::exception(WIN32_ERROR_MSG2(result));
    }

    // reset buffer size
    buf.resize(value_size, 0);
    // try to get data again
    result = ::RegQueryValueExW(_h, name.empty() ? NULL : name.c_str(), NULL, value_type, (LPBYTE)buf.data(), &value_size);
    if (ERROR_SUCCESS != result) {
        throw NX::exception(WIN32_ERROR_MSG(result, "win_registry::read_value, RegQueryValueExW"));
    }

    return std::move(buf);
}

void reg_key::internal_set_value(const std::wstring& name, unsigned long value_type, const void* value, unsigned long size)
{
    long result = ::RegSetValueExW(_h, name.empty() ? NULL : name.c_str(), 0, value_type, (LPBYTE)value, size);
    if (ERROR_SUCCESS != result) {
        throw NX::exception(WIN32_ERROR_MSG2(result));
    }
}

sid::sid() : _sid(NULL)
{
}

sid::sid(PSID p) : _sid(NULL)
{
    if (p != NULL && IsValidSid(p)) {
        unsigned long size = GetLengthSid(p);
        if (0 != size) {
            _sid = (PSID)LocalAlloc(LMEM_FIXED, size);
            if (NULL != _sid) {
                if (!CopySid(size, _sid, p)) {
                    LocalFree(_sid);
                    _sid = NULL;
                }
            }
        }
    }
}

sid::sid(const std::wstring& s) : _sid(NULL)
{
    if (!ConvertStringSidToSidW(s.c_str(), &_sid)) {
        _sid = NULL;
    }
}

sid::~sid()
{
    clear();
}


const SID_IDENTIFIER_AUTHORITY sid::null_authority   = { 0,0,0,0,0,0 };
const SID_IDENTIFIER_AUTHORITY sid::world_authority  = { 0,0,0,0,0,1 };
const SID_IDENTIFIER_AUTHORITY sid::nt_authority     = { 0,0,0,0,0,5 };

sid sid::create(PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority,
                BYTE nSubAuthorityCount,
                DWORD dwSubAuthority0,
                DWORD dwSubAuthority1,
                DWORD dwSubAuthority2,
                DWORD dwSubAuthority3,
                DWORD dwSubAuthority4,
                DWORD dwSubAuthority5,
                DWORD dwSubAuthority6,
                DWORD dwSubAuthority7)
{
    sid s;
    PSID p = NULL;
    if (AllocateAndInitializeSid(pIdentifierAuthority,
                                 nSubAuthorityCount,
                                 dwSubAuthority0,
                                 dwSubAuthority1,
                                 dwSubAuthority2,
                                 dwSubAuthority3,
                                 dwSubAuthority4,
                                 dwSubAuthority5,
                                 dwSubAuthority6,
                                 dwSubAuthority7, &p) && p != NULL) {
        s = p;
        FreeSid(p);
        p = NULL;
    }
    return s;
}

std::wstring sid::serialize(PSID psid)
{
    if (NULL == psid) {
        return std::wstring();
    }

    std::wstring s;
    LPWSTR str = NULL;
    if (ConvertSidToStringSid(psid, &str)) {
        s = str;
        LocalFree(str);
        str = NULL;
    }
    assert(NULL == str);
    return std::move(s);
}

bool sid::is_null_auth(PSID psid)
{
    PSID_IDENTIFIER_AUTHORITY pauth = GetSidIdentifierAuthority(psid);
    return (NULL != pauth && 0 == memcmp(pauth, &sid::null_authority, sizeof(SID_IDENTIFIER_AUTHORITY)));
}

bool sid::is_world_auth(PSID psid)
{
    PSID_IDENTIFIER_AUTHORITY pauth = GetSidIdentifierAuthority(psid);
    return (NULL != pauth && 0 == memcmp(pauth, &sid::world_authority, sizeof(SID_IDENTIFIER_AUTHORITY)));
}

bool sid::is_nt_auth(PSID psid)
{
    PSID_IDENTIFIER_AUTHORITY pauth = GetSidIdentifierAuthority(psid);
    return (NULL != pauth && 0 == memcmp(pauth, &sid::nt_authority, sizeof(SID_IDENTIFIER_AUTHORITY)));
}

bool sid::is_everyone_sid(PSID psid)
{
    return IsWellKnownSid(psid, WinWorldSid) ? true : false;
}

bool sid::is_nt_local_system_sid(PSID psid)
{
    return IsWellKnownSid(psid, WinLocalSystemSid) ? true : false;
}

bool sid::is_nt_local_service_sid(PSID psid)
{
    return IsWellKnownSid(psid, WinLocalServiceSid) ? true : false;
}

bool sid::is_nt_network_service_sid(PSID psid)
{
    return IsWellKnownSid(psid, WinNetworkServiceSid) ? true : false;
}

bool sid::is_nt_domain_sid(PSID psid)
{
    return (sid::is_nt_auth(psid) && (*GetSidSubAuthorityCount(psid) != 0) && (SECURITY_NT_NON_UNIQUE == *GetSidSubAuthority(psid, 0)));
}

bool sid::is_nt_builtin_sid(PSID psid)
{
    return (sid::is_nt_auth(psid) && (*GetSidSubAuthorityCount(psid) != 0) && (SECURITY_BUILTIN_DOMAIN_RID == *GetSidSubAuthority(psid, 0)));
}

void sid::clear()
{
    if (NULL != _sid) {
        LocalFree(_sid);
        _sid = NULL;
    }
}

unsigned long sid::length() const
{
    return empty() ? 0UL : GetLengthSid(_sid);
}

sid& sid::operator = (const sid& other)
{
    if (this != &other) {
        clear();
        unsigned long size = other.length();
        if (0 != size) {
            _sid = (PSID)LocalAlloc(LMEM_FIXED, size);
            if (NULL != _sid) {
                if (!CopySid(size, _sid, other)) {
                    LocalFree(_sid);
                    _sid = NULL;
                }
            }
        }
    }
    return *this;
}

sid& sid::operator = (PSID other)
{
    clear();
    if (NULL != other) {
        unsigned long size = GetLengthSid(other);
        if (0 != size) {
            _sid = (PSID)LocalAlloc(LMEM_FIXED, size);
            if (NULL != _sid) {
                if (!CopySid(size, _sid, other)) {
                    LocalFree(_sid);
                    _sid = NULL;
                }
            }
        }
    }
    return *this;
}

std::wstring sid::serialize() const
{
    return sid::serialize(_sid);
}

bool sid::operator == (const sid& other) const
{
    if (empty() || other.empty()) {
        return (empty() && other.empty());
    }
    return EqualSid(_sid, other) ? true : false;
}

bool sid::operator == (PSID other) const
{
    if (empty() || NULL == other) {
        return (empty() && NULL == other);
    }
    return EqualSid(_sid, other) ? true : false;
}


//
//
//

host::host()
{
    unsigned long size = MAX_PATH;
    if (!GetComputerNameExW(ComputerNameDnsFullyQualified, NX::string_buffer<wchar_t>(_fqdn, MAX_PATH), &size)) {
        _fqdn.clear();
    }
    size = MAX_PATH;
    if (!GetComputerNameExW(ComputerNameDnsHostname, NX::string_buffer<wchar_t>(_host, MAX_PATH), &size)) {
        _host.clear();
    }
    size = MAX_PATH;
    if (!GetComputerNameExW(ComputerNameDnsDomain, NX::string_buffer<wchar_t>(_domain, MAX_PATH), &size)) {
        _domain.clear();
    }
}

host::~host()
{
}

void host::clear()
{
    _fqdn.clear();
    _host.clear();
    _domain.clear();
}

host& host::operator = (const host& other)
{
    if (this != &other) {
        _fqdn = other.fqdn_name();
        _host = other.dns_host_name();
        _domain = other.dns_domain_name();
    }
    return *this;
}

bool host::operator == (const host& other) const
{
    return NX::utility::iequal<wchar_t>(other.fqdn_name(), fqdn_name());
}

bool host::operator == (const std::wstring& other) const
{
    return NX::utility::iequal<wchar_t>(other, fqdn_name());
}

std::wstring win::sam_compatiple_name_to_principle_name(const std::wstring& name)
{
    std::wstring principle_name;
    unsigned long size = MAX_PATH;
    if (!::TranslateNameW(name.c_str(), NameSamCompatible, NameUserPrincipal, NX::string_buffer<wchar_t>(principle_name, MAX_PATH), &size)) {
        principle_name.clear();
    }
    return std::move(principle_name);
}

std::wstring win::principle_name_to_sam_compatiple_name(const std::wstring& name)
{
    std::wstring sam_compatiple_name;
    unsigned long size = MAX_PATH;
    if (!::TranslateNameW(name.c_str(), NameUserPrincipal, NameSamCompatible, NX::string_buffer<wchar_t>(sam_compatiple_name, MAX_PATH), &size)) {
        sam_compatiple_name.clear();
    }
    return std::move(sam_compatiple_name);
}

std::wstring win::get_object_name(PSID psid)
{
    static const unsigned long max_name_size = 64;
    static  const win::host h;

    std::wstring name;
    std::wstring dns_user_name;
    std::wstring dns_domain_name;
    unsigned long dns_user_name_size = max_name_size;
    unsigned long dns_domain_name_size = max_name_size;
    SID_NAME_USE sid_name_use = SidTypeUser;


    if (sid::is_nt_local_system_sid(psid)) {
        return std::wstring(L"SYSTEM");
    }
    else if (sid::is_nt_local_service_sid(psid)) {
        return std::wstring(L"LOCAL SERVICE");
    }
    else if (sid::is_nt_network_service_sid(psid)) {
        return std::wstring(L"NETWORK SERVICE");
    }
    else {
        ; // Nothing
    }

    if (!LookupAccountSidW(NULL,
                           psid,
                           NX::string_buffer<wchar_t>(dns_user_name, max_name_size),
                           &dns_user_name_size,
                           NX::string_buffer<wchar_t>(dns_domain_name, max_name_size),
                           &dns_domain_name_size,
                           &sid_name_use)) {
        if (ERROR_NONE_MAPPED == GetLastError()) {
            return std::wstring();
        }
    }

    switch (sid_name_use)
    {
    case SidTypeUser:
    case SidTypeGroup:
        if (dns_domain_name.empty()) {
            name = std::move(dns_user_name);
        }
        else {
            if (NX::utility::iequal(h.dns_host_name(), dns_domain_name)) {
                name = std::move(dns_user_name);
            }
            else {
                std::wstring sam_compatible_name(dns_domain_name + L"\\" + dns_user_name);
                name = win::sam_compatiple_name_to_principle_name(sam_compatible_name);
            }
        }
        break;
    case SidTypeAlias:
        name = dns_domain_name + L"\\" + dns_user_name;
        break;
    case SidTypeWellKnownGroup:
        if (sid::is_everyone_sid(psid)) {
            name = std::move(dns_user_name);
        }
        break;
    case SidTypeDomain:
    case SidTypeDeletedAccount:
    case SidTypeInvalid:
    case SidTypeUnknown:
    case SidTypeComputer:
    case SidTypeLabel:
        break;
    }

    return std::move(name);
}

//
//
//

user_or_group::user_or_group()
{
}

user_or_group::user_or_group(const std::wstring& uid, const std::wstring& uname) : _id(uid), _name(uname)
{
}

user_or_group::~user_or_group()
{
}

user_or_group& user_or_group::operator = (const user_or_group& other)
{
    if (this != &other) {
        _id = other.id();
        _name = other.name();
    }
    return *this;
}

bool user_or_group::operator == (const user_or_group& other) const
{
    return (NX::utility::iequal<wchar_t>(id(), other.id()) && NX::utility::iequal<wchar_t>(name(), other.name()));
}


token::token() : _h(NULL)
{
}

token::token(HANDLE h) : _h(h)
{
}

token::~token()
{
}

user_or_group token::get_user() const
{
    std::vector<unsigned char> buf;
    PTOKEN_USER token_user = NULL;
    unsigned long size = 0;

    GetTokenInformation(_h, TokenUser, NULL, 0, &size);
    if (0 == size) {
        throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
    }

    size += sizeof(TOKEN_USER);
    buf.resize(size, 0);
    token_user = (PTOKEN_USER)buf.data();

    if (!GetTokenInformation(_h, TokenUser, token_user, size, &size)) {
        throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
    }

    const std::wstring& user_sid = sid::serialize(token_user->User.Sid);
    const std::wstring& user_name = get_object_name(token_user->User.Sid);
    return win::user_or_group(user_sid, user_name);
}

std::vector<user_or_group> token::get_user_groups() const
{
    std::vector<user_or_group>  groups;
    std::vector<unsigned char> buf;
    PTOKEN_GROUPS token_groups = NULL;
    unsigned long size = 0;

    GetTokenInformation(_h, TokenGroups, NULL, 0, &size);
    if (0 == size) {
        throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
    }

    size += sizeof(TOKEN_GROUPS);
    buf.resize(size, 0);
    token_groups = (PTOKEN_GROUPS)buf.data();

    if (!GetTokenInformation(_h, TokenGroups, token_groups, size, &size)) {
        throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
    }

    for (int i = 0; i < (int)token_groups->GroupCount; i++) {
        const std::wstring& group_sid = sid::serialize(token_groups->Groups[i].Sid);
        const std::wstring& group_name = get_object_name(token_groups->Groups[i].Sid);
        if (!group_name.empty()) {
            groups.push_back(user_or_group(group_sid, group_name));
        }
    }

    return std::move(groups);
}

process_token::process_token() : token()
{
}

process_token::process_token(unsigned long process_id) : token()
{
    if (0 != process_id) {
        HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, process_id);
        if (NULL != process_handle) {
            if (!OpenProcessToken(process_handle, TOKEN_QUERY, &_h)) {
                _h = NULL;
            }
            CloseHandle(process_handle);
        }
    }
    else {
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &_h)) {
            _h = NULL;
        }
    }
}

process_token::~process_token()
{
}

session_token::session_token() : token()
{
}

session_token::session_token(unsigned long session_id) : token()
{
    if (!WTSQueryUserToken(session_id, &_h)) {
        _h = NULL;
    }
}

session_token::~session_token()
{
}