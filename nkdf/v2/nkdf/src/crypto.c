

#include "stdafx.h"

#include <bcrypt.h>

#include <nkdf\crypto.h>


typedef struct _PROVIDER {
    const LPCWSTR Algorithm;
    BCRYPT_ALG_HANDLE Handle;
} PROVIDER;

typedef enum PROVIDER_ID {
    PROV_AES = 0,
    PROV_RSA,
    PROV_MD5,
    PROV_SHA1,
    PROV_SHA2,
    PROV_MAX
} PROVIDER_ID;

static PROVIDER Providers[] = {
    { BCRYPT_AES_ALGORITHM, NULL },
    { BCRYPT_RSA_ALGORITHM, NULL },
    { BCRYPT_MD5_ALGORITHM, NULL },
    { BCRYPT_SHA1_ALGORITHM, NULL },
    { BCRYPT_SHA256_ALGORITHM, NULL },
    { NULL, NULL }
};

typedef struct _AES_KEY {
    BCRYPT_KEY_HANDLE Handle;
    ULONG   BitsLength;
    ULONG   CbcLength;
} AES_KEY, *PAES_KEY;




NTSTATUS
NkCryptoInit(
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    try {

        int i = 0;

        while (Providers[i++].Algorithm != NULL) {

            Status = BCryptOpenAlgorithmProvider(&Providers[i].Handle, Providers[i].Algorithm, MS_PRIMITIVE_PROVIDER, (PROV_RSA == i) ? 0 : BCRYPT_PROV_DISPATCH);
            if (!NT_SUCCESS(Status)) {
                try_return(Status);
            }
            if (PROV_AES == i) {
                Status = BCryptSetProperty(Providers[i].Handle, BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0);
            }

            // Move to next
            ++i;
        }

    try_exit:
        NOTHING;
    }
    finally {
        if (!NT_SUCCESS(Status)) {
            NkCryptoCleanup();
        }
    }

    return Status;
}

VOID
NkCryptoCleanup(
    )
{
    int i = 0;

    while (Providers[i].Algorithm != NULL) {

        if (NULL != Providers[i].Handle) {
            (VOID)BCryptCloseAlgorithmProvider(Providers[i].Handle, 0);
            Providers[i].Handle = NULL;
        }

        // Move to next
        ++i;
    }
}

ULONG
NkCryptoGetIntProperty(
    BCRYPT_HANDLE Handle,
    LPCWSTR PropertyName
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Result = 0;
    ULONG ReturnedSize = 0;
    Status = BCryptGetProperty(Handle, PropertyName, (PUCHAR)&Result, sizeof(ULONG), &ReturnedSize, 0);
    if (!NT_SUCCESS(Status)) {
        Result = 0;
    }
    return Result;
}



NTSTATUS
NkAesGenerateKey(
    _Out_ PHANDLE KeyHandle,
    _In_ ULONG BitsLength,
    _In_ ULONG CbcLength
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    UCHAR    Key[32] = { 0 };
    
    PAGED_CODE();
    
    try {

        if (BitsLength != 128 && BitsLength != 256) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        RtlZeroMemory(Key, sizeof(Key));
        Status = BCryptGenRandom(NULL, Key, 32, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = NkAesImportKey(KeyHandle, Key, (BitsLength == 256) ? 32 : 16, CbcLength);

    try_exit: NOTHING;
    }
    finally {
        if (!NT_SUCCESS(Status)) {
            *KeyHandle = NULL;
        }
    }

    return Status;
}

NTSTATUS
NkAesImportKey(
    _Out_ PHANDLE KeyHandle,
    _In_ PUCHAR Key,
    _In_ ULONG Size,
    _In_ ULONG CbcLength
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PAES_KEY AesKey = NULL;
    PBCRYPT_KEY_DATA_BLOB_HEADER KeyBlob = NULL;
    
    PAGED_CODE();
    
    try {

        if (Size != 16 && Size != 32) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        if (NULL == Providers[PROV_AES].Handle) {
            try_return(Status = STATUS_INVALID_HANDLE);
        }

        AesKey = ExAllocatePoolWithTag(NonPagedPool, sizeof(AES_KEY), TAG_KEY);
        if (NULL == AesKey) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }
        RtlZeroMemory(AesKey, sizeof(AES_KEY));
        AesKey->BitsLength = (Size == 32) ? 256 : 128;
        AesKey->CbcLength  = CbcLength;

        KeyBlob = ExAllocatePoolWithTag(NonPagedPool, sizeof(BCRYPT_KEY_DATA_BLOB_HEADER) + Size, TAG_TMP);
        if (NULL == KeyBlob) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory(KeyBlob, sizeof(BCRYPT_KEY_DATA_BLOB_HEADER) + Size);
        KeyBlob->cbKeyData = Size;
        KeyBlob->dwVersion = BCRYPT_KEY_DATA_BLOB_VERSION1;
        KeyBlob->dwMagic = BCRYPT_KEY_DATA_BLOB_MAGIC;
        RtlCopyMemory(KeyBlob + 1, Key, Size);
        
        Status = BCryptImportKey(Providers[PROV_AES].Handle,
            NULL,
            BCRYPT_KEY_DATA_BLOB,
            &AesKey->Handle,
            NULL,
            0,
            (PUCHAR)KeyBlob,
            (ULONG)sizeof(BCRYPT_KEY_DATA_BLOB_HEADER) + Size,
            0);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

    try_exit: NOTHING;
    }
    finally {

        ExFreePool(KeyBlob);
        KeyBlob = NULL;

        if (!NT_SUCCESS(Status)) {
            if (NULL != AesKey) {
                NkAesDestroytKey((HANDLE)AesKey);
                AesKey = NULL;
            }
        }

        *KeyHandle = (HANDLE)AesKey;
    }

    return Status;
}

