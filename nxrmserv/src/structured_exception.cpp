
#include <Windows.h>
#include <eh.h>

#pragma warning(push)
#pragma warning(disable: 4091)
#include <imagehlp.h>
#pragma warning(pop)

#include "nxrmserv.h"
#include "structured_exception.hpp"


using namespace NX;

static void translate(unsigned int code, _EXCEPTION_POINTERS* pointers);

NX::structured_exception& NX::structured_exception::operator = (const NX::structured_exception& other) noexcept
{
    if (this != &other) {
        _code = other.code();
        _pointers = other.exception_pointers();
    }
    return *this;
}

void NX::structured_exception::raise() const
{
    RaiseException(999, EXCEPTION_NONCONTINUABLE, 0, NULL);
}

std::wstring NX::structured_exception::exception_message() const noexcept
{
    static const std::wstring fmt(L"Exception %s (0x%.8X) at address 0x%p");
    std::wstring    msg;

    switch (_code)
    {
    case 0:
        break;
    case EXCEPTION_ACCESS_VIOLATION:
        swprintf_s(buffer_string<wchar_t>(msg, MAX_PATH), MAX_PATH, L"%s (0x%.8X) at address 0x%p", L"EXCEPTION_ACCESS_VIOLATION", _code, exception_address());
        break;
    case EXCEPTION_DATATYPE_MISALIGNMENT:
        swprintf_s(buffer_string<wchar_t>(msg, MAX_PATH), MAX_PATH, L"%s (0x%.8X) at address 0x%p", L"EXCEPTION_DATATYPE_MISALIGNMENT", _code, exception_address());
        break;
    case EXCEPTION_BREAKPOINT:
        swprintf_s(buffer_string<wchar_t>(msg, MAX_PATH), MAX_PATH, L"%s (0x%.8X) at address 0x%p", L"EXCEPTION_BREAKPOINT", _code, exception_address());
        break;
    case EXCEPTION_SINGLE_STEP:
        swprintf_s(buffer_string<wchar_t>(msg, MAX_PATH), MAX_PATH, L"%s (0x%.8X) at address 0x%p", L"EXCEPTION_SINGLE_STEP", _code, exception_address());
        break;
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
        swprintf_s(buffer_string<wchar_t>(msg, MAX_PATH), MAX_PATH, L"%s (0x%.8X) at address 0x%p", L"EXCEPTION_ARRAY_BOUNDS_EXCEEDED", _code, exception_address());
        break;
    case EXCEPTION_FLT_DENORMAL_OPERAND:
        swprintf_s(buffer_string<wchar_t>(msg, MAX_PATH), MAX_PATH, L"%s (0x%.8X) at address 0x%p", L"EXCEPTION_FLT_DENORMAL_OPERAND", _code, exception_address());
        break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        swprintf_s(buffer_string<wchar_t>(msg, MAX_PATH), MAX_PATH, L"%s (0x%.8X) at address 0x%p", L"EXCEPTION_FLT_DIVIDE_BY_ZERO", _code, exception_address());
        break;
    case EXCEPTION_FLT_INEXACT_RESULT:
        swprintf_s(buffer_string<wchar_t>(msg, MAX_PATH), MAX_PATH, L"%s (0x%.8X) at address 0x%p", L"EXCEPTION_FLT_INEXACT_RESULT", _code, exception_address());
        break;
    case EXCEPTION_FLT_INVALID_OPERATION:
        swprintf_s(buffer_string<wchar_t>(msg, MAX_PATH), MAX_PATH, L"%s (0x%.8X) at address 0x%p", L"EXCEPTION_FLT_INVALID_OPERATION", _code, exception_address());
        break;
    case EXCEPTION_FLT_OVERFLOW:
        swprintf_s(buffer_string<wchar_t>(msg, MAX_PATH), MAX_PATH, L"%s (0x%.8X) at address 0x%p", L"EXCEPTION_FLT_OVERFLOW", _code, exception_address());
        break;
    case EXCEPTION_FLT_STACK_CHECK:
        swprintf_s(buffer_string<wchar_t>(msg, MAX_PATH), MAX_PATH, L"%s (0x%.8X) at address 0x%p", L"EXCEPTION_FLT_STACK_CHECK", _code, exception_address());
        break;
    case EXCEPTION_FLT_UNDERFLOW:
        swprintf_s(buffer_string<wchar_t>(msg, MAX_PATH), MAX_PATH, L"%s (0x%.8X) at address 0x%p", L"EXCEPTION_FLT_UNDERFLOW", _code, exception_address());
        break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        swprintf_s(buffer_string<wchar_t>(msg, MAX_PATH), MAX_PATH, L"%s (0x%.8X) at address 0x%p", L"EXCEPTION_INT_DIVIDE_BY_ZERO", _code, exception_address());
        break;
    case EXCEPTION_INT_OVERFLOW:
        swprintf_s(buffer_string<wchar_t>(msg, MAX_PATH), MAX_PATH, L"%s (0x%.8X) at address 0x%p", L"EXCEPTION_INT_OVERFLOW", _code, exception_address());
        break;
    case EXCEPTION_PRIV_INSTRUCTION:
        swprintf_s(buffer_string<wchar_t>(msg, MAX_PATH), MAX_PATH, L"%s (0x%.8X) at address 0x%p", L"EXCEPTION_PRIV_INSTRUCTION", _code, exception_address());
        break;
    case EXCEPTION_IN_PAGE_ERROR:
        swprintf_s(buffer_string<wchar_t>(msg, MAX_PATH), MAX_PATH, L"%s (0x%.8X) at address 0x%p", L"EXCEPTION_IN_PAGE_ERROR", _code, exception_address());
        break;
    case EXCEPTION_ILLEGAL_INSTRUCTION:
        swprintf_s(buffer_string<wchar_t>(msg, MAX_PATH), MAX_PATH, L"%s (0x%.8X) at address 0x%p", L"EXCEPTION_ILLEGAL_INSTRUCTION", _code, exception_address());
        break;
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
        swprintf_s(buffer_string<wchar_t>(msg, MAX_PATH), MAX_PATH, L"%s (0x%.8X) at address 0x%p", L"EXCEPTION_NONCONTINUABLE_EXCEPTION", _code, exception_address());
        break;
    case EXCEPTION_STACK_OVERFLOW:
        swprintf_s(buffer_string<wchar_t>(msg, MAX_PATH), MAX_PATH, L"%s (0x%.8X) at address 0x%p", L"EXCEPTION_STACK_OVERFLOW", _code, exception_address());
        break;
    case EXCEPTION_INVALID_DISPOSITION:
        swprintf_s(buffer_string<wchar_t>(msg, MAX_PATH), MAX_PATH, L"%s (0x%.8X) at address 0x%p", L"EXCEPTION_INVALID_DISPOSITION", _code, exception_address());
        break;
    case EXCEPTION_GUARD_PAGE:
        swprintf_s(buffer_string<wchar_t>(msg, MAX_PATH), MAX_PATH, L"%s (0x%.8X) at address 0x%p", L"EXCEPTION_GUARD_PAGE", _code, exception_address());
        break;
    case EXCEPTION_INVALID_HANDLE:
        swprintf_s(buffer_string<wchar_t>(msg, MAX_PATH), MAX_PATH, L"%s (0x%.8X) at address 0x%p", L"EXCEPTION_INVALID_HANDLE", _code, exception_address());
        break;
    default:
        swprintf_s(buffer_string<wchar_t>(msg, MAX_PATH), MAX_PATH, L"%s (0x%.8X) at address 0x%p", L"Unknown exception", _code, exception_address());
        break;
    }

    return std::move(msg);
}

