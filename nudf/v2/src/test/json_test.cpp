

#include <Windows.h>

#include <iostream>

#include <nudf\eh.hpp>
#include <nudf\filesys.hpp>
#include <nudf\crypto.hpp>
#include <nudf\string.hpp>
#include <nudf\conversion.hpp>
#include <nudf\json.hpp>




extern NX::fs::module_path  current_image;
extern NX::fs::dos_filepath current_image_dir;
extern NX::fs::dos_filepath current_work_dir;

bool test_json(void)
{
    static std::wstring json_str1(
        L"{"
        L"    \"firstName\": \"John\","
        L"    \"lastName\" : \"Smith\","
        L"    \"isAlive\" : true,"
        L"    \"age\" : 25,"
        L"    \"address\" : {"
        L"    \"streetAddress\": \"21 2nd Street\","
        L"        \"city\" : \"New York\","
        L"        \"state\" : \"NY\","
        L"        \"postalCode\" : \"10021-3100\""
        L"    },"
        L"    \"phoneNumbers\": ["
        L"    {"
        L"        \"type\": \"home\","
        L"        \"number\" : \"212 555-1234\""
        L"    },"
        L"    {"
        L"        \"type\": \"office\","
        L"        \"number\" : \"646 555-4567\""
        L"    }"
        L"    ],"
        L"    \"children\": [],"
        L"    \"spouse\" : null"
        L"}"
        );

    bool result = false;

    int i = 0;
    while (i++ < 100) {
        try {
            std::shared_ptr<NX::json_value> sp = NX::json_value::parse(json_str1);
            std::wstring s = sp->serialize();
            result = true;
        }
        catch (std::exception& e) {
            std::cout << "JSON Parsing Error: " << e.what() << std::endl;
        }
    }

    return result;
}