

#include <Windows.h>

#include <nudf\string.hpp>
#include <nudf\eh.hpp>


#define EXCEPTION_NEXTLABS_EH   0x00009999


static void translate(unsigned int code, _EXCEPTION_POINTERS* pointers);


NX::structured_exception& NX::structured_exception::operator = (const structured_exception& other) noexcept
{
    if (this != &other) {
        _code = other.code();
        _pointers = other.exception_pointers();
    }
    return *this;
}

std::string NX::structured_exception::exception_message() const noexcept
{
    std::string  msg;

    switch (_code)
    {
    case 0:
        break;
    case EXCEPTION_ACCESS_VIOLATION:
        sprintf_s(NX::string_buffer<char>(msg, MAX_PATH), MAX_PATH, "%s (0x%.8X) at address 0x%p", "EXCEPTION_ACCESS_VIOLATION", _code, exception_address());
        break;
    case EXCEPTION_DATATYPE_MISALIGNMENT:
        sprintf_s(NX::string_buffer<char>(msg, MAX_PATH), MAX_PATH, "%s (0x%.8X) at address 0x%p", "EXCEPTION_DATATYPE_MISALIGNMENT", _code, exception_address());
        break;
    case EXCEPTION_BREAKPOINT:
        sprintf_s(NX::string_buffer<char>(msg, MAX_PATH), MAX_PATH, "%s (0x%.8X) at address 0x%p", "EXCEPTION_BREAKPOINT", _code, exception_address());
        break;
    case EXCEPTION_SINGLE_STEP:
        sprintf_s(NX::string_buffer<char>(msg, MAX_PATH), MAX_PATH, "%s (0x%.8X) at address 0x%p", "EXCEPTION_SINGLE_STEP", _code, exception_address());
        break;
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
        sprintf_s(NX::string_buffer<char>(msg, MAX_PATH), MAX_PATH, "%s (0x%.8X) at address 0x%p", "EXCEPTION_ARRAY_BOUNDS_EXCEEDED", _code, exception_address());
        break;
    case EXCEPTION_FLT_DENORMAL_OPERAND:
        sprintf_s(NX::string_buffer<char>(msg, MAX_PATH), MAX_PATH, "%s (0x%.8X) at address 0x%p", "EXCEPTION_FLT_DENORMAL_OPERAND", _code, exception_address());
        break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        sprintf_s(NX::string_buffer<char>(msg, MAX_PATH), MAX_PATH, "%s (0x%.8X) at address 0x%p", "EXCEPTION_FLT_DIVIDE_BY_ZERO", _code, exception_address());
        break;
    case EXCEPTION_FLT_INEXACT_RESULT:
        sprintf_s(NX::string_buffer<char>(msg, MAX_PATH), MAX_PATH, "%s (0x%.8X) at address 0x%p", "EXCEPTION_FLT_INEXACT_RESULT", _code, exception_address());
        break;
    case EXCEPTION_FLT_INVALID_OPERATION:
        sprintf_s(NX::string_buffer<char>(msg, MAX_PATH), MAX_PATH, "%s (0x%.8X) at address 0x%p", "EXCEPTION_FLT_INVALID_OPERATION", _code, exception_address());
        break;
    case EXCEPTION_FLT_OVERFLOW:
        sprintf_s(NX::string_buffer<char>(msg, MAX_PATH), MAX_PATH, "%s (0x%.8X) at address 0x%p", "EXCEPTION_FLT_OVERFLOW", _code, exception_address());
        break;
    case EXCEPTION_FLT_STACK_CHECK:
        sprintf_s(NX::string_buffer<char>(msg, MAX_PATH), MAX_PATH, "%s (0x%.8X) at address 0x%p", "EXCEPTION_FLT_STACK_CHECK", _code, exception_address());
        break;
    case EXCEPTION_FLT_UNDERFLOW:
        sprintf_s(NX::string_buffer<char>(msg, MAX_PATH), MAX_PATH, "%s (0x%.8X) at address 0x%p", "EXCEPTION_FLT_UNDERFLOW", _code, exception_address());
        break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        sprintf_s(NX::string_buffer<char>(msg, MAX_PATH), MAX_PATH, "%s (0x%.8X) at address 0x%p", "EXCEPTION_INT_DIVIDE_BY_ZERO", _code, exception_address());
        break;
    case EXCEPTION_INT_OVERFLOW:
        sprintf_s(NX::string_buffer<char>(msg, MAX_PATH), MAX_PATH, "%s (0x%.8X) at address 0x%p", "EXCEPTION_INT_OVERFLOW", _code, exception_address());
        break;
    case EXCEPTION_PRIV_INSTRUCTION:
        sprintf_s(NX::string_buffer<char>(msg, MAX_PATH), MAX_PATH, "%s (0x%.8X) at address 0x%p", "EXCEPTION_PRIV_INSTRUCTION", _code, exception_address());
        break;
    case EXCEPTION_IN_PAGE_ERROR:
        sprintf_s(NX::string_buffer<char>(msg, MAX_PATH), MAX_PATH, "%s (0x%.8X) at address 0x%p", "EXCEPTION_IN_PAGE_ERROR", _code, exception_address());
        break;
    case EXCEPTION_ILLEGAL_INSTRUCTION:
        sprintf_s(NX::string_buffer<char>(msg, MAX_PATH), MAX_PATH, "%s (0x%.8X) at address 0x%p", "EXCEPTION_ILLEGAL_INSTRUCTION", _code, exception_address());
        break;
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
        sprintf_s(NX::string_buffer<char>(msg, MAX_PATH), MAX_PATH, "%s (0x%.8X) at address 0x%p", "EXCEPTION_NONCONTINUABLE_EXCEPTION", _code, exception_address());
        break;
    case EXCEPTION_STACK_OVERFLOW:
        sprintf_s(NX::string_buffer<char>(msg, MAX_PATH), MAX_PATH, "%s (0x%.8X) at address 0x%p", "EXCEPTION_STACK_OVERFLOW", _code, exception_address());
        break;
    case EXCEPTION_INVALID_DISPOSITION:
        sprintf_s(NX::string_buffer<char>(msg, MAX_PATH), MAX_PATH, "%s (0x%.8X) at address 0x%p", "EXCEPTION_INVALID_DISPOSITION", _code, exception_address());
        break;
    case EXCEPTION_GUARD_PAGE:
        sprintf_s(NX::string_buffer<char>(msg, MAX_PATH), MAX_PATH, "%s (0x%.8X) at address 0x%p", "EXCEPTION_GUARD_PAGE", _code, exception_address());
        break;
    case EXCEPTION_INVALID_HANDLE:
        sprintf_s(NX::string_buffer<char>(msg, MAX_PATH), MAX_PATH, "%s (0x%.8X) at address 0x%p", "EXCEPTION_INVALID_HANDLE", _code, exception_address());
        break;
    default:
        sprintf_s(NX::string_buffer<char>(msg, MAX_PATH), MAX_PATH, "%s (0x%.8X) at address 0x%p", "Unknown exception", _code, exception_address());
        break;
    }

    return std::move(msg);
}

void NX::structured_exception::raise() const
{
    RaiseException(EXCEPTION_NEXTLABS_EH, EXCEPTION_NONCONTINUABLE, 0, NULL);
}

void NX::structured_exception::set_translate()
{
    static bool first_time = true;
    if (first_time) {
        _set_se_translator(translate);
        first_time = false;
    }
}


//
//  static translator
//
void translate(unsigned int code, _EXCEPTION_POINTERS* pointers)
{
    bool handle = false;

    switch (code)
    {
    case EXCEPTION_BREAKPOINT:
    case EXCEPTION_SINGLE_STEP:
    case EXCEPTION_NEXTLABS_EH:   // this is from us, ignore it
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
