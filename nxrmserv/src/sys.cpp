

#include <winsock2.h>
#include <Windows.h>
#include <iphlpapi.h>
#include <Ws2tcpip.h>
#include <Mstcpip.h>
#include <stdio.h>
#include <assert.h>
#include <tlhelp32.h>
#include <Wtsapi32.h>
#include <Userenv.h>
#include <Sddl.h>
#define SECURITY_WIN32
#include <security.h>

#include <string>
#include <vector>
#include <algorithm>

#include <nudf\exception.hpp>
#include <nudf\string.hpp>
#include <nudf\registry.hpp>
#include <nudf\crypto.hpp>
#include <nudf\cert.hpp>

#include "nxrmserv.h"
#include "sys.hpp"


using namespace NX;





namespace NX {
namespace sys {


#define NXRM_MEMTUNING_MAX_WORKINGSET             (5*1024*1024)
#define NXRM_MEMTUNING_MIN_WORKINGSET             (3*1024*1024)

void trim_working_set()
{
    //CoFreeUnusedLibraries();
    //SetProcessWorkingSetSize(GetCurrentProcess(), NXRM_MEMTUNING_MIN_WORKINGSET, NXRM_MEMTUNING_MAX_WORKINGSET);
}

class osinf
{
public:
    enum {
        WINUNKNOWN = 0,
        WIN2000,
        WINXP,
        WINXP64PRO,
        WIN2K3,
        WINHOMESRV,
        WIN2K3R2,
        WINVISTA,
        WIN2K8,
        WIN2K8R2,
        WIN7,
        WIN2K12,
        WIN8,
        WIN2K12R2,
        WIN81,
        WIN10,
        WIN2K16
    };
    osinf() : _osid(WINUNKNOWN), _arch(arch_unknown), _os_type(0), _lcid(0), _langid(0), _uilangid(0), _os_name(L"Unknown"), _os_type_name(L"Unknown")
    {
        WCHAR wzLcName[128] = {0};

#ifdef _WIN64
        _arch = arch_amd64;
#else
        BOOL IsWow64 = FALSE;
        _arch = (IsWow64Process(GetCurrentProcess(), &IsWow64) && IsWow64) ? arch_amd64 : arch_i386;
#endif

        _lcid = ::GetSystemDefaultLCID();
        _langid = ::GetSystemDefaultLangID();
        _uilangid = ::GetSystemDefaultUILanguage();
        ::GetSystemDefaultLocaleName(wzLcName, 128);
        _os_locale_name = wzLcName;

        memset(&_osver, 0, sizeof(_osver));
        _osver.dwOSVersionInfoSize  = sizeof(OSVERSIONINFOEXW);
#pragma warning(push)
#pragma warning(disable: 4996)
        GetVersionExW((LPOSVERSIONINFO)&_osver);
#pragma warning(pop)

        if(5 == _osver.dwMajorVersion) {

            if(0 == _osver.dwMinorVersion) {
                _osid = WIN2000;
                _server = false;
                _os_name = L"Windows 2000";
            }
            else if (1 == _osver.dwMinorVersion){
                _osid = WINXP;
                _server = false;
                _os_name = L"Windows XP";
            }
            else if (2 == _osver.dwMinorVersion){
                SYSTEM_INFO sysinfo;
                GetSystemInfo(&sysinfo);
                if(_osver.wProductType==VER_NT_WORKSTATION && sysinfo.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64) {
                    _osid = WINXP64PRO;
                    _server = false;
                    _os_name = L"Windows XP Professional x64 Edition";
                }
                else if(_osver.wSuiteMask & VER_SUITE_WH_SERVER) {
                    _osid = WINHOMESRV;
                    _server = true;
                    _os_name = L"Windows Home Server";
                }
                else {
                    _osid = (0 == GetSystemMetrics(SM_SERVERR2)) ? WIN2K3 : WIN2K3R2;
                    _server = true;
                    _os_name = (0 == GetSystemMetrics(SM_SERVERR2)) ? L"Windows Server 2003" : L"Windows Server 2003 R2";
                }
            }
            else {
                _osid = WINUNKNOWN;
                _server = false;
            }
        }
        else if(6 == _osver.dwMajorVersion) {

            if(0 == _osver.dwMinorVersion) {
                _osid = (_osver.wProductType == VER_NT_WORKSTATION) ? WINVISTA : WIN2K8;
                _server = (_osver.wProductType == VER_NT_WORKSTATION) ? false : true;
                _os_name = (_osver.wProductType == VER_NT_WORKSTATION) ? L"Windows Vista" : L"Windows Server 2008";
            }
            else if (1 == _osver.dwMinorVersion){
                _osid = (_osver.wProductType == VER_NT_WORKSTATION) ? WIN7 : WIN2K8R2;
                _server = (_osver.wProductType == VER_NT_WORKSTATION) ? false : true;
                _os_name = (_osver.wProductType == VER_NT_WORKSTATION) ? L"Windows 7" : L"Windows Server 2008 R2";
            }
            else if (2 == _osver.dwMinorVersion){
                _osid = (_osver.wProductType == VER_NT_WORKSTATION) ? WIN8 : WIN2K12;
                _server = (_osver.wProductType == VER_NT_WORKSTATION) ? false : true;
                _os_name = (_osver.wProductType == VER_NT_WORKSTATION) ? L"Windows 8" : L"Windows Server 2012";
            }
            else if (3 == _osver.dwMinorVersion){
                _osid = (_osver.wProductType == VER_NT_WORKSTATION) ? WIN81 : WIN2K12R2;
                _server = (_osver.wProductType == VER_NT_WORKSTATION) ? false : true;
                _os_name = (_osver.wProductType == VER_NT_WORKSTATION) ? L"Windows 8.1" : L"Windows Server 2012 R2";
            }
            else {
                _osid = WINUNKNOWN;
                _server = false;
            }
        }
        else if(10 == _osver.dwMajorVersion) {
            if(0 == _osver.dwMinorVersion) {
                _osid = (_osver.wProductType == VER_NT_WORKSTATION) ? WIN10 : WIN2K16;
                _server = (_osver.wProductType == VER_NT_WORKSTATION) ? false : true;
                _os_name = (_osver.wProductType == VER_NT_WORKSTATION) ? L"Windows 10" : L"Windows Server 2016";
            }
            else {
                _osid = WINUNKNOWN;
                _server = false;
            }
        }
        else {
            _osid = WINUNKNOWN;
            _server = false;
        }

        GetProductInfo(_osver.dwMajorVersion, _osver.dwMinorVersion, _osver.wServicePackMajor, _osver.wServicePackMinor, &_os_type);
        switch(_os_type)
        {
        case PRODUCT_ULTIMATE:
            _os_type_name = L"Ultimate Edition";
            break;
        case PRODUCT_PROFESSIONAL:
            _os_type_name = L"Professional";
            break;
        case PRODUCT_HOME_PREMIUM:
            _os_type_name = L"Home Premium Edition";
            break;
        case PRODUCT_HOME_BASIC:
            _os_type_name = L"Home Basic Edition";
            break;
        case PRODUCT_ENTERPRISE:
            _os_type_name = L"Enterprise Edition";
            break;
        case PRODUCT_BUSINESS:
            _os_type_name = L"Business Edition";
            break;
        case PRODUCT_STARTER:
            _os_type_name = L"Starter Edition";
            break;
        case PRODUCT_CLUSTER_SERVER:
            _os_type_name = L"Cluster Server Edition";
            break;
        case PRODUCT_DATACENTER_SERVER:
            _os_type_name = L"Datacenter Edition";
            break;
        case PRODUCT_DATACENTER_SERVER_CORE:
            _os_type_name = L"Datacenter Edition (core installation)";
            break;
        case PRODUCT_ENTERPRISE_SERVER:
            _os_type_name = L"Enterprise Edition";
            break;
        case PRODUCT_ENTERPRISE_SERVER_CORE:
            _os_type_name = L"Enterprise Edition (core installation)";
            break;
        case PRODUCT_ENTERPRISE_SERVER_IA64:
            _os_type_name = L"Enterprise Edition for Itanium-based Systems";
            break;
        case PRODUCT_SMALLBUSINESS_SERVER:
            _os_type_name = L"Small Business Server";
            break;
        case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
            _os_type_name = L"Small Business Server Premium Edition";
            break;
        case PRODUCT_STANDARD_SERVER:
            _os_type_name = L"Standard Edition";
            break;
        case PRODUCT_STANDARD_SERVER_CORE:
            _os_type_name = L"Standard Edition (core installation)";
            break;
        case PRODUCT_WEB_SERVER:
            _os_type_name = L"Web Server Edition";
            break;
        default:
            break;
        }
    }

