

#include <Windows.h>

#include <iostream>

#include <nudf\eh.hpp>
#include <nudf\filesys.hpp>
#include <nudf\crypto.hpp>
#include <nudf\string.hpp>
#include <nudf\conversion.hpp>
#include <nudf\winutil.hpp>




extern NX::fs::module_path  current_image;
extern NX::fs::dos_filepath current_image_dir;
extern NX::fs::dos_filepath current_work_dir;

bool test_misc(void)
{
    bool result = true;

    try {

        const std::vector<NX::win::installation::software>& installed_software = NX::win::installation::get_installed_software();
        std::cout << "    Installed software: " << installed_software.size() << std::endl;
        const std::vector<std::wstring>& installed_kbs = NX::win::installation::get_installed_kbs();
        std::cout << "    Installed KBs: " << installed_kbs.size() << std::endl;

    }
    catch (std::exception& e) {
        std::cout << "    Exception: " << e.what() << std::endl;
        result = false;
    }

    return result;
}