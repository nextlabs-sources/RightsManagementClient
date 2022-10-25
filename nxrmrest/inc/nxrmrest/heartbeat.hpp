




#ifndef __NXRM_REST_HEARTBEAT_HPP__
#define __NXRM_REST_HEARTBEAT_HPP__


#include <nudf\httputil.hpp>

namespace nxrmrest {


class CHeartbeatRequest : public nudf::http::CRequest
{
public:
    CHeartbeatRequest();
    virtual ~CHeartbeatRequest();
};


}   // namespace nxrmrest


#endif // __NXRM_REST_HEARTBEAT_HPP__