

#ifndef __NXREST_XSTYPE_VERSION_HPP__
#define __NXREST_XSTYPE_VERSION_HPP__

#include <string>

#include <nudf\string.hpp>

namespace nxrmrest {
namespace schema {
namespace type {

    
class CVersion
{
public:
    CVersion() : _major(0), _minor(0), _maintenance(0), _patch(0), _build(0)
    {
    }

    virtual ~CVersion()
    {
    }

    inline int GetMajor() const throw() {return _major;}
    inline void SetMajor(int major) throw() {_major=major;}
    inline int GetMinor() const throw() {return _minor;}
    inline void SetMinor(int minor) throw() {_minor=minor;}
    inline int GetMaintenance() const throw() {return _maintenance;}
    inline void SetMaintenance(int maintenance) throw() {_maintenance=maintenance;}
    inline int GetPatch() const throw() {return _patch;}
    inline void SetPatch(int patch) throw() {_patch=patch;}
    inline int GetBuild() const throw() {return _build;}
    inline void SetBuild(int build) throw() {_build=build;}
    inline void SetVersion(int major, int minor, int maintenance, int patch, int build)
    {
        _major = major;
        _minor = minor;
        _maintenance = maintenance;
        _patch = patch;
        _build = build;
    }
    void Reset()
    {
        SetVersion(0, 0, 0, 0, 0);
    }
    CVersion& operator = (const CVersion& ver)
    {
        if(this != &ver) {
            SetVersion(ver.GetMajor(), ver.GetMinor(), ver.GetMaintenance(), ver.GetPatch(), ver.GetBuild());
        }
        return *this;
    }
    bool operator == (const CVersion& ver)
    {
        return  (_major==ver.GetMajor() && _minor==ver.GetMinor() && _maintenance==ver.GetMaintenance() && _patch==ver.GetPatch() && _build==ver.GetBuild());
    }
    bool operator > (const CVersion& ver)
    {
        if(_major != ver.GetMajor()) {
            return (_major > ver.GetMajor()) ? true : false;
        }
        if(_minor != ver.GetMinor()) {
            return (_minor > ver.GetMinor()) ? true : false;
        }
        if(_maintenance != ver.GetMaintenance()) {
            return (_maintenance > ver.GetMaintenance()) ? true : false;
        }
        if(_patch != ver.GetPatch()) {
            return (_patch > ver.GetPatch()) ? true : false;
        }        
        return (_build > ver.GetBuild()) ? true : false;
    }

    std::wstring toString() const throw()
    {
        std::wstring ver;
        ver += nudf::string::FromInt<wchar_t>(_major);
        ver += L".";
        ver += nudf::string::FromInt<wchar_t>(_minor);
        ver += L".";
        ver += nudf::string::FromInt<wchar_t>(_maintenance);
        ver += L".";
        ver += nudf::string::FromInt<wchar_t>(_patch);
        ver += L".";
        ver += nudf::string::FromInt<wchar_t>(_build);
    }
    bool fromString(const std::wstring& ver) throw()
    {
        return (5 ==swscanf_s(ver.c_str(), L"%d.%d.%d.%d.%d", &_major, &_minor, &_maintenance, &_patch, &_build));
    }

private:
    int    _major;
    int    _minor;
    int    _maintenance;
    int    _patch;
    int    _build;
};


}   // namespace nxrmrest::schema::type
}   // namespace nxrmrest::schema
}   // namespace nxrmrest


#endif  // __NXREST_XSTYPE_VERSION_HPP__