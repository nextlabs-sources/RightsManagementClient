
#include <winsock2.h>
#include <iphlpapi.h>
#include <Windows.h>
#include <assert.h>
#include <Wtsapi32.h>
#include <Sddl.h>

#include <nudf\exception.hpp>
#include <nudf\string.hpp>
#include <nudf\bitsmap.hpp>
#include <nudf\registry.hpp>

#include "engine.hpp"
#include "diagnose.hpp"


using namespace nxrm::engine::diagnose;

const WCHAR* WINOSID_NAMES[] = {
    //WINUNKNOWN = 0,
    L"Unknown Windows Version",
    //WIN2000,
    L"Windows 2000",
    //WINXP,
    L"Windows XP",
    //WINXP64PRO,
    L"Windows XP Professional x64 Edition",
    //WIN2K3,
    L"Windows Server 2003",
    //WINHOMESERVER,
    L"Windows Home Server",
    //WIN2K3R2,
    L"Windows Server 2003 R2",
    //WINVISTA,
    L"Windows Vista",
    //WIN2K8,
    L"Windows Server 2008",
    //WIN2K8R2,
    L"Windows Server 2008 R2",
    //WIN7,
    L"Windows 7",
    //WINSRV2K12,
    L"Windows Server 2012",
    //WIN8,
    L"Windows 8",
    //WINSRV2K12R2,
    L"Windows Server 2012 R2",
    //WIN81
    L"Windows 8.1"
};

//
// We need to get following information
//  - Hardware
//      * CPU
//      * Memory
//      * Harddisk
//      * Network Adapters - Name, MAC Address, etc.
//  - Operating System
//      * Version
//      * Language
//      * Region
//      * Service pack
//      * Windows Update
//         -> Name
//         -> Publisher
//      * Host Name
//  - Services
//      * Service Name
//      * Service Type
//      * Service Image
//      * Service Start Type
//  - Software
//      * Name
//      * Publisher
//      * Version
//      * Install Time
//  - NextLabs Software
//      * Name
//      * Publisher
//      * Version
//      * Install Time
//  - NextLabs Rights Management
//      * Name
//      * Publisher
//      * Version
//      * Install Time
//      * File Existence
//

LPCWSTR CpuArchToName(WORD wArch)
{
    switch(wArch)
    {
    case PROCESSOR_ARCHITECTURE_AMD64: return L"x64";
    case PROCESSOR_ARCHITECTURE_ARM: return L"ARM";
    case PROCESSOR_ARCHITECTURE_IA64: return L"IA64";
    case PROCESSOR_ARCHITECTURE_INTEL: return L"x86";
    case PROCESSOR_ARCHITECTURE_UNKNOWN:
    default:
        break;
    }
    return L"Unknown";
}

