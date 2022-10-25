

#ifndef __NXREST_XS_REGISTERAGENT_REQUEST_HPP__
#define __NXREST_XS_REGISTERAGENT_REQUEST_HPP__

#include <string>

#include <nxrmrest\schema\types\AgentRegistrationData.hpp>

namespace nxrmrest {
namespace schema {


class CAgentRegisterRequest
{
public:
    CAgentRegisterRequest(){}
    virtual ~CAgentRegisterRequest(){}

    inline const type::CAgentRegisterData& GetAgentRegData() const throw() {return _data;}
    inline void SetAgentRegData(const type::CAgentRegisterData& data) throw() {_data = data;}

    CAgentRegisterRequest& operator = (const CAgentRegisterRequest& request)
    {
        if(this != &request) {
            _data = request.GetAgentRegData();
        }
        return *this;
    }

private:
    type::CAgentRegisterData    _data;
};


}   // namespace nxrmrest::schema
}   // namespace nxrmrest


#endif  // __NXREST_XS_REGISTERAGENT_REQUEST_HPP__