

#include <Windows.h>
#include <stdio.h>
#include <AccCtrl.h>
#include <Aclapi.h>
#include <assert.h>

#include <nudf\exception.hpp>
#include <nudf\pipe.hpp>

using namespace nudf;
using namespace nudf::ipc;



typedef struct _PIPEINST {
   OVERLAPPED   Overlap;
   CPipeServer* Server;
   HANDLE       Pipe;
   ULONG        ToTransfer;
   UCHAR        Buffer[1];
} PIPEINST, *LPPIPEINST;


#define PIPENAME_PREFIX     L"\\\\.\\pipe\\"


DWORD WINAPI MainPipeThread(LPVOID lpThreadParameter);
VOID WINAPI CompletedWriteRoutine(DWORD, DWORD, LPOVERLAPPED); 
VOID WINAPI CompletedReadRoutine(DWORD, DWORD, LPOVERLAPPED); 





CPipeServer::CPipeServer() : m_dwTimeout(DEFAULT_PIPE_TIMEOUT),
                             m_dwBufSize(DEFAULT_PIPE_BUFSIZE),
                             m_hMainThread(NULL),
                             m_dwMainThreadId(0),
                             m_fStopping(FALSE),
                             m_hConnEvent(NULL),
                             m_fEveryone(FALSE)
{
    m_wzName[0] = L'\0';
}

CPipeServer::CPipeServer(_In_ LPCWSTR wzName, _In_ DWORD dwBufSize, _In_ DWORD dwTimeout) : m_dwTimeout(dwTimeout),
                                                                                            m_dwBufSize(dwBufSize),
                                                                                            m_hMainThread(NULL),
                                                                                            m_dwMainThreadId(0),
                                                                                            m_fStopping(FALSE),
                                                                                            m_hConnEvent(NULL),
                                                                                            m_fEveryone(FALSE)
{
    SetName(wzName);
}

CPipeServer::~CPipeServer()
{
}

VOID CPipeServer::SetName(_In_ LPCWSTR wzName) throw()
{
    m_wzName[0] = L'\0';
    wcsncpy_s(m_wzName, MAX_PATH, PIPENAME_PREFIX, _TRUNCATE);
    wcscat_s(m_wzName, MAX_PATH-10, wzName);
}

VOID CPipeServer::SetTimeout(_In_ ULONG dwTimeout) throw()
{
    m_dwTimeout = dwTimeout;
}

VOID CPipeServer::SetBufferSize(_In_ ULONG dwBufSize) throw()
{
    m_dwBufSize = dwBufSize;
}

VOID CPipeServer::Start()
{
    try {

        m_hConnEvent = ::CreateEventW(NULL, FALSE, FALSE, NULL);
        if(NULL == m_hConnEvent) {
            throw WIN32ERROR();
        }

        m_hMainThread = ::CreateThread(NULL, 0, MainPipeThread, this, 0, &m_dwMainThreadId);
        if(NULL == m_hMainThread) {
            m_dwMainThreadId = 0;
            throw WIN32ERROR();
        }
    }
    catch(const nudf::CException& e) {
        if(NULL != m_hConnEvent) {
            CloseHandle(m_hConnEvent);
            m_hConnEvent = NULL;
        }
        if(NULL != m_hMainThread) {
            CloseHandle(m_hMainThread);
            m_hMainThread = NULL;
        }
        throw e;
    }
}

VOID CPipeServer::Shutdown()
{
    if(NULL != m_hMainThread) {
        m_fStopping = TRUE;
        SetEvent(m_hConnEvent);
        (VOID)::WaitForSingleObject(m_hMainThread, INFINITE);
        CloseHandle(m_hMainThread);
        m_hMainThread = NULL;
        m_dwMainThreadId = 0;
        m_fStopping = FALSE;
    }
    if(NULL != m_hConnEvent) {
        CloseHandle(m_hConnEvent);
        m_hConnEvent = NULL;
    }
}

