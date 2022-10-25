

#include <ntifs.h>
#include <Ntstrsafe.h>


#include <nkdf/basic/time.h>




//
//  time to string
//
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkTimeToStringA(
                _In_ LONGLONG Time,
                _Out_writes_(TIME_FORMATA_SIZE) CHAR* TimeString
                )
{
    TIME_FIELDS     TimeFields;
    LARGE_INTEGER   LargeTime;
    NTSTATUS        Status = STATUS_SUCCESS;

    LargeTime.QuadPart = Time;
    RtlTimeToTimeFields(&LargeTime, &TimeFields);
    Status = RtlStringCbPrintfA(TimeString,
                                TIME_FORMATA_SIZE,
                                "%04d-%02d-%02d %02d:%02d:%02d.%03d",
                                TimeFields.Year,
                                TimeFields.Month,
                                TimeFields.Day,
                                TimeFields.Hour,
                                TimeFields.Minute,
                                TimeFields.Second,
                                TimeFields.Milliseconds
                                );
    TimeString[TIME_FORMATA_SIZE-1] = '\0';
    return Status;
}


_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeGetSystemTime(
                    )
{
    LARGE_INTEGER SystemTime = { 0, 0 };
    KeQuerySystemTime(&SystemTime);
    return SystemTime.QuadPart;
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeGetLocalTime(
                   )
{
    LARGE_INTEGER SystemTime = { 0, 0 };
    LARGE_INTEGER LocalTime = { 0, 0 };
    KeQuerySystemTime(&SystemTime);
    ExSystemTimeToLocalTime(&SystemTime, &LocalTime);
    return LocalTime.QuadPart;
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeAdjustTime(
                 _In_ LONGLONG SystemTime,
                 _In_ LONG N
                 )
{
    return (SystemTime + (N * 10000));
}

//
//  time convert
//
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeSecondToPicoSecond(
                         _In_ LONGLONG Time
                         )
{
    return (Time * 1000000000000);
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeSecondToNanoSecond(
                         _In_ LONGLONG Time
                         )
{
    return (Time * 1000000000);
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeSecondToMicroSecond(
                          _In_ LONGLONG Time
                          )
{
    return (Time * 1000000);
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeSecondToMilliSecond(
                          _In_ LONGLONG Time
                          )
{
    return (Time * 1000);
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimePicoSecondToSecond(
                         _In_ LONGLONG Time
                         )
{
    return (Time / 1000000000000);
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeNanoSecondToSecond(
                         _In_ LONGLONG Time
                         )
{
    return (Time / 1000000000);
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeMicroSecondToSecond(
                          _In_ LONGLONG Time
                          )
{
    return (Time / 1000000);
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeMilliSecondToSecond(
                          _In_ LONGLONG Time
                          )
{
    return (Time / 1000);
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeMilliSecondToNanoSecond(
                              _In_ LONGLONG Time
                              )
{
    return (Time * 1000000);
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeMicroSecondToNanoSecond(
                              _In_ LONGLONG Time
                              )
{
    return (Time * 1000);
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeNanoSecondToMilliSecond(
                              _In_ LONGLONG Time
                              )
{
    return (Time / 1000000);
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeNanoSecondToMicroSecond(
                              _In_ LONGLONG Time
                              )
{
    return (Time / 1000);
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeSystemTimeToNanoSecond(
                             _In_ LONGLONG Time
                             )
{
    return (Time * 100);
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeSystemTimeToMicroSecond(
                              _In_ LONGLONG Time
                              )
{
    return (Time / 10);
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeSystemTimeToMilliSecond(
                              _In_ LONGLONG Time
                              )
{
    return (Time / 10000);
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeNanoSecondToSystemTime(
                             _In_ LONGLONG Time
                             )
{
    return (Time / 100);
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeMicroSecondToSystemTime(
                              _In_ LONGLONG Time
                              )
{
    return (Time * 10);
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeMilliSecondToSystemTime(
                              _In_ LONGLONG Time
                              )
{
    return (Time * 10000);
}


//
//  System Time
//

_Check_return_
LONGLONG
NkTimeGetFrequency(
                   )
{
    static LARGE_INTEGER PerformanceFrequency = { 0, 0 };
    if (PerformanceFrequency.QuadPart == 0) {
        KeQueryPerformanceCounter(&PerformanceFrequency);
    }
    return PerformanceFrequency.QuadPart;
}


_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONGLONG
NkTimeGetSystemLiveTime(
                        )
{
    LARGE_INTEGER TickCount = { 0, 0 };

    static LONGLONG MicroFrequency = 0;
    if (0 == MicroFrequency) {
        MicroFrequency = NkTimeGetFrequency() / 1000000;
    }

    if (0 != MicroFrequency) {
        TickCount = KeQueryPerformanceCounter(NULL);
        TickCount.QuadPart /= MicroFrequency;
    }

    return TickCount.QuadPart;
}

#define DELAY_ONE_MICROSECOND 	(-10)
#define DELAY_ONE_MILLISECOND	(DELAY_ONE_MICROSECOND*1000)

_IRQL_requires_max_(APC_LEVEL)
VOID
NkTimeSleep(
            _In_ LONG MilliSeconds
            )
{
    LARGE_INTEGER   Interval = { 0, 0 };
    Interval.QuadPart  = DELAY_ONE_MILLISECOND;
    Interval.QuadPart *= MilliSeconds;
    KeDelayExecutionThread(KernelMode, 0, &Interval);
}