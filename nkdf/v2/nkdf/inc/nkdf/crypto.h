


#pragma once
#ifndef __NKDF_CRYPTO_H__
#define __NKDF_CRYPTO_H__



#define KEY_BITS_128        128
#define KEY_BITS_256        256


_Check_return_
NTSTATUS
NkCryptoInit(
    );

VOID
NkCryptoCleanup(
    );

_Check_return_
NTSTATUS
NkAesGenerateKey(
    _Out_ PHANDLE KeyHandle,
    _In_ ULONG BitsLength,
    _In_ ULONG CbcSize
    );

_Check_return_
NTSTATUS
NkAesImportKey(
    _Out_ PHANDLE KeyHandle,
    _In_ PUCHAR Key,
    _In_ ULONG Size,
    _In_ ULONG CbcSize
    );

VOID
NkAesDestroytKey(
    _In_ HANDLE KeyHandle
    );

_Check_return_
NTSTATUS
NkAesExportKey(
    _In_ HANDLE KeyHandle,
    _In_ PUCHAR Buffer,
    _In_ PULONG Size
    );

_Check_return_
NTSTATUS
NkAesEncrypt(
    HANDLE KeyHandle,
    PUCHAR Buffer,
    ULONG Size,
    ULONGLONG Ivec
    );

_Check_return_
NTSTATUS
NkAesDecrypt(
    _In_ HANDLE KeyHandle,
    _In_ PUCHAR Buffer,
    _In_ ULONG Size,
    _In_ ULONGLONG Ivec
    );

_Check_return_
NTSTATUS
NkRsaGenerateKey(
    _Out_ PHANDLE KeyHandle,
    _In_ ULONG BitsLength
    );

_Check_return_
NTSTATUS
NkRsaImportKey(
    _Out_ PHANDLE KeyHandle,
    _In_ PVOID KeyBlob,
    _In_ ULONG Size
    );

_Check_return_
NTSTATUS
NkRsaImportKeyEx(
    _Out_ PHANDLE KeyHandle,
    _In_ PVOID KeyBlob,
    _In_ ULONG Size,
    _In_ BOOLEAN PublicOnly
    );

VOID
NkRsaDestroyKey(
    _In_ HANDLE KeyHandle
    );

_Check_return_
NTSTATUS
NkRsaSign(
    _In_ HANDLE KeyHandle,
    _In_ const UCHAR* Data,
    _In_ ULONG DataSize,
    _In_ PUCHAR Signature,
    _In_ PULONG SignatureSize
    );

_Check_return_
NTSTATUS
NkRsaVerify(
    _In_ HANDLE KeyHandle,
    _In_ const UCHAR* Data,
    _In_ ULONG DataSize,
    _In_ const UCHAR* Signature,
    _In_ ULONG SignatureSize
    );

_Check_return_
NTSTATUS
NkRsaEncrypt(
    _In_ HANDLE KeyHandle,
    _In_ const UCHAR* Data,
    _In_ ULONG DataSize,
    _In_ PUCHAR Cipher,
    _In_ PULONG CipherSize
    );

_Check_return_
NTSTATUS
NkRsaDecrypt(
    _In_ HANDLE KeyHandle,
    _In_ const UCHAR* Cipher,
    _In_ ULONG CipherSize,
    _In_ PUCHAR Data,
    _In_ PULONG DataSize
    );

_Check_return_
NTSTATUS
NkMd5(
    _In_ const UCHAR* Data,
    _In_ ULONG DataSize,
    _Out_writes_bytes_(16) PUCHAR HashData
    );

_Check_return_
NTSTATUS
NkSha1(
    _In_ const UCHAR* Data,
    _In_ ULONG DataSize,
    _Out_writes_bytes_(20) PUCHAR HashData
    );

_Check_return_
NTSTATUS
NkSha2(
    _In_ const UCHAR* Data,
    _In_ ULONG DataSize,
    _Out_writes_bytes_(32) PUCHAR HashData
    );

_Check_return_
ULONG
NkCrc32(
    _In_ ULONG InitialCrc,
    _In_reads_(Size) const VOID* Data,
    _In_ ULONG Size
    );

_Check_return_
ULONGLONG
NkCrc64(
    _In_ ULONGLONG InitialCrc,
    _In_reads_(Size) const VOID* Data,
    _In_ ULONG Size
    );


#endif