VOID CPipeServer::CreatePipeInstance(PHANDLE phPipe, LPOVERLAPPED lpOverlap, PBOOL pfPending)
{
    HANDLE hPipe = INVALID_HANDLE_VALUE;
    DWORD dwError = 0;
    // SD
    SECURITY_ATTRIBUTES sa = {0};
    PSECURITY_DESCRIPTOR sd = NULL;
    EXPLICIT_ACCESS ea;
    PACL acl = NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
    PSID pEveryoneSid = NULL;

    static const DWORD dwOpenMode = PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED;
    static const DWORD dwPipeMode = PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT;
    static const DWORD dwMaxInstances = PIPE_UNLIMITED_INSTANCES;

    *phPipe = INVALID_HANDLE_VALUE;
    *pfPending = FALSE;

    if(m_fEveryone) {

        AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pEveryoneSid);

        ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
        ea.grfAccessPermissions = GENERIC_READ | GENERIC_WRITE;
        ea.grfAccessMode        = SET_ACCESS;
        ea.grfInheritance       = NO_INHERITANCE;
        ea.Trustee.TrusteeForm  = TRUSTEE_IS_SID;
        ea.Trustee.TrusteeType  = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ea.Trustee.ptstrName    = (LPWSTR)pEveryoneSid;

        SetEntriesInAclW(1, &ea, NULL, &acl);

        sd = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
        InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(sd, TRUE, acl, FALSE);

        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = sd;
        sa.bInheritHandle = FALSE;
    }
    
    hPipe = CreateNamedPipeW(m_wzName, dwOpenMode, dwPipeMode, dwMaxInstances, m_dwBufSize, m_dwBufSize, m_dwTimeout, m_fEveryone ? (&sa) : NULL);
    DWORD dwErr = GetLastError();
    if(NULL != acl) {LocalFree(acl); acl = NULL;}
    if(NULL != sd) {LocalFree(sd); sd = NULL;}
    if(NULL != pEveryoneSid) {FreeSid(pEveryoneSid); pEveryoneSid = NULL;}
    if (hPipe == INVALID_HANDLE_VALUE)  {
        throw WIN32ERROR2(dwErr);
    }
    
    // Overlapped ConnectNamedPipe should return zero. 
    if (0 != ConnectNamedPipe(hPipe, lpOverlap)) {
        dwError = GetLastError();
        CloseHandle(hPipe);
        throw WIN32ERROR2(dwError);
    }

    
    dwError = GetLastError();
    if(ERROR_IO_PENDING == dwError) {
        *pfPending = TRUE;
    }
    else if(ERROR_PIPE_CONNECTED == dwError) {
        if (!SetEvent(lpOverlap->hEvent)) {
            dwError = GetLastError();
            CloseHandle(hPipe);
            throw WIN32ERROR2(dwError);
        }
    }
    else {
        // Unknown Error Code
        CloseHandle(hPipe);
        throw WIN32ERROR2(dwError);
    }

    *phPipe = hPipe;
}

VOID CPipeServer::OnClientConnect(_In_ HANDLE hPipe, _In_ LPOVERLAPPED lpOverlap, _In_ BOOL fPending) throw()
{
    LPPIPEINST lpPipeInst = NULL;

    const ULONG cbInstanceSize = sizeof(PIPEINST) + m_dwBufSize;

    try {

        if (fPending) {
            DWORD cbRet = 0;
            if (!GetOverlappedResult(hPipe, lpOverlap, &cbRet, FALSE)) {
                throw WIN32ERROR();
            }
        }

        // Allocate storage for this instance.
        lpPipeInst = (LPPIPEINST)GlobalAlloc(GPTR, cbInstanceSize); 
        if (lpPipeInst == NULL) {
            SetLastError(ERROR_OUTOFMEMORY);
            throw WIN32ERROR();
        }

        // Initialize Instance
        memset(lpPipeInst, 0, cbInstanceSize);
        lpPipeInst->Pipe    = hPipe;
        lpPipeInst->Server  = this;
        // Read data
        CompletedWriteRoutine(0, 0, (LPOVERLAPPED)lpPipeInst); 
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        (VOID)DisconnectNamedPipe(hPipe);
        (VOID)CloseHandle(hPipe);
        if(NULL != lpPipeInst) {
            GlobalFree(lpPipeInst);
            lpPipeInst = NULL;
        }
    }
}

VOID CPipeServer::OnRequest(_In_ const UCHAR* pbRequest, _In_ ULONG cbRequest, _In_ UCHAR* pbReply, _In_ ULONG cbReply, _Out_ PULONG pcbValidReply, _Out_ PBOOL pfClose)
{
    UNREFERENCED_PARAMETER(pbRequest);
    UNREFERENCED_PARAMETER(cbRequest);
    UNREFERENCED_PARAMETER(pbReply);
    UNREFERENCED_PARAMETER(cbReply);
    *pcbValidReply = 0;
    *pfClose = FALSE;
}

