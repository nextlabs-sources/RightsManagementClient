#include "stdafx.h"
#include "LogFileAccess.h"
#include <nudf\exception.hpp>
#include <chrono>
#include <shlwapi.h>
#include <fstream>
#include <locale>
#include <codecvt>

//std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

LogFileAccess::LogFileAccess()
{
	OUTMSG("FileHeaderAccess is called\n");
	m_ControlerFileData.pFieldsData = NULL;
}



LogFileAccess::~LogFileAccess()
{
	//DebugBreak();
	OUTMSG("~FileHeaderAccess is called\n");
	CloseHandle(m_FileHandle);
	delete[] m_ControlerFileData.pFieldsData;
}

bool LogFileAccess::BuildMultiMap()
{
	OUTMSG("BuildMultiMap started\n");
	if (m_ControlerFileData.nTotalRecords == 0 || m_indexedFields.size() == 0)
	{	//There is nothing we need to do if there is no records or no indexed fields
		return false;
	}

	//for (auto fieldData : m_indexedFields)
	//{
	//	InitializeMutiMap(fieldData);
	//}

	int nReadLocation = nHeaderSize;

	auto storeInfo = std::unique_ptr<CHAR, void(*)(CHAR*) >(new CHAR[GetLogEntrySize()],
		[](CHAR *p) {delete[] p; });
	
	for (int i = 0; i < m_ControlerFileData.nTotalRecords; i++)
	{

		SetFilePointer(m_FileHandle, nReadLocation, NULL, FILE_BEGIN);
		DWORD numOfBytesRead;
		int lastError;
		if (!ReadFile(m_FileHandle, storeInfo.get(), GetLogEntrySize(), &numOfBytesRead, NULL) || !numOfBytesRead)
		{
			lastError = GetLastError();
			OUTMSG2("m_FileHandle=%x Read end failed, lasterror = %d\n", m_FileHandle, lastError);
			return false;
		}
		OUTMSG1("m_FileHandle=%x, Read end success\n", m_FileHandle);
		AddNewRecordToIndex(storeInfo.get(), nReadLocation);
		nReadLocation += m_ControlerFileData.nPhysLogEntrySize;
	}

	return true;
}

bool LogFileAccess::AddNewRecordToIndex( CHAR *recBuffer, int nLocation)
{
	for (auto v : m_indexedFields)
	{
		InsertToIndexMap(v, recBuffer, nLocation);
	}
	return true;
}

bool LogFileAccess::ErasePair(shared_ptr<INTERNAL_FIELD_SPEC> &fieldSpec, int val, int nLogLocation)
{
	auto iterpair = fieldSpec->queryMap.IntMap.equal_range(val);

	auto it = iterpair.first;
	for (; it != iterpair.second; ++it)
	{
		if (it->second == nLogLocation)
		{
			fieldSpec->queryMap.IntMap.erase(it);
			return true;
		}
	}
	return false;
}

bool LogFileAccess::ErasePair(shared_ptr<INTERNAL_FIELD_SPEC> &fieldSpec, __int64 val, int nLogLocation)
{
	auto iterpair = fieldSpec->queryMap.Int64Map.equal_range(val);

	auto it = iterpair.first;
	for (; it != iterpair.second; ++it)
	{
		if (it->second == nLogLocation)
		{
			fieldSpec->queryMap.Int64Map.erase(it);
			return true;
		}
	}
	return false;
}

bool LogFileAccess::ErasePair(shared_ptr<INTERNAL_FIELD_SPEC> &fieldSpec, wstring val, int nLogLocation)
{
	auto iterpair = fieldSpec->queryMap.StrMap.equal_range(val);
	auto it = iterpair.first;
	for (; it != iterpair.second; ++it)
	{
		if (it->second == nLogLocation)
		{
			fieldSpec->queryMap.StrMap.erase(it);
			return true;
		}
	}
	return false;
}

bool LogFileAccess::RemoveFromIndexMap(shared_ptr<INTERNAL_FIELD_SPEC> &fieldSpec, CHAR *pLogRecord, int nLogLocation)
{
	switch (fieldSpec->fieldType)
	{
	case INT_TYPE:
	{
		int val = *(int *)(pLogRecord + fieldSpec->nOffset);
		_ASSERT(ErasePair(fieldSpec, val, nLogLocation));
		break;
	}
	case SHORT_TYPE:
	{
		short val = *(short *)(pLogRecord + fieldSpec->nOffset);
		_ASSERT(ErasePair(fieldSpec, val, nLogLocation));
		break;
	}
	case BOOL_TYPE:
	{
		BOOL val = *(BOOL *)(pLogRecord + fieldSpec->nOffset);
		_ASSERT(ErasePair(fieldSpec, val, nLogLocation));
		break;
	}
	case INT64_TYPE:
	{
		__int64 val = *(__int64 *)(pLogRecord + fieldSpec->nOffset);
		_ASSERT(ErasePair(fieldSpec, val, nLogLocation));
		break;
	}
	case STRING_TYPE:
	{
		wstring val = (wchar_t *)(pLogRecord + fieldSpec->nOffset);
		_ASSERT(ErasePair(fieldSpec, val, nLogLocation));
		break;
	}
	default:
		return false;
	}
	return true;
}

