
#ifndef __NXRM_ENGINE_IPC_HPP__
#define __NXRM_ENGINE_IPC_HPP__


#include <winioctl.h>

#include <nudf\exception.hpp>
#include <nudf\pipe.hpp>
#include <nudf\shared\engineenv.h>
#include <nudf\shared\enginectl.h>

namespace nxrm {
namespace engine {


#define NXRMSERV_IPC_BUFSIZE    8192
#define NXRMSERV_IPC_TIMEOUT    5000    // 5 seconds



class CIpcServ : public nudf::ipc::CPipeServer
{
public:
    CIpcServ();
    virtual ~CIpcServ();
    
    VOID Initialize();
    VOID Cleanup();
    
    virtual VOID OnRequest(_In_ const UCHAR* pbRequest, _In_ ULONG cbRequest, _In_ UCHAR* pbReply, _In_ ULONG cbReply, _Out_ PULONG pcbValidReply, _Out_ PBOOL pfClose) throw();


protected:
    VOID OnQueryStatus(_In_ const RM_QUERYSTATUS_REQUEST* request, _Out_ RM_ENGINE_STATUS* status);
    BOOL OnCollectLog(_In_ const RM_REQUEST_HEADER* request);
    void OnUpdatePolicy(_In_ const RM_REQUEST_HEADER* request);
    DWORD OnAuthn(_In_ const RM_AUTHN_REQUEST* request);
};

}   // namespace engine
}   // namespace nxrm


#endif  // #ifndef __NXRM_ENGINE_IPC_HPP__