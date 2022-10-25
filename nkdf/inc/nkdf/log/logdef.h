
/**
 * \file <nkdf/log/logdef.h>
 * \brief Header file for logging definitions
 *
 * This header file defines const and macros for logging system
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */


#ifndef __NKDF_LOG_DEFINES_H__
#define __NKDF_LOG_DEFINES_H__


/**
 * \addtogroup nkdf-log
 * @{
 */


/**
 * \defgroup nkdf-log-def Definitions
 * @{
 */


/**
 * \defgroup nkdf-log-def-enum Enums
 * @{
 */

/** \enum _LOGLEVEL
 */
enum _LOGLEVEL {
    LogCritical = 0,    /**< The CRITICAL level designates very severe error events that will presumably lead the application to abort. (0) */
    LogError,           /**< The ERROR level designates error events that might still allow the application to continue running. (1) */
    LogWarning,         /**< The WARNING level designates potentially harmful situations. (2) */
    LogInfo,            /**< The INFO level designates informational messages that highlight the progress of the application at coarse-grained level. (3) */
    LogDebug,           /**< The DEBUG Level designates fine-grained informational events that are most useful to debug an application. (4) */
    LogDump,            /**< The DUMP Level designates finer-grained informational events than the DEBUG (5) */
    LogMax              /**< Maximum level (6) */
};
typedef enum _LOGLEVEL LOGLEVEL;    /**< LOGLEVEL */

/** \def LogAssert
 *  The LogAssert is equivlent to LogCritical.
 */
#define LogAssert   LogCritical


/**@}*/ // Group End: nkdf-log-def-enum


/**
 * \defgroup nkdf-log-def-const Consts
 * @{
 */

/**
 * \defgroup nkdf-log-def-const-header Header
 * Defines the size and format of a log item's header.
 * @{
 */


/** \def LOG_HEADER_LENGTH
 *  The header size is fixed which is 49 bytes.
 */
#define LOG_HEADER_LENGTH   49

/** \def LOG_HEADER_FORMAT
 *  The header is always in following format:
 *  "[Date Time] <ProcessId.ThreadId> LEVEL: " \n
 *  \par Here are some examples: \n
 *  "[2013/03/29 15:20:26.654] <0x3720.0x3812> CRIT : " \n
 *  "[2013/03/29 15:20:26.654] <0x3720.0x3812> ERROR: " \n
 *  "[2013/03/29 15:20:26.654] <0x3720.0x3812> WARN : " \n
 *  "[2013/03/29 15:20:26.654] <0x3720.0x3812> INFO : " \n
 *  "[2013/03/29 15:20:26.654] <0x3720.0x3812> DEBUG: " \n
 *  "[2013/03/29 15:20:26.654] <0x3720.0x3812> DUMP : " \n
 */
#define LOG_HEADER_FORMAT   "[%04d/%02d/%02d %02d:%02d:%02d.%03d] <0x%04X.0x%04X> %s: "


/**@}*/ // Group End: nkdf-log-def-const-header


/** \def MAX_ROTATE_COUNT
 *  The maximum number of rotation is 10.
 */
#define MAX_ROTATE_COUNT    10

/*
The maximum PAGED memory used to store log messages is 1024*512 (512KB)
*/

/** \def MIN_LOG_QUEUE_SIZE
 *  The minimum log queue size is 256.
 */
#define MIN_LOG_QUEUE_SIZE  256

/** \def MAX_LOG_QUEUE_SIZE
 *  The maximum log queue size is 1024.
 *  So the maximum PAGED memory used to store log messages is 1024*512 (512KB).
 */
#define MAX_LOG_QUEUE_SIZE  1024

/** \def MAX_LOG_MSG_LENGTH
 *  The maximum log message length is 512 bytes.
 */
#define MAX_LOG_MSG_LENGTH  512


/** \def MIN_LOG_FILE_SIZE
 *  The minimum log file size 1 megabytes.
 */
#define MIN_LOG_FILE_SIZE   1

/** \def MAX_LOG_FILE_SIZE
 *  The maximum log file size 64 megabytes.
 */
#define MAX_LOG_FILE_SIZE   64



/**@}*/ // Group End: nkdf-log-def-const


/**@}*/ // Group End: nkdf-log-def


/**@}*/ // Group End: nkdf-log




#endif  // __NKDF_LOG_DEFINES_H__