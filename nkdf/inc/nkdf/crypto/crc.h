
/**
 * \file <nkdf/crypto/crc.h>
 * \brief Header file for crc crypto routines
 *
 * This header file declares crc crypto routines
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */


#ifndef __NKDF_CRYPTO_CRC_H__
#define __NKDF_CRYPTO_CRC_H__


/**
 * \addtogroup nkdf-crypto
 * @{
 */


/**
 * \defgroup nkdf-crypto-crc CRC
 * @{
 */


/**
 * \defgroup nkdf-crypto-crc-api Routines
 * @{
 */


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


/**@}*/ // Group End: nkdf-crypto-crc-api

/**@}*/ // Group End: nkdf-crypto-crc

/**@}*/ // Group End: nkdf-crypto



#endif  // #ifndef __NKDF_CRYPTO_CRC_H__