    ~osinf(){}


    bool is_server() noexcept {return _server;}
    bool is_workstation() noexcept {return ((!_server) && 0!=_osid);}
    bool is_32bits() noexcept {return (_arch == arch_i386);}
    bool is_64bits() noexcept {return (_arch == arch_amd64);}
    ULONG os_type() noexcept {return _os_type;}

    LCID locale_id() noexcept {return _lcid;}
    LANGID language_id() const noexcept {return _langid;}
    LANGID ui_language_id() const noexcept {return _uilangid;}

    const std::wstring& os_name() const noexcept {return _os_name;}
    const std::wstring& os_type_name() const noexcept {return _os_type_name;}
    const std::wstring& os_locale_name() const noexcept {return _os_locale_name;}
    
    bool is_win_2000(bool or_later) const noexcept {return or_later ? (_osid==WIN2000) : (_osid>=WIN2000);}
    bool is_win_xp(bool or_later) const noexcept {return or_later ? (_osid==WINXP) : (_osid>=WINXP);}
    bool is_win_xp_64pro(bool or_later) const noexcept {return or_later ? (_osid==WINXP64PRO) : (_osid>=WINXP64PRO);}
    bool is_win_home_server(bool or_later) const noexcept {return or_later ? (_osid==WINHOMESRV) : (_osid>=WINHOMESRV);}
    bool is_win_2003(bool or_later) const noexcept {return or_later ? (_osid==WIN2K3) : (_osid>=WIN2K3);}
    bool is_win_2003r2(bool or_later) const noexcept {return or_later ? (_osid==WIN2K3R2) : (_osid>=WIN2K3R2);}
    bool is_win_vista(bool or_later) const noexcept {return or_later ? (_osid==WINVISTA) : (_osid>=WINVISTA);}
    bool is_win_2008(bool or_later) const noexcept {return or_later ? (_osid==WIN2K8) : (_osid>=WIN2K8);}
    bool is_win_7(bool or_later) const noexcept {return or_later ? (_osid==WIN7) : (_osid>=WIN7);}
    bool is_win_2008r2(bool or_later) const noexcept {return or_later ? (_osid==WIN2K8R2) : (_osid>=WIN2K8R2);}
    bool is_win_8(bool or_later) const noexcept {return or_later ? (_osid==WIN8) : (_osid>=WIN8);}
    bool is_win_2012(bool or_later) const noexcept {return or_later ? (_osid==WIN2K12) : (_osid>=WIN2K12);}
    bool is_win_81(bool or_later) const noexcept {return or_later ? (_osid==WIN81) : (_osid>=WIN81);}
    bool is_win_2012r2(bool or_later) const noexcept {return or_later ? (_osid==WIN2K12R2) : (_osid>=WIN2K12R2);}
    bool is_win_10(bool or_later) const noexcept {return or_later ? (_osid==WIN10) : (_osid>=WIN10);}
    bool is_win_2016(bool or_later) const noexcept {return or_later ? (_osid==WIN2K16) : (_osid>=WIN2K16);}

public:
    OSVERSIONINFOEXW    _osver;
    CPUARCH             _arch;
    bool                _server;

    ULONG               _osid;
    ULONG               _os_type;

    LCID                _lcid;
    LANGID              _langid;
    LANGID              _uilangid;

    std::wstring        _os_name;
    std::wstring        _os_type_name;
    std::wstring        _os_locale_name;
};

class cpu_info
{
    // forward declarations
    class CCpuIdData;
    class CBits;

    template<unsigned long N>
    class CBitsMap
    {
    public:
        CBitsMap() throw()
        {
            _map.resize((N + 31) / 32, 0);
        }

        virtual ~CBitsMap() throw()
        {
        }

        bool Get(_In_ unsigned long bit) const throw()
        {
            if (bit >= N) {
                return false;
            }
            return (0 != (_map[bit / 32] & GetBitMask(bit % 32)));
        }

        bool operator [](_In_ unsigned long bit) const throw()
        {
            if (bit >= N) {
                return false;
            }
            return (0 != (_map[bit / 32] & GetBitMask(bit % 32)));
        }

        void Set(_In_ unsigned long bit) const throw()
        {
            if (bit >= N) {
                return;
            }
            _map[bit / 32] |= GetBitMask(bit % 32);
        }

        void Clear(_In_ unsigned long bit) const throw()
        {
            if (bit >= N) {
                return;
            }
            _map[bit / 32] &= (~GetBitMask(bit % 32));
        }

        CBitsMap<N>& operator = (_In_ unsigned long data)
        {
            _map[0] = data;
            return *this;
        }

        CBitsMap<N>& operator = (_In_ int data)
        {
            _map[0] = (unsigned long)data;
            return *this;
        }

        unsigned long size() const throw() { return N; }

    protected:
        __forceinline unsigned long GetBitMask(_In_ unsigned long bit) const throw()
        {
            switch (bit)
            {
            case 0: return 0x00000001;
            case 1: return 0x00000002;
            case 2: return 0x00000004;
            case 3: return 0x00000008;
            case 4: return 0x00000010;
            case 5: return 0x00000020;
            case 6: return 0x00000040;
            case 7: return 0x00000080;
            case 8: return 0x00000100;
            case 9: return 0x00000200;
            case 10: return 0x00000400;
            case 11: return 0x00000800;
            case 12: return 0x00001000;
            case 13: return 0x00002000;
            case 14: return 0x00004000;
            case 15: return 0x00008000;
            case 16: return 0x00010000;
            case 17: return 0x00020000;
            case 18: return 0x00040000;
            case 19: return 0x00080000;
            case 20: return 0x00100000;
            case 21: return 0x00200000;
            case 22: return 0x00400000;
            case 23: return 0x00800000;
            case 24: return 0x01000000;
            case 25: return 0x02000000;
            case 26: return 0x04000000;
            case 27: return 0x08000000;
            case 28: return 0x10000000;
            case 29: return 0x20000000;
            case 30: return 0x40000000;
            case 31: return 0x80000000;
            default: break;
            }
            return 0;
        }

