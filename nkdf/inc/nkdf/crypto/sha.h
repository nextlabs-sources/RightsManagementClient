
/**
 * \file <nkdf/crypto/sha.h>
 * \brief Header file for sha crypto routines
 *
 * This header file declares sha crypto routines
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */


#ifndef __NKDF_CRYPTO_SHA_H__
#define __NKDF_CRYPTO_SHA_H__



/**
 * \addtogroup nkdf-crypto
 * @{
 */


/**
 * \defgroup nkdf-crypto-sha SHA
 * @{
 */


/**
 * \defgroup nkdf-crypto-sha-api Routines
 * @{
 */


_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkSha1Hash(
           _In_reads_(Size) const VOID* Data,
           _In_ ULONG Size,
           _Out_writes_(20) PVOID Hash
           );

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkSha256Hash(
             _In_reads_(Size) const VOID* Data,
             _In_ ULONG Size,
             _Out_writes_(32) PVOID Hash
             );

/**@}*/ // Group End: nkdf-crypto-sha-api

/**@}*/ // Group End: nkdf-crypto-sha

/**@}*/ // Group End: nkdf-crypto


#endif  // #ifndef __NKDF_CRYPTO_SHA_H__



