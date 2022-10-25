

#pragma once
#ifndef __NUDF_STRING_HPP__
#define __NUDF_STRING_HPP__



#include <exception>
#include <string>
#include <vector>

namespace NX {

template <typename T>
class string_buffer
{
public:
    string_buffer(std::basic_string<T>& str, size_t len) : _s(str)
    {
        // ctor
        _buf.resize(len + 1, 0);
    }


    ~string_buffer()
    {
        _s = std::basic_string<T>(_buf.data());      // copy to string passed by ref at construction
    }

    // auto conversion to serve as windows function parameter
    inline operator T* () throw() { return _buf.data(); }

private:
    // No copy allowed
    string_buffer(const string_buffer<T>& c) {}
    // No assignment allowed
    string_buffer& operator= (const string_buffer<T>& c) { return *this; }

private:
    std::basic_string<T>&   _s;
    std::vector<T>          _buf;
};

std::string string_formater(const char* format, ...);
std::wstring string_formater(const wchar_t* format, ...);

namespace utility {
    

template<typename CharType>
bool is_digit(CharType ch)
{
    return (ch >= CharType('0') && ch <= CharType('9'));
}

template<typename CharType>
bool is_hex(CharType ch)
{
    ch = (CharType)tolower((int)ch);
    return (is_digit<CharType>(ch) || (ch >= CharType('a') && ch <= CharType('f')));
}

template<typename CharType>
unsigned char hex_to_uchar(CharType c)
{
    if (c >= '0' && c <= '9') {
        return (c - '0');
    }
    else if (c >= 'a' && c <= 'f') {
        return (c - 'a' + 10);
    }
    else if (c >= 'A' && c <= 'F') {
        return (c - 'A' + 10);
    }
    else {
        ; // assert(false);
    }
    return 0;
}

template<typename CharType>
int hex_to_int(CharType c)
{
    if (c >= '0' && c <= '9') {
        return (c - '0');
    }
    else if (c >= 'a' && c <= 'f') {
        return (c - 'a' + 10);
    }
    else if (c >= 'A' && c <= 'F') {
        return (c - 'A' + 10);
    }
    else {
        ; // assert(false);
    }
    return 0;
}

template<typename CharType>
bool is_alphabet(CharType ch)
{
    ch = (CharType)tolower((int)ch);
    return (ch >= CharType('a') && ch <= CharType('z'));
}

template<typename CharType>
bool iequal(CharType c1, CharType c2)
{
    return (tolower(c1) == tolower(c2));
}

template<typename CharType>
bool iequal(const CharType* s1, const CharType* s2)
{
    do {
        if (!iequal<CharType>(*s1, *s2)) {
            return false;
        }
    } while ((*(s1++)) != 0 && (*(s2++)) != 0);
    return true;
}

template<typename CharType>
bool iequal(const std::basic_string<CharType>& s1, const std::basic_string<CharType>& s2)
{
    return iequal<CharType>(s1.c_str(), s2.c_str());
}

template<typename CharType>
int icompare(CharType c1, CharType c2)
{
    c1 = tolower(c1);
    c2 = tolower(c2);
    return ((c1 == c2) ? 0 : (c1 > c2 ?  1 : (-1)));
}

template<typename CharType>
int icompare(const CharType* s1, const CharType* s2)
{
    int result = 0;
    do {
        result = icompare<CharType>(*s1, *s2);
    } while (0 == result && (*(s1++)) != 0 && (*(s2++)) != 0);
    return result;
}

template<typename CharType>
int icompare(const std::basic_string<CharType>& s1, const std::basic_string<CharType>& s2)
{
    return icompare<CharType>(s1.c_str(), s2.c_str());
}


template <typename CharType>
std::vector<unsigned char> hex_string_to_buffer(const std::basic_string<CharType>& s)
{
    std::vector<unsigned char> buf;
    const size_t count = s.length() / 2;
    for (int i = 0; i < (int)count; i++) {
        unsigned char ch = hex_to_uchar(s[2 * i]);
        ch <<= 4;
        ch |= hex_to_uchar(s[2 * i + 1]);
        buf.push_back(ch);
    }
    return std::move(buf);
}


}

}

#endif