void nxrm::engine::diagnose::DumpHwInfo() throw()
{
    CHwInfo hwinfo;
    const ULONG OneKB = 1024;
    const ULONG OneMB = 1048576; //1024*1024;

    LOGSYSINF(L" ");
    LOGSYSINF(L"  =============================");
    LOGSYSINF(L"    Hardware Information");
    LOGSYSINF(L"  =============================");

    // CPU
    LOGSYSINF(L"  [CPU]");
    LOGSYSINF(L"    Vender: %s", hwinfo.GetProcessorsInfo().GetCpuInfo().Vendor().c_str());
    if(hwinfo.GetProcessorsInfo().GetSpeed() < 1000.0) {
        LOGSYSINF(L"    Brand: %s (%s), %d MHz", hwinfo.GetProcessorsInfo().GetCpuInfo().Brand().c_str(), CpuArchToName(hwinfo.GetSysInfo()->wProcessorArchitecture), (int)hwinfo.GetProcessorsInfo().GetSpeed());
    }
    else {
        LOGSYSINF(L"    Brand: %s (%s), %.3f GHz", hwinfo.GetProcessorsInfo().GetCpuInfo().Brand().c_str(), CpuArchToName(hwinfo.GetSysInfo()->wProcessorArchitecture), hwinfo.GetProcessorsInfo().GetSpeed()/1000.0);
    }
    LOGSYSINF(L"    Physical Processors: %d", hwinfo.GetProcessorsInfo().GetPhysicalProcessor());
    LOGSYSINF(L"    Processor Cores: %d", hwinfo.GetProcessorsInfo().GetCores());
    LOGSYSINF(L"    Logical Processors: %d", hwinfo.GetProcessorsInfo().GetLogicalProcessor());
    LOGSYSINF(L"    CpuId.Cores: %d", hwinfo.GetProcessorsInfo().GetCpuInfo().GetCores());
    LOGSYSINF(L"    CpuId.LogicalProcessors: %d", hwinfo.GetProcessorsInfo().GetCpuInfo().GetLogicalProcessors());
    LOGSYSINF(L"    CpuId.HyperThreads: %s", hwinfo.GetProcessorsInfo().GetCpuInfo().HYPERTHREADS() ? L"yes" : L"no");
    LOGSYSINF(L"    NUAM Nodes: %d", hwinfo.GetProcessorsInfo().GetNuamNodeCount());
    LOGSYSINF(L"    L1 Cache (Count/Size): %d / %d", hwinfo.GetProcessorsInfo().GetL1CacheCount(), hwinfo.GetProcessorsInfo().GetL1CacheSize());
    LOGSYSINF(L"    L2 Cache (Count/Size): %d / %d", hwinfo.GetProcessorsInfo().GetL2CacheCount(), hwinfo.GetProcessorsInfo().GetL2CacheSize());
    LOGSYSINF(L"    L3 Cache (Count/Size): %d / %d", hwinfo.GetProcessorsInfo().GetL3CacheCount(), hwinfo.GetProcessorsInfo().GetL3CacheSize());

    // Memory
    LOGSYSINF(L"  [Memory]");
    LOGSYSINF(L"    There are %ld percent of memory in use.", hwinfo.GetMemStatus()->dwMemoryLoad);
    LOGSYSINF(L"    Total: %I64d MB", hwinfo.GetMemStatus()->ullTotalPhys/OneMB);
    LOGSYSINF(L"    Free: %I64d MB", hwinfo.GetMemStatus()->ullAvailPhys/OneMB);
    LOGSYSINF(L"    Total VM: %I64d MB", hwinfo.GetMemStatus()->ullTotalVirtual/OneMB);
    LOGSYSINF(L"    Free VM: %I64d MB", hwinfo.GetMemStatus()->ullAvailVirtual/OneMB);

    // Harddisk
    LOGSYSINF(L"  [Disks]");
    for(std::vector<CHarddiskInfo>::const_iterator it=hwinfo.GetDisksInfo().begin(); it!=hwinfo.GetDisksInfo().end(); ++it) {
        LOGSYSINF(L"    %s (%s)", (*it).GetName().c_str(), (*it).GetTypeName().c_str());
        LOGSYSINF(L"        * Total Space: %I64d Bytes", (*it).GetTotalSpace());
        LOGSYSINF(L"        * Total Free Space: %I64d Bytes", (*it).GetTotalFreeSpace());
        LOGSYSINF(L"        * Total Available Space: %I64d Bytes", (*it).GetAvailableFreeSpace());
    }

    // Network Adapter
    LOGSYSINF(L"  [Network Adapters]");
    for(std::vector<CNetAdapter>::const_iterator it=hwinfo.GetAdaptersInfo().begin(); it!=hwinfo.GetAdaptersInfo().end(); ++it) {
        std::wstring wsMac;
        if(!(*it).GetMacAddress().empty()) {
            wsMac = nudf::string::FromBytes<wchar_t>(&((*it).GetMacAddress()[0]), (ULONG)(*it).GetMacAddress().size());
        }
        LOGSYSINF(L"    %s", (*it).GetName().c_str());
        LOGSYSINF(L"        * MAC Address: %s", wsMac.c_str());
        LOGSYSINF(L"        * Friendly Name: %s", (*it).GetFriendlyName().c_str());
        LOGSYSINF(L"        * Description: %s", (*it).GetDescription().c_str());
        LOGSYSINF(L"        * IfType: %s", (*it).GetIfTypeName().c_str());
        LOGSYSINF(L"        * OperStatus: %s", (*it).GetOperStatusName().c_str());
        LOGSYSINF(L"        * Ipv4 Enabled: %s", (*it).GetIpv4Enabled() ? L"yes":L"no");
        LOGSYSINF(L"        * Ipv6 Enabled: %s", (*it).GetIpv6Enabled() ? L"yes":L"no");
        LOGSYSINF(L"        * Speed (Transmit/Receive): %I64d / %I64d", (*it).GetTransmitLinkSpeed(), (*it).GetReceiveLinkSpeed());
    }
}

