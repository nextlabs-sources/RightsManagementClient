
#ifndef __NXRM_ENGINE_DIAGNOSE_HPP__
#define __NXRM_ENGINE_DIAGNOSE_HPP__

#include <string>
#include <vector>

#include <nudf\exception.hpp>
#include <nudf\log.hpp>
#include <nudf\bitsmap.hpp>
#include <nudf\shared\engineenv.h>

namespace nxrm {
namespace engine {
namespace diagnose {



void DumpHwInfo() throw();
void DumpOsInfo() throw();


class CHarddiskInfo
{
public:
    CHarddiskInfo();
    CHarddiskInfo(_In_ LPCWSTR name, _In_ UINT type, _In_ ULONGLONG total, _In_ ULONGLONG available_free, _In_ ULONGLONG total_free);
    ~CHarddiskInfo();
    
    CHarddiskInfo& operator = (const CHarddiskInfo& disk);

    inline const std::wstring& GetName() const throw() {return _name;}
    inline const std::wstring& GetTypeName() const throw() {return _typename;}
    inline UINT GetType() const throw() {return _type;}
    inline ULONGLONG GetTotalSpace() const throw() {return _total;}
    inline ULONGLONG GetAvailableFreeSpace() const throw() {return _available_free;}
    inline ULONGLONG GetTotalFreeSpace() const throw() {return _total_free;}

private:
    std::wstring _name;
    std::wstring _typename;
    UINT         _type;
    ULONGLONG    _total;
    ULONGLONG    _available_free;
    ULONGLONG    _total_free;
};

class CNetAdapter
{
public:
    CNetAdapter();
    CNetAdapter(_In_ PVOID address);
    ~CNetAdapter();

    CNetAdapter& operator = (const CNetAdapter& adapter);

    inline const std::wstring& GetName() const throw() {return _name;}
    inline const std::wstring& GetFriendlyName() const throw() {return _friendlyname;}
    inline const std::wstring& GetDescription() const throw() {return _description;}
    inline const std::vector<UCHAR>& GetMacAddress() const throw() {return _macaddr;}
    inline ULONG GetIfType() const throw() {return _iftype;}
    inline const std::wstring& GetIfTypeName() const throw() {return _iftypename;}
    inline ULONG GetIpv4Enabled() const throw() {return _ipv4enabled;}
    inline ULONG GetIpv6Enabled() const throw() {return _ipv6enabled;}
    inline ULONG GetOperStatus() const throw() {return _operstatus;}
    inline const std::wstring& GetOperStatusName() const throw() {return _operstatusname;}
    inline ULONGLONG GetTransmitLinkSpeed() const throw() {return _translinkspeed;}
    inline ULONGLONG GetReceiveLinkSpeed() const throw() {return _recvlinkspeed;}

private:
    std::wstring    _name;
    std::wstring    _friendlyname;
    std::wstring    _description;
    std::vector<UCHAR> _macaddr;
    ULONG           _iftype;
    std::wstring    _iftypename;
    ULONG           _ipv4enabled;
    ULONG           _ipv6enabled;
    ULONG           _operstatus;
    std::wstring    _operstatusname;
    ULONGLONG       _translinkspeed;
    ULONGLONG       _recvlinkspeed;

};

class CCpuInfo
{
    // forward declarations
    class CCpuIdData;
    class CBits;

public:
    CCpuInfo()
    {
        hyperthreads_ = false;
        cores_ = 0;
        logicals_ = 0;
        Load();
    }
    ~CCpuInfo()
    {
    }

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
        if(isIntel_) {
            cores_ = ((data_[4][0] >> 26) & 0x3f) + 1;
        }
        else if(isAMD_) {
            cores_ = ((unsigned)(extdata_[8][2] & 0xff)) + 1;
        }

        hyperthreads_ = ((cpuFeatures & (1 << 28)) && (cores_ < logicals_));

    }

public:
    // getters
    const std::wstring& Vendor(void) const throw() { return vendor_; }
    const std::wstring& Brand(void) const throw() { return brand_; }

    unsigned long GetCores() const throw() {return cores_;}
    unsigned long GetLogicalProcessors() const throw() {return logicals_;}
    bool HYPERTHREADS() const throw() {return hyperthreads_;}

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