bool LogFileAccess::RemoveRecordFromIndex(CHAR *recBuffer, int nLogLocation)
{
	for (auto v : m_indexedFields)
	{
		RemoveFromIndexMap(v, recBuffer, nLogLocation);
	}
	return true;
}

void LogFileAccess::SaveToFieldMap()
{
	for (int i = 0; i < m_ControlerFileData.nNumOfFields; i++)
	{
		//Build fieldData map here
		auto fieldData = make_shared<INTERNAL_FIELD_SPEC>();
		fieldData->fieldType = m_ControlerFileData.pFieldsData[i].fieldType;
		fieldData->nOffset = m_ControlerFileData.pFieldsData[i].nOffset;
		fieldData->bMapValid = m_ControlerFileData.pFieldsData[i].bBuildIdx;
		if (fieldData->bMapValid)
		{
			m_indexedFields.push_back(fieldData);
		}
		m_fieldInfo[m_ControlerFileData.pFieldsData[i].szFieldName] = fieldData;
	}
}



bool LogFileAccess::ReadFileHeader(HANDLE FileHandle, const wstring &logFileName)
{
	m_FileHandle = FileHandle;
	m_logFileName = logFileName;
	if (INVALID_HANDLE_VALUE == m_FileHandle) {

		throw WIN32ERROR();
	}

	DWORD numOfBytesRead;
	int nDataSize = sizeof(m_ControlerFileData) - sizeof(USER_LOG_FIELD_SPEC *);
	if (!ReadFile(FileHandle, &m_ControlerFileData, nDataSize, &numOfBytesRead, NULL) || !numOfBytesRead)
	{
		return false;
	}
	
	m_ControlerFileData.pFieldsData = new USER_LOG_FIELD_SPEC[m_ControlerFileData.nNumOfFields];
	if (!ReadFile(FileHandle, m_ControlerFileData.pFieldsData,
		sizeof(USER_LOG_FIELD_SPEC) * m_ControlerFileData.nNumOfFields, &numOfBytesRead, NULL) || !numOfBytesRead)
	{
		return false;
	}

	SaveToFieldMap();
	BuildMultiMap();
	return true;

}

HANDLE LogFileAccess::CreateFileHeader(const wchar_t *szCallerDesc, const wchar_t *logFileName, int nNumberOfFieldsInLogRecords, const USER_LOG_FIELD_SPEC rcdInfo[], int nLogEntrySize, int nMaxLenInMegaBytes)
{
	m_FileHandle = ::CreateFileW(logFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH, NULL);
	if (INVALID_HANDLE_VALUE == m_FileHandle) {
		
		throw WIN32ERROR();
	}
	int dataSize = _countof(m_ControlerFileData.szNextLabToken);
	wcscpy_s(m_ControlerFileData.szNextLabToken, dataSize,
		L"Nextlabs User Log Data");
	m_logFileName = logFileName;
	dataSize = _countof(m_ControlerFileData.szCallerName);
	wcscpy_s(m_ControlerFileData.szCallerName, dataSize, szCallerDesc);

	nMaxLenInMegaBytes = min(nMaxLenInMegaBytes, MAX_LOG_SPACE_SIZE);	//limit to 2G space.

	m_ControlerFileData.nMaxFileSpace = nMaxLenInMegaBytes * BYTES_IN_MEGABYTE;
	m_ControlerFileData.nTotalRecords = 0;	
	m_ControlerFileData.nLogRecordSize= nLogEntrySize;
	m_ControlerFileData.nPhysLogEntrySize = ((nLogEntrySize + sizeof(USER_LOG_EXTRA_INFO)) / MIN_LOG_ENTRY_SIZE + 1) * MIN_LOG_ENTRY_SIZE;		//Record size to be multiple of 4k bytes.
	m_ControlerFileData.nStartLocation = nHeaderSize;
	m_ControlerFileData.nEndLocation = nHeaderSize;
	m_ControlerFileData.nNumOfFields = nNumberOfFieldsInLogRecords;
	m_ControlerFileData.nLastSerialNumber = 1;		//We start serial number from on for each file
	m_ControlerFileData.bFullyFilled = false;
	m_ControlerFileData.nTotalJasonSize = 0;
	lstrcpy(m_ControlerFileData.szJsonPath, logFileName);
	PathRemoveExtension(m_ControlerFileData.szJsonPath);

	//m_ControlerFileData.szJsonPath = FilePath;
	m_ControlerFileData.pFieldsData = new USER_LOG_FIELD_SPEC[m_ControlerFileData.nNumOfFields];
	int nDataSize = sizeof(m_ControlerFileData) - sizeof(USER_LOG_FIELD_SPEC *);
	DWORD numOfBytesWritten;
	int nFieldArraySize = sizeof(USER_LOG_FIELD_SPEC) * m_ControlerFileData.nNumOfFields;
	int lastError;
	if (!WriteFile(m_FileHandle, &m_ControlerFileData, nDataSize, &numOfBytesWritten, NULL))
	{
		lastError = GetLastError();
	}
	memcpy(m_ControlerFileData.pFieldsData, rcdInfo, nFieldArraySize);
	if (!WriteFile(m_FileHandle, m_ControlerFileData.pFieldsData, nFieldArraySize, &numOfBytesWritten, NULL))
	{
		lastError = GetLastError();
	}
	//m_ControlerFileData.nMaxFileSpace = min(m_ControlerFileData.nMaxFileSpace, 1024); // if file length > 4GB, we are limiting the size to 4G here
	DWORD curPos = SetFilePointer(m_FileHandle, nMaxLenInMegaBytes * BYTES_IN_MEGABYTE/2 - 1, NULL, FILE_BEGIN);
	DWORD dwLen;
	WriteFile(m_FileHandle, L"\0", 1, &dwLen, NULL);
	CloseHandle(m_FileHandle); //Make the size to be user assigned one.
	m_FileHandle = ::CreateFileW(logFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL);
	if (INVALID_HANDLE_VALUE == m_FileHandle) {

		throw WIN32ERROR();
	}

	SaveToFieldMap();
	return m_FileHandle;
}