void nxrm::engine::diagnose::DumpOsInfo() throw()
{
    COsInfo osinfo;

    LOGSYSINF(L" ");
    LOGSYSINF(L"  =============================");
    LOGSYSINF(L"    Software Information");
    LOGSYSINF(L"  =============================");

    // CPU
    LOGSYSINF(L"  [OS]");
    LOGSYSINF(L"    Windows Edition:");
    LOGSYSINF(L"        %s", osinfo.GetOsName().c_str());
    if(L'\0' != osinfo.GetOsVer()->szCSDVersion[0]) {
        LOGSYSINF(L"        %s", osinfo.GetOsVer()->szCSDVersion);
    }
    LOGSYSINF(L"    System Type: %s-bit Operating System", osinfo.IsWin64() ? L"64" : L"32");
    LOGSYSINF(L"    System Locale: %s (Id: %08X)", osinfo.GetLocaleName().c_str(), osinfo.GetLocaleId());
    LOGSYSINF(L"    System Language Id: %04X", osinfo.GetLanguageId());
    LOGSYSINF(L"    System UI Language Id: %04X", osinfo.GetUILanguageId());


    // Software
    CInstalledPrograms progs;
    LOGSYSINF(L"  [Installed Software]");
    if(osinfo.IsWin64()) {
        LOGSYSINF(L"      <64-bit Programs: %I64d>", progs.GetPrograms().size());
        for(std::vector<CProgram>::const_iterator it=progs.GetPrograms().begin(); it!=progs.GetPrograms().end(); ++it) {
            LOGSYSINF(L"          %s", (*it).GetName().c_str());
            LOGDBG   (L"             - guid: %s", (*it).GetGuid().c_str());
            LOGSYSINF(L"             - version: %s", (*it).GetVersion().c_str());
            LOGDBG   (L"             - publisher: %s", (*it).GetPublisher().c_str());
            LOGSYSINF(L"             - language: %s (%d)", (*it).GetLanguageName().c_str(), (*it).GetLanguageId());
            LOGDBG   (L"             - installed-location: %s", (*it).GetInstallLocation().c_str());
        }
        LOGSYSINF(L"      <32-bit Programs: %I64d>", progs.GetProgramsWow6432().size());
        for(std::vector<CProgram>::const_iterator it=progs.GetProgramsWow6432().begin(); it!=progs.GetProgramsWow6432().end(); ++it) {
            LOGSYSINF(L"          %s", (*it).GetName().c_str());
            LOGDBG   (L"             - guid: %s", (*it).GetGuid().c_str());
            LOGSYSINF(L"             - version: %s", (*it).GetVersion().c_str());
            LOGDBG   (L"             - publisher: %s", (*it).GetPublisher().c_str());
            LOGSYSINF(L"             - language: %s (%d)", (*it).GetLanguageName().c_str(), (*it).GetLanguageId());
            LOGDBG   (L"             - installed-location: %s", (*it).GetInstallLocation().c_str());
        }
    }
    else {
        LOGSYSINF(L"    <Total  Programs: %I64d>", progs.GetPrograms().size());
        for(std::vector<CProgram>::const_iterator it=progs.GetPrograms().begin(); it!=progs.GetPrograms().end(); ++it) {
            LOGSYSINF(L"      %s", (*it).GetName().c_str());
            LOGDBG   (L"         - guid: %s", (*it).GetGuid().c_str());
            LOGSYSINF(L"         - version: %s", (*it).GetVersion().c_str());
            LOGDBG   (L"         - publisher: %s", (*it).GetPublisher().c_str());
            LOGSYSINF(L"         - language: %s (%d)", (*it).GetLanguageName().c_str(), (*it).GetLanguageId());
            LOGDBG   (L"         - installed-location: %s", (*it).GetInstallLocation().c_str());
        }
    }

    if(!_LogServ.AcceptLevel(LOGDEBUG)) {
        LOGSYSINF(L" ");
        LOGSYSINF(L" ");
        return;
    }

    // Services
    CServices services;
    LOGDBG(L"  [Services]");
    LOGDBG(L"    <Total Services: %I64d>", services.GetServices().size());
    for(std::vector<CService>::const_iterator it=services.GetServices().begin(); it!=services.GetServices().end(); ++it) {
        LOGDBG(L"      %s", (*it).GetName().c_str());
        LOGDBG(L"         - DisplayName: %s", (*it).GetDisplayName().c_str());
        LOGDBG(L"         - ImagePath: %s", (*it).GetImage().c_str());
        LOGDBG(L"         - Type: %s", (*it).GetTypeText());
        LOGDBG(L"         - Start: %s", (*it).GetStartText());
        LOGDBG(L"         - CurrentState: %s", (*it).GetCurrentStateText());
        LOGDBG(L"         - ErrorControl: %s", (*it).GetErrorControlText());
    }

    // Last
    LOGSYSINF(L" ");
    LOGSYSINF(L" ");
}



//
//  class CHwInfo
//


CHwInfo::CHwInfo()
{
    Load();
}

CHwInfo::~CHwInfo()
{
}

void CHwInfo::Load()
{
    memset(&_sysinfo, 0, sizeof(_sysinfo));
    GetSystemInfo(&_sysinfo);
    memset(&_memstatus, 0, sizeof(_memstatus));
    _memstatus.dwLength = sizeof(_memstatus);
    GlobalMemoryStatusEx(&_memstatus);
    LoadDisks();
    LoadNetAdapters();

}

void CHwInfo::LoadDisks()
{
    WCHAR wzDrive[4] = {L'C', L':', L'\\', 0};
    for(WCHAR i=L'C'; i<=L'Z'; i++) {

        UINT uType = 0;
        wzDrive[0] = i;
        uType = GetDriveTypeW(wzDrive);

        if(DRIVE_FIXED == uType || DRIVE_REMOTE == uType || DRIVE_REMOVABLE == uType || DRIVE_CDROM == uType) {
            
            ULARGE_INTEGER liFreeBytesAvailable = {0, 0};
            ULARGE_INTEGER liTotalNumberOfBytes = {0, 0};
            ULARGE_INTEGER liTotalNumberOfFreeBytes = {0, 0};

            GetDiskFreeSpaceExW(wzDrive, &liFreeBytesAvailable, &liTotalNumberOfBytes, &liTotalNumberOfFreeBytes);
            _disks.push_back(CHarddiskInfo(wzDrive, uType, liTotalNumberOfBytes.QuadPart, liFreeBytesAvailable.QuadPart, liTotalNumberOfFreeBytes.QuadPart));
        }
    }
}

