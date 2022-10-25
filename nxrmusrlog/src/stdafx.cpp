// stdafx.cpp : source file that includes just the standard includes
// nxrmusrlog.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include <stdlib.h>
#include "stdio.h"
#include "stdarg.h"

#ifdef _DEBUG
void OutputParamFunction(const char *str, ...)
{

	//The three dots represent the variable parameters.To parse it we use the va_(variable) set of functions :

	char buf[2048];

	va_list ptr;
	va_start(ptr, str);
	vsprintf_s(buf, _countof(buf), str, ptr);

	OutputDebugStringA(buf);
}

void OutputParamFunction(const wchar_t *str, ...)
{

	//The three dots represent the variable parameters.To parse it we use the va_(variable) set of functions :

	wchar_t buf[2048];

	va_list ptr;
	va_start(ptr, str);
	vswprintf_s(buf, _countof(buf), str, ptr);
	va_end(ptr);

	OutputDebugStringW(buf);
}

#endif

// and not in this file
