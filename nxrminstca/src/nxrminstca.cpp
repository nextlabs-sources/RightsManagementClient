// nxrminstca.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <msi.h>
#include <msiquery.h>
#include <stdio.h>
#include <Winreg.h>
#include <Shlwapi.h>
#include <shellapi.h>


#define PRODUCT_NAME L"NextLabs Rights Management"
#define FILENAME_REGISTER L"register.xml"

#define MAX_DRIVERFILES 2
const wchar_t *wstrSourceDriverFiles[MAX_DRIVERFILES] = {L"drv2\\nxrmdrv.sys", L"drv1\\nxrmflt.sys"};
const wchar_t *wstrDistDriverFiles[MAX_DRIVERFILES] = {L"nxrmdrv.sys", L"nxrmflt.sys"};

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

//Note:  Messagebox can not use in defered execution since not be able to get UILevel property
UINT _stdcall MessageAndLogging(MSIHANDLE hInstall, BOOL bLogOnly, const WCHAR* wstrMsg )
{
	if(bLogOnly == FALSE && hInstall!= NULL)
	{
		INT nUILevel =0;
		WCHAR wstrTemp[2] = {0};
		DWORD dwBufsize = 0;
		
		dwBufsize = sizeof(dwBufsize)/sizeof(WCHAR);	
		if(ERROR_SUCCESS == MsiGetProperty(hInstall, TEXT("UILevel"), wstrTemp, &dwBufsize))
		{
			nUILevel = _wtoi(wstrTemp);
		}

		if(nUILevel > 2)
		{
			MessageBox(GetForegroundWindow(),(LPCWSTR) wstrMsg, (LPCWSTR)PRODUCT_NAME, MB_OK|MB_ICONWARNING);	
		}
	}

	//add log here
	PMSIHANDLE hRecord = MsiCreateRecord(1);
	if(hRecord !=NULL)
	{
		MsiRecordSetString(hRecord, 0, wstrMsg);
		// send message to running installer
		MsiProcessMessage(hInstall, INSTALLMESSAGE_INFO, hRecord);
		MsiCloseHandle(hRecord);
	}

	
	return ERROR_SUCCESS;
}//return service current status, or return 0 for service not existed

DWORD GetServiceStatus(const wchar_t *wstrServiceName)
{
	SC_HANDLE hSCManager,hService;
	WCHAR wstrTemp[128] = {0};
	DWORD dwErrorCode = 0;

	if ( wstrServiceName==NULL || wstrServiceName[0]==L'\0')	
		return 0;
	
	hSCManager = OpenSCManager(NULL, NULL, GENERIC_READ);
	if (hSCManager==NULL)
	{
		dwErrorCode = GetLastError();
		swprintf_s(wstrTemp,128, L"Open SC Menager Failed. Error Code: %d", dwErrorCode);
		//MessageBox(GetForegroundWindow(), (LPCWSTR)wstrTemp,(LPCWSTR)PRODUCT_NAME , MB_OK|MB_ICONWARNING);
		return 0;
	}

	hService = OpenService(hSCManager, wstrServiceName, GENERIC_READ);	
	if (hService== NULL)
	{		
		CloseServiceHandle(hSCManager);
		return 0;
	}

	SERVICE_STATUS_PROCESS ServiceStatus;
	ZeroMemory(&ServiceStatus, sizeof(ServiceStatus));
	DWORD dwBytesNeeded = 0;

	if (!QueryServiceStatusEx(hService,
							SC_STATUS_PROCESS_INFO,
							(LPBYTE)&ServiceStatus,
							sizeof(ServiceStatus),
							&dwBytesNeeded))
	{
		dwErrorCode = GetLastError();
		swprintf_s(wstrTemp,128,L"SC query Service status failed. Error Code: %d", dwErrorCode);
		//MessageBox(GetForegroundWindow(), (LPCWSTR)wstrTemp, (LPCWSTR)PRODUCT_NAME, MB_OK|MB_ICONWARNING);
		CloseServiceHandle(hSCManager);
		CloseServiceHandle(hService);
		return 0;
	}

	DWORD dwStatus = 0;
	dwStatus = ServiceStatus.dwCurrentState;
	
	CloseServiceHandle(hSCManager);
	CloseServiceHandle(hService);

	return dwStatus;
}


BOOL SHCopy(LPCWSTR from, LPCWSTR to, BOOL bDeleteFrom)
{
	SHFILEOPSTRUCT fileOp = {0};
	WCHAR newFrom[MAX_PATH];
	WCHAR newTo[MAX_PATH];

	if(bDeleteFrom)
		fileOp.wFunc = FO_MOVE;
	else
		fileOp.wFunc = FO_COPY;

	fileOp.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR;

	wcscpy_s(newFrom, from);
	newFrom[wcslen(from) + 1] = NULL;
	fileOp.pFrom = newFrom;

	wcscpy_s(newTo, to);
	newTo[wcslen(to) + 1] = NULL;
	fileOp.pTo = newTo;

	int result = SHFileOperation(&fileOp);

	return result == 0;
}

//*****************************************************************************************************
//				START MSI CUSTOM ACTION FUNCTION HERE
//*****************************************************************************************************

UINT __stdcall MyTest(MSIHANDLE hInstall )
{
	//MessageBox(GetForegroundWindow(),(LPCWSTR) L"Hello world, I am here # 1", (LPCWSTR)PRODUCT_NAME, MB_OK);	
	MessageAndLogging(hInstall, FALSE, L"Hello world, I am here # 1 " );
	
	return ERROR_SUCCESS;
}

