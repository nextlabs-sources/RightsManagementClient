#include "..\inc\LogService.h"
#
#include <fstream>
#include "LogService.h"
#include <nudf\web\json.hpp>
#include "nxrmusrlog.h"
#include <iostream>
#include <stdlib.h>

using namespace user_logging;
using namespace NX::web::json;

LogService::LogService() : m_bInitialized(false)
{

};


bool user_logging::LogService::Initialize(wchar_t *szUser)
{



	if (m_bInitialized)
	{//If we aready initialized we'll return false here
		return false;
	}

	m_ManagerID = NxrmulInitialize(L"NextLabs Service",
		(wstring(L"c:\\users\\eli\\nextlabslogging\\") + szUser + L"\\" + szUser + L".ndb").c_str(),
		sizeof(m_RecordFields)/ sizeof(USER_LOG_FIELD_SPEC), (USER_LOG_FIELD_SPEC *)m_RecordFields, sizeof(LogDbStruct), 2);
	if (m_ManagerID == ERROR_INITIALIZATION_FAILED)
	{
	}
	else if (m_ManagerID == ERROR_EXCEEDED_MAXIMUM)
	{
	}
	else
	{
		m_bInitialized = true;
	}

	return m_bInitialized;
}

bool user_logging::LogService::SetLogEntry(const LogDbStruct & logData)
{
	m_Data = logData;
	return true;
}

bool LogService::GetLogEntryByIndex(int nIndex, LogDbStruct *logData, wstring &JSONstring)
{

	bool bRet = false;
	USER_LOG_EXTRA_INFO logInfo;
	if (!m_bInitialized) return false;

	if (NxrmulGetOneLogEntry(m_ManagerID, logData, sizeof(LogDbStruct), nIndex, &logInfo))
	{
		string jsonStr;
		fstream jsonFile;
		jsonFile.open(logInfo.szJsonPath, std::fstream::in | std::fstream::out);
		wcout << logInfo.szJsonPath << endl;
		getline(jsonFile, jsonStr);
		jsonFile.close();
		JSONstring = NX::utility::conversions::utf8_to_utf16(jsonStr);
		bRet = true;
	}
	return bRet;
}

bool LogService::GetLogEntryByIndex(int nIndex, LogData &logData)
{
	bool bRet = false;
	USER_LOG_EXTRA_INFO logInfo;
	if (!m_bInitialized) return false;

	if (NxrmulGetOneLogEntry(m_ManagerID, (LogDbStruct*)&(logData.dbData), sizeof(LogDbStruct), nIndex, &logInfo))
	{
		string jsonStr;
		fstream jsonFile;
		jsonFile.open(logInfo.szJsonPath, std::fstream::in | std::fstream::out);
		wcout << logInfo.szJsonPath << endl;
		getline(jsonFile, jsonStr);
		jsonFile.close();
		wstring JSONstring = NX::utility::conversions::utf8_to_utf16(jsonStr);
		logData.jsonValue = NX::web::json::value::parse(JSONstring);
		bRet = true;
	}
	return bRet;
}


bool LogService::GetEntryByLogID(__int64 nID, LogDbStruct & logData, NX::web::json::value &JSONValue)
{
	USER_LOG_FIELD_VAL queryVal;
	wcscpy_s(queryVal.szFieldName, _countof(queryVal.szFieldName), m_RecordFields[0].szFieldName);
	queryVal.Val.llValue = nID;
	vector<vector<char>> logArray;
	bool bRet = NxrmulGetLogEntryByField(m_ManagerID, queryVal, logArray);
	if (bRet && logArray.size() > 0)
	{
		LogStructOutput *pData = (LogStructOutput *)(logArray.at(0).data());
		string jsonStr;
		fstream jsonFile;
		jsonFile.open(pData->internalState.szJsonPath, std::fstream::in | std::fstream::out);
		getline(jsonFile, jsonStr);
		jsonFile.close();
		wstring JSONstring = NX::utility::conversions::utf8_to_utf16(jsonStr);
		JSONValue = NX::web::json::value::parse(JSONstring);
		logData = pData->entryData;
	}
	else
	{
		bRet = false;
	}
	return bRet;
}


int LogService::GetLogEntryCount()
{
	if (!m_bInitialized) return 0;
	return NxrmulGetNumberOfLogEntries(m_ManagerID);
}

