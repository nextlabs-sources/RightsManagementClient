

#include <Windows.h>

#include <iostream>

#include <nudf\eh.hpp>
#include <nudf\filesys.hpp>
#include <nudf\crypto.hpp>
#include <nudf\string.hpp>
#include <nudf\conversion.hpp>
#include <nudf\security.hpp>




extern NX::fs::module_path  current_image;
extern NX::fs::dos_filepath current_image_dir;
extern NX::fs::dos_filepath current_work_dir;

bool test_security(void)
{
    bool result = true;

    try {

        const std::wstring const_wss(L"this is password");
        NX::secure::wsstring wss(const_wss);
        if (const_wss != wss.decrypt_string()) {
            throw std::exception("NX::secure::wsstring --> decrypted string not match original string");
        }

        const std::vector<unsigned char> const_mem({ 'a', 'b', 'c' });
        NX::secure::smemory smem(const_mem);
        if (const_mem != smem.decrypt()) {
            throw std::exception("NX::secure::smemory --> decrypted buffer not match original buffer");
        }
    }
    catch (std::exception& e) {
        std::cout << "    Exception: " << e.what() << std::endl;
        result = false;
    }

    return result;
}