//bool LogFileAccess::BuildIndexForFields()
//{//Go throught each record and make the mutimap
//	return true;
//}
bool LogFileAccess::IsFileOldFormat(int nNumberOfFieldsInLogRecords, const USER_LOG_FIELD_SPEC rcdInfo[], int nMaxLenInMegaBytes)
{
	if (m_ControlerFileData.nNumOfFields != nNumberOfFieldsInLogRecords)
	{
		return false;
	}
	else if (m_ControlerFileData.nMaxFileSpace != nMaxLenInMegaBytes * BYTES_IN_MEGABYTE)
	{
		return false;
	} 
	else if(memcmp(rcdInfo, m_ControlerFileData.pFieldsData, sizeof(USER_LOG_FIELD_SPEC) * m_ControlerFileData.nNumOfFields))
	{
		return false;
	}
	return true;

}

bool LogFileAccess::ChangeRotateWhenEnd()
{
	bool bRet = true;
	m_ControlerFileData.nEndLocation += m_ControlerFileData.nPhysLogEntrySize; //Next 8 k bytes
	int nRecordNumber = (m_ControlerFileData.nEndLocation - nHeaderSize) / m_ControlerFileData.nPhysLogEntrySize;
	if (nRecordNumber >= m_ControlerFileData.nTotalRecords)
	{ //I am making sure that the space left is less than 2 kilobytes, we could not allocate more space for loggin here
		OUTMSG("The log entries filled the log file, started to circle around\n");
		m_ControlerFileData.nEndLocation = nHeaderSize;		//Get back to the first record
		OUTMSG1("The end location is %x\n", m_ControlerFileData.nEndLocation);
	}

	return bRet;
}


