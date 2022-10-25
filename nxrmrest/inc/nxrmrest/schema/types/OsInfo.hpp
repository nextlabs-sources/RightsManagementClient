

#ifndef __NXREST_XSTYPE_OSINFO_HPP__
#define __NXREST_XSTYPE_OSINFO_HPP__

#include <string>

#include <nudf\string.hpp>

namespace nxrmrest {
namespace schema {
namespace type {

    
class COsInfo
{
public:
    COsInfo()
    {
    }

    virtual ~COsInfo()
    {
    }

    inline const std::wstring& GetType() const throw() {return _type;}
    inline void SetMajor(const std::wstring& type) throw() {_type=type;}
    inline const std::wstring& GetVersion() const throw() {return _version;}
    inline void SetVersion(const std::wstring& version) throw() {_version=version;}
    void Reset()
    {
        _type = L"";
        _version = L"";
    }
    COsInfo& operator = (const COsInfo& info)
    {
        if(this != &info) {
            _type = info.GetType();
            _version = info.GetVersion();
        }
        return *this;
    }

private:
    std::wstring    _type;
    std::wstring    _version;
};


}   // namespace nxrmrest::schema::type
}   // namespace nxrmrest::schema
}   // namespace nxrmrest


#endif  // __NXREST_XSTYPE_OSINFO_HPP__