    private:
        std::vector<unsigned long> _map;
    };

public:
    cpu_info()
    {
        hyperthreads_ = false;
        cores_ = 0;
        logicals_ = 0;
        Load();
    }
    ~cpu_info()
    {
    }

private:
    void Load()
    {
        CCpuIdData cpui;
        int nIds_;
        int nExIds_;
        unsigned long cpuFeatures = 0;

        // Calling __cpuid with 0x0 as the function_id argument
        // gets the number of the highest valid function ID.
        __cpuid(cpui.data(), 0);
        nIds_ = cpui[0];

        for (int i = 0; i <= nIds_; ++i)
        {
            __cpuidex(cpui.data(), i, 0);
            data_.push_back(cpui);
        }

        // Capture vendor string
        char vendor[0x20];
        memset(vendor, 0, sizeof(vendor));
        *reinterpret_cast<int*>(vendor) = data_[0][1];
        *reinterpret_cast<int*>(vendor + 4) = data_[0][3];
        *reinterpret_cast<int*>(vendor + 8) = data_[0][2];
        std::string sVender(vendor);
        vendor_ = std::wstring(sVender.begin(), sVender.end());
        if (vendor_ == L"GenuineIntel") {
            isIntel_ = true;
        }
        else if (vendor_ == L"AuthenticAMD") {
            isAMD_ = true;
        }
        else {
            ; // Unknown
        }

        // logical processors
        logicals_ = (data_[1][1] >> 16) & 0xFF;
        cores_ = logicals_;
        cpuFeatures = data_[1][3];


        // load bitset with flags for function 0x00000001
        if (nIds_ >= 1) {
            f_1_ECX_ = data_[1][2];
            f_1_EDX_ = data_[1][3];
        }

        // load bitset with flags for function 0x00000007
        if (nIds_ >= 7) {
            f_7_EBX_ = data_[7][1];
            f_7_ECX_ = data_[7][2];
        }

        // Calling __cpuid with 0x80000000 as the function_id argument
        // gets the number of the highest valid extended ID.
        __cpuid(cpui.data(), 0x80000000);
        nExIds_ = cpui[0];

        char brand[0x40];
        memset(brand, 0, sizeof(brand));

        for (int i = 0x80000000; i <= nExIds_; ++i) {
            __cpuidex(cpui.data(), i, 0);
            extdata_.push_back(cpui);
        }

        // load bitset with flags for function 0x80000001
        if (nExIds_ >= 0x80000001) {
            f_81_ECX_ = extdata_[1][2];
            f_81_EDX_ = extdata_[1][3];
        }

        // Interpret CPU brand string if reported
        if (nExIds_ >= 0x80000004) {
            memcpy(brand, extdata_[2].data(), sizeof(cpui));
            memcpy(brand + 16, extdata_[3].data(), sizeof(cpui));
            memcpy(brand + 32, extdata_[4].data(), sizeof(cpui));
            std::string sBrand(brand);
            brand_ = std::wstring(sBrand.begin(), sBrand.end());
        }

        // Get cores        
        if (isIntel_) {
            cores_ = ((data_[4][0] >> 26) & 0x3f) + 1;
        }
        else if (isAMD_) {
            cores_ = ((unsigned)(extdata_[8][2] & 0xff)) + 1;
        }

        hyperthreads_ = ((cpuFeatures & (1 << 28)) && (cores_ < logicals_));

    }

public:
    // getters
    const std::wstring& Vendor(void) const throw() { return vendor_; }
    const std::wstring& Brand(void) const throw() { return brand_; }

    unsigned long GetCores() const throw() { return cores_; }
    unsigned long GetLogicalProcessors() const throw() { return logicals_; }
    bool HYPERTHREADS() const throw() { return hyperthreads_; }

    bool SSE3(void) const throw() { return f_1_ECX_[0]; }
    bool PCLMULQDQ(void) const throw() { return f_1_ECX_[1]; }
    bool MONITOR(void) const throw() { return f_1_ECX_[3]; }
    bool SSSE3(void) const throw() { return f_1_ECX_[9]; }
    bool FMA(void) const throw() { return f_1_ECX_[12]; }
    bool CMPXCHG16B(void) const throw() { return f_1_ECX_[13]; }
    bool SSE41(void) const throw() { return f_1_ECX_[19]; }
    bool SSE42(void) const throw() { return f_1_ECX_[20]; }
    bool MOVBE(void) const throw() { return f_1_ECX_[22]; }
    bool POPCNT(void) const throw() { return f_1_ECX_[23]; }
    bool AES(void) const throw() { return f_1_ECX_[25]; }
    bool XSAVE(void) const throw() { return f_1_ECX_[26]; }
    bool OSXSAVE(void) const throw() { return f_1_ECX_[27]; }
    bool AVX(void) const throw() { return f_1_ECX_[28]; }
    bool F16C(void) const throw() { return f_1_ECX_[29]; }
    bool RDRAND(void) const throw() { return f_1_ECX_[30]; }

    bool MSR(void) const throw() { return f_1_EDX_[5]; }
    bool CX8(void) const throw() { return f_1_EDX_[8]; }
    bool SEP(void) const throw() { return f_1_EDX_[11]; }
    bool CMOV(void) const throw() { return f_1_EDX_[15]; }
    bool CLFSH(void) const throw() { return f_1_EDX_[19]; }
    bool MMX(void) const throw() { return f_1_EDX_[23]; }
    bool FXSR(void) const throw() { return f_1_EDX_[24]; }
    bool SSE(void) const throw() { return f_1_EDX_[25]; }
    bool SSE2(void) const throw() { return f_1_EDX_[26]; }

    bool FSGSBASE(void) const throw() { return f_7_EBX_[0]; }
    bool BMI1(void) const throw() { return f_7_EBX_[3]; }
    bool HLE(void) const throw() { return isIntel_ && f_7_EBX_[4]; }
    bool AVX2(void) const throw() { return f_7_EBX_[5]; }
    bool BMI2(void) const throw() { return f_7_EBX_[8]; }
    bool ERMS(void) const throw() { return f_7_EBX_[9]; }
    bool INVPCID(void) const throw() { return f_7_EBX_[10]; }
    bool RTM(void) const throw() { return isIntel_ && f_7_EBX_[11]; }
    bool AVX512F(void) const throw() { return f_7_EBX_[16]; }
    bool RDSEED(void) const throw() { return f_7_EBX_[18]; }
    bool ADX(void) const throw() { return f_7_EBX_[19]; }
    bool AVX512PF(void) const throw() { return f_7_EBX_[26]; }
    bool AVX512ER(void) const throw() { return f_7_EBX_[27]; }
    bool AVX512CD(void) const throw() { return f_7_EBX_[28]; }
    bool SHA(void) const throw() { return f_7_EBX_[29]; }

    bool PREFETCHWT1(void) const throw() { return f_7_ECX_[0]; }

    bool LAHF(void) const throw() { return f_81_ECX_[0]; }
    bool LZCNT(void) const throw() { return isIntel_ && f_81_ECX_[5]; }
    bool ABM(void) const throw() { return isAMD_ && f_81_ECX_[5]; }
    bool SSE4a(void) const throw() { return isAMD_ && f_81_ECX_[6]; }
    bool XOP(void) const throw() { return isAMD_ && f_81_ECX_[11]; }
    bool TBM(void) const throw() { return isAMD_ && f_81_ECX_[21]; }

    bool SYSCALL(void) const throw() { return isIntel_ && f_81_EDX_[11]; }
    bool MMXEXT(void) const throw() { return isAMD_ && f_81_EDX_[22]; }
    bool RDTSCP(void) const throw() { return isIntel_ && f_81_EDX_[27]; }
    bool _3DNOWEXT(void) const throw() { return isAMD_ && f_81_EDX_[30]; }
    bool _3DNOW(void) const throw() { return isAMD_ && f_81_EDX_[31]; }

private:
    std::wstring vendor_;
    std::wstring brand_;
    bool isIntel_;
    bool isAMD_;
    unsigned long cores_;
    unsigned long logicals_;
    bool hyperthreads_;

    CBitsMap<32> f_1_ECX_;
    CBitsMap<32> f_1_EDX_;
    CBitsMap<32> f_7_EBX_;
    CBitsMap<32> f_7_ECX_;
    CBitsMap<32> f_81_ECX_;
    CBitsMap<32> f_81_EDX_;
    std::vector<CCpuIdData> data_;
    std::vector<CCpuIdData> extdata_;

    class CCpuIdData
    {
    public:
        CCpuIdData()
        {
            _data[0] = _data[1] = _data[2] = _data[3] = -1;
        }
        ~CCpuIdData()
        {
        }
        CCpuIdData& operator = (const CCpuIdData& cpudata)
        {
            memcpy(_data, cpudata._data, sizeof(_data));
        }

