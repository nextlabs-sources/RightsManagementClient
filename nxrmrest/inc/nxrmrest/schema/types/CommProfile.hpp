

#ifndef __NXREST_XSTYPE_COMM_PROFILE_HPP__
#define __NXREST_XSTYPE_COMM_PROFILE_HPP__


#include <string>

#include <nxrmrest\schema\types\Version.hpp>
#include <nxrmrest\schema\types\OsInfo.hpp>
#include <nxrmrest\schema\types\HwInfo.hpp>
#include <nxrmrest\schema\types\AgentType.hpp>
#include <nxrmrest\schema\types\BasicTypes.hpp>
#include <nxrmrest\schema\types\CommonFaults.hpp>


namespace nxrmrest {
namespace schema {
namespace type {

 
class CCommProfile
{
public:
    CCommProfile() : _default(false), _heartbeatFrequency(0), _logFrequency(0), _logSizeLimit(0)
    {
        _createtime.dwHighDateTime = 0;
        _createtime.dwLowDateTime = 0;
        _modifytime.dwHighDateTime = 0;
        _modifytime.dwLowDateTime = 0;
    }
    virtual ~CCommProfile()
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
    inline const CAgentType& GetAgentType() const throw() {return _agentType;}
    inline void SetAgentType(const CAgentType& type) throw() {_agentType = type;}
    inline const std::wstring& GetDABSLocation() const throw() {return _DABSLocation;}
    inline void SetDABSLocation(const std::wstring& location) throw() {_DABSLocation = location;}
    inline ULONGLONG GetHeartBeatFrequency() const throw() {return _heartbeatFrequency;}
    inline void SetHeartBeatFrequency(ULONGLONG frequency) throw() {_heartbeatFrequency = frequency;}
    inline ULONGLONG GetLogFrequency() const throw() {return _logFrequency;}
    inline void SetLogFrequency(ULONGLONG frequency) throw() {_logFrequency = frequency;}
    inline ULONGLONG GetLogSizeLimit() const throw() {return _logSizeLimit;}
    inline void SetLogSizeLimit(ULONGLONG size) throw() {_logSizeLimit = size;}

    CCommProfile& operator = (const CCommProfile& profile)
    {
        if(this != &profile) {
            _id = profile.GetProfileId();
            _name = profile.GetProfileName();
            _createtime = profile.GetCreateTime();
            _modifytime = profile.GetModifyTime();
            _default = profile.GetDefaultFlag();
            _agentType = profile.GetAgentType();
            _DABSLocation = profile.GetDABSLocation();
            _heartbeatFrequency = profile.GetHeartBeatFrequency();
            _logFrequency = profile.GetLogFrequency();
            _logSizeLimit = profile.GetLogSizeLimit();
        }
        return *this;
    }

    void Clear() throw()
    {
        _createtime.dwHighDateTime = 0;
        _createtime.dwLowDateTime = 0;
        _modifytime.dwHighDateTime = 0;
        _modifytime.dwLowDateTime = 0;
        _id.clear();
        _name.clear();
        _default = false;
        _DABSLocation.clear();
        _agentType.Clear();
        _heartbeatFrequency = 0;
        _logFrequency = 0;
        _logSizeLimit = 0;
    }


private:
    std::wstring    _id;
    std::wstring    _name;
    bool            _default;
    FILETIME        _createtime;
    FILETIME        _modifytime;
    CAgentType      _agentType;
    std::wstring    _DABSLocation;
    ULONGLONG       _heartbeatFrequency;    // In seconds
    ULONGLONG       _logFrequency;          // In seconds
    ULONGLONG       _logSizeLimit;          // In bytes
};


}   // namespace nxrmrest::schema::type
}   // namespace nxrmrest::schema
}   // namespace nxrmrest


#endif  // __NXREST_XSTYPE_COMM_PROFILE_HPP__