

#include <Windows.h>

#include <iostream>

#include <nudf\shared\logdef.h>

#include "servctrl.hpp"

using namespace NXSERV;

int main(int argc, char** argv)
{
    system("pause");
    std::cout << "Test 1: query Status" << std::endl;
    const serv_status& status = NXSERV::query_serv_status();
    if (status.empty()) {
        std::cout << "    ==> Failed" << std::endl;
    }
    else {
        std::wcout << L"    ==> Succeeded" << std::endl;
        std::wcout << L"      * Connected: " << (status.is_connected() ? L"yes" : L"no") << std::endl;
        std::wcout << L"      * DebugMode: " << (status.is_debug_mode_on() ? L"yes" : L"no") << std::endl;
        std::wcout << L"      * Policy TIme: " << status.get_policy_time() << std::endl;
        std::wcout << L"      * HeartBeat TIme: " << status.get_heartbeat_time() << std::endl;
        std::wcout << L"      * Product Version: " << status.get_product_version() << std::endl;
        std::wcout << L"      * Logon Tenant: " << status.get_logon_tenant() << std::endl;
        std::wcout << L"      * Logon User: " << status.get_logon_user() << std::endl;
        std::wcout << L"      * Logon Time: " << status.get_logon_time() << std::endl;
    }

    std::cout << "Test 2: update policy" << std::endl;
    if (!NXSERV::update_policy()) {
        std::cout << "    ==> Failed" << std::endl;
    }
    else {
        std::cout << "    ==> Succeeded" << std::endl;
    }

    std::cout << "Test 3: enable debug" << std::endl;
    if (!NXSERV::enable_debug(true)) {
        std::cout << "    ==> Failed" << std::endl;
    }
    else {
        std::cout << "    ==> Succeeded" << std::endl;
    }

    std::cout << "Test 2: collect debug log" << std::endl;
    if (!NXSERV::collect_debug_log()) {
        std::cout << "    ==> Failed" << std::endl;
    }
    else {
        std::cout << "    ==> Succeeded" << std::endl;
    }

    std::cout << "Test 2: query authn info" << std::endl;
    const NXSERV::authn_info& ainfo = NXSERV::query_authn_info();
    if (ainfo.empty()) {
        std::cout << "    ==> Failed" << std::endl;
    }
    else {
        std::cout << "    ==> Succeeded" << std::endl;
    }

    std::cout << "Test 2: login" << std::endl;
    const NXSERV::authn_result& aresult = NXSERV::log_in();
    if (aresult.empty()) {
        std::cout << "    ==> Failed" << std::endl;
    }
    else {
        std::cout << "    ==> Succeeded" << std::endl;
    }

    std::cout << "Test 2: logout" << std::endl;
    if(!NXSERV::log_out()) {
        std::cout << "    ==> Failed" << std::endl;
    }
    else {
        std::cout << "    ==> Succeeded" << std::endl;
    }


    std::cout << "Test 2: set dwm flag" << std::endl;
    if (!NXSERV::set_dwm_status(true)) {
        std::cout << "    ==> Failed" << std::endl;
    }
    else {
        std::cout << "    ==> Succeeded" << std::endl;
    }

    std::cout << "Test 2: logging" << std::endl;    
    if (!NXSERV::write_log(LOGINFO, L"SERVCTL TEST ==> Succeed")) {
        std::cout << "    ==> Failed" << std::endl;
    }
    else {
        std::cout << "    ==> Succeeded" << std::endl;
    }

    return 0;
}