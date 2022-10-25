

#include <ntifs.h>
#include <Bcrypt.h>

#include <nkdf/basic/defines.h>
#include <nkdf/crypto/provider.h>
#include <nkdf/crypto/sha.h>




_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkSha1Hash(
           _In_reads_(Size) const VOID* Data,
           _In_ ULONG Size,
           _Out_writes_(20) PVOID Hash
           )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    BCRYPT_HASH_HANDLE  Handle = NULL;
    ULONG               HashLength = 0;
    UCHAR               HashData[64];
    ULONG               cbHashObject = 0;
    ULONG               cbData = 0;
    PUCHAR              pbHashObject = NULL;


    if (NULL == Hash) {
        return STATUS_INVALID_PARAMETER;
    }
    if (NULL == Data || 0 == Size) {
        return STATUS_INVALID_PARAMETER;
    }
    RtlZeroMemory(Hash, 20);

    // Parameters check
    if (!NkCryptoInitialized()) {
        return STATUS_DEVICE_NOT_READY;
    }

    try {

        ULONG cbResult = 0;

        Status = BCryptGetProperty(NkCryptoGetProvider(PROV_SHA1),
                                   BCRYPT_OBJECT_LENGTH,
                                   (PUCHAR)&cbHashObject,
                                   sizeof(ULONG),
                                   &cbData,
                                   0);
        if (!NT_SUCCESS(Status)) {			
            try_return(Status);
        }

        pbHashObject = ExAllocatePoolWithTag(NonPagedPool, cbHashObject, TAG_TEMP);
        if (!pbHashObject) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }

        Status = BCryptCreateHash(NkCryptoGetProvider(PROV_SHA1),
                                  &Handle,
                                  pbHashObject,
                                  cbHashObject,
                                  NULL,
                                  0,
                                  0);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = BCryptHashData(Handle,
                                (PUCHAR)Data,
                                Size,
                                0);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = BCryptGetProperty(NkCryptoGetProvider(PROV_SHA1),
                                   BCRYPT_HASH_LENGTH,
                                   (PUCHAR)&HashLength,
                                   sizeof(ULONG),
                                   &cbResult,
                                   0);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        if (HashLength > 64) {
            try_return(Status = STATUS_BUFFER_OVERFLOW);
        }

        RtlZeroMemory(HashData, 64);
        Status = BCryptFinishHash(Handle,
                                  HashData,
                                  HashLength,
                                  0);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        RtlCopyMemory(Hash, HashData, 20);

try_exit: NOTHING;
    }
    finally {

        if (NULL != Handle) {
            (VOID)BCryptDestroyHash(Handle);
            Handle = NULL;
        }

        if (NULL != pbHashObject) {
            ExFreePool(pbHashObject);
            pbHashObject = NULL;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkSha256Hash(
             _In_reads_(Size) const VOID* Data,
             _In_ ULONG Size,
             _Out_writes_(32) PVOID Hash
             )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    BCRYPT_HASH_HANDLE  Handle = NULL;
    ULONG               HashLength = 0;
    UCHAR               HashData[64];
    ULONG               cbHashObject = 0;
    ULONG               cbData = 0;
    PUCHAR              pbHashObject = NULL;


    if (NULL == Hash) {
        return STATUS_INVALID_PARAMETER;
    }
    if (NULL == Data || 0 == Size) {
        return STATUS_INVALID_PARAMETER;
    }
    RtlZeroMemory(Hash, 32);

    // Parameters check
    if (!NkCryptoInitialized()) {
        return STATUS_DEVICE_NOT_READY;
    }

    try {

        ULONG cbResult = 0;

        Status = BCryptGetProperty(NkCryptoGetProvider(PROV_SHA256),
                                   BCRYPT_OBJECT_LENGTH,
                                   (PUCHAR)&cbHashObject,
                                   sizeof(ULONG),
                                   &cbData,
                                   0);
        if (!NT_SUCCESS(Status)) {			
            try_return(Status);
        }

        pbHashObject = ExAllocatePoolWithTag(NonPagedPool, cbHashObject, TAG_TEMP);
        if (!pbHashObject) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }

        Status = BCryptCreateHash(NkCryptoGetProvider(PROV_SHA256),
                                  &Handle,
                                  pbHashObject,
                                  cbHashObject,
                                  NULL,
                                  0,
                                  0);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = BCryptHashData(Handle,
                                (PUCHAR)Data,
                                Size,
                                0);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = BCryptGetProperty(NkCryptoGetProvider(PROV_SHA256),
                                   BCRYPT_HASH_LENGTH,
                                   (PUCHAR)&HashLength,
                                   sizeof(ULONG),
                                   &cbResult,
                                   0);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        if (HashLength > 64) {
            try_return(Status = STATUS_BUFFER_OVERFLOW);
        }

        RtlZeroMemory(HashData, 64);
        Status = BCryptFinishHash(Handle,
                                  HashData,
                                  HashLength,
                                  0);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        RtlCopyMemory(Hash, HashData, 32);

try_exit: NOTHING;
    }
    finally {

        if (NULL != Handle) {
            (VOID)BCryptDestroyHash(Handle);
            Handle = NULL;
        }

        if (NULL != pbHashObject) {
            ExFreePool(pbHashObject);
            pbHashObject = NULL;
        }
    }

    return Status;
}