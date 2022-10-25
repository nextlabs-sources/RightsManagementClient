
/**
* \file <nkdf/fs.h>
* \brief Header file for libkfs library
*
* This is the top level header file for libkfs library which implements
* most common file system operations. The library inlcude following
* features:
*    - volume operation
*    - disk operation
*    - file operation (Zw/Flt)
*    - mini-filter CSQ
*    - mini-filter communication
*
* \author Gavin Ye
* \version 1.0.0.0
* \date 10/6/2014
*
*/


#ifndef __NKDF_FILESYSTEM_H__
#define __NKDF_FILESYSTEM_H__


/**
 * \defgroup nkdf-fs Library: FileSystem
 * @{
 */

#include <nkdf/fs/vol.h>        /**< Volume operation support. */
#include <nkdf/fs/disk.h>       /**< Disk operation support. */
#include <nkdf/fs/fsop.h>       /**< File operation support. */
#include <nkdf/fs/fltop.h>      /**< Mini-filter file operation support. */
#include <nkdf/fs/fltcsq.h>     /**< Mini-filter cancel-safe-queue support. */
#include <nkdf/fs/fltconn.h>    /**< Mini-filter communication support. */


/**@}*/ // Group End: nkdf-fs

#endif  // #ifndef __NKDF_FILESYSTEM_H__