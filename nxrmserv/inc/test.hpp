

#ifndef __NXRMSERV_TEST_HPP__
#define __NXRMSERV_TEST_HPP__


#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>

#include "policy.hpp"

namespace NX {
namespace test {



class test_template
{
public:
    test_template() {}
    test_template(const std::wstring& name, const std::wstring& desc) : _name(name), _desc(desc) {}
    virtual ~test_template() {}

    virtual bool run() noexcept = 0;

    inline const std::wstring& name() const noexcept { return _name; }
    inline const std::wstring& description() const noexcept { return _desc; }

private:
    std::wstring    _name;
    std::wstring    _desc;
};

class test_master
{
public:
    test_master() {}
    virtual ~test_master() {}

    void add_test(std::shared_ptr<test_template> pt) noexcept { _test_map.push_back(pt); }
    void run_all()
    {
        std::for_each(_test_map.begin(), _test_map.end(), [&](std::shared_ptr<test_template> pt) {
            std::wcout << L"Test Case: " << pt->name() << std::endl;
            if (pt->run()) {
                std::wcout << L"    ==> Succeeded" << std::endl;
            }
            else {
                std::wcout << L"    ==> Failed" << std::endl;
            }
        });
    }
    void run(const std::wstring& name)
    {
        auto pos = std::find_if(_test_map.begin(), _test_map.end(), [&](std::shared_ptr<test_template> pt) ->bool {
            return (0 == _wcsicmp(name.c_str(), pt->name().c_str()));
        });
        if (pos == _test_map.end()) {
            std::wcout << L"Test Case (" << name << L") not found" << std::endl;
            return;
        }

        run_test((*pos).get());
    }

    inline bool empty() const noexcept { return _test_map.empty(); }

private:
    void run_test(test_template* pt)
    {
        try {
            std::wcout << L"Test Case: " << pt->name() << std::endl;
            if (pt->run()) {
                std::wcout << L"    ==> Succeeded" << std::endl;
            }
            else {
                std::wcout << L"    ==> Failed" << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cout << "    ==> Exception (" << e.what() << ")" << std::endl;
        }
    }

private:
    std::vector<std::shared_ptr<test_template>> _test_map;
};

class test_policy : public test_template
{
public:
    test_policy();
    virtual ~test_policy();
    virtual bool run() noexcept;

protected:
    void test_xml_policy();
    void test_json_policy();
    void test_pql_policy();
    void save_policy();
    void evaluation();

private:
    NX::EVAL::policy_bundle _bundle;
};


}   // namespace NX::test
}   // namespace NX

#endif