bool LogFileAccess::UpdateHeaderInfo()
{
	DWORD dwSize;
	bool bRet = true;
	m_ControlerFileData.nEndLocation += m_ControlerFileData.nPhysLogEntrySize; //Next 8 k bytes
	if (m_ControlerFileData.nEndLocation + m_ControlerFileData.nTotalJasonSize + 2 * BYTES_IN_KILOBYTE 
				>= (int)m_ControlerFileData.nMaxFileSpace) 
	{ //I am making sure that the space left is less than 2 kilobytes, we could not allocate more space for loggin here
		OUTMSG("The log entries filled the log file, started to circle around\n");
		m_ControlerFileData.bFullyFilled = true;
		m_ControlerFileData.nEndLocation = nHeaderSize;		//Get back to the first record
		OUTMSG1("The end location is %x\n", m_ControlerFileData.nEndLocation);
	}
	else if (m_ControlerFileData.nEndLocation >= (int)(dwSize=GetFileSize(m_FileHandle, NULL)))
	{	//If it is not good enough space, add another Mega byte to the file size.
		//Try to predict how much space for the table
		
		OUTMSG1("logging db size = %x\n", dwSize);
		unsigned int nSpaceLeft = m_ControlerFileData.nMaxFileSpace
			- (m_ControlerFileData.nEndLocation + m_ControlerFileData.nTotalJasonSize);
		DWORD curPos;
		if (nSpaceLeft > 2 * BYTES_IN_MEGABYTE)
		{
			curPos = SetFilePointer(m_FileHandle, dwSize + BYTES_IN_MEGABYTE - 1, NULL, FILE_BEGIN);
			OUTMSG("Add another 1 MB for the log storage\n");

		}
		else
		{	//We predict that table and json use the same amount of space, allocate in kilobytes.
			int newKiloByte = nSpaceLeft / (2 * m_ControlerFileData.nPhysLogEntrySize);
			newKiloByte = newKiloByte < 16 ? 16 : newKiloByte;		//To acclerate allocation when small space left
			int newFileSpace = dwSize + newKiloByte * m_ControlerFileData.nPhysLogEntrySize;
			curPos = SetFilePointer(m_FileHandle, 
				newFileSpace - 1, NULL, FILE_BEGIN);

			OUTMSG1("Add another %dkb for the log storage\n", newKiloByte);

		}
		DWORD dwLen;
		WriteFile(m_FileHandle, L"\0", 1, &dwLen, NULL);
		CloseHandle(m_FileHandle); //Make the size to be user assigned one.
		m_FileHandle = ::CreateFileW(m_logFileName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL);
		if (INVALID_HANDLE_VALUE == m_FileHandle) {
			int error = GetLastError();
			OUTMSG1("Could not open file, last error = %d\n", error);
			throw WIN32ERROR();
		}
		if(m_FileHandle == INVALID_HANDLE_VALUE)
		{
		
			bRet = false;
		}
		dwSize = GetFileSize(m_FileHandle, NULL);
	}
	return bRet;
}

bool LogFileAccess::LogUserDataStart()
{
	OUTMSG("LogUserDataStart:\n");
	int nDataSize = sizeof(m_ControlerFileData) - sizeof(USER_LOG_FIELD_SPEC *);
	LockFile(m_FileHandle, 0, 0, nDataSize, 0);
	OUTMSG3("start Location=%x, TotalRecords=%d, Filled=%d\n", m_ControlerFileData.nEndLocation,
		m_ControlerFileData.nTotalRecords, m_ControlerFileData.bFullyFilled);
	int nRet = -1;
	do
	{
		SetFilePointer(m_FileHandle, 0, NULL, FILE_BEGIN);
		int lastError;

		DWORD numOfBytesWritten;
		if (!WriteFile(m_FileHandle, &m_ControlerFileData, nDataSize, &numOfBytesWritten, NULL))
		{
			lastError = GetLastError();
			break;
		}
		nRet = 0;
	} while (false);

	UnlockFile(m_FileHandle, 0, 0, nDataSize, 0);
	OUTMSG3("End Location=%x, TotalRecords=%d, Filled=%d\n", m_ControlerFileData.nEndLocation,
		m_ControlerFileData.nTotalRecords, m_ControlerFileData.bFullyFilled);


	return (nRet == 0);
}

bool LogFileAccess::EraseCurrentJson(void *pData)
{
	bool bRet = true;
	USER_LOG_EXTRA_INFO *pInternal = (USER_LOG_EXTRA_INFO *)((char *)pData + m_ControlerFileData.nLogRecordSize);
	_ASSERT(DeleteFile(pInternal->szJsonPath));
	OUTMSG2("nTotalJasonSize = %d, nJSONSize = %d\n", m_ControlerFileData.nTotalJasonSize, pInternal->nJSONSize);
	m_ControlerFileData.nTotalJasonSize -= pInternal->nJSONSize;
	return bRet;
}

//bool LogFileAccess::InitializeMutiMap(shared_ptr<INTERNAL_FIELD_SPEC>& fieldSpec)
//{
//	switch (fieldSpec->fieldType)
//	{
//	case INT_TYPE:
//	case SHORT_TYPE:
//	case BOOL_TYPE:
//	{
//		fieldSpec->queryMap.pIntMap = make_unique<multimap<int, long>>();
//		break;
//	}
//	break;
//	case INT64_TYPE:
//	{
//		fieldSpec->queryMap.pInt64Map = make_unique<multimap<__int64, long>>();
//		break;
//	}
//	break;
//	case STRING_TYPE:
//	{
//		fieldSpec->queryMap.pStrMap = make_unique<multimap<wstring, long>>();
//		break;
//	}
//	break;
//	default:
//		return false;
//	}
//	return true;
//}


