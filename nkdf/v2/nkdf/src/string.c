

#include "stdafx.h"


#include <nkdf\string.h>

VOID
NkFreeString(
    _In_ PUNICODE_STRING String
    )
{
    if (NULL != String->Buffer) {
        ExFreePool(String->Buffer);
        RtlZeroMemory(String, sizeof(UNICODE_STRING));
    }
}

_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkFormatStringW(
    _Inout_ PUNICODE_STRING FormattedString,
    _In_ const WCHAR* Format,
    ...
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    va_list va;

    try {
        va_start(va, Format);
        Status = RtlStringCbVPrintfW(FormattedString->Buffer, FormattedString->MaximumLength, Format, va);
        va_end(va);
    }
    finally {
        ; // Nothing
    }

    return Status;
}

_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkFormatStringA(
    _Inout_ PANSI_STRING FormattedString,
    _In_ const CHAR* Format,
    ...
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    va_list va;

    try {
        va_start(va, Format);
        Status = RtlStringCbVPrintfA(FormattedString->Buffer, FormattedString->MaximumLength, Format, va);
        va_end(va);
    }
    finally {
        ; // Nothing
    }

    return Status;
}

_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkFormatStringExW(
    _Out_ PUNICODE_STRING FormattedString,  // Allocated by this function
    _In_ POOL_TYPE Type,
    _In_ ULONG Tag,
    _In_ const WCHAR* Format,
    ...
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    va_list va;
    WCHAR* Buffer = NULL;

    try {

        size_t RequiredLength = 0;

        Status = RtlStringCbLengthW(Format, 0x7FFF, &RequiredLength);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        do {

            RequiredLength += 256;    // Extra 128 characters to hold parameters
            Buffer = ExAllocatePoolWithTag(Type, RequiredLength, Tag);
            if (NULL == Buffer) {
                try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
            }
            RtlZeroMemory(Buffer, RequiredLength);

            va_start(va, Format);
            Status = RtlStringCbVPrintfW(Buffer, RequiredLength, Format, va);
            va_end(va);

            if (Status == STATUS_BUFFER_OVERFLOW) {
                // buffer is not big enough
                ExFreePool(Buffer);
                Buffer = NULL;
                continue;
            }

            if (NT_SUCCESS(Status)) {
                RtlInitUnicodeString(FormattedString, Buffer);
                FormattedString->MaximumLength = (USHORT)RequiredLength;
                Buffer = NULL;
            }
            break;

        } while (STATUS_BUFFER_OVERFLOW == Status);

    try_exit: NOTHING;
    }
    finally {

        if (NULL != Buffer) {
            ExFreePool(Buffer);
            Buffer = NULL;
        }
    }

    return Status;
}

_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkFormatStringExA(
    _Out_ PANSI_STRING FormattedString,  // Allocated by this function
    _In_ POOL_TYPE Type,
    _In_ ULONG Tag,
    _In_ const CHAR* Format,
    ...
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    va_list va;
    CHAR* Buffer = NULL;

    try {

        size_t RequiredLength = 0;

        Status = RtlStringCbLengthA(Format, 0x7FFF, &RequiredLength);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        do {

            RequiredLength += 128;    // Extra 128 characters to hold parameters
            Buffer = ExAllocatePoolWithTag(Type, RequiredLength, Tag);
            if (NULL == Buffer) {
                try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
            }
            RtlZeroMemory(Buffer, RequiredLength);

            va_start(va, Format);
            Status = RtlStringCbVPrintfA(Buffer, RequiredLength, Format, va);
            va_end(va);

            if (Status == STATUS_BUFFER_OVERFLOW) {
                // buffer is not big enough
                ExFreePool(Buffer);
                Buffer = NULL;
                continue;
            }

            if (NT_SUCCESS(Status)) {
                RtlInitAnsiString(FormattedString, Buffer);
                FormattedString->MaximumLength = (USHORT)RequiredLength;
                Buffer = NULL;
            }
            break;

        } while (STATUS_BUFFER_OVERFLOW == Status);

    try_exit: NOTHING;
    }
    finally {

        if (NULL != Buffer) {
            ExFreePool(Buffer);
            Buffer = NULL;
        }
    }

    return Status;
}


