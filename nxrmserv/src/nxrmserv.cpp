

#include <Windows.h>
#include <assert.h>

#include <iostream>
#include <string>

#include <nudf\exception.hpp>
#include <nudf\log.hpp>
#include <nudf\string.hpp>
#include <nudf\crypto.hpp>
#include <nudf\shared\moddef.h>

#include <rapidxml\rapidxml.hpp>

#include "nxrmserv.h"
#include "nxrmsvc.hpp"
#include "servlog.hpp"

#include "policy.hpp"

#include "test.hpp"


DECLARE_NXRM_MODULE_SERVER();


NX::global  GLOBAL;

static const std::wstring   CmdInstall(L"--install");
static const std::wstring   CmdUninstall(L"--uninstall");
static const std::wstring   CmdTest(L"--test");
static const std::wstring   CmdDelayStart(L"--delay=");

int wmain(int argc, wchar_t** argv)
{
    int nRet = 0;
    int nDelayStart = 0;

    if(argc > 2) {
        return ERROR_INVALID_PARAMETER;
    }
    
    if(!GLOBAL.init()) {
        return -1;
    }

    //Sleep(30000);
    NX::structured_exception::set_translate();

    std::vector<NX::sys::hardware::net_adapter> adapters;
    std::vector<NX::sys::hardware::disk>        disks;
    NX::sys::hardware::get_net_adapters(adapters);
    NX::sys::hardware::get_disks(disks);
    static const unsigned __int64 one_gb = 1073741824ULL; // 0x40000000

    LOGSYSINF(L" ");
    LOGSYSINF(L" ");
    LOGSYSINF(L"===============================================================");
    LOGSYSINF(L"*            NEXTLABS RIGHTS MANAGEMENT SERVICE               *");
#ifndef _DEBUG
    LOGSYSINF(L"*                    Version %02d.%02d.%04d                       *", VERSION_MAJOR, VERSION_MINOR, BUILD_NUMBER);
#else
    LOGSYSINF(L"*                 Version %02d.%02d.%04d (DEBUG)                  *", VERSION_MAJOR, VERSION_MINOR, BUILD_NUMBER);
#endif
    LOGSYSINF(L"===============================================================");
    LOGSYSINF(L"[SERVER] System Information:");
    LOGSYSINF(L"[SERVER]   - Host: %s", GLOBAL.host().in_domain() ? GLOBAL.host().fully_qualified_domain_name().c_str() : GLOBAL.host().name().c_str());
    LOGSYSINF(L"[SERVER]   - OS: %s (%s)", NX::sys::os::os_name().c_str(), NX::sys::os::is_64bits() ? L"64 bits" : L"32 bits");
    LOGSYSINF(L"[SERVER]   - Language: %s", NX::sys::os::os_locale_name().c_str());
    LOGSYSINF(L"[SERVER]   - Timezone: %s", L"PDT (-07:00)");
    LOGSYSINF(L"[SERVER]   - CPU: %s", NX::sys::hardware::cpu_brand().c_str());
    LOGSYSINF(L"[SERVER]   - Memory: %d MB", NX::sys::hardware::memory_in_mb());
    LOGSYSINF(L"[SERVER]   - Harddisks:");
    LOGSYSINF(L"[SERVER]     ------------------------------------------");
    LOGSYSINF(L"[SERVER]         Drive    Type       Space (GB)   Free ");
    std::for_each(disks.begin(), disks.end(), [&](const NX::sys::hardware::disk& d) {
        double total = (0 == d.total()) ? 0 : (((double)d.total() / one_gb));
        wchar_t sv[64] = { 0 };
        std::wstring s;
        swprintf_s(nudf::string::tempstr<wchar_t>(s, 64), 64, L"%.1f", total);
        if (s.length() >= 6) {
            wcsncpy_s(sv, 63, s.c_str(), _TRUNCATE);
        }
        else {
            static const wchar_t* space_buf = L"      ";
            wcsncpy_s(sv, 63, space_buf, (6 - s.length()));
            wcsncat_s(sv, 63, s.c_str(), _TRUNCATE);
        }
        int total_free = (0 == d.total()) ? 0 : ((int)((d.total_free() * 100) / d.total()));
        LOGSYSINF(L"[SERVER]          %C:      %9s   %s       %02d%%", d.name().c_str()[0], d.type_string().c_str(), sv, total_free);
    });
    LOGSYSINF(L"[SERVER]   - Network Adapters:");
    LOGSYSINF(L"[SERVER]     ------------------------------------------");
    std::for_each(adapters.begin(), adapters.end(), [&](const NX::sys::hardware::net_adapter& adapter) {
        LOGSYSINF(L"[SERVER]     * %s (%s)", adapter.friendly_name().c_str(), adapter.description().c_str());
        LOGSYSINF(L"[SERVER]         Media State:  %s", (adapter.is_connected() ? L"Connected" : L"Disconnected"));
        LOGSYSINF(L"[SERVER]         IPv4 Address: %s", (adapter.ipv4_addresses().empty() ? L"" : adapter.ipv4_addresses()[0].c_str()));
    });
    NX::logged_on_users lou;
    LOGSYSINF(L"[SERVER]   - Users on this machine:");
    std::for_each(lou.users().begin(), lou.users().end(), [&](const std::pair<std::wstring, std::wstring>& user) {
        LOGSYSINF(L"[SERVER]       * %s: %s", user.first.c_str(), user.second.c_str());
    });
    LOGSYSINF(L"[SERVER]   - NextLabs Rights Management Client:");
    LOGSYSINF(L"[SERVER]       * Version: %s", GLOBAL.product().version().version_str().c_str());
    LOGSYSINF(L"[SERVER]       * Language: %s", GLOBAL.product().language().name().c_str());
    LOGSYSINF(L"[SERVER]       * Debug: yes");
    LOGSYSINF(L" ");
    LOGSYSINF(L"[SERVER] Global data initialized");
    if (!nudf::crypto::Initialize()) {
        LOGSYSINF(L"[SERVER] Fail to initialize crypto library");
    }
    LOGSYSINF(L"[SERVER] Crypto library initialized");
    
    //
    //  Run with parameters
    //
    if(argc >= 2) {

        if(0 == _wcsicmp(argv[1], CmdInstall.c_str())) {

            if(!nudf::win::svc::Exist(NXRMSERV_NAME)) {

                try {

                    nudf::win::svc::CService RmSvc;
                    nudf::win::svc::CConfig RmSvcCfg;

                    RmSvcCfg.SetBinaryPathName(GLOBAL.module().full_path().c_str());
                    RmSvcCfg.SetDisplayName(NXRMSERV_DISPNAME);
                    RmSvcCfg.SetServiceType(SERVICE_WIN32_OWN_PROCESS);
                    RmSvcCfg.SetStartType(SERVICE_DEMAND_START);
                    RmSvcCfg.SetErrorControl(SERVICE_ERROR_NORMAL);
                    RmSvc.Create(NXRMSERV_NAME, RmSvcCfg);

                    std::cout << "Service Install successfully!" << std::endl;
                }
                catch(const nudf::CException& e) {
                    std::cout << "Service Install failed" << std::endl;
                    std::cout << "    Error: " << e.GetCode() << std::endl;
                    std::cout << "    File: " << e.GetFile() << std::endl;
                    std::cout << "    Line: " << e.GetLine() << std::endl;
                    std::cout << "    Function: " << e.GetFunction() << std::endl;
                    nRet = e.GetCode();
                }
            }
            else {
                std::cout << "Service already exists." << std::endl;
            }
            return nRet;
        }
        else if(0 == _wcsicmp(argv[1], CmdUninstall.c_str())) {

            if(nudf::win::svc::Exist(NXRMSERV_NAME)) {

                try {
                    nudf::win::svc::Delete(NXRMSERV_NAME);
                    std::cout << "Service has been uninstalled!" << std::endl;
                }
                catch(const nudf::CException& e) {
                    std::cout << "Service Uninstall failed:" << std::endl;
                    std::cout << "    Error: " << e.GetCode() << std::endl;
                    std::cout << "    File: " << e.GetFile() << std::endl;
                    std::cout << "    Line: " << e.GetLine() << std::endl;
                    std::cout << "    Function: " << e.GetFunction() << std::endl;
                    nRet = e.GetCode();
                }
            }
            else {
                std::cout << "Service NOT exists." << std::endl;
            }
            return nRet;
        }
        else if(0 == _wcsicmp(argv[1], L"--help") || 0 == _wcsicmp(argv[1], L"-h") || 0 == _wcsicmp(argv[1], L"/h")) {
            std::wcout << NXRMSERV_DESC << L" (Ver " << VERSION_MAJOR << L"." << VERSION_MINOR << L"." << BUILD_NUMBER << L"." << L")" << std::endl;
            std::cout << "    nxrmserv.exe" " [--install|--uninstall|--help|--delay=<milliseconds>]" << std::endl;
            return 0;
        }
        else if(0 == _wcsnicmp(argv[1], CmdDelayStart.c_str(), CmdDelayStart.length())) {
            if(!nudf::string::ToInt<wchar_t>(argv[1]+CmdDelayStart.length(), &nDelayStart)) {
                nDelayStart = 0;
            }
        }
        else if (0 == _wcsicmp(argv[1], CmdTest.c_str())) {
            NX::test::test_master master;
            for (int i = 2; i < argc; i++) {
                if (0 == _wcsicmp(argv[i], L"policy")) {
                    master.add_test(std::shared_ptr<NX::test::test_template>(new NX::test::test_policy()));
                }
                else {
                    continue;   // unknown test case name
                }
            }
            if (master.empty()) {
                // run all test
                master.add_test(std::shared_ptr<NX::test::test_template>(new NX::test::test_policy()));
            }
            master.run_all();
        }
        else {
            return ERROR_INVALID_PARAMETER;
        }
    }

    //
    // Run as normal service
    //
    nudf::win::svc::IServiceInstance<CRmService>  RmService;

    try {
        RmService->set_delay(nDelayStart);
        LOGSYSINF(L"[SERVER] Launching service ...");
        RmService->Run();
    }
    catch(const nudf::CException& e) {
        nRet = e.GetCode();
        LOG_EXCEPTION(e);
    }

    // Return
    return nRet;
}