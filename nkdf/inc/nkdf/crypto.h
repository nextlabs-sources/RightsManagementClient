
/**
* \file <nkdf/crypto.h>
* \brief Header file for libkcrypto library
*
* This is the top level header file for libkcrypto library which implements
* crypto althorithm for kernel mode developement. The library inlcude following
* crypto althorithm:
*    - AES
*    - RC4
*    - MD5
*    - SHA (1/256)
*    - CRC (32/64)
*    - base64
*
* \author Gavin Ye
* \version 1.0.0.0
* \date 10/6/2014
*
*/

#ifndef __NKDF_CRYPTO_H__
#define __NKDF_CRYPTO_H__


/**
 * \defgroup nkdf-crypto Library: Crypto
 * @{
 */

#include <nkdf/crypto/provider.h>   /**< CNG Provider. */
#include <nkdf/crypto/aes.h>        /**< AES Algorithm. */
#include <nkdf/crypto/rsa.h>        /**< RSA Algorithm. */
#include <nkdf/crypto/rc4.h>        /**< RC4 Algorithm. */
#include <nkdf/crypto/md5.h>        /**< MD5 Algorithm. */
#include <nkdf/crypto/sha.h>        /**< SHA (1/256) Algorithm. */
#include <nkdf/crypto/crc.h>        /**< CRC (32/64) Algorithm. */
#include <nkdf/crypto/base64.h>     /**< Base64 Algorithm. */



/**@}*/ // Group End: nkdf-crypto

#endif  // #ifndef __NKDF_CRYPTO_H__