

#include <Windows.h>


#include <boost\algorithm\string.hpp>

#include "nxrmesdk.h"




void* __stdcall RSEAllocEx(unsigned long size, bool init, unsigned char init_ch)
{
    void* p = NULL;
    if (size == 0) {
        return p;
    }
    p = malloc(size);
    if (NULL != p && init) {
        memset(p, init_ch, size);
    }
    return p;
}

void* __stdcall RSEAlloc(unsigned long size)
{
    return RSEAllocEx(size, TRUE, 0);
}

void __stdcall RSEFree(void* mem)
{
    if (NULL != mem) {
        free(mem);
    }
}


HANDLE WINAPI RSECreate(const wchar_t* work_dir)
{
    return NULL;
}

void WINAPI RSEDestroy(HANDLE engine_handle)
{
}

bool WINAPI RSEConfig(HANDLE engine_handle)
{
    return false;
}

bool WINAPI RSEStart(HANDLE engine_handle)
{
    return false;
}

bool WINAPI RSEStop(HANDLE engine_handle)
{
    return false;
}
