

#include <Windows.h>
#include <stdio.h>
#include <assert.h>
#include <memory>

#include "stdafx.h"

#include <nudf\exception.hpp>
#include "LogThread.h"
#include "LogFileAccess.h"


static DWORD WINAPI LogThread(_In_ LPVOID Param);


CLog::CLog() : _AcceptLevel(LOGWARNING), _QueueSize(DEFAULT_LOG_QUEUE_SIZE), _Thread(NULL), _Active(FALSE)
{
    ::InitializeCriticalSection(&_QueueLock);
    _Events[0] = NULL;
    _Events[1] = NULL;
}

CLog::~CLog()
{
    ::DeleteCriticalSection(&_QueueLock);
    if(NULL != _Events[0]) {
        CloseHandle(_Events[0]);
    }
    if(NULL != _Events[1]) {
        CloseHandle(_Events[1]);
    }
}

VOID CLog::Start()
{
    DWORD dwThreadId = 0;

    if(NULL == _Events[0]) {
        _Events[0] = ::CreateEventW(NULL, FALSE, FALSE, NULL);
        if(NULL == _Events[0]) {
            throw WIN32ERROR();
        }
    }
    if(NULL == _Events[1]) {
        _Events[1] = ::CreateEventW(NULL, FALSE, FALSE, NULL);
        if(NULL == _Events[1]) {
            throw WIN32ERROR();
        }
    }

    _Active = TRUE;
    _Thread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)LogThread, this, 0, &dwThreadId);
    if(NULL == _Thread) {
        _Active = FALSE;
        throw WIN32ERROR();
    }
}

VOID CLog::Stop()
{
    _Active = FALSE;
    if(NULL != _Thread) {
        SetEvent(_Events[0]);
        WaitForSingleObject(_Thread, INFINITE);
        CloseHandle(_Thread);
        _Thread = NULL;
    }
}


//typedef struct {
//	std::shared_ptr<CHAR> storeInfo;
//	std::shared_ptr<CHAR> JSonString;
//} LoggingInfo;

BOOL CLog::Push(void *Info, const char *JsonString) throw()
{
    BOOL bSucceed = FALSE;
    if(NULL == Info ) {
        return FALSE;
    }
	::EnterCriticalSection(&_QueueLock);

	//We need one time stamp and one serial number for the record, add 16 bytes here
	size_t nJsonStringLen = strlen(JsonString) + 1;
	auto storeInfo = std::shared_ptr<CHAR>(new CHAR[_LogEntrySize + nJsonStringLen], 
				[](CHAR *p) {delete[] p;});

	memcpy(storeInfo.get(), Info, _LogEntrySize);
	strcpy_s(storeInfo.get() + _LogEntrySize, nJsonStringLen, JsonString);
		
    if(_Queue.size() < _QueueSize) {
		OUTMSG1("_Queue.size = %d\n", _Queue.size());
        _Queue.push(storeInfo);
        bSucceed = TRUE;
    }
    ::LeaveCriticalSection(&_QueueLock);
    SetEvent(_Events[1]);
    return bSucceed;
}

DWORD CLog::WorkerThread() throw()
{
    DWORD dwRet = 0;
    DWORD dwWait = 0;
    
    while(IsRunning()) {
    
        dwWait = WaitForMultipleObjects(2, _Events, FALSE, INFINITE);
        switch(dwWait)
        {
        case WAIT_OBJECT_0:
            // Stop Event
            assert(!IsRunning());
            if(_Active) {
                _Active = FALSE;
            }
            break;
        case (WAIT_OBJECT_0+1):
            // new LogEntry Event
            while(TRUE) 
			{
                ::EnterCriticalSection(&_QueueLock);
                if(!_Queue.empty())
				{
					size_t nEntryNumber = _Queue.size();
                    auto sInfo = _Queue.front();
                    _Queue.pop();
					void *pData = sInfo.get();
					((LogFileAccess*)_FileHeaderAccess)->LogUserData(sInfo.get(), sInfo.get() + _LogEntrySize);
				}
				::LeaveCriticalSection(&_QueueLock);
				if (_Queue.size() == 0) {
					break;
				}
            }
            break;
        default:
            dwRet = GetLastError();
            if(_Active) {
                _Active = FALSE;
            }
            break;
        }
    }

    return dwRet;
}

DWORD WINAPI LogThread(_In_ LPVOID Param)
{
    CLog* spLog = (CLog*)Param;
    return spLog->WorkerThread();
}


bool CLog::GetOneLogEntry(void *pData, int nBufferLen, int nIndex, USER_LOG_EXTRA_INFO *pExtraInfo) throw()
{
	bool bRet = false;
	::EnterCriticalSection(&_QueueLock);
	bRet = ((LogFileAccess*)_FileHeaderAccess)->GetOneLogEntry(pData, nBufferLen, nIndex, pExtraInfo);
	::LeaveCriticalSection(&_QueueLock);
	return bRet;
}

bool CLog::GetRecordsByField(const USER_LOG_FIELD_VAL &queryVal, vector<vector<char>> &logArray, int nCount)
{
	bool bRet = false;
	::EnterCriticalSection(&_QueueLock);
	bRet = ((LogFileAccess*)_FileHeaderAccess)->GetRecordsByField(queryVal, logArray, nCount);
	::LeaveCriticalSection(&_QueueLock);
	return bRet;

}