_Check_return_
USHORT
NkCchLengthA(
    _In_ const CHAR* s
    )
{
    USHORT i = 0;
    while (0 != s[i]) {
        i++;
    }
    return i;
}

_Check_return_
USHORT
NkCchLengthW(
    _In_ const WCHAR* s
    )
{
    USHORT i = 0;
    while (0 != s[i]) {
        i++;
    }
    return i;
}

_Check_return_
LONG
NkCompareCchA(
    _In_ CHAR C1,
    _In_ CHAR C2,
    _In_ BOOLEAN CaseInSensitive
    )
{
    if (CaseInSensitive) {
        C1 = RtlUpperChar(C1);
        C2 = RtlUpperChar(C2);
    }
    return ((C1 == C2) ? 0 : (C1 > C2 ? 1 : -1));
}

_Check_return_
LONG
NkCompareCchW(
    _In_ WCHAR C1,
    _In_ WCHAR C2,
    _In_ BOOLEAN CaseInSensitive
    )
{
    if (CaseInSensitive) {
        C1 = RtlUpcaseUnicodeChar(C1);
        C2 = RtlUpcaseUnicodeChar(C2);
    }
    return ((C1 == C2) ? 0 : (C1 > C2 ? 1 : -1));
}


_Check_return_
LONG
NkCompareStringA(
    _In_ const CHAR* Str1,
    _In_ const CHAR* Str2,
    _In_ BOOLEAN CaseInSensitive
    )
{
    LONG Result = 0;
    do {
        Result = NkCompareCchA(*Str1, *Str2, CaseInSensitive);
    } while (Result == 0 && 0 != *Str1++ && 0 != *Str2++);
    return Result;
}

_Check_return_
LONG
NkCompareStringW(
    _In_ const WCHAR* Str1,
    _In_ const WCHAR* Str2,
    _In_ BOOLEAN CaseInSensitive
    )
{
    LONG Result = 0;
    do {
        Result = NkCompareCchW(*Str1, *Str2, CaseInSensitive);
    } while (Result == 0 && 0 != *Str1++ && 0 != *Str2++);
    return Result;
}

_Check_return_
LONG
NkCompareUtf16(
    _In_ PCUNICODE_STRING Str1,
    _In_ PCUNICODE_STRING Str2,
    _In_ BOOLEAN CaseInSensitive
    )
{
    LONG Result = 0;
    const USHORT Minlength = ((Str1->Length < Str2->Length) ? Str1->Length : Str2->Length) / sizeof(WCHAR);
    USHORT i = 0;
    while (i < Minlength && 0 == (Result = NkCompareCchW(Str1->Buffer[i], Str2->Buffer[i], CaseInSensitive))) {
        i++;
    }

    return (0 != Result) ? Result : ((Str1->Length == Str2->Length) ? 0 : (Str1->Length > Str2->Length ? 1 : -1));
}

_Check_return_
BOOLEAN
NkEqualStringA(
    _In_ const CHAR* Str1,
    _In_ const CHAR* Str2,
    _In_ BOOLEAN CaseInSensitive
    )
{
    return (0 == NkCompareStringA(Str1, Str2, CaseInSensitive)) ? TRUE : FALSE;
}

_Check_return_
BOOLEAN
NkEqualStringW(
    _In_ const WCHAR* Str1,
    _In_ const WCHAR* Str2,
    _In_ BOOLEAN CaseInSensitive
    )
{
    return (0 == NkCompareStringW(Str1, Str2, CaseInSensitive)) ? TRUE : FALSE;
}

