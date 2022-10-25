// testusrlog.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include <iostream>
#include "nxrmusrlog.h"
#include <crtdbg.h>
#include <time.h>
#include <vector>
#include <thread>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <mutex>
#include <nudf\web\json.hpp>

#include "LogService.h"


using namespace std;
using namespace user_logging;

typedef struct myEntry {
	int nID;		//User this field as a key to search
	long lTimeStamp;
	wchar_t szActionName[100];
	wchar_t szstringData[100];
	int nActionData;
	float fActionData;
	short stActionData;
	double dbActionData;
	bool bActionData;

	wchar_t szFilePath[MAX_PATH];

	bool operator==(myEntry b)
	{
		return (nID == b.nID && !lstrcmp(szActionName, b.szActionName));
	}

} LogEntry;


NX::web::json::value  CreateJSONValue(int uID);

typedef struct
{
	LogEntry entryData;
	USER_LOG_EXTRA_INFO internalState;
} OutData;


vector<LogEntry> writtenEntries;
vector<LogEntry> readEntries;

LogEntry record;

time_t timeStore;

LogEntry &InitializedEntry(LogEntry &record)
{
	record.lTimeStamp = (long)time(&timeStore);
	record.nID = 0;
	wstring actionName(L"User Login");
	actionName += 1;
	wcscpy_s(record.szActionName, _countof(record.szActionName), L"User Login");
	wcscpy_s(record.szstringData, _countof(record.szActionName), L"Login Name: joe");
	record.nActionData = 1;
	record.fActionData = 1.1f;
	record.stActionData = 100;
	record.dbActionData = 1.1;
	record.bActionData = true;
	wcscpy_s(record.szFilePath, _countof(record.szActionName), L"C:\\nextlabs\\Documents\\rightsmanagementclient\\testusrlog\\testusrlog\\MyTestLog.ndb");
	return record;
}


//void WriteDataSequentially(int managerID)
//{
//	GetInitializedEntry(record);
//	for (int i = 0; i < 50; i++)
//	{
//		wchar_t actionName[20] = L"User Login";
//		wsprintf(actionName, L"%s %d", L"User Login", i);
//		record.nID = i;
//		wcscpy_s(record.szActionName, _countof(record.szActionName), actionName);
//		if (!NxrmulWriteLogEntry(managerID, &record))
//		{
//			cout << "NxrmulLogUserData failed:" << i << "\n";
//		}
//		else
//		{
//			cout << "NxrmulLogUserData done:" << i << "\n";
//			writtenEntries.push_back(record);
//		}
//	}
//}

void GetEntriesByRecordID(int managerID)
{
	USER_LOG_FIELD_VAL queryVal = { L"Record ID", 10 };
	vector<vector<char>> logArray;
	if (NxrmulGetLogEntryByField(managerID, queryVal, logArray, 0))
	{
		for (auto v : logArray)
		{
			OutData *pData = (OutData *)v.data();

			std::time_t end_time = pData->internalState.logWritingTime / 1000000;
			double fractionSeconds = (double)(pData->internalState.logWritingTime % 1000000) / 1000000;

			//cout << "SerialNo=" << pData->internalState.logSerialNumber << " written time=" << ctime(&end_time)
			//	<< " " << fractionSeconds << " seconds ";

			wcout << L"ID: " << pData->entryData.nID << L"Action: " << pData->entryData.szActionName
				<< L"\n";
		}
	}

}

void GetEntriesByActivityVal(int managerID)
{
	USER_LOG_FIELD_VAL queryVal = { L"Activity Name", L"User Login 5"};
	vector<vector<char>> logArray;
	NxrmulGetLogEntryByField(managerID, queryVal, logArray);

	for (auto v : logArray)
	{
		OutData *pData = (OutData *)v.data();

		std::time_t end_time = pData->internalState.logWritingTime / 1000000;
		double fractionSeconds = (double)(pData->internalState.logWritingTime % 1000000) / 1000000;

		//cout << "SerialNo=" << pData->internalState.logSerialNumber << " written time=" << ctime(&end_time)
		//	<< " " << fractionSeconds << " seconds ";

		wcout << L"ID: " << pData->entryData.nID << L"Action: " << pData->entryData.szActionName
							<< L"\n";

	}
}

