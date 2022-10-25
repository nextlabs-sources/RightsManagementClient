
/**
* \file <nkdf/process.h>
* \brief Header file for libkproc library
*
* This is the top level header file for libkproc library which implements
* help APIs to support following features:
*    - process monitor (w/ callback)
*    - utility APIs to manipulate process
*
* \author Gavin Ye
* \version 1.0.0.0
* \date 10/6/2014
*
*/

#ifndef __NKDF_PROCESS_H__
#define __NKDF_PROCESS_H__


/**
 * \defgroup nkdf-proc Library: Process
 * @{
 */


#include <nkdf/process/utility.h>   /**< Process utility APIs. */
#include <nkdf/process/monitor.h>   /**< Process monitor. */


/**@}*/ // Group End: nkdf-proc


#endif  // __NKDF_PROCESS_H__