_Check_return_
BOOLEAN
NkEqualUtf16(
    _In_ PCUNICODE_STRING Str1,
    _In_ PCUNICODE_STRING Str2,
    _In_ BOOLEAN CaseInSensitive
    )
{
    return (0 == NkCompareUtf16(Str1, Str2, CaseInSensitive)) ? TRUE : FALSE;
}

_Check_return_
BOOLEAN
NkStartsWithA(
    _In_ const CHAR* Str1,
    _In_ const CHAR* Str2,
    _In_ BOOLEAN CaseInSensitive
    )
{
    if (Str2[0] == 0) {
        return FALSE;
    }

    while (0 != *Str2) {
        if (0 != NkCompareCchA(*Str1++, *Str2++, CaseInSensitive)) {
            return FALSE;
        }
    }

    return TRUE;
}

_Check_return_
BOOLEAN
NkStartsWithW(
    _In_ const WCHAR* Str1,
    _In_ const WCHAR* Str2,
    _In_ BOOLEAN CaseInSensitive
    )
{
    if (Str2[0] == 0) {
        return FALSE;
    }

    while (0 != *Str2) {
        if (0 != NkCompareCchW(*Str1++, *Str2++, CaseInSensitive)) {
            return FALSE;
        }
    }

    return TRUE;
}

_Check_return_
BOOLEAN
NkStartsWithUtf16(
    _In_ PCUNICODE_STRING Str1,
    _In_ PCUNICODE_STRING Str2,
    _In_ BOOLEAN CaseInSensitive
    )
{
    USHORT CmpCch = 0;
    USHORT i = 0;

    if (Str2->Length > Str1->Length) {
        return FALSE;
    }

    CmpCch = Str2->Length / sizeof(WCHAR);
    for (i = 0; i < CmpCch; i++) {
        if (0 != NkCompareCchW(Str1->Buffer[i], Str2->Buffer[i], CaseInSensitive)) {
            return FALSE;
        }
    }
    return TRUE;
}


_Check_return_
BOOLEAN
NkEndsWithA(
    _In_ const CHAR* Str1,
    _In_ const CHAR* Str2,
    _In_ BOOLEAN CaseInSensitive
    )
{
    ANSI_STRING s1;
    ANSI_STRING s2;
    USHORT StartPos = 0;
    USHORT i = 0;

    RtlInitAnsiString(&s1, Str1);
    RtlInitAnsiString(&s2, Str2);

    if (s1.Length < s2.Length) {
        return FALSE;
    }

    StartPos = s1.Length - s2.Length;
    for (i = 0; i < s2.Length; i++) {
        if (0 != NkCompareCchW(s1.Buffer[StartPos + i], s2.Buffer[i], CaseInSensitive)) {
            return FALSE;
        }
    }

    return TRUE;
}

_Check_return_
BOOLEAN
NkEndsWithW(
    _In_ const WCHAR* Str1,
    _In_ const WCHAR* Str2,
    _In_ BOOLEAN CaseInSensitive
    )
{
    UNICODE_STRING s1;
    UNICODE_STRING s2;
    USHORT StartPos = 0;
    USHORT CmpCch = 0;
    USHORT i = 0;

    RtlInitUnicodeString(&s1, Str1);
    RtlInitUnicodeString(&s2, Str2);

    if (s1.Length < s2.Length) {
        return FALSE;
    }

    StartPos = s1.Length - s2.Length;
    CmpCch = s2.Length / sizeof(WCHAR);
    for (i = 0; i < CmpCch; i++) {
        if (0 != NkCompareCchW(s1.Buffer[StartPos + i], s2.Buffer[i], CaseInSensitive)) {
            return FALSE;
        }
    }

    return TRUE;
}