std::vector<std::thread> WriteLoginInMultiThreads()
{
	//LogEntry record;
	LogDbStruct logEntry = { 1,time(NULL),L"Operation",L"Evaluation", L"Encrypt",{ 0,L"Console" },
	{ L"Example Host Name", L"192.168.1.1" },{ L"c:\\windows\\notepad",L"Microsoft" } };

	std::vector<std::thread> threads;

	static mutex lockToWrite;
	LogService logSvc1;
	LogService logSvc2;

	if (logSvc1.Initialize(L"User1") && logSvc2.Initialize(L"User2"))
	{

		for (int i = 0; i < 50; ++i) {
			threads.push_back(std::thread([i, logEntry, &logSvc1, &logSvc2]() {
				//LogDbStruct rcd = logEntry;
				//rcd.uid = i;
				//wcscpy_s(rcd.loggingType, _countof(rcd.loggingType), L"My logging Type");

				auto data = CreateJSONValue(i);
				logSvc1.SetLogID(i);
				logSvc1.SetTimeStamp(time(NULL));
				logSvc1.SetLogEntry(logEntry);

				logSvc2.SetLogID(i);
				logSvc2.SetTimeStamp(time(NULL));
				logSvc2.SetLogEntry(logEntry);


				if (logSvc1.WriteLogJson(data) && logSvc2.WriteLogJson(data))
				{
					lock_guard<std::mutex> lock(lockToWrite);
					cout << "WriteLogJson failed:" << i << "\n";
				}
				else
				{
					lock_guard<std::mutex> lock(lockToWrite);
					cout << "NxrmulLogUserData done:" << i << "\n";

				}
				{
					lock_guard<std::mutex> lock(lockToWrite);
					std::cout << "Created a Rocord from thread " << std::this_thread::get_id() << std::endl;
				}

				Sleep(100);
				LogDbStruct mylogEntry;

				NX::web::json::value JSONValue;
				if (logSvc1.GetEntryByLogID(i, mylogEntry, JSONValue))
				{
					std::wcout << L"logSvc1 jsonstring=" << JSONValue.serialize() << std::endl;
				}

				if (logSvc2.GetEntryByLogID(i, mylogEntry, JSONValue))
				{
					std::wcout << L"logSvc2 jsonstring=" << JSONValue.serialize() << std::endl;
				}
			}));
		}
	}
	return threads;
}

void ReadLogEntriesSequentially(int managerID)
{

	int nTotalCount = NxrmulGetNumberOfLogEntries(managerID);
	int j = 0;
	if (nTotalCount)
	{
		for (int i=0; i < nTotalCount; i++)
		{

		
			LogEntry outRecord;
			USER_LOG_EXTRA_INFO logInfo;

			if (NxrmulGetOneLogEntry(managerID, &outRecord, sizeof(outRecord), i, &logInfo))
			{
				std::time_t end_time = logInfo.logWritingTime / 1000000;
				double fractionSeconds = (double)(logInfo.logWritingTime % 1000000) / 1000000;

				//cout << "SerialNo=" << logInfo.logSerialNumber << " written time=" << ctime(&end_time) 
				//	<< " " << fractionSeconds << " seconds ";
			
				wcout << L"ID: " << outRecord.nID << L" Action: " << outRecord.szActionName 
						<< L"\n"; 

				//while (writtenEntries[j].nID != outRecord.nID)
				//{
				//	j++;
				//}

				//if (writtenEntries[j] == outRecord)
				//{
				//	wcout << L"ID:" << outRecord.nID << L"Action:" << outRecord.szActionName << L"Is a correct one"
				//		<< L"\n";
				//}
				//else
				//{
				//	wcout << L"ID:" << outRecord.nID << L"Action:" << outRecord.szActionName << L"Is a not correct log"
				//		<< L"\n";
				//}
				//j++;

				//readEntries.push_back(outRecord);
			}
			else
			{
				cout << "Read Failed\n";
			}

		}
	}

}

