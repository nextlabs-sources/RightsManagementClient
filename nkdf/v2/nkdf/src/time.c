

#include "stdafx.h"


#include <nkdf\time.h>

//
//  Declaration
//





//
//  Definition
//

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkTimeToStringA(
    _In_ LONGLONG Time,
    _In_ BOOLEAN IsLocalTime,
    _Out_writes_z_(TIME_FORMAT_SIZE) CHAR* TimeString
    )
{
    TIME_FIELDS     TimeFields;
    LARGE_INTEGER   LargeTime;
    NTSTATUS        Status = STATUS_SUCCESS;

    LargeTime.QuadPart = Time;
    RtlTimeToTimeFields(&LargeTime, &TimeFields);
    Status = RtlStringCbPrintfA(TimeString,
        TIME_FORMAT_SIZE,
        TIME_FORMATA,
        TimeFields.Year,
        TimeFields.Month,
        TimeFields.Day,
        TimeFields.Hour,
        TimeFields.Minute,
        TimeFields.Second,
        TimeFields.Milliseconds,
        IsLocalTime ? ' ' : 'Z'
        );
    TimeString[TIME_FORMAT_SIZE - 1] = '\0';
    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkTimeToStringW(
    _In_ LONGLONG Time,
    _In_ BOOLEAN IsLocalTime,
    _Out_writes_z_(TIME_FORMAT_SIZE) WCHAR* TimeString
    )
{
    TIME_FIELDS     TimeFields;
    LARGE_INTEGER   LargeTime;
    NTSTATUS        Status = STATUS_SUCCESS;

    LargeTime.QuadPart = Time;
    RtlTimeToTimeFields(&LargeTime, &TimeFields);
    Status = RtlStringCbPrintfW(TimeString,
        TIME_FORMAT_SIZE * sizeof(WCHAR),
        TIME_FORMATW,
        TimeFields.Year,
        TimeFields.Month,
        TimeFields.Day,
        TimeFields.Hour,
        TimeFields.Minute,
        TimeFields.Second,
        TimeFields.Milliseconds,
        IsLocalTime ? L' ' : L'Z'
        );
    TimeString[TIME_FORMAT_SIZE - 1] = '\0';
    return Status;
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkGetSystemTime(
    )
{
    LARGE_INTEGER SystemTime = { 0, 0 };
    KeQuerySystemTime(&SystemTime);
    return SystemTime.QuadPart;
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkGetLocalTime(
    )
{
    LARGE_INTEGER SystemTime = { 0, 0 };
    LARGE_INTEGER LocalTime = { 0, 0 };
    KeQuerySystemTime(&SystemTime);
    ExSystemTimeToLocalTime(&SystemTime, &LocalTime);
    return LocalTime.QuadPart;
}


_Check_return_
LONGLONG
NkPerformanceFrequency(
    _In_ FREQUENCY_PRECISION Precision
    )
{
    typedef struct _PERFORMANCE_FREQUENCIES {
        LONGLONG    SecondsFrequency;
        LONGLONG    MicrosecondsFrequency;
        LONGLONG    MillisecondsFrequency;
    } PERFORMANCE_FREQUENCIES;

    static PERFORMANCE_FREQUENCIES Frequencies = {
        0,
        0,
        0
    };

    if (Frequencies.SecondsFrequency == 0) {
        LARGE_INTEGER PerformanceFrequency = { 0, 0 };
        KeQueryPerformanceCounter(&PerformanceFrequency);
        Frequencies.SecondsFrequency = PerformanceFrequency.QuadPart;
        Frequencies.MillisecondsFrequency = PerformanceFrequency.QuadPart / 1000;
        Frequencies.MicrosecondsFrequency = PerformanceFrequency.QuadPart / 1000000;
    }

    // Default is microseconds frequency
    return ((Precision == PrecisionSeconds) ? Frequencies.SecondsFrequency : (Precision == PrecisionMilliSeconds ? Frequencies.MillisecondsFrequency : Frequencies.MicrosecondsFrequency));
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
__forceinline
LONGLONG
NkSystemTickCount(
    )
{
    LARGE_INTEGER TickCount = KeQueryPerformanceCounter(NULL);
    return TickCount.QuadPart;
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkSystemLiveSeconds(
    )
{
    return (NkSystemTickCount() / NkPerformanceFrequency(PrecisionSeconds));
}

_IRQL_requires_max_(APC_LEVEL)
VOID
NkSleepMicroSeconds(
    _In_ LONGLONG MicroSeconds
    )
{
    static const LONGLONG DelayOneMicroSecond = (-10);
    LARGE_INTEGER   Interval = { 0, 0 };
    Interval.QuadPart = MicroSeconds * DelayOneMicroSecond;
    KeDelayExecutionThread(KernelMode, 0, &Interval);
}

_IRQL_requires_max_(APC_LEVEL)
VOID
NkSleepMilliSeconds(
    _In_ LONGLONG MilliSeconds
    )
{
    static const LONGLONG DelayOneMilliSecond = (-10000);
    LARGE_INTEGER   Interval = { 0, 0 };
    Interval.QuadPart = MilliSeconds * DelayOneMilliSecond;
    KeDelayExecutionThread(KernelMode, 0, &Interval);
}

_IRQL_requires_max_(APC_LEVEL)
VOID
NkSleepSeconds(
    _In_ LONG Seconds
    )
{
    static const LONGLONG DelayOneSecond = (-10000000);
    LARGE_INTEGER   Interval = { 0, 0 };
    Interval.QuadPart = Seconds * DelayOneSecond;
    KeDelayExecutionThread(KernelMode, 0, &Interval);
}