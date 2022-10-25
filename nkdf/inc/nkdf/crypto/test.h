
/**
 * \file <nkdf/crypto/test.h>
 * \brief Header file for crypto self-test
 *
 * This header file declares crypto self-test routines
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */



#ifndef __NKDF_CRYPTO_TEST_H__
#define __NKDF_CRYPTO_TEST_H__



/**
 * \addtogroup nkdf-crypto
 * @{
 */


/**
 * \defgroup nkdf-crypto-test Self Test
 * @{
 */

#ifdef _DEBUG
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoTest(
             );
#endif



/**@}*/ // Group End: nkdf-crypto-test

/**@}*/ // Group End: nkdf-crypto


#endif  // __NKDF_CRYPTO_TEST_H__