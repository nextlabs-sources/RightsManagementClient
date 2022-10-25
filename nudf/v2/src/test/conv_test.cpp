
#include <Windows.h>

#include <iostream>
#include <string>
#include <set>
#include <map>

#include <nudf\eh.hpp>
#include <nudf\debug.hpp>
#include <nudf\string.hpp>
#include <nudf\xml.hpp>
#include <nudf\security.hpp>
#include <nudf\http_client.hpp>
#include <nudf\json.hpp>
#include <nudf\time.hpp>
#include <nudf\winutil.hpp>
#include <nudf\filesys.hpp>



extern NX::fs::module_path  current_image;
extern NX::fs::dos_filepath current_image_dir;
extern NX::fs::dos_filepath current_work_dir;


// test routines
bool test_conv(void)
{
    bool result = true;

    try {
        ; // Nothing
    }
    catch (std::exception& e) {
        std::cout << "    Exception: " << e.what() << std::endl;
        result = false;
    }

    return result;
}