
/**
 * \file <nkdf/crypto/md5.h>
 * \brief Header file for md5 crypto routines
 *
 * This header file declares md5 crypto routines
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */


#ifndef __NKDF_CRYPTO_MD5_H__
#define __NKDF_CRYPTO_MD5_H__



/**
 * \addtogroup nkdf-crypto
 * @{
 */


/**
 * \defgroup nkdf-crypto-md5 MD5
 * @{
 */


/**
 * \defgroup nkdf-crypto-md5-api Routines
 * @{
 */


_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkMd5Hash(
          _In_reads_(Size) const VOID* Data,
          _In_ ULONG Size,
          _Out_writes_(16) PVOID Hash
          );


/**@}*/ // Group End: nkdf-crypto-md5-api

/**@}*/ // Group End: nkdf-crypto-md5

/**@}*/ // Group End: nkdf-crypto



#endif  // #ifndef __NKDF_CRYPTO_MD5_H__