//CACleanUp, call in defered execution in system context
UINT __stdcall UninstallCleanUp(MSIHANDLE hInstall )
{
	HKEY hKey = NULL;
		
	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
									TEXT("SYSTEM\\CurrentControlSet\\services\\"),
									0, 
									KEY_ALL_ACCESS, 
									&hKey))
	{			
		SHDeleteKey(hKey,TEXT("nxrmdrv"));	
		SHDeleteKey(hKey,TEXT("nxrmserv"));	
		SHDeleteKey(hKey,TEXT("nxrmflt"));	
		RegCloseKey(hKey);
	}
	MessageAndLogging(hInstall, TRUE, TEXT("******** MXRMLOG: Uninstall clean up done."));

   return ERROR_SUCCESS;
}

//CAResetDrvReg, call in defered execution in system context
UINT __stdcall ResetNxrmdrvRegdata(MSIHANDLE hInstall )
{
	MessageAndLogging(hInstall, TRUE, TEXT("******** MXRMLOG: Start update registry for drivers."));
	//remove pending delete flag for nxrmdrv duing major upgrade
	HKEY hKey = NULL;
	WCHAR wstrTemp[MAX_PATH] = {0};
	DWORD dwBufsize = 0;

	
	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
									TEXT("SYSTEM\\CurrentControlSet\\services\\nxrmdrv\\"),
									0, 
									KEY_WRITE|KEY_READ, 
									&hKey))

	{	
		 dwBufsize = sizeof(wstrTemp)/sizeof(WCHAR);
		if (ERROR_SUCCESS == RegQueryValueEx(hKey, 			
											TEXT("DeleteFlag"),
											NULL, 
											NULL, 
											(LPBYTE)wstrTemp, 
											& dwBufsize))
		{
			RegDeleteValue(hKey,TEXT("DeleteFlag"));
			MessageAndLogging(hInstall, TRUE, TEXT("Remove delete mode."));
		}

		DWORD dwStart = 1; //reset to 1 will auto start driver after reboot
		if (ERROR_SUCCESS == RegSetValueEx(hKey, 			
											TEXT("Start"),
											NULL, 
											REG_DWORD, 
											(const BYTE*)&dwStart,
											sizeof(dwStart)))
		{
			MessageAndLogging(hInstall, TRUE, TEXT("Reset to auto start."));
		}

		RegCloseKey(hKey);
	}
	MessageAndLogging(hInstall, TRUE, TEXT("******** MXRMLOG:  Update registry for drivers done."));
   return ERROR_SUCCESS;
}

//CACheckPendingReboot, call in immediate excution
UINT __stdcall CheckRebootCondition(MSIHANDLE hInstall)
{
	DWORD dwRetStatus = 0;
	UINT uiRet =0;

	MessageAndLogging(hInstall, TRUE, TEXT("******** MXRMLOG: Start check reboot status."));

	dwRetStatus = GetServiceStatus(L"nxrmdrv"); //running
	if(dwRetStatus == SERVICE_RUNNING)
	{
		MessageAndLogging(hInstall, TRUE, TEXT("NXRMLOG: nxrmdrv is running."));
		if( GetServiceStatus(L"nxrmflt") != SERVICE_RUNNING ) //not running
		{
			MessageAndLogging(hInstall, TRUE, TEXT("NXRMLOG: nxrmdflt is stoped. Needs to reboot this computer."));
			uiRet =  MsiSetProperty(hInstall, L"REBOOT", L"Force");
			if (uiRet != ERROR_SUCCESS )
			{
				MessageAndLogging(hInstall, FALSE, TEXT("Set Force REBOOT property failed."));
				return ERROR_INSTALL_FAILURE; 
			}
		}
		else
		{
			MessageAndLogging(hInstall, TRUE, TEXT("NXRMLOG: nxrmdflt is running. No reboot need."));
		}
			
	}
	
	MessageAndLogging(hInstall, TRUE, TEXT("******** MXRMLOG: Check reboot status done."));

	return ERROR_SUCCESS;
}