VOID
NkAesDestroytKey(
    _In_ HANDLE KeyHandle
    )
{
    if (NULL != KeyHandle) {

        PAES_KEY AesKey = (PAES_KEY)KeyHandle;
        if (NULL != AesKey->Handle) {
            BCryptDestroyKey(AesKey->Handle);
        }
        RtlZeroMemory(AesKey, sizeof(AES_KEY));
        ExFreePool(AesKey);
    }
}

NTSTATUS
NkAesExportKey(
    _In_ HANDLE KeyHandle,
    _In_ PUCHAR Buffer,
    _In_ PULONG Size
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(KeyHandle);
    UNREFERENCED_PARAMETER(Buffer);
    UNREFERENCED_PARAMETER(Size);


    PAGED_CODE();

    try {
    }
    finally {
    }

    return Status;
}

NTSTATUS
NkAesEncrypt(
    _In_ HANDLE KeyHandle,
    _In_ PUCHAR Buffer,
    _In_ ULONG Size,
    _In_ ULONGLONG Ivec
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    try {

        PAES_KEY AesKey = (PAES_KEY)KeyHandle;
        const ULONG KeyLength = (AesKey->BitsLength == 256) ? 16 : 32;

        if (NULL == AesKey) {
            try_return(Status = STATUS_INVALID_HANDLE);
        }

        if (!IS_ALIGNED(Ivec, AesKey->CbcLength)) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }
        if (!IS_ALIGNED(Size, KeyLength)) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        while (Size != 0) {

            const ULONG BytesToEncrypt = (Size > AesKey->CbcLength) ? AesKey->CbcLength : Size;
            const ULONGLONG ReverseIvec = ~Ivec;
            ULONG BytesEncrypted = 0;
            UCHAR IV[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

            RtlCopyMemory(IV, &Ivec, sizeof(Ivec));
            RtlCopyMemory(IV + 8, &ReverseIvec, sizeof(ReverseIvec));

            Status = BCryptEncrypt(AesKey->Handle,
                Buffer,
                BytesToEncrypt,
                NULL,
                IV,
                16,
                Buffer,
                BytesToEncrypt,
                &BytesEncrypted,
                0);
            if (!NT_SUCCESS(Status)) {
                try_return(Status);
            }

            // Update position
            ASSERT(BytesToEncrypt == BytesEncrypted);
            Buffer += BytesEncrypted;
            Ivec += BytesEncrypted;
            Size -= BytesEncrypted;
        }

    try_exit: NOTHING;
    }
    finally {
        ; // NOTHING
    }

    return Status;
}

NTSTATUS
NkAesDecrypt(
    _In_ HANDLE  KeyHandle,
    _In_ PUCHAR  Buffer,
    _In_ ULONG   Size,
    _In_ ULONGLONG Ivec
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    try {

        PAES_KEY AesKey = (PAES_KEY)KeyHandle;
        const ULONG KeyLength = (AesKey->BitsLength == 256) ? 16 : 32;

        if (NULL == AesKey) {
            try_return(Status = STATUS_INVALID_HANDLE);
        }

        if (!IS_ALIGNED(Ivec, AesKey->CbcLength)) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }
        if (!IS_ALIGNED(Size, KeyLength)) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        while (Size != 0) {

            const ULONG BytesToDecrypt = (Size > AesKey->CbcLength) ? AesKey->CbcLength : Size;
            const ULONGLONG ReverseIvec = ~Ivec;
            ULONG BytesDecrypted = 0;
            UCHAR IV[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

            RtlCopyMemory(IV, &Ivec, sizeof(Ivec));
            RtlCopyMemory(IV + 8, &ReverseIvec, sizeof(ReverseIvec));

            Status = BCryptDecrypt(AesKey->Handle,
                Buffer,
                BytesToDecrypt,
                NULL,
                IV,
                16,
                Buffer,
                BytesToDecrypt,
                &BytesDecrypted,
                0);
            if (!NT_SUCCESS(Status)) {
                try_return(Status);
            }

            // Update position
            ASSERT(BytesToDecrypt == BytesDecrypted);
            Buffer += BytesDecrypted;
            Ivec += BytesDecrypted;
            Size -= BytesDecrypted;
        }

    try_exit: NOTHING;
    }
    finally {
        ; // NOTHING
    }

    return Status;
}


