#
#include <fstream>
#include "LogService.h"
#include <nudf\web\json.hpp>
#include "nxrmusrlog.h"
#include <iostream>

using namespace user_logging;

LogService::LogService() : m_bInitialized(false)
{

	USER_LOG_FIELD_SPEC recordFields[] =
	{
			{ L"Log ID", INT64_TYPE, offsetuid, true },
			{ L"Time Stamp", INT64_TYPE, offsetTimeStamp, false },
			{ L"Log Type", STRING_TYPE, offsetLoggingType, false },
			{ L"Rights", STRING_TYPE, offsetRights, false },
			{ L"Operation", STRING_TYPE, offsetOperation, false },
			//Environment
			{ L"Second Since Last Heartbeat", INT_TYPE, offsetfEvironSecondsSinceLastHeartBeat, false },
			{ L"Session Type", STRING_TYPE, offsetstEnrironSessionType,  false },
			//User
			{ L"User Name", DOUBLE_TYPE, offsetUserName, false },
			{ L"User ID", BOOL_TYPE, offsetUserId, false },
			//Host
			{ L"Host Name", STRING_TYPE, offsetHostName, false },
			{ L"Host IPv4", STRING_TYPE, offsetHostIpv4,  false },
			//app
			{ L"Application Path", STRING_TYPE, offsetAppImagePath, false },
			{ L"Application Publisher", STRING_TYPE, offsetAppPublisher,  false }
	};
	int arrayCount = sizeof(recordFields) / sizeof(USER_LOG_FIELD_SPEC);
	for (int i = 0; i < arrayCount; i++)
	{ //I have compiler error when using move semantics, will come back to visit it.
		m_RecordFields[i] = recordFields[i];
	}

	//Resource
};


bool user_logging::LogService::Initialize(wchar_t *szUser)
{



	if (m_bInitialized)
	{//If we aready initialized we'll return false here
		return false;
	}

	m_ManagerID = NxrmulInitialize(L"NextLabs Service",
		(wstring(szUser) + L"\\" + szUser + L".ndb").c_str(),
		sizeof(m_RecordFields)/sizeof(USER_LOG_FIELD_SPEC), &m_RecordFields[0], sizeof(LogDbStruct), 1);
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
	wcscpy(queryVal.szFieldName, m_RecordFields[0].szFieldName);
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
	USER_LOG_FIELD_VAL queryVal;
	wcscpy(queryVal.szFieldName, m_RecordFields[2].szFieldName);
	wcscpy(queryVal.Val.strValue, loggingType.c_str());
	vector<vector<char>> logArray;
	bool bRet = NxrmulGetLogEntryByField(m_ManagerID, queryVal, logArray, NULL); //NULL means to get all valUes
	logDataArr.clear();
	if (bRet && logArray.size() > 0)
	{
		for(auto v : logArray)
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
	std::wstring ws = JSONValue.serialize();
	//return ws;
	std::string s = NX::utility::conversions::utf16_to_utf8(ws);

	return NxrmulWriteLogEntry(m_ManagerID, &m_Data, s.c_str());
}

LogService::~LogService()
{
	if (m_bInitialized)
	{
		NxrmulUninitalize(m_ManagerID);
	}
}
