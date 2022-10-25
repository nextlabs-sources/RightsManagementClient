
#include <Windows.h>
#include <stdio.h>
#include <assert.h>

#include <nudf\resutil.hpp>
#include <nudf\string.hpp>

using namespace nudf::util::res;



std::wstring nudf::util::res::LoadMessage(_In_ HMODULE module, _In_ UINT id, _In_ ULONG max_length, _In_ DWORD langid, _In_opt_ LPCWSTR default_msg)
{
    std::wstring wstr;
    if(NULL == module || 0 == FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE, module, id, langid, nudf::string::tempstr<wchar_t>(wstr, max_length),  max_length,  NULL)) {
        wstr.clear();
    }
    if(wstr.empty() && NULL!=default_msg) {
        wstr = default_msg;
    }
    return wstr;
}

std::wstring nudf::util::res::LoadMessageEx(_In_ HMODULE module, _In_ UINT id, _In_ ULONG max_length, _In_ DWORD langid, _In_opt_ LPCWSTR default_msg, ...)
{
    std::wstring wstr;
    std::wstring fmt;
    std::vector<wchar_t> buf;
    int len = 0;

    fmt = nudf::util::res::LoadMessage(module, id, max_length, langid, default_msg);
    if(fmt.empty()) {
        return wstr;
    }

    va_list args;
    va_start(args, default_msg);
    len = _vscwprintf_l(fmt.c_str(), 0, args) + 1;
    buf.resize(len, 0);
    if(!buf.empty()) {
        vswprintf_s(&buf[0], len, fmt.c_str(), args); // C4996
        wstr = &buf[0];
    }
    va_end(args);

    return wstr;
}