bool LogFileAccess::InsertToIndexMap(shared_ptr<INTERNAL_FIELD_SPEC>& fieldSpec, const CHAR * pLogRecord, const int nRecLocation)
{
	switch (fieldSpec->fieldType)
	{
	case INT_TYPE:
	{
		int val = *(int *)(pLogRecord + fieldSpec->nOffset);
		fieldSpec->queryMap.IntMap.insert(make_pair(val, nRecLocation));
		break;
	}
	case SHORT_TYPE:
	{
		short val = *(short *)(pLogRecord + fieldSpec->nOffset);
		fieldSpec->queryMap.IntMap.insert(make_pair((int)val, nRecLocation));
		break;
	}
	case BOOL_TYPE:
	{
		BOOL val = *(BOOL *)(pLogRecord + fieldSpec->nOffset);
		fieldSpec->queryMap.IntMap.insert(make_pair((int)val, nRecLocation));
		break;
	}
	break;
	case INT64_TYPE:
	{
		__int64 val = *(__int64 *)(pLogRecord + fieldSpec->nOffset);
		fieldSpec->queryMap.Int64Map.insert(make_pair(val, nRecLocation));
		break;
	}
	break;
	case STRING_TYPE:
	{
		wstring val = (wchar_t *)(pLogRecord + fieldSpec->nOffset);
		fieldSpec->queryMap.StrMap.insert(make_pair(val, nRecLocation));
		break;
	}
	default:
		return false;
	}
	return true;
}

int LogFileAccess::WriteJsonFile(const wchar_t *szJsonFileName, const char *JsonString)
{
	int fileSize = 0;
	try {
		fstream jsonFile;
		jsonFile.open(szJsonFileName, std::fstream::in | std::fstream::out | std::fstream::app);
		jsonFile << JsonString;
		//Find json file size here
		jsonFile.seekg(0, ios_base::end);
		fileSize = (int)jsonFile.tellg();
		jsonFile.close();
	}
	catch (...)
	{
		OUTMSG("JSON WRITTEN HAS EXCEPTION\n");
	}
	return fileSize;
}

using namespace std :: chrono;

