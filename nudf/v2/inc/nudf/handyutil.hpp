

#pragma once
#ifndef __NUDF_HANDYUTIL_HPP__
#define __NUDF_HANDYUTIL_HPP__


#include <exception>
#include <string>

namespace NX {

namespace utility {

template <typename T>
bool any_flags_on(T flags, T f)
{
    return (0 != (flags & f));
}

template <typename T>
bool flags_on(T flags, T f)
{
    return (f == (flags & f));
}

template <typename T>
void set_flags(T& flags, T f)
{
    flags |= f;
}

template <typename T>
void clear_flags(T& flags, T f)
{
    flags &= ~f;
}

#define any_flags32_on(_FS, _F)  NX::utility::any_flags_on<unsigned long>(_FS, _F)
#define flags32_on(_FS, _F)      NX::utility::flags_on<unsigned long>(_FS, _F)
#define set_flags32(_FS, _F)     NX::utility::set_flags<unsigned long>(_FS, _F)
#define clear_flags32(_FS, _F)   NX::utility::clear_flags<unsigned long>(_FS, _F)
#define any_flags64_on(_FS, _F)  NX::utility::any_flags_on<unsigned __int64>(_FS, _F)
#define flags64_on(_FS, _F)      NX::utility::flags_on<unsigned __int64>(_FS, _F)
#define set_flags64(_FS, _F)     NX::utility::set_flags<unsigned __int64>(_FS, _F)
#define clear_flags64(_FS, _F)   NX::utility::clear_flags<unsigned __int64>(_FS, _F)


template<typename T>
T round_to_size(T size, T alignment)
{
    return ((size + (alignment -1)) & ~(alignment -1));
}

template<typename T>
bool is_aligned(T size, T alignment)
{
    return ((size & (alignment - 1)) == 0);
}

#define RoundToSize32(_size, _alignment)    NX::utility::round_to_size<unsigned long>(_size, _alignment)
#define RoundToSize64(_size, _alignment)    NX::utility::round_to_size<unsigned __int64>(_size, _alignment)
#define IsAligned32(_size, _alignment)      NX::utility::is_aligned<unsigned long>(_size, _alignment)
#define IsAligned64(_size, _alignment)      NX::utility::is_aligned<unsigned __int64>(_size, _alignment)


}

}   // namespace NX


#endif