        inline int* data() throw() { return _data; }
        inline int operator [](int id) const { return ((id >= 0 && id<4) ? _data[id] : -1); }

        int _data[4];
    };
};

class mem_info
{
public:
    mem_info()
    {
        memset(&_status, 0, sizeof(_status));
        _status.dwLength = sizeof(_status);
        GlobalMemoryStatusEx(&_status);
    }
    ~mem_info()
    {
    }

    inline const MEMORYSTATUSEX& status() const { return _status; }
private:
    MEMORYSTATUSEX  _status;
};


}   // namespace sys
}   // namespace NX


static NX::sys::osinf       _osinf;
static NX::sys::cpu_info    _cpuinf;
static NX::sys::mem_info    _meminf;

bool NX::sys::os::is_server() noexcept {return _osinf.is_server();}
bool NX::sys::os::is_workstation() noexcept {return _osinf.is_workstation();}
bool NX::sys::os::is_32bits() noexcept {return _osinf.is_32bits();}
bool NX::sys::os::is_64bits() noexcept {return _osinf.is_64bits();}

ULONG NX::sys::os::os_type() noexcept {return _osinf.os_type();}
LCID NX::sys::os::locale_id() noexcept {return _osinf.locale_id();}
LANGID NX::sys::os::language_id() noexcept {return _osinf.language_id();}
LANGID NX::sys::os::ui_language_id() noexcept {return _osinf.ui_language_id();}

const std::wstring& NX::sys::os::os_name() noexcept {return _osinf.os_name();}
const std::wstring& NX::sys::os::os_type_name() noexcept {return _osinf.os_type_name();}
const std::wstring& NX::sys::os::os_locale_name() noexcept {return _osinf.os_locale_name();}

bool NX::sys::os::is_win_2000(bool or_later) noexcept {return _osinf.is_win_2000(or_later);}
bool NX::sys::os::is_win_xp(bool or_later) noexcept {return _osinf.is_win_xp(or_later);}
bool NX::sys::os::is_win_xp_64pro(bool or_later) noexcept {return _osinf.is_win_xp_64pro(or_later);}
bool NX::sys::os::is_win_home_server(bool or_later) noexcept {return _osinf.is_win_home_server(or_later);}
bool NX::sys::os::is_win_2003(bool or_later) noexcept {return _osinf.is_win_2003(or_later);}
bool NX::sys::os::is_win_2003r2(bool or_later) noexcept {return _osinf.is_win_2003r2(or_later);}
bool NX::sys::os::is_win_vista(bool or_later) noexcept {return _osinf.is_win_vista(or_later);}
bool NX::sys::os::is_win_2008(bool or_later) noexcept {return _osinf.is_win_2008(or_later);}
bool NX::sys::os::is_win_7(bool or_later) noexcept {return _osinf.is_win_7(or_later);}
bool NX::sys::os::is_win_2008r2(bool or_later) noexcept {return _osinf.is_win_2008r2(or_later);}
bool NX::sys::os::is_win_8(bool or_later) noexcept {return _osinf.is_win_8(or_later);}
bool NX::sys::os::is_win_2012(bool or_later) noexcept {return _osinf.is_win_2012(or_later);}
bool NX::sys::os::is_win_81(bool or_later) noexcept {return _osinf.is_win_81(or_later);}
bool NX::sys::os::is_win_2012r2(bool or_later) noexcept {return _osinf.is_win_2012r2(or_later);}
bool NX::sys::os::is_win_10(bool or_later) noexcept {return _osinf.is_win_10(or_later);}
bool NX::sys::os::is_win_2016(bool or_later) noexcept {return _osinf.is_win_2016(or_later);}


const std::wstring& NX::sys::hardware::cpu_brand() noexcept { return _cpuinf.Brand(); }
unsigned long NX::sys::hardware::memory_in_mb() noexcept { return (unsigned long)(_meminf.status().ullTotalPhys / 0x100000); }
unsigned __int64 NX::sys::hardware::memory_in_bytes() noexcept {return _meminf.status().ullTotalPhys;}

void NX::sys::hardware::get_net_adapters(std::vector<NX::sys::hardware::net_adapter>& adapters)
{
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
            return;
        }

        dwSize += sizeof(IP_ADAPTER_ADDRESSES);
        buf.resize(dwSize, 0);
        pAddresses = (PIP_ADAPTER_ADDRESSES)buf.data();
        memset(pAddresses, 0, dwSize);
    }

    if(NULL == pAddresses) {
        return;
    }

    dwRetVal = GetAdaptersAddresses(AF_UNSPEC, 0, NULL, pAddresses, &dwSize);
    if (ERROR_SUCCESS != dwRetVal) {
        return;
    }

    pCurAddress = pAddresses;
    do {
        if (IF_TYPE_SOFTWARE_LOOPBACK == pCurAddress->IfType) {
            continue;
        }
        adapters.push_back(NX::sys::hardware::net_adapter(pCurAddress));
    } while (NULL != (pCurAddress = pCurAddress->Next));
}

std::vector<std::wstring> NX::sys::hardware::get_active_ipv4()
{
    std::vector<std::wstring> ipv4_list;
    std::vector<NX::sys::hardware::net_adapter> adapters;
    NX::sys::hardware::get_net_adapters(adapters);
    std::for_each(adapters.begin(), adapters.end(), [&](const NX::sys::hardware::net_adapter& adapter) {
        if (adapter.is_connected()) {
            const std::vector<std::wstring>& list = adapter.ipv4_addresses();
            for (auto it = list.begin(); it != list.end(); ++it) {
                ipv4_list.push_back(*it);
            }
        }
    });
    return ipv4_list;
}

void NX::sys::hardware::get_disks(std::vector<NX::sys::hardware::disk>& disks)
{
    for (wchar_t c = L'C'; c <= L'Z'; c++) {
        NX::sys::hardware::disk d;
        if (d.load(c)) {
            disks.push_back(d);
        }
    }
}

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


typedef PWSTR (NTAPI* FpRtlIpv4AddressToString)(_In_ const IN_ADDR *Addr, _Out_ PWSTR S);
typedef PWSTR (NTAPI* FpRtlIpv6AddressToString)(_In_ const IN6_ADDR *Addr, _Out_ PWSTR S);

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