int LogFileAccess::LogUserData(void *pData, const char *JsonString)
{
	OUTMSG("LogUserData:\n");
	DWORD curPos = SetFilePointer(m_FileHandle, m_ControlerFileData.nEndLocation, NULL, FILE_BEGIN);
	DWORD dwLen;
	int writtingLocation = m_ControlerFileData.nEndLocation;
	LockFile(m_FileHandle, writtingLocation, 0, m_ControlerFileData.nLogRecordSize, 0);
	int nDataSize = sizeof(m_ControlerFileData) - sizeof(USER_LOG_FIELD_SPEC *);
	LockFile(m_FileHandle, 0, 0, nDataSize, 0);
	OUTMSG3("start Location=%x, TotalRecords=%d, nTotalJasonSize=%d\n", m_ControlerFileData.nEndLocation,
		m_ControlerFileData.nTotalRecords, m_ControlerFileData.nTotalJasonSize);
	DWORD dwSize = GetFileSize(m_FileHandle, NULL);
	OUTMSG1("LoggingFilesize=%x\n", dwSize);

	//OUTMSG3("FileHandle=%x, TotalRecords=%d, Filled=%d\n", m_FileHandle,
	//	m_ControlerFileData.nTotalRecords, m_ControlerFileData.bFullyFilled);
	int nRet = -1;
	USER_LOG_EXTRA_INFO *pInternal = (USER_LOG_EXTRA_INFO *)((char *)pData + m_ControlerFileData.nLogRecordSize);
	do
	{	
		time_point<std::chrono::system_clock> currentTime;
		currentTime = std::chrono::system_clock::now();
		__int64 CurrTimeInMicro = duration_cast<microseconds>(currentTime.time_since_epoch()).count();
		__int64 CurrTimeInSecond = duration_cast<seconds>(currentTime.time_since_epoch()).count();

		pInternal->logWritingTime = CurrTimeInMicro;
		pInternal->logSerialNumber = m_ControlerFileData.nLastSerialNumber++;


		if (m_ControlerFileData.bFullyFilled) //When it is filled, we need to delete json file and update the size of json
		{//Read from file
			auto storeInfo = std::unique_ptr<CHAR, void(*)(CHAR*) >(new CHAR[GetLogEntrySize()],
				[](CHAR *p) {delete[] p; });
			DWORD numOfBytesRead;
			if (ReadFile(m_FileHandle, storeInfo.get(), GetLogEntrySize(), &numOfBytesRead, NULL) && numOfBytesRead > 0)
			{
				EraseCurrentJson(storeInfo.get());
				RemoveRecordFromIndex(storeInfo.get(), m_ControlerFileData.nEndLocation);
			}
			else
			{
				break;
			}
			//Restore file pointer here
			curPos = SetFilePointer(m_FileHandle, m_ControlerFileData.nEndLocation, NULL, FILE_BEGIN);
		}

		wchar_t szJsonFileName[MAX_PATH];
		//Create the json file name with the serial number
		wsprintf(szJsonFileName, L"%s_%d.JSON", m_ControlerFileData.szJsonPath, pInternal->logSerialNumber);
		//save json file according to the name
		int fileSize = WriteJsonFile(szJsonFileName, JsonString);
		_ASSERT(fileSize < 10000);
		pInternal->nJSONSize = fileSize;
		wcscpy_s(pInternal->szJsonPath, MAX_PATH, szJsonFileName);
		OUTMSG2("nTotalJasonSize = %d, nJSONSize = %d\n", m_ControlerFileData.nTotalJasonSize, fileSize);
		m_ControlerFileData.nTotalJasonSize += fileSize;
		DWORD dwSize = GetFileSize(m_FileHandle, NULL);
		OUTMSG1("before write LoggingFilesize=%x\n", dwSize);

		if (!WriteFile(m_FileHandle, pData, m_ControlerFileData.nLogRecordSize + nExtraInfoSize, &dwLen, NULL))
		{
			break;
		}

		AddNewRecordToIndex((CHAR *)pData, m_ControlerFileData.nEndLocation);	//Here we add the record to index.


		if (!m_ControlerFileData.bFullyFilled)
		{
			if (UpdateHeaderInfo())
			{
				m_ControlerFileData.nTotalRecords++;
			}
		}
		else
		{
			ChangeRotateWhenEnd();
		}
		SetFilePointer(m_FileHandle, 0, NULL, FILE_BEGIN);
		int lastError;

		DWORD numOfBytesWritten;
		if (!WriteFile(m_FileHandle, &m_ControlerFileData, nDataSize, &numOfBytesWritten, NULL))
		{
			lastError = GetLastError();
			break;
		}
		nRet = 0;
	} while (false);

	UnlockFile(m_FileHandle, 0, 0, nDataSize, 0);
	UnlockFile(m_FileHandle, writtingLocation, 0, m_ControlerFileData.nLogRecordSize, 0);
	//OUTMSG3("FileHandle=%x, TotalRecords=%d, Filled=%d\n", m_FileHandle,
	//	m_ControlerFileData.nTotalRecords, m_ControlerFileData.bFullyFilled);

	OUTMSG3("start Location=%x, TotalRecords=%d, nTotalJasonSize=%d\n", m_ControlerFileData.nEndLocation,
		m_ControlerFileData.nTotalRecords, m_ControlerFileData.nTotalJasonSize);
	dwSize = GetFileSize(m_FileHandle, NULL);
	OUTMSG1("LoggingFilesize=%x\n", dwSize);


	return (nRet == 0 ? m_ControlerFileData.nTotalRecords - 1 : nRet);
}


bool LogFileAccess::GetOneLogEntry(void *pData, int nBufferLen, int nIndex, USER_LOG_EXTRA_INFO *pExtraInfo)
{
	_ASSERT(nBufferLen >= m_ControlerFileData.nLogRecordSize);
	_ASSERT(nIndex < m_ControlerFileData.nTotalRecords);
	OUTMSG("Read start\n");

	int absoluteLocation = m_ControlerFileData.nEndLocation + nIndex * m_ControlerFileData.nPhysLogEntrySize;
	int endOfFile = m_ControlerFileData.nTotalRecords * m_ControlerFileData.nPhysLogEntrySize + nHeaderSize;
	OUTMSG1("endOfFile=%x, Read end success\n", endOfFile);
	int nReadLocation;
	if(m_ControlerFileData.bFullyFilled)
	{
		nReadLocation = absoluteLocation >= endOfFile ? absoluteLocation - endOfFile + nHeaderSize : absoluteLocation;
	}
	else
	{
		nReadLocation = nIndex * m_ControlerFileData.nPhysLogEntrySize + nHeaderSize;
	}
		
	SetFilePointer(m_FileHandle, nReadLocation, NULL, FILE_BEGIN);
	auto storeInfo = std::unique_ptr<CHAR, void(*)(CHAR*)>(new CHAR[GetLogEntrySize()], [](CHAR *p) {delete[] p; });
	DWORD numOfBytesRead;
	int lastError;
	if (!ReadFile(m_FileHandle, storeInfo.get(), GetLogEntrySize(), &numOfBytesRead, NULL) || !numOfBytesRead)
	{
		lastError = GetLastError();
		OUTMSG2("m_FileHandle=%x Read end failed, lasterror = %d\n", m_FileHandle, lastError);
		return false;
	}
	memcpy(pData, storeInfo.get(), m_ControlerFileData.nLogRecordSize);
	if(pExtraInfo)
	{
		memcpy(pExtraInfo, storeInfo.get() + m_ControlerFileData.nLogRecordSize, sizeof(USER_LOG_EXTRA_INFO));
	}
	OUTMSG1("m_FileHandle=%x, Read end success\n", m_FileHandle);
	return true;
}

