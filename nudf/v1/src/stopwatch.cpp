

#include <Windows.h>

#include <nudf\stopwatch.hpp>

using namespace nudf::util;


class CPerfFrequency
{
public:
    CPerfFrequency()
    {
        _frequency.QuadPart = 0LL;
        QueryPerformanceFrequency(&_frequency);
        _loresol_frequency.QuadPart = _frequency.QuadPart / 1000;
        _hiresol_frequency.QuadPart = _loresol_frequency.QuadPart / 1000;
    }
    ~CPerfFrequency()
    {
        _frequency.QuadPart = 0LL;
    }

    inline __int64 GetHighResolutionFrequency() const throw() {return _hiresol_frequency.QuadPart;}
    inline __int64 GetLowResolutionFrequency() const throw() {return _loresol_frequency.QuadPart;}

private:
    LARGE_INTEGER _frequency;
    LARGE_INTEGER _hiresol_frequency;   // MicroSeconds
    LARGE_INTEGER _loresol_frequency;   // MilliSeconds
};

static CPerfFrequency _perf_frequency;

CStopWatch::CStopWatch() : _frequency(0)
{
    _frequency = _perf_frequency.GetLowResolutionFrequency();
}

CStopWatch::CStopWatch(_In_ bool highresolution, _In_ bool autostart) : _frequency(0)
{
    _frequency = highresolution ? _perf_frequency.GetHighResolutionFrequency() : _perf_frequency.GetLowResolutionFrequency();
    if(autostart) {
        Start();
    }
}

CStopWatch::~CStopWatch()
{
}

void CStopWatch::Start() throw()
{
    _start = 0;
    if(_frequency != 0) {
        if(!QueryPerformanceCounter((PLARGE_INTEGER)&_start)) {
            _start = 0;
        }
    }
}

void CStopWatch::Stop() throw()
{
    __int64 stoptime = 0;
    if(0 == _frequency || 0 == _start) {
        _start = 0;
        _elapse = 0;
        return;
    }
    if(!QueryPerformanceCounter((PLARGE_INTEGER)&stoptime)) {
        _start = 0;
        _elapse = 0;
        return;
    }
    stoptime -= _start;
    _start = 0;
    _elapse = stoptime / _frequency;
}

CAutoStopWatch::CAutoStopWatch() : CStopWatch(false, true)
{
}

CAutoStopWatch::CAutoStopWatch(_In_ bool highresolution) : CStopWatch(highresolution, true)
{
}

CAutoStopWatch::~CAutoStopWatch()
{
}