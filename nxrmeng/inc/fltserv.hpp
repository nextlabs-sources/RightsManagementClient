
#ifndef __NXRM_ENGINE_FLT_SERV_H__
#define __NXRM_ENGINE_FLT_SERV_H__


#include <nudf\thread.hpp>
#include <nudf\shared\keydef.h>

#include "nxrmflt.h"

namespace nxrm {
namespace engine {
    

    
class CFltWorkerThread : public nudf::util::thread::CThreadEx
{
public:
    CFltWorkerThread();
    virtual ~CFltWorkerThread();
    
    virtual DWORD OnStart(_In_opt_ PVOID Context);
    virtual DWORD OnRunning(_In_opt_ PVOID Context);
};


typedef struct _FLTNOTIFYITEM {
	LIST_ENTRY      Link;
    ULONG           Type;
	PVOID           Notification;
	PVOID           Context;
}FLTNOTIFYITEM, *PFLTNOTIFYITEM;

class CFltServ : public nudf::util::thread::CThreadExPool<CFltWorkerThread>
{
public:
    CFltServ();
    virtual ~CFltServ();

    void Start(const NXRM_KEY_BLOB* KeyBlob, ULONG KeyBlobSize);
    virtual void Stop() throw();
    ULONG SetSaveAsForecast(ULONG ProcessId, _In_opt_ const WCHAR* SrcFileName, _In_ const WCHAR* SaveAsFileName);
    ULONG SetPolicyChanged();
    ULONG UpdateKeyChain(const NXRM_KEY_BLOB* KeyBlob, ULONG KeyBlobSize);
    
    PFLTNOTIFYITEM PopRequest();
    VOID PushRequest(PFLTNOTIFYITEM Request);
    VOID CleanRequestQueue();

    ULONG MakeCheckRightsRequest(_In_ PVOID Msg, _In_ ULONG Length, _In_ PVOID MsgCtx);
    ULONG MakeBlockNotificationRequest(_In_ PVOID Msg, _In_ ULONG Length, _In_ PVOID MsgCtx);
    ULONG MakeKeyErrorNotificationRequest(_In_ PVOID Msg, _In_ ULONG Length, _In_ PVOID MsgCtx);
    ULONG MakePurgeCacheRequest(_In_ PVOID Msg, _In_ ULONG Length, _In_ PVOID MsgCtx);
    VOID OnCheckRights(_In_ const NXRM_CHECK_RIGHTS_NOTIFICATION* request, _In_ PVOID context);
    VOID OnBlockNotification(_In_ const NXRM_BLOCK_NOTIFICATION* request, _In_ PVOID context);
    VOID OnKeyErrorNotification(_In_ const NXRM_KEYCHAIN_ERROR_NOTIFICATION* request, _In_ PVOID context);
    VOID OnPurgeCacheNotification(_In_ const NXRM_PURGE_CACHE_NOTIFICATION* request, _In_ PVOID context);

public:
    VOID OnReplyMessage(_In_ PFLTNOTIFYITEM NotifyItem);

private:
    LIST_ENTRY       _list;
    CRITICAL_SECTION _listlock;
    HANDLE           _fltman;
};

       
}   // namespace engine
}   // namespace nxrm


#endif  // #ifndef __NXRM_ENGINE_FLT_SERV_H__