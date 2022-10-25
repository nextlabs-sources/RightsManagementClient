

#ifndef __NUDF_LOG_HPP__
#define __NUDF_LOG_HPP__



#include <string>
#include <queue>
#include <memory>
#include <nudf\shared\logdef.h>
#include "nxrmusrlog.h"

#define MIN_LOG_QUEUE_SIZE      256
#define MAX_LOG_QUEUE_SIZE      4096
#define DEFAULT_LOG_QUEUE_SIZE  1024

class LogFileAccess;

class CLog
{
public:
    CLog();
    virtual ~CLog();

    virtual VOID Start();
    virtual VOID Stop();

    inline LOGLEVEL GetAcceptLevel() const throw() {return _AcceptLevel;}
    inline BOOL AcceptLevel(_In_ LOGLEVEL Level) const throw() {return (_AcceptLevel < Level) ? FALSE : TRUE;}
    inline ULONG GetQueueSize() const throw() {return _QueueSize;}
    inline VOID SetAcceptLevel(_In_ LOGLEVEL Level) throw() {_AcceptLevel = Level;}
    inline VOID SetQueueSize(_In_ ULONG Size) throw() {_QueueSize = (Size < MIN_LOG_QUEUE_SIZE) ? MIN_LOG_QUEUE_SIZE : ((Size > MAX_LOG_QUEUE_SIZE) ? MAX_LOG_QUEUE_SIZE : Size);}
    inline BOOL IsRunning() const throw() {return _Active;}

	bool GetOneLogEntry(void *pData, int nBufferLen, int nIndex, USER_LOG_EXTRA_INFO *pExtraInfo) throw();
	bool GetRecordsByField(const USER_LOG_FIELD_VAL & queryVal, vector<vector<char>>& logArray, int nCount);
	//bool GetRecordsByField(const USER_LOG_FIELD_VAL &filedVal, const USER_LOG_FIELD_VAL &queryVal, vector<vector<char>> &logArray, int nCount) throw();

    //BOOL Push(_In_ const CLogEntry& le) throw();
    BOOL Push(void *Info, const char *JsonString) throw();
    DWORD WorkerThread() throw();
	void SetFileHeaderAccess(void *pHeader)
	{
		_FileHeaderAccess = pHeader;
	}
	void SetLogEntrySize(int size)
	{
		_LogEntrySize = size;
	}

protected:
    //virtual VOID LogEntry(_In_ const std::wstring& s) throw() = 0;
    
private:
    LOGLEVEL        _AcceptLevel;
    ULONG           _QueueSize;
    std::queue<std::shared_ptr<CHAR>> _Queue;
    CRITICAL_SECTION        _QueueLock;
    HANDLE          _Events[2];
    HANDLE          _Thread;
    BOOL            _Active;
	void *_FileHeaderAccess;
	int _LogEntrySize;
};



#endif  // #ifndef __NUDF_LOG_HPP__