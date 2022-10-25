

#include <Windows.h>
#include <assert.h>

#include "engine.hpp"
#include "timerserv.hpp"


using namespace nxrm::engine;


CTimer::CTimer() : _elapse(0), _hthread(NULL)
{
    _timer_events[0] = ::CreateEventW(NULL, FALSE, FALSE, NULL);
    _timer_events[1] = ::CreateEventW(NULL, FALSE, FALSE, NULL);
}

CTimer::~CTimer()
{
    if(NULL != _timer_events[1]) {
        CloseHandle(_timer_events[1]);
        _timer_events[1] = NULL;
    }
    if(NULL != _timer_events[0]) {
        Kill();
        CloseHandle(_timer_events[0]);
        _timer_events[0] = NULL;
    }
}

BOOL CTimer::Set(_In_ ULONG elapse, BOOL initial)
{
    BOOL bRet = TRUE;
    _elapse = elapse;
    if(_hthread == NULL) {
        bRet = InterSetTimer();
    }
    if(initial) {
        Trigger();
    }
    return bRet;
}

BOOL CTimer::Trigger()
{
    if(NULL == _hthread) {
        return FALSE;
    }
    if(NULL == _timer_events[1]) {
        return FALSE;
    }
    return SetEvent(_timer_events[1]);
}

void CTimer::Kill(BOOL wait)
{
    if(NULL != _hthread) {
        SetEvent(_timer_events[0]);
        if(wait) {
            ::WaitForSingleObject(_hthread, INFINITE);
        }
        CloseHandle(_hthread);
        _hthread = NULL;
    }
}

BOOL CTimer::InterSetTimer()
{
    DWORD tid = 0;

    assert(NULL == _hthread);

    if(NULL == _timer_events[0]) {        
        _timer_events[0] = ::CreateEventW(NULL, FALSE, FALSE, NULL);
        if(NULL == _timer_events[0]) {
            return FALSE;
        }
    }
    if(NULL == _timer_events[1]) {        
        _timer_events[1] = ::CreateEventW(NULL, FALSE, FALSE, NULL);
        if(NULL == _timer_events[1]) {
            return FALSE;
        }
    }

    _hthread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TimerThreadProc, this, 0, &tid);
    if(NULL == _hthread) {
        return FALSE;
    }

    return TRUE;
}

DWORD WINAPI CTimer::TimerThreadProc(_In_ LPVOID lpVoid)
{
    CTimer* lpThis = (CTimer*)lpVoid;
    BOOL bRun = TRUE;

    assert(NULL != lpThis);

    while(bRun) {

        DWORD dwResult = ::WaitForMultipleObjects(2, lpThis->_timer_events, FALSE, lpThis->_elapse);
        switch(dwResult)
        {
        case WAIT_OBJECT_0:
            bRun = FALSE;
            break;

        case (WAIT_OBJECT_0+1):
        case WAIT_TIMEOUT:
            lpThis->OnTimer();
            break;

        default:
            bRun = FALSE;
            break;
        }
    }

    return 0;
}

void CRegisterTimer::OnTimer()
{
    gpEngine->OnTimerRegister();
}

void CHeartBeatTimer::OnTimer()
{
    gpEngine->OnTimerHeartBeat();
}

void CCheckUpdateTimer::OnTimer()
{
    gpEngine->OnTimerCheckUpdate();
}

void CLogTimer::OnTimer()
{
    gpEngine->OnTimerLog();
}