//CAFindFile, call in immediate excution
UINT __stdcall FindConfigFile(MSIHANDLE hInstall )
{
	WCHAR wstrSourceDir[MAX_PATH] = {0};
	WCHAR wstrTemp[MAX_PATH] = {0};
	DWORD dwPathBuffer = 0;
	UINT uiRet = 0;
	WCHAR wstrMsg[128] = {0};
	DWORD dwErrorCode = 0;
	BOOL bFindFileError =FALSE;

	MessageAndLogging(hInstall, TRUE, TEXT("******** MXRMLOG: Start checking file Register.xml"));

	//1. Get source path from MSI
	dwPathBuffer = sizeof(wstrSourceDir)/sizeof(WCHAR);
	uiRet = MsiGetProperty(hInstall, TEXT("SourceDir"), wstrSourceDir, &dwPathBuffer );
	if( uiRet != ERROR_SUCCESS)
	{
		dwErrorCode = GetLastError();
		swprintf_s(wstrMsg, 128, L"Get Souce dirctory from Msi failed. Error Code: %d", dwErrorCode);
		MessageAndLogging(hInstall, FALSE, (LPCWSTR)wstrMsg );
	
		return ERROR_INSTALL_FAILURE; 
	}

	//2. Try getting parent dir of OriginalDatabase if SourceDir is not defined.
	if (dwPathBuffer == 0)
	{
		dwPathBuffer = _countof(wstrSourceDir);
		uiRet = MsiGetProperty(hInstall, TEXT("OriginalDatabase"), wstrSourceDir, &dwPathBuffer);
		if(uiRet != ERROR_SUCCESS)
		{
			swprintf_s(wstrMsg, L"Get Original Database from Msi failed. Error Code: %u", uiRet);
			MessageAndLogging(hInstall, FALSE, (LPCWSTR)wstrMsg );
			return ERROR_INSTALL_FAILURE;
		}

		//Find parent dir of OriginalDatabase
		WCHAR *p = wcsrchr(wstrSourceDir, L'\\');
		if (p == NULL)
		{
			swprintf_s(wstrMsg, L"Can't find parent directory of OriginaDatabase \"%s\".", wstrSourceDir);
			MessageAndLogging(hInstall, FALSE, (LPCWSTR)wstrMsg );
			bFindFileError = TRUE;
		}
		else
		{
			*p = L'\0';
		}
	}

	if (!bFindFileError)
	{
		//Check if file exist
		if(wstrSourceDir[wcslen(wstrSourceDir)-1]!= L'\\')
		{
			wcscat_s(wstrSourceDir, MAX_PATH,  L"\\");
		}
		wcscat_s(wstrSourceDir, MAX_PATH, FILENAME_REGISTER );

		if (GetFileAttributes(wstrSourceDir)==INVALID_FILE_ATTRIBUTES && GetLastError()==ERROR_FILE_NOT_FOUND)
		{
			MessageAndLogging(hInstall, TRUE, TEXT("File not found, try current directory...."));
			bFindFileError = TRUE;
		}
	}
	
	//3. Try CURRENTDIRECTORY property in reinstall mode
	if(bFindFileError)
	{
		ZeroMemory(wstrSourceDir, sizeof(wstrSourceDir));
		uiRet = 0;
		dwPathBuffer = sizeof(wstrSourceDir)/sizeof(WCHAR);

		uiRet = MsiGetProperty(hInstall, TEXT("CURRENTDIRECTORY"), wstrSourceDir, &dwPathBuffer );
		if( uiRet != ERROR_SUCCESS)
		{
			dwErrorCode = GetLastError();
			swprintf_s(wstrMsg, 128, L"Get Current dirctory from Msi failed. Error Code: %d", dwErrorCode);
			MessageAndLogging(hInstall, FALSE, (LPCWSTR)wstrMsg );
	
			return ERROR_INSTALL_FAILURE; 
		}
	
		//Check if file exist
		if(wstrSourceDir[wcslen(wstrSourceDir)-1]!= L'\\')
		{
			wcscat_s(wstrSourceDir, MAX_PATH,  L"\\");
		}
		wcscat_s(wstrSourceDir, MAX_PATH, FILENAME_REGISTER );

		if (GetFileAttributes(wstrSourceDir)==INVALID_FILE_ATTRIBUTES && GetLastError()==ERROR_FILE_NOT_FOUND)
		{
			// 4. Try installdir in REINSTALL mode
			WCHAR wstrReinstall[MAX_PATH] = {0};
			DWORD dwBuffer = sizeof(wstrReinstall)/sizeof(WCHAR);
			uiRet =0;
			uiRet = MsiGetProperty(hInstall, TEXT("REINSTALL"), wstrReinstall, &dwBuffer );
			
			if( uiRet != ERROR_SUCCESS || wcslen(wstrReinstall) == 0) //not in reinstall
			{
				MessageAndLogging(hInstall, FALSE, TEXT("The required configuration file is not found.  Please run the Client Package Builder first."));
				return ERROR_INSTALL_FAILURE; 
			}
			
			ZeroMemory(wstrSourceDir, sizeof(wstrSourceDir));
			dwPathBuffer = sizeof(wstrSourceDir)/sizeof(WCHAR);
			uiRet =0;
			uiRet = MsiGetProperty(hInstall, TEXT("INSTALLDIR"), wstrSourceDir, &dwPathBuffer );
			if( uiRet != ERROR_SUCCESS)
			{
				dwErrorCode = GetLastError();
				swprintf_s(wstrMsg, 128, L"Get install dirctory from Msi failed. Error Code: %d", dwErrorCode);
				MessageAndLogging(hInstall, FALSE, (LPCWSTR)wstrMsg );
				return ERROR_INSTALL_FAILURE; 
			}

			if(wstrSourceDir[wcslen(wstrSourceDir)-1]!= L'\\')
			{
				wcscat_s(wstrSourceDir, MAX_PATH,  L"\\");
			}
			wcscat_s(wstrSourceDir, MAX_PATH, L"conf\\");
			wcscat_s(wstrSourceDir, MAX_PATH, FILENAME_REGISTER );
			

			if (GetFileAttributes(wstrSourceDir)==INVALID_FILE_ATTRIBUTES )
			{		
				MessageAndLogging(hInstall, FALSE, TEXT("The required configuration file is not found.  Please run the Client Package Builder first."));
				return ERROR_INSTALL_FAILURE; 
			}
		}	

	}

	//get temp path
	DWORD dwRetVal = 0;
	dwRetVal = GetTempPath(MAX_PATH, wstrTemp);                                 
    if ((dwRetVal > MAX_PATH) || (dwRetVal == 0))
    {
		MessageAndLogging(hInstall, FALSE, TEXT("Failed to get temp path in this computer."));
        return ERROR_INSTALL_FAILURE;
    }
	
	// verify temp path exists
	HANDLE hTempFile = INVALID_HANDLE_VALUE;
	hTempFile = CreateFile(	wstrTemp,
							GENERIC_READ,
							FILE_SHARE_READ|FILE_SHARE_WRITE,
							NULL,
							OPEN_EXISTING|CREATE_NEW,
							FILE_FLAG_BACKUP_SEMANTICS,
							NULL);
		
	if ( hTempFile == INVALID_HANDLE_VALUE ) 
	{
		if (!CreateDirectory(wstrTemp, NULL))
		{
			dwErrorCode = GetLastError();
			if ( dwErrorCode != ERROR_ALREADY_EXISTS )
			{
				swprintf_s(wstrMsg, 128, L"Failed to create temp path. Error Code: %d", dwErrorCode);
				MessageAndLogging(hInstall, FALSE, (LPCWSTR)wstrMsg);
				return ERROR_INSTALL_FAILURE;
			}
		}		
	}
	CloseHandle(hTempFile);
	
	//Move file from source to temp
	if(wstrTemp[wcslen(wstrTemp)-1] != L'\\')
	{
		wcscat_s(wstrTemp, MAX_PATH,  L"\\");
	}	
	wcscat_s(wstrTemp, MAX_PATH, FILENAME_REGISTER);

	SetFileAttributes(wstrTemp, FILE_ATTRIBUTE_NORMAL);
	
	if( CopyFile(wstrSourceDir, wstrTemp, FALSE)== FALSE) //Failed
	{
		dwErrorCode = GetLastError();
		swprintf_s(wstrMsg, 128, L"Failed to copy file to temp path. Error Code: %d", dwErrorCode);
		MessageAndLogging(hInstall, FALSE, (LPCWSTR)wstrMsg);
		return ERROR_INSTALL_FAILURE; 
	}

	MessageAndLogging(hInstall, TRUE, TEXT("******** MXRMLOG: Checking file Register.xml done.  Status: Good."));

    return ERROR_SUCCESS;
}

