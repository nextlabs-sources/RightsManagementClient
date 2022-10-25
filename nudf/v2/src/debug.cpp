

#include <Windows.h>
#include <stdio.h>

#pragma warning(push)
#pragma warning(disable: 4091)
#include <imagehlp.h>
#pragma warning(pop)

#include <nudf\string.hpp>
#include <nudf\conversion.hpp>
#include <nudf\debug.hpp>


__forceinline std::string get_file_name(const std::string& file_name)
{
    if (file_name.empty()) {
        return "Unknown File";
    }
    else {
        auto pos = file_name.find_last_of('\\');
        return (pos == std::string::npos) ? file_name : file_name.substr(pos + 1);
    }
}

std::string NX::dbg::build_error_msg(const std::string& category, const std::string format, ...)
{
    std::string s;
    
    if (!category.empty()) {
        s += category;
        s += ": ";
    }

    if (!format.empty()) {
        va_list args;
        int     len = 0;
        std::string s2;
        va_start(args, format);
        len = _vscprintf_l(format.c_str(), 0, args) + 1;
        vsprintf_s(NX::string_buffer<char>(s2, len), len, format.c_str(), args);
        va_end(args);
        s += " ";
        s += s2;
    }

    return std::move(s);
}

std::string NX::dbg::build_error_msg(const std::string& file_name, const std::string& function_name, int line, const std::string& category, const std::string format, ...)
{
    std::string s;

    if (!category.empty()) {
        s += category;
        s += ": ";
    }

    if (!format.empty()) {
        va_list args;
        std::string s2;
        int     len = 0;
        va_start(args, format);
        len = _vscprintf_l(format.c_str(), 0, args) + 1;
        vsprintf_s(NX::string_buffer<char>(s2, len), len, format.c_str(), args);
        va_end(args);
        s += s2;
    }

    if (!s.empty()) {
        const std::string& real_file_name = get_file_name(file_name);
        s += NX::string_formater(" (%s, %s, %d)", function_name.c_str(), real_file_name.c_str(), line);
    }

    return std::move(s);
}

std::string NX::dbg::translate_winhttp_error(int err, const char* source)
{
    std::string s;
    if (err >= 12000) {
        static HMODULE mod = GetModuleHandleA("winhttp.dll"); // this handle DOES NOT need to be freed
        if (0 == ::FormatMessageA(FORMAT_MESSAGE_FROM_HMODULE, mod, err, 0, NX::string_buffer<char>(s, 1024), 1024, NULL)) {
            s = "Unknown error";
            if (nullptr != source && 0 != source[0]) {
                s += " (";
                s += source;
                s += ")";
            }
        }
    }
    else {
        s = NX::dbg::translate_win32_error(err, source);
    }
    return std::move(s);
}

std::string NX::dbg::translate_win32_error(int err, const char* source)
{
    std::string s;
    if (0 == ::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, NX::string_buffer<char>(s, 1024), 1024, NULL)) {
        s = "Unknown error";
    }
    if (nullptr != source && 0 != source[0]) {
        s += " (";
        s += source;
        s += ")";
    }
    return std::move(s);
}

void NX::dbg::dump(const std::wstring& file, _EXCEPTION_POINTERS* eps, bool full) noexcept
{
    /* Prototype MiniDumpWriteDump @ dbghelp.dll for dynamic load */
    typedef BOOL(WINAPI *MiniDumpWriteDump_fn_t)(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE, PMINIDUMP_EXCEPTION_INFORMATION, PMINIDUMP_USER_STREAM_INFORMATION, PMINIDUMP_CALLBACK_INFORMATION);
    static const unsigned int MiniDumpMini = MiniDumpWithDataSegs | MiniDumpWithHandleData | MiniDumpWithHandleData | MiniDumpWithProcessThreadData;
    static const unsigned int MiniDumpFull = MiniDumpWithDataSegs | MiniDumpWithHandleData | MiniDumpWithHandleData | MiniDumpWithProcessThreadData | MiniDumpWithFullMemory;

    HMODULE hlib_dbghelp = NULL;
    MiniDumpWriteDump_fn_t   MiniDumpWriteDump_fn = NULL;
    HANDLE ph = NULL;
    HANDLE fh = INVALID_HANDLE_VALUE;

    try {

        hlib_dbghelp = LoadLibraryW(L"dbghelp.dll");
        if (hlib_dbghelp == NULL) {
            throw std::exception("fail to load dbghelp.dll");
        }

        /* MiniDumpWriteDump @ dbghelp.dll */
        MiniDumpWriteDump_fn = (MiniDumpWriteDump_fn_t)GetProcAddress(hlib_dbghelp, "MiniDumpWriteDump");
        if (NULL == MiniDumpWriteDump_fn) {
            throw std::exception("fail to get function MiniDumpWriteDump");
        }

        ph = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
        if (ph == NULL) {
            throw std::exception("fail to open process");
        }

        fh = CreateFileW(file.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (fh == INVALID_HANDLE_VALUE) {
            throw std::exception("fail to open dump file");
        }

        MINIDUMP_EXCEPTION_INFORMATION mdei;
        mdei.ThreadId = GetCurrentThreadId();
        mdei.ExceptionPointers = eps;
        mdei.ClientPointers = FALSE;
        // generate a dump file
        MiniDumpWriteDump_fn(ph, GetCurrentProcessId(), fh, (MINIDUMP_TYPE)(full ? MiniDumpFull : MiniDumpMini), (NULL == eps) ? NULL : (&mdei), NULL, NULL);

        // finished
        MiniDumpWriteDump_fn = NULL;
        FreeLibrary(hlib_dbghelp);
        hlib_dbghelp = NULL;
        CloseHandle(ph);
        ph = NULL;
        CloseHandle(fh);
        fh = INVALID_HANDLE_VALUE;
    }
    catch (const std::exception& e) {

        UNREFERENCED_PARAMETER(e);

        MiniDumpWriteDump_fn = NULL;
        if (NULL != hlib_dbghelp) {
            FreeLibrary(hlib_dbghelp);
            hlib_dbghelp = NULL;
        }
        if (NULL != ph) {
            CloseHandle(ph);
            ph = NULL;
        }
        if (NULL != fh) {
            CloseHandle(fh);
            fh = INVALID_HANDLE_VALUE;
        }
    }
}