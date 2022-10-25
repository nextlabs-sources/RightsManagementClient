

#ifndef __TIMER_SERV_HPP__
#define __TIMER_SERV_HPP__


namespace nxrm {
namespace engine {


class CTimer
{
public:
    CTimer();
    virtual ~CTimer();

    BOOL Set(_In_ ULONG elapse, BOOL initial=FALSE);
    BOOL Trigger();
    void Kill(BOOL wait=TRUE);
    inline ULONG GetElapse() const throw() {return _elapse;}
    inline void SetElapse(ULONG elapse) throw() {_elapse = (0==elapse) ? 1000: elapse;}

    virtual void OnTimer() = 0;

private:
    CTimer& operator = (const CTimer& timer) {return *this;}
    BOOL InterSetTimer();
    void InterOnTimer();

    static DWORD WINAPI TimerThreadProc(_In_ LPVOID lpVoid);


private:
    ULONG  _elapse;
    HANDLE _hthread;
    HANDLE _timer_events[2];    // 0: Stop Event
                                // 1: Trigger Event
};


class CRegisterTimer : public CTimer
{
public:
    CRegisterTimer() : CTimer() {}
    virtual ~CRegisterTimer() {}
    virtual void OnTimer();
};

class CHeartBeatTimer : public CTimer
{
public:
    CHeartBeatTimer() : CTimer() {}
    virtual ~CHeartBeatTimer() {}
    virtual void OnTimer();
};

class CCheckUpdateTimer : public CTimer
{
public:
    CCheckUpdateTimer() : CTimer() {}
    virtual ~CCheckUpdateTimer() {}
    virtual void OnTimer();
};

class CLogTimer : public CTimer
{
public:
    CLogTimer() : CTimer() {}
    virtual ~CLogTimer() {}
    virtual void OnTimer();
};



}   // namespace engine
}   // namespace nxrm






#endif  // __TIMER_SERV_HPP__