
#pragma once
#ifndef __NUDF_SECURITY_HPP__
#define __NUDF_SECURITY_HPP__

#include <string>
#include <vector>

namespace NX {

namespace secure {



class smemory
{
public:
    smemory();
    smemory(const unsigned char* data, size_t length);
    smemory(const std::vector<unsigned char>& data);
    ~smemory();

    smemory& operator = (const smemory& other) noexcept;
    virtual void encrypt(const std::vector<unsigned char>& data);
    virtual std::vector<unsigned char> decrypt() const;
    void clear() noexcept;


    inline size_t size() const noexcept { return _size; }
    inline bool empty() const noexcept { return (0 == _size); }

private:
    std::vector<unsigned char> _buffer;
    size_t _size;
};

template<typename CharType>
class basic_sstring : public smemory
{
public:
    basic_sstring() : smemory()
    {
    }
    basic_sstring(const std::basic_string<CharType>& s) : smemory((unsigned char*)s.c_str(), s.length() * sizeof(CharType))
    {
    }
    ~basic_sstring()
    {
    }

    basic_sstring<CharType>& operator = (const basic_sstring<CharType>& other) noexcept
    {
        if (this != &other) {
            clear();
            encrypt(other.decrypt());
        }
        return *this;
    }

    virtual void encrypt(const std::basic_string<CharType>& s)
    {
        if (!s.empty()) {
            const unsigned char* p = (const unsigned char*)s.c_str();
            const size_t n = s.length() * sizeof(CharType);
            smemory::encrypt(std::vector<unsigned char>(p, p + n));
        }
    }

    virtual std::basic_string<CharType> decrypt_string() const
    {
        const std::vector<unsigned char>& v = smemory::decrypt();
        const size_t n = v.size() / sizeof(CharType);
        if (0 == n) {
            return std::basic_string<CharType>();
        }
        const CharType* p = (const CharType*)v.data();
        return std::basic_string<CharType>(p, p + n);
    }

    inline size_t lengtgh() const noexcept { return (size() / sizeof(CharType)); }

private:
    virtual void encrypt(const std::vector<unsigned char>& data) { throw std::exception("not impl"); }
    virtual std::vector<unsigned char> decrypt() const { throw std::exception("not impl"); }
};

typedef basic_sstring<char>     sstring;
typedef basic_sstring<wchar_t>  wsstring;


}


}



#endif