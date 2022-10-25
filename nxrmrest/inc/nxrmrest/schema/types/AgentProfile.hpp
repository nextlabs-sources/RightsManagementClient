

#ifndef __NXREST_XSTYPE_AGENT_PROFILE_HPP__
#define __NXREST_XSTYPE_AGENT_PROFILE_HPP__


#include <string>

#include <nxrmrest\schema\types\Version.hpp>
#include <nxrmrest\schema\types\OsInfo.hpp>
#include <nxrmrest\schema\types\HwInfo.hpp>
#include <nxrmrest\schema\types\AgentType.hpp>
#include <nxrmrest\schema\types\CommonFaults.hpp>



namespace nxrmrest {
namespace schema {
namespace type {

        
class CAgentProfile
{
public:
    CAgentProfile() : _default(false)
    {
        _createtime.dwHighDateTime = 0;
        _createtime.dwLowDateTime = 0;
        _modifytime.dwHighDateTime = 0;
        _modifytime.dwLowDateTime = 0;
    }
    virtual ~CAgentProfile()
    {
    }

    inline const std::wstring& GetProfileId() const throw() {return _id;}
    inline void SetProfileId(const std::wstring& id) throw() {_id = id;}
    inline const std::wstring& GetProfileName() const throw() {return _name;}
    inline void SetProfileName(const std::wstring& name) throw() {_name = name;}
    inline FILETIME GetCreateTime() const throw() {return _createtime;}
    inline void SetCreateTime(FILETIME time) throw() {_createtime = time;}
    inline FILETIME GetModifyTime() const throw() {return _modifytime;}
    inline void SetModifyTime(FILETIME time) throw() {_modifytime = time;}
    inline bool GetDefaultFlag() const throw() {return _default;}
    inline void SetDefaultFlag(bool set) throw() {_default = set;}

    CAgentProfile& operator = (const CAgentProfile& profile)
    {
        if(this != &profile) {
            _id = profile.GetProfileId();
            _name = profile.GetProfileName();
            _createtime = profile.GetCreateTime();
            _modifytime = profile.GetModifyTime();
            _default = profile.GetDefaultFlag();
        }
        return *this;
    }

    void Clear() throw()
    {
        _createtime.dwHighDateTime = 0;
        _createtime.dwLowDateTime = 0;
        _modifytime.dwHighDateTime = 0;
        _modifytime.dwLowDateTime = 0;
        _name.clear();
        _id.clear();
        _default = false;
    }


private:
    std::wstring    _id;
    std::wstring    _name;
    FILETIME        _createtime;
    FILETIME        _modifytime;
    bool            _default;
};


}   // namespace nxrmrest::schema::type
}   // namespace nxrmrest::schema
}   // namespace nxrmrest


#endif  // __NXREST_XSTYPE_AGENT_PROFILE_HPP__