

#ifndef __NXREST_XS_REGISTERAGENT_RESPONSE_HPP__
#define __NXREST_XS_REGISTERAGENT_RESPONSE_HPP__

#include <string>

#include <nxrmrest\schema\types\Version.hpp>
#include <nxrmrest\schema\types\OsInfo.hpp>
#include <nxrmrest\schema\types\HwInfo.hpp>
#include <nxrmrest\schema\types\AgentType.hpp>
#include <nxrmrest\schema\types\CommonFaults.hpp>
#include <nxrmrest\schema\types\AgentStartupConfiguration.hpp>


namespace nxrmrest {
namespace schema {
    
class CAgentRegisterResponse
{
public:
    CAgentRegisterResponse(){}
    virtual ~CAgentRegisterResponse(){}

    inline const std::wstring& GetCertificate() const throw() {return _cert;}
    inline void SetCertificate(const std::wstring& cert) throw() {_cert = cert;}
    inline type::FAULTS GetFault() const throw() {return _fault;}
    inline void SetFault(type::FAULTS fault) throw() {_fault = fault;}
    inline const type::CAgentStartupConf& GetStartupConf() const throw() {return _conf;}
    inline void SetStartupConf(const type::CAgentStartupConf& conf) throw() {_conf = conf;}


private:
    type::FAULTS    _fault;
    std::wstring    _cert;
    type::CAgentStartupConf _conf;
};

}   // namespace nxrmrest::schema
}   // namespace nxrmrest


#endif  // __NXREST_XS_REGISTERAGENT_RESPONSE_HPP__