void CHwInfo::LoadNetAdapters()
{
    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    PIP_ADAPTER_ADDRESSES pCurAddress = NULL;
    ULONG dwSize = sizeof(IP_ADAPTER_ADDRESSES);
    ULONG dwRetVal = 0;
    

    pAddresses = (PIP_ADAPTER_ADDRESSES)malloc(dwSize);
    if(NULL == pAddresses) {
        goto _exit;
    }
    memset(pAddresses, 0, dwSize);

    dwRetVal = GetAdaptersAddresses(AF_UNSPEC, 0, NULL, pAddresses, &dwSize);
    if(ERROR_SUCCESS != dwRetVal) {
        if(ERROR_BUFFER_OVERFLOW != dwRetVal) {
            goto _exit;
        }
        free(pAddresses); pAddresses = NULL;

        dwSize += sizeof(IP_ADAPTER_ADDRESSES);
        pAddresses = (PIP_ADAPTER_ADDRESSES)malloc(dwSize);
        if(NULL == pAddresses) {
            goto _exit;
        }
        memset(pAddresses, 0, dwSize);
    }

    dwRetVal = GetAdaptersAddresses(AF_UNSPEC, 0, NULL, pAddresses, &dwSize);
    if(ERROR_SUCCESS != dwRetVal) {
        goto _exit;
    }

    pCurAddress = pAddresses;
    while(NULL != pCurAddress) {
        _adapters.push_back(CNetAdapter(pCurAddress));
        pCurAddress = pCurAddress->Next;
    }

_exit:
    if(NULL != pAddresses) {
        free(pAddresses);
        pAddresses = NULL;
    }
}


//
//  class COsInfo
//
COsInfo::COsInfo() : _osid(WINUNKNOWN)
{
#ifdef _WIN64
    _iswin64 = true;
#else
    BOOL IsWow64 = FALSE;
    _iswin64 = (IsWow64Process(GetCurrentProcess(), &IsWow64) && IsWow64);
#endif

   Load();
}

COsInfo::~COsInfo()
{
}

void COsInfo::Load()
{
    memset(&_osver, 0, sizeof(_osver));
    _osver.dwOSVersionInfoSize  = sizeof(OSVERSIONINFOEXW);
#pragma warning(push)
#pragma warning(disable: 4996)
    GetVersionExW((LPOSVERSIONINFO)&_osver);
#pragma warning(pop)
    _osid = VersionToId(_osver);
    _osname = WINOSID_NAMES[_osid];
    
    GetProductInfo(_osver.dwMajorVersion, _osver.dwMinorVersion, _osver.wServicePackMajor, _osver.wServicePackMinor, &_ostype);
    switch(_ostype)
    {
    case PRODUCT_ULTIMATE:
        _ostypename = L"Ultimate Edition";
        break;
    case PRODUCT_PROFESSIONAL:
        _ostypename = L"Professional";
        break;
    case PRODUCT_HOME_PREMIUM:
        _ostypename = L"Home Premium Edition";
        break;
    case PRODUCT_HOME_BASIC:
        _ostypename = L"Home Basic Edition";
        break;
    case PRODUCT_ENTERPRISE:
        _ostypename = L"Enterprise Edition";
        break;
    case PRODUCT_BUSINESS:
        _ostypename = L"Business Edition";
        break;
    case PRODUCT_STARTER:
        _ostypename = L"Starter Edition";
        break;
    case PRODUCT_CLUSTER_SERVER:
        _ostypename = L"Cluster Server Edition";
        break;
    case PRODUCT_DATACENTER_SERVER:
        _ostypename = L"Datacenter Edition";
        break;
    case PRODUCT_DATACENTER_SERVER_CORE:
        _ostypename = L"Datacenter Edition (core installation)";
        break;
    case PRODUCT_ENTERPRISE_SERVER:
        _ostypename = L"Enterprise Edition";
        break;
    case PRODUCT_ENTERPRISE_SERVER_CORE:
        _ostypename = L"Enterprise Edition (core installation)";
        break;
    case PRODUCT_ENTERPRISE_SERVER_IA64:
        _ostypename = L"Enterprise Edition for Itanium-based Systems";
        break;
    case PRODUCT_SMALLBUSINESS_SERVER:
        _ostypename = L"Small Business Server";
        break;
    case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
        _ostypename = L"Small Business Server Premium Edition";
        break;
    case PRODUCT_STANDARD_SERVER:
        _ostypename = L"Standard Edition";
        break;
    case PRODUCT_STANDARD_SERVER_CORE:
        _ostypename = L"Standard Edition (core installation)";
        break;
    case PRODUCT_WEB_SERVER:
        _ostypename = L"Web Server Edition";
        break;
    default:
        break;
    }

    if(WINUNKNOWN != _osid) {
        if(!_ostypename.empty()) {
            _osname += L" ";
            _osname += _ostypename;
        }
        _osname += L" (Build ";
        _osname += nudf::string::FromInt<wchar_t>(_osver.dwBuildNumber);
        _osname += L")";
    }

    _lcid = GetSystemDefaultLCID();
    _langid = GetSystemDefaultLangID();
    _uilangid = GetSystemDefaultUILanguage();
    WCHAR wzLcName[128] = {0};
    GetSystemDefaultLocaleName(wzLcName, 128);
    _lcname = wzLcName;
}

