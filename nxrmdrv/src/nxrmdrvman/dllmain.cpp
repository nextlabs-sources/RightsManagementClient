// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "nxrmdrvman.h"

BOOL APIENTRY DllMain( HMODULE hModule,
					   DWORD  ul_reason_for_call,
					   LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

ULONG NXRMDRVMAN_API __stdcall nxrmdrvmanEnableAntiTampering(BOOL Enable)
{
	ULONG uRet = ERROR_SUCCESS;

	HANDLE hnxrmdrv = INVALID_HANDLE_VALUE;

	ULONG pid = Enable?GetCurrentProcessId():0;

	ULONG ulbytesret = 0;

	do 
	{
		hnxrmdrv = CreateFile(NXRMDRV_WIN32_DEVICE_NAME,
							  GENERIC_READ|GENERIC_WRITE,
							  FILE_SHARE_READ|FILE_SHARE_WRITE,
							  NULL,
							  OPEN_EXISTING,
							  FILE_ATTRIBUTE_NORMAL,
							  NULL);

		if(hnxrmdrv == INVALID_HANDLE_VALUE)
		{
			uRet = GetLastError();
			break;
		}

		if(!DeviceIoControl(hnxrmdrv,
							IOCTL_NXRMDRV_SET_PROTECTED_PID,
							&pid,
							sizeof(pid),
							NULL,
							0,
							&ulbytesret,
							NULL))
		{
			uRet = GetLastError();
			break;
		}

	} while (FALSE);

	if(hnxrmdrv != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hnxrmdrv);
		hnxrmdrv = INVALID_HANDLE_VALUE;
	}

	return uRet;
}