//CACopyfile, call in defered execution in system context
UINT __stdcall CopyConfigFile(MSIHANDLE hInstall ) //run in defered execution
{
	WCHAR wstrSourceDir[MAX_PATH] = {0};
	WCHAR wstrInstallDir[MAX_PATH] = {0};
	DWORD dwPathBuffer = 0;
	WCHAR wstrMsg[128] = {0};
	DWORD dwErrorCode = 0;

	MessageAndLogging(hInstall, TRUE, TEXT("******** MXRMLOG: Start copy config file."));
	//get current Installdir from MSI
	dwPathBuffer = sizeof(wstrInstallDir)/sizeof(WCHAR);
	if(ERROR_SUCCESS !=  MsiGetProperty(hInstall, TEXT("CustomActionData"), wstrInstallDir, & dwPathBuffer))
	{
		dwErrorCode = GetLastError();
		swprintf_s(wstrMsg, 128, L"Failed to get install directory from MSI. Error Code: %d", dwErrorCode);
		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);//log only
	
		return ERROR_INSTALL_FAILURE;
	}


	if(wstrInstallDir[wcslen(wstrInstallDir)-1]!= L'\\')
	{
		wcscat_s(wstrInstallDir, L"\\");
	}	
	wcscat_s(wstrInstallDir, L"conf\\");
	wcscat_s(wstrInstallDir, FILENAME_REGISTER);

	//get file from temp
	DWORD dwRetVal = 0;
	dwRetVal = GetTempPath(MAX_PATH, wstrSourceDir);                                 
    if ((dwRetVal > MAX_PATH) || (dwRetVal == 0))
    {
		MessageAndLogging(hInstall, TRUE, TEXT("Failed to get temp path in this computer."));
        return ERROR_INSTALL_FAILURE;
    }

	if(wstrSourceDir[wcslen(wstrSourceDir)-1]!= L'\\')
	{
		wcscat_s(wstrSourceDir, L"\\");
	}
	wcscat_s(wstrSourceDir, FILENAME_REGISTER);

	//prevent read only file already existed
	SetFileAttributes(wstrInstallDir, FILE_ATTRIBUTE_NORMAL); 
	
	//Move file from Temp to Install Directory
	if(CopyFile(wstrSourceDir, wstrInstallDir, FALSE)== FALSE)
	{
		dwErrorCode = GetLastError();
		swprintf_s(wstrMsg, 128, L"Copy Register.xml file failed. Error Code: %d", dwErrorCode);

		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);
		return ERROR_INSTALL_FAILURE; 
	}

	//Clean up file
	DeleteFile(wstrSourceDir);

	MessageAndLogging(hInstall, TRUE, TEXT("******** MXRMLOG: Copy config file success."));

    return ERROR_SUCCESS;

}

