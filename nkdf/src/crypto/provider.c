
#include <ntifs.h>
#include <Bcrypt.h>

#include <nkdf/basic/defines.h>
#include <nkdf/crypto/provider.h>
#include <nkdf/crypto/aes.h>
#include <nkdf/crypto/md5.h>




static const LPCWSTR ALgorithmNames[PROV_MAX] = {
    BCRYPT_AES_ALGORITHM,   // AES
    BCRYPT_RSA_ALGORITHM,   // RSA
    BCRYPT_RC4_ALGORITHM,   // RC4
    BCRYPT_MD5_ALGORITHM,   // MD5
    BCRYPT_SHA1_ALGORITHM,  // SHA1
    BCRYPT_SHA256_ALGORITHM // SHA256
};

static BCRYPT_ALG_HANDLE Providers[PROV_MAX] = {
    NULL,   // AES
    NULL,   // RSA
    NULL,   // RC4
    NULL,   // MD5
    NULL,   // SHA1
    NULL    // SHA256
};

#ifdef DEBUG
NTSTATUS
TestAes(
        );
#endif

//  Assign text sections for each routine.
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NkCryptoInit)
#pragma alloc_text(PAGE, NkCryptoCleanup)
#endif


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoInit(
             )
{
    NTSTATUS    Status = STATUS_SUCCESS;

    PAGED_CODE();


    try {

        BCRYPT_KEY_LENGTHS_STRUCT   KeyLengthInfo = { 0, 0, 0 };
        ULONG ReturnedLength = 0;
		int i = 0;

        for (i = 0; i < PROV_MAX; i++) {
            if (NULL == Providers[i]) {
                Status = BCryptOpenAlgorithmProvider(&Providers[i], ALgorithmNames[i], MS_PRIMITIVE_PROVIDER, (PROV_RSA == i) ? 0 : BCRYPT_PROV_DISPATCH);
                if (!NT_SUCCESS(Status)) {
                    try_return(Providers[i] = NULL);
                }
            }
        }

        //
        // extra initialize
        //

        // For AES Algorithm

        Status = BCryptSetProperty(Providers[PROV_AES],
                                   BCRYPT_CHAINING_MODE,
                                   (PUCHAR)BCRYPT_CHAIN_MODE_CBC,
                                   sizeof(BCRYPT_CHAIN_MODE_CBC),
                                   0);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        //
        //  make sure provider support AES 128, AES 256, RSA 1024 and RSA 2048
        //
        Status = BCryptGetProperty(Providers[PROV_AES], BCRYPT_KEY_LENGTHS, (PUCHAR)&KeyLengthInfo, sizeof(KeyLengthInfo), &ReturnedLength, 0);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        // For RSA Algorithm 
        Status = BCryptGetProperty(Providers[PROV_RSA], BCRYPT_KEY_LENGTHS, (PUCHAR)&KeyLengthInfo, sizeof(KeyLengthInfo), &ReturnedLength, 0);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

#ifdef DEBUG
        TestAes();
#endif

try_exit: NOTHING;
    }
    finally {
        if (!NT_SUCCESS(Status)) {
            NkCryptoCleanup();
        }
    }

    return Status;
}

_IRQL_requires_(PASSIVE_LEVEL)
VOID
NkCryptoCleanup(
                )
{
    int i = 0;
	PAGED_CODE();
    for (i = 0; i < PROV_MAX; i++) {
        if (NULL != Providers[i]) {
            (VOID)BCryptCloseAlgorithmProvider(Providers[i], 0);
            Providers[i] = NULL;
        }
    }
}

_Check_return_
BOOLEAN
NkCryptoInitialized(
                    )
{
	int i = 0;

    for (i = 0; i < PROV_MAX; i++) {
        if (NULL == Providers[i]) {
            return FALSE;
        }
    }

    return TRUE;
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
BCRYPT_ALG_HANDLE
NkCryptoGetProvider(
                    _In_ NKCRYPTO_PROV_ID Id
                    )
{
    return (Id < 0 || Id >= PROV_MAX) ? NULL : Providers[Id];
}


#ifdef DEBUG
NTSTATUS
TestAes(
        )
{
    const UCHAR Key[32] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10 };
    CHAR Info[32];
    HANDLE AesObject = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    RtlZeroMemory(Info, 32);
    RtlCopyMemory(Info, "This is a test", 15);

    Status = NkCreateAesObject(Key, 32, 512, &AesObject);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    try {

        Status = NkAesEncrypt(AesObject, 0, Info, 32);
        if (!NT_SUCCESS(Status)) {
            try_return(NOTHING);
        }

        Status = NkAesDecrypt(AesObject, 0, Info, 32);
        if (!NT_SUCCESS(Status)) {
            try_return(NOTHING);
        }

        if (15 != RtlCompareMemory(Info, "This is a test", 15)) {
            try_return(NOTHING);
        }

        Status = STATUS_SUCCESS;

try_exit: NOTHING;
    }
    finally {
        NkDestroyAesObject(AesObject);
    }

    return Status;
}
#endif