#pragma once
#include <map>
#include <string>
#include "nxrmusrlog.h"
#include "LogThread.h"


using namespace std;



//typedef struct {
//	wstring searchStr;
//	int nFileNumber;
//} RECORD_STR_SCH_KEY;
//
//typedef struct {
//	INT searchID;
//	int nFileNumber;
//} RECORD_STR_SCH_KEY;




//#define NL_LOG_EXT L".ndb" extension will be defined in the caller.

class UserLogManager : public CLog
{
private:
	
	int nCurrentAppendFile;
	long inserbLocation;
	HANDLE m_FileHandle;
	void *pFileHeaderAccess;
	int m_entrySize;

public:
	
	UserLogManager();
	~UserLogManager();
	bool Initialize(const wchar_t szCallerDesc[], const wchar_t szLogFileName[], int nNumberOfFieldsInLogRecords, const USER_LOG_FIELD_SPEC rcdInfo[], int nLogEntrySize, int nMaxLenInMegaBytes);
	int LogUserData(void *pData, const char *JsonString);
	int NumberOfLogEntries(); //This function will find how many log entries in current log file.
	//bool GetOneLogEntry(void *pData, int nBufferLen, int nIndex, USER_LOG_EXTRA_INFO *pExtraInfo);
	bool GetRecordsByField(const USER_LOG_FIELD_VAL &queryVal, vector<vector<char>> &logArray, int nCount);

	VOID Start();

	VOID Stop();


};