//CACopyfile, call in defered execution in system context
UINT __stdcall CopyDriverFiles(MSIHANDLE hInstall ) //run in defered execution
{	
	WCHAR wstrInstallDir[MAX_PATH] = {0};
	WCHAR wstrInstallDirbin[MAX_PATH] = {0};
	WCHAR wstrWindowsDir[MAX_PATH] ={0};
	WCHAR wstrSystemDirDrivers[MAX_PATH] ={0};
	DWORD dwPathBuffer = 0;
	UINT uiRetCode = 0;

	MessageAndLogging(hInstall, TRUE, TEXT("******** MXRMLOG: Start copy driver files."));

	//get current Installdir from MSI
	dwPathBuffer = sizeof(wstrInstallDir)/sizeof(WCHAR);
	uiRetCode =  MsiGetProperty(hInstall, TEXT("CustomActionData"), wstrInstallDir, &dwPathBuffer);
	if(ERROR_SUCCESS != uiRetCode)
	{
		WCHAR wstrMsg[128] = {0};		
		swprintf_s(wstrMsg, 128, L"NXRMLOG: Failed to get install directory from MSI. Error Code: %d", uiRetCode);
		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);//log only	
		return ERROR_INSTALL_FAILURE;
	}
	
	if(wstrInstallDir[wcslen(wstrInstallDir)-1]!= L'\\')
	{
		wcscat_s(wstrInstallDir, _countof(wstrInstallDir),  L"\\");
	}
	wcscpy_s(wstrInstallDirbin, MAX_PATH, wstrInstallDir );	
	wcscat_s(wstrInstallDirbin, _countof(wstrInstallDirbin),  L"bin\\");

	if(!GetSystemWindowsDirectory(wstrWindowsDir, MAX_PATH))
	{
		MessageAndLogging(hInstall, TRUE, TEXT("NXRMLOG: Failed to get windows directory in this computer."));
        return ERROR_INSTALL_FAILURE;
	}

	if(wstrWindowsDir[wcslen(wstrWindowsDir)-1]!= L'\\')	
	{
		wcscat_s(wstrWindowsDir, _countof(wstrWindowsDir),  L"\\");
	}
	wcscpy_s(wstrSystemDirDrivers, MAX_PATH, wstrWindowsDir);
	wcscat_s(wstrSystemDirDrivers, _countof(wstrSystemDirDrivers),  L"System32\\drivers\\");
	

	//start copy *.sys files from install directory to system32\\drivers
	PVOID OldValue = NULL;
	if( Wow64DisableWow64FsRedirection(&OldValue) ) 
	{
		for(int i=0; i<MAX_DRIVERFILES; i++)
		{
			WCHAR wstrFile[MAX_PATH] ={0};
			WCHAR wstrDistFile[MAX_PATH] ={0};
			ZeroMemory(wstrFile, sizeof(wstrFile));
			wcscpy_s(wstrFile, MAX_PATH, wstrInstallDirbin);
			wcscat_s(wstrFile, _countof(wstrFile), wstrSourceDriverFiles[i]);

			ZeroMemory(wstrDistFile, sizeof(wstrDistFile));
			wcscpy_s(wstrDistFile, MAX_PATH, wstrSystemDirDrivers);
			wcscat_s(wstrDistFile, _countof(wstrDistFile), wstrDistDriverFiles[i]);
		
			SetFileAttributes(wstrDistFile, FILE_ATTRIBUTE_NORMAL);

			if(CopyFile(wstrFile, wstrDistFile, FALSE)== FALSE) //Failed
			{
				DWORD lastErr = GetLastError();
				WCHAR wstrMsg[1024] = {0};
				swprintf_s(wstrMsg, 1024, L"NXRMLOG: Copy driver file from %s to %s failed. Error Code: %d", wstrFile, wstrDistFile, lastErr);
				MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);
				if(lastErr != ERROR_SHARING_VIOLATION)
				{
					Wow64RevertWow64FsRedirection(OldValue);
					return ERROR_INSTALL_FAILURE;
				}
			}
			else
			{
				WCHAR wstrMsg[1024] = {0};
				swprintf_s(wstrMsg, 1024, L"NXRMLOG: Copy driver file from %s to %s success.", wstrFile , wstrDistFile);
				MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);
			}
		}
		Wow64RevertWow64FsRedirection(OldValue) ;
	}

	
	//Get nxrmdrv driverstore path info from register**********************************
	HKEY hKey;
	LONG lResult;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
									TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\DIFx\\Services\\nxrmdrv\\"),
									0, 
									KEY_READ|KEY_WOW64_64KEY, 
									&hKey);
	
	if(ERROR_SUCCESS == lResult )
	{	
		WCHAR wstrTemp[MAX_PATH];
		DWORD dwBufsize = 0;
		dwBufsize = sizeof(wstrTemp)*sizeof(WCHAR);
		if (ERROR_SUCCESS == RegQueryValueEx(hKey, 			
											TEXT("RefCount"),
											NULL, 
											NULL, 
											(LPBYTE)wstrTemp, 
											&dwBufsize))
		{
			WCHAR wstrSys[MAX_PATH] = {0};
			WCHAR wstrInf[MAX_PATH] = {0};
			WCHAR wstrSourceSys[MAX_PATH] = {0};
			WCHAR wstrSourceInf[MAX_PATH] = {0};
			swprintf_s(wstrSys, MAX_PATH, L"%sSystem32\\DRVSTORE\\%s\\nxrmdrv.sys", wstrWindowsDir ,wstrTemp);
			swprintf_s(wstrInf, MAX_PATH, L"%sSystem32\\DRVSTORE\\%s\\nxrmdrv.inf", wstrWindowsDir ,wstrTemp);
			swprintf_s(wstrSourceSys, MAX_PATH, L"%sdrv2\\nxrmdrv.sys", wstrInstallDirbin);
			swprintf_s(wstrSourceInf, MAX_PATH, L"%sdrv2\\nxrmdrv.inf", wstrInstallDirbin);
			
			OldValue = NULL;
			if( Wow64DisableWow64FsRedirection(&OldValue) )
			{
				SetFileAttributes(wstrSys, FILE_ATTRIBUTE_NORMAL);
				if(CopyFile(wstrSourceSys, wstrSys, FALSE)==FALSE) //failed
				{
					WCHAR wstrMsg[1024] = {0};
					swprintf_s(wstrMsg, 1024, L"NXRMLOG: ERROR CODE: %d , copy driver file from %s to %s failed. ", GetLastError(), wstrSourceSys, wstrSys);
					MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);

				}
				SetFileAttributes(wstrInf, FILE_ATTRIBUTE_NORMAL);
				if(CopyFile(wstrSourceInf, wstrInf, FALSE)== FALSE)
				{
					WCHAR wstrMsg[1024] = {0};
					swprintf_s(wstrMsg, 1024, L"NXRMLOG: ERROR CODE: %d , copy driver file from %s to %s failed. ", GetLastError(), wstrSourceInf, wstrInf);
					MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);
				}
				Wow64RevertWow64FsRedirection(OldValue) ;
			}
		}
		else
		{
			WCHAR wstrMsg[1024] = {0};
			swprintf_s(wstrMsg, 1024, L"NXRMLOG: query key for nxrmdrv error. ERROR CODE: %d",GetLastError());
			MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);
		}
		RegCloseKey(hKey);
	}
	else
	{
		WCHAR wstrMsg[1024] = {0};
		swprintf_s(wstrMsg, 1024, L"NXRMLOG: open key nxrmdrv error. ERROR CODE: %d", lResult);
		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);
	}
		

	//Get nxrmflt driverstore path info from register	
	hKey = NULL;
	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
									TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\DIFx\\Services\\nxrmflt\\"),
									0, 
									KEY_READ|KEY_WOW64_64KEY, 
									&hKey))

	{	
		WCHAR wstrTemp[MAX_PATH] = {0};
		DWORD dwBufsize = 0;
	
		dwBufsize = sizeof(wstrTemp)/sizeof(WCHAR);
		if (ERROR_SUCCESS == RegQueryValueEx(hKey, 			
											TEXT("RefCount"),
											NULL, 
											NULL, 
											(LPBYTE)wstrTemp, 
											& dwBufsize))
		{
			WCHAR wstrSys[MAX_PATH] = {0};
			WCHAR wstrInf[MAX_PATH] = {0};
			WCHAR wstrSourceSys[MAX_PATH] = {0};
			WCHAR wstrSourceInf[MAX_PATH] = {0};
			swprintf_s(wstrSys, MAX_PATH, L"%sSystem32\\DRVSTORE\\%s\\nxrmflt.sys", wstrWindowsDir ,wstrTemp);
			swprintf_s(wstrInf, MAX_PATH, L"%sSystem32\\DRVSTORE\\%s\\nxrmflt.inf", wstrWindowsDir ,wstrTemp);
			swprintf_s(wstrSourceSys, MAX_PATH, L"%sdrv1\\nxrmflt.sys", wstrInstallDirbin);
			swprintf_s(wstrSourceInf, MAX_PATH, L"%sdrv1\\nxrmflt.inf", wstrInstallDirbin);

			OldValue = NULL;
			if( Wow64DisableWow64FsRedirection(&OldValue) )
			{
				SetFileAttributes(wstrSys, FILE_ATTRIBUTE_NORMAL);
				if(CopyFile(wstrSourceSys, wstrSys, FALSE)==FALSE) //failed
				{
					WCHAR wstrMsg[1024] = {0};
					swprintf_s(wstrMsg, 1024, L"NXRMLOG: ERROR CODE: %d , copy driver file from %s to %s failed. ", GetLastError(), wstrSourceSys, wstrSys);
					MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);

				}
				SetFileAttributes(wstrInf, FILE_ATTRIBUTE_NORMAL);
				if(CopyFile(wstrSourceInf, wstrInf, FALSE)== FALSE)
				{
					WCHAR wstrMsg[1024] = {0};
					swprintf_s(wstrMsg, 1024, L"NXRMLOG: ERROR CODE: %d , copy driver file from %s to %s failed. ", GetLastError(), wstrSourceInf, wstrInf);
					MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);
				}
				Wow64RevertWow64FsRedirection(OldValue) ;
			}
		}
		else
		{
			WCHAR wstrMsg[1024] = {0};
			swprintf_s(wstrMsg, 1024, L"NXRMLOG: query key for nxrmflt error. ERROR CODE: %d",GetLastError());
			MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);
		}

		RegCloseKey(hKey);
	}
	else
	{
		WCHAR wstrMsg[1024] = {0};
		swprintf_s(wstrMsg, 1024, L"NXRMLOG: open key nxrmflt error. ERROR CODE: %d", lResult);
		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);
	}

	
	MessageAndLogging(hInstall, TRUE, TEXT("******** MXRMLOG: Copy driver file done."));
    return ERROR_SUCCESS;

}