NX::sys::hardware::net_adapter::net_adapter(_In_ void* address)
{
    PIP_ADAPTER_ADDRESSES pAdapter = (PIP_ADAPTER_ADDRESSES)address;
    std::string s(pAdapter->AdapterName);
    _name = std::wstring(s.begin(), s.end());
    if (NULL != pAdapter->FriendlyName) {
        _friendly_name = pAdapter->FriendlyName;
    }
    if (NULL != pAdapter->FriendlyName) {
        _description = pAdapter->Description;
    }
    if (NULL != pAdapter->PhysicalAddress && 0 != pAdapter->PhysicalAddressLength) {
        std::for_each(pAdapter->PhysicalAddress, pAdapter->PhysicalAddress + pAdapter->PhysicalAddressLength, [&](const BYTE v) {
            wchar_t buf[8] = { 0 };
            swprintf_s(buf, 8, L"%02X", v);
            if(!_phyaddr.empty()) _phyaddr += L"-";
            _phyaddr += buf;
        });
    }
    _trans_link_speed = pAdapter->TransmitLinkSpeed;
    _recv_link_speed = pAdapter->ReceiveLinkSpeed;
    _if_type = pAdapter->IfType;
    switch (_if_type)
    {
    case IF_TYPE_OTHER: _if_type_name = L"Others"; break;
    case IF_TYPE_ETHERNET_CSMACD: _if_type_name = L"Ethernet network interface"; break;
    case IF_TYPE_ISO88025_TOKENRING: _if_type_name = L"Token ring network interface"; break;
    case IF_TYPE_PPP: _if_type_name = L"PPP network interface"; break;
    case IF_TYPE_SOFTWARE_LOOPBACK: _if_type_name = L"Software loopback network interface"; break;
    case IF_TYPE_ATM: _if_type_name = L"ATM network interface"; break;
    case IF_TYPE_IEEE80211: _if_type_name = L"IEEE 802.11 wireless network interface"; break;
    case IF_TYPE_TUNNEL: _if_type_name = L"Tunnel type encapsulation network interface"; break;
    case IF_TYPE_IEEE1394: _if_type_name = L"IEEE 1394 (Firewire) high performance serial bus network interface"; break;
    default: _if_type_name = L"Unknown"; break;
    }
    _ipv4_enabled = pAdapter->Ipv4Enabled;
    _ipv6_enabled = pAdapter->Ipv6Enabled;
    _oper_status = pAdapter->OperStatus;
    switch (_oper_status)
    {
    case IfOperStatusUp: _oper_status_name = L"Up"; break;
    case IfOperStatusDown: _oper_status_name = L"Down"; break;
    case IfOperStatusTesting: _oper_status_name = L"Testing Mode"; break;
    case IfOperStatusDormant: _oper_status_name = L"Pending"; break;
    case IfOperStatusNotPresent: _oper_status_name = L"Down (Component not present)"; break;
    case IfOperStatusLowerLayerDown: _oper_status_name = L"Down (Lower layer interface is down)"; break;
    case IfOperStatusUnknown:
    default: _oper_status_name = L"Unknown"; break;
    }

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
        else if(ip_address->Address.lpSockaddr->sa_family == AF_INET6) {
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

bool NX::sys::hardware::net_adapter::is_connected() const noexcept
{
    return (_oper_status == IfOperStatusUp);
}

std::wstring NX::sys::hardware::disk::type_string() const noexcept
{
    switch (_type)
    {
    case DRIVE_REMOVABLE: return L"Removable";
    case DRIVE_FIXED:     return L"Fixed    ";
    case DRIVE_REMOTE:    return L"Remote   ";
    case DRIVE_CDROM:     return L"CD/DVD   ";
    case DRIVE_RAMDISK:   return L"RAM Disk ";
    default:  break;
    }
    return L"Unknown Type";
}

void NX::sys::hardware::disk::clear() noexcept
{
    _name.clear();
    _type = 0;
    _total = 0;
    _available_free = 0;
    _total_free = 0;
}

bool NX::sys::hardware::disk::empty() const noexcept
{
    return _name.empty();
}

bool NX::sys::hardware::disk::is_fixed() const noexcept
{
    return (_type == DRIVE_FIXED);
}

bool NX::sys::hardware::disk::is_removable() const noexcept
{
    return (_type == DRIVE_REMOVABLE);
}

bool NX::sys::hardware::disk::is_remote() const noexcept
{
    return (_type == DRIVE_REMOTE);
}

bool NX::sys::hardware::disk::is_cdrom() const noexcept
{
    return (_type == DRIVE_CDROM);
}

bool NX::sys::hardware::disk::is_ramdiak() const noexcept
{
    return (_type == DRIVE_RAMDISK);
}

bool NX::sys::hardware::disk::load(const wchar_t drive) noexcept
{
    UINT uType = 0;
    WCHAR wzDrive[4] = { drive, L':', L'\\', 0 };

    clear();

    uType = ::GetDriveTypeW(wzDrive);
    if (DRIVE_FIXED == uType || DRIVE_REMOTE == uType || DRIVE_REMOVABLE == uType || DRIVE_CDROM == uType) {
        ULARGE_INTEGER liFreeBytesAvailable = { 0, 0 };
        ULARGE_INTEGER liTotalNumberOfBytes = { 0, 0 };
        ULARGE_INTEGER liTotalNumberOfFreeBytes = { 0, 0 };
        GetDiskFreeSpaceExW(wzDrive, &liFreeBytesAvailable, &liTotalNumberOfBytes, &liTotalNumberOfFreeBytes);

        wzDrive[2] = L'\0';
        _name = wzDrive;
        _type = uType;
        _total = liTotalNumberOfBytes.QuadPart;
        _available_free = liFreeBytesAvailable.QuadPart;
        _total_free = liTotalNumberOfFreeBytes.QuadPart;
        return true;
    }

    return false;
}

//
//  class NX::version
//
version::version() : _major(0),_minor(0),_build(0)
{
}

version::version(const std::wstring& v) : _major(0),_minor(0),_build(0)
{
    load_string(v);
}

version::version(unsigned long major, unsigned long minor, unsigned long build) : _major(major),_minor(minor),_build(build)
{
    std::vector<wchar_t> buf;
    buf.resize(64, 0);
    swprintf_s(&buf[0], 64, L"%d.%d.%d", major, minor, build);
    _str = &buf[0];
}

version::~version()
{
}

version& version::operator = (const version& v)
{
    if(this != &v) {
        _str = v.version_str();
        _major = v.version_major();
        _minor = v.version_minor();
        _build = v.version_build();
    }
    return *this;
}

bool version::operator == (const version& v)
{
    return (_str == v.version_str());
}

bool version::operator == (const std::wstring& v)
{
    return (_str == v);
}

void version::clear() noexcept
{
    _str.clear();
    _major = 0;
    _minor = 0;
    _build = 0;
}

void version::load_string(const std::wstring& v) noexcept
{
    std::wstring major;
    std::wstring minor;
    std::wstring build;

    if(vdalidate_string(v)) {

        std::wstring s(v);
        std::wstring::size_type pos;

        // 1st dot
        pos = s.find(L'.');
        if(pos == std::wstring::npos) {
            // not even one '.'
            major = s;
        }
        else {
            major = s.substr(0, pos);
            s = s.substr(pos+1);
        }

        // 2nd dot
        pos = s.find(L'.');
        if(pos == std::wstring::npos) {
            // not even one '.'
            minor = s;
        }
        else {
            minor = s.substr(0, pos);
            s = s.substr(pos+1);
        }

        // 3rd dot
        pos = s.find(L'.');
        if(pos == std::wstring::npos) {
            // not even one '.'
            build = s;
        }
        else {
            build = s.substr(0, pos);
        }

        // output
        _str = major + L"." + minor + L"." + build;
        _major = major.empty() ? 0 : _wtoi(major.c_str());
        _minor = minor.empty() ? 0 : _wtoi(minor.c_str());
        _build = build.empty() ? 0 : _wtoi(build.c_str());
    }
}


typedef bool (is_digit)(const wchar_t);
bool version::vdalidate_string(const std::wstring& v) noexcept
{
    std::wstring::const_iterator pos = std::find_if_not<std::wstring::const_iterator, is_digit>(v.begin(), v.end(), [](const wchar_t c)->bool{return (c==L'.' || (c>=L'0' && c<=L'9'));});
    return (pos == v.end());
}


//
//  class NX::module
//

module::module()
{
}

module::module(const std::wstring& module_path, bool lowercase) : _full_path(module_path)
{
    if(lowercase) {
        std::transform(_full_path.begin(), _full_path.end(), _full_path.begin(), tolower);
    }
    module::parse_filepath(_full_path, _parent_dir, _image_name);
}

module::module(HMODULE m, bool lowercase)
{
    std::vector<wchar_t> buf;
    buf.resize(MAX_PATH+1, 0);
    GetModuleFileNameW(m, &buf[0], MAX_PATH);

    _full_path = &buf[0];
    if(lowercase) {
        std::transform(_full_path.begin(), _full_path.end(), _full_path.begin(), tolower);
    }
    module::parse_filepath(_full_path, _parent_dir, _image_name);
}

module::~module()
{
}

void module::clear() noexcept
{
    _full_path.clear();
    _parent_dir.clear();
    _image_name.clear();
}

void module::to_lower() noexcept
{
    std::transform(_full_path.begin(), _full_path.end(), _full_path.begin(), tolower);
    std::transform(_parent_dir.begin(), _parent_dir.end(), _parent_dir.begin(), tolower);
    std::transform(_image_name.begin(), _image_name.end(), _image_name.begin(), tolower);
}

void module::to_upper() noexcept
{
    std::transform(_full_path.begin(), _full_path.end(), _full_path.begin(), toupper);
    std::transform(_parent_dir.begin(), _parent_dir.end(), _parent_dir.begin(), toupper);
    std::transform(_image_name.begin(), _image_name.end(), _image_name.begin(), toupper);
}

module& module::operator = (const module& m)
{
    if(this != &m) {
        _full_path = m.full_path();
        _parent_dir = m.parent_dir();
        _image_name = m.image_name();
    }
    return *this;
}

module& module::operator = (const std::wstring& module_path)
{
    _full_path = module_path;
    module::parse_filepath(_full_path, _parent_dir, _image_name);
    return *this;
}

void module::parse_filepath(const std::wstring& fullpath, std::wstring& parent_dir, std::wstring& final_part)
{
    std::wstring::size_type pos = fullpath.find_last_of(L'\\');
    if(pos == std::wstring::npos) {
        parent_dir.clear();
        final_part = fullpath;
    }
    else {
        parent_dir = fullpath.substr(0, pos);
        final_part = fullpath.substr(pos+1);
    }
}

void module::parse_filename(const std::wstring& filename, std::wstring& first_part, std::wstring& suffix)
{
    std::wstring::size_type pos = filename.find_last_of(L'.');
    if(pos == std::wstring::npos) {
        suffix.clear();
        first_part = filename;
    }
    else {
        first_part = filename.substr(0, pos);
        suffix = filename.substr(pos);
    }
}


//
//  class language
//

const unsigned long language::default_lang_id = 1033;
const std::wstring language::default_lang_name(L"en-US");

language::language() : _id(0)
{
}

language::language(unsigned long id) : _id(id), _name(language::id_to_name(id))
{
}

language::~language()
{
}

language& language::operator = (const language& lg)
{
    if(this != &lg) {
        _id = lg.id();
        _name = lg.name();
    }
    return *this;
}

language& language::operator = (unsigned long id)
{
    _id = id;
    _name = language::id_to_name(id);
    return *this;
}

void language::clear()
{
    _id = 0;
    _name.clear();
}

std::wstring language::id_to_name(unsigned long id)
{
    wchar_t wzName[LOCALE_NAME_MAX_LENGTH+1] = {0};
    return ((0 != id) && (0 != LCIDToLocaleName(MAKELCID(id, SORT_DEFAULT), wzName, LOCALE_NAME_MAX_LENGTH, 0))) ? wzName : L"";
}



//
//  class product
//

product::product() : _arch(NX::arch_unknown)
{
}

product::product(const std::wstring& name, NX::CPUARCH arch) : _arch(arch)
{
    if(arch_unknown == arch) {
#ifdef _AMD64_
        if(!find(name, arch_amd64)) {
            find(name, arch_i386);
        }
#else
        if(!find(name, arch_i386)) {
            find(name, arch_amd64);
        }
#endif
    }
    else {
        find(name, arch);
    }
}

product::~product()
{
}

product& product::operator = (const product& p)
{
    if(this != &p) {
        _name = p.name();
        _version = p.version();
        _publisher = p.publisher();
        _guid = p.guid();
        _language = p.language();
        _install_root = p.install_root();
        _arch = p.architecture();
    }
    return *this;
}

void product::clear()
{
    _name.clear();
    _version.clear();
    _publisher.clear();
    _guid.clear();
    _language.clear();
    _install_root.clear();
    _arch = arch_unknown;
}

bool product::load(const std::wstring& guid, NX::CPUARCH arch) noexcept
{
    REGSAM  samDesired = KEY_READ;
    HKEY    hKey = NULL;
    std::wstring wsKey(L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\");


    if(arch == arch_unknown) {
#ifdef _AMD64_
        arch = arch_amd64;
#else
        arch = arch_i386;
#endif
    }

    samDesired |= ((arch_i386==arch) ? KEY_WOW64_32KEY : KEY_WOW64_64KEY);
    wsKey += guid;
    if(ERROR_SUCCESS != ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, wsKey.c_str(), 0, samDesired, &hKey)) {
        return false;
    }

    _guid = guid;
    _arch = arch;

    WCHAR   wzData[1024] = {0};
    DWORD   cbData = 2048;
    DWORD   dwType = 0;

    if(ERROR_SUCCESS == RegQueryValueExW(hKey, L"DisplayName", NULL, &dwType, (LPBYTE)wzData, &cbData) && REG_SZ==dwType && L'\0'!=wzData[0]) {
        _name = wzData;
    }

    // Get version
    memset(wzData, 0, sizeof(wzData));
    cbData = 2048;
    if(ERROR_SUCCESS == RegQueryValueExW(hKey, L"DisplayVersion", NULL, &dwType, (LPBYTE)wzData, &cbData) && REG_SZ==dwType && L'\0'!=wzData[0]) {
        _version = std::wstring(wzData);
    }

    // Get InstallLocation
    memset(wzData, 0, sizeof(wzData));
    cbData = 2048;
    if(ERROR_SUCCESS == RegQueryValueExW(hKey, L"InstallLocation", NULL, &dwType, (LPBYTE)wzData, &cbData) && REG_SZ==dwType && L'\0'!=wzData[0]) {
        // End with L'\\'?
        if(wzData[wcslen(wzData)-1] == L'\\') {
            wzData[wcslen(wzData)-1]  = L'\0';
        }
        _install_root = wzData;
    }

    // Get Publisher
    memset(wzData, 0, sizeof(wzData));
    cbData = 2048;
    if(ERROR_SUCCESS == RegQueryValueExW(hKey, L"Publisher", NULL, &dwType, (LPBYTE)wzData, &cbData) && REG_SZ==dwType && L'\0'!=wzData[0]) {
        _publisher = wzData;
    }

    // Get Publisher
    DWORD dwLangId = 0;
    cbData = sizeof(DWORD);
    if(ERROR_SUCCESS == RegQueryValueExW(hKey, L"Language", NULL, &dwType, (LPBYTE)&dwLangId, &cbData) && REG_DWORD==dwType) {
        _language = dwLangId;
    }
    
    ::RegCloseKey(hKey);
    hKey = NULL;

    return true;
}

bool product::find(const std::wstring& name, NX::CPUARCH arch) noexcept
{
    bool    bFound = FALSE;
    WCHAR   wzGuid[MAX_PATH] = {0};
    ULONG   i = 0;
    REGSAM  samDesired = KEY_READ;
    HKEY    hKey = NULL;


    samDesired |= ((arch_i386==arch) ? KEY_WOW64_32KEY : KEY_WOW64_64KEY);
    if(ERROR_SUCCESS != ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", 0, samDesired, &hKey)) {
        return false;
    }

    while(0 == RegEnumKeyW(hKey, i++, wzGuid, MAX_PATH-1)) {

        HKEY    hProgramKey = NULL;
        WCHAR   wzData[1024] = {0};
        DWORD   cbData = 2048;
        DWORD   dwType = 0;

        if(ERROR_SUCCESS != ::RegOpenKeyExW(hKey, wzGuid, 0, samDesired, &hProgramKey) || L'\0'==wzGuid[0]) {
            continue;
        }

        // Get information from subkey
        if(ERROR_SUCCESS != RegQueryValueExW(hProgramKey, L"DisplayName", NULL, &dwType, (LPBYTE)wzData, &cbData) || REG_SZ!=dwType || L'\0'==wzData[0]) {
            ::RegCloseKey(hProgramKey);
            continue;
        }

        if(0 == _wcsicmp(wzData, name.c_str())) {
            
            // Found!
            bFound = TRUE;
            _name = wzData;
            _guid = wzGuid;
            _arch = arch;

            // Get version
            memset(wzData, 0, sizeof(wzData));
            cbData = 2048;
            if(ERROR_SUCCESS == RegQueryValueExW(hProgramKey, L"DisplayVersion", NULL, &dwType, (LPBYTE)wzData, &cbData) && REG_SZ==dwType && L'\0'!=wzData[0]) {
                _version = std::wstring(wzData);
            }

            // Get InstallLocation
            memset(wzData, 0, sizeof(wzData));
            cbData = 2048;
            if(ERROR_SUCCESS == RegQueryValueExW(hProgramKey, L"InstallLocation", NULL, &dwType, (LPBYTE)wzData, &cbData) && REG_SZ==dwType && L'\0'!=wzData[0]) {
                _install_root = wzData;
            }

            // Get Publisher
            memset(wzData, 0, sizeof(wzData));
            cbData = 2048;
            if(ERROR_SUCCESS == RegQueryValueExW(hProgramKey, L"Publisher", NULL, &dwType, (LPBYTE)wzData, &cbData) && REG_SZ==dwType && L'\0'!=wzData[0]) {
                _publisher = wzData;
            }

            // Get Publisher
            DWORD dwLangId = 0;
            cbData = sizeof(DWORD);
            if(ERROR_SUCCESS == RegQueryValueExW(hProgramKey, L"Language", NULL, &dwType, (LPBYTE)&dwLangId, &cbData) && REG_DWORD==dwType) {
                _language = dwLangId;
            }
        }

        // Free key
        ::RegCloseKey(hProgramKey);
        hProgramKey = NULL;

        // Stop searching
        if(bFound) {
            break;
        }
    }

    // Free key
    ::RegCloseKey(hKey);
    hKey = NULL;
    return bFound;
}


//
//  class host
//
NX::host::host()
{
    std::vector<wchar_t> buf;
    unsigned long        size = 1024;
    buf.resize(1024, 0);
    if (GetComputerNameExW(ComputerNameDnsHostname, &buf[0], &size)) {
        _name = &buf[0];    // host name
        buf[0] = L'\0';
        size = 1024;
        if (GetComputerNameExW(ComputerNameDnsFullyQualified, &buf[0], &size)) {
            _fqdn = &buf[0];    // fully qualified domain name
        }
    }
    std::transform(_name.begin(), _name.end(), _name.begin(), tolower);
    std::transform(_fqdn.begin(), _fqdn.end(), _fqdn.begin(), tolower);
}


//
//  class NX::logged_on_users
//
class UserCache
{
public:
    UserCache()
    {
        InitializeCriticalSection(&_lock);
    }
    ~UserCache()
    {
        DeleteCriticalSection(&_lock);
    }

    std::wstring find(const std::wstring& sid)
    {
        std::wstring name;
        ::EnterCriticalSection(&_lock);
        auto it = _map.find(sid);
        if (it != _map.end())
            name = (*it).second;
        ::LeaveCriticalSection(&_lock);
        return std::move(name);
    }

    bool is_bad(const std::wstring& sid)
    {
        bool result = false;
        ::EnterCriticalSection(&_lock);
        result = (_badmap.end() != _badmap.find(sid));
        ::LeaveCriticalSection(&_lock);
        return result;
    }

    void add(const std::wstring& sid, const std::wstring& name)
    {
        ::EnterCriticalSection(&_lock);
        _map[sid] = name;
        ::LeaveCriticalSection(&_lock);
    }

    void add_bad(const std::wstring& sid)
    {
        ::EnterCriticalSection(&_lock);
        _badmap[sid] = 0;
        ::LeaveCriticalSection(&_lock);
    }

private:
    CRITICAL_SECTION _lock;
    std::map<std::wstring, std::wstring> _map;
    std::map<std::wstring, int> _badmap;
};

static UserCache gUserCache;

NX::logged_on_users::logged_on_users()
{
    static const std::wstring sid_prefix(L"S-1-5-21-");
    WCHAR wzKeyName[MAX_PATH] = { 0 };
    std::wstring key_name;
    int i = 0;

    // HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProfileList
    nudf::win::CRegKey regprofile;
    if (!regprofile.Open(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList", KEY_READ)) {
        return;
    }

    //  Initialize Map of Users who have logged on this machine
    while (ERROR_SUCCESS == RegEnumKeyW(regprofile, i++, nudf::string::tempstr<wchar_t>(key_name, MAX_PATH), MAX_PATH)) {

        if (0 == _wcsnicmp(key_name.c_str(), sid_prefix.c_str(), sid_prefix.length())) {

            std::transform(key_name.begin(), key_name.end(), key_name.begin(), toupper);
            if (!gUserCache.is_bad(key_name)) {

                std::wstring user_name = gUserCache.find(key_name);
                if (!user_name.empty()) {
                    _users.push_back(std::pair<std::wstring, std::wstring>(key_name, user_name));
                }
                else {

                    PSID sid = NULL;
                    if (!ConvertStringSidToSidW(key_name.c_str(), &sid)) {
                        gUserCache.add_bad(key_name);
                        key_name.clear();
                    }
                    else {
                        NX::user u = NX::user::get_user(sid);
                        LocalFree(sid);
                        sid = NULL;
                        if (!u.empty()) {
                            user_name = u.principle_name().empty() ? u.name() : u.principle_name();
                            _users.push_back(std::pair<std::wstring, std::wstring>(key_name, user_name));
                            gUserCache.add(key_name, user_name);
                        }
                        else {
                            gUserCache.add_bad(key_name);
                        }
                    }
                }
            }
        }
        key_name.clear();
    }
}

//
//  class NX::pe_cert
//
bool NX::pe_cert::load(const std::wstring& file) noexcept
{
    bool result = false;
    DWORD dwEncoding, dwContentType, dwFormatType;
    HCERTSTORE hStore = NULL;
    HCRYPTMSG hMsg = NULL;
    PCCERT_CONTEXT pCertContext = NULL;


    try {

        // Get message handle and store handle from the signed file.
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
            const unsigned long err = GetLastError();
            if (0x80092009 != err) {
                LOGWARN(L"pe_cert::load::CryptQueryObject failed (%08X/%d, %s)", err, err, file.c_str());
                throw std::exception("CryptQueryObject failed");
            }
            else {
                throw std::exception("CryptQueryObject object not found");
            }
        }

        DWORD dwSignerInfo = 0;
        std::vector<UCHAR> vSignerData;
        PCMSG_SIGNER_INFO pSignerInfo = NULL;
        CERT_INFO CertInfo = { 0 };

        if (!CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, 0, NULL, &dwSignerInfo)) {
            LOGDBG(L"pe_cert::load::CryptMsgGetParam (get size) failed (%d, %s)", GetLastError(), file.c_str());
            throw std::exception("CryptMsgGetParam failed");
        }

        // Allocate memory for signer information.
        vSignerData.resize(dwSignerInfo, 0);
        pSignerInfo = reinterpret_cast<PCMSG_SIGNER_INFO>(vSignerData.data());

        // Get Signer Information.
        if (!CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, 0, (PVOID)pSignerInfo, &dwSignerInfo)) {
            LOGDBG(L"pe_cert::load::CryptMsgGetParam failed (%d, %s)", GetLastError(), file.c_str());
            throw std::exception("CryptMsgGetParam failed");
        }

        memset(&CertInfo, 0, sizeof(CertInfo));
        CertInfo.Issuer = pSignerInfo->Issuer;
        CertInfo.SerialNumber = pSignerInfo->SerialNumber;
        pCertContext = CertFindCertificateInStore(hStore, dwEncoding, 0, CERT_FIND_SUBJECT_CERT, (PVOID)&CertInfo, NULL);
        if (NULL == pCertContext) {
            LOGDBG(L"pe_cert::load::CertFindCertificateInStore failed (%d, %s)", GetLastError(), file.c_str());
            throw std::exception("CertFindCertificateInStore failed");
        }

        result = true;
        nudf::crypto::CCertDecoder::DecodeSubject(pCertContext, _subject);
        nudf::crypto::CCertDecoder::DecodeIssuer(pCertContext, _issuer);
        nudf::crypto::CCertDecoder::DecodeSerial(pCertContext, _serial);
        nudf::crypto::CCertDecoder::DecodeThumbprint(pCertContext, _thumbprint);
        nudf::crypto::CCertDecoder::DecodeAlgorithm(pCertContext, _signature_alg, _thumbprint_alg);
        nudf::crypto::CCertDecoder::DecodeValidDate(pCertContext, &_validfrom, &_validthru);
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        result = false;
    }

    if (NULL != pCertContext) {
        CertFreeCertificateContext(pCertContext);
    }
    if (NULL != hMsg) {
        CryptMsgClose(hMsg);
    }
    if (NULL != hStore) {
        CertCloseStore(hStore, 0);
    }

    return result;
}

