

#include <ntifs.h>
#include <Bcrypt.h>

#include <nkdf/basic/defines.h>
#include <nkdf/crypto/provider.h>
#include <nkdf/crypto/aes.h>



typedef struct _NKAESOBJECT {
    BCRYPT_KEY_HANDLE   KeyHandle;
    ULONG               BitsLength;
    ULONG               CbcLength;
} NKAESOBJECT, *PNKAESOBJECT;

#define MAX_IV_SIZE     64


_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkCreateAesObject(
                  _In_reads_(Size) const UCHAR* Key,
                  _In_ ULONG Size,
                  _In_ ULONG CbcSize,
                  _Out_ PHANDLE Handle
                  )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    PNKAESOBJECT    Object = NULL;
    PUCHAR          KeyBlob = NULL;

    // Parameters check
    if (NULL == Key || NULL == Handle) {
        return STATUS_INVALID_PARAMETER;
    }
    if (Size != 16 && Size != 32) {
        return STATUS_INVALID_PARAMETER;
    }
    if (0 != (CbcSize % Size)) {
        return STATUS_INVALID_PARAMETER;
    }
    if (0 != (CbcSize % 512)) {
        return STATUS_INVALID_PARAMETER;
    }

    if (!NkCryptoInitialized()) {
        return STATUS_DEVICE_NOT_READY;
    }

    try {
        
        // Allocate CNG Context
        Object = (PNKAESOBJECT)ExAllocatePoolWithTag(NonPagedPool, sizeof(NKAESOBJECT), TAG_ALG);
        if (NULL == Object) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }
        RtlZeroMemory(Object, sizeof(NKAESOBJECT));


        // Init CNG
        Object->CbcLength = CbcSize;
        Object->BitsLength = Size * 8;

        KeyBlob = ExAllocatePoolWithTag(NonPagedPool,
                                        sizeof(BCRYPT_KEY_DATA_BLOB_HEADER) + Size,
                                        TAG_TEMP);
        if (NULL == KeyBlob) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }


        // Fill key BLOB
        RtlZeroMemory(KeyBlob, sizeof(BCRYPT_KEY_DATA_BLOB_HEADER) + Size);
        ((PBCRYPT_KEY_DATA_BLOB_HEADER)KeyBlob)->cbKeyData = Size;
        ((PBCRYPT_KEY_DATA_BLOB_HEADER)KeyBlob)->dwVersion = BCRYPT_KEY_DATA_BLOB_VERSION1;
        ((PBCRYPT_KEY_DATA_BLOB_HEADER)KeyBlob)->dwMagic   = BCRYPT_KEY_DATA_BLOB_MAGIC;
        RtlCopyMemory(KeyBlob + sizeof(BCRYPT_KEY_DATA_BLOB_HEADER), Key, Size);

        Status = BCryptImportKey(NkCryptoGetProvider(PROV_AES),
                                 NULL,
                                 BCRYPT_KEY_DATA_BLOB,
                                 &Object->KeyHandle,
                                 NULL,
                                 0,
                                 KeyBlob,
                                 sizeof(BCRYPT_KEY_DATA_BLOB_HEADER) + Size,
                                 0);

try_exit: NOTHING;
    }
    finally {

        if (NULL != KeyBlob) {
            ExFreePool(KeyBlob);
            KeyBlob = NULL;
        }

        if (!NT_SUCCESS(Status)) {
            if (NULL != Object) {
                if (NULL != Object->KeyHandle) {
                    BCryptDestroyKey(Object->KeyHandle);
                    Object->KeyHandle = NULL;
                }
                ExFreePool(Object);
                Object = NULL;
            }
        }
    }

    *Handle = (PVOID)Object;
    return Status;
}

