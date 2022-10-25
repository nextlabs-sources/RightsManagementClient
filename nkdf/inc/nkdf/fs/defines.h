
/**
 * \file <nkdf/fs/defines.h>
 * \brief Header file for file system support defiitions
 *
 * This header file contains definitions for NKDF file system support
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */


#ifndef __NKDF_FS_DEFINES_H__
#define __NKDF_FS_DEFINES_H__

#include <ntifs.h>


/**
 * \addtogroup nkdf-fs
 * @{
 */


/**
 * \defgroup nkdf-fs-def Definitions
 * @{
 */


/**
 * \defgroup nkdf-fs-def-struct Structs
 * @{
 */


/**
 * \struct _FILE_OBJECT_EX
 */
typedef struct _FILE_OBJECT_EX {
    HANDLE          Handle; /**< File Handle */
    PFILE_OBJECT    Object; /**< File Object */
}
FILE_OBJECT_EX,     /**< FILE_OBJECT_EX */
*PFILE_OBJECT_EX;   /**< PFILE_OBJECT_EX */



/**@}*/ // Group End: nkdf-fs-def-struct


/**@}*/ // Group End: nkdf-fs-def


/**@}*/ // Group End: nkdf-fs




#endif  // #ifndef __NKDF_FS_DEFINES_H__