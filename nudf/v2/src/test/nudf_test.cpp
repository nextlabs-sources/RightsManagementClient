

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

// test routines
extern bool test_eh(void);
extern bool test_debug(void);
extern bool test_string(void);
extern bool test_time(void);
extern bool test_uri(void);
extern bool test_conv(void);
extern bool test_win(void);
extern bool test_crypto(void);
extern bool test_filesys(void);
extern bool test_security(void);
extern bool test_xml(void);
extern bool test_json(void);
extern bool test_http(void);
extern bool test_misc(void);

static const char* pause_info = "Press any key to continue ...";


NX::fs::module_path  current_image(NULL);
NX::fs::dos_filepath current_image_dir(current_image.file_dir());
NX::fs::dos_filepath current_work_dir(NX::fs::dos_filepath::get_current_directory());


typedef bool (*test_func)(void);
class test_job {
public:
    test_job(test_func fn, bool on = false) : _fn(fn), _on(on) {}
    ~test_job() {}

    bool run() const { return _fn(); }
    void enable() { _on = true; }
    void disable() { _on = false; }
    bool is_on() const { return _on; }
    test_func func() const { return _fn; }
    test_job& operator = (const test_job& other)
    {
        if (this != &other) {
            _fn = other.func();
            _on = other.is_on();
        }
        return *this;
    }

    
private:
    test_func   _fn;
    bool        _on;
};

static std::map<std::string, test_job> jobs = {
    { "error handling", test_job(test_eh) },
    { "debug", test_job(test_debug) },
    { "string", test_job(test_string) },
    { "time", test_job(test_time) },
    { "uri", test_job(test_uri) },
    { "conv", test_job(test_conv) },
    { "win", test_job(test_win) },
    { "crypto", test_job(test_crypto) },
    { "fs", test_job(test_filesys) },
    { "security", test_job(test_security) },
    { "json", test_job(test_json) },
    { "xml", test_job(test_xml) },
    { "http", test_job(test_http) },
    { "misc", test_job(test_misc) }
};

static bool run_jobs(bool stop_at_error = true);

// main
int main(int argc, char** argv)
{
    int result = 0;
    bool debug_break = false;
    
    for (int i = 1; i < argc; i++) {
        std::string job_name = argv[i];
        std::transform(job_name.begin(), job_name.end(), job_name.begin(), tolower);
        if (job_name == "break") {
            debug_break = true;
            continue;
        }
        if (job_name == "all") {
            std::for_each(jobs.begin(), jobs.end(), [](std::pair<const std::string, test_job>& it) {
                it.second.enable();
            });
            break;
        }

        auto it = jobs.find(job_name);
        if (it != jobs.end()) {
            (*it).second.enable();
        }
    }
    
    if (!run_jobs()) {
        std::cout << std::endl;
        std::cout << "FAILED" << std::endl;
        return -1;
    }

    std::cout << std::endl;
    std::cout << "PASSED" << std::endl;
    return 0;
}

bool run_jobs(bool stop_at_error)
{
    bool final_result = true;

    try {

        std::for_each(jobs.begin(), jobs.end(), [&](std::pair<const std::string, test_job>& item) {

            bool result = true;

            if (!item.second.is_on()) {
                return;
            }

            std::cout << "" << std::endl;
            std::cout << "TEST >> " << item.first << std::endl;
            result = item.second.run();

            if (!result) {
                final_result = false;
                if (stop_at_error) {
                    throw std::exception("stop at error");
                }
            }
        });
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }

    return final_result;
}