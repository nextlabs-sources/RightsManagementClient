#include "stdafx.h"
#include "UserLogManager.h"
#include "LogFileAccess.h"
#include <nudf\exception.hpp>
#include "logthread.h"
#include "map"
#include <shlwapi.h>
#include <shlobj.h>


//UserLogManager *UserLogManager::pLogManager = NULL;

UserLogManager::UserLogManager() : CLog()
{
	m_FileHandle = NULL;
	pFileHeaderAccess = new LogFileAccess();
	SetFileHeaderAccess(pFileHeaderAccess);
	
}


UserLogManager::~UserLogManager()
{
	Stop();
	//This handle can be created in log manager or file header access class. But we need to close it here
	LogFileAccess *pFileHeader = (LogFileAccess *)pFileHeaderAccess;
	delete pFileHeader;

}


VOID UserLogManager::Start()
{
	std::wstring wsFile;

	if (IsRunning()) {
		SetLastError(ERROR_INVALID_HANDLE_STATE);
		throw WIN32ERROR();
	}

		CLog::Start();

}

VOID UserLogManager::Stop()
{
	CLog::Stop();
}




bool UserLogManager::Initialize(const wchar_t szCallerDesc[], const wchar_t szLogFileName[], int nNumberOfFieldsInLogRecords,
	const USER_LOG_FIELD_SPEC rcdInfo[], int nLogEntrySize, int nMaxLenInMegaBytes)
{
	_ASSERT(nNumberOfFieldsInLogRecords <= 100);
	_ASSERT(nMaxLenInMegaBytes > 0);
	bool bRet = false;

	//Verify that we have correct field size
	int nTotalRecordSize = 0;
	
	
	LogFileAccess *pFileHeader = (LogFileAccess *)pFileHeaderAccess; 
	wstring logFileName(szLogFileName);
	
	//logFileName += NL_LOG_EXT;
	m_FileHandle = ::CreateFileW(logFileName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL);

	if (INVALID_HANDLE_VALUE == m_FileHandle) {
		do
		{
			DWORD dwRet = GetLastError();
			if (dwRet == ERROR_FILE_NOT_FOUND)
			{
				try
				{
					m_FileHandle = pFileHeader->CreateFileHeader(szCallerDesc, logFileName.c_str(), nNumberOfFieldsInLogRecords, rcdInfo, nLogEntrySize, nMaxLenInMegaBytes);
					if (INVALID_HANDLE_VALUE == m_FileHandle) {

						throw WIN32ERROR();
					}
				}
				catch (...)
				{
					//Do nothing will return false
					break;

				}
				bRet = true;
			}
			else if(dwRet == ERROR_PATH_NOT_FOUND)
			{
				try
				{
					wchar_t szDirectory[MAX_PATH];
					lstrcpy(szDirectory, logFileName.c_str());
					PathRemoveFileSpec(szDirectory);
					int errorCode=SHCreateDirectoryEx(NULL, szDirectory, NULL);
					_ASSERT(errorCode == ERROR_SUCCESS);

					m_FileHandle = pFileHeader->CreateFileHeader(szCallerDesc, logFileName.c_str(), nNumberOfFieldsInLogRecords, rcdInfo, nLogEntrySize, nMaxLenInMegaBytes);
				}
				catch (exception e)
				{
					OutputDebugStringA(e.what());
					break;

				}
				bRet = true;
			}
		} while (false);
	}
	else
	{
		do
		{
			if (pFileHeader->ReadFileHeader(m_FileHandle, logFileName))
			{
				if (pFileHeader->IsFileOldFormat(nNumberOfFieldsInLogRecords, rcdInfo, nMaxLenInMegaBytes))
				{
					
					bRet = true;
					break;
				}
			}
			//Here we remove the old file
			pFileHeader->DeleteAllJsonFiles();
			CloseHandle(m_FileHandle);
			_ASSERT(DeleteFile(logFileName.c_str()));


			m_FileHandle = NULL;
			try
			{
				m_FileHandle = pFileHeader->CreateFileHeader(szCallerDesc, logFileName.c_str(), nNumberOfFieldsInLogRecords, rcdInfo, nLogEntrySize, nMaxLenInMegaBytes);
				if (INVALID_HANDLE_VALUE == m_FileHandle) {

					throw WIN32ERROR();
				}

			}
			catch (...)
			{
				bRet = false;
				break;
			}
			bRet = true;
		} while (false);
	}
	if(bRet)
	{
		try
		{
			Start();
		}
		catch (...)
		{
			bRet = false;
		}
	}

	if (bRet)
	{
		m_entrySize = pFileHeader->GetLogEntrySize();
		SetLogEntrySize(m_entrySize);
	}

	return bRet;
}


int UserLogManager::LogUserData(void *pData, const char *JsonString)
{
	return Push(pData, JsonString);
}

int  UserLogManager::NumberOfLogEntries() //This function will find how many log entries in current log file.
{
	LogFileAccess *pFileHeader = (LogFileAccess *)pFileHeaderAccess;
	_ASSERT(pFileHeader != NULL);
	return pFileHeader->NumberOfLogEntries();
}


bool UserLogManager::GetRecordsByField(const USER_LOG_FIELD_VAL &queryVal, vector<vector<char>> &logArray, int nCount)
{

	return CLog::GetRecordsByField(queryVal, logArray, nCount);
	
}