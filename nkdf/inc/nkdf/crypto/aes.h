
/**
 * \file <nkdf/crypto/aes.h>
 * \brief Header file for aes crypto routines
 *
 * This header file declares aes crypto routines
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */


#ifndef __NKDF_CRYPTO_AES_H__
#define __NKDF_CRYPTO_AES_H__




/**
 * \addtogroup nkdf-crypto
 * @{
 */


/**
 * \defgroup nkdf-crypto-aes AES
 * @{
 */


/**
 * \defgroup nkdf-crypto-aes-api Routines
 * @{
 */

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkCreateAesObject(
                  _In_reads_(Size) const UCHAR* Key,
                  _In_ ULONG Size,
                  _In_ ULONG CbcSize,
                  _Out_ PHANDLE Handle
                  );

_IRQL_requires_max_(DISPATCH_LEVEL)
VOID
NkDestroyAesObject(
                   _In_ HANDLE Handle
                   );

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkAesEncrypt(
             _In_ HANDLE Handle,
             _In_ ULONGLONG Ivec,
             _Inout_updates_(Size) PVOID Data,
             _In_ ULONG Size
             );

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkAesDecrypt(
             _In_ HANDLE Handle,
             _In_ ULONGLONG Ivec,
             _Inout_updates_(Size) PVOID Data,
             _In_ ULONG Size
             );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkAesGenerateKey(
                 _Out_writes_(Size) PVOID Key,
                 _In_ ULONG Size
                 );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
ULONG
NkRandom(
         );



/**@}*/ // Group End: nkdf-crypto-aes-api

/**@}*/ // Group End: nkdf-crypto-aes

/**@}*/ // Group End: nkdf-crypto


#endif  // #ifndef __NKDF_CRYPTO_AES_H__