//int main_test()
//{
//	int offsetID = GetOffset(record, nID);
//	int offsetTimeStamp = GetOffset(record, lTimeStamp);
//	int offsetszActionName = GetOffset(record, szActionName);
//	int offsetszstringData = GetOffset(record, szstringData);
//	int offsetnActionData = GetOffset(record, nActionData);
//	int offsetfActionData = GetOffset(record, fActionData);
//	int offsetstActionData = GetOffset(record, stActionData);
//	int offsetdbActionData = GetOffset(record, dbActionData);
//	int offsetbActionData = GetOffset(record, bActionData);
//	int offsetszFilePath = GetOffset(record, szFilePath);
//
//	USER_LOG_FIELD_SPEC myRecordFields[]= {
//
//		{ L"Record ID", INT_TYPE, offsetID, true },
//		{L"Time Stamp", INT_TYPE, offsetTimeStamp, false},
//		{ L"Activity Name", STRING_TYPE, offsetszActionName, false },
//		{ L"Activity Description", STRING_TYPE, offsetszstringData, false },
//
//		{ L"Integer Data", INT_TYPE, offsetnActionData, false },
//		{ L"Float Data", FLOAT_TYPE, offsetfActionData, false },
//		{ L"Short Data", SHORT_TYPE, offsetstActionData,  false },
//		{ L"long Data", DOUBLE_TYPE, offsetdbActionData, false },
//		{ L"bool Data", BOOL_TYPE, offsetbActionData, false},
//		{ L"File Path", STRING_TYPE, offsetszFilePath, false }
//
//	};
//	cout << "Call initialize\n";
//	int managerID = NxrmulInitialize(L"NextLabs Service",
//		L"MyTestLog.ndb", 
//		10, &myRecordFields[0], sizeof(LogEntry), 2);
//	if (managerID == ERROR_INITIALIZATION_FAILED)
//	{
//		cout << "Invalid file path\n";
//		exit(1);
//	} 
//	else if (managerID == ERROR_EXCEEDED_MAXIMUM)
//	{
//		cout << "Exceeded maximum = " << MAX_LOG_MANAGER << " loggings\n";
//		exit(1);
//	}
//
//	cout << "Writing user log data sequentially\n";
//	WriteDataSequentially(managerID);
//	cout << "Read user log data by Record ID\n";
//	GetEntriesByRecordID(managerID);
//	cout << "Read user log data by Action Value\n";
//	GetEntriesByActivityVal(managerID);
//	cout << "Writing user log data in multithreads\n";
//	auto threads = WriteLoginInMultiThreads(managerID);
//		
//	//ReadLogEntriesSequentially(managerID);
//
//	for (auto& thread : threads) {
//		thread.join();
//	}
//
//	NxrmulUninitalize(managerID);
//
//	int c = getchar();
//    return c == 'b' ? 1 : 0;
//}

string CreateJSONString(int uID)
{
	NX::web::json::value userLogInfo = NX::web::json::value::object();
	userLogInfo[L"uid"] = NX::web::json::value::number(uID);
	userLogInfo[L"timeStamp"] = NX::web::json::value::number(time(NULL));
	userLogInfo[L"LoggingType"] = NX::web::json::value::string(L"Evaluation");
	userLogInfo[L"Rights"] = NX::web::json::value::string(L"Evaluation");
	userLogInfo[L"Operation"] = NX::web::json::value::string(L"Encrypt");
	std::wstring ws = userLogInfo.serialize();
	//return ws;
	std::string s = NX::utility::conversions::utf16_to_utf8(ws);
	return s;
}

NX::web::json::value  CreateJSONValue(int uID)
{
	NX::web::json::value userLogInfo = NX::web::json::value::object();
	userLogInfo[JSON_UID_INDEX] = NX::web::json::value::number(uID);
	userLogInfo[JSON_TIMESTAMP_INDEX] = NX::web::json::value::string(L"12:00 pm, 2/24/2016");
	userLogInfo[JSON_LOGGINGTYPE_INDEX] = NX::web::json::value::string(L"Evaluation");
	userLogInfo[JSON_RIGHTS_INDEX] = NX::web::json::value::string(L"Evaluation");
	userLogInfo[JSON_OPERATION_INDEX] = NX::web::json::value::string(L"Encrypt");
	return userLogInfo;
}

NX::web::json::value  CreateJSONValue(int uID, time_t ts, wstring loggingType, wstring right, wstring operation )
{
	NX::web::json::value userLogInfo = NX::web::json::value::object();
	userLogInfo[JSON_UID_INDEX] = NX::web::json::value::number(uID);
	userLogInfo[JSON_TIMESTAMP_INDEX] = NX::web::json::value::number(ts);
	userLogInfo[JSON_LOGGINGTYPE_INDEX] = NX::web::json::value::string(loggingType);
	userLogInfo[JSON_RIGHTS_INDEX] = NX::web::json::value::string(right);
	userLogInfo[JSON_OPERATION_INDEX] = NX::web::json::value::string(operation);
	return userLogInfo;
}


