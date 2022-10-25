// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			OUTMSG1("DLL process attach, processID=%d\n", GetCurrentProcessId());
			break;
		case DLL_THREAD_ATTACH:
			OUTMSG1("DLL thread attach, threadID=%d\n", GetCurrentThreadId());
			break;
		case DLL_THREAD_DETACH:
			OUTMSG1("DLL thread detach, threadID=%d\n", GetCurrentThreadId());
			break;
		case DLL_PROCESS_DETACH:
			OUTMSG1("DLL process detach, processID=%d\n", GetCurrentProcessId());
			break;
	}
	
	return TRUE;
}