    nudf::util::CBitsMap<32> f_1_ECX_;
    nudf::util::CBitsMap<32> f_1_EDX_;
    nudf::util::CBitsMap<32> f_7_EBX_;
    nudf::util::CBitsMap<32> f_7_ECX_;
    nudf::util::CBitsMap<32> f_81_ECX_;
    nudf::util::CBitsMap<32> f_81_EDX_;
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

        inline int* data() throw() {return _data;}
        inline int operator [](int id) const {return ((id>=0 && id<4) ? _data[id] : -1);}

        int _data[4];
    };
};

class CProcessors
{
public:
    CProcessors();
    ~CProcessors();

    CProcessors& operator = (const CProcessors& processor);
    void Load();

    inline ULONG GetPhysicalProcessor() const throw() {return _PhysicalProcessors;}
    inline ULONG GetCores() const throw() {return _Cores;}
    inline ULONG GetLogicalProcessor() const throw() {return _LogicalProcessors;}
    inline ULONG GetNuamNodeCount() const throw() {return _NuamNodeCount;}
    inline ULONG GetL1CacheCount() const throw() {return _L1CacheCount;}
    inline ULONG GetL2CacheCount() const throw() {return _L2CacheCount;}
    inline ULONG GetL3CacheCount() const throw() {return _L3CacheCount;}
    inline ULONG GetL1CacheSize() const throw() {return _L1CacheSize;}
    inline ULONG GetL2CacheSize() const throw() {return _L2CacheSize;}
    inline ULONG GetL3CacheSize() const throw() {return _L3CacheSize;}
    inline double GetSpeed() const throw() {return _Speed;}
    inline const CCpuInfo& GetCpuInfo() const throw() {return _cpuinfo;}


protected:
    ULONG CountSetBits(ULONG_PTR bitMask);
    double CalculateSpeed();

private:
    ULONG   _PhysicalProcessors;
    ULONG   _Cores;
    ULONG   _LogicalProcessors;
    ULONG   _NuamNodeCount;
    ULONG   _L1CacheCount;
    ULONG   _L1CacheSize;
    ULONG   _L2CacheCount;
    ULONG   _L2CacheSize;
    ULONG   _L3CacheCount;
    ULONG   _L3CacheSize;
    double  _Speed;
    CCpuInfo _cpuinfo;
};


class CHwInfo
{
public:
    CHwInfo();
    ~CHwInfo();

    void Load();

    inline const HW_PROFILE_INFOW* GetHwProfile() const throw() {return &_profile;}
    inline const SYSTEM_INFO* GetSysInfo() const throw() {return &_sysinfo;}
    inline const MEMORYSTATUSEX* GetMemStatus() const throw() {return &_memstatus;}
    inline const std::vector<CHarddiskInfo>& GetDisksInfo() const throw() {return _disks;}
    inline const std::vector<CNetAdapter>& GetAdaptersInfo() const throw() {return _adapters;}
    inline const CProcessors& GetProcessorsInfo() const throw() {return _processors;}

protected:
    void LoadDisks();
    void LoadNetAdapters();

private:
    HW_PROFILE_INFOW    _profile;
    SYSTEM_INFO         _sysinfo;
    MEMORYSTATUSEX      _memstatus;
    std::vector<CHarddiskInfo>  _disks;
    std::vector<CNetAdapter>  _adapters;
    CProcessors         _processors;
};

typedef enum WINOSID {
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
    WIN81
} WINOSID;

class COsInfo
{
public:
    COsInfo();
    ~COsInfo();

    void Load();

    inline const OSVERSIONINFOEX* GetOsVer() const throw() {return &_osver;}
    inline bool IsWin64() const throw() {return _iswin64;}
    inline WINOSID GetOsId() const throw() {return _osid;}
    inline const std::wstring& GetOsName() const throw() {return _osname;}