void NX::pe_cert::clear() noexcept
{
    _subject.clear();
    _issuer.clear();
    _serial.clear();
    _thumbprint.clear();
    _thumbprint_alg.clear();
    _signature_alg.clear();
    memset(&_validfrom, 0, sizeof(_validfrom));
    memset(&_validthru, 0, sizeof(_validthru));
}

NX::pe_file::pe_file()
{
}

NX::pe_file::pe_file(const std::wstring& file)
{
}

NX::pe_file::~pe_file()
{
}

void NX::pe_file::load(const std::wstring& file) noexcept
{
    HANDLE hFile;
    HANDLE hFileMapping;
    LPVOID lpFileBase = NULL;
    PIMAGE_DOS_HEADER dosHeader = NULL;
    PIMAGE_NT_HEADERS ntHeader = NULL;

    clear();

    hFile = CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE) {
        return;
    }

    hFileMapping = CreateFileMappingW(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (NULL == hFileMapping) {
        CloseHandle(hFile);
        return;
    }

    lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
    if (NULL == lpFileBase) {
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        return;
    }

    // Parse PE Header
    dosHeader = (PIMAGE_DOS_HEADER)lpFileBase;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        goto _exit;
    }

    // Get NT Header
    ntHeader = (PIMAGE_NT_HEADERS)((PUCHAR)dosHeader + dosHeader->e_lfanew);
    if (0x00004550 != ntHeader->Signature) {
        goto _exit;
    }
    memcpy(&_nt_header, ntHeader, sizeof(_nt_header));

    // Close file
    CloseHandle(hFileMapping); hFileMapping = NULL;
    CloseHandle(hFile); hFile = INVALID_HANDLE_VALUE;

    // Get Signature
    _cert.load(file);