_IRQL_requires_max_(DISPATCH_LEVEL)
VOID
NkDestroyAesObject(
                   _In_ HANDLE Handle
                   )
{
    PNKAESOBJECT Object = (PNKAESOBJECT)Handle;

    if (NULL != Object) {

        if (NULL != Object->KeyHandle) {
            BCryptDestroyKey(Object->KeyHandle);
            Object->KeyHandle = NULL;
        }

        RtlZeroMemory(Object, sizeof(NKAESOBJECT));
        ExFreePool(Object);
        Object = NULL;
    }
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkAesEncrypt(
             _In_ HANDLE Handle,
             _In_ ULONGLONG Ivec,
             _Inout_updates_(Size) PVOID Data,
             _In_ ULONG Size
             )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    PNKAESOBJECT    Object = (PNKAESOBJECT)Handle;
    UCHAR           IV[MAX_IV_SIZE];


    // Parameters check
    if (NULL == Object || NULL == Object->KeyHandle || NULL == Data || 0 == Size) {
        return STATUS_INVALID_PARAMETER;
    }
    if (0 != (Ivec % Object->CbcLength)) {
        return STATUS_INVALID_PARAMETER;
    }
    if (0 != (Size%(Object->BitsLength/8))) {
        return STATUS_INVALID_PARAMETER;
    }

    while (Size != 0) {

        ULONG   BytesToEncrypt = min(Size, Object->CbcLength);
        ULONG   OutSize = 0;
        RtlZeroMemory(IV, MAX_IV_SIZE);
        RtlCopyMemory(IV, &Ivec, sizeof(ULONGLONG));
        Status = BCryptEncrypt( Object->KeyHandle,
                                Data,
                                BytesToEncrypt,
                                NULL,
                                IV,
                                Object->BitsLength / 8,
                                Data,
                                BytesToEncrypt,
                                &OutSize,
                                0);
        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        ASSERT(OutSize == BytesToEncrypt);
        Size -= BytesToEncrypt;
        Ivec += BytesToEncrypt;
        Data = (PVOID)(((PUCHAR)Data) + BytesToEncrypt);
    }
    

    return Status;
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkAesDecrypt(
             _In_ HANDLE Handle,
             _In_ ULONGLONG Ivec,
             _Inout_updates_(Size) PVOID Data,
             _In_ ULONG Size
             )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    PNKAESOBJECT    Object = (PNKAESOBJECT)Handle;
    UCHAR           IV[MAX_IV_SIZE];


    // Parameters check
    if (NULL == Object || NULL == Object->KeyHandle || NULL == Data || 0 == Size) {
        return STATUS_INVALID_PARAMETER;
    }
    if (0 != (Ivec % Object->CbcLength)) {
        return STATUS_INVALID_PARAMETER;
    }
    if (0 != (Size % (Object->BitsLength / 8))) {
        return STATUS_INVALID_PARAMETER;
    }

    while (Size != 0) {

        ULONG   BytesToDecrypt = min(Size, Object->CbcLength);
        ULONG   OutSize = 0;
        RtlZeroMemory(IV, MAX_IV_SIZE);
        RtlCopyMemory(IV, &Ivec, sizeof(ULONGLONG));
        Status = BCryptDecrypt( Object->KeyHandle,
                                Data,
                                BytesToDecrypt,
                                NULL,
                                IV,
                                Object->BitsLength / 8,
                                Data,
                                BytesToDecrypt,
                                &OutSize,
                                0
                                );
        ASSERT(OutSize == BytesToDecrypt);
        Size -= BytesToDecrypt;
        Ivec += BytesToDecrypt;
        Data = (PVOID)(((PUCHAR)Data) + BytesToDecrypt);
    }

    return Status;
}


_Check_return_
_IRQL_requires_max_(APC_LEVEL)
ULONG
NkRandom(
         )
{
    static ULONG RandSeed = 0;

    PAGED_CODE();

    if (0 == RandSeed) {
        LARGE_INTEGER Counter;
        Counter = KeQueryPerformanceCounter(NULL);
        RandSeed = Counter.LowPart ^ Counter.HighPart;
    }

    return RtlRandom(&RandSeed);
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkAesGenerateKey(
                 _Out_writes_(Size) PVOID Key,
                 _In_ ULONG Size
                 )
{
    ULONG* p = (ULONG*)Key;
    if (16 != Size && 24 != Size && 32 != Size) {
        return STATUS_INVALID_PARAMETER;
    }

    RtlZeroMemory(Key, Size);
    p[0] = NkRandom();
    p[1] = NkRandom();
    if (Size >= 24) {
        p[2] = NkRandom();
    }
    if (Size == 32) {
        p[3] = NkRandom();
    }
    return STATUS_SUCCESS;
}