WINOSID COsInfo::VersionToId(const OSVERSIONINFOEXW& osver)
{
    if(5 == osver.dwMajorVersion) {

        if(0 == osver.dwMinorVersion) {
            return WIN2000;
        }
        else if (1 == osver.dwMinorVersion){
            return WINXP;
        }
        else if (2 == osver.dwMinorVersion){
            SYSTEM_INFO sysinfo;
            GetSystemInfo(&sysinfo);
            if(osver.wProductType==VER_NT_WORKSTATION && sysinfo.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64) {
                return WINXP64PRO;
            }
            else if(osver.wSuiteMask & VER_SUITE_WH_SERVER) {
                return WINHOMESRV;
            }
            else {
                return (0 == GetSystemMetrics(SM_SERVERR2)) ? WIN2K3 : WIN2K3R2;
            }
        }
        else {
            return WINUNKNOWN;
        }
    }
    else if(6 == osver.dwMajorVersion) {

        if(0 == osver.dwMinorVersion) {
            return (osver.wProductType == VER_NT_WORKSTATION) ? WINVISTA : WIN2K8;
        }
        else if (1 == osver.dwMinorVersion){
            return (osver.wProductType == VER_NT_WORKSTATION) ? WIN7 : WIN2K8R2;
        }
        else if (2 == osver.dwMinorVersion){
            return (osver.wProductType == VER_NT_WORKSTATION) ? WIN8 : WIN2K12;
        }
        else if (3 == osver.dwMinorVersion){
            return (osver.wProductType == VER_NT_WORKSTATION) ? WIN81 : WIN2K12R2;
        }
        else {
            return WINUNKNOWN;
        }
    }
    else {
        return WINUNKNOWN;
    }
}


//
//  class CHarddiskInfo
//

CHarddiskInfo::CHarddiskInfo() : _type(0),
    _total(0),
    _available_free(0),
    _total_free(0)
{
}

CHarddiskInfo::CHarddiskInfo(_In_ LPCWSTR name, _In_ UINT type, _In_ ULONGLONG total, _In_ ULONGLONG available_free, _In_ ULONGLONG total_free) : _name(name),
    _type(type),
    _total(total),
    _available_free(available_free),
    _total_free(total_free)
{
    switch(type)
    {
    case DRIVE_REMOVABLE: _typename = L"Removable Drive"; break;
    case DRIVE_FIXED: _typename = L"Fixed Harddisk"; break;
    case DRIVE_REMOTE: _typename = L"Remote Drive"; break;
    case DRIVE_CDROM: _typename = L"CD/DVD"; break;
    case DRIVE_RAMDISK: _typename = L"RAM Disk"; break;
    default:  _typename = L"Unknown Type"; break;
    }
}

CHarddiskInfo::~CHarddiskInfo()
{
}

CHarddiskInfo& CHarddiskInfo::operator = (const CHarddiskInfo& disk)
{
    if(this != &disk) {
        _name = disk.GetName();
        _typename = disk.GetTypeName();
        _type = disk.GetType();
        _total = disk.GetTotalSpace();
        _available_free = disk.GetAvailableFreeSpace();
        _total_free = disk.GetTotalFreeSpace();
    }
    return *this;
}


//
//  class CHarddiskInfo
//

CNetAdapter::CNetAdapter()
{
}

CNetAdapter::CNetAdapter(_In_ PVOID address)
{    
    PIP_ADAPTER_ADDRESSES pAdapter = (PIP_ADAPTER_ADDRESSES)address;
    std::string s(pAdapter->AdapterName);
    _name = std::wstring(s.begin(), s.end());
    if(NULL != pAdapter->FriendlyName) {
        _friendlyname = pAdapter->FriendlyName;
    }
    if(NULL != pAdapter->FriendlyName) {
        _description = pAdapter->Description;
    }
    if(NULL != pAdapter->PhysicalAddress && 0 != pAdapter->PhysicalAddressLength) {
        _macaddr.resize(pAdapter->PhysicalAddressLength, 0);
        memcpy(&_macaddr[0], pAdapter->PhysicalAddress, pAdapter->PhysicalAddressLength);
    }
    _translinkspeed = pAdapter->TransmitLinkSpeed;
    _recvlinkspeed = pAdapter->ReceiveLinkSpeed;
    _iftype = pAdapter->IfType;
    switch(_iftype)
    {
    case IF_TYPE_OTHER: _iftypename = L"Others"; break;
    case IF_TYPE_ETHERNET_CSMACD: _iftypename = L"Ethernet network interface"; break;
    case IF_TYPE_ISO88025_TOKENRING: _iftypename = L"Token ring network interface"; break;
    case IF_TYPE_PPP: _iftypename = L"PPP network interface"; break;
    case IF_TYPE_SOFTWARE_LOOPBACK: _iftypename = L"Software loopback network interface"; break;
    case IF_TYPE_ATM: _iftypename = L"ATM network interface"; break;
    case IF_TYPE_IEEE80211: _iftypename = L"IEEE 802.11 wireless network interface"; break;
    case IF_TYPE_TUNNEL: _iftypename = L"Tunnel type encapsulation network interface"; break;
    case IF_TYPE_IEEE1394: _iftypename = L"IEEE 1394 (Firewire) high performance serial bus network interface"; break;
    default: _iftypename = L"Unknown"; break;
    }
    _ipv4enabled = pAdapter->Ipv4Enabled;
    _ipv6enabled = pAdapter->Ipv6Enabled;
    _operstatus = pAdapter->OperStatus;
    switch(_operstatus)
    {
    case IfOperStatusUp: _operstatusname = L"Up"; break;
    case IfOperStatusDown: _operstatusname = L"Down"; break;
    case IfOperStatusTesting: _operstatusname = L"Testing Mode"; break;
    case IfOperStatusDormant: _operstatusname = L"Pending"; break;
    case IfOperStatusNotPresent: _operstatusname = L"Down (Component not present)"; break;
    case IfOperStatusLowerLayerDown: _operstatusname = L"Down (Lower layer interface is down)"; break;
    case IfOperStatusUnknown:
    default: _operstatusname = L"Unknown"; break;
    }
}

