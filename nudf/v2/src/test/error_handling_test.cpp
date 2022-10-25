

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

bool test_eh(void)
{
    ENABLE_STRUCTURED_EXCEPTION();

    bool result = false;

    try {
        typedef void(*crash_t)();
        crash_t crash = nullptr;
#pragma prefast(suppress: 6011, "Dereferencing NULL pointer")
        crash();
    }
    catch (NX::structured_exception& e) {
        UNREFERENCED_PARAMETER(e);
        result = true;
    }

    return result;
}