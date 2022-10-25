#pragma once
#include "windows.h"
#include "time.h"
#include <string>
#include <nxrmusrlog.h>
#include <nudf\web\json.hpp>

using namespace std;

namespace user_logging
{
#define TYPE_NAME_LENGTH	20
#define RIGHT_LENGTH		20
#define OPERATION_LENGTH	20
#define SESSION_TYPE_LEN	20
#define	USER_NAME_LEN		30
#define USER_ID_LEN			20
#define	MAX_NUM_OF_ATTRIBUTES	10
#define	MAX_ATTRIBUTE_NAME_LEN	20
#define MAX_ATTRIBUTE_VALUE_LEN	20
#define MAX_ATTRIBUTE_TYPE_LEN	20
#define MAX_HOST_NAME_LEN		20
#define MAX_HOST_IPV4_LEN		20
#define	MAX_APPLICATION_PUBLISHER_LEN	40
#define MAX_NUM_OF_TAGS		10
#define MAX_TAG_NAME_LEN	20
#define	MAX_TAG_VALUE_LEN	40
#define MAX_TAG_TYPE_LEN	20
#define	MAX_NUM_OF_HIT_POLICIES		10
#define MAX_POLICY_NAME_LEN	50
#define MAX_NUM_OF_FILEDS			30

//Define Field name
#define JSON_UID_INDEX	L"uid" 
#define JSON_TIMESTAMP_INDEX	L"timeStamp" 
#define JSON_LOGGINGTYPE_INDEX	L"loggingType" 
#define JSON_RIGHTS_INDEX	L"rights" 
#define JSON_OPERATION_INDEX	L"operation" 
#define JSON_ENVSECONDS_INDEX	L"envSeconds" 
#define JSON_ENVSESSIONTYPE_INDEX	L"envSessionType" 
#define JSON_HOSTNAME_INDEX	L"hostName" 
#define JSON_APPLICATIONPATH_INDEX	L"applicationPath" 
#define JSON_APPPUBLISHER_INDEX	L"AppPublisher" 



	enum LoggingType {EVALUATION, OPERATION};

	typedef 
	struct {
		int secondsSinceLastHeartBeat;
		wchar_t szSessionType[SESSION_TYPE_LEN];
	
	} Enviroment;

	typedef
	struct {
		wchar_t szName[MAX_ATTRIBUTE_NAME_LEN];
		wchar_t szValue[MAX_ATTRIBUTE_VALUE_LEN];
		wchar_t szType[MAX_ATTRIBUTE_TYPE_LEN];
	} Attribute;

	typedef
	struct {
		wchar_t szName[USER_NAME_LEN];
		wchar_t szId[USER_ID_LEN];
		//int numOfAttributs;
		//Attribute attributes[MAX_NUM_OF_ATTRIBUTES];
	} User;

	typedef
	struct {
		wchar_t szName[MAX_HOST_NAME_LEN];
		wchar_t szIpv4[MAX_HOST_IPV4_LEN];
	} Host;
	
	typedef
	struct {
		wchar_t imagePath[MAX_PATH];
		wchar_t szPublisher[MAX_APPLICATION_PUBLISHER_LEN];

	} Appplication;
	
	typedef 
	struct {
		wchar_t szName[MAX_TAG_NAME_LEN];
		wchar_t szValue[MAX_TAG_VALUE_LEN];
		wchar_t szType[MAX_TAG_TYPE_LEN];

	} Tag;
	typedef
	struct {
		wchar_t szPath[MAX_PATH];
		//int numOfTags;
		//Tag tags[MAX_NUM_OF_TAGS];
	} Resource;
	
	typedef
	struct {
		int nId;
		wchar_t szName[MAX_POLICY_NAME_LEN];
	} HitPolicy;


	typedef struct LogDBData {
		__int64 uid;
		__int64 timeStamp;
		wchar_t loggingType[TYPE_NAME_LENGTH];
		wchar_t rights[RIGHT_LENGTH];
		wchar_t operation[OPERATION_LENGTH];
		Enviroment eviron;
		//User user;
		Host host;
		Appplication app;
		bool operator==(LogDBData b)
		{
			return (uid == b.uid && timeStamp == b.timeStamp);
		}
	} LogDbStruct;

	typedef struct logDataOut {
		LogDbStruct dbData;
		NX::web::json::value jsonValue;
	} LogData;


	typedef struct
	{
		LogDbStruct entryData;
		USER_LOG_EXTRA_INFO internalState;
	} LogStructOutput;




	class LogService
	{
		const int offsetuid = GetOffset(m_Data, uid);
		const int offsetTimeStamp = GetOffset(m_Data, timeStamp);
		const int offsetLoggingType = GetOffset(m_Data, loggingType);
		const int offsetRights = GetOffset(m_Data, rights);
		const int offsetOperation = GetOffset(m_Data, operation);
		const int offsetfEvironSecondsSinceLastHeartBeat = GetOffset(m_Data, eviron.secondsSinceLastHeartBeat);
		const int offsetstEnrironSessionType = GetOffset(m_Data, eviron.szSessionType);
		//const int offsetUserName = GetOffset(m_Data, user.szName);
		//const int offsetUserId = GetOffset(m_Data, user.szId);
		const int offsetHostName = GetOffset(m_Data, host.szName);
		const int offsetHostIpv4 = GetOffset(m_Data, host.szIpv4);

		const int offsetAppImagePath = GetOffset(m_Data, app.imagePath);
		const int offsetAppPublisher = GetOffset(m_Data, app.szPublisher);

		int m_ManagerID; 
		LogDbStruct m_Data;
		bool m_bInitialized;
		int nMaxLoggingSize;
		int nTotalLoggingsize;
		USER_LOG_FIELD_SPEC m_RecordFields[11]=
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
			//{ L"User Name", DOUBLE_TYPE, offsetUserName, false },
			//{ L"User ID", BOOL_TYPE, offsetUserId, false },
			//Host
			{ L"Host Name", STRING_TYPE, offsetHostName, false },
			{ L"Host IPv4", STRING_TYPE, offsetHostIpv4,  false },
			//app
			{ L"Application Path", STRING_TYPE, offsetAppImagePath, false },
			{ L"Application Publisher", STRING_TYPE, offsetAppPublisher,  false }
		};

		int m_ArrayCount;
	public:
		LogService();
		bool Initialize(wchar_t *szUser);
		bool SetLogEntry(const LogDbStruct &logData);
		void SetTimeStamp(time_t ts) { m_Data.timeStamp = ts; }
		void SetLogID(int ilogID) { m_Data.uid = ilogID;  }
		int WriteLogEntry(const LogData &logData);
		int WriteLogJson(const char *JsonStr);
		bool GetLogEntryByIndex(int nIndex, LogDbStruct * logData, wstring & JSONstring);
		bool GetLogEntryByIndex(int nIndex, LogData & logData);
		bool GetEntryByLogID(__int64 nID, LogDbStruct & logData, NX::web::json::value & JSONValue);
		int GetLogEntryCount();
		bool GetAllEntriesByType(wstring loggingType, vector<LogData>& logDataArr);
		bool GetNumberOfEntriesByType(wstring loggingType, vector<LogData> &logDataArr, int nNumber);
		bool GetAllEntriesByType(wstring loggingType, vector<LogStructOutput>& logDataArr);
		bool GetNumberOfEntriesByType(wstring loggingType, vector<LogStructOutput>& logDataArr, int nNumber);
		int WriteLogJson(const NX::web::json::value & JSONValue);
		~LogService();
	};
};

