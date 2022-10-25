


#pragma once
#ifndef __NKDF_STRING_H__
#define __NKDF_STRING_H__


#define NKDECLARE_CONST_UNICODE_STRING(_var, _string)   \
    const WCHAR _var ## _buffer[] = _string;            \
    __pragma(warning(push))                             \
    __pragma(warning(disable:4221))                     \
    __pragma(warning(disable:4204))                     \
    const UNICODE_STRING _var = { sizeof(_string) - sizeof(WCHAR), sizeof(_string), (PWCH) _var ## _buffer } \
    __pragma(warning(pop))

#define NKDECLARE_GLOBAL_CONST_UNICODE_STRING(_var, _str)   \
    extern const __declspec(selectany) UNICODE_STRING _var = RTL_CONSTANT_STRING(_str)


VOID
NkFreeString(
    _In_ PUNICODE_STRING String
    );


_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkFormatStringW(
    _Out_ PUNICODE_STRING FormattedString,  // Allocated by caller
    _In_ const WCHAR* Format,
    ...
    );

_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkFormatStringA(
    _Out_ PANSI_STRING FormattedString,  // Allocated by caller
    _In_ const CHAR* Format,
    ...
    );


_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkFormatStringExW(
    _Out_ PUNICODE_STRING FormattedString,  // Allocated by this function
    _In_ POOL_TYPE Type,
    _In_ ULONG Tag,
    _In_ const WCHAR* Format,
    ...
    );

_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkFormatStringExA(
    _Out_ PANSI_STRING FormattedString,  // Allocated by this function
    _In_ POOL_TYPE Type,
    _In_ ULONG Tag,
    _In_ const CHAR* Format,
    ...
    );

_Check_return_
USHORT
NkCchLengthA(
    _In_ const CHAR* s
    );

_Check_return_
USHORT
NkCchLengthW(
    _In_ const WCHAR* s
    );

_Check_return_
LONG
NkCompareCchA(
    _In_ CHAR C1,
    _In_ CHAR C2,
    _In_ BOOLEAN CaseInSensitive
    );

_Check_return_
LONG
NkCompareCchW(
    _In_ WCHAR C1,
    _In_ WCHAR C2,
    _In_ BOOLEAN CaseInSensitive
    );

_Check_return_
LONG
NkCompareStringA(
    _In_ const CHAR* Str1,
    _In_ const CHAR* Str2,
    _In_ BOOLEAN CaseInSensitive
    );

_Check_return_
LONG
NkCompareStringW(
    _In_ const WCHAR* Str1,
    _In_ const WCHAR* Str2,
    _In_ BOOLEAN CaseInSensitive
    );

_Check_return_
LONG
NkCompareUtf16(
    _In_ PCUNICODE_STRING Str1,
    _In_ PCUNICODE_STRING Str2,
    _In_ BOOLEAN CaseInSensitive
    );

_Check_return_
BOOLEAN
NkEqualStringA(
    _In_ const CHAR* Str1,
    _In_ const CHAR* Str2,
    _In_ BOOLEAN CaseInSensitive
    );

_Check_return_
BOOLEAN
NkEqualStringW(
    _In_ const WCHAR* Str1,
    _In_ const WCHAR* Str2,
    _In_ BOOLEAN CaseInSensitive
    );

_Check_return_
BOOLEAN
NkEqualUtf16(
    _In_ PCUNICODE_STRING Str1,
    _In_ PCUNICODE_STRING Str2,
    _In_ BOOLEAN CaseInSensitive
    );

_Check_return_
BOOLEAN
NkStartsWithA(
    _In_ const CHAR* Str1,
    _In_ const CHAR* Str2,
    _In_ BOOLEAN CaseInSensitive
    );

_Check_return_
BOOLEAN
NkStartsWithW(
    _In_ const WCHAR* Str1,
    _In_ const WCHAR* Str2,
    _In_ BOOLEAN CaseInSensitive
    );

_Check_return_
BOOLEAN
NkStartsWithUtf16(
    _In_ PCUNICODE_STRING Str1,
    _In_ PCUNICODE_STRING Str2,
    _In_ BOOLEAN CaseInSensitive
    );

_Check_return_
BOOLEAN
NkEndsWithA(
    _In_ const CHAR* Str1,
    _In_ const CHAR* Str2,
    _In_ BOOLEAN CaseInSensitive
    );

_Check_return_
BOOLEAN
NkEndsWithW(
    _In_ const WCHAR* Str1,
    _In_ const WCHAR* Str2,
    _In_ BOOLEAN CaseInSensitive
    );

_Check_return_
BOOLEAN
NkEndsWithUtf16(
    _In_ PCUNICODE_STRING Str1,
    _In_ PCUNICODE_STRING Str2,
    _In_ BOOLEAN CaseInSensitive
    );

_Check_return_
const CHAR*
NkFindCharA(
    _In_ const CHAR* s,
    _In_ const CHAR c,
    _In_ BOOLEAN CaseInSensitive
    );

_Check_return_
const WCHAR*
NkFindCharW(
    _In_ const WCHAR* s,
    _In_ const WCHAR c,
    _In_ BOOLEAN CaseInSensitive
    );

_Check_return_
UNICODE_STRING
NkFindUnicodeString(
    _In_ PCUNICODE_STRING s,
    _In_ const WCHAR c,
    _In_ BOOLEAN CaseInSensitive
    );

_Check_return_
const CHAR*
NkFindFirstOfA(
    _In_ const CHAR* s,
    _In_ const CHAR* c,
    _In_ BOOLEAN CaseInSensitive
    );

_Check_return_
const WCHAR*
NkFindFirstOfW(
    _In_ const WCHAR* s,
    _In_ const WCHAR* c,
    _In_ BOOLEAN CaseInSensitive
    );

_Check_return_
UNICODE_STRING
NkFindFirstOfUtf16(
    _In_ PCUNICODE_STRING s,
    _In_ const WCHAR* c,
    _In_ BOOLEAN CaseInSensitive
    );



#endif