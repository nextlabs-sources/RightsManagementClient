


#include <Windows.h>

#include <algorithm>

#include <nudf\eh.hpp>
#include <nudf\string.hpp>
#include <nudf\conversion.hpp>



std::wstring NX::conversion::utf8_to_utf16(const std::string& s)
{
    std::wstring ws;
    if (!s.empty()) {
        if (0 == MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.length(), NX::string_buffer<wchar_t>(ws, s.length() + 1), (int)s.length() + 1)) {
            ws.clear();
        }
    }
    return std::move(ws);
}

std::string NX::conversion::utf16_to_utf8(const std::wstring& ws)
{
    std::string s;
    if (!ws.empty()) {
        if (0 == WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.length(), NX::string_buffer<char>(s, ws.length()), ((int)ws.length()/2) * 3 + 1, nullptr, nullptr)) {
            s.clear();
        }
    }
    return std::move(s);
}

std::wstring NX::conversion::ansi_to_utf16(const std::string& s)
{
    std::wstring ws;
    if (!s.empty()) {
        if (0 == MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.length(), NX::string_buffer<wchar_t>(ws, s.length()), (int)s.length() + 1)) {
            ws.clear();
        }
    }
    return std::move(ws);
}

std::string NX::conversion::ansi_to_utf8(const std::string& s)
{
    std::wstring ws = ansi_to_utf16(s);
    return std::move(utf16_to_utf8(ws));
}

std::wstring NX::conversion::to_utf16(const std::wstring& s)
{
    return s;
}

std::wstring NX::conversion::to_utf16(const std::string& s)
{
    return std::move(NX::conversion::utf8_to_utf16(s));
}

std::wstring NX::conversion::to_wstring(unsigned char v)
{
    std::wstring ws = NX::string_formater(L"%02X", v);
    return std::move(ws);
}

std::wstring NX::conversion::to_wstring(unsigned short v)
{
    std::wstring ws = NX::string_formater(L"0x%04X", v);
    return std::move(ws);
}

std::wstring NX::conversion::to_wstring(unsigned long v)
{
    std::wstring ws = NX::string_formater(L"0x%08X", v);
    return std::move(ws);
}

std::wstring NX::conversion::to_wstring(unsigned int v)
{
    std::wstring ws = NX::string_formater(L"0x%08X", v);
    return std::move(ws);
}

std::wstring NX::conversion::to_wstring(unsigned __int64 v)
{
    std::wstring ws = NX::string_formater(L"0x%016llX", v);
    return std::move(ws);
}

std::wstring NX::conversion::to_wstring(short v)
{
    std::wstring ws = NX::string_formater(L"%d", (int)v);
    return std::move(ws);
}

std::wstring NX::conversion::to_wstring(long v)
{
    std::wstring ws = NX::string_formater(L"%d", v);
    return std::move(ws);
}

std::wstring NX::conversion::to_wstring(int v)
{
    std::wstring ws = NX::string_formater(L"%d", v);
    return std::move(ws);
}

std::wstring NX::conversion::to_wstring(__int64 v)
{
    std::wstring ws = NX::string_formater(L"%I64d", v);
    return std::move(ws);
}

std::wstring NX::conversion::to_wstring(float v, int precision)
{
    std::wstring fmt(L"%%.");
    fmt += to_wstring(precision) + L"f";
    std::wstring ws = NX::string_formater(fmt.c_str(), v);
    return std::move(ws);
}

std::wstring NX::conversion::to_wstring(double v, int precision)
{
    std::wstring fmt(L"%%.");
    fmt += to_wstring(precision) + L"f";
    std::wstring ws = NX::string_formater(fmt.c_str(), v);
    return std::move(ws);
}

std::wstring NX::conversion::to_wstring(unsigned char* v, size_t size)
{
    std::wstring ws;
    std::for_each(v, v + size, [&](unsigned char c) {
        ws += to_wstring(c);
    });
    return std::move(ws);
}

std::wstring NX::conversion::to_wstring(const std::vector<unsigned char>& v)
{
    std::wstring ws;
    std::for_each(v.begin(), v.end(), [&](unsigned char c) {
        ws += to_wstring(c);
    });
    return std::move(ws);
}

std::string NX::conversion::to_string(unsigned char v)
{
    std::string s = NX::string_formater("%02X", v);
    return std::move(s);
}

std::string NX::conversion::to_string(unsigned short v)
{
    std::string s = NX::string_formater("0x%04X", v);
    return std::move(s);
}

std::string NX::conversion::to_string(unsigned long v)
{
    std::string s = NX::string_formater("0x%08X", v);
    return std::move(s);
}

std::string NX::conversion::to_string(unsigned int v)
{
    std::string s = NX::string_formater("0x%08X", v);
    return std::move(s);
}

std::string NX::conversion::to_string(unsigned __int64 v)
{
    std::string s = NX::string_formater("0x%016llX", v);
    return std::move(s);
}

std::string NX::conversion::to_string(short v)
{
    std::string s = NX::string_formater("%d", (int)v);
    return std::move(s);
}

std::string NX::conversion::to_string(long v)
{
    std::string s = NX::string_formater("%d", v);
    return std::move(s);
}

std::string NX::conversion::to_string(int v)
{
    std::string s = NX::string_formater("%d", v);
    return std::move(s);
}

std::string NX::conversion::to_string(__int64 v)
{
    std::string s = NX::string_formater("%I64d", v);
    return std::move(s);
}

std::string NX::conversion::to_string(float v, int precision)
{
    std::string fmt("%%.");
    fmt += to_string(precision) + "f";
    std::string s = NX::string_formater(fmt.c_str(), v);
    return std::move(s);
}

std::string NX::conversion::to_string(double v, int precision)
{
    std::string fmt("%%.");
    fmt += to_string(precision) + "f";
    std::string s = NX::string_formater(fmt.c_str(), v);
    return std::move(s);
}

std::string NX::conversion::to_string(unsigned char* v, size_t size)
{
    std::string s;
    std::for_each(v, v + size, [&](unsigned char c) {
        s += to_string(c);
    });
    return std::move(s);
}

std::string NX::conversion::to_string(const std::vector<unsigned char>& v)
{
    std::string s;
    std::for_each(v.begin(), v.end(), [&](unsigned char c) {
        s += to_string(c);
    });
    return std::move(s);
}
