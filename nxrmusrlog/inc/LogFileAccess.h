#pragma once
#include "UserLogManager.h"
typedef struct {
	wchar_t szNextLabToken[32];
	wchar_t szCallerName[32]; //Process calling different 
	int nTotalRecords;
	int nLogRecordSize;
	int nPhysLogEntrySize;
	unsigned int nMaxFileSpace;
	int nStartLogTime;
	int nStartLocation;
	int nEndLocation;
	bool bFullyFilled;		//This means that the file is full filled with log
	int nLastSerialNumber;	//Last logging entry serial number
	wchar_t szJsonPath[MAX_PATH];	//Jason path
	int nTotalJasonSize;
	int nNumOfFields;
	USER_LOG_FIELD_SPEC *pFieldsData;
} CONTROLER_FILE_DATA;

#define BYTES_IN_MEGABYTE	1024 * 1024
#define MIN_LOG_ENTRY_SIZE	1024
#define BYTES_IN_KILOBYTE	1024

#define MAX_LOG_SPACE_SIZE	2048		//I have defined 2 GB as maximum log space

class LogFileAccess
{
private:
	CONTROLER_FILE_DATA m_ControlerFileData;
	map<wstring, shared_ptr<INTERNAL_FIELD_SPEC>> m_fieldInfo;
	vector<shared_ptr<INTERNAL_FIELD_SPEC>> m_indexedFields;

	HANDLE m_FileHandle;
	wstring m_logFileName;
	const int nHeaderSize = 8192;	//Make header size to be 8K
	//int nPhysRecordSize;	//Physical record size, this value will be added 4k according to the size of the 
	const int nExtraInfoSize = sizeof(USER_LOG_EXTRA_INFO);		//This is the space for extra info added each log entry

	int ComparefieldValue(char *entryData, const INTERNAL_FIELD_SPEC &spec, const USER_LOG_FIELD_VAL &queryVal);
	int WriteJsonFile(const wchar_t *szJsonFileName, const char *JsonString);
	bool EraseCurrentJson(void *pData);
	bool InitializeMutiMap(shared_ptr<INTERNAL_FIELD_SPEC>& fieldSpec);
	bool InsertToIndexMap(shared_ptr<INTERNAL_FIELD_SPEC> &fieldSpec, const CHAR *pLogRecord, const int nRecLocation);

public:
	~LogFileAccess();
	bool BuildMultiMap();

	bool AddNewRecordToIndex(CHAR * recBuffer, int nLocation);

	bool ErasePair(shared_ptr<INTERNAL_FIELD_SPEC>& fieldSpec, int val, int nLogLocation);

	bool ErasePair(shared_ptr<INTERNAL_FIELD_SPEC>& fieldSpec, __int64 val, int nLogLocation);

	bool ErasePair(shared_ptr<INTERNAL_FIELD_SPEC>& fieldSpec, wstring val, int nLogLocation);

	bool ErasePair(int val, int nLogLocation);

	bool RemoveFromIndexMap(shared_ptr<INTERNAL_FIELD_SPEC>& fieldSpec, CHAR * pLogRecord, int nLogLocation);

	bool RemoveRecordFromIndex(CHAR * recBuffer, int nLogLocation);
	
	//bool RemoveRecordFromIndex(CHAR * recBuffer);

	void SaveToFieldMap();
	LogFileAccess();

	bool BuildIndexForFields();

	bool IsFileOldFormat(int nNumberOfFieldsInLogRecords, const USER_LOG_FIELD_SPEC rcdInfo[], int nMaxLenInMegaBytes);
	bool ReadFileHeader(HANDLE FileHandle, const wstring &logFileName);
	HANDLE CreateFileHeader(const wchar_t *szCallerDesc, const wchar_t *logFileName, int nNumberOfFieldsInLogRecords, const USER_LOG_FIELD_SPEC rcdInfo[], int nLogEntrySize, int nMaxLenInMegaBytes);
	int  LogUserData(void *pData, const char *JsonString); //It returns the index of the inserted entry index
	bool LogUserDataStart();		//Initialize the file with 0 records.
	bool UpdateHeaderInfo();
	bool ChangeRotateWhenEnd();

	int NumberOfLogEntries() //This function will find how many log entries in current log file.
	{
		return m_ControlerFileData.nTotalRecords;
	}
	bool GetOneLogEntry(void *pData, int nBufferLen, int nIndex, USER_LOG_EXTRA_INFO *pExtraInfo);
	bool GetRecordsByField(const USER_LOG_FIELD_VAL & queryVal, vector<vector<char>>& logArray, int nCount);
	bool PushValueToArray(long nLocation, vector<vector<char>>& logArray);
	bool SearchIndexByField(const shared_ptr<INTERNAL_FIELD_SPEC>& field, const USER_LOG_FIELD_VAL & queryVal, vector<vector<char>>& logArray, int nCount);
	//bool SearchFileByField(const USER_LOG_FIELD_VAL & queryVal, const INTERNAL_FIELD_SPEC & spec, vector<vector<char>>& logArray, int nCount, bool bFirstInFirstOut);
	bool SearchFileByField(const USER_LOG_FIELD_VAL & queryVal, const INTERNAL_FIELD_SPEC & spec, vector<vector<char>>& logArray, int nCount);
	//bool GetRecordsByField(const USER_LOG_FIELD_VAL & fieldVal, const USER_LOG_FIELD_VAL & queryVal, vector<vector<char>>& logArray, int nCount);
	int GetLogEntrySize() { return m_ControlerFileData.nLogRecordSize + nExtraInfoSize; };
	//bool GetRecordsByField(const INTERNAL_FIELD_SPEC &spec, const USER_LOG_FIELD_VAL &queryVal, vector<vector<char>> &logArray);
	bool GetOneRecordsByField(const INTERNAL_FIELD_SPEC & spec, const USER_LOG_FIELD_VAL & queryVal, vector<char>& logArray, int nCount);
	bool DeleteAllJsonFiles();
};