_exit:
    if (NULL != hFileMapping) {
        CloseHandle(hFileMapping); hFileMapping = NULL;
    }
    if (INVALID_HANDLE_VALUE != hFile) {
        CloseHandle(hFile); hFile = INVALID_HANDLE_VALUE;
    }
}

void NX::pe_file::clear() noexcept
{
    _cert.clear();
    memset(&_nt_header, 0, sizeof(_nt_header));
}

pe_file& NX::pe_file::operator = (const pe_file& pe) noexcept
{
    if (this != &pe) {
        memcpy(&_nt_header, pe.nt_headers(), sizeof(_nt_header));
        _cert = pe.cert();
    }
    return *this;
}


namespace {
class publisher_cache
{
public:
    publisher_cache()
    {
        ::InitializeCriticalSection(&_lock);
    }
    ~publisher_cache()
    {
        ::DeleteCriticalSection(&_lock);
    }

private:
    class image_key
    {
    public:
        image_key() : _signature(0)
        {
        }
        image_key(const std::wstring& image) : _image(image), _signature(0)
        {
            std::transform(_image.begin(), _image.end(), _image.begin(), tolower);
            NX::pe_file image_pe;
            image_pe.load(_image);
            _signature = image_pe.nt_headers()->Signature;
        }
        image_key(const std::wstring& image, unsigned long signature) : _image(image), _signature(signature)
        {
            std::transform(_image.begin(), _image.end(), _image.begin(), tolower);
        }
        ~image_key()
        {
        }

