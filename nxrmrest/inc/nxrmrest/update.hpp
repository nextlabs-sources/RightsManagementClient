




#ifndef __NXRM_REST_UPDATE_HPP__
#define __NXRM_REST_UPDATE_HPP__


#include <nudf\httputil.hpp>

namespace nxrmrest {


class CUpdateRequest : public nudf::http::CRequest
{
public:
    CUpdateRequest();
    virtual ~CUpdateRequest();
};


}   // namespace nxrmrest


#endif // __NXRM_REST_UPDATE_HPP__