typedef struct _RSA_KEY {
    BCRYPT_KEY_HANDLE Handle;
    ULONG BitsLength;
    BOOLEAN PublicOnly;
} RSA_KEY, *PRSA_KEY;


_Check_return_
NTSTATUS
NkRsaGenerateKey(
    _Out_ PHANDLE KeyHandle,
    _In_ ULONG BitsLength
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PRSA_KEY RsaKey = NULL;

    try {

        RsaKey = ExAllocatePoolWithTag(NonPagedPool, sizeof(RSA_KEY), TAG_KEY);
        if (NULL == RsaKey) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }
        RtlZeroMemory(RsaKey, sizeof(RSA_KEY));


        Status = BCryptGenerateKeyPair(Providers[PROV_RSA].Handle, &RsaKey->Handle, BitsLength, 0);
        if (0 != Status) {
            try_return(Status);
        }

        RsaKey->BitsLength = BitsLength;
        RsaKey->PublicOnly = FALSE;

    try_exit: NOTHING;
    }
    finally {
        if (!NT_SUCCESS(Status)) {
            if (NULL != RsaKey) {
                NkRsaDestroyKey((HANDLE)RsaKey);
            }
        }
    }

    *KeyHandle = RsaKey;
    return Status;
}

_Check_return_
NTSTATUS
NkRsaImportKey(
    _Out_ PHANDLE KeyHandle,
    _In_ PVOID KeyBlob,
    _In_ ULONG Size
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PRSA_KEY RsaKey = NULL;

    try {
        
        RsaKey = ExAllocatePoolWithTag(NonPagedPool, sizeof(RSA_KEY), TAG_KEY);
        if (NULL == RsaKey) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }
        RtlZeroMemory(RsaKey, sizeof(RSA_KEY));

        // Try private first
        Status = BCryptImportKeyPair(Providers[PROV_RSA].Handle, NULL, BCRYPT_RSAPRIVATE_BLOB, &RsaKey->Handle, (PUCHAR)KeyBlob, Size, 0);
        if (NT_SUCCESS(Status)) {
            // Private key exists
            RsaKey->PublicOnly = TRUE;
        }
        else {
            // Otherwise try public key
            Status = BCryptImportKeyPair(Providers[PROV_RSA].Handle, NULL, BCRYPT_RSAPUBLIC_BLOB, &RsaKey->Handle, (PUCHAR)KeyBlob, Size, 0);
            if (!NT_SUCCESS(Status)) {
                try_return(Status);
            }
            RsaKey->PublicOnly = FALSE;
        }
        RsaKey->BitsLength = NkCryptoGetIntProperty(RsaKey->Handle, BCRYPT_KEY_STRENGTH);


    try_exit: NOTHING;
    }
    finally {
        if (!NT_SUCCESS(Status)) {
            if (NULL != RsaKey) {
                NkRsaDestroyKey((HANDLE)RsaKey);
                RsaKey = NULL;
            }
        }
    }

    *KeyHandle = RsaKey;
    return Status;
}

_Check_return_
NTSTATUS
NkRsaImportKeyEx(
    _Out_ PHANDLE KeyHandle,
    _In_ PVOID KeyBlob,
    _In_ ULONG Size,
    _In_ BOOLEAN PublicOnly
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PRSA_KEY RsaKey = NULL;

    try {

        RsaKey = ExAllocatePoolWithTag(NonPagedPool, sizeof(RSA_KEY), TAG_KEY);
        if (NULL == RsaKey) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }
        RtlZeroMemory(RsaKey, sizeof(RSA_KEY));

        Status = BCryptImportKeyPair(Providers[PROV_RSA].Handle, NULL, PublicOnly ? BCRYPT_RSAPUBLIC_BLOB : BCRYPT_RSAPRIVATE_BLOB, &RsaKey->Handle, (PUCHAR)KeyBlob, Size, 0);
        if (0 != Status) {
            try_return(Status);
        }

        RsaKey->BitsLength = NkCryptoGetIntProperty(RsaKey->Handle, BCRYPT_KEY_STRENGTH);
        RsaKey->PublicOnly = PublicOnly;

    try_exit: NOTHING;
    }
    finally {
        if (!NT_SUCCESS(Status)) {
            if (NULL != RsaKey) {
                NkRsaDestroyKey((HANDLE)RsaKey);
                RsaKey = NULL;
            }
        }
    }

    *KeyHandle = RsaKey;
    return Status;
}

VOID
NkRsaDestroyKey(
    _In_ HANDLE KeyHandle
    )
{
    if (NULL != KeyHandle) {
        PRSA_KEY RsaKey = (PRSA_KEY)KeyHandle;
        if (RsaKey->Handle != NULL) {
            BCryptDestroyKey(RsaKey->Handle);
            RsaKey->Handle = NULL;
        }
        ExFreePool(RsaKey);
    }
}