bool LogFileAccess::GetRecordsByField(const USER_LOG_FIELD_VAL &queryVal, vector<vector<char>> &logArray, int nCount)
{
	if (m_ControlerFileData.nTotalRecords == 0) return false;

	auto iter = m_fieldInfo.find(queryVal.szFieldName);
	if (iter == m_fieldInfo.end())
	{
		return false;
	}
	auto field = iter->second;
	if (field->bMapValid && m_indexedFields.size())
	{//This field is already indexed, we can find data inside

		return SearchIndexByField(field, queryVal, logArray, nCount);
	}
	else
	{
		return SearchFileByField(queryVal, *field.get(), logArray, nCount);
	}
}

bool LogFileAccess::PushValueToArray(long nLocation, vector<vector<char>> &logArray)
{
	vector<char> entryData;
	entryData.resize(GetLogEntrySize());

	SetFilePointer(m_FileHandle, nLocation, NULL, FILE_BEGIN);
	DWORD numOfBytesRead;
	int lastError;
	if (!ReadFile(m_FileHandle, entryData.data(), GetLogEntrySize(), &numOfBytesRead, NULL) || !numOfBytesRead)
		//if (!ReadFile(m_FileHandle, pData, m_ControlerFileData.nLogRecordSize, &numOfBytesRead, NULL) || !numOfBytesRead)
	{
		lastError = GetLastError();
		OUTMSG("Read end failed\n");
		return false;
	}

	logArray.push_back(entryData);
	return true;

}


bool LogFileAccess::SearchIndexByField(const shared_ptr<INTERNAL_FIELD_SPEC> &field, const USER_LOG_FIELD_VAL &queryVal, vector<vector<char>> &logArray, int nCount)
{
	switch (field->fieldType)
	{
	case INT_TYPE:
	case SHORT_TYPE:
	case BOOL_TYPE:
	//{	
	//	int foundCount = 0;
	//	int val = queryVal.Val.iValue;
	//	auto  ret = field->queryMap.IntMap.equal_range(val);
	//	if (!nCount)
	//	{	//We get all values
	//		auto it = ret.second;
	//		while (it != ret.first)
	//		{
	//			--it;
	//			auto value = it->second;
	//			PushValueToArray(value, logArray);
	//		}
	//	}
	//	else
	//	{
	//		auto it = ret.second; 
	//		while ( it != ret.first)
	//		{
	//			--it;
	//			auto value = it->second;
	//			PushValueToArray(value, logArray);
	//			foundCount++;
	//			if (foundCount >= nCount)
	//			{
	//				break;
	//			}
	//		}
	//	}
	//	break;
	//}
	{	
		int foundCount = 0;
		int val = queryVal.Val.iValue;
		auto  ret = field->queryMap.IntMap.equal_range(val);
		if (!nCount)
		{	//We get all values
			for (auto it = ret.first; it != ret.second; ++it)
			{
				auto value = it->second;
				PushValueToArray(value, logArray);
			}
		}
		else
		{
			for (auto it = ret.first; it != ret.second; ++it)
			{
				auto value = it->second;
				PushValueToArray(value, logArray);
				foundCount++;
				if (foundCount >= nCount)
				{
					break;
				}
			}
		}
		break;
	}
	case INT64_TYPE:
	{
		int foundCount = 0;
		auto val = queryVal.Val.llValue;
		auto ret = field->queryMap.Int64Map.equal_range(val);
		if (!nCount)
		{	//We get all values
			for (auto it = ret.first; it != ret.second; ++it)
			{
				auto value = it->second;
				PushValueToArray(value, logArray);
			}
		}
		else
		{
			for (auto it = ret.first; it != ret.second; ++it)
			{
				auto value = it->second;
				PushValueToArray(value, logArray);
				foundCount++;
				if (foundCount >= nCount)
				{
					break;
				}
			}
		}
		break;
	}
	case STRING_TYPE:
	{
		int foundCount = 0;
		auto val = queryVal.Val.strValue;
		auto ret = field->queryMap.StrMap.equal_range(val);
		if (!nCount)
		{	//We get all values
			for (auto it = ret.first; it != ret.second; ++it)
			{
				auto value = it->second;
				PushValueToArray(value, logArray);
			}
		}
		else
		{
			for (auto it = ret.first; it != ret.second; ++it)
			{
				auto value = it->second;
				PushValueToArray(value, logArray);
				foundCount++;
				if (foundCount >= nCount)
				{
					break;
				}
			}
		}
		break;
	}
	default:
		return false;
	}
	return true;
}

