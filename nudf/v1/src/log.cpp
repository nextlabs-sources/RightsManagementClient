

#include <Windows.h>
#include <stdio.h>
#include <assert.h>

#include <nudf\exception.hpp>
#include <nudf\log.hpp>


using namespace nudf::util::log;

static DWORD WINAPI LogThread(_In_ LPVOID Param);

LPCSTR LevelToName(_In_ LOGLEVEL Level)
{
    switch(Level)
    {
    case LOGCRITICAL:
        return LOGNCRITICAL;
    case LOGERROR:
        return LOGNERROR;
    case LOGWARNING:
        return LOGNWARNING;
    case LOGINFO:
        return LOGNINFO;
    case LOGDEBUG:
        return LOGNDEBUG;
    case LOGDETAIL:
        return LOGNDETAIL;
    default:
        break;
    }

    if(Level >= LOGUSER) {
        return LOGNUSER;
    }

    return LOGNUNKNOWN;
}


CLogEntry::CLogEntry()
{
}

CLogEntry::CLogEntry(_In_ LOGLEVEL Level, _In_opt_ LPCSTR Module, _In_ LPCWSTR Info, ...)
{
    va_list args;
    int     len = 0;
    wchar_t *buffer = NULL;
    wchar_t header[256] = {0};
    SYSTEMTIME st;

    GetLocalTime(&st);
    if(NULL == Module) {
        Module = "UNK";
    }

    // Format: "[YYYYMMDDHHMMSSMMM MOD LVL PID.TID] "
    //   - YYYYMMDDHHMMSSMMM: Local Time
    //   - MOD: Module Name
    //   - LVL: Log Level Name
    //   - TID: Current Thread Id
    swprintf_s(header, 256, L"[%04d-%02d-%02d %02d:%02d:%02d.%03d %S %S %03X] ",
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
        Module, LevelToName(Level), GetCurrentThreadId());
    
    _Data = header;

    va_start(args, Info);
    len = _vscwprintf_l(Info, 0, args) + 1;
    buffer = new (std::nothrow) wchar_t[len];
    if(NULL != buffer) {
        vswprintf_s(buffer, len, Info, args ); // C4996
        _Data += buffer;
        delete []buffer;
        buffer = NULL;
    }
    else {
        _Data += L"__OutOfmemory__";
    }
    va_end(args);
}

CLogEntry::CLogEntry(_In_ LOGLEVEL Level, _In_opt_ LPCSTR Module, _In_ LONG Error, _In_ LPCSTR File, _In_ LPCSTR Function, _In_ LONG Line, _In_ LPCWSTR Info, ...)
{
    va_list args;
    int     len = 0;
    wchar_t *buffer = NULL;
    wchar_t header[256] = {0};
    wchar_t szNumber[64] = {0};
    SYSTEMTIME st;

    GetLocalTime(&st);
    if(NULL == Module) {
        Module = "UNK";
    }

    // Format: "[YYYYMMDDHHMMSSMMM MOD LVL PID.TID] "
    //   - YYYYMMDDHHMMSSMMM: Local Time
    //   - MOD: Module Name
    //   - LVL: Log Level Name
    //   - TID: Current Thread Id
    swprintf_s(header, 256, L"[%04d-%02d-%02d %02d:%02d:%02d.%03d %S %S %03X] ",
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
        Module, LevelToName(Level), GetCurrentThreadId());

    _Data = header;
    
    va_start(args, Info);
    len = _vscwprintf_l(Info, 0, args) + 1;
    buffer = new (std::nothrow) wchar_t[len];
    if(NULL != buffer) {
        vswprintf_s( buffer, len, Info, args ); // C4996
        _Data += buffer;
        delete []buffer;
        buffer = NULL;
    }
    else {
        _Data += L"__OutOfmemory__";
    }
    va_end(args);

    if(NULL != File) {
        std::string sFile(File);
        std::string sFunc(Function);
        _Data += L" (";
        _Data += std::wstring(sFile.begin(), sFile.end()); _Data += L", ";
        swprintf_s(szNumber, 63, L"%d", Line); _Data += szNumber; _Data += L", ";
        if(NULL != Function) {_Data += std::wstring(sFunc.begin(), sFunc.end()); _Data += L", ";}
        swprintf_s(szNumber, 63, L"%d", Error); _Data += szNumber;
        _Data += L")";
    }
}

CLogEntry::~CLogEntry()
{
}


//
//  class CLog
//

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

BOOL CLog::Push(_In_ const CLogEntry& le) throw()
{
    return Push(le.GetData().c_str());
}