_Check_return_
NTSTATUS
NkRsaSign(
    _In_ HANDLE KeyHandle,
    _In_ const UCHAR* Data,
    _In_ ULONG DataSize,
    _Inout_opt_ PUCHAR Signature,
    _Inout_ PULONG SignatureSize
    )
{
    NTSTATUS Status = STATUS_SUCCESS;


    PAGED_CODE();

    if (NULL == KeyHandle) {
        return STATUS_INVALID_HANDLE;
    }
    if (NULL == SignatureSize || NULL == Data || 0 == DataSize) {
        return STATUS_INVALID_PARAMETER;
    }

    try {

        const ULONG OutputSize = *SignatureSize;
        PRSA_KEY RsaKey = (PRSA_KEY)KeyHandle;
        UCHAR HashData[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        BCRYPT_PKCS1_PADDING_INFO Pkcs1PaddingInfo = { BCRYPT_SHA256_ALGORITHM };
        ULONG RequiredSize = 0;


        Status = NkSha2(Data, DataSize, HashData);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = BCryptSignHash(RsaKey->Handle, &Pkcs1PaddingInfo, HashData, 32, NULL, 0, &RequiredSize, BCRYPT_PAD_PKCS1);
        if (0 == RequiredSize) {
            try_return(Status);
        }

        if (NULL == Signature || 0 == OutputSize) {
            *SignatureSize = RequiredSize;
            try_return(Status = STATUS_SUCCESS);
        }

        if (OutputSize < RequiredSize) {
            *SignatureSize = RequiredSize;
            try_return(Status = STATUS_BUFFER_TOO_SMALL);
        }

        Status = BCryptSignHash(RsaKey->Handle, &Pkcs1PaddingInfo, HashData, 32, Signature, OutputSize, SignatureSize, BCRYPT_PAD_PKCS1);
        if (!NT_SUCCESS(Status)) {
            *SignatureSize = 0;
            try_return(Status);
        }
        
    try_exit: NOTHING;
    }
    finally {
        ; // Nothing
    }

    return Status;
}

_Check_return_
NTSTATUS
NkRsaVerify(
    _In_ HANDLE KeyHandle,
    _In_ const UCHAR* Data,
    _In_ ULONG DataSize,
    _In_ const UCHAR* Signature,
    _In_ ULONG SignatureSize
    )
{
    NTSTATUS Status = STATUS_SUCCESS;


    PAGED_CODE();

    if (NULL == KeyHandle) {
        return STATUS_INVALID_HANDLE;
    }
    if (0 == DataSize || NULL == Data) {
        return STATUS_INVALID_PARAMETER;
    }
    if (0 == SignatureSize || NULL == Signature) {
        return STATUS_INVALID_PARAMETER;
    }

    try {

        PRSA_KEY RsaKey = (PRSA_KEY)KeyHandle;
        UCHAR HashData[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        BCRYPT_PKCS1_PADDING_INFO Pkcs1PaddingInfo = { BCRYPT_SHA256_ALGORITHM };


        Status = NkSha2(Data, DataSize, HashData);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = BCryptVerifySignature(RsaKey->Handle, &Pkcs1PaddingInfo, (PUCHAR)HashData, 32, (PUCHAR)Signature, SignatureSize, BCRYPT_PAD_PKCS1);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

    try_exit: NOTHING;
    }
    finally {
        ; // Nothing
    }

    return Status;
}

_Check_return_
NTSTATUS
NkRsaEncrypt(
    _In_ HANDLE KeyHandle,
    _In_ const UCHAR* Data,
    _In_ ULONG DataSize,
    _Inout_opt_ PUCHAR Cipher,
    _Inout_ PULONG CipherSize
    )
{
    NTSTATUS Status = STATUS_SUCCESS;


    PAGED_CODE();

    if (NULL == KeyHandle) {
        return STATUS_INVALID_HANDLE;
    }
    if (NULL == CipherSize || 0 == DataSize || NULL == Data) {
        return STATUS_INVALID_PARAMETER;
    }


    try {

        const ULONG OutputSize = *CipherSize;
        PRSA_KEY RsaKey = (PRSA_KEY)KeyHandle;
        ULONG RequiredSize = 0;


        Status = BCryptEncrypt(RsaKey->Handle, (PUCHAR)Data, DataSize, NULL, NULL, 0, NULL, 0, &RequiredSize, BCRYPT_PAD_PKCS1);
        if (0 == RequiredSize) {
            try_return(Status);
        }

        if (NULL == Cipher || 0 == OutputSize) {
            *CipherSize = RequiredSize;
            try_return(Status = STATUS_SUCCESS);
        }

        if (OutputSize < RequiredSize) {
            *CipherSize = RequiredSize;
            try_return(Status = STATUS_BUFFER_TOO_SMALL);
        }

        Status = BCryptEncrypt(RsaKey->Handle, (PUCHAR)Data, DataSize, NULL, NULL, 0, Cipher, OutputSize, CipherSize, BCRYPT_PAD_PKCS1);
        if (!NT_SUCCESS(Status)) {
            *CipherSize = 0;
            try_return(Status);
        }

    try_exit: NOTHING;
    }
    finally {
        ; // Nothing
    }

    return Status;
}

_Check_return_
NTSTATUS
NkRsaDecrypt(
    _In_ HANDLE KeyHandle,
    _In_ const UCHAR* Cipher,
    _In_ ULONG CipherSize,
    _Inout_opt_ PUCHAR Data,
    _Inout_ PULONG DataSize
    )
{
    NTSTATUS Status = STATUS_SUCCESS;


    PAGED_CODE();

    if (NULL == KeyHandle) {
        return STATUS_INVALID_HANDLE;
    }
    if (NULL == DataSize || 0 == CipherSize || NULL == Cipher) {
        return STATUS_INVALID_PARAMETER;
    }

    try {

        const ULONG OutputSize = *DataSize;
        PRSA_KEY RsaKey = (PRSA_KEY)KeyHandle;
        ULONG RequiredSize = 0;



        Status = BCryptDecrypt(RsaKey->Handle, (PUCHAR)Cipher, CipherSize, NULL, NULL, 0, NULL, 0, &RequiredSize, BCRYPT_PAD_PKCS1);
        if (0 == RequiredSize) {
            try_return(Status);
        }

        if (NULL == Data || 0 == OutputSize) {
            *DataSize = RequiredSize;
            try_return(Status = STATUS_SUCCESS);
        }

        if (OutputSize < RequiredSize) {
            *DataSize = RequiredSize;
            try_return(Status = STATUS_BUFFER_TOO_SMALL);
        }

        Status = BCryptDecrypt(RsaKey->Handle, (PUCHAR)Cipher, CipherSize, NULL, NULL, 0, Data, OutputSize, DataSize, BCRYPT_PAD_PKCS1);
        if (!NT_SUCCESS(Status)) {
            *DataSize = 0;
            try_return(Status);
        }

    try_exit: NOTHING;
    }
    finally {
        ; // Nothing
    }

    return Status;
}

_Check_return_
NTSTATUS
NkMd5(
    _In_ const UCHAR* Data,
    _In_ ULONG DataSize,
    _Out_writes_bytes_(16) PUCHAR HashData
    )
{
    static ULONG HashObjectLength = 0;
    static ULONG HashLength = 0;

    NTSTATUS Status = STATUS_SUCCESS;
    BCRYPT_HASH_HANDLE HashHandle = NULL;
    PUCHAR HashObject = NULL;

    if (0 == HashObjectLength) {
        HashObjectLength = NkCryptoGetIntProperty(Providers[PROV_MD5].Handle, BCRYPT_OBJECT_LENGTH);
    }
    if (0 == HashLength) {
        HashLength = NkCryptoGetIntProperty(Providers[PROV_MD5].Handle, BCRYPT_HASH_LENGTH);
    }

    try {

        ASSERT(16 == HashLength);

        HashObject = ExAllocatePoolWithTag(NonPagedPool, HashObjectLength, TAG_TMP);
        if (NULL == HashObject) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }
        RtlZeroMemory(HashObject, HashObjectLength);

        Status = BCryptCreateHash(Providers[PROV_MD5].Handle,
            &HashHandle,
            HashObject,
            HashObjectLength,
            NULL,
            0,
            0);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = BCryptHashData(HashHandle,
            (PUCHAR)Data,
            DataSize,
            0);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        RtlZeroMemory(HashData, 20);
        Status = BCryptFinishHash(HashHandle,
            HashData,
            HashLength,
            0);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

    try_exit: NOTHING;
    }
    finally {

        if (NULL != HashHandle) {
            (VOID)BCryptDestroyHash(HashHandle);
            HashHandle = NULL;
        }

        if (NULL != HashObject) {
            ExFreePool(HashObject);
            HashObject = NULL;
        }
    }

    return Status;
}

_Check_return_
NTSTATUS
NkSha1(
    _In_ const UCHAR* Data,
    _In_ ULONG DataSize,
    _Out_writes_bytes_(20) PUCHAR HashData
    )
{
    static ULONG HashObjectLength = 0;
    static ULONG HashLength = 0;

    NTSTATUS Status = STATUS_SUCCESS;
    BCRYPT_HASH_HANDLE HashHandle = NULL;
    PUCHAR HashObject = NULL;

    if (0 == HashObjectLength) {
        HashObjectLength = NkCryptoGetIntProperty(Providers[PROV_SHA1].Handle, BCRYPT_OBJECT_LENGTH);
    }
    if (0 == HashLength) {
        HashLength = NkCryptoGetIntProperty(Providers[PROV_SHA1].Handle, BCRYPT_HASH_LENGTH);
    }

    try {

        ASSERT(20 == HashLength);

        HashObject = ExAllocatePoolWithTag(NonPagedPool, HashObjectLength, TAG_TMP);
        if (NULL == HashObject) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }
        RtlZeroMemory(HashObject, HashObjectLength);

        Status = BCryptCreateHash(Providers[PROV_SHA1].Handle,
            &HashHandle,
            HashObject,
            HashObjectLength,
            NULL,
            0,
            0);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = BCryptHashData(HashHandle,
            (PUCHAR)Data,
            DataSize,
            0);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        RtlZeroMemory(HashData, 20);
        Status = BCryptFinishHash(HashHandle,
            HashData,
            HashLength,
            0);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

    try_exit: NOTHING;
    }
    finally {

        if (NULL != HashHandle) {
            (VOID)BCryptDestroyHash(HashHandle);
            HashHandle = NULL;
        }

        if (NULL != HashObject) {
            ExFreePool(HashObject);
            HashObject = NULL;
        }
    }

    return Status;
}