bool LogFileAccess::SearchFileByField(const USER_LOG_FIELD_VAL &queryVal, const INTERNAL_FIELD_SPEC &spec, vector<vector<char>> &logArray, int nCount)
{

	int absoluteLocation = m_ControlerFileData.nEndLocation;
	int endOfFile = m_ControlerFileData.nTotalRecords * m_ControlerFileData.nPhysLogEntrySize + nHeaderSize;
	int nReadLocation;
	if (m_ControlerFileData.bFullyFilled)
	{
		nReadLocation = absoluteLocation;
	}
	else
	{
		nReadLocation = nHeaderSize;
	}

	vector<char> entryData;
	entryData.resize(GetLogEntrySize());



	bool bCircleBack = true;		//use this value to control circle back for the logging entries

	do 
	{
		SetFilePointer(m_FileHandle, nReadLocation, NULL, FILE_BEGIN);

		DWORD numOfBytesRead;
		int lastError;
		if (!ReadFile(m_FileHandle, entryData.data(), GetLogEntrySize(), &numOfBytesRead, NULL) || !numOfBytesRead)
		//if (!ReadFile(m_FileHandle, pData, m_ControlerFileData.nLogRecordSize, &numOfBytesRead, NULL) || !numOfBytesRead)
		{
			lastError = GetLastError();
			OUTMSG("Read end failed\n");
			return false;
		}
		else
		{
			if (!ComparefieldValue(entryData.data(), spec, queryVal))
			{	// we find a match, we need to add to the result vector
				logArray.push_back(entryData);
				if (nCount)
				{
					if (logArray.size() >= (unsigned int)nCount)
					{//We return it here if we reach the count records
						break;
					}
				}
			}
		}

		nReadLocation += m_ControlerFileData.nPhysLogEntrySize;
		if (m_ControlerFileData.bFullyFilled)
		{
			if (bCircleBack)
			{	//When file is filled which meanings that it is circle around.
				if (nReadLocation >= endOfFile)
				{
					nReadLocation = nHeaderSize;
					bCircleBack = false;
				}
			} 
			else
			{
				if (nReadLocation >= m_ControlerFileData.nEndLocation)
				{
					break;
				}
			}
		}
		else
		{
			if (nReadLocation >= m_ControlerFileData.nEndLocation)
			{
				break;		//This is the end of the file
			}
		}
	} while (true);
	
	OUTMSG("Read end success\n");
	return true;
 
}



template <class T> int CompareValue(T a, T b)
{
	if (a == b) return 0;
	else if (a > b) return 1;
	return -1;
}

int LogFileAccess::ComparefieldValue(char *entryData, const INTERNAL_FIELD_SPEC &spec, const USER_LOG_FIELD_VAL &queryVal)
{

	switch (spec.fieldType)
	{
		case INT_TYPE:
		{
			int a = *(int*)(entryData + spec.nOffset);
			return CompareValue(a, queryVal.Val.iValue);
		}
		case INT64_TYPE:
		{
			__int64 a = *(__int64*)(entryData + spec.nOffset);
			return CompareValue(a, queryVal.Val.llValue);
		}
		case STRING_TYPE:
		{
			wchar_t *a = (wchar_t*)(entryData + spec.nOffset);
			return wcscmp(a, queryVal.Val.strValue);
		}
		case FLOAT_TYPE:
		{
			float a = *(float*)(entryData + spec.nOffset);
			return CompareValue(a, queryVal.Val.fValue);
		}
		case DOUBLE_TYPE:
		{
			double a = *(double*)(entryData + spec.nOffset);
			return CompareValue(a, queryVal.Val.dbValue);
		}
		case BOOL_TYPE:
		{
			bool a = *(bool*)(entryData + spec.nOffset);
			return CompareValue(a, queryVal.Val.bValue);
		}
		case SHORT_TYPE:
		{
			short a = *(short*)(entryData + spec.nOffset);
			return CompareValue(a, queryVal.Val.stValue);
		}
	}
	return -1;
}

bool LogFileAccess::DeleteAllJsonFiles()
{
	WIN32_FIND_DATA fd;
	wchar_t szJsonFileName[MAX_PATH];
	wcscpy_s(szJsonFileName, MAX_PATH, m_ControlerFileData.szJsonPath);
	PathRemoveFileSpec(szJsonFileName);

	HANDLE hFind = FindFirstFileW((wstring(szJsonFileName)+ L"\\*.JSON").c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			DeleteFileW((wstring(szJsonFileName) + L"\\"+ fd.cFileName).c_str());
		} while (FindNextFileW(hFind, &fd));
		FindClose(hFind);
	}
	return true;
}