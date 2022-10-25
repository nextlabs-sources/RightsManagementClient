

#ifndef __NX_SE_EXCEPTION_HPP__
#define __NX_SE_EXCEPTION_HPP__

#include <Windows.h>
#include <string>
#include <vector>

namespace NX {


class structured_exception
{
    template <typename T>
    class buffer_string
    {
    public:
        buffer_string(std::basic_string<T>& str, size_t len) : _s(str)
        {
            // ctor
            _buf.resize(len + 1, 0);
        }
        ~buffer_string()
        {
            _s = std::basic_string<T>(&_buf[0]);      // copy to string passed by ref at construction
        }

        // auto conversion to serve as windows function parameter
        inline operator T* () throw() { return (&_buf[0]); }

    private:
        // No copy allowed
        buffer_string(buffer_string<T>& c) {}
        // No assignment allowed
        buffer_string& operator= (const buffer_string<T>& c) { return *this; }

    private:
        std::basic_string<T>&   _s;
        std::vector<T>          _buf;
    };

public:
    structured_exception() : _code(0), _pointers(NULL)
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
    inline void* exception_address() const noexcept { return (NULL == _pointers) ? NULL : _pointers->ExceptionRecord->ExceptionAddress; }

    structured_exception& operator = (const structured_exception& other) noexcept;
    std::wstring exception_message() const noexcept;
    void raise() const;

    void dump() noexcept;
    void dump(const std::wstring& file) noexcept;
    static void set_translate();

private:
    UINT _code;
    _EXCEPTION_POINTERS* _pointers;

};


}   // namespace NX

#define SET_SE_TRANSLATE()  

#endif