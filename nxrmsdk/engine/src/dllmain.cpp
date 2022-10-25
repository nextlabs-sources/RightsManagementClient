

#include <Windows.h>




BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID lpvReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    case DLL_THREAD_ATTACH:
        CoInitialize(NULL);
        break;
    case DLL_THREAD_DETACH:
        CoUninitialize();
        break;
    }

    return TRUE;
}