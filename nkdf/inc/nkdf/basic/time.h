
/**
 * \file <nkdf/basic/time.h>
 * \brief Header file for time help routines
 * This header file declare time help routines
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */

#ifndef __NKDF_BASIC_TIME_H__
#define __NKDF_BASIC_TIME_H__



/**
 * \addtogroup nkdf-basic
 * @{
 */


/**
 * \defgroup nkdf-basic-time Time Support
 * @{
 */


/**
 * \defgroup nkdf-basic-time-macro Help Macros
 * @{
 */

#define TIME_FORMATA        "YYYY-MM-DD HH:MM:SS.MMM"   /**< Time format */
#define TIME_FORMATA_SIZE   24                          /**< Time format, include last '\0' */


/**@}*/ // Group End: nkdf-basic-time-macro


/**
 * \defgroup nkdf-basic-time-api Routines
 * @{
 */


/**
 * \brief Convert time to string
 * \IRQL Requires PASSIVE_LEVEL
 * \param Time Time to convert
 * \param TimeString Output buffer whose size must be at least TIME_FORMATA_SIZE.
 * \return NTSTATUS
 */
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkTimeToStringA(
                _In_ LONGLONG Time,
                _Out_writes_(TIME_FORMATA_SIZE) CHAR* TimeString
                );

/**
 * \brief Get current system time
 * \IRQL Less than or equal to DISPATCH_LEVEL
 * \return Current system time
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeGetSystemTime(
                    );

/**
 * \brief Get current local time
 * \IRQL Less than or equal to DISPATCH_LEVEL
 * \return Current local time
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeGetLocalTime(
                   );


/**
 * \brief Adjust a system/local time by adding or substracting N milliseconds
 * \IRQL Less than or equal to DISPATCH_LEVEL
 * \param N N milliseconds.
 * \return new system time
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeAdjustTime(
                 _In_ LONGLONG SystemTime,
                 _In_ LONG N
                 );


/**
 * \brief Convert seconds to picoseconds
 * \IRQL Less than or equal to DISPATCH_LEVEL
 * \param Time Time in seconds.
 * \return Picoseconds
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeSecondToPicoSecond(
                         _In_ LONGLONG Time
                         );

/**
 * \brief Convert seconds to nanoseconds
 * \IRQL Less than or equal to DISPATCH_LEVEL
 * \param Time Time in seconds.
 * \return Nanoseconds
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeSecondToNanoSecond(
                         _In_ LONGLONG Time
                         );

/**
 * \brief Convert seconds to microseconds
 * \IRQL Less than or equal to DISPATCH_LEVEL
 * \param Time Time in seconds.
 * \return Microseconds
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeSecondToMicroSecond(
                          _In_ LONGLONG Time
                          );

/**
 * \brief Convert seconds to microseconds
 * \IRQL Less than or equal to DISPATCH_LEVEL
 * \param Time Time in seconds.
 * \return Milliseconds
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeSecondToMilliSecond(
                          _In_ LONGLONG Time
                          );

/**
 * \brief Convert picoseconds to seconds
 * \IRQL Less than or equal to DISPATCH_LEVEL
 * \param Time Time in picoseconds.
 * \return Seconds
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimePicoSecondToSecond(
                         _In_ LONGLONG Time
                         );

/**
 * \brief Convert nanoseconds to seconds
 * \IRQL Less than or equal to DISPATCH_LEVEL
 * \param Time Time in nanoseconds.
 * \return Seconds
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeNanoSecondToSecond(
                         _In_ LONGLONG Time
                         );

/**
 * \brief Convert microseconds to seconds
 * \IRQL Less than or equal to DISPATCH_LEVEL
 * \param Time Time in microseconds.
 * \return Seconds
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeMicroSecondToSecond(
                          _In_ LONGLONG Time
                          );

/**
 * \brief Convert milliseconds to seconds
 * \IRQL Less than or equal to DISPATCH_LEVEL
 * \param Time Time in milliseconds.
 * \return Seconds
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeMilliSecondToSecond(
                          _In_ LONGLONG Time
                          );

/**
 * \brief Convert milliseconds to nanoseconds
 * \IRQL Less than or equal to DISPATCH_LEVEL
 * \param Time Time in milliseconds.
 * \return Nanoeconds
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeMilliSecondToNanoSecond(
                              _In_ LONGLONG Time
                              );

/**
 * \brief Convert microseconds to nanoseconds
 * \IRQL Less than or equal to DISPATCH_LEVEL
 * \param Time Time in microseconds.
 * \return Nanoeconds
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeMicroSecondToNanoSecond(
                              _In_ LONGLONG Time
                              );

/**
 * \brief Convert nanoseconds to milliseconds
 * \IRQL Less than or equal to DISPATCH_LEVEL
 * \param Time Time in nanoseconds.
 * \return Milliseconds
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeNanoSecondToMilliSecond(
                              _In_ LONGLONG Time
                              );

/**
 * \brief Convert nanoseconds to microseconds
 * \IRQL Less than or equal to DISPATCH_LEVEL
 * \param Time Time in nanoseconds.
 * \return Microseconds
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeNanoSecondToMicroSecond(
                              _In_ LONGLONG Time
                              );

/**
 * \brief Convert SYSTEMTIME to nanoseconds
 * \IRQL Less than or equal to DISPATCH_LEVEL
 * \param Time Time in SYSTEMTIME.
 * \return nanoseconds
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeSystemTimeToNanoSecond(
                             _In_ LONGLONG Time
                             );

/**
 * \brief Convert SYSTEMTIME to microseconds
 * \IRQL Less than or equal to DISPATCH_LEVEL
 * \param Time Time in SYSTEMTIME.
 * \return microseconds
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeSystemTimeToMicroSecond(
                              _In_ LONGLONG Time
                              );

/**
 * \brief Convert SYSTEMTIME to milliseconds
 * \IRQL Less than or equal to DISPATCH_LEVEL
 * \param Time Time in SYSTEMTIME.
 * \return milliseconds
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeSystemTimeToMilliSecond(
                              _In_ LONGLONG Time
                              );

/**
 * \brief Convert nanoseconds to SYSTEMTIME
 * \IRQL Less than or equal to DISPATCH_LEVEL
 * \param Time Time in nanoseconds.
 * \return SYSTEMTIME
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeNanoSecondToSystemTime(
                             _In_ LONGLONG Time
                             );

/**
 * \brief Convert microseconds to SYSTEMTIME
 * \IRQL Less than or equal to DISPATCH_LEVEL
 * \param Time Time in microseconds.
 * \return SYSTEMTIME
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeMicroSecondToSystemTime(
                              _In_ LONGLONG Time
                              );

/**
 * \brief Convert milliseconds to SYSTEMTIME
 * \IRQL Less than or equal to DISPATCH_LEVEL
 * \param Time Time in milliseconds.
 * \return SYSTEMTIME
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeMilliSecondToSystemTime(
                              _In_ LONGLONG Time
                              );

/**
 * \brief Get system live time
 * \IRQL Less than or equal to DISPATCH_LEVEL
 * \return System live time in microseconds.
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeGetSystemLiveTime(
                        );

/**
 * \brief Sleep for X milliseconds
 * \IRQL Less than or equal to DISPATCH_LEVEL
 * \return N/A
 */
_IRQL_requires_max_(APC_LEVEL)
VOID
NkTimeSleep(
            _In_ LONG MilliSeconds
            );


/**@}*/ // Group End: nkdf-basic-time-api


/**@}*/ // Group End: nkdf-basic-tme


/**@}*/ // Group End: nkdf-basic



#endif  // #ifndef __NKDF_BASIC_TIME_H__