DWORD CPipeServer::WorkerRoutine()
{
    DWORD       dwRet = 0;
    OVERLAPPED  oOverlap;
    HANDLE      hPipe = INVALID_HANDLE_VALUE;
    BOOL        fPending = FALSE;

    //
    //  Create/Connect first pipe instance
    //
    memset(&oOverlap, 0, sizeof(oOverlap));
    oOverlap.hEvent = m_hConnEvent;

    try {
        CreatePipeInstance(&hPipe, &oOverlap, &fPending);
    }
    catch(const nudf::CException& e) {
        dwRet = e.GetCode();
        return dwRet;
    }

    while(!m_fStopping) {

        DWORD dwWait = 0;

        dwWait = WaitForSingleObjectEx(m_hConnEvent, INFINITE, TRUE);
        if(m_fStopping) {
            m_fStopping = FALSE;
            (VOID)DisconnectNamedPipe(hPipe);
            (VOID)CloseHandle(hPipe);
            hPipe = INVALID_HANDLE_VALUE;
            return 0;
        }

        switch(dwWait)
        {
        case WAIT_OBJECT_0:
            OnClientConnect(hPipe, &oOverlap, fPending);
            hPipe = INVALID_HANDLE_VALUE;
            try {
                CreatePipeInstance(&hPipe, &oOverlap, &fPending);
            }
            catch(const nudf::CException& e) {
                dwRet = e.GetCode();
                return dwRet;
            }
            break;

        case WAIT_IO_COMPLETION:
            break;

        default:
            return 0U;
        }
    }

    return dwRet;
}

DWORD WINAPI MainPipeThread(LPVOID lpThreadParameter)
{
    CPipeServer* lpServer = (CPipeServer*)lpThreadParameter;
    return lpServer->WorkerRoutine();
}

VOID WINAPI CompletedWriteRoutine(DWORD dwErr, DWORD cbWritten, LPOVERLAPPED lpOverLap) 
{ 
   LPPIPEINST lpPipeInst; 
   BOOL fRead = FALSE; 
   
   // lpOverlap points to storage for this instance. 
   lpPipeInst = (LPPIPEINST)lpOverLap; 

   // The write operation has finished, so read the next request (if
   // there is no error). 
   if ((dwErr == 0) && (cbWritten == lpPipeInst->ToTransfer)) {
       fRead = ReadFileEx(lpPipeInst->Pipe,
                          lpPipeInst->Buffer,
                          lpPipeInst->Server->GetBufSize(),
                          (LPOVERLAPPED)lpPipeInst,
                          (LPOVERLAPPED_COMPLETION_ROUTINE)CompletedReadRoutine
                          );
      if(!fRead) {
          DisconnectNamedPipe(lpPipeInst->Pipe);
          CloseHandle(lpPipeInst->Pipe);
          GlobalFree(lpPipeInst);
          lpPipeInst = NULL;
      }
   }
} 
 
// CompletedReadRoutine(DWORD, DWORD, LPOVERLAPPED) 
// This routine is called as an I/O completion routine after reading 
// a request from the client. It gets data and writes it to the pipe. 
VOID WINAPI CompletedReadRoutine(DWORD dwErr, DWORD cbBytesRead, LPOVERLAPPED lpOverLap) 
{ 
   LPPIPEINST   lpPipeInst; 
   BOOL         fWrite = FALSE;
   ULONG        cbToWrite = 0;
   BOOL         fClose = FALSE;
   
   // lpOverlap points to storage for this instance.
   lpPipeInst = (LPPIPEINST)lpOverLap;
   
   // The read operation has finished, so write a response (if no
   // error occurred).
   if ((dwErr == 0) && (cbBytesRead != 0)) {

       lpPipeInst->Server->OnRequest(lpPipeInst->Buffer, cbBytesRead, lpPipeInst->Buffer, lpPipeInst->Server->GetBufSize(), &lpPipeInst->ToTransfer, &fClose);
       if(!fClose) {
           fWrite = WriteFileEx( 
               lpPipeInst->Pipe, 
               lpPipeInst->Buffer, 
               lpPipeInst->ToTransfer,
               (LPOVERLAPPED)lpPipeInst, 
               (LPOVERLAPPED_COMPLETION_ROUTINE)CompletedWriteRoutine); 
       }
   }


   // Disconnect if an error occurred. 
   if (!fWrite || fClose)  {
       DisconnectNamedPipe(lpPipeInst->Pipe);
       CloseHandle(lpPipeInst->Pipe);
       GlobalFree(lpPipeInst);
       lpPipeInst = NULL;
   }
} 


