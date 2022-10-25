

#include <Windows.h>
#include <assert.h>
#include <Wtsapi32.h>
#include <Userenv.h>

#include <nudf\exception.hpp>
#include <nudf\session.hpp>


using namespace nudf::win;

CSession::CSession() : _SessionId(-1)
{
}

CSession::CSession(_In_ ULONG dwSessionId) : _SessionId(dwSessionId)
{
}

CSession::~CSession()
{
}

BOOL CSession::IsActive() throw()
{
    WTS_CONNECTSTATE_CLASS State = WTSDown;
    if(!GetConnState(&State)) {
        return FALSE;
    }
    return (WTSActive == State);
}

BOOL CSession::IsConnected() throw()
{
    WTS_CONNECTSTATE_CLASS State = WTSDown;
    if(!GetConnState(&State)) {
        return FALSE;
    }
    return (WTSConnected == State);
}

BOOL CSession::IsDisconnected() throw()
{
    WTS_CONNECTSTATE_CLASS State = WTSDown;
    if(!GetConnState(&State)) {
        return FALSE;
    }
    return (WTSDisconnected == State);
}

BOOL CSession::IsIdle() throw()
{
    WTS_CONNECTSTATE_CLASS State = WTSDown;
    if(!GetConnState(&State)) {
        return FALSE;
    }
    return (WTSIdle == State);
}

BOOL CSession::IsConsole() throw()
{
    USHORT Proto = -1;
    if(!GetClientProtocol(&Proto)) {
        return FALSE;
    }
    return (0 == Proto);
}

BOOL CSession::IsRdp() throw()
{
    USHORT Proto = -1;
    if(!GetClientProtocol(&Proto)) {
        return FALSE;
    }
    return (2 == Proto);
}

BOOL CSession::GetInitialProgram(_Out_ std::wstring& wsInitProgram) throw()
{
    DWORD   dwBytesReturned = 0;
    LPWSTR  pbBuffer = NULL;

    if(!WTSQuerySessionInformationW(NULL, _SessionId, WTSInitialProgram, &pbBuffer, &dwBytesReturned)) {
        return FALSE;
    }

    wsInitProgram = pbBuffer;
    WTSFreeMemory(pbBuffer);
    return TRUE;
}

BOOL CSession::GetLogonUserName(_Out_ std::wstring& wsUserName, _Out_ std::wstring& wsDomainName) throw()
{
    DWORD   dwBytesReturned = 0;
    LPWSTR  pbBuffer = NULL;

    if(!WTSQuerySessionInformationW(NULL, _SessionId, WTSUserName, &pbBuffer, &dwBytesReturned)) {
        return FALSE;
    }
    wsUserName = pbBuffer;
    WTSFreeMemory(pbBuffer); pbBuffer = NULL;

    if(WTSQuerySessionInformationW(NULL, _SessionId, WTSDomainName, &pbBuffer, &dwBytesReturned)) {
        wsDomainName = pbBuffer;
        WTSFreeMemory(pbBuffer); pbBuffer = NULL;
    }

    return TRUE;
}

BOOL CSession::GetWinStationName(_Out_ std::wstring& wsWinStationName) throw()
{
    DWORD   dwBytesReturned = 0;
    LPWSTR  pbBuffer = NULL;

    if(!WTSQuerySessionInformationW(NULL, _SessionId, WTSWinStationName, &pbBuffer, &dwBytesReturned)) {
        return FALSE;
    }
    wsWinStationName = pbBuffer;
    WTSFreeMemory(pbBuffer); pbBuffer = NULL;

    return TRUE;
}

BOOL CSession::GetClientName(_Out_ std::wstring& wsClientName) throw()
{
    DWORD   dwBytesReturned = 0;
    LPWSTR  pbBuffer = NULL;

    if(!WTSQuerySessionInformationW(NULL, _SessionId, WTSClientName, &pbBuffer, &dwBytesReturned)) {
        return FALSE;
    }
    wsClientName = pbBuffer;
    WTSFreeMemory(pbBuffer); pbBuffer = NULL;

    return TRUE;
}

BOOL CSession::GetClientAddress(_Out_ PWTS_CLIENT_ADDRESS wtsClientAddress) throw()
{
    DWORD   dwBytesReturned = 0;
    LPWSTR  pbBuffer = NULL;

    if(!WTSQuerySessionInformationW(NULL, _SessionId, WTSClientAddress, &pbBuffer, &dwBytesReturned)) {
        return FALSE;
    }
    if(dwBytesReturned >= sizeof(WTS_CLIENT_ADDRESS)) {
        memcpy(wtsClientAddress, pbBuffer, sizeof(WTS_CLIENT_ADDRESS));
    }
    WTSFreeMemory(pbBuffer); pbBuffer = NULL;
    return TRUE;
}

BOOL CSession::GetConnState(_Out_ WTS_CONNECTSTATE_CLASS* wtsConnState) throw()
{
    DWORD   dwBytesReturned = 0;
    LPWSTR  pbBuffer = NULL;

    if(!WTSQuerySessionInformationW(NULL, _SessionId, WTSConnectState, &pbBuffer, &dwBytesReturned)) {
        return FALSE;
    }
    *wtsConnState = *((WTS_CONNECTSTATE_CLASS*)pbBuffer);
    WTSFreeMemory(pbBuffer); pbBuffer = NULL;
    return TRUE;
}

BOOL CSession::GetClientProtocol(_Out_ USHORT* Proto) throw()
{
    DWORD   dwBytesReturned = 0;
    LPWSTR  pbBuffer = NULL;

    if(!WTSQuerySessionInformationW(NULL, _SessionId, WTSClientProtocolType, &pbBuffer, &dwBytesReturned)) {
        return FALSE;
    }
    *Proto = *((USHORT*)pbBuffer);
    WTSFreeMemory(pbBuffer); pbBuffer = NULL;
    return TRUE;
}

HANDLE CSession::GetToken() const throw()
{
    HANDLE hToken = NULL;

    if(-1 != _SessionId) {
        if(!WTSQueryUserToken(_SessionId, &hToken)) {
            hToken = NULL;
        }
    }

    return hToken;
}

BOOL CSession::GetUser(_Out_ CUser& u) throw()
{
    BOOL bRet = FALSE;

    if(-1 == _SessionId) {
        return FALSE;
    }

    if(!IsActive()) {
        return FALSE;
    }

    HANDLE hToken = NULL;

    if(!WTSQueryUserToken(_SessionId, &hToken)) {
        return FALSE;
    }

    try {
        u.GetInfoByToken(hToken);
        CloseHandle(hToken);
        hToken = NULL;
        bRet = TRUE;
    }
    catch(const nudf::CException& e) {
        CloseHandle(hToken);
        hToken = NULL;
        UNREFERENCED_PARAMETER(e);
    }

    return bRet;
}

CSession& CSession::operator =(const CSession& s) throw()
{
    if(this != &s) {
        _SessionId = s.GetSessionId();
    }
    return *this;
}

CSession& CSession::operator =(ULONG dwSessionId) throw()
{
    _SessionId = dwSessionId;
    return *this;
}

BOOL CSession::operator ==(const CSession& s) const throw()
{
    return (_SessionId == s.GetSessionId());
}

BOOL CSession::operator ==(ULONG dwSessionId) const throw()
{
    return (_SessionId == dwSessionId);
}