CNetAdapter::~CNetAdapter()
{
}

CNetAdapter& CNetAdapter::operator = (const CNetAdapter& adapter)
{
    if(this != &adapter) {
        _name = adapter.GetName();
        _friendlyname = adapter.GetFriendlyName();
        _description = adapter.GetDescription();
        _macaddr = adapter.GetMacAddress();
        _iftype = adapter.GetIfType();
        _iftypename = adapter.GetIfTypeName();
        _ipv4enabled = adapter.GetIpv4Enabled();
        _ipv6enabled = adapter.GetIpv6Enabled();
        _operstatus = adapter.GetOperStatus();
        _operstatusname = adapter.GetOperStatusName();
        _translinkspeed = adapter.GetTransmitLinkSpeed();
        _recvlinkspeed = adapter.GetReceiveLinkSpeed();
    }
    return *this;
}


//
//  class CProcessor
//
CProcessors::CProcessors() : _PhysicalProcessors(0),
    _Cores(0),
    _LogicalProcessors(0),
    _NuamNodeCount(0),
    _L1CacheCount(0),
    _L2CacheCount(0),
    _L3CacheCount(0),
    _L1CacheSize(0),
    _L2CacheSize(0),
    _L3CacheSize(0),
    _Speed(0.0)
{
    Load();
}

CProcessors::~CProcessors()
{
}

ULONG CProcessors::CountSetBits(ULONG_PTR bitMask)
{
    DWORD LSHIFT = sizeof(ULONG_PTR)*8 - 1;
    DWORD bitSetCount = 0;
    ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;    
    DWORD i;
    
    for (i = 0; i <= LSHIFT; ++i) {
        bitSetCount += ((bitMask & bitTest)?1:0);
        bitTest/=2;
    }

    return bitSetCount;
}

double CProcessors::CalculateSpeed()
{
    LARGE_INTEGER qwWait, qwStart, qwCurrent;
    unsigned __int64 Start = 0;

    QueryPerformanceCounter(&qwStart);
    QueryPerformanceFrequency(&qwWait);
    qwWait.QuadPart >>= 5;
    Start = __rdtsc();
    do {
        QueryPerformanceCounter(&qwCurrent);
    } while((qwCurrent.QuadPart - qwStart.QuadPart) < qwWait.QuadPart);
    
    return ((__rdtsc() - Start) << 5) / 1000000.0;
}

void CProcessors::Load()
{
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION processors = NULL;
    ULONG size = 0;
    ULONG count = 0;

    _Speed = CalculateSpeed();
    
    size = sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
    processors = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(size);
    if(NULL == processors) {
        return;
    }
    memset(processors, 0, size);
    
    if(!GetLogicalProcessorInformation(processors, &size)) {

        if(ERROR_INSUFFICIENT_BUFFER != GetLastError()) {
            free(processors); processors = NULL;
            return;
        }

        free(processors); processors = NULL;
        size += (sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) - 1);
        processors = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(size);
        if(NULL == processors) {
            return;
        }
        memset(processors, 0, size);

        if(!GetLogicalProcessorInformation(processors, &size)) {
            free(processors); processors = NULL;
            return;
        }
    }

    count = size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
    for(ULONG i=0; i<count; i++) {
        switch (processors[i].Relationship) 
        {
        case RelationNumaNode:
            // Non-NUMA systems report a single record of this type.
            _NuamNodeCount++;
            break;

        case RelationProcessorCore:
            _Cores++;

            // A hyperthreaded core supplies more than one logical processor.
            _LogicalProcessors += CountSetBits(processors[i].ProcessorMask);
            break;

        case RelationCache:
            // Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache. 
            if (processors[i].Cache.Level == 1) {
                _L1CacheCount++;
                _L1CacheSize += processors[i].Cache.Size;
            }
            else if (processors[i].Cache.Level == 2) {
                _L2CacheCount++;
                _L2CacheSize += processors[i].Cache.Size;
            }
            else if (processors[i].Cache.Level == 3) {
                _L3CacheCount++;
                _L3CacheSize += processors[i].Cache.Size;
            }
            break;

        case RelationProcessorPackage:
            // Logical processors share a physical package.
            _PhysicalProcessors++;
            break;

        default:
            break;
        }
    }
}

