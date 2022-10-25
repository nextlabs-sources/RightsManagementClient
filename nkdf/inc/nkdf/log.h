
/**
* \file <nkdf/log.h>
* \brief Header file for libklog library
*
* This is the top level header file for libklog library which implements
* a kernel mode logging system. The logging system support following output:
*    - output to log file
*    - output to debugger
*    - foutput to Windows Event Log (TBD)
*
* \author Gavin Ye
* \version 1.0.0.0
* \date 10/6/2014
*
*/



#ifndef __NKDF_LOG_H__
#define __NKDF_LOG_H__


/**
 * \defgroup nkdf-log Library: Logging
 * @{
 */

#include <nkdf/log/logdef.h>    /**< Logging definition. */
#include <nkdf/log/filelog.h>   /**< File logging. */


/**@}*/ // Group End: nkdf-log

#endif  // __NKDF_LOG_H__