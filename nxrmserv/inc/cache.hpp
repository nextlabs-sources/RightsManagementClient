

#ifndef __NXSERV_CACHE_HPP__
#define __NXSERV_CACHE_HPP__

#include <Windows.h>
#include <string>
#include <map>
#include <algorithm>

namespace NX {


class exclusive_locker
{
public:
    exclusive_locker(SRWLOCK* plock) : _plock(plock)
    {
        if (NULL != _plock) {
            AcquireSRWLockExclusive(_plock);
        }
    }
    ~exclusive_locker()
    {
        if (NULL != _plock) {
            ReleaseSRWLockExclusive(_plock);
            _plock = NULL;
        }
    }

protected:
    exclusive_locker() : _plock(NULL) {}
private:
    SRWLOCK* _plock;
};
class shared_locker
{
public:
    shared_locker(SRWLOCK* plock) : _plock(plock)
    {
        if (NULL != _plock) {
            AcquireSRWLockShared(_plock);
        }
    }
    ~shared_locker()
    {
        if (NULL != _plock) {
            ReleaseSRWLockShared(_plock);
            _plock = NULL;
        }
    }

protected:
    shared_locker() : _plock(NULL) {}
private:
    SRWLOCK* _plock;
};

class rwlocker
{
public:
    rwlocker()
    {
        ::InitializeSRWLock(&_rwlock);
    }
    ~rwlocker()
    {
    }
    operator SRWLOCK* () { return &_rwlock; }
    //operator SRWLOCK& () { return _rwlock; }
private:
    SRWLOCK _rwlock;
};
// K: key's type
// T: value's type
template<typename K, typename V, unsigned long N, unsigned long S>
class cache_map
{
public:
    cache_map()
    {
        //::InitializeSRWLock(&_lock);
    }
    virtual ~cache_map()
    {
    }

    V get(const K& key)
    {
        V value;
        shared_locker(this->_lock);
        auto pos = _map.find(key);
        if (pos != _map.end()) {
            value = (*pos).second.second;
        }
        return value;
    }

    void set(const K& key, const V& value)
    {
        FILETIME ft = { 0, 0 };
        GetSystemTimeAsFileTime(&ft);
        unsigned __int64 expire_time = ft.dwHighDateTime;
        expire_time <<= 32;
        expire_time += ft.dwLowDateTime;
        expire_time += (S * 10000000);
        exclusive_locker(this->_lock);
        auto pos = _map.find(key);
        if (pos == _map.end()) {
            _map[key] = std::pair<unsigned __int64, V>(expire_time, value);
            fit_size();
        }
        else {
            (*pos).second.first = expire_time;
        }
    }

    void erase(const K& key)
    {
        exclusive_locker(this->_lock);
        _map.erase(key);
    }

    size_t size() const noexcept
    {
        shared_locker(this->_lock);
        return _map.size();
    }

    bool empty() const noexcept
    {
        shared_locker(this->_lock);
        return _map.empty();
    }

    void clear() noexcept
    {
        exclusive_locker(this->_lock);
        _map.clear();
    }
    
protected:
    void fit_size()
    {
        if (_map.size() >= N) {
            FILETIME ft = { 0, 0 };
            GetSystemTimeAsFileTime(&ft);
            unsigned __int64 current_time = ft.dwHighDateTime;
            current_time <<= 32;
            current_time += ft.dwLowDateTime;
            while (true) {
                auto pos = std::find_if(_map.begin(), _map.end(), [&](auto item) -> bool {
                    return (item.second.first >= current_time);
                });
                if (pos == _map.end()) {
                    break;
                }
                _map.erase(pos);
            }
        }
    }

private:
    std::map<K, std::pair<unsigned __int64, V>> _map;
    rwlocker                                    _lock;
};



}   // namespace NX


#endif