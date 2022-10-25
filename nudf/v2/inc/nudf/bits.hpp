
#pragma once
#ifndef __NUDF_BITS_HPP__
#define __NUDF_BITS_HPP__

#include <vector>

namespace NX {

bool check_bit(unsigned long value, unsigned long bit);
void set_bit(unsigned long& value, unsigned long bit);
void clear_bit(unsigned long& value, unsigned long bit);

template<unsigned long N>
class bits_map
{
public:
    bits_map() throw()
    {
        _map.resize((N + 31) / 32, 0);
    }

    virtual ~bits_map() throw()
    {
    }

    bool is_bit_on(_In_ unsigned long bit) const throw()
    {
        if (bit >= N) {
            return false;
        }
        return NX::check_bit(_map[bit / 32], bit % 32);
    }

    bool operator [](_In_ unsigned long bit) const throw()
    {
        return is_bit_on(bit);
    }

    void set_bit(_In_ unsigned long bit) const throw()
    {
        if (bit < N) {
            NX::set_bit(_map[bit / 32], bit % 32);
        }        
    }

    void clear_bit(_In_ unsigned long bit) const throw()
    {
        if (bit < N) {
            NX::clear_bit(_map[bit / 32], bit % 32);
        }
    }

    void clear_all()
    {
        if (!_map.empty()) {
            memset(_map.data(), 0, _map.size()*sizeof(unsigned long));
        }
    }

    bits_map<N>& operator = (_In_ unsigned long data)
    {
        _map[0] = data;
        return *this;
    }

    bits_map<N>& operator = (_In_ int data)
    {
        _map[0] = (unsigned long)data;
        return *this;
    }

    unsigned long size() const throw() { return N; }

private:
    std::vector<unsigned long> _map;
};


} // namespace NX



#endif