//CAStopRMService, call in deferred execution in system context
UINT __stdcall StopRMService(MSIHANDLE hInstall) //run in deferred execution
{
	MessageAndLogging(hInstall, TRUE, TEXT("******** NXRMLOG: Start stopping RM service."));

	if (GetServiceStatus(L"nxrmserv") != SERVICE_RUNNING)
	{
		MessageAndLogging(hInstall, TRUE, L"NXRLOG: nxrmserv is not running.  No need to stop it.");
		return ERROR_SUCCESS;
	}

	SC_HANDLE hSCManager;
	hSCManager = OpenSCManager(NULL, NULL, 0);
	if (hSCManager == NULL)
	{
		WCHAR wstrMsg[1024];
		swprintf_s(wstrMsg, L"NXRMLOG: OpenSCManager failed. Error Code: %lu", GetLastError());
		MessageAndLogging(hInstall, TRUE, wstrMsg);
		return ERROR_INSTALL_FAILURE;
	}

	SC_HANDLE hService;
	hService = OpenService(hSCManager, L"nxrmserv", SERVICE_STOP);
	if (hService == NULL)
	{
		WCHAR wstrMsg[1024];
		swprintf_s(wstrMsg, L"NXRMLOG: OpenService() failed. Error Code: %lu", GetLastError());
		MessageAndLogging(hInstall, TRUE, wstrMsg);
		CloseServiceHandle(hSCManager);
		return ERROR_INSTALL_FAILURE;
	}

	SERVICE_STATUS status;
	if (!ControlService(hService, SERVICE_CONTROL_STOP, &status))
	{
		WCHAR wstrMsg[1024];
		swprintf_s(wstrMsg, L"NXRMLOG: ControlService() failed. Error Code: %lu", GetLastError());
		MessageAndLogging(hInstall, TRUE, wstrMsg);
		CloseServiceHandle(hService);
		CloseServiceHandle(hSCManager);
		return ERROR_INSTALL_FAILURE;
	}

	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);

	// Check the status until it is stopped or until 30 seconds has passed.
	const int numTries = 30;

	for (int i = 0; i < numTries; i++)
	{
		if (GetServiceStatus(L"nxrmserv") == SERVICE_STOPPED)
		{
			break;
		}
		MessageAndLogging(hInstall, TRUE, L"NXRMLOG: waiting for nxrmserv to finish stopping");
		Sleep(1000);
	}

	// Wait one more second, just in case the service process hasn't exited
	// yet after reporting its status as SERVICE_STOPPED.
	Sleep(1000);

	// Whether the service has stopped or not, we return success in order to
	// let the installer continue.
	MessageAndLogging(hInstall, TRUE, TEXT("******** NXRMLOG: Stopping RM service done."));
	return ERROR_SUCCESS;
}