_Check_return_
NTSTATUS
NkSha2(
    _In_ const UCHAR* Data,
    _In_ ULONG DataSize,
    _Out_writes_bytes_(32) PUCHAR HashData
    )
{
    static ULONG HashObjectLength = 0;
    static ULONG HashLength = 0;

    NTSTATUS Status = STATUS_SUCCESS;
    BCRYPT_HASH_HANDLE HashHandle = NULL;
    PUCHAR HashObject = NULL;

    if (0 == HashObjectLength) {
        HashObjectLength = NkCryptoGetIntProperty(Providers[PROV_SHA2].Handle, BCRYPT_OBJECT_LENGTH);
    }
    if (0 == HashLength) {
        HashLength = NkCryptoGetIntProperty(Providers[PROV_SHA2].Handle, BCRYPT_HASH_LENGTH);
    }

    try {

        ASSERT(32 == HashLength);

        HashObject = ExAllocatePoolWithTag(NonPagedPool, HashObjectLength, TAG_TMP);
        if (NULL == HashObject) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }
        RtlZeroMemory(HashObject, HashObjectLength);

        Status = BCryptCreateHash(Providers[PROV_SHA2].Handle,
            &HashHandle,
            HashObject,
            HashObjectLength,
            NULL,
            0,
            0);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = BCryptHashData(HashHandle,
            (PUCHAR)Data,
            DataSize,
            0);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        RtlZeroMemory(HashData, 20);
        Status = BCryptFinishHash(HashHandle,
            HashData,
            HashLength,
            0);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

    try_exit: NOTHING;
    }
    finally {

        if (NULL != HashHandle) {
            (VOID)BCryptDestroyHash(HashHandle);
            HashHandle = NULL;
        }

        if (NULL != HashObject) {
            ExFreePool(HashObject);
            HashObject = NULL;
        }
    }

    return Status;
}


