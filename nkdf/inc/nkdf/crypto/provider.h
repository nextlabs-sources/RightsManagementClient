
/**
 * \file <nkdf/crypto/provider.h>
 * \brief Header file for crypto provider
 *
 * This header file declares crypto provider routines
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */



#ifndef __NKDF_CRYPTO_PROVIDER_H__
#define __NKDF_CRYPTO_PROVIDER_H__

#include <Bcrypt.h>




/**
 * \addtogroup nkdf-crypto
 * @{
 */


/**
 * \defgroup nkdf-crypto-prov Provider
 * @{
 */


/**
 * \defgroup nkdf-crypto-prov-def Defitions
 * @{
 */


/**
 *  \enum _NKCRYPTO_PROV_ID
 *  Provider IDs
 */
enum _NKCRYPTO_PROV_ID {
    PROV_AES,       /**< AES Provider    (0) */
    PROV_RSA,       /**< RSA Provider    (1) */
    PROV_RC4,       /**< RC4 Provider    (2) */
    PROV_MD5,       /**< MD5 Provider    (3) */
    PROV_SHA1,      /**< SHA1 Provider   (4) */
    PROV_SHA256,    /**< SHA256 Provider (5) */
    PROV_MAX        /**< Maximum Provider Id Value  (6) */
};

typedef enum _NKCRYPTO_PROV_ID NKCRYPTO_PROV_ID;    /**< Enum type */


/**@}*/ // Group End: nkdf-crypto-prov-def


/**
 * \defgroup nkdf-crypto-prov-api Routines
 * @{
 */


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCryptoInit(
             );

_IRQL_requires_(PASSIVE_LEVEL)
VOID
NkCryptoCleanup(
                );

_Check_return_
BOOLEAN
NkCryptoInitialized();

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
BCRYPT_ALG_HANDLE
NkCryptoGetProvider(
                    _In_ NKCRYPTO_PROV_ID Id
                    );


/**@}*/ // Group End: nkdf-crypto-prov-api

/**@}*/ // Group End: nkdf-crypto-prov

/**@}*/ // Group End: nkdf-crypto



#endif  // #ifndef __NKDF_CRYPTO_H__