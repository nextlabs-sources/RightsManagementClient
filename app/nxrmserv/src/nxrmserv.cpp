

#include <Windows.h>

#include <string>
#include <iostream>
#include <boost/algorithm/string.hpp>

#include <nudf\eh.hpp>
#include <nudf\debug.hpp>
#include <nudf\filesys.hpp>

#include "nxrmserv.hpp"
#include "serv.hpp"


// {EB898211-0949-4D3E-A9B7-172F1FA61F4C}
const GUID NxServAppGuid = { 0xeb898211, 0x949, 0x4d3e,{ 0xa9, 0xb7, 0x17, 0x2f, 0x1f, 0xa6, 0x1f, 0x4c } };


static int install(const std::wstring& image);
static int uninstall();




int wmain(int argc, wchar_t** argv)
{
    int ret = 0;
    const NX::fs::module_path image_path(NULL);
    const NX::fs::dos_filepath parent_dir(image_path.file_dir());

    // with parameter?
    if (argc > 1) {

        if (0 == _wcsicmp(argv[1], L"/i") || 0 == _wcsicmp(argv[1], L"--install")) {
            ret = install(image_path.path());
            if (0 == ret) {
                std::cout << "Install service nxrmserv: Succeeded" << std::endl;
            }
            else {
                std::cout << "Install service nxrmserv: Failed (" << ret << ")" << std::endl;
            }
        }
        else if (0 == _wcsicmp(argv[1], L"/u") || 0 == _wcsicmp(argv[1], L"--uninstall")) {
            ret = uninstall();
            if (0 == ret) {
                std::cout << "Uninstall service nxrmserv: Succeeded" << std::endl;
            }
            else {
                std::cout << "Uninstall service nxrmserv: Failed (" << ret << ")" << std::endl;
            }
        }
        else if (0 == _wcsicmp(argv[1], L"/c") || 0 == _wcsicmp(argv[1], L"--console")) {
            try {
                rmserv serv;
                serv.run_as_console();
            }
            catch (const std::exception& e) {
                std::cout << "nxrmsd exception: " << e.what() << std::endl;
                ret = GetLastError();
                if (0 == ret) {
                    ret = -1;
                }
            }
        }
        else {
            try {
                rmserv serv;
                serv.run();
            }
            catch (const std::exception& e) {
                UNREFERENCED_PARAMETER(e);
            }
        }
    }

    //
    // Run as normal service
    //

    try {
        rmserv  svc;
        svc.run();
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    return ret;
}


int install(const std::wstring& image)
{
    int ret = 0;

    if (NX::win::service_control::exists(NXRMS_SERVICE_NAME)) {
        SetLastError(ERROR_SERVICE_EXISTS);
        return ERROR_SERVICE_EXISTS;
    }

    try {

        const NX::win::service_info rms_info(NXRMS_SERVICE_NAME,
            image,
            NXRMS_SERVICE_DISPLAY_NAME,
            NXRMS_SERVICE_DESCRIPTION,
            SERVICE_WIN32_OWN_PROCESS,
            SERVICE_AUTO_START);
        NX::win::service_control sc;
        sc.create(rms_info);

        // set default value
        //rmserv_conf conf(true, 18443, 18444, true, 0);
        //conf.apply();
    }
    catch (const std::exception& e) {
        ret = GetLastError();
        if (0 == ret) {
            ret = -1;
        }
        std::cout << "ERROR: " << e.what() << " (" << ret << ")" << std::endl;
    }

    return ret;
}

int uninstall()
{
    int ret = 0;

    if (!NX::win::service_control::exists(NXRMS_SERVICE_NAME)) {
        SetLastError(ERROR_SERVICE_NOT_FOUND);
        return ERROR_SERVICE_NOT_FOUND;
    }

    try {

        NX::win::service_control sc;
        sc.open(NXRMS_SERVICE_NAME);
        sc.remove(true, 60000); // wait for one minutes
    }
    catch (const std::exception& e) {
        ret = GetLastError();
        if (0 == ret) {
            ret = -1;
        }
        std::cout << "ERROR: " << e.what() << " (" << ret << ")" << std::endl;
    }

    return ret;
}