bool LogService::GetAllEntriesByType(wstring loggingType, vector<LogData> &logDataArr)
{
	return GetNumberOfEntriesByType(loggingType, logDataArr, NULL);
}

bool LogService::GetNumberOfEntriesByType(wstring loggingType, vector<LogData> &logDataArr, int nNumber)
{
	USER_LOG_FIELD_VAL queryVal;
	wcscpy_s(queryVal.szFieldName, _countof(queryVal.szFieldName), m_RecordFields[2].szFieldName);
	wcscpy_s(queryVal.Val.strValue, _countof(queryVal.szFieldName), loggingType.c_str());
	vector<vector<char>> logArray;
	bool bRet = NxrmulGetLogEntryByField(m_ManagerID, queryVal, logArray, nNumber); //NULL means to get all valUes
	logDataArr.clear();
	if (bRet && logArray.size() > 0)
	{
		for (auto v : logArray)
		{
			LogStructOutput *pData = (LogStructOutput *)(v.data());
			string jsonStr;
			fstream jsonFile;
			jsonFile.open(pData->internalState.szJsonPath, std::fstream::in | std::fstream::out);
			getline(jsonFile, jsonStr);
			jsonFile.close();
			wstring JSONstring = NX::utility::conversions::utf8_to_utf16(jsonStr);
			LogData tempData = { pData->entryData,  NX::web::json::value::parse(JSONstring) };
			logDataArr.push_back(move(tempData));
		}
	}
	else
	{
		bRet = false;
	}

	return bRet;

}



bool LogService::GetAllEntriesByType(wstring loggingType, vector<LogStructOutput> &logDataArr)
{
	return GetNumberOfEntriesByType(loggingType, logDataArr, NULL);
}

bool LogService::GetNumberOfEntriesByType(wstring loggingType, vector<LogStructOutput> &logDataArr, int nNumber)
{
	USER_LOG_FIELD_VAL queryVal;
	wcscpy_s(queryVal.szFieldName, _countof(queryVal.szFieldName), m_RecordFields[2].szFieldName);
	wcscpy_s(queryVal.Val.strValue, _countof(queryVal.szFieldName), loggingType.c_str());
	logDataArr.clear();
	vector<vector<char>> logArray;
	bool bRet = NxrmulGetLogEntryByField(m_ManagerID, queryVal, logArray, nNumber); //NULL means to get all valUes
	for (auto v : logArray)
	{
		LogStructOutput *pData = (LogStructOutput *)v.data();
		logDataArr.push_back(*pData);
	}

	return bRet;

}



int LogService::WriteLogEntry(const LogData & logData)
{
	if (!m_bInitialized) return 0;
	m_Data = logData.dbData;
	return WriteLogJson(logData.jsonValue);
}

int LogService::WriteLogJson(const char *JSONstring)
{
	if (!m_bInitialized) return 0;
	return NxrmulWriteLogEntry(m_ManagerID, &m_Data, JSONstring);
}


int LogService::WriteLogJson(const NX::web::json::value &JSONValue)
{

	if (!m_bInitialized) return 0;
	LogDbStruct dbData;
	std::wstring ws = JSONValue.serialize();
	std::string s = NX::utility::conversions::utf16_to_utf8(ws);
	try
	{
		dbData.uid = JSONValue.at(JSON_UID_INDEX).as_integer();
		dbData.timeStamp = JSONValue.at(JSON_TIMESTAMP_INDEX).as_integer();
		wcscpy_s(dbData.loggingType, _countof(dbData.loggingType), JSONValue.at(JSON_LOGGINGTYPE_INDEX).as_string().c_str());
		wcscpy_s(dbData.rights, _countof(dbData.rights), JSONValue.at(JSON_RIGHTS_INDEX).as_string().c_str());
		wcscpy_s(dbData.operation, _countof(dbData.operation), JSONValue.at(JSON_OPERATION_INDEX).as_string().c_str());
	}
	catch (exception e)
	{
		return false;
	}
	return NxrmulWriteLogEntry(m_ManagerID, &dbData, s.c_str());
}

LogService::~LogService()
{
	if (m_bInitialized)
	{
		NxrmulUninitalize(m_ManagerID);
	}
}