bool testWritAndReadByJsonString()
{
	bool bRet = false;
	LogDbStruct logEntry = { 1,time(NULL),L"Operation",L"Evaluation", L"Encrypt",{ 0,L"Console" },
	{ L"Example Host Name", L"192.168.1.1" },{ L"c:\\windows\\notepad",L"Microsoft" } };

	LogService logSvc1;
	LogService logSvc2;
	//

	if (logSvc1.Initialize(L"User1"))
	{
		logSvc1.SetLogEntry(logEntry);
		cout << "logSvc1 Write in records:" << endl;
		for (int i = 0; i < 800; i++)
		{
			logSvc1.SetTimeStamp(time(NULL));
			string data = CreateJSONString(i);
			cout << data.c_str() << endl;

			logSvc1.SetLogID(i);
			logSvc1.WriteLogJson(data.c_str());
		}
		cout << "logSvc1 Read in records:" << endl;
		Sleep(100);
		int count = logSvc1.GetLogEntryCount();
		wstring jsonString;
		LogDbStruct myDBData;
		for (int i = 0; i < count; i++)
		{
			logSvc1.GetLogEntryByIndex(i, &myDBData, jsonString);
			wcout << jsonString << endl;
		}
		bRet = true;
	}
	else
	{
		cout << "Log initialize failed";
	}
	return bRet;
}


bool testWritAndReadByJsonValue()
{
	bool bRet = false;
	LogDbStruct logEntry = { 1,time(NULL),L"Operation",L"Evaluation", L"Encrypt",{ 0,L"Console" },
	{ L"Example Host Name", L"192.168.1.1" },{ L"c:\\windows\\notepad",L"Microsoft" } };

	LogService logSvc1;
	LogService logSvc2;
	//

	if (logSvc1.Initialize(L"User1"))
	{
		logSvc1.SetLogEntry(logEntry);
		cout << "logSvc1 Write in records:" << endl;
		for (int i = 0; i < 800; i++)
		{
			logSvc1.SetTimeStamp(time(NULL));
			auto data = CreateJSONValue(i);
			logSvc1.WriteLogJson(data);
		}
		cout << "logSvc1 Read in records:" << endl;
		Sleep(100);
		int count = logSvc1.GetLogEntryCount();
		wstring jsonString;
		LogDbStruct myDBData;
		for (int i = 0; i < count; i++)
		{
			logSvc1.GetLogEntryByIndex(i, &myDBData, jsonString);
			wcout << jsonString << endl;
		}
		bRet = true;
	}
	else
	{
		cout << "Log initialize failed";
	}
	return bRet;
}

bool testWriteAndReadByLogData()
{
	bool bRet = false;
	LogDbStruct logEntry = { 1,time(NULL),L"Operation",L"Evaluation", L"Encrypt",{ 0,L"Console" },
	{ L"Example Host Name", L"192.168.1.1" },{ L"c:\\windows\\notepad",L"Microsoft" } };

	LogService logSvc1;
	LogService logSvc2;
	//

	if (logSvc1.Initialize(L"User1"))
	{
		logSvc1.SetLogEntry(logEntry);
		cout << "logSvc1 Write in records:" << endl;
		for (int i = 0; i < 800; i++)
		{
			LogData MyData;
			logEntry.timeStamp = time(NULL);
			logEntry.uid = i;
			
			MyData.dbData = logEntry;
			MyData.jsonValue = CreateJSONValue(i);
			logSvc1.WriteLogEntry(MyData);
		}
		cout << "logSvc1 Read in records:" << endl;
		Sleep(100);
		int count = logSvc1.GetLogEntryCount();
		wstring jsonString;
		LogData myData;
		for (int i = 0; i < count; i++)
		{
			logSvc1.GetLogEntryByIndex(i, myData);
			wcout << myData.jsonValue.serialize() << endl;
		}
		bRet = true;
	}
	else
	{
		cout << "Log initialize failed";
	}
	return bRet;
}


int testWrite4Evalation()
{
	bool bRet = false;
	LogDbStruct logEntry = { 1,time(NULL),L"Operation",L"Evaluation", L"Encrypt",{ 0,L"Console" },
	{ L"Example Host Name", L"192.168.1.1" },{ L"c:\\windows\\notepad",L"Microsoft" } };

	LogService logSvc1;
	LogService logSvc2;
	int nCount = 0;
	//

	if (logSvc1.Initialize(L"User1"))
	{
		logSvc1.SetLogEntry(logEntry);
		cout << "logSvc1 Write in records:" << endl;
		for (int i = 0; i < 200; i++)
		{
			if (i % 2 == 0)
			{
				wcscpy_s(logEntry.loggingType, _countof(logEntry.loggingType), L"Debug");
			}
			else
			{
				if (i % 10 == 1)
				{
					wcscpy_s(logEntry.loggingType, _countof(logEntry.loggingType), L"Evaluation");
					nCount++;
				}
			}
			logSvc1.SetTimeStamp(time(NULL));
			wchar_t buffer[65];
			_itow(i, buffer, 10);
			auto data = CreateJSONValue(i, time(NULL), logEntry.loggingType, (wstring)L"right" + buffer, (wstring)L"operation" + buffer);
			//logSvc1.SetLogEntry(logEntry);
			//logSvc1.SetLogID(i);
			logSvc1.WriteLogJson(data);
		}
		cout << "logSvc1 Read in records:" << endl;
		Sleep(100);
		//int count = logSvc1.GetLogEntryCount();
		//wstring jsonString;
		//LogDbStruct myDBData;
		//for (int i = 0; i < count; i++)
		//{
		//	logSvc1.GetLogEntryByIndex(i, &myDBData, jsonString);
		//	wcout << jsonString << endl;
		//}
		//bRet = true;
	}
	else
	{
		cout << "Log initialize failed";
	}
	return nCount;
}