static const unsigned long crc32_tab[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};


static const unsigned __int64 crc64_tab[256] = {
    0x0000000000000000ULL, 0x42F0E1EBA9EA3693ULL,
    0x85E1C3D753D46D26ULL, 0xC711223CFA3E5BB5ULL,
    0x493366450E42ECDFULL, 0x0BC387AEA7A8DA4CULL,
    0xCCD2A5925D9681F9ULL, 0x8E224479F47CB76AULL,
    0x9266CC8A1C85D9BEULL, 0xD0962D61B56FEF2DULL,
    0x17870F5D4F51B498ULL, 0x5577EEB6E6BB820BULL,
    0xDB55AACF12C73561ULL, 0x99A54B24BB2D03F2ULL,
    0x5EB4691841135847ULL, 0x1C4488F3E8F96ED4ULL,
    0x663D78FF90E185EFULL, 0x24CD9914390BB37CULL,
    0xE3DCBB28C335E8C9ULL, 0xA12C5AC36ADFDE5AULL,
    0x2F0E1EBA9EA36930ULL, 0x6DFEFF5137495FA3ULL,
    0xAAEFDD6DCD770416ULL, 0xE81F3C86649D3285ULL,
    0xF45BB4758C645C51ULL, 0xB6AB559E258E6AC2ULL,
    0x71BA77A2DFB03177ULL, 0x334A9649765A07E4ULL,
    0xBD68D2308226B08EULL, 0xFF9833DB2BCC861DULL,
    0x388911E7D1F2DDA8ULL, 0x7A79F00C7818EB3BULL,
    0xCC7AF1FF21C30BDEULL, 0x8E8A101488293D4DULL,
    0x499B3228721766F8ULL, 0x0B6BD3C3DBFD506BULL,
    0x854997BA2F81E701ULL, 0xC7B97651866BD192ULL,
    0x00A8546D7C558A27ULL, 0x4258B586D5BFBCB4ULL,
    0x5E1C3D753D46D260ULL, 0x1CECDC9E94ACE4F3ULL,
    0xDBFDFEA26E92BF46ULL, 0x990D1F49C77889D5ULL,
    0x172F5B3033043EBFULL, 0x55DFBADB9AEE082CULL,
    0x92CE98E760D05399ULL, 0xD03E790CC93A650AULL,
    0xAA478900B1228E31ULL, 0xE8B768EB18C8B8A2ULL,
    0x2FA64AD7E2F6E317ULL, 0x6D56AB3C4B1CD584ULL,
    0xE374EF45BF6062EEULL, 0xA1840EAE168A547DULL,
    0x66952C92ECB40FC8ULL, 0x2465CD79455E395BULL,
    0x3821458AADA7578FULL, 0x7AD1A461044D611CULL,
    0xBDC0865DFE733AA9ULL, 0xFF3067B657990C3AULL,
    0x711223CFA3E5BB50ULL, 0x33E2C2240A0F8DC3ULL,
    0xF4F3E018F031D676ULL, 0xB60301F359DBE0E5ULL,
    0xDA050215EA6C212FULL, 0x98F5E3FE438617BCULL,
    0x5FE4C1C2B9B84C09ULL, 0x1D14202910527A9AULL,
    0x93366450E42ECDF0ULL, 0xD1C685BB4DC4FB63ULL,
    0x16D7A787B7FAA0D6ULL, 0x5427466C1E109645ULL,
    0x4863CE9FF6E9F891ULL, 0x0A932F745F03CE02ULL,
    0xCD820D48A53D95B7ULL, 0x8F72ECA30CD7A324ULL,
    0x0150A8DAF8AB144EULL, 0x43A04931514122DDULL,
    0x84B16B0DAB7F7968ULL, 0xC6418AE602954FFBULL,
    0xBC387AEA7A8DA4C0ULL, 0xFEC89B01D3679253ULL,
    0x39D9B93D2959C9E6ULL, 0x7B2958D680B3FF75ULL,
    0xF50B1CAF74CF481FULL, 0xB7FBFD44DD257E8CULL,
    0x70EADF78271B2539ULL, 0x321A3E938EF113AAULL,
    0x2E5EB66066087D7EULL, 0x6CAE578BCFE24BEDULL,
    0xABBF75B735DC1058ULL, 0xE94F945C9C3626CBULL,
    0x676DD025684A91A1ULL, 0x259D31CEC1A0A732ULL,
    0xE28C13F23B9EFC87ULL, 0xA07CF2199274CA14ULL,
    0x167FF3EACBAF2AF1ULL, 0x548F120162451C62ULL,
    0x939E303D987B47D7ULL, 0xD16ED1D631917144ULL,
    0x5F4C95AFC5EDC62EULL, 0x1DBC74446C07F0BDULL,
    0xDAAD56789639AB08ULL, 0x985DB7933FD39D9BULL,
    0x84193F60D72AF34FULL, 0xC6E9DE8B7EC0C5DCULL,
    0x01F8FCB784FE9E69ULL, 0x43081D5C2D14A8FAULL,
    0xCD2A5925D9681F90ULL, 0x8FDAB8CE70822903ULL,
    0x48CB9AF28ABC72B6ULL, 0x0A3B7B1923564425ULL,
    0x70428B155B4EAF1EULL, 0x32B26AFEF2A4998DULL,
    0xF5A348C2089AC238ULL, 0xB753A929A170F4ABULL,
    0x3971ED50550C43C1ULL, 0x7B810CBBFCE67552ULL,
    0xBC902E8706D82EE7ULL, 0xFE60CF6CAF321874ULL,
    0xE224479F47CB76A0ULL, 0xA0D4A674EE214033ULL,
    0x67C58448141F1B86ULL, 0x253565A3BDF52D15ULL,
    0xAB1721DA49899A7FULL, 0xE9E7C031E063ACECULL,
    0x2EF6E20D1A5DF759ULL, 0x6C0603E6B3B7C1CAULL,
    0xF6FAE5C07D3274CDULL, 0xB40A042BD4D8425EULL,
    0x731B26172EE619EBULL, 0x31EBC7FC870C2F78ULL,
    0xBFC9838573709812ULL, 0xFD39626EDA9AAE81ULL,
    0x3A28405220A4F534ULL, 0x78D8A1B9894EC3A7ULL,
    0x649C294A61B7AD73ULL, 0x266CC8A1C85D9BE0ULL,
    0xE17DEA9D3263C055ULL, 0xA38D0B769B89F6C6ULL,
    0x2DAF4F0F6FF541ACULL, 0x6F5FAEE4C61F773FULL,
    0xA84E8CD83C212C8AULL, 0xEABE6D3395CB1A19ULL,
    0x90C79D3FEDD3F122ULL, 0xD2377CD44439C7B1ULL,
    0x15265EE8BE079C04ULL, 0x57D6BF0317EDAA97ULL,
    0xD9F4FB7AE3911DFDULL, 0x9B041A914A7B2B6EULL,
    0x5C1538ADB04570DBULL, 0x1EE5D94619AF4648ULL,
    0x02A151B5F156289CULL, 0x4051B05E58BC1E0FULL,
    0x87409262A28245BAULL, 0xC5B073890B687329ULL,
    0x4B9237F0FF14C443ULL, 0x0962D61B56FEF2D0ULL,
    0xCE73F427ACC0A965ULL, 0x8C8315CC052A9FF6ULL,
    0x3A80143F5CF17F13ULL, 0x7870F5D4F51B4980ULL,
    0xBF61D7E80F251235ULL, 0xFD913603A6CF24A6ULL,
    0x73B3727A52B393CCULL, 0x31439391FB59A55FULL,
    0xF652B1AD0167FEEAULL, 0xB4A25046A88DC879ULL,
    0xA8E6D8B54074A6ADULL, 0xEA16395EE99E903EULL,
    0x2D071B6213A0CB8BULL, 0x6FF7FA89BA4AFD18ULL,
    0xE1D5BEF04E364A72ULL, 0xA3255F1BE7DC7CE1ULL,
    0x64347D271DE22754ULL, 0x26C49CCCB40811C7ULL,
    0x5CBD6CC0CC10FAFCULL, 0x1E4D8D2B65FACC6FULL,
    0xD95CAF179FC497DAULL, 0x9BAC4EFC362EA149ULL,
    0x158E0A85C2521623ULL, 0x577EEB6E6BB820B0ULL,
    0x906FC95291867B05ULL, 0xD29F28B9386C4D96ULL,
    0xCEDBA04AD0952342ULL, 0x8C2B41A1797F15D1ULL,
    0x4B3A639D83414E64ULL, 0x09CA82762AAB78F7ULL,
    0x87E8C60FDED7CF9DULL, 0xC51827E4773DF90EULL,
    0x020905D88D03A2BBULL, 0x40F9E43324E99428ULL,
    0x2CFFE7D5975E55E2ULL, 0x6E0F063E3EB46371ULL,
    0xA91E2402C48A38C4ULL, 0xEBEEC5E96D600E57ULL,
    0x65CC8190991CB93DULL, 0x273C607B30F68FAEULL,
    0xE02D4247CAC8D41BULL, 0xA2DDA3AC6322E288ULL,
    0xBE992B5F8BDB8C5CULL, 0xFC69CAB42231BACFULL,
    0x3B78E888D80FE17AULL, 0x7988096371E5D7E9ULL,
    0xF7AA4D1A85996083ULL, 0xB55AACF12C735610ULL,
    0x724B8ECDD64D0DA5ULL, 0x30BB6F267FA73B36ULL,
    0x4AC29F2A07BFD00DULL, 0x08327EC1AE55E69EULL,
    0xCF235CFD546BBD2BULL, 0x8DD3BD16FD818BB8ULL,
    0x03F1F96F09FD3CD2ULL, 0x41011884A0170A41ULL,
    0x86103AB85A2951F4ULL, 0xC4E0DB53F3C36767ULL,
    0xD8A453A01B3A09B3ULL, 0x9A54B24BB2D03F20ULL,
    0x5D45907748EE6495ULL, 0x1FB5719CE1045206ULL,
    0x919735E51578E56CULL, 0xD367D40EBC92D3FFULL,
    0x1476F63246AC884AULL, 0x568617D9EF46BED9ULL,
    0xE085162AB69D5E3CULL, 0xA275F7C11F7768AFULL,
    0x6564D5FDE549331AULL, 0x279434164CA30589ULL,
    0xA9B6706FB8DFB2E3ULL, 0xEB46918411358470ULL,
    0x2C57B3B8EB0BDFC5ULL, 0x6EA7525342E1E956ULL,
    0x72E3DAA0AA188782ULL, 0x30133B4B03F2B111ULL,
    0xF7021977F9CCEAA4ULL, 0xB5F2F89C5026DC37ULL,
    0x3BD0BCE5A45A6B5DULL, 0x79205D0E0DB05DCEULL,
    0xBE317F32F78E067BULL, 0xFCC19ED95E6430E8ULL,
    0x86B86ED5267CDBD3ULL, 0xC4488F3E8F96ED40ULL,
    0x0359AD0275A8B6F5ULL, 0x41A94CE9DC428066ULL,
    0xCF8B0890283E370CULL, 0x8D7BE97B81D4019FULL,
    0x4A6ACB477BEA5A2AULL, 0x089A2AACD2006CB9ULL,
    0x14DEA25F3AF9026DULL, 0x562E43B4931334FEULL,
    0x913F6188692D6F4BULL, 0xD3CF8063C0C759D8ULL,
    0x5DEDC41A34BBEEB2ULL, 0x1F1D25F19D51D821ULL,
    0xD80C07CD676F8394ULL, 0x9AFCE626CE85B507ULL
};

_Check_return_
ULONG
NkCrc32(
    _In_ ULONG InitialCrc,
    _In_reads_(Size) const VOID* Data,
    _In_ ULONG Size
    )
{
    const UCHAR *p = (const UCHAR*)Data;
    ULONG crc = InitialCrc;

    crc ^= ~0U;

    while (Size--) {
        crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
    }

    crc ^= ~0U;
    return crc;
}

_Check_return_
ULONGLONG
NkCrc64(
    _In_ ULONGLONG InitialCrc,
    _In_reads_(Size) const VOID* Data,
    _In_ ULONG Size
    )
{
    const unsigned char *p = (const unsigned char*)Data;
    ULONGLONG crc = InitialCrc;

    crc ^= ~0ULL;

    while (Size--) {
        crc = crc64_tab[((crc >> 56) ^ *p++) & 0xFF] ^ (crc << 8);
    }

    crc ^= ~0ULL;
    return crc;
}