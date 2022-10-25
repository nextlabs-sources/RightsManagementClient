

#ifndef __NXRM_SERV_SVC_HPP__
#define __NXRM_SERV_SVC_HPP__

#include <string>
#include <thread>
#include <nudf\svc.hpp>

class CRmService : public nudf::win::svc::IService
{
public:
    CRmService();
    virtual ~CRmService();

    inline void set_delay(int delay) {_delay = (delay > 0) ? delay : 0;};

    virtual void OnStart();
    virtual void OnStop() noexcept;
    virtual void OnPause();
    virtual void OnResume();
    
    virtual LONG OnSessionLogon(_In_ WTSSESSION_NOTIFICATION* wtsn) noexcept;
    virtual LONG OnSessionLogoff(_In_ WTSSESSION_NOTIFICATION* wtsn) noexcept;

private:
    int _delay;
    CRITICAL_SECTION    _logon_lock;
};


#endif  // #ifndef __NXRM_SERV_SVC_HPP__