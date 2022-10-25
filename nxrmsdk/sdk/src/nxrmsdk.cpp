

#include <Windows.h>

#include <boost\algorithm\string.hpp>

#include "nxrmsdk.h"


void* __stdcall RSAllocEx(unsigned long size, bool init, unsigned char init_ch)
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

void* __stdcall RSAlloc(unsigned long size)
{
    return RSAllocEx(size, TRUE, 0);
}

void __stdcall RSFree(void* mem)
{
    if (NULL != mem) {
        free(mem);
    }
}