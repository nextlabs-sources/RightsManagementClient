

#ifndef __COREHLP_HPP__
#define __COREHLP_HPP__


typedef PVOID NXCONTEXT;

#define MAX_CORE_TIMEOUT        (30*1000)

class CCoreObject
{
public:
    CCoreObject();
    virtual ~CCoreObject();

    int Connect();
    void Disconnect();

    BOOL InitTransportClient();
    BOOL CloseTransportClient();
    NXCONTEXT SubmitRequest(ULONG type, PVOID req, ULONG length);
    BOOL SubmitNotify(ULONG type, PVOID req, ULONG length);
    BOOL WaitForResponse(NXCONTEXT context, PVOID resp, ULONG length, ULONG *bytesret);
    BOOL IsTransportEnabled();
    BOOL IsXxxChannelOn(const LONG channel);
    BOOL IsProtectedProcess(DWORD pid);
    LONG GetRmPolicySn();

    inline BOOL IsConnected() const throw() {return (NULL != _h);}

private:
    HANDLE  _h;
};


#endif