bool testGetEntryByID()
{
	bool bRet = false;
	LogDbStruct logEntry = { 1,time(NULL),L"Operation",L"Evaluation", L"Encrypt",{ 0,L"Console" },
	{ L"Example Host Name", L"192.168.1.1" },{ L"c:\\windows\\notepad",L"Microsoft" } };

	LogService logSvc1;
	LogService logSvc2;
	//

	if (logSvc1.Initialize(L"User1"))
	{
		logSvc1.SetLogEntry(logEntry);
		cout << "logSvc1 Write in records:" << endl;
		for (int i = 0; i < 10; i++)
		{
			if (i % 2 == 0)
			{
				wcscpy_s(logEntry.loggingType, _countof(logEntry.loggingType), L"Debug");
			}
			else
			{
				if (i % 10 == 1)
				{
					wcscpy_s(logEntry.loggingType, _countof(logEntry.loggingType), L"Evaluation");
				}
			}
			logSvc1.SetTimeStamp(time(NULL));
			auto data = CreateJSONValue(i);
			logSvc1.SetLogID(i);
			logSvc1.SetLogEntry(logEntry);
			logSvc1.WriteLogJson(data);
		}



		NX::web::json::value JSONValue;
		if(logSvc1.GetEntryByLogID(1, logEntry, JSONValue))
		{
			wcout << JSONValue.serialize() << endl;
			wcout << L"Entry ID = " << logEntry.uid << endl;
			bRet = true;
		}
		if (logSvc1.GetEntryByLogID(2, logEntry, JSONValue))
		{
			wcout << JSONValue.serialize() << endl;
			wcout << L"Entry ID = " << logEntry.uid << endl;
			bRet = true;
		}
		if (logSvc1.GetEntryByLogID(5, logEntry, JSONValue))
		{
			wcout << JSONValue.serialize() << endl;
			wcout << L"Entry ID = " << logEntry.uid << endl;
			bRet = true;
		}
	}
	else
	{
		cout << "Log initialize failed";
	}
	return bRet;
}

bool testGetAllEntresByType(int nCount)
{
	bool bRet = false;
	LogDbStruct logEntry = { 1,time(NULL),L"Operation",L"Evaluation", L"Encrypt",{ 0,L"Console" },
	{ L"Example Host Name", L"192.168.1.1" },{ L"c:\\windows\\notepad",L"Microsoft" } };

	LogService logSvc1;
	//

	if (logSvc1.Initialize(L"User1"))
	{
		logSvc1.SetLogEntry(logEntry);
		vector<NX::web::json::value> JSONArr;
		vector<LogStructOutput> logArr;
		if (logSvc1.GetNumberOfEntriesByType(L"Evaluation", logArr, 10))
		{
			if (logArr.size() == nCount)
			{
				cout << "Entry count match!!\n";
			}
			else
			{
				cout << "Entry count not match!!\n";
			}
			for (auto v : logArr)
			{
				wcout <<v.internalState.szJsonPath << endl;
				wcout << L"Entry ID = " << v.entryData.uid << endl;
				bRet = true;
			}
		}
	}
	else
	{
		cout << "Log initialize failed";
	}
	return bRet;
}

int main()
{

	//auto threads = WriteLoginInMultiThreads();
	//
	//for (auto& thread : threads) {
	//	thread.join();
	//}

	//testWriteAndReadByLogData();
	//testGetEntryByID();
	int nCount = testWrite4Evalation();
	testGetAllEntresByType(nCount);
	//testWrite4Evalation();
	//testWrite4Evalation();
	//testWrite4Evalation();
	//testWrite4Evalation();

	//while (true)
	//{
	//	int a = 0;
	//	try
	//	{
	//		if (main_test() == 1) break;
	//	}
	//	catch (exception e)
	//	{
	//		cout << e.what();
	//	}
	//	a = a;
	//}



	getchar();

	return 0;
}