_Check_return_
BOOLEAN
NkEndsWithUtf16(
    _In_ PCUNICODE_STRING Str1,
    _In_ PCUNICODE_STRING Str2,
    _In_ BOOLEAN CaseInSensitive
    )
{
    USHORT StartPos = 0;
    USHORT CmpCch = 0;
    USHORT i = 0;

    if (Str1->Length < Str2->Length) {
        return FALSE;
    }

    StartPos = Str1->Length - Str2->Length;
    CmpCch = Str2->Length / sizeof(WCHAR);
    for (i = 0; i < CmpCch; i++) {
        if (0 != NkCompareCchW(Str1->Buffer[StartPos + i], Str2->Buffer[i], CaseInSensitive)) {
            return FALSE;
        }
    }

    return TRUE;
}

_Check_return_
const CHAR*
NkFindCharA(
    _In_ const CHAR* s,
    _In_ const CHAR c,
    _In_ BOOLEAN CaseInSensitive
    )
{
    while (0 != *s && 0 != NkCompareCchA(*s, c, CaseInSensitive)) {
        s++;
    }
    return (0 == *s) ? NULL : s;
}

_Check_return_
const WCHAR*
NkFindCharW(
    _In_ const WCHAR* s,
    _In_ const WCHAR c,
    _In_ BOOLEAN CaseInSensitive
    )
{
    while (0 != *s && 0 != NkCompareCchW(*s, c, CaseInSensitive)) {
        s++;
    }
    return (0 == *s) ? NULL : s;
}

_Check_return_
UNICODE_STRING
NkFindUtf16(
    _In_ PCUNICODE_STRING s,
    _In_ const WCHAR c,
    _In_ BOOLEAN CaseInSensitive
    )
{
    UNICODE_STRING s2;

    RtlCopyMemory(&s2, s, sizeof(UNICODE_STRING));
    while (s2.Length != 0 && 0 != NkCompareCchW(s2.Buffer[0], c, CaseInSensitive)) {
        s2.Length -= 2;
        s2.MaximumLength -= 2;
        (0 == s2.Length) ? (s2.Buffer++) : (s2.Buffer = NULL);
    }
    if (s2.Length == 0) {
        s2.MaximumLength = 0;
        s2.Buffer = NULL;
    }
    return s2;
}

_Check_return_
const CHAR*
NkFindFirstOfA(
    _In_ const CHAR* s,
    _In_ const CHAR* c,
    _In_ BOOLEAN CaseInSensitive
    )
{
    while (0 != *s) {
        INT i = 0;
        while (0 != c[i]) {
            if (0 == NkCompareCchA(*s, c[i], CaseInSensitive)) {
                return s;
            }
            i++;
        }
        s++;
    }
    return NULL;
}

_Check_return_
const WCHAR*
NkFindFirstOfW(
    _In_ const WCHAR* s,
    _In_ const WCHAR* c,
    _In_ BOOLEAN CaseInSensitive
    )
{
    while (0 != *s) {
        INT i = 0;
        while (0 != c[i]) {
            if (0 == NkCompareCchW(*s, c[i], CaseInSensitive)) {
                return s;
            }
            i++;
        }
        s++;
    }
    return NULL;
}

_Check_return_
UNICODE_STRING
NkFindFirstOfUtf16(
    _In_ PCUNICODE_STRING s,
    _In_ const WCHAR* c,
    _In_ BOOLEAN CaseInSensitive
    )
{
    UNICODE_STRING s2;

    RtlCopyMemory(&s2, s, sizeof(UNICODE_STRING));
    while (0 != s2.Length) {
        INT i = 0;
        while (0 != c[i]) {
            if (0 == NkCompareCchW(s2.Buffer[0], c[i], CaseInSensitive)) {
                return s2;
            }
            i++;
        }
        s2.Length -= 2;
        s2.MaximumLength -= 2;
        (0 == s2.Length) ? (s2.Buffer++) : (s2.Buffer = NULL);
    }
    if (s2.Length == 0) {
        s2.MaximumLength = 0;
        s2.Buffer = NULL;
    }
    return s2;
}