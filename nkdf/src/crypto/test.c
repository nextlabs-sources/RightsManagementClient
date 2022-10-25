



#include <ntifs.h>

#include <nkdf/basic/defines.h>
#include <nkdf/crypto.h>

#include <nkdf/crypto/test.h>



#ifdef _DEBUG

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoTestAes128(
                   );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoTestAes256(
                   );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoTestMD5(
                );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoTestSHA1(
                 );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoTestSHA256(
                   );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoTestCRC32(
                  );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoTestCRC64(
                  );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoTestRC4(
                );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoTestBase64(
                   );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoTestRsa1024(
                   );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoTestRsa2048(
                    );



static PUCHAR      OriginalData = NULL;
static PUCHAR      SwapData = NULL;
static const ULONG DataSize = 1024;


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoTest(
             )
{
    NTSTATUS    Status = STATUS_SUCCESS;

    Status = NkCryptoInit();
    if (!NT_SUCCESS(Status)) {
        return Status;
    }


    try {

        ULONG i = 0;

        OriginalData = ExAllocatePoolWithTag(NonPagedPool, DataSize, TAG_TEMP);
        if (NULL == OriginalData) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }

        SwapData = ExAllocatePoolWithTag(NonPagedPool, DataSize, TAG_TEMP);
        if (NULL == SwapData) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlSecureZeroMemory(SwapData, DataSize);
        for (i = 0; i < DataSize; i++) {
            OriginalData[i] = 'Z';
        }

        // Test AES 128
        Status = NkCryptoTestAes128();
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        // Test AES 256
        Status = NkCryptoTestAes256();
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        // Test MD5
        Status = NkCryptoTestMD5();
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        // Test SHA1
        Status = NkCryptoTestSHA1();
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        // Test SHA256
        Status = NkCryptoTestSHA256();
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        // Test CRC 32
        Status = NkCryptoTestCRC32();
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        // Test CRC 64
        Status = NkCryptoTestCRC64();
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        // Test RC4
        Status = NkCryptoTestRC4();
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        // Test Base64
        Status = NkCryptoTestBase64();
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        // Test RSA 1024
        Status = NkCryptoTestRsa1024();
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        // Test RSA 2048
        Status = NkCryptoTestRsa2048();
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

try_exit: NOTHING;
    }
    finally {

        NkCryptoCleanup();

        if (NULL != OriginalData) {
            ExFreePool(OriginalData);
            OriginalData = NULL;
        }
        if (NULL != SwapData) {
            ExFreePool(SwapData);
            SwapData = NULL;
        }
    }

    return Status;
}


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoTestAes128(
                   )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    const UCHAR* Plain = OriginalData;
    PUCHAR Cipher = SwapData;
    HANDLE AesObject = NULL;
    UCHAR Key[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    Status = NkCreateAesObject(Key, 16, 512, &AesObject);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    try {

        Status = NkAesGenerateKey(Key, 16);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        RtlCopyMemory(Cipher, Plain, DataSize);
        Status = NkAesEncrypt(AesObject, 0, Cipher, DataSize);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = NkAesDecrypt(AesObject, 0, Cipher, DataSize);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        if (DataSize != RtlCompareMemory(Plain, Cipher, DataSize)) {
            try_return(Status = STATUS_UNSUCCESSFUL);
        }

try_exit: NOTHING;
    }
    finally {
        NkDestroyAesObject(AesObject);
        AesObject = NULL;
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoTestAes256(
                   )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    const UCHAR* Plain = OriginalData;
    PUCHAR Cipher = SwapData;
    HANDLE AesObject = NULL;
    UCHAR Key[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    Status = NkCreateAesObject(Key, 16, 512, &AesObject);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    try {

        Status = NkAesGenerateKey(Key, 32);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        RtlCopyMemory(Cipher, Plain, DataSize);
        Status = NkAesEncrypt(AesObject, 0, Cipher, DataSize);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = NkAesDecrypt(AesObject, 0, Cipher, DataSize);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        if (DataSize != RtlCompareMemory(Plain, Cipher, DataSize)) {
            try_return(Status = STATUS_UNSUCCESSFUL);
        }

    try_exit: NOTHING;
    }
    finally {
        NkDestroyAesObject(AesObject);
        AesObject = NULL;
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoTestMD5(
                )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    const UCHAR* Plain = OriginalData;
    UCHAR Md5Hash[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    Status = NkMd5Hash(Plain, DataSize, Md5Hash);

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoTestSHA1(
                 )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    const UCHAR* Plain = OriginalData;
    UCHAR ShaHash[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    Status = NkSha1Hash(Plain, DataSize, ShaHash);

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoTestSHA256(
                   )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    const UCHAR* Plain = OriginalData;
    UCHAR ShaHash[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    Status = NkSha256Hash(Plain, DataSize, ShaHash);

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoTestCRC32(
                  )
{
    const UCHAR* Plain = OriginalData;
    ULONG Crc32 = 0;

    Crc32 = NkCrc32(0, Plain, DataSize);

    return (0 != Crc32) ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoTestCRC64(
                  )
{
    const UCHAR* Plain = OriginalData;
    ULONGLONG Crc64 = 0;

    Crc64 = NkCrc64(0, Plain, DataSize);

    return (0 != Crc64) ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoTestRC4(
                )
{
    NTSTATUS Status = STATUS_SUCCESS;
    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoTestBase64(
                   )
{
    NTSTATUS Status = STATUS_SUCCESS;
    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoTestRsa1024(
                    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoTestRsa2048(
                    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    return Status;
}


#endif  // #ifdef _DEBUG