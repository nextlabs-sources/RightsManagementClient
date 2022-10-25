

#include <Windows.h>

#include <nudf\exception.hpp>

#pragma warning(push)
#pragma warning(disable: 4244)
#include <rapidxml\rapidxml.hpp>
#include <rapidxml\rapidxml_utils.hpp>
#include <rapidxml\rapidxml_print.hpp>
#pragma warning(pop)



#include "test.hpp"


using namespace NX;
using namespace NX::test;


test_policy::test_policy() : test_template(L"Policy Test", L"Test nxrmserv's policy system")
{
}

test_policy::~test_policy()
{
}

bool test_policy::run() noexcept
{
    try {
        test_xml_policy();
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        return false;
    }
    return true;
}

void test_policy::test_xml_policy()
{
    std::wstring ws_policy_xml;
    try {

        rapidxml::file<> xmlFile("d:\\temp\\policy.xml");
        rapidxml::xml_document<> doc;

        doc.parse<0>(xmlFile.data());

        std::string s;
        rapidxml::print(std::back_inserter(s), doc);
        ws_policy_xml = std::wstring(s.begin(), s.end());
    }
    catch (const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        std::cout << "Exception: " << e.GetFile() << ", " << e.GetFunction() << ", " << e.GetLine() << ", " << (e.what() ? e.what() : "") << std::endl;
        throw std::exception("fail to load xml policy file");
    }

    _bundle = NX::EVAL::policy_bundle::load(ws_policy_xml);
}

void test_policy::test_json_policy()
{
}

void test_policy::test_pql_policy()
{
}

void test_policy::save_policy()
{
}

void test_policy::evaluation()
{
}
