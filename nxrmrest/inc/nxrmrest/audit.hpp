




#ifndef __NXRM_REST_AUDIT_HPP__
#define __NXRM_REST_AUDIT_HPP__


#include <nudf\httputil.hpp>

namespace nxrmrest {


class CAuditRequest : public nudf::http::CRequest
{
public:
    CAuditRequest();
    virtual ~CAuditRequest();
};


}   // namespace nxrmrest


#endif // __NXRM_REST_AUDIT_HPP__