CProcessors& CProcessors::operator = (const CProcessors& processors)
{
    if(this != &processors) {        
        _PhysicalProcessors = processors.GetPhysicalProcessor();
        _Cores = processors.GetCores();
        _LogicalProcessors = processors.GetLogicalProcessor();
        _NuamNodeCount = processors.GetNuamNodeCount();
        _L1CacheCount = processors.GetL1CacheCount();
        _L2CacheCount = processors.GetL2CacheCount();
        _L3CacheCount = processors.GetL3CacheCount();
        _L1CacheSize = processors.GetL1CacheSize();
        _L2CacheSize = processors.GetL2CacheSize();
        _L3CacheSize = processors.GetL3CacheSize();
        _Speed = processors.GetSpeed();
    }
    return *this;
}


CProgram::CProgram()
{
}

CProgram::CProgram(_In_ LPCWSTR guid, _In_opt_ LPCWSTR name, _In_opt_ LPCWSTR version, _In_opt_ LPCWSTR location, _In_opt_ LPCWSTR publisher, _In_opt_ LPCWSTR date, _In_opt_ ULONG vermajor, _In_opt_ ULONG verminor, _In_opt_ ULONG langid)
{
    _guid = guid;
    _name = name?name:L"";
    _version = version?version:L"";
    _location = location?location:L"";
    _publisher = publisher?publisher:L"";
    _date = date?date:L"";
    _vermajor = vermajor;
    _verminor = verminor;
    _langid = langid;
    WCHAR wzName[LOCALE_NAME_MAX_LENGTH+1] = {0};
    if(0 != LCIDToLocaleName(MAKELCID(langid, SORT_DEFAULT), wzName, LOCALE_NAME_MAX_LENGTH, 0)) {
        _langname = wzName;
    }
}

CProgram::~CProgram()
{
}

CProgram& CProgram::operator = (const CProgram& prog)
{
    if(this != &prog) {
        _name = prog.GetName();
        _version = prog.GetVersion();
        _location = prog.GetInstallLocation();
        _date = prog.GetInstallDate();
        _langname = prog.GetLanguageName();
        _vermajor = prog.GetVerMajor();
        _verminor = prog.GetVerMinor();
        _langid = prog.GetLanguageId();
    }
    return *this;
}

    

CInstalledPrograms::CInstalledPrograms()
{

#ifdef _WIN64
    _is64bitos = true;
#else
    BOOL bResult = FALSE;
    if(IsWow64Process(GetCurrentProcess(), &bResult)) {
        _is64bitos = bResult ? true : false;
    }
#endif

    Load();
}

CInstalledPrograms::~CInstalledPrograms()
{
}

void CInstalledPrograms::Load()
{
    // HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall
    LoadEx(false, _prog);

    if(_is64bitos) {
        LoadEx(true, _prog32);
    }
}

