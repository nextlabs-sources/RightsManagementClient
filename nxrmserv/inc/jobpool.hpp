

#ifndef __NX_JOB_QUEUE_HPP__
#define __NX_JOB_QUEUE_HPP__


#include <string>
#include <vector>
#include <memory>
#include <map>
#include <thread>
#include <algorithm>

#include <nudf\web\json.hpp>

#include "agent.hpp"


namespace NX {


class job
{
public:
    job();
    job(const std::wstring& name, unsigned long repeat_interval=0);
    virtual ~job();

    void start();
    void stop(bool wait=true);
    void trigger();

    inline const std::wstring& name() const noexcept { return _name; }
    inline bool stopping() const noexcept { return _stopping; }
    inline unsigned long repeat_interval() const noexcept { return _repeat_interval; }
    inline void reset_repeat_interval(unsigned long interval) noexcept { _repeat_interval = interval; }

    virtual void do_job() noexcept {};

    static void worker(job* j) noexcept;
    static const std::wstring jn_register;
    static const std::wstring jn_heartbeat;
    static const std::wstring jn_audit;
    static const std::wstring jn_checkupdate;

private:
    std::wstring    _name;
    bool            _stopping;
    HANDLE          _evt;
    std::thread     _worker;
    unsigned long   _repeat_interval;
};

class job_pool
{
public:
    job_pool();
    virtual ~job_pool();

    bool add(std::shared_ptr<job>& p);
    std::shared_ptr<job> remove(const std::wstring& name);
    std::shared_ptr<job> get(const std::wstring& name);
    void clear();

private:
    std::vector<std::shared_ptr<job>> _queue;
    CRITICAL_SECTION _queue_lock;
};

class job_register : public job
{
public:
    job_register() : job(NX::job::jn_register, 180000) {} // repeat every 3 minutes
    virtual ~job_register() {}
    virtual void do_job() noexcept;
};

class job_heartbeat : public job
{
public:
    job_heartbeat() : job(NX::job::jn_heartbeat, 180000) {} // repeat every 3 minutes
    job_heartbeat(unsigned long iterval) : job(NX::job::jn_heartbeat, iterval) {}
    virtual ~job_heartbeat() {}
    virtual void do_job() noexcept;
};

class job_audit : public job
{
public:
    job_audit() : job(NX::job::jn_audit, 300000) {}    // repeat every 5 minutes
    job_audit(unsigned long iterval) : job(NX::job::jn_audit, iterval) {}
    virtual ~job_audit() {}
    virtual void do_job() noexcept;
};

class job_checkupdate : public job
{
public:
    job_checkupdate() : job(NX::job::jn_checkupdate, 600000) {}    // repeat every 10 minutes
    job_checkupdate(unsigned long iterval) : job(NX::job::jn_checkupdate, iterval) {}
    virtual ~job_checkupdate() {}
    virtual void do_job() noexcept;
};



}


#endif