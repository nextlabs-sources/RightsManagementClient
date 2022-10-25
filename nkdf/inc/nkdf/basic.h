
/**
 * \file <nkdf/basic.h>
 * \brief Header file for libkbasic library
 *
 * This is the top level header file for libkbasic library which implements
 * most basic features used by kernel mode developement. The library inlcude
 * following fetaures:
 *    - definitions
 *    - PE image APIs
 *    - memory management APIs
 *    - string APIs
 *    - time APIs
 *    - registry APIs
 *    - device APIs
 *    - debug APIs
 *    - CSQ
 *    - safe list
 *    - safe generic table
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */

#ifndef __NKDF_BASIC_H__
#define __NKDF_BASIC_H__


/**
 * \defgroup nkdf-basic Library: Basic
 * @{
 */

#include <nkdf/basic/defines.h>     /**< NKDF definitions. */
#include <nkdf/basic/image.h>       /**< PE Image support. */
//#include <nkdf/basic/ntapi.h>       /**< NT undocumented APIs. */
#include <nkdf/basic/mem.h>         /**< Memory support. */
#include <nkdf/basic/string.h>      /**< String support. */
#include <nkdf/basic/time.h>        /**< Time support. */
#include <nkdf/basic/registry.h>    /**< Registry support. */
#include <nkdf/basic/device.h>      /**< Device support. */
#include <nkdf/basic/debug.h>       /**< Debug support. */
#include <nkdf/basic/csq.h>         /**< Safe-cancel-queue support. */
#include <nkdf/basic/list.h>        /**< Safe list. */
#include <nkdf/basic/gtable.h>      /**< Safe generic table. */



/**@}*/ // Group End: nkdf-basic

#endif  // #ifndef __NKDF_BASIC_H__