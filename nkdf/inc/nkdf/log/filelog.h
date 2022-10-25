
/**
 * \file <nkdf/log/filelog.h>
 * \brief Header file for file logging
 *
 * This header file declares routines used by file logging.
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */


#ifndef __NKDF_LOG_FILELOG_H__
#define __NKDF_LOG_FILELOG_H__

#include <ntifs.h>
#include <fltkernel.h>
#include <nkdf/log/logdef.h>


/**
 * \addtogroup nkdf-log
 * @{
 */


/**
 * \defgroup nkdf-log-filelog File Logging
 * @{
 */


/**
 * \defgroup nkdf-log-filelog-inline Inline Routines
 * @{
 */

/**
 * \brief Convert a log level to corresponding name.
 * \param Level The log level to be converted.
 * \return The name of this level.
 */
__forceinline
const CHAR*
NkLogGetLevelName(
                  _In_ LOGLEVEL Level
                  )
{
    switch (Level)
    {
    case LogCritical:
        return "CRIT ";
    case LogError:
        return "ERROR";
    case LogWarning:
        return "WARN ";
    case LogInfo:
        return "INFO ";
    case LogDebug:
        return "DEBUG";
    case LogDump:
        return "DUMP ";
    case LogMax:
    default:
        break;
    }

    return "UNKWN";
}

/**@}*/ // Group End: nkdf-log-filelog-inline


/**
 * \defgroup nkdf-log-filelog-api Routines
 * @{
 */


/**
 * \brief Function to initialize logging system. There can only be one logging system in a driver.
 * \IRQL Must be PASSIVE_LEVEL
 * \param AcceptedLevel The maximum log level accepted by the logging system.
 * \param RotateCount Count of rotation.
 * \param QueueLimit The maximum log message queue size.
 * \param FileSizeLimit The maximum log file queue size.
 * \param Directory The directory where the log file is created.
 * \param Name The name of the log file (without extension).
 * \return NTSTATUS
 */
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkLogInitialize(
                _In_ LOGLEVEL AcceptedLevel,
                _In_ ULONG RotateCount,
                _In_ ULONG QueueLimit,
                _In_ ULONG FileSizeLimit,
                _In_ PUNICODE_STRING Directory,
                _In_ PUNICODE_STRING Name
                );

/**
 * \brief Function to shutdown and cleanup logging system.
 * \IRQL Must be PASSIVE_LEVEL
 * \return N/A
 */
_IRQL_requires_(PASSIVE_LEVEL)
VOID
NkLogCleanup(
             );

/**
 * \brief Function to check if current thread is logging system worker thread.
 * \return BOOLEAN
 * \retval TRUE Current thread is logging system worker thread.
 * \retval FALSE Current thread is NOT logging system worker thread.
 */
_Check_return_
BOOLEAN
NkLogIsLogThread(
                 );

/**
 * \brief Function to get accepted log level of current logging system.
 * \return LOGLEVEL
 */
LOGLEVEL
NkLogGetLevel(
              );

/**
 * \brief Function to set new accepted log level of current logging system.
 * \param Level New log level.
 * \return N/A
 */
VOID
NkLogSetLevel(
              _In_ LOGLEVEL Level
              );

/**
 * \brief Function to get log rotation count of current logging system.
 * \return ULONG
 */
ULONG
NkLogGetRotate(
               );

/**
 * \brief Function to set new log rotation count of current logging system.
 * \param Count New log rotation count.
 * \return N/A
 */
VOID
NkLogSetRotate(
               _In_ ULONG Count
               );

/**
 * \brief Function to get log queue size of current logging system.
 * \return ULONG
 */
ULONG
NkLogGetQueueSizeLimit(
                       );

/**
 * \brief Function to set new log queue size of current logging system.
 * \param Limit New log queue size.
 * \return N/A
 */
VOID
NkLogSetQueueSizeLimit(
                       _In_ ULONG Limit
                       );

/**
 * \brief Function to get log file size limitation of current logging system.
 * \return ULONG, the maximum log file size, in megabytes.
 */
ULONG
NkLogGetFileSizeLimit(
                      );

/**
 * \brief Function to set new log file size of current logging system.
 * \param Limit New log file size.
 * \return N/A
 */
VOID
NkLogSetFileSizeLimit(
                      _In_ ULONG Limit
                      );

/**
 * \brief Function to log a message.
 * \IRQL Less or equal to DISPATCH_LEVEL
 * \param Level The level of this message.
 * \param MsgFmt The format of this message.
 * \return N/A
 */
_IRQL_requires_max_(DISPATCH_LEVEL)
VOID
NkLog(
      _In_ LOGLEVEL Level,
      _In_ const CHAR* MsgFmt,
      ...
      );

/**@}*/ // Group End: nkdf-log-filelog-api


/**
* \defgroup nkdf-log-filelog-macro Handy Macros
* @{
*/

/** \def TRACE(LEVEL, FORMAT, ...)
 *  Log a message.
 */
#define TRACE(LEVEL, FORMAT, ...)           \
    if((LEVEL)<=NkLogGetLevel()) {          \
        NkLog(LEVEL, FORMAT, __VA_ARGS__);  \
    }

/** \def TRACEFUNC(LEVEL, FORMAT, ...)
 *  Log a message with function information (function name and line).
 */
#define TRACEFUNC(LEVEL, FORMAT, ...)                                           \
    if((LEVEL)<=NkLogGetLevel()) {                                              \
        NkLog(LEVEL, "(%s, %d) "##FORMAT, __FUNCTION__, __LINE__, __VA_ARGS__); \
    }

/** \def WIND()
 *  Log a message which wind a function.
 */
#define WIND()                                  \
    if(LogDump<=NkLogGetLevel()) {              \
        NkLog(LEVEL, "WIND %s", __FUNCTION__);  \
    }

/** \def UNWIND()
 *  Log a message which unwind a function.
 */
#define UNWIND()                                 \
    if(LogDump<=NkLogGetLevel()) {               \
        NkLog(LEVEL, "UNWIND %s", __FUNCTION__); \
    }

/**@}*/ // Group End: nkdf-log-filelog-macro



/**@}*/ // Group End: nkdf-log-filelog


/**@}*/ // Group End: nkdf-log


#endif  // __NKDF_LOG_FILELOG_H__