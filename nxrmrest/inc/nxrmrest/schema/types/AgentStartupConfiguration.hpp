

#ifndef __NXREST_XSTYPE_AGENT_STARTUP_CONF_HPP__
#define __NXREST_XSTYPE_AGENT_STARTUP_CONF_HPP__


#include <string>

#include <nxrmrest\schema\types\Version.hpp>
#include <nxrmrest\schema\types\OsInfo.hpp>
#include <nxrmrest\schema\types\HwInfo.hpp>
#include <nxrmrest\schema\types\AgentType.hpp>
#include <nxrmrest\schema\types\CommonFaults.hpp>
#include <nxrmrest\schema\types\CommProfile.hpp>
#include <nxrmrest\schema\types\AgentProfile.hpp>


namespace nxrmrest {
namespace schema {
namespace type {


    
class CAgentStartupConf
{
public:
    CAgentStartupConf(){}
    virtual ~CAgentStartupConf(){}

    inline const CCommProfile& GetCommProfile() const throw() {return _commProfile;}
    inline void SetCommProfile(const CCommProfile& profile) throw() {_commProfile = profile;}
    inline const CAgentProfile& GetAgentProfile() const throw() {return _agentProfile;}
    inline void SetAgentProfile(const CAgentProfile& profile) throw() {_agentProfile = profile;}
    inline const std::wstring& GetRegistrationId() const throw() {return _regId;}
    inline void SetRegistrationId(const std::wstring& id) throw() {_regId = id;}

    CAgentStartupConf& operator = (const CAgentStartupConf& conf)
    {
        if(this != &conf) {
            _commProfile = conf.GetCommProfile();
            _agentProfile = conf.GetAgentProfile();
            _regId = conf.GetRegistrationId();
        }
        return *this;
    }


private:
    CCommProfile    _commProfile;
    CAgentProfile   _agentProfile;
    std::wstring    _regId;
};


}   // namespace nxrmrest::schema::type
}   // namespace nxrmrest::schema
}   // namespace nxrmrest


#endif  // __NXREST_XSTYPE_AGENT_STARTUP_CONF_HPP__