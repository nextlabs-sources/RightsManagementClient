#pragma once
#include <winbase.h>
#include <vector>
#include <map>
using namespace std;

//This is header file to provide functions as a DLL, to write user log data to either local file or to right management server
enum FieldType {INT_TYPE, INT64_TYPE, STRING_TYPE, FLOAT_TYPE, DOUBLE_TYPE, BOOL_TYPE, SHORT_TYPE};
#define MAX_FIELD_NAME_LEN	32
//We allow maximun 100 field for each record, I do think it is enough.
#define MAX_NUM_OF_REC_FIELDS  100

#define	MAX_LOG_RECORD_SIZE		8192		//8K

typedef struct
{
	wchar_t szFieldName[MAX_FIELD_NAME_LEN];
	FieldType fieldType;
	int nOffset;
	bool bBuildIdx;					//Specify this field build memory index, limit to one field at this time as we control the footprint for memery usage
} USER_LOG_FIELD_SPEC;

typedef struct
{
	wchar_t szFieldName[MAX_FIELD_NAME_LEN];
	union valUnion {
		wchar_t strValue[MAX_PATH];
		int iValue;
		float fValue;
		double dbValue;
		bool bValue;
		__int64 llValue;
		short stValue;
	} Val;
} USER_LOG_FIELD_VAL; 

typedef struct
{
	FieldType fieldType;
	int nFieldSize;
	int nOffset;
	bool bMapValid;
	struct {
		multimap<wstring, long> StrMap;
		multimap<int, long> IntMap;
		multimap<__int64, long> Int64Map;
	} queryMap;
	//struct myMap {
	//	multimap<wstring, long> StrMap;
	//	multimap<int, long> IntMap;
	//	multimap<__int64, long> Int64Map;
	//} queryMap;
} INTERNAL_FIELD_SPEC;


typedef struct
{
	long long logWritingTime;
	long long logSerialNumber;			//We start to write log from 1, and keep increment
	int nJSONSize;
	wchar_t szJsonPath[MAX_PATH];
} USER_LOG_EXTRA_INFO;



#define GetOffset(DataRecord, DataField) (int)((char *)&DataRecord.DataField - (char *)&DataRecord);

//
#define MINIMUN_LOG_FILE_SIZE	10		//Any size smaller than 10, will be setting to 10
//Initialize function create or open the log file for write, this function support mutiple logging in one process, logging can be in different process. the max string length is 127 
//Parameter: szCallerDesc, this makes the program can write one or more database at the same time, this should be a readable string says "NextLab Service Log"
//parameter: nNumberOfFieldsInLogRecords, this data is to describe the the logging record, how many fields in the record
//parameter: rcdInfo, this data is to describe the detail of the data that callers used to LogUserData()
//Parameter: nMaxLenInMegaBytes, this is the size limit for the logging file. when user set this number, the size will not be bigger than the nunber, minimun is 10
//The prefix NxrmUl here means "next labs right management user log"
//The return value is the LogManager index, use need to use this number to call other functions to point to the correct log file

#define MAX_LOG_MANAGER	1000 //For one process only 10 initialization is allowd for logging 10 files at the same time
#define ERROR_EXCEEDED_MAXIMUM	-1 //If one process tries to create more than 10 the the initialize function will return this value
#define ERROR_INITIALIZATION_FAILED -2 //If initialization is filed because of the file operation error, caller can use GetLastError to find what is exactly wrong

int APIENTRY NxrmulInitialize(const wchar_t szCallerDesc[], const wchar_t szLogFileName[], int nNumberOfFieldsInLogRecords, const USER_LOG_FIELD_SPEC rcdInfo[], int nLogEntrySize, int nMaxLenInMegaBytes);
//SearchFieldID APIENTRY NxrmulCreateSearchField( wchar_t szFieldName ); //This will cause the program to build mapping array to get data faster, return an SearchFieldID
//bool APIENTRY NxrmulDestroySearchField(wchar_t szFieldName); //This will cause the program to build mapping array to get data faster, return an SearchFieldID
int APIENTRY NxrmulWriteLogEntry(int managerID, void *pData, const char *JsonString); ////This name is limited to 31 unicode character, anything more than that will be truncated

//int  APIENTRY NxrmulGetRecordCountByField(wchar_t szFieldName, const void *fieldData); //This function is provided to get How many log records are generated by the specified fieldData, this data type is defined by search field that has be sent at initialization time


//nCount here means how many number of entry caller requesting. If nCount = 0, means that it requesting all entries for the search criteria.
bool APIENTRY NxrmulGetLogEntryByField(int managerID, const USER_LOG_FIELD_VAL &queryVal, vector<vector<char>> &logArray, int nCount=1);

bool APIENTRY NxrmulUninitalize(int managerID); //This function will destroy objects and destroy data and close log file.

int APIENTRY NxrmulGetNumberOfLogEntries(int managerID); //This function will find how many log entries in current log file.

bool APIENTRY NxrmulGetOneLogEntry(int managerID, void *pData, int nBufferLen, int nIndex, USER_LOG_EXTRA_INFO *pExtraInfo = NULL); //This function will get one record my index
