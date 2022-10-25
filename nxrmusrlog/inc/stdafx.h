// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

void OutputParamFunction(const char *str, ...);
void OutputParamFunction(const wchar_t *str, ...);
#if _DEBUG
#define OUTMSG(a) OutputParamFunction(a)
#define OUTMSG1(a,b1) OutputParamFunction(a,b1)
#define OUTMSG2(a,b1,b2) OutputParamFunction(a,b1,b2)
#define OUTMSG3(a,b1,b2,b3) OutputParamFunction(a,b1,b2,b3)
#else
#define OUTMSG(a)
#define OUTMSG1(a,b1)
#define OUTMSG2(a,b1,b2)
#define OUTMSG3(a,b1,b2,b3)
#endif


// TODO: reference additional headers your program requires here