void NX::structured_exception::dump() noexcept
{
    std::wstring file(GLOBAL.dir_root());
    std::wstring name;
    SYSTEMTIME st = { 0 };
    GetLocalTime(&st);
    swprintf_s(buffer_string<wchar_t>(name, MAX_PATH), MAX_PATH, L"nxrmserv-%04d%02d%02d%02d%02d%02d%03d.dmp", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    if (file.empty()) {
        file = name;
    }
    else {
        file += L"\\" + name;
    }
    dump(file);
}

void NX::structured_exception::dump(const std::wstring& file) noexcept
{
    /* Prototype MiniDumpWriteDump @ dbghelp.dll for dynamic load */
    typedef BOOL(WINAPI *MiniDumpWriteDump_fn_t)(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE, PMINIDUMP_EXCEPTION_INFORMATION, PMINIDUMP_USER_STREAM_INFORMATION, PMINIDUMP_CALLBACK_INFORMATION);
    static bool initialized = false;
    static MiniDumpWriteDump_fn_t   MiniDumpWriteDump_fn = NULL;
    static const unsigned int MiniDumpType = MiniDumpWithDataSegs |
                                             MiniDumpWithHandleData |
                                             MiniDumpWithHandleData |
                                             MiniDumpWithProcessThreadData |
                                             MiniDumpWithFullMemory;

    if (NULL == _pointers) {
        return;
    }

    if (!initialized) {
        initialized = true;
        HMODULE hlib_dbghelp = LoadLibraryW(L"dbghelp.dll");
        if (hlib_dbghelp == NULL) {
            return;
        }
        /* MiniDumpWriteDump @ dbghelp.dll */
        MiniDumpWriteDump_fn = (MiniDumpWriteDump_fn_t)GetProcAddress(hlib_dbghelp, "MiniDumpWriteDump");
    }
    if (NULL == MiniDumpWriteDump_fn) {
        return;
    }

    HANDLE ph = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
    if (ph == NULL) {
        return;
    }

    HANDLE fh = CreateFileW(file.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fh == INVALID_HANDLE_VALUE) {
        CloseHandle(ph);
        ph = NULL;
        return;
    }

    MINIDUMP_EXCEPTION_INFORMATION mn_exception_info;
    mn_exception_info.ThreadId = GetCurrentThreadId();
    mn_exception_info.ExceptionPointers = _pointers;
    mn_exception_info.ClientPointers = FALSE;
    // generate a dump file
    MiniDumpWriteDump_fn(ph, GetCurrentProcessId(), fh, (MINIDUMP_TYPE)MiniDumpType, &mn_exception_info, NULL, NULL);

    // done
    CloseHandle(ph);
    ph = NULL;
    CloseHandle(fh);
    fh = INVALID_HANDLE_VALUE;
}

void NX::structured_exception::set_translate()
{
    static bool first_time = true;
    if (first_time) {
        _set_se_translator(translate);
        first_time = false;
    }
}

void translate(unsigned int code, _EXCEPTION_POINTERS* pointers)
{
    bool handle = false;

    switch (code)
    {
    case EXCEPTION_BREAKPOINT:
    case EXCEPTION_SINGLE_STEP:
    case 999:   // this is from us, ignore it
        break;
    case EXCEPTION_ACCESS_VIOLATION:
    case EXCEPTION_DATATYPE_MISALIGNMENT:
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
    case EXCEPTION_FLT_DENORMAL_OPERAND:
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
    case EXCEPTION_FLT_INEXACT_RESULT:
    case EXCEPTION_FLT_INVALID_OPERATION:
    case EXCEPTION_FLT_OVERFLOW:
    case EXCEPTION_FLT_STACK_CHECK:
    case EXCEPTION_FLT_UNDERFLOW:
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
    case EXCEPTION_INT_OVERFLOW:
    case EXCEPTION_PRIV_INSTRUCTION:
    case EXCEPTION_IN_PAGE_ERROR:
    case EXCEPTION_ILLEGAL_INSTRUCTION:
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
    case EXCEPTION_STACK_OVERFLOW:
    case EXCEPTION_INVALID_DISPOSITION:
    case EXCEPTION_GUARD_PAGE:
    case EXCEPTION_INVALID_HANDLE:
    default:
        handle = true;
        break;
    }

    if (handle) {
        throw NX::structured_exception(code, pointers);
    }
}