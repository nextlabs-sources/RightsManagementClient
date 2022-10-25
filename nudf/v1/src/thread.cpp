

#include <Windows.h>
#include <assert.h>

#include <nudf\exception.hpp>
#include <nudf\thread.hpp>


//using namespace nudf::util::thread;

typedef struct _NXTHREADCONTEXT {
    HANDLE  hEvent;
    DWORD   lResult;
    PVOID   pThis;
    PVOID   pUserContext;
} NXTHREADCONTEXT, *PNXTHREADCONTEXT;


DWORD WINAPI NuThreadStartRoutine(_In_ LPVOID Context);
DWORD WINAPI NuThreadStartRoutineEx(_In_ LPVOID Context);

nudf::util::thread::CThread::CThread() : _h(NULL)
{
}

nudf::util::thread::CThread::~CThread()
{
    Stop();
}

void nudf::util::thread::CThread::Start(_In_opt_ PVOID Context)
{
    NXTHREADCONTEXT ThreadContext = {NULL, 0, this, Context};
    DWORD dwThreadId = 0;

    if(NULL != _h) {
        throw WIN32ERROR2(ERROR_INVALID_STATE);
    }

    ThreadContext.hEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
    if(NULL == ThreadContext.hEvent) {
        throw WIN32ERROR();
    }

    try {

        _h = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)NuThreadStartRoutine, &ThreadContext, 0, &dwThreadId);
        if(NULL == _h) {
            throw WIN32ERROR();
        }
        (VOID)::WaitForSingleObject(ThreadContext.hEvent, INFINITE);
        if(0 != ThreadContext.lResult) {
            throw WIN32ERROR2(ThreadContext.lResult);
        }
        CloseHandle(ThreadContext.hEvent);
        ThreadContext.hEvent = NULL;
    }
    catch(const nudf::CException& e) {
        CloseHandle(ThreadContext.hEvent);
        ThreadContext.hEvent = NULL;
        if(NULL != _h) {
            CloseHandle(_h);
            _h = NULL;
        }
        throw e;
    }
}

void nudf::util::thread::CThread::Stop() throw()
{
    if(NULL != _h) {
        ::WaitForSingleObject(_h, INFINITE);
        CloseHandle(_h);
        _h = NULL;
    }
}

DWORD WINAPI NuThreadStartRoutine(_In_ LPVOID Context)
{
    PNXTHREADCONTEXT ThreadContext = (PNXTHREADCONTEXT)Context;
    assert(NULL != ThreadContext);
    LPVOID UserContext = ThreadContext->pUserContext;
    nudf::util::thread::CThread* ThreadObj = (nudf::util::thread::CThread*)ThreadContext->pThis;

    ThreadContext->lResult = ThreadObj->OnStart(UserContext);
    SetEvent(ThreadContext->hEvent);
    ThreadContext = NULL;

    return ThreadObj->OnRunning(UserContext);
}


nudf::util::thread::CThreadEx::CThreadEx() : CThread(), _bExternalJobEvent(FALSE)
{
    _hEvents[0] = NULL;
    _hEvents[1] = NULL;
}

nudf::util::thread::CThreadEx::~CThreadEx()
{
    Stop();
}

void nudf::util::thread::CThreadEx::Start(_In_opt_ HANDLE hJobEvent, _In_opt_ PVOID Context)
{
    NXTHREADCONTEXT ThreadContext = {NULL, 0, this, Context};
    DWORD dwThreadId = 0;

    if(NULL != _h) {
        throw WIN32ERROR2(ERROR_INVALID_STATE);
    }

    ThreadContext.hEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
    if(NULL == ThreadContext.hEvent) {
        throw WIN32ERROR();
    }

    try {

        _hEvents[0] = CreateEventA(NULL, TRUE, FALSE, NULL);       // Stop Event
        if(NULL == _hEvents[0]) {
            throw WIN32ERROR();
        }
        if(NULL != hJobEvent) {
            _hEvents[1] = hJobEvent;
            _bExternalJobEvent = TRUE;
        }
        else {
            _hEvents[1] = CreateEventA(NULL, TRUE, FALSE, NULL);    // Job Arrive Event
            if(NULL == _hEvents[1]) {
                throw WIN32ERROR();
            }
            _bExternalJobEvent = FALSE;
        }

        _h = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)NuThreadStartRoutineEx, &ThreadContext, 0, &dwThreadId);
        if(NULL == _h) {
            throw WIN32ERROR();
        }
        (VOID)::WaitForSingleObject(ThreadContext.hEvent, INFINITE);
        if(0 != ThreadContext.lResult) {
            throw WIN32ERROR2(ThreadContext.lResult);
        }
        CloseHandle(ThreadContext.hEvent);
        ThreadContext.hEvent = NULL;
    }
    catch(const nudf::CException& e) {
        CloseHandle(ThreadContext.hEvent);
        ThreadContext.hEvent = NULL;
        if(NULL != _hEvents[0]) {
            CloseHandle(_hEvents[0]);
            _hEvents[0] = NULL;
        }
        if(!_bExternalJobEvent && NULL != _hEvents[1]) {
            CloseHandle(_hEvents[1]);
        }
        _hEvents[1] = NULL;
        if(NULL != _h) {
            CloseHandle(_h);
            _h = NULL;
        }
        throw e;
    }
}

void nudf::util::thread::CThreadEx::Stop() throw()
{
    if(NULL != _h) {

        assert(NULL != _hEvents[0]);

        SetEvent(_hEvents[0]);
        CThread::Stop();

        CloseHandle(_hEvents[0]); _hEvents[0] = NULL;
        if(NULL!=_hEvents[1] && !_bExternalJobEvent) {
            CloseHandle(_hEvents[1]);
        }
        _hEvents[1] = NULL;
    }
}

DWORD WINAPI NuThreadStartRoutineEx(_In_ LPVOID Context)
{
    DWORD dwResult = 0;
    BOOL  bWorking = TRUE;
    PNXTHREADCONTEXT ThreadContext = (PNXTHREADCONTEXT)Context;
    assert(NULL != ThreadContext);
    LPVOID UserContext = ThreadContext->pUserContext;
    nudf::util::thread::CThreadEx* ThreadObj = (nudf::util::thread::CThreadEx*)ThreadContext->pThis;

    ThreadContext->lResult = ThreadObj->OnStart(UserContext);
    SetEvent(ThreadContext->hEvent);
    ThreadContext = NULL;

    do {

        DWORD dwWait = ::WaitForMultipleObjects(2, ThreadObj->GetEvents(), FALSE, INFINITE);
        switch(dwWait)
        {
        case WAIT_OBJECT_0:
            dwResult = 0;
            bWorking = FALSE;
            break;
        case (WAIT_OBJECT_0+1):
            (VOID)ThreadObj->OnRunning(UserContext);
            break;
        default:
            dwResult = dwWait;
            bWorking = FALSE;
            break;
        }
    } while(bWorking);

    return dwResult;
}