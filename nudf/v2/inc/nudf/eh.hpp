

#pragma once
#ifndef __NUDF_EH_HPP__
#define __NUDF_EH_HPP__


#include <exception>
#include <string>

namespace NX {

class exception : public std::exception
{
public:
    exception() : std::exception(), _code(0)
    {
    }

    explicit exception(const std::string& msg)
        : std::exception(msg.empty() ? nullptr : msg.c_str()), _code(GetLastError())
    {
    }

    explicit exception(const std::string& msg, long error_code)
        : std::exception(msg.empty() ? nullptr : msg.c_str()), _code(error_code)
    {
    }
    
    exception(NX::exception const& other)
        : std::exception(other), _code(0)
    {
    }

    virtual ~exception()
    {
    }

    NX::exception& operator=(NX::exception const& other)
    {
        if (this != &other) {
            std::exception::operator=(other);
            _code = other.code();
        }
        return *this;
    }

    inline long code() const { return _code; }

private:
    long   _code;
};

class structured_exception
{
public:
    structured_exception() : _code(0), _pointers(nullptr)
    {
    }
    structured_exception(unsigned int code, _EXCEPTION_POINTERS* pointers) : _code(code), _pointers(pointers)
    {
    }
    structured_exception(const structured_exception& see) : _code(see.code()), _pointers(see.exception_pointers())
    {
    }
    virtual ~structured_exception() {}

    inline unsigned int code() const noexcept { return _code; }
    inline _EXCEPTION_POINTERS* exception_pointers() const noexcept { return _pointers; }
    inline void* exception_address() const noexcept { return (nullptr == _pointers) ? nullptr : _pointers->ExceptionRecord->ExceptionAddress; }

    structured_exception& operator = (const structured_exception& other) noexcept;
    std::string exception_message() const noexcept;
    void raise() const;

    static void set_translate();

private:
    UINT _code;
    _EXCEPTION_POINTERS* _pointers;
};

#ifdef NX_STRUCTURED_EXCEPTION_ENABLED
#define ENABLE_STRUCTURED_EXCEPTION()   NX::structured_exception::set_translate()
#else
#define ENABLE_STRUCTURED_EXCEPTION()
#endif


}   // namespace NX


#endif