//
//  class CPipeClient
//

CPipeClient::CPipeClient()
{
}

CPipeClient::CPipeClient(_In_ LPCWSTR wzName)
{
    m_wzName[0] = L'\0';
    wcsncpy_s(m_wzName, MAX_PATH, PIPENAME_PREFIX, _TRUNCATE);
    wcscat_s(m_wzName, MAX_PATH-10, wzName);
}

CPipeClient::~CPipeClient()
{
    Disconnect();
}

VOID CPipeClient::Connect(_In_ ULONG WaitTime)
{
    DWORD dwMode = 0;

    m_hPipe = CreateFileW(m_wzName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if(INVALID_HANDLE_VALUE == m_hPipe) {

        if (GetLastError() != ERROR_PIPE_BUSY) {
            throw WIN32ERROR();
        }

        if(!WaitNamedPipeW(m_wzName, WaitTime))  { 
            SetLastError(ERROR_TIMEOUT);
            throw WIN32ERROR();
        }

        // Try again
        m_hPipe = CreateFileW(m_wzName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if(INVALID_HANDLE_VALUE == m_hPipe) {
            throw WIN32ERROR();
        }
    }
    
    dwMode = PIPE_READMODE_MESSAGE; 
    if(!SetNamedPipeHandleState(m_hPipe, &dwMode, NULL, NULL)) {
        DWORD dwError = GetLastError();
        CloseHandle(m_hPipe);
        m_hPipe = INVALID_HANDLE_VALUE;
        SetLastError(dwError);
        throw WIN32ERROR();
    }

    // Succeed
}

VOID CPipeClient::Disconnect() throw()
{
    if(INVALID_HANDLE_VALUE != m_hPipe) {
        CloseHandle(m_hPipe);
        m_hPipe = INVALID_HANDLE_VALUE;
    }
}

VOID CPipeClient::Request(_In_ const UCHAR* pbReq, _In_ ULONG cbReq, _Out_opt_ PUCHAR pbReply, _In_ ULONG cbReply, _Out_opt_ PULONG cbReturned)
{
    DWORD dwToWrite = 0;
    DWORD dwWritten = 0;
    DWORD dwRead = 0;

    dwToWrite = cbReq;

    if(0 == cbReq) {
        SetLastError(ERROR_INVALID_PARAMETER);
        throw WIN32ERROR();
    }

    while(0 != dwToWrite) {
        if(!WriteFile(m_hPipe, pbReq, dwToWrite, &dwWritten, NULL)) {
            throw WIN32ERROR();
        }
        dwToWrite -= dwWritten;
    }

    if(cbReq != dwWritten) {
        SetLastError(ERROR_IO_INCOMPLETE);
        throw WIN32ERROR();
    }

    if(NULL == pbReply && 0 != cbReply) {
        SetLastError(ERROR_INVALID_PARAMETER);
        throw WIN32ERROR();
    }

    if(NULL == pbReply) {
        return;
    }

    while(TRUE) {

        DWORD dwError = 0;

        if(!ReadFile(m_hPipe, pbReply, cbReply, &dwRead, NULL)) {

            dwError = GetLastError();
            if(ERROR_MORE_DATA != dwError) {
                throw WIN32ERROR();
            }

            assert(ERROR_MORE_DATA == dwError);
            if(cbReply == 0) {
                SetLastError(ERROR_BUFFER_OVERFLOW);
                throw WIN32ERROR();
            }

            pbReply += dwRead;
            if(NULL != cbReturned) *cbReturned += dwRead;
            continue;
        }
        else {
            if(NULL != cbReturned) *cbReturned += dwRead;
        }

        break;
    }
}


CAnonymousPipe::CAnonymousPipe() : _hRd(NULL), _hWr(NULL), _Size(0)
{
}

CAnonymousPipe::~CAnonymousPipe()
{
    Destroy();
}

VOID CAnonymousPipe::Create(_In_opt_ LPSECURITY_ATTRIBUTES lpPipeAttributes)
{
    if(!CreatePipe(&_hRd, &_hWr, lpPipeAttributes, _Size)) {
        throw WIN32ERROR();
    }
}

VOID CAnonymousPipe::Destroy()
{
    if(_hRd) {
        CloseHandle(_hRd);
        _hRd = NULL;
    }
    if(_hWr) {
        CloseHandle(_hWr);
        _hWr = NULL;
    }
    _Size = 0;
}
