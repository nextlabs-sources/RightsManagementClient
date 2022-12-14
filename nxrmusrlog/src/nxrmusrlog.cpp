// nxrmusrlog.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "nxrmusrlog.h"
#include "userlogmanager.h"
#include <mutex>

static map <int, shared_ptr<UserLogManager>> logManagers;
static int managerID = 0;
using namespace std;

static mutex lockInit;


int nxrmFindNextManagerID(int managerID)
{
	managerID++;
	if (managerID >= MAX_LOG_MANAGER)
	{
		managerID = 0;
	}
	return managerID;
}

int APIENTRY NxrmulInitialize(const wchar_t szCallerDesc[], const wchar_t szLogFileName[], int nNumberOfFieldsInLogRecords, const USER_LOG_FIELD_SPEC *rcdInfo, int nLogEntrySize, int nMaxLenInMegaBytes)
{
	lock_guard<mutex> lock(lockInit);  // make sure that this function does not preempt itsself

	if (logManagers.size() > MAX_LOG_MANAGER)
	{//We are limiting the UserLogManager to be under 10
		return ERROR_EXCEEDED_MAXIMUM;
	}
	
	auto pLogManager = shared_ptr<UserLogManager>(new UserLogManager());

	if ( pLogManager->Initialize(szCallerDesc, 
			szLogFileName, nNumberOfFieldsInLogRecords, rcdInfo, nLogEntrySize, nMaxLenInMegaBytes))
	{	
		int currentID = managerID;
		if (logManagers.size() == MAX_LOG_MANAGER)
		{
			return ERROR_EXCEEDED_MAXIMUM;
		}
		if (logManagers.find(currentID) != logManagers.end())
		{

			do {
				currentID = nxrmFindNextManagerID(managerID);
			} while (logManagers.find(currentID) != logManagers.end());
		}
		logManagers[managerID] = pLogManager;
		managerID= nxrmFindNextManagerID(managerID);;
		return currentID;
	}
	return ERROR_INITIALIZATION_FAILED;
}


//using namespace UserLog;

//Initialize function create or open the log file for write, The database can be write only by one process but can be read by many process at the same time, the smax tring length is 127 
//Parameter: szCallerDesc, this makes the program can write one or more database at the same time, this should be a readable string says "NextLab Service Log"
//parameter: nNumberOfFieldsInLogRecords, this data is to describe the the logging record, how many fields in the record
//parameter: rcdInfo, this data is to describe the detail of the data that callers used to LogUserData()
//Parameter: nMaxLenInMegaBytes, this is the size limit for the logging file. when user set this number, the size will not be bigger than the nunber, minimun is 10




//This function logs data into the file and returns the index of the entry index to the caller, 
//Caller can use this index to get the data of the record
int APIENTRY NxrmulWriteLogEntry(int managerID, void *pData, const char *JsonString)
{
	//_ASSERT(g_pLogManager != NULL);
	auto pLogManager = logManagers[managerID];
	if(pLogManager)
	{
		return pLogManager->LogUserData(pData, JsonString);
	}
	return 0;
}

//This function is provided to get How many log records are generated by the specified fieldData
//int  APIENTRY NxrmulGetRecordCountByFieldID(wchar_t szFieldName, const void *fieldData)
//{
//	return true;
//}

//Caller need to allocate memory for pData, size need to be nCountToFill * sizeof(USER_LOG_RECORD)
bool APIENTRY NxrmulGetLogEntryByField(int managerID, const USER_LOG_FIELD_VAL &queryVal, vector<vector<char>> &logArray, int nCount)
{
	//vector<vector<char>> logArray;
	auto pLogManager = logManagers[managerID];
	if (pLogManager)
	{
		return pLogManager->GetRecordsByField(queryVal, logArray, nCount);
	}
	return false;
}


bool APIENTRY NxrmulUninitalize(int managerID) //This function will destroy objects and destroy data and close log file.
{
	lock_guard<mutex> lock(lockInit);  // make sure that this function does not preempt itsself
	int retVal = (int)logManagers.erase(managerID);
	return retVal != 0;
}

int APIENTRY NxrmulGetNumberOfLogEntries(int managerID) //This function will find how many log entries in current log file.
{
	auto pLogManager = logManagers[managerID];
	return pLogManager->NumberOfLogEntries();
	return 0;
}

bool APIENTRY NxrmulGetOneLogEntry(int managerID, void *pData, int nBufferLen, int nIndex, USER_LOG_EXTRA_INFO *pExtraInfo) //This function will get one record my index
{
	
	auto pLogManager = logManagers[managerID];
	return pLogManager->GetOneLogEntry(pData, nBufferLen, nIndex, pExtraInfo);
}
