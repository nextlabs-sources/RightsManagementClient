

#ifndef __NXREST_XSTYPE_COMMON_FAULTS_HPP__
#define __NXREST_XSTYPE_COMMON_FAULTS_HPP__



namespace nxrmrest {
namespace schema {
namespace type {


typedef enum _FAULTS {
    FAULT_SUCCEED = 0,
    FAULT_SERVICE_NOT_READY,
    FAULT_UNAUTHORIZED_CALLER
} FAULTS;

}   // namespace nxrmrest::schema::type
}   // namespace nxrmrest::schema
}   // namespace nxrmrest


#endif  // __NXREST_XSTYPE_COMMON_FAULTS_HPP__