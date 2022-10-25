

#include <ntifs.h>
#include <Ntstrsafe.h>


#include <nkdf/basic/defines.h>
#include <nkdf/basic/string.h>



//
//  Compare
//

_Check_return_
LONG
NkCompareStringA(
                 _In_ const CHAR* Str1,
                 _In_ const CHAR* Str2,
                 _In_ BOOLEAN CaseInSensitive
                 )
{
    LONG Ret = 0;
    do {        
        Ret = NkCompareA(*Str1, *Str2, CaseInSensitive);
    } while (0 == Ret && (0 != *(Str1++)) && (0 != *(Str2++)));
    return Ret;
}

_Check_return_
LONG
NkCompareStringW(
                 _In_ const WCHAR* Str1,
                 _In_ const WCHAR* Str2,
                 _In_ BOOLEAN CaseInSensitive
                )
{
    LONG Ret = 0;
    do {        
        Ret = NkCompareW(*Str1, *Str2, CaseInSensitive);
    } while (0 == Ret && (0 != *(Str1++)) && (0 != *(Str2++)));
    return Ret;
}

_Check_return_
LONG
NkCompareUnicodeString(
                       _In_ PCUNICODE_STRING Str1,
                       _In_ PCUNICODE_STRING Str2,
                       _In_ BOOLEAN CaseInSensitive
                       )
{
    USHORT i = 0;

	if (Str1->Length != Str2->Length) {
        return (Str1->Length > Str2->Length) ? 1 : -1;
    }
    else {
        for ( i = 0; i < (Str1->Length / sizeof(WCHAR)); i++) {
            LONG Ret = NkCompareW(Str1->Buffer[i], Str2->Buffer[i], CaseInSensitive);
            if (Ret != 0) {
                return Ret;
            }
        }
    }
    return 0;
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
NkEqualUnicodeString(
                     _In_ PCUNICODE_STRING Str1,
                     _In_ PCUNICODE_STRING Str2,
                     _In_ BOOLEAN CaseInSensitive
                     )
{
    return (0 == NkCompareUnicodeString(Str1, Str2, CaseInSensitive)) ? TRUE : FALSE;
}

_Check_return_
BOOLEAN
NkStartsWithA(
              _In_ const CHAR* Str1,
              _In_ const CHAR* Str2,
              _In_ BOOLEAN CaseInSensitive
              )
{
    if (NULL == Str2 || 0 == *Str2) {
        return FALSE;
    }

    while (0 != *Str2) {
        if (0 != NkCompareA(*(Str1++), *(Str2++), CaseInSensitive)) {
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
    if (NULL == Str2 || 0 == *Str2) {
        return FALSE;
    }

    while (0 != *Str2) {
        if (0 != NkCompareW(*(Str1++), *(Str2++), CaseInSensitive)) {
            return FALSE;
        }
    }
    return TRUE;
}

_Check_return_
BOOLEAN
NkStartsWithUnicodeString(
                          _In_ PCUNICODE_STRING Str1,
                          _In_ PCUNICODE_STRING Str2,
                          _In_ BOOLEAN CaseInSensitive
                          )
{
	USHORT i = 0;

    if (Str1->Length < Str2->Length) {
        return FALSE;
    }
    for (i = 0; i < (Str2->Length / sizeof(WCHAR)); i++) {
        if (0 != NkCompareW(Str1->Buffer[i], Str2->Buffer[i], CaseInSensitive)) {
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
    ANSI_STRING as1;
    ANSI_STRING as2;
    USHORT Offset = 0;
	USHORT i = 0;

    RtlInitAnsiString(&as1, Str1);
    RtlInitAnsiString(&as2, Str2);


    if (as1.Length < as2.Length) {
        return FALSE;
    }

    Offset = as1.Length - as2.Length;
    for (i = 0; i < as2.Length; i++) {
        if (0 != NkCompareW(as1.Buffer[Offset + i], as2.Buffer[i], CaseInSensitive)) {
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
    UNICODE_STRING us1;
    UNICODE_STRING us2;

    RtlInitUnicodeString(&us1, Str1);
    RtlInitUnicodeString(&us2, Str2);
    return NkEndsWithUnicodeString(&us1, &us2, CaseInSensitive);
}

_Check_return_
BOOLEAN
NkEndsWithUnicodeString(
                        _In_ PCUNICODE_STRING Str1,
                        _In_ PCUNICODE_STRING Str2,
                        _In_ BOOLEAN CaseInSensitive
                        )
{
    USHORT Offset = 0;
	USHORT i = 0;

    if (Str1->Length < Str2->Length) {
        return FALSE;
    }

    Offset = (Str1->Length - Str2->Length) / sizeof(WCHAR);
    for (i = 0; i < (Str2->Length / sizeof(WCHAR)); i++) {
        if (0 != NkCompareW(Str1->Buffer[Offset + i], Str2->Buffer[i], CaseInSensitive)) {
            return FALSE;
        }
    }
    return TRUE;
}

_Check_return_
CHAR*
NkFindA(
        _In_ const CHAR* Str1,
        _In_ const CHAR* Str2,
        _In_ BOOLEAN CaseInSensitive
        )
{
    const CHAR *cp = (const CHAR *)Str1;
    const CHAR *s1 = NULL;
    const CHAR *s2 = NULL;

    if (!*Str2)
        return((CHAR*)Str1);

    while (*cp)
    {
        s1 = cp;
        s2 = (const CHAR *)Str2;

        while (*s1 && *s2 && NkEqualA(*s1, *s2, CaseInSensitive))
            s1++, s2++;

        if (!*s2)
            return((CHAR*)cp);

        cp++;
    }

    return NULL;
}

_Check_return_
WCHAR*
NkFindW(
        _In_ const WCHAR* Str1,
        _In_ const WCHAR* Str2,
        _In_ BOOLEAN CaseInSensitive
        )
{
    const WCHAR *cp = (const WCHAR *)Str1;
    const WCHAR *s1 = NULL;
    const WCHAR *s2 = NULL;

    if (!*Str2)
        return((WCHAR*)Str1);

    while (*cp)
    {
        s1 = cp;
        s2 = (const WCHAR *)Str2;

        while (*s1 && *s2 && NkEqualW(*s1, *s2, CaseInSensitive))
            s1++, s2++;

        if (!*s2)
            return((WCHAR*)cp);

        cp++;
    }

    return NULL;
}

_Check_return_
UNICODE_STRING
NkFindUnicodeString(
                    _In_ PCUNICODE_STRING Str1,
                    _In_ PCUNICODE_STRING Str2,
                    _In_ BOOLEAN CaseInSensitive
                    )
{
    UNICODE_STRING cp = { 0, 0, NULL };

    if (Str1->Length < Str2->Length) {
        return cp;
    }
    else if (Str1->Length == Str2->Length) {
        if (NkEqualUnicodeString(Str1, Str2, CaseInSensitive)) {
            RtlCopyMemory(&cp, Str1, sizeof(UNICODE_STRING));
        }
        return cp;
    }
    else {
        
        RtlCopyMemory(&cp, Str1, sizeof(UNICODE_STRING));
        while (cp.Length >= Str2->Length) {
            if (NkStartsWithUnicodeString(&cp, Str2, CaseInSensitive)) {
                return cp;
            }
            cp.Buffer++;
            cp.Length -= sizeof(WCHAR);
            cp.MaximumLength -= sizeof(WCHAR);
        }
    }

    RtlZeroMemory(&cp, sizeof(cp));
    return cp;
}

_Check_return_
BOOLEAN
NkContainsA(
            _In_ const CHAR* Str1,
            _In_ const CHAR* Str2,
            _In_ BOOLEAN CaseInSensitive
            )
{
    return (NULL != NkFindA(Str1, Str2, CaseInSensitive)) ? TRUE : FALSE;
}

_Check_return_
BOOLEAN
NkContainsW(
            _In_ const WCHAR* Str1,
            _In_ const WCHAR* Str2,
            _In_ BOOLEAN CaseInSensitive
            )
{
    return (NULL != NkFindW(Str1, Str2, CaseInSensitive)) ? TRUE : FALSE;
}

_Check_return_
BOOLEAN
NkContainsUnicodeString(
                        _In_ PCUNICODE_STRING Str1,
                        _In_ PCUNICODE_STRING Str2,
                        _In_ BOOLEAN CaseInSensitive
                        )
{
    UNICODE_STRING cp = { 0, 0, NULL };
    cp = NkFindUnicodeString(Str1, Str2, CaseInSensitive);
    return NkIsEmptyUnicodeString(&cp) ? FALSE : TRUE;
}


/*
For example:
A:
OriginalPatterns    = L"*.test"
Return: Token       = L'*'
Pattern     = L".test"
NewPatterns = NULL
B:
OriginalPatterns    = L"testB://*xyz"
Return: Token       = L'*'
Pattern     = L"testB://"
NewPatterns = L"*xyz"
C:
OriginalPatterns    = L"no wildcard"
Return: Token       = TOKEN_NOT_FOUND
Pattern     = L"no wildcard"
NewPatterns = NULL
D:
OriginalPatterns    = L"?:\\test\foo*.doc"
Return: Token       = L'?'
Pattern     = L":\\test\foo"
NewPatterns = L".doc"
*/
WCHAR
NkPatternSplit(
               _In_ UNICODE_STRING OriginalPatterns,
               _Out_ PUNICODE_STRING Pattern,
               _Out_ PUNICODE_STRING NewPatterns
               )
{
    static const WCHAR TOKEN_NOT_FOUND = 0xFFFF;
    WCHAR   Token = TOKEN_NOT_FOUND;
    USHORT  i;
    USHORT  Loop;

    RtlZeroMemory(NewPatterns, sizeof(UNICODE_STRING));

    ASSERT(OriginalPatterns.Length != 0);
    if (L'*' == OriginalPatterns.Buffer[0]) {
        Token = L'*';
        Pattern->Buffer = OriginalPatterns.Buffer + 1;
        Pattern->Length = OriginalPatterns.Length - sizeof(WCHAR);
        Pattern->MaximumLength = Pattern->Length;
    }
    else if (L'?' == OriginalPatterns.Buffer[0]) {
        Token = L'?';
        Pattern->Buffer = OriginalPatterns.Buffer + 1;
        Pattern->Length = OriginalPatterns.Length - sizeof(WCHAR);
        Pattern->MaximumLength = Pattern->Length;
    }
    else {
        Token = TOKEN_NOT_FOUND;
        Pattern->Buffer = OriginalPatterns.Buffer;
        Pattern->Length = OriginalPatterns.Length;
        Pattern->MaximumLength = Pattern->Length;
    }

    if (0 == Pattern->Length) {
        Pattern->Buffer = NULL;
        return Token;
    }

    Loop = Pattern->Length / sizeof(WCHAR);
    for (i = 0; i<Loop; i++) {
        if (Pattern->Buffer[i] == L'*' || Pattern->Buffer[i] == L'?') {
            NewPatterns->Buffer = &Pattern->Buffer[i];
            NewPatterns->Length = Pattern->Length - i*sizeof(WCHAR);
            NewPatterns->MaximumLength = NewPatterns->Length;

            Pattern->Length = Pattern->Length - NewPatterns->Length;
            if (0 == Pattern->Length) {
                Pattern->Buffer = NULL;
            }
            break;
        }
    }

    return Token;
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
BOOLEAN
NkStringMatch(
              _In_ UNICODE_STRING Str,
              _In_ UNICODE_STRING Pattern,
              _In_ BOOLEAN CaseInSensitive
              )
{
    static const WCHAR TOKEN_NOT_FOUND = 0xFFFF;

    do {

        WCHAR           CurrentToken;
        UNICODE_STRING  CurrentPattern;

        // Get current pattern compare part
        CurrentToken = NkPatternSplit(Pattern, &CurrentPattern, &Pattern);

        if (TOKEN_NOT_FOUND == CurrentToken) {

            if (CurrentPattern.Length == 0) {
                continue;
            }

            if (!NkStartsWithUnicodeString(&Str, &CurrentPattern, CaseInSensitive)) {
                return FALSE;
            }

            Str.Buffer += CurrentPattern.Length / sizeof(WCHAR);
            Str.Length -= CurrentPattern.Length;
            Str.MaximumLength = Str.Length;
        }
        else if (L'?' == CurrentToken){
            if (Str.Length < sizeof(WCHAR)) {
                return FALSE;
            }
            // Ignore one character
            Str.Buffer++;
            Str.Length -= sizeof(WCHAR);
            Str.MaximumLength = Str.Length;
            // Compare
            if (CurrentPattern.Length == 0) {
                // There are two situations:
                // a. Current token is followed by another token
                // b. This is the last character in the pattern

                // #b
                if (0 == Pattern.Length) {
                    // If source string ends at the same time, then it match the pattern
                    // Otherwise it doesn't match
                    return ((0 == Str.Length) ? TRUE : FALSE);
                }

                // #a
                // Move to next
                continue;
            }
            if (!NkStartsWithUnicodeString(&Str, &CurrentPattern, CaseInSensitive)) {
                return FALSE;
            }
            // Move to next
            Str.Buffer += CurrentPattern.Length / sizeof(WCHAR);
            Str.Length -= CurrentPattern.Length;
            Str.MaximumLength = Str.Length;
        }
        else if (L'*' == CurrentToken){

            if (CurrentPattern.Length == 0) {
                // There are two situations:
                // a. Current token is followed by another token
                // b. This is the last character in the pattern

                // #b
                if (0 == Pattern.Length) {
                    return TRUE;
                }

                // #a
                // This should never happen, but to let compare continue, we move to next token
                continue;
            }

            // Don't have it?
            Str = NkFindUnicodeString(&Str, &CurrentPattern, CaseInSensitive);
            if (Str.Length == 0) {
                return FALSE;
            }

            // Move to next
            Str.Buffer += CurrentPattern.Length / sizeof(WCHAR);
            Str.Length -= CurrentPattern.Length;
            Str.MaximumLength = Str.Length;
        }
        else {
            // Shouldn't reach here
            ASSERT(FALSE);
        }

    } while (Pattern.Length != 0);

    return TRUE;
}
              
//separate a string based on a specific token
_IRQL_requires_max_(DISPATCH_LEVEL)
VOID
NkSplitUnicodeString (
                      _In_ const UNICODE_STRING Str,
                      _In_ WCHAR Token,
                      _Out_ PUNICODE_STRING FirstPart,
                      _Out_ PUNICODE_STRING RemainPart
                      )
{
    USHORT  i;
    USHORT  Loop;

    RtlZeroMemory(FirstPart, sizeof(UNICODE_STRING));
    RemainPart->Buffer = Str.Buffer;
    RemainPart->Length = Str.Length;
    RemainPart->MaximumLength = Str.MaximumLength;

    Loop = Str.Length / sizeof(WCHAR);
    for(i=0; i<Loop; i++) {

        ASSERT(NULL != RemainPart->Buffer);

        RemainPart->Buffer++;
        RemainPart->Length -= sizeof(WCHAR);
        RemainPart->MaximumLength = RemainPart->Length;
        if(0 == RemainPart->Length) {
            RemainPart->Buffer = NULL;
        }

        if(Token == Str.Buffer[i]) {
            FirstPart->Buffer = ((0 == i) ? NULL : Str.Buffer);
            FirstPart->Length = i * sizeof(WCHAR);
            FirstPart->MaximumLength = FirstPart->Length;
            return;
        }
    }

    // Not found? Return whole string
    ASSERT(0 == RemainPart->Length);
    ASSERT(NULL == RemainPart->Buffer);

    FirstPart->Buffer = Str.Buffer;
    FirstPart->Length = Str.Length;
    FirstPart->MaximumLength = FirstPart->Length;
}


_IRQL_requires_max_(DISPATCH_LEVEL)
VOID
NkBackSplitUnicodeString (
                          _In_ const UNICODE_STRING Str,
                          _In_ WCHAR Token,
                          _Out_ PUNICODE_STRING FinalPart,
                          _Out_ PUNICODE_STRING RemainPart
                          )
{
    USHORT  i;
    USHORT  Loop;

    RtlZeroMemory(FinalPart, sizeof(UNICODE_STRING));
    RemainPart->Buffer = Str.Buffer;
    RemainPart->Length = Str.Length;
    RemainPart->MaximumLength = Str.Length;

    Loop = Str.Length / sizeof(WCHAR);

    // Is last character match token?
    if (Str.Buffer[Loop - 1] == Token) {
        RemainPart->Length -= sizeof(WCHAR);
        RemainPart->MaximumLength -= sizeof(WCHAR);
        return;
    }

    for (i = 0; i<Loop; i++) {

        ASSERT(NULL != RemainPart->Buffer);
        if (Token == Str.Buffer[Loop - 1 - i]) {
            
            FinalPart->Buffer = &(Str.Buffer[Loop - i]);
            FinalPart->Length = i * sizeof(WCHAR);
            FinalPart->MaximumLength = FinalPart->Length;
            RemainPart->Length -= ((i+1) * sizeof(WCHAR));
            RemainPart->MaximumLength = RemainPart->Length;
            return;
        }
    }

    // Not found? Return whole string
    FinalPart->Buffer = Str.Buffer;
    FinalPart->Length = Str.Length;
    FinalPart->MaximumLength = FinalPart->Length;
    RtlZeroMemory(RemainPart, sizeof(UNICODE_STRING));
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkAllocUnicodeString(
                     _Out_ PUNICODE_STRING String,
                     _In_ PCUNICODE_STRING Source,
                     _In_ BOOLEAN PagePool,
                     _In_ ULONG PoolTag
                     )
{
    NTSTATUS Status;

    Status = NkAllocEmptyUnicodeString(String, Source->Length + sizeof(WCHAR), PagePool, PoolTag);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    String->Length = Source->Length;
    RtlCopyMemory(String->Buffer, Source->Buffer, Source->Length);
    return STATUS_SUCCESS;
}


_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkAllocEmptyUnicodeString(
                          _Out_ PUNICODE_STRING String,
                          _In_ USHORT MaximumLength,
                          _In_ BOOLEAN PagePool,
                          _In_ ULONG PoolTag
                          )
{
    NTSTATUS    Status = STATUS_SUCCESS;

    try {

        if (0 == MaximumLength) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        String->MaximumLength = MaximumLength;
        String->Buffer = ExAllocatePoolWithTag(PagePool ? PagedPool : NonPagedPool, String->MaximumLength, PoolTag);
        if (NULL == String->Buffer) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory(String->Buffer, String->MaximumLength);
        String->Length = 0;

try_exit: NOTHING;
    }
    finally {

        if (!NT_SUCCESS(Status)) {
            RtlZeroMemory(String, sizeof(UNICODE_STRING));
        }
    }

    return Status;
}



_IRQL_requires_max_(DISPATCH_LEVEL)
VOID
NkGetFileNameFromPath(
                      _In_ const UNICODE_STRING FilePath,
                      _Out_ PUNICODE_STRING FileName
                      )
{
    UNICODE_STRING RemainPart = { 0, 0, NULL };
    NkBackSplitUnicodeString(FilePath, L'\\', FileName, &RemainPart);
}