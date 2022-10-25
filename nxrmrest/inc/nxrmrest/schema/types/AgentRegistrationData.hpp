

#ifndef __NXREST_XSTYPE_AGENT_REGISTRATION_DATA_HPP__
#define __NXREST_XSTYPE_AGENT_REGISTRATION_DATA_HPP__


#include <string>

#include <nxrmrest\schema\types\Version.hpp>
#include <nxrmrest\schema\types\OsInfo.hpp>
#include <nxrmrest\schema\types\HwInfo.hpp>
#include <nxrmrest\schema\types\AgentType.hpp>

namespace nxrmrest {
namespace schema {
namespace type {

    
class CAgentRegisterData
{
public:
    CAgentRegisterData(){}
    virtual ~CAgentRegisterData(){}

    inline const std::wstring& GetHost() const throw() {return _host;}
    inline void SetHost(const std::wstring& host) throw() {_host = host;}
    inline const type::CAgentType& GetAgentType() const throw() {return _agentType;}
    inline void SetAgentType(const type::CAgentType& type) throw() {_agentType = type;}
    inline const std::wstring& GetTenantId() const throw() {return _tenantId;}
    inline void SetTenantId(const std::wstring& id) throw() {_tenantId = id;}
    inline const type::CVersion& GetVersion() const throw() {return _version;}
    inline void SetVersion(const type::CVersion& ver) throw() {_version = ver;}
    inline const type::COsInfo& GetOsInfo() const throw() {return _osinfo;}
    inline void SetOsInfo(const type::COsInfo& info) throw() {_osinfo = info;}
    inline const type::CHwInfo& GetHwInfo() const throw() {return _hwinfo;}
    inline void SetHwInfo(const type::CHwInfo& info) throw() {_hwinfo = info;}
    inline const type::CGroupInfo& GetGroupInfo() const throw() {return _groupinfo;}
    inline void SetGroupInfo(const type::CGroupInfo& info) throw() {_groupinfo = info;}

    CAgentRegisterData& operator = (const CAgentRegisterData& data)
    {
        if(this != &data) {
            _host = data.GetHost();
            _agentType = data.GetAgentType();
            _tenantId = data.GetTenantId();
            _version = data.GetVersion();
            _osinfo = data.GetOsInfo();
            _hwinfo = data.GetHwInfo();
            _groupinfo = data.GetGroupInfo();
        }
        return *this;
    }

private:
    std::wstring        _host;
    std::wstring        _tenantId;
    type::CAgentType    _agentType;
    type::CVersion      _version;
    type::COsInfo       _osinfo;
    type::CHwInfo       _hwinfo;
    type::CGroupInfo    _groupinfo;
};


}   // namespace nxrmrest::schema::type
}   // namespace nxrmrest::schema
}   // namespace nxrmrest


#endif  // __NXREST_XSTYPE_AGENT_REGISTRATION_DATA_HPP__