


#include <Windows.h>
#include <assert.h>

#include <iostream>

#include <nudf\eh.hpp>
#include <nudf\string.hpp>
#include <nudf\filesys.hpp>
#include <nudf\service.hpp>
#include <nudf\http_service.hpp>
#include <nudf\winutil.hpp>

#include <boost/algorithm/string.hpp>

#include "nxkmsd.hpp"
#include "httpd.hpp"
#include "serv.hpp"


// {254744FF-5252-414A-A31E-2D7B0122E345}
const GUID KmsAppGuid = { 0x254744ff, 0x5252, 0x414a,{ 0xa3, 0x1e, 0x2d, 0x7b, 0x1, 0x22, 0xe3, 0x45 } };


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
                std::cout << "Install service nxrmsd: Succeeded" << std::endl;
            }
            else {
                std::cout << "Install service nxrmsd: Failed (" << ret << ")" << std::endl;
            }
        }
        else if (0 == _wcsicmp(argv[1], L"/u") || 0 == _wcsicmp(argv[1], L"--uninstall")) {
            ret = uninstall();
            if (0 == ret) {
                std::cout << "Uninstall service nxrmsd: Succeeded" << std::endl;
            }
            else {
                std::cout << "Uninstall service nxrmsd: Failed (" << ret << ")" << std::endl;
            }
        }
        else if (0 == _wcsicmp(argv[1], L"--config")) {

            if (argc < 3) {
                std::cout << "Invalid config command" << std::endl;
                return ERROR_INVALID_PARAMETER;
            }

            if (0 == _wcsicmp(argv[2], L"add_port")) {
                std::vector<unsigned short> ports;
                for (int i =3; i < argc; i++) {
                    int port = _wtoi(argv[i]);
                    if (port > 0 && port <= 0x7FFF) {
                        ports.push_back((unsigned short)port);
                    }
                }
                if (ports.empty()) {
                    std::cout << "Invalid port" << std::endl;
                    return ERROR_INVALID_PARAMETER;
                }
            }
            else if (0 == _wcsicmp(argv[2], L"remove_port")) {
                std::vector<unsigned short> ports;
                for (int i = 3; i < argc; i++) {
                    int port = _wtoi(argv[i]);
                    if (port > 0 && port <= 0x7FFF) {
                        ports.push_back((unsigned short)port);
                    }
                }
            }
            else {
                std::wcout << L"Unknown config command (" << argv[2] << L")" << std::endl;
                return ERROR_INVALID_PARAMETER;
            }

            for (int i = 2; i < argc; i++) {
                if (boost::algorithm::istarts_with(argv[i], L"add_port=")) {
                    // set port
                }
                else if (boost::algorithm::istarts_with(argv[i], L"remove_port=")) {
                    // remove port
                }
                else {
                    std::wcout << L"Unknown config data (" << argv[i] << L")" << std::endl;
                }
            }
        }
        else if (0 == _wcsicmp(argv[1], L"/c") || 0 == _wcsicmp(argv[1], L"--console")) {
            try {
                kms_serv serv;
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
                kms_serv serv;
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
        kms_serv  svc;
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
        kms_serv_conf conf(true, 18443, 18444, true, 0);
        conf.apply();
    }
    catch (const std::exception& e) {
        ret = GetLastError();
        if (0 == ret) {
            ret = -1;
        }
        std::cout << "ERROR: " << e.what() <<" (" << ret << ")" << std::endl;
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