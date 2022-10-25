


#pragma once
#ifndef __NKDF_TIME_H__
#define __NKDF_TIME_H__


// Format String: "YYYY-MM-DDTHH:MM:SS.MMM[Z]"
#define TIME_FORMATA        "%04d-%02d-%02dT%02d:%02d:%02d.%03d%c"  /**< Time format */
#define TIME_FORMATW        L"%04d-%02d-%02dT%02d:%02d:%02d.%03d%c" /**< Time format */
#define TIME_FORMAT_SIZE    25                                      /**< Time format, include last '\0' */


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkTimeToStringA(
    _In_ LONGLONG Time,
    _In_ BOOLEAN IsLocalTime,
    _Out_writes_z_(TIME_FORMAT_SIZE) CHAR* TimeString
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkTimeToStringW(
    _In_ LONGLONG Time,
    _In_ BOOLEAN IsLocalTime,
    _Out_writes_z_(TIME_FORMAT_SIZE) WCHAR* TimeString
    );

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkGetSystemTime(
    );

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkGetLocalTime(
    );

typedef enum FREQUENCY_PRECISION {
    PrecisionMicroSeconds = 0,
    PrecisionMilliSeconds,
    PrecisionSeconds
} FREQUENCY_PRECISION;

_Check_return_
LONGLONG
NkPerformanceFrequency(
    _In_ FREQUENCY_PRECISION Precision
    );

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
__forceinline
LONGLONG
NkSystemTickCount(
    );

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkSystemLiveSeconds(
    );

_IRQL_requires_max_(APC_LEVEL)
VOID
NkSleepMicroSeconds(
    _In_ LONGLONG MicroSeconds
    );

_IRQL_requires_max_(APC_LEVEL)
VOID
NkSleepMilliSeconds(
    _In_ LONGLONG MilliSeconds
    );

_IRQL_requires_max_(APC_LEVEL)
VOID
NkSleepSeconds(
    _In_ LONG MilliSeconds
    );


__forceinline
LONGLONG
NkTimeToNanoSeconds(
    _In_ LONGLONG Time
    )
{
    return (Time * 100);
}

__forceinline
LONGLONG
NkTimeToMicroSeconds(
    _In_ LONGLONG Time
    )
{
    return (Time / 10);
}

__forceinline
LONGLONG
NkTimeToMilliSeconds(
    _In_ LONGLONG Time
    )
{
    return (Time / 10000);
}

__forceinline
LONGLONG
NkTimeToSeconds(
    _In_ LONGLONG Time
    )
{
    return (Time / 10000000);
}

__forceinline
LONGLONG
NkAdjustNanoSeconds(
    _In_ LONGLONG Time,
    _In_ LONGLONG N
    )
{
    return (Time + (N / 100));
}

__forceinline
LONGLONG
NkAdjustMicroSeconds(
    _In_ LONGLONG Time,
    _In_ LONGLONG N
    )
{
    return (Time + (N * 10));
}

__forceinline
LONGLONG
NkAdjustMilliSeconds(
    _In_ LONGLONG Time,
    _In_ LONGLONG N
    )
{
    return (Time + (N * 10000));
}

__forceinline
LONGLONG
NkAdjustSeconds(
    _In_ LONGLONG Time,
    _In_ LONGLONG N
    )
{
    return (Time + (N * 10000000));
}


#endif