    inline LCID GetLocaleId() const throw() {return _lcid;}
    inline LANGID GetLanguageId() const throw() {return _langid;}
    inline LANGID GetUILanguageId() const throw() {return _uilangid;}
    inline const std::wstring& GetLocaleName() const throw() {return _lcname;}

protected:
    WINOSID VersionToId(const OSVERSIONINFOEXW& osver);

private:
    OSVERSIONINFOEXW    _osver;
    ULONG               _ostype;
    std::wstring        _ostypename;
    bool                _iswin64;
    WINOSID             _osid;
    std::wstring        _osname;
    std::wstring        _spname;
    LCID                _lcid;
    std::wstring        _lcname;
    LANGID              _langid;
    LANGID              _uilangid;
};


class CProgram
{
public:
    CProgram();
    CProgram(_In_ LPCWSTR guid, _In_opt_ LPCWSTR name, _In_opt_ LPCWSTR version, _In_opt_ LPCWSTR location, _In_opt_ LPCWSTR publisher, _In_opt_ LPCWSTR date, _In_opt_ ULONG vermajor, _In_opt_ ULONG verminor, _In_opt_ ULONG langid);
    ~CProgram();

    CProgram& operator = (const CProgram& prog);

    inline const std::wstring& GetGuid() const throw() {return _guid;}
    inline const std::wstring& GetName() const throw() {return _name;}
    inline const std::wstring& GetVersion() const throw() {return _version;}
    inline const std::wstring& GetPublisher() const throw() {return _publisher;}
    inline const std::wstring& GetLanguageName() const throw() {return _langname;}
    inline const std::wstring& GetInstallLocation() const throw() {return _location;}
    inline const std::wstring& GetInstallDate() const throw() {return _date;}
    inline ULONG GetVerMajor() const throw() {return _vermajor;}
    inline ULONG GetVerMinor() const throw() {return _verminor;}
    inline ULONG GetLanguageId() const throw() {return _langid;}


private:
    std::wstring    _guid;
    std::wstring    _name;
    std::wstring    _version;
    std::wstring    _location;
    std::wstring    _date;
    std::wstring    _publisher;
    std::wstring    _langname;
    ULONG           _vermajor;
    ULONG           _verminor;
    ULONG           _langid;
};

class CInstalledPrograms
{
public:
    CInstalledPrograms();
    ~CInstalledPrograms();

    void Load();
    inline const std::vector<CProgram>& GetPrograms() const throw() {return _prog;}
    inline const std::vector<CProgram>& GetProgramsWow6432() const throw() {return _prog32;}

protected:
    void LoadEx(_In_ BOOL wow64_32, _Out_ std::vector<CProgram>& progs);

private:
    std::vector<CProgram> _prog;
    std::vector<CProgram> _prog32;
    bool _is64bitos;
};

class CService
{
public:
    CService();
    CService(_In_ LPCWSTR name, _In_ LPCWSTR displayname, _In_ ULONG type, _In_ ULONG state);
    ~CService();

    CService& operator = (const CService& svc);

    inline const std::wstring& GetName() const throw() {return _name;}
    inline const std::wstring& GetImage() const throw() {return _image;}
    inline const std::wstring& GetDisplayName() const throw() {return _displayname;}
    inline const std::wstring& GetDescription() const throw() {return _description;}
    inline const std::wstring& GetGroup() const throw() {return _group;}
    inline ULONG GetType() const throw() {return _type;}
    inline ULONG GetStart() const throw() {return _start;}
    inline ULONG GetErrorControl() const throw() {return _errctl;}
    inline ULONG GetCurrentState() const throw() {return _state;}

    LPCWSTR GetTypeText() const throw();
    LPCWSTR GetStartText() const throw();
    LPCWSTR GetErrorControlText() const throw();
    LPCWSTR GetCurrentStateText() const throw();


private:
    std::wstring    _name;
    std::wstring    _image;
    std::wstring    _displayname;
    std::wstring    _description;
    std::wstring    _group;
    ULONG           _type;
    ULONG           _start;
    ULONG           _errctl;
    ULONG           _state;
};

class CServices
{
public:
    CServices();
    virtual ~CServices();

    virtual void Load() throw();

    inline const std::vector<CService>& GetServices() const throw() {return _services;}

protected:
    void LoadEx(DWORD dwServiceType, DWORD dwServiceState) throw();

private:
    std::vector<CService> _services;
};

}   // namespace nxrm::engine::diagnose
}   // namespace nxrm::engine
}   // namespace nxrm


#endif  // #ifndef __NXRM_ENGINE_DIAGNOSE_HPP__