//CABackupLogs
UINT __stdcall BackupLogFiles(MSIHANDLE hInstall )
{
	MessageAndLogging(hInstall, TRUE, TEXT("******** MXRMLOG: Start back up log files. "));

	HKEY hKey = NULL;
	WCHAR wstrInstallPath[MAX_PATH] = {0};
	WCHAR wstrInstallLogDir[MAX_PATH] = {0};
	WCHAR wstrInstallConfDir[MAX_PATH] = {0};
	WCHAR wstrTempDir[MAX_PATH] = {0};
	WCHAR wstrTempLogDir[MAX_PATH] = {0};
	WCHAR wstrTempConfDir[MAX_PATH] = {0};
	DWORD dwBufsize = 0;
	BOOL bFoundInstDir =FALSE;

	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
									TEXT("SYSTEM\\CurrentControlSet\\services\\nxrmserv\\"),
									0, 
									KEY_READ, 
									&hKey))

	{	
		WCHAR wstrTemp[MAX_PATH] = {0};
		DWORD dwBufsize = sizeof(wstrTemp);

		if (ERROR_SUCCESS == RegQueryValueEx(hKey, 			
											TEXT("ImagePath"),
											NULL, 
											NULL, 
											(LPBYTE)wstrTemp, 
											& dwBufsize))
		{
			WCHAR* pStr = NULL;
			WCHAR* pStr1 = NULL;
			WCHAR* pStrNext = NULL;

			pStr1 = wcstok_s(wstrTemp, TEXT("\""), &pStrNext);
			if(pStr1!= NULL)
				pStr = wcsstr(pStr1, TEXT("bin\\"));

			if( pStr != NULL)
			{	
				wcsncpy_s(wstrInstallPath, MAX_PATH, pStr1, pStr-pStr1);

				wcscpy_s(wstrInstallLogDir, MAX_PATH, wstrInstallPath);	
				wcscat_s(wstrInstallLogDir, _countof(wstrInstallLogDir),  L"log\\*");

				wcscpy_s(wstrInstallConfDir, MAX_PATH, wstrInstallPath);					
				wcscat_s(wstrInstallConfDir, _countof(wstrInstallConfDir),  L"conf\\*");

				bFoundInstDir = TRUE;
			}
		}

		RegCloseKey(hKey);
	}
	
	if(!bFoundInstDir)
	{
		MessageAndLogging(hInstall, TRUE, TEXT("The previous install path does not found. "));
		return ERROR_SUCCESS;
	}

	//get file from temp
	DWORD dwRetVal = 0;
	dwRetVal = GetTempPath(MAX_PATH,  wstrTempDir);                                 
    if ((dwRetVal > MAX_PATH) || (dwRetVal == 0))
    {
		MessageAndLogging(hInstall, TRUE, TEXT("Failed to get temp path in this computer."));
		return ERROR_SUCCESS;
    }
	
	if(wstrTempDir[wcslen(wstrTempDir)-1]!= L'\\')
	{
		wcscat_s(wstrTempDir, _countof(wstrTempDir),  L"\\");
	}

	wcscpy_s(wstrTempLogDir, MAX_PATH, wstrTempDir);	
	wcscat_s(wstrTempLogDir, _countof(wstrTempLogDir),  L"NxrmLog\\");

	BOOL result1 = SHCopy(wstrInstallLogDir, wstrTempLogDir, FALSE);
	MessageAndLogging(hInstall, TRUE, TEXT("******** MXRMLOG: Back up log file done."));

	wcscpy_s(wstrTempConfDir, MAX_PATH, wstrTempDir);
	wcscat_s(wstrTempConfDir, _countof(wstrTempConfDir),  L"NxrmConf\\");

	BOOL result2 = SHCopy(wstrInstallConfDir, wstrTempConfDir, FALSE);	
	MessageAndLogging(hInstall, TRUE, TEXT("******** MXRMLOG: Back up Conf file done."));

    return ERROR_SUCCESS;
}

