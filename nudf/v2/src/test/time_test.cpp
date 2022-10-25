

#include <Windows.h>

#include <iostream>

#include <nudf\eh.hpp>
#include <nudf\filesys.hpp>
#include <nudf\crypto.hpp>
#include <nudf\string.hpp>
#include <nudf\conversion.hpp>
#include <nudf\time.hpp>



extern NX::fs::module_path  current_image;
extern NX::fs::dos_filepath current_image_dir;
extern NX::fs::dos_filepath current_work_dir;

bool test_time(void)
{
    bool result = true;

    try {

        NX::time::datetime dt1(L"2016-05-03 11:49:25.123");
        NX::time::datetime dt2(L"2016-05-03 11:49:25.123+08:00");
        NX::time::datetime dt3(L"2016-05-03 11:49:25.123Z");

        std::wstring sdt1 = dt1.serialize(true, true);
        std::wstring sdt2 = dt2.serialize(true, true);
        std::wstring sdt3 = dt3.serialize(true, true);
    }
    catch (std::exception& e) {
        std::cout << "    Exception: " << e.what() << std::endl;
        result = false;
    }

    return result;
}