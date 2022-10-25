
#pragma once
#ifndef __NUDF_DEBUG_HPP__
#define __NUDF_DEBUG_HPP__

#include <string>


namespace NX {

namespace dbg {


std::string build_error_msg(const std::string& category, const std::string format, ...);
std::string build_error_msg(const std::string& file_name, const std::string& function_name, int line, const std::string& category, const std::string format, ...);

std::string translate_win32_error(int err, const char* source = nullptr);
std::string translate_winhttp_error(int err, const char* source = nullptr);

void dump(const std::wstring& file, _EXCEPTION_POINTERS* eps, bool full = true) noexcept;


#define ERROR_MSG(c, m, ...)    NX::dbg::build_error_msg(c, m, __VA_ARGS__)
#define ERROR_MSG2(c, m, ...)   NX::dbg::build_error_msg(__FILE__, __FUNCTION__, __LINE__, c, m, __VA_ARGS__)


#define NT_STATUS_MSG(status, msg)  NX::dbg::build_error_msg("Win32", "NT Status (%08X) at %s", status, msg)
#define NT_STATUS_MSG2(status)      NX::dbg::build_error_msg(__FILE__, __FUNCTION__, __LINE__, "Win32", "NT Status (%08X)", status)

#define HRESULT_MSG(hr, msg)        NX::dbg::build_error_msg("Win32", "HRESULT (%08X) at %s", hr, msg)
#define HRESULT_MSG2(hr)            NX::dbg::build_error_msg(__FILE__, __FUNCTION__, __LINE__, "Win32", "HRESULT (%08X)", hr)

#ifndef NX_USE_WINHTTP_ERROR
#define WIN32_ERROR_MSG(err, msg)   NX::dbg::build_error_msg("Win32", NX::dbg::translate_win32_error(err, msg))
#define WIN32_ERROR_MSG2(err)       NX::dbg::build_error_msg(__FILE__, __FUNCTION__, __LINE__, "Win32", NX::dbg::translate_win32_error(err, nullptr))
#else
#define WIN32_ERROR_MSG(err, msg)   NX::dbg::build_error_msg("Win32", NX::dbg::translate_winhttp_error(err, msg))
#define WIN32_ERROR_MSG2(err)       NX::dbg::build_error_msg(__FILE__, __FUNCTION__, __LINE__, "Win32", NX::dbg::translate_winhttp_error(err, nullptr))
#endif

}

}


#endif