//CARestoreLogs, call in defered execution in system context
UINT __stdcall RestoreLogFiles(MSIHANDLE hInstall )
{
	MessageAndLogging(hInstall, TRUE, TEXT("******** MXRMLOG: Start Restore log and conf files. "));

	HKEY hKey = NULL;
	WCHAR wstrInstallDir[MAX_PATH] = {0};	
	WCHAR wstrInstallLogDir[MAX_PATH] = {0};
	WCHAR wstrInstallConfDir[MAX_PATH] = {0};
	WCHAR wstrTempDir[MAX_PATH] = {0};
	WCHAR wstrTempLogDir[MAX_PATH] = {0};
	WCHAR wstrTempConfDir[MAX_PATH] = {0};

	DWORD dwBufsize = 0;
	BOOL bFoundLogDir = FALSE;
	UINT uiRetCode = 0;

	
	//get current Installdir from MSI
	dwBufsize = sizeof(wstrInstallDir)/sizeof(WCHAR);
	uiRetCode =  MsiGetProperty(hInstall, TEXT("CustomActionData"), wstrInstallDir, &dwBufsize);
	if(ERROR_SUCCESS != uiRetCode)
	{
		WCHAR wstrMsg[128] = {0};		
		swprintf_s(wstrMsg, 128, L"NXRMLOG: Failed to get install directory from MSI. Error Code: %d", uiRetCode);
		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);//log only	
		return ERROR_SUCCESS;
	}
	
	if(wstrInstallDir[wcslen(wstrInstallDir)-1]!= L'\\')
	{
		wcscat_s(wstrInstallDir, _countof(wstrInstallDir),  L"\\");
	}

	wcscpy_s(wstrInstallLogDir, MAX_PATH, wstrInstallDir);	
	wcscat_s(wstrInstallLogDir, _countof(wstrInstallLogDir),  L"log\\");

	wcscpy_s(wstrInstallConfDir, MAX_PATH, wstrInstallDir);					
	wcscat_s(wstrInstallConfDir, _countof(wstrInstallConfDir),  L"conf\\");

	
	//get file from temp
	DWORD dwRetVal = 0;
	dwRetVal = GetTempPath(MAX_PATH,  wstrTempDir);                                 
    if ((dwRetVal > MAX_PATH) || (dwRetVal == 0))
    {
		MessageAndLogging(hInstall, TRUE, TEXT("Failed to get temp path in this computer."));
		return ERROR_SUCCESS;
    }
	
	if(wstrTempDir[wcslen(wstrTempDir)-1]!= L'\\')
	{
		wcscat_s(wstrTempDir, _countof(wstrTempDir),  L"\\");
	}

	//Restore log	
	//for move files
	wcscpy_s(wstrTempLogDir, MAX_PATH, wstrTempDir);
	wcscat_s(wstrTempLogDir, _countof(wstrTempLogDir),  L"NxrmLog\\*"); 
	BOOL result1 = SHCopy(wstrTempLogDir, wstrInstallLogDir, TRUE);
	// for clean up 
	WCHAR wstrRemoveTempLogDir[MAX_PATH]= {0};
	wcscpy_s(wstrRemoveTempLogDir, MAX_PATH, wstrTempDir);
	wcscat_s(wstrRemoveTempLogDir, _countof(wstrRemoveTempLogDir),  L"NxrmLog"); 
	RemoveDirectory(wstrRemoveTempLogDir);
	MessageAndLogging(hInstall, TRUE, TEXT("******** MXRMLOG: Restore log files done."));

	//Restore conf	
	//for move files
	wcscpy_s(wstrTempConfDir, MAX_PATH, wstrTempDir);
	wcscat_s(wstrTempConfDir, _countof(wstrTempConfDir),  L"NxrmConf\\*"); 
	BOOL result2 = SHCopy(wstrTempConfDir, wstrInstallConfDir, TRUE);
	// for clean up later
	WCHAR wstrRemoveTempConfDir[MAX_PATH]= {0};
	wcscpy_s(wstrRemoveTempConfDir, MAX_PATH, wstrTempDir);
	wcscat_s(wstrRemoveTempConfDir, _countof(wstrRemoveTempConfDir),  L"NxrmConf"); 
	RemoveDirectory(wstrRemoveTempConfDir);


	MessageAndLogging(hInstall, TRUE, TEXT("******** MXRMLOG: Restore Conf files done."));

    return ERROR_SUCCESS;

}

//CADeleteProfiles, call in defered execution in system context
UINT __stdcall DeleteProfiles(MSIHANDLE hInstall )
{
	WCHAR wstrSourceDir[MAX_PATH];
	DWORD dwPathBuffer;
	UINT uiRet;

	dwPathBuffer = _countof(wstrSourceDir);
	uiRet = MsiGetProperty(hInstall, TEXT("CustomActionData"), wstrSourceDir, &dwPathBuffer );

	if(wstrSourceDir[wcslen(wstrSourceDir)-1]!= L'\\')
	{
		wcscat_s(wstrSourceDir, _countof(wstrSourceDir),  L"\\");
	}

	wcscat_s(wstrSourceDir, L"profiles");
	wstrSourceDir[wcslen(wstrSourceDir) + 1] = L'\0';

	SHFILEOPSTRUCT fileOp;

	fileOp.wFunc = FO_DELETE;
	fileOp.pFrom = wstrSourceDir;
	fileOp.fFlags = FOF_NO_UI;

	int result = SHFileOperation(&fileOp);
	if (result == 0 || result == ERROR_FILE_NOT_FOUND)
	{
		MessageAndLogging(hInstall, TRUE, TEXT("******** MXRMLOG: Delete profiles done."));
		return ERROR_SUCCESS;
	}
	else
	{
		WCHAR wstrMsg[1024];
		swprintf_s(wstrMsg, 1024, L"******** MXRMLOG: Delete profiles error. ERROR CODE: %d", result);
		MessageAndLogging(hInstall, TRUE, wstrMsg);
		return ERROR_INSTALL_FAILURE;
	}
}