void CInstalledPrograms::LoadEx(_In_ BOOL wow64_32, _Out_ std::vector<CProgram>& progs)
{
    WCHAR wzSubKeyName[MAX_PATH] = {0};
    ULONG i = 0;
    REGSAM samDesired = KEY_READ;

    nudf::win::CRegKey regkey;
    nudf::win::CRegLocalMachine reglm;

    samDesired |= (wow64_32 ? KEY_WOW64_32KEY : KEY_WOW64_64KEY);
    if(!regkey.Open(reglm, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", samDesired)) {
        return;
    }

    while(0 == RegEnumKeyW(regkey, i++, wzSubKeyName, MAX_PATH-1)) {

        nudf::win::CRegKey regsubkey;
        std::wstring name;
        std::wstring version;
        std::wstring location;
        std::wstring date;
        std::wstring publisher;
        DWORD vermajor = 0;
        DWORD verminor = 0;
        DWORD langid = 0;

        if(!regsubkey.Open(regkey, wzSubKeyName, samDesired)) {
            continue;
        }

        // Get following information from subkey
        regsubkey.GetValue(L"DisplayName", name, NULL);
        if(name.empty()) {
            continue;
        }
        regsubkey.GetValue(L"DisplayVersion", version, NULL);
        regsubkey.GetValue(L"InstallLocation", location, NULL);
        regsubkey.GetValue(L"InstallDate", date, NULL);
        regsubkey.GetValue(L"Publisher", publisher, NULL);
        regsubkey.GetValue(L"Language", &langid);
        regsubkey.GetValue(L"VersionMajor", &vermajor);
        regsubkey.GetValue(L"VersionMinor", &verminor);

        progs.push_back(CProgram(wzSubKeyName, name.c_str(), version.c_str(), location.c_str(), publisher.c_str(), date.c_str(), vermajor, verminor, langid));
    }
}

//
//  class CService
//
CService::CService() : _type(0), _start(0), _errctl(0), _state(0)
{
}

CService::CService(_In_ LPCWSTR name, _In_ LPCWSTR displayname, _In_ ULONG type, _In_ ULONG state)
{
    _name = name;
    _displayname = displayname;
    _type = type;
    _state = state;

    std::wstring wsKey(L"SYSTEM\\CurrentControlSet\\services\\");
    nudf::win::CRegKey svckey;
    nudf::win::CRegLocalMachine lmkey;

    wsKey += name;
    if(svckey.Open(lmkey, wsKey.c_str(), KEY_READ)) {
        svckey.GetValue(L"ImagePath", _image, NULL);
        svckey.GetValue(L"Description", _description, NULL);
        svckey.GetValue(L"Group", _group, NULL);
        svckey.GetValue(L"Start", &_start);
        svckey.GetValue(L"ErrorControl", &_errctl);
    }
}

CService::~CService()
{
}

CService& CService::operator = (const CService& svc)
{
    if(this != &svc) {
        _name = svc.GetName();
        _image = svc.GetImage();
        _displayname = svc.GetDisplayName();
        _description = svc.GetDescription();
        _group = svc.GetGroup();
        _type = svc.GetType();
        _start = svc.GetStart();
        _errctl = svc.GetErrorControl();
        _state = svc.GetCurrentState();
    }
    return *this;
}

LPCWSTR CService::GetTypeText() const throw()
{
    if(SERVICE_FILE_SYSTEM_DRIVER & _type) {
        return L"FileSystem Driver";
    }
    else if(SERVICE_KERNEL_DRIVER & _type) {
        return L"Kernel Driver";
    }
    else if(SERVICE_WIN32_OWN_PROCESS & _type) {
        return (0 == (_type&SERVICE_INTERACTIVE_PROCESS)) ? L"Win32 Own Process" : L"Win32 Own Process (Interactive)";
    }
    else if(SERVICE_WIN32_SHARE_PROCESS & _type) {
        return (0 == (_type&SERVICE_INTERACTIVE_PROCESS)) ? L"Win32 Share Process" : L"Win32 Share Process (Interactive)";
    }
    else {
        return L"Unknown";
    }
}

LPCWSTR CService::GetStartText() const throw()
{
    switch(_start)
    {
    case SERVICE_BOOT_START:
        return L"Boot";
    case SERVICE_SYSTEM_START:
        return L"System";
    case SERVICE_AUTO_START:
        return L"Auto";
    case SERVICE_DEMAND_START:
        return L"Demand";
    case SERVICE_DISABLED:
        return L"Disabled";
    default:
        break;
    }
    return L"Unknown";
}

LPCWSTR CService::GetErrorControlText() const throw()
{
    switch(_errctl)
    {
    case SERVICE_ERROR_IGNORE:
        return L"Ignore";
    case SERVICE_ERROR_NORMAL:
        return L"Normal";
    case SERVICE_ERROR_SEVERE:
        return L"Severe";
    case SERVICE_ERROR_CRITICAL:
        return L"Critical";
    default:
        break;
    }
    return L"Unknown";
}

LPCWSTR CService::GetCurrentStateText() const throw()
{
    switch(_state)
    {
    case SERVICE_CONTINUE_PENDING:
        return L"Continue Pending";
    case SERVICE_PAUSE_PENDING:
        return L"Pause Pending";
    case SERVICE_PAUSED:
        return L"Paused";
    case SERVICE_RUNNING:
        return L"Running";
    case SERVICE_START_PENDING:
        return L"Start Pending";
    case SERVICE_STOP_PENDING:
        return L"Stop Pending";
    case SERVICE_STOPPED:
        return L"Stopped";
    default:
        break;
    }
    return L"Unknown";
}


//
//  class CServices
//

CServices::CServices()
{
    Load();
}

CServices::~CServices()
{
}

void CServices::Load() throw()
{
    LoadEx(SERVICE_DRIVER | SERVICE_WIN32, SERVICE_STATE_ALL);
}

void CServices::LoadEx(DWORD dwServiceType, DWORD dwServiceState) throw()
{
    SC_HANDLE hSCManager = NULL;
    std::vector<UCHAR> buf;
    ULONG count = 0;
    ULONG size_required = 0;
    LPENUM_SERVICE_STATUS svc = NULL;
    
    hSCManager = OpenSCManagerW(NULL, NULL, GENERIC_READ);
    if(NULL == hSCManager) {
        return;
    }

    EnumServicesStatusW(hSCManager, dwServiceType, dwServiceState, NULL, 0, &size_required, &count, NULL);
    if(0 == size_required) {
        return;
    }

    size_required += sizeof(ENUM_SERVICE_STATUS);
    buf.resize(size_required, 0);
    if(!EnumServicesStatusW(hSCManager, dwServiceType, dwServiceState, (LPENUM_SERVICE_STATUS)&buf[0], (ULONG)buf.size(), &size_required, &count, NULL)) {
        return;
    }

    svc = (LPENUM_SERVICE_STATUS)&buf[0];
    for(int i=0; i<(int)count; i++) {
        _services.push_back(CService(svc[i].lpServiceName, svc[i].lpDisplayName, svc[i].ServiceStatus.dwServiceType, svc[i].ServiceStatus.dwCurrentState));        
    }
}
