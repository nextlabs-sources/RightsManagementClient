
/**
 * \file <nkdf/basic/string.h>
 * \brief Header file for string help routines
 * This header file declare string help routines
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */

#ifndef __NKDF_BASIC_STRING_H__
#define __NKDF_BASIC_STRING_H__


/**
 * \addtogroup nkdf-basic
 * @{
 */


/**
 * \defgroup nkdf-basic-string String Support
 * @{
 */


/**
 * \defgroup nkdf-basic-string-macro Help Macros
 * @{
 */

/** \def NKDECLARE_CONST_UNICODE_STRING(_var, _string)
 *  Declare const UNICODE_STRING.
 */
#define NKDECLARE_CONST_UNICODE_STRING(_var, _string)   \
    const WCHAR _var ## _buffer[] = _string;            \
    __pragma(warning(push))                             \
    __pragma(warning(disable:4221))                     \
    __pragma(warning(disable:4204))                     \
    const UNICODE_STRING _var = { sizeof(_string) - sizeof(WCHAR), sizeof(_string), (PWCH) _var ## _buffer } \
    __pragma(warning(pop))

/** \def NKDECLARE_GLOBAL_CONST_UNICODE_STRING(_var, _string)
 *  Declare global const UNICODE_STRING.
 */
#define NKDECLARE_GLOBAL_CONST_UNICODE_STRING(_var, _str)   \
    extern const __declspec(selectany) UNICODE_STRING _var = RTL_CONSTANT_STRING(_str)


/**@}*/ // Group End: nkdf-basic-string-macro


/**
 * \defgroup nkdf-basic-string-api Routines
 * @{
 */


/**
 * \brief Compare ansi-string.
 * \param Str1 First string.
 * \param Str2 Second string.
 * \param CaseInSensitive Is case insensitive?
 * \return LONG
 * \retval 0 Str1 equals to Str2
 * \retval 1 Str1 greater than Str2
 * \retval -1 Str1 less than Str2
 */
_Check_return_
LONG
NkCompareStringA(
                 _In_ const CHAR* Str1,
                 _In_ const CHAR* Str2,
                 _In_ BOOLEAN CaseInSensitive
                 );

/**
 * \brief Compare wide-string.
 * \param Str1 First string.
 * \param Str2 Second string.
 * \param CaseInSensitive Is case insensitive?
 * \return LONG
 * \retval 0 Str1 equals to Str2
 * \retval 1 Str1 greater than Str2
 * \retval -1 Str1 less than Str2
 */
_Check_return_
LONG
NkCompareStringW(
                 _In_ const WCHAR* Str1,
                 _In_ const WCHAR* Str2,
                 _In_ BOOLEAN CaseInSensitive
                 );

/**
 * \brief Compare UNICODE_STRING.
 * \param Str1 First string.
 * \param Str2 Second string.
 * \param CaseInSensitive Is case insensitive?
 * \return LONG
 * \retval 0 Str1 equals to Str2
 * \retval 1 Str1 greater than Str2
 * \retval -1 Str1 less than Str2
 */
_Check_return_
LONG
NkCompareUnicodeString(
                       _In_ PCUNICODE_STRING Str1,
                       _In_ PCUNICODE_STRING Str2,
                       _In_ BOOLEAN CaseInSensitive
                       );

/**
 * \brief Check if two ansi strings are equal.
 * \param Str1 First string.
 * \param Str2 Second string.
 * \param CaseInSensitive Is case insensitive?
 * \return BOOLEAN
 * \retval TRUE Equal
 * \retval FALSE Not equal
 */
_Check_return_
BOOLEAN
NkEqualStringA(
               _In_ const CHAR* Str1,
               _In_ const CHAR* Str2,
               _In_ BOOLEAN CaseInSensitive
               );

/**
 * \brief Check if two unicode strings are equal.
 * \param Str1 First string.
 * \param Str2 Second string.
 * \param CaseInSensitive Is case insensitive?
 * \return BOOLEAN
 * \retval TRUE Equal
 * \retval FALSE Not equal
 */
_Check_return_
BOOLEAN
NkEqualStringW(
               _In_ const WCHAR* Str1,
               _In_ const WCHAR* Str2,
               _In_ BOOLEAN CaseInSensitive
               );

/**
 * \brief Check if two UNICODE_STRINGs are equal.
 * \param Str1 First string.
 * \param Str2 Second string.
 * \param CaseInSensitive Is case insensitive?
 * \return BOOLEAN
 * \retval TRUE Equal
 * \retval FALSE Not equal
 */
_Check_return_
BOOLEAN
NkEqualUnicodeString(
                     _In_ PCUNICODE_STRING Str1,
                     _In_ PCUNICODE_STRING Str2,
                     _In_ BOOLEAN CaseInSensitive
                      );

/**
 * \brief Check if Str1 starts with Str2.
 * \param Str1 First string.
 * \param Str2 Second string.
 * \param CaseInSensitive Is case insensitive?
 * \return BOOLEAN
 */
_Check_return_
BOOLEAN
NkStartsWithA(
              _In_ const CHAR* Str1,
              _In_ const CHAR* Str2,
              _In_ BOOLEAN CaseInSensitive
              );

/**
 * \brief Check if Str1 starts with Str2.
 * \param Str1 First string.
 * \param Str2 Second string.
 * \param CaseInSensitive Is case insensitive?
 * \return BOOLEAN
 */
_Check_return_
BOOLEAN
NkStartsWithW(
              _In_ const WCHAR* Str1,
              _In_ const WCHAR* Str2,
              _In_ BOOLEAN CaseInSensitive
              );

/**
 * \brief Check if Str1 starts with Str2.
 * \param Str1 First string.
 * \param Str2 Second string.
 * \param CaseInSensitive Is case insensitive?
 * \return BOOLEAN
 */
_Check_return_
BOOLEAN
NkStartsWithUnicodeString(
                          _In_ PCUNICODE_STRING Str1,
                          _In_ PCUNICODE_STRING Str2,
                          _In_ BOOLEAN CaseInSensitive
                          );

/**
 * \brief Check if Str1 ends with Str2.
 * \param Str1 First string.
 * \param Str2 Second string.
 * \param CaseInSensitive Is case insensitive?
 * \return BOOLEAN
 */
_Check_return_
BOOLEAN
NkEndsWithA(
            _In_ const CHAR* Str1,
            _In_ const CHAR* Str2,
            _In_ BOOLEAN CaseInSensitive
            );

/**
 * \brief Check if Str1 ends with Str2.
 * \param Str1 First string.
 * \param Str2 Second string.
 * \param CaseInSensitive Is case insensitive?
 * \return BOOLEAN
 */
_Check_return_
BOOLEAN
NkEndsWithW(
            _In_ const WCHAR* Str1,
            _In_ const WCHAR* Str2,
            _In_ BOOLEAN CaseInSensitive
            );

/**
 * \brief Check if Str1 ends with Str2.
 * \param Str1 First string.
 * \param Str2 Second string.
 * \param CaseInSensitive Is case insensitive?
 * \return BOOLEAN
 */
_Check_return_
BOOLEAN
NkEndsWithUnicodeString(
                        _In_ PCUNICODE_STRING Str1,
                        _In_ PCUNICODE_STRING Str2,
                        _In_ BOOLEAN CaseInSensitive
                        );

/**
 * \brief Find Str2 ends in Str1.
 * \param Str1 First string.
 * \param Str2 Second string.
 * \param CaseInSensitive Is case insensitive?
 * \return BOOLEAN
 */
_Check_return_
CHAR*
NkFindA(
        _In_ const CHAR* Str1,
        _In_ const CHAR* Str2,
        _In_ BOOLEAN CaseInSensitive
        );

/**
 * \brief Find Str2 ends in Str1.
 * \param Str1 First string.
 * \param Str2 Second string.
 * \param CaseInSensitive Is case insensitive?
 * \return BOOLEAN
 */
_Check_return_
WCHAR*
NkFindW(
        _In_ const WCHAR* Str1,
        _In_ const WCHAR* Str2,
        _In_ BOOLEAN CaseInSensitive
        );

/**
 * \brief Find Str2 ends in Str1.
 * \param Str1 First string.
 * \param Str2 Second string.
 * \param CaseInSensitive Is case insensitive?
 * \return BOOLEAN
 */
_Check_return_
UNICODE_STRING
NkFindUnicodeString(
                    _In_ PCUNICODE_STRING Str1,
                    _In_ PCUNICODE_STRING Str2,
                    _In_ BOOLEAN CaseInSensitive
                    );

/**
 * \brief Check if Str1 contains Str2.
 * \param Str1 First string.
 * \param Str2 Second string.
 * \param CaseInSensitive Is case insensitive?
 * \return BOOLEAN
 */
_Check_return_
BOOLEAN
NkContainsA(
            _In_ const CHAR* Str1,
            _In_ const CHAR* Str2,
            _In_ BOOLEAN CaseInSensitive
            );

/**
 * \brief Check if Str1 contains Str2.
 * \param Str1 First string.
 * \param Str2 Second string.
 * \param CaseInSensitive Is case insensitive?
 * \return BOOLEAN
 */
_Check_return_
BOOLEAN
NkContainsW(
            _In_ const WCHAR* Str1,
            _In_ const WCHAR* Str2,
            _In_ BOOLEAN CaseInSensitive
            );

/**
 * \brief Check if Str1 contains Str2.
 * \param Str1 First string.
 * \param Str2 Second string.
 * \param CaseInSensitive Is case insensitive?
 * \return BOOLEAN
 */
_Check_return_
BOOLEAN
NkContainsUnicodeString(
                        _In_ PCUNICODE_STRING Str1,
                        _In_ PCUNICODE_STRING Str2,
                        _In_ BOOLEAN CaseInSensitive
                        );

/**
 * \brief Check if Str match Pattern.
 * \param Str String to check.
 * \param Pattern The pattern (support wildcards).
 * \param CaseInSensitive Is case insensitive?
 * \return BOOLEAN
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
BOOLEAN
NkStringMatch(
              _In_ UNICODE_STRING Str,
              _In_ UNICODE_STRING Pattern,
              _In_ BOOLEAN CaseInSensitive
              );

/**
 * \brief Split a UNICODE_STRING by Token from begining.
 * \param Str String to split.
 * \param Token The character used to split string.
 * \param FirstPart First part after split.
 * \param RemainPart Remaining part after split.
 * \return N/A
 */
_IRQL_requires_max_(DISPATCH_LEVEL)
VOID
NkSplitUnicodeString (
                      _In_ const UNICODE_STRING Str,
                      _In_ WCHAR Token,
                      _Out_ PUNICODE_STRING FirstPart,
                      _Out_ PUNICODE_STRING RemainPart
                      );

/**
 * \brief Split a UNICODE_STRING by Token from end.
 * \param Str String to split.
 * \param Token The character used to split string.
 * \param FinalPart Final part after split.
 * \param RemainPart Remaining part after split.
 * \return N/A
 */
_IRQL_requires_max_(DISPATCH_LEVEL)
VOID
NkBackSplitUnicodeString (
                          _In_ const UNICODE_STRING Str,
                          _In_ WCHAR Token,
                          _Out_ PUNICODE_STRING FinalPart,
                          _Out_ PUNICODE_STRING RemainPart
                          );


/**
 * \brief Allocate a new string which has the same content as the Source.
 * \param String Pointer to UNICODE_STRING to receive allocated new string.
 * \param Source Source string.
 * \param PagePool Paged pool or non-paged pool.
 * \param PoolTag Pool tag for the allocation.
 * \return N/A
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkAllocUnicodeString(
                     _Out_ PUNICODE_STRING String,
                     _In_ PCUNICODE_STRING Source,
                     _In_ BOOLEAN PagePool,
                     _In_ ULONG PoolTag
                     );

/**
 * \brief Allocate an empty UNICODE_STRING.
 * \param String Pointer to UNICODE_STRING to receive allocated new string.
 * \param MaximumLength Buffer size (UNICODE_STRING->MaximumLength), in bytes.
 * \param PagePool Paged pool or non-paged pool.
 * \param PoolTag Pool tag for the allocation.
 * \return NTSTATUS
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkAllocEmptyUnicodeString(
                          _Out_ PUNICODE_STRING String,
                          _In_ USHORT MaximumLength,
                          _In_ BOOLEAN PagePool,
                          _In_ ULONG PoolTag
                          );

/**
 * \brief Get file name from a path.
 * \param FilePath Full path.
 * \param FileName Pointer to a UNICODE_STRING to receive file name.
 * \return N/A
 */
_IRQL_requires_max_(DISPATCH_LEVEL)
VOID
NkGetFileNameFromPath(
                      _In_ const UNICODE_STRING FilePath,
                      _Out_ PUNICODE_STRING FileName
                      );


/**@}*/ // Group End: nkdf-basic-string-api



/**
 * \defgroup nkdf-basic-string-inline Inline Routines
 * @{
 */

/**
 * \brief Convert a character to uppercase
 * \param C Character to convert.
 * \return Uppercase character
 */
__forceinline
_Check_return_
CHAR
NkToUpperA(
           _In_ CHAR C
           )
{
    return (C >= 'a' && C <= 'z') ? (C - 0x20) : C;
}

/**
 * \brief Convert a wide-char character to uppercase
 * \param C Character to convert.
 * \return Uppercase wide-char character
 */
__forceinline
_Check_return_
WCHAR
NkToUpperW(
           _In_ WCHAR C
           )
{
    return (C >= L'a' && C <= L'z') ? (C - 0x20) : C;
}

/**
 * \brief Convert a character to lowercase
 * \param C Character to convert.
 * \return Uppercase character
 */
__forceinline
_Check_return_
CHAR
NkToLowerA(
           _In_ CHAR C
           )
{
    return (C >= 'A' && C <= 'Z') ? (C + 0x20) : C;
}

/**
 * \brief Convert a wide-char character to lowercase
 * \param C Character to convert.
 * \return Uppercase character
 */
__forceinline
_Check_return_
WCHAR
NkToLowerW(
           _In_ WCHAR C
           )
{
    return (C >= L'A' && C <= L'Z') ? (C + 0x20) : C;
}

/**
 * \brief Compare two characters
 * \param c1 First character
 * \param c2 Second character
 * \param CaseInSensitive Is case insensitive?
 * \return LONG
 * \retval 0 c1 equals to c2
 * \retval 1 c1 greater than c2
 * \retval -1 c1 less than c2
 */
__forceinline
_Check_return_
LONG
NkCompareA(
           _In_ CHAR c1,
           _In_ CHAR c2,
           _In_ BOOLEAN CaseInSensitive
           )
{
    if (CaseInSensitive) {
        c1 = NkToUpperA(c1);
        c2 = NkToUpperA(c2);
    }
    return (c1 == c2) ? 0 : ((c1 > c2) ? 1 : -1);
}

/**
 * \brief Check if two characters are euqal
 * \param c1 First character
 * \param c2 Second character
 * \param CaseInSensitive Is case insensitive?
 * \return BOOLEAN
 */
__forceinline
_Check_return_
BOOLEAN
NkEqualA(
         _In_ CHAR c1,
         _In_ CHAR c2,
         _In_ BOOLEAN CaseInSensitive
         )
{
    if (CaseInSensitive) {
        c1 = NkToUpperA(c1);
        c2 = NkToUpperA(c2);
    }
    return (c1 == c2) ? TRUE : FALSE;
}

/**
 * \brief Compare two characters
 * \param c1 First character
 * \param c2 Second character
 * \param CaseInSensitive Is case insensitive?
 * \return LONG
 * \retval 0 c1 equals to c2
 * \retval 1 c1 greater than c2
 * \retval -1 c1 less than c2
 */
__forceinline
_Check_return_
LONG
NkCompareW(
           _In_ WCHAR c1,
           _In_ WCHAR c2,
           _In_ BOOLEAN CaseInSensitive
           )
{
    if (CaseInSensitive) {
        c1 = NkToUpperW(c1);
        c2 = NkToUpperW(c2);
    }
    return (c1 == c2) ? 0 : ((c1 > c2) ? 1 : -1);
}

/**
 * \brief Check if two characters are euqal
 * \param c1 First character
 * \param c2 Second character
 * \param CaseInSensitive Is case insensitive?
 * \return BOOLEAN
 */
__forceinline
_Check_return_
BOOLEAN
NkEqualW(
         _In_ WCHAR c1,
         _In_ WCHAR c2,
         _In_ BOOLEAN CaseInSensitive
         )
{
    if (CaseInSensitive) {
        c1 = NkToUpperW(c1);
        c2 = NkToUpperW(c2);
    }
    return (c1 == c2) ? TRUE : FALSE;
}

/**
 * \brief Check if a characters is alphabet
 * \param C Character to check
 * \return BOOLEAN
 */
__forceinline
_Check_return_
BOOLEAN
NkIsAlphabetA(
              _In_ CHAR C
              )
{
    C = NkToUpperA(C);
    return (C >= 'A' && C <= 'Z') ? TRUE : FALSE;
}

/**
 * \brief Check if a characters is alphabet
 * \param C Character to check
 * \return BOOLEAN
 */
__forceinline
_Check_return_
BOOLEAN
NkIsAlphabetW(
              _In_ WCHAR C
              )
{
    C = NkToUpperW(C);
    return (C >= L'A' && C <= L'Z') ? TRUE : FALSE;
}

/**
 * \brief Check if a characters is number
 * \param C Character to check
 * \return BOOLEAN
 */
__forceinline
_Check_return_
BOOLEAN
NkIsNumberA(
            _In_ CHAR C
            )
{
    return (C >= '0' && C <= '9') ? TRUE : FALSE;
}

/**
 * \brief Check if a characters is number
 * \param C Character to check
 * \return BOOLEAN
 */
__forceinline
_Check_return_
BOOLEAN
NkIsNumberW(
            _In_ WCHAR C
            )
{
    return (C >= L'0' && C <= L'9') ? TRUE : FALSE;
}

/**
 * \brief Check if a characters is hex
 * \param C Character to check
 * \return BOOLEAN
 */
__forceinline
_Check_return_
BOOLEAN
NkIsHexA(
         _In_ CHAR C
         )
{
    C = NkToUpperA(C);
    return ((C >= '0' && C <= '9') || (C >= 'A' && C <= 'F')) ? TRUE : FALSE;
}

/**
 * \brief Check if a characters is hex
 * \param C Character to check
 * \return BOOLEAN
 */
__forceinline
_Check_return_
BOOLEAN
NkIsHexW(
         _In_ WCHAR C
         )
{
    C = NkToUpperW(C);
    return ((C >= L'0' && C <= L'9') || (C >= L'A' && C <= L'F')) ? TRUE : FALSE;
}

/**
 * \brief Check if a string is dos path
 * \param Path Path to check
 * \return BOOLEAN
 */
__forceinline
_Check_return_
BOOLEAN
NkIsDosPathA(
             _In_ const CHAR* Path
             )
{
    return (NkIsAlphabetA(Path[0]) && ':'==Path[1] && '\\'==Path[2]);
}

/**
 * \brief Check if a string is dos path
 * \param Path Path to check
 * \return BOOLEAN
 */
__forceinline
_Check_return_
BOOLEAN
NkIsDosPathW(
             _In_ const WCHAR* Path
             )
{
    return (NkIsAlphabetW(Path[0]) && L':' == Path[1] && L'\\' == Path[2]);
}

/**
 * \brief Check if a string is dos path
 * \param Path Path to check
 * \return BOOLEAN
 */
__forceinline
_Check_return_
BOOLEAN
NkIsUnicodeDosPath(
                   _In_ PCUNICODE_STRING Path
                   )
{
    return (Path->Length >= 6 && NkIsAlphabetW(Path->Buffer[0]) && L':' == Path->Buffer[1] && L'\\' == Path->Buffer[2]);
}

/**
 * \brief Check if a string is global dos path
 * \param Path Path to check
 * \return BOOLEAN
 */
__forceinline
_Check_return_
BOOLEAN
NkIsGlobalDosPathA(
                   _In_ const CHAR* Path
                   )
{
    return ('\\' == Path[0]
            && '?' == Path[1]
            && '?' == Path[2]
            && '\\' == Path[3]
            && NkIsAlphabetA(Path[4])
            && ':' == Path[5]
            && '\\' == Path[6]);
}

/**
 * \brief Check if a string is global dos path
 * \param Path Path to check
 * \return BOOLEAN
 */
__forceinline
_Check_return_
BOOLEAN
NkIsGlobalDosPathW(
                   _In_ const WCHAR* Path
                   )
{
    return (L'\\' == Path[0]
            && L'?' == Path[1]
            && L'?' == Path[2]
            && L'\\' == Path[3]
            && NkIsAlphabetW(Path[4])
            && L':' == Path[5]
            && L'\\' == Path[6]);
}

/**
 * \brief Check if a string is global dos path
 * \param Path Path to check
 * \return BOOLEAN
 */
__forceinline
_Check_return_
BOOLEAN
NkIsUnicodeGlobalDosPath(
                         _In_ PCUNICODE_STRING Path
                         )
{
    return (Path->Length >= 14
            && L'\\' == Path->Buffer[0]
            && L'?' == Path->Buffer[1]
            && L'?' == Path->Buffer[2]
            && L'\\' == Path->Buffer[3]
            && NkIsAlphabetW(Path->Buffer[4])
            && L':' == Path->Buffer[5]
            && L'\\' == Path->Buffer[6]);
}

__forceinline
VOID
NkParseFilePath(
                _In_ UNICODE_STRING FilePath,
                _Out_ PUNICODE_STRING ParentDir,
                _Out_ PUNICODE_STRING FileName
                )
{
    USHORT i = 0;
    USHORT Size = FilePath.Length / sizeof(WCHAR);
    RtlZeroMemory(ParentDir, sizeof(UNICODE_STRING));
    RtlZeroMemory(FileName, sizeof(UNICODE_STRING));
    if (0 == Size) {
        ASSERT(FALSE);
        return;
    }

    RtlCopyMemory(FileName, &FilePath, sizeof(UNICODE_STRING));
    for (i = 0; i < Size; i++) {
        if (FilePath.Buffer[Size - 1 - i] == L'\\') {
            ParentDir->Buffer = FilePath.Buffer;
            ParentDir->Length = FilePath.Length - (i * sizeof(WCHAR));
            ParentDir->MaximumLength = ParentDir->Length;

            FileName->Length = i * sizeof(WCHAR);
            FileName->MaximumLength = FileName->Length;
            FileName->Buffer = (0 == i) ? NULL : (&FilePath.Buffer[Size - i]);
            break;
        }
    }
}

__forceinline
VOID
NkParseFileName(
                _In_ UNICODE_STRING FileName,
                _Out_ PUNICODE_STRING NamePart,
                _Out_ PUNICODE_STRING Extension
                )
{
    USHORT i = 0;
    USHORT Size = FileName.Length / sizeof(WCHAR);
    RtlZeroMemory(NamePart, sizeof(UNICODE_STRING));
    RtlZeroMemory(Extension, sizeof(UNICODE_STRING));
    if (0 == Size) {
        ASSERT(FALSE);
        return;
    }

    RtlCopyMemory(NamePart, &FileName, sizeof(UNICODE_STRING));
    for (i = 0; i < Size; i++) {

        if (FileName.Buffer[Size - 1 - i] == L'.') {

            Extension->Buffer = &(FileName.Buffer[Size - 1 - i]);
            Extension->Length = sizeof(WCHAR) * (i + 1);
            Extension->MaximumLength = Extension->Length;
            NamePart->Length = (Size - i - 1) * sizeof(WCHAR);
            NamePart->MaximumLength = NamePart->Length;
            if (0 == NamePart->Length) {
                NamePart->Buffer = NULL;
            }
            break;
        }
    }
}

/**
 * \brief Check if a UNICODE_STRING is empty
 * \param Str String to check
 * \return BOOLEAN
 */
__forceinline
_Check_return_
BOOLEAN
NkIsEmptyUnicodeString(
                       _In_ PCUNICODE_STRING Str
                       )
{
    return (0 == Str->Length) ? TRUE : FALSE;
}

/**
 * \brief Free UNICODE_STRING safely
 * \param Str String to free
 * \return N/A
 */
__forceinline
VOID
NkFreeUnicodeString(
                    _In_ PUNICODE_STRING Str
                    )
{
    if (NULL != Str->Buffer) {
        ExFreePool(Str->Buffer);
    }
    Str->Buffer = NULL;
    Str->Length = 0;
    Str->MaximumLength = 0;
}


/**@}*/ // Group End: nkdf-basic-string-inline


/**@}*/ // Group End: nkdf-basic-string


/**@}*/ // Group End: nkdf-basic


#endif  // #ifndef __NKDF_BASIC_STRING_H__