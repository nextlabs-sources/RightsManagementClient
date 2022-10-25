
#include <Windows.h>

#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <map>

#include <nudf\eh.hpp>
#include <nudf\debug.hpp>
#include <nudf\string.hpp>
#include <nudf\conversion.hpp>
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

static void dump_installed_software();
static void dump_installed_kbs();
static void test_pe_file();


// test routines
bool test_win(void)
{
    bool result = true;

    try {
        test_pe_file();
        dump_installed_software();
        dump_installed_kbs();
    }
    catch (std::exception& e) {
        std::cout << "    Exception: " << e.what() << std::endl;
        result = false;
    }

    return result;
}

class dump_file
{
public:
    dump_file() : _h(INVALID_HANDLE_VALUE) {}
    ~dump_file()
    {
        close();
    }

    inline bool opened() const { return (INVALID_HANDLE_VALUE != _h); }
    void open(const std::wstring& file, bool trunc, bool write_through)
    {
        _h = ::CreateFileW(file.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, trunc ? CREATE_ALWAYS : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    void close()
    {
        if (INVALID_HANDLE_VALUE != _h) {
            CloseHandle(_h);
            _h = INVALID_HANDLE_VALUE;
        }
    }

    dump_file& operator << (const std::wstring& s)
    {
        if (s.length() != 0) {
            unsigned long bytes_written = 0;
            ::WriteFile(_h, s.c_str(), (unsigned long)(s.length() * 2), &bytes_written, NULL);
        }

        return *this;
    }

    dump_file& operator << (int n)
    {
        return (*this << NX::conversion::to_wstring(n));
    }

    dump_file& operator << (__int64 n)
    {
        return (*this << NX::conversion::to_wstring(n));
    }

    dump_file& operator << (unsigned long u)
    {
        return (*this << NX::conversion::to_wstring(u));
    }

    dump_file& operator << (unsigned __int64 u)
    {
        return (*this << NX::conversion::to_wstring(u));
    }

    static const std::wstring endl;
    
private:
    HANDLE _h;
};

const std::wstring dump_file::endl(L"\r\n");

void dump_installed_software()
{
    const std::vector<NX::win::installation::software>& installed_software = NX::win::installation::get_installed_software();
    std::wstring file = current_image_dir.path() + L"\\installed_software.txt";
    
    dump_file fs;
    fs.open(file, true, true);
    fs << L"Total Installed software: " << (int)installed_software.size() << dump_file::endl;
    int i = 0;
    std::for_each(installed_software.begin(), installed_software.end(), [&](const NX::win::installation::software& s) {
        
        fs << L"" << dump_file::endl;
        fs << i++ << L". \t" << s.name() << dump_file::endl;
        fs << L"   Architecture: " << (s.x64() ? L"x64" : L"x86") << dump_file::endl;
        fs << L"   Language:     " << s.lang().name() << dump_file::endl;
        fs << L"   Version:      " << s.version() << dump_file::endl;
        fs << L"   Publisher:    " << s.publisher() << dump_file::endl;
        fs << L"   Install Date: " << s.install_date() << dump_file::endl;
        fs << L"   Install Dir:  " << s.install_dir() << dump_file::endl;
    });
    fs.close();
}

void dump_installed_kbs()
{
    const std::vector<std::wstring>& installed_kbs = NX::win::installation::get_installed_kbs();
    std::wstring file = current_image_dir.path() + L"\\installed_kbs.txt";
    
    dump_file fs;
    fs.open(file, true, true);
    fs << L"Total Installed KBs: " << (int)installed_kbs.size() << dump_file::endl;
    std::for_each(installed_kbs.begin(), installed_kbs.end(), [&](const std::wstring& kbs) {
        fs << L"   " << kbs << dump_file::endl;
    });
    fs.close();
}

void test_pe_file()
{
    NX::win::pe_file winword_info(L"C:\\Program Files (x86)\\Microsoft Office\\Office15\\WINWORD.EXE");
}