
#include <windows.h>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  dwReasonForCall,
                       LPVOID lpReserved
					 )
{
	switch (dwReasonForCall)
	{
	case DLL_PROCESS_ATTACH:
        ::DisableThreadLibraryCalls(hModule);
        break;
    case DLL_PROCESS_DETACH:
        break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}

	return TRUE;
}

