

#ifndef __NXRM_PKG_PROFILE_BASIC_HPP__
#define __NXRM_PKG_PROFILE_BASIC_HPP__

#include <string>
#include <nudf\string.hpp>

#define PROFILE_ATTRIBUTE_ID        L"id"
#define PROFILE_ATTRIBUTE_NAME      L"name"
#define PROFILE_ATTRIBUTE_TIMESTAMP L"timestamp"

namespace nxrm {
namespace pkg {


class CProfile
{
public:
    CProfile()
    {    
        _timestamp.dwHighDateTime = 0;
        _timestamp.dwLowDateTime = 0;
    }
    virtual ~CProfile(){}
    
    inline const std::wstring& GetId() const {return _id;}
    inline void SetId(const std::wstring& id) throw() {_id = id;}
    inline const std::wstring& GetName() const {return _name;}
    inline void SetName(const std::wstring& name) throw() {_name = name;}
    inline const FILETIME& GetTimestamp() const {return _timestamp;}
    inline ULONGLONG GetTimestampUll() const {return *((ULONGLONG*)(&_timestamp));}
    inline void SetTimestamp(const FILETIME& t) throw() {_timestamp = t;}
    inline void SetTimestamp(const std::wstring& st) throw()
    {
        if(!nudf::string::ToSystemTime<wchar_t>(st, &_timestamp, NULL)) {
            _timestamp.dwHighDateTime = 0;
            _timestamp.dwLowDateTime = 0;
        }
    }
    virtual void ResetTimestamp() throw()
    {
        _timestamp.dwHighDateTime = 0;
        _timestamp.dwLowDateTime = 0;
    }
    inline CProfile& operator =(const CProfile& profile) throw()
    {
        if(this != &profile) {
            _id = profile.GetId();
            _name = profile.GetName();
            _timestamp = profile.GetTimestamp();
        }
        return *this;
    }
    inline void Clear() throw()
    {
        _id.clear();
        _name.clear();
        _timestamp.dwHighDateTime = 0;
        _timestamp.dwLowDateTime = 0;
    }

private:
    std::wstring    _id;
    std::wstring    _name;
    FILETIME        _timestamp;
};

    
}   // namespace nxrm::pkg
}   // namespace nxrm


#endif  // __NXRM_PKG_PROFILE_BASIC_HPP__