        inline const std::wstring& image() const { return _image; }
        inline unsigned long signature() const { return _signature; }

        image_key& operator = (const image_key& other)
        {
            if (this != &other) {
                _image = other.image();
                _signature = other.signature();
            }
            return *this;
        }

        bool operator == (const image_key& other) const
        {
            if (this == &other) {
                return true;
            }
            return (signature() == other.signature() && image() == other.image());
        }

        bool operator < (const image_key& other) const
        {
            if (this == &other) {
                return false;
            }
            int n = _wcsicmp(image().c_str(), other.image().c_str());
            if (0 == n) {
                return (signature() < other.signature());
            }

            return (n < 0);
        }

        bool operator > (const image_key& other) const
        {
            if (this == &other) {
                return false;
            }
            int n = _wcsicmp(image().c_str(), other.image().c_str());
            if (0 == n) {
                return (signature() > other.signature());
            }

            return (n > 0);
        }

    private:
        std::wstring    _image;
        unsigned long   _signature;
    };

public:
    bool find(const std::wstring& image, std::wstring& publisher) const
    {
        bool result = false;

        image_key   key(image);

        ::EnterCriticalSection(&_lock);
        auto pos = _map.find(key);
        if (pos != _map.end()) {
            result = true;
            publisher = (*pos).second;
        }
        ::LeaveCriticalSection(&_lock);
        return result;
    }

    void set(const std::wstring& image, const std::wstring& publisher)
    {
        image_key   key(image);
        long count = 0;
        ::EnterCriticalSection(&_lock);
        _map[key] = publisher;
        count = (long)_map.size();
        ::LeaveCriticalSection(&_lock);
        LOGDBG(L"Cache Image Publisher (%d)\r\n    \"%s\" ==> \"%s\" ", count, publisher.c_str(), key.image().c_str());
    }

private:
    std::map<image_key, std::wstring>   _map;
    mutable CRITICAL_SECTION            _lock;
};
}

NX::process_info::process_info(unsigned long process_id) : _session_id(-1)
{
    PROCESSENTRY32W pe32;

    memset(&pe32, 0, sizeof(pe32));
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == hSnapshot) {
        return;
    }
    if (!Process32FirstW(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        hSnapshot = INVALID_HANDLE_VALUE;
        return;
    }

    do {
        if (process_id == pe32.th32ProcessID) {
            if (!ProcessIdToSessionId(process_id, &_session_id)) {
                _session_id = -1;
            }
            _image = pe32.szExeFile;
            std::transform(_image.begin(), _image.end(), _image.begin(), tolower);
            if (boost::algorithm::starts_with(_image, L"\\??\\")) {
                _image = _image.substr(4);
            }
            _publisher = get_publisher(_image);
            break;
        }
    } while (Process32NextW(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
    hSnapshot = INVALID_HANDLE_VALUE;
}

NX::process_info::process_info(unsigned long session_id, const std::wstring& image) : _session_id(session_id), _image(image)
{
    std::transform(_image.begin(), _image.end(), _image.begin(), tolower);
    if (boost::algorithm::starts_with(_image, L"\\??\\")) {
        _image = _image.substr(4);
    }
    _publisher = get_publisher(_image);
}

std::wstring NX::process_info::get_publisher(const std::wstring& image)
{
    static publisher_cache _cache;

    std::wstring publisher;

    // exist in cache
    if (_cache.find(image, publisher)) {
        return std::move(publisher);
    }

    // not exist
    pe_cert cert(image);
    publisher = cert.subject();
    _cache.set(image, publisher);
    return std::move(publisher);
}