BOOL CLog::Push(_In_ LPCWSTR Info) throw()
{
    BOOL bSucceed = FALSE;
    if(NULL == Info || '\0' == Info) {
        return FALSE;
    }
    ::EnterCriticalSection(&_QueueLock);
    if(_Queue.size() < _QueueSize) {
        _Queue.push(Info);
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
            while(TRUE) {
                std::wstring sInfo;
                ::EnterCriticalSection(&_QueueLock);
                if(!_Queue.empty()) {
                    sInfo = _Queue.front();
                    _Queue.pop();
                }
                ::LeaveCriticalSection(&_QueueLock);
                if(sInfo.length() == 0) {
                    break;
                }
                LogEntry(sInfo);
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

//
//  class CFileLog
//
static unsigned char ubf16bom[2] = {0xFF, 0xFE};

CFileLog::CFileLog() : CLog(), _FileHandle(INVALID_HANDLE_VALUE), _FileSize(DEFAULT_LOG_FILE_SIZE), _Rotation(DEFAULT_LOG_ROTATION), _LogExt(L".txt")
{
}

CFileLog::~CFileLog()
{
    Stop();
}

VOID CFileLog::Start()
{
    std::wstring wsFile;

    if(_LogName.length() == 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        throw WIN32ERROR();
    }

    if(IsRunning()) {
        SetLastError(ERROR_INVALID_HANDLE_STATE);
        throw WIN32ERROR();
    }

    wsFile = _LogDir + _LogName + _LogExt;
    _FileHandle = ::CreateFileW(wsFile.c_str(), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(INVALID_HANDLE_VALUE == _FileHandle) {
        throw WIN32ERROR();
    }
    
    try {

        if(GetLastError() == 0) {
            // File doesn't exist, and a new file is created
            // Write UNT16 BOM
            DWORD dwWrtn = 0;
            if(!::WriteFile(_FileHandle, ubf16bom, 2, &dwWrtn, NULL)) {
                throw WIN32ERROR();
            }
        }
        else {
            SetFilePointer(_FileHandle, 0, NULL, FILE_END);
        }

        CLog::Start();
    }
    catch(const nudf::CException& e) {
        if(INVALID_HANDLE_VALUE != _FileHandle) {
            CloseHandle(_FileHandle);
            _FileHandle = INVALID_HANDLE_VALUE;
        }
        throw e;
    }
}

VOID CFileLog::Stop()
{
    CLog::Stop();
    if(INVALID_HANDLE_VALUE != _FileHandle) {
        CloseHandle(_FileHandle);
        _FileHandle = INVALID_HANDLE_VALUE;
    }
}

VOID CFileLog::LogEntry(_In_ const std::wstring& s) throw()
{
    DWORD dwWritten = 0;
    static const char endl[4] = {0x0D, 0x00, 0x0A, 0x00};
    
    if(INVALID_HANDLE_VALUE == _FileHandle) {
        return;
    }

    if(s.length() == 0) {
        return;
    }

    ::WriteFile(_FileHandle, endl, 4, &dwWritten, NULL);
    ::WriteFile(_FileHandle, s.c_str(), ((DWORD)s.length())*sizeof(wchar_t), &dwWritten, NULL);

    if(GetFileSize(_FileHandle, NULL) >= _FileSize) {
        try {
            Rotate();
        }
        catch(const nudf::CException& e) {
            UNREFERENCED_PARAMETER(e);
        }
    }
}

VOID CFileLog::SetLogDir(_In_opt_ LPCWSTR Dir) throw()
{
    if(NULL == Dir || L'\0' == Dir[0]) {
        _LogDir = L"";
    }
    else {
        _LogDir = Dir;
        if(Dir[_LogDir.length() - 1] != L'\\') { // Not ends with L"\"
            _LogDir += L"\\";
        }
    }
}

VOID CFileLog::SetLogName(_In_ LPCWSTR Name) throw()
{
    if(NULL != Name && L'\0' != Name[0]) {
        _LogName = Name;
        if(_LogName.length() > 4 && 0 == _wcsicmp(&Name[_LogDir.length() - 4], L".txt")) {  // Ends with L".log"
            _LogName = _LogName.substr(0, _LogName.length() - 4);                           // Remove ".log"
        }
    }
}

LPCWSTR RotateToName(_In_ int n)
{
    switch(n)
    {
    case 0: return L"0";
    case 1: return L"1";
    case 2: return L"2";
    case 3: return L"3";
    case 4: return L"4";
    case 5: return L"5";
    case 6: return L"6";
    case 7: return L"7";
    case 8: return L"8";
    case 9: return L"9";
    default: break;
    }
    return L"";
}

VOID CFileLog::Rotate()
{
    std::wstring wsFile;
    std::wstring wsTarget;
    int nMax = _Rotation - 1;
    DWORD dwWrtn = 0;

    // Make sure file is closed
    if(INVALID_HANDLE_VALUE != _FileHandle) {
        CloseHandle(_FileHandle);
        _FileHandle = INVALID_HANDLE_VALUE;
    }

    // Rotate
    if(_Rotation > 0) {

        nMax = (int)_Rotation - 1;
        for(int i=0; i<=nMax; i++) {
            wsFile = _LogDir + _LogName; wsFile += RotateToName(nMax-1-i); wsFile += _LogExt;
            wsTarget = _LogDir + _LogName; wsTarget += RotateToName(nMax-i); wsTarget += _LogExt;
            ::MoveFileExW(wsFile.c_str(), wsTarget.c_str(), MOVEFILE_REPLACE_EXISTING);
        }
    }
    
    // Re-open file
    wsFile = _LogDir + _LogName + _LogExt;
    _FileHandle = ::CreateFileW(wsFile.c_str(), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(INVALID_HANDLE_VALUE == _FileHandle) {
        throw WIN32ERROR();
    }
    (VOID)::WriteFile(_FileHandle, ubf16bom, 2, &dwWrtn, NULL);
}
