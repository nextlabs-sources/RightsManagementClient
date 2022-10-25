

#include <Windows.h>

#include <iostream>

#include <nudf\eh.hpp>
#include <nudf\filesys.hpp>
#include <nudf\crypto.hpp>
#include <nudf\string.hpp>
#include <nudf\conversion.hpp>




extern NX::fs::module_path  current_image;
extern NX::fs::dos_filepath current_image_dir;
extern NX::fs::dos_filepath current_work_dir;

bool test_filesys(void)
{
    bool result = true;

    try {

        std::vector<std::wstring> buf = {
            L"S:\\build\\release_candidate\\RightsManagementClient\\8.2\\946\\NextLabsRightsManagement-8.2.946.zip",
            L"D:\\A.Brief.History.of.Humankind.mobi",
            L"\\\\?\\build\\release_candidate\\RightsManagementClient\\8.2\\946\\NextLabsRightsManagement-8.2.946.zip",
            L"\\??\\D:\\A.Brief.History.of.Humankind.mobi",
            L"\\\\nextlabs.com\\share\\data\\build\\release_candidate\\RightsManagementClient\\8.2\\946\\NextLabsRightsManagement-8.2.946.zip"
        };

        std::for_each(buf.begin(), buf.end(), [](const std::wstring& s) {
            NX::fs::nt_filepath ntpath(s);
            NX::fs::dos_filepath dospath(s);
            std::cout << std::endl;
            std::wcout << L"Input: " << s << std::endl;
            std::wcout << L"    [NT Path]  " << ntpath.path() << std::endl;
            std::wcout << L"    [Dos Path] " << dospath.path() << std::endl;
        });
    }
    catch (std::exception& e) {
        std::cout << "    Exception: " << e.what() << std::endl;
        result = false;
    }

    return result;
}