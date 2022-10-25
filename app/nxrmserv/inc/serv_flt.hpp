

#pragma once
#ifndef __NXRM_DRVCORE_SERV_HPP__
#define __NXRM_DRVCORE_SERV_HPP__


#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <thread>

#include <boost/noncopyable.hpp>

#include "drvflt.hpp"

class drvflt_serv
{
public:
    class flt_request
    {
    public:
        flt_request() : _type(0), _context(NULL)
        {
        }
        flt_request(unsigned long type, void* msg, unsigned long length, void* msg_context);
        ~flt_request() {}

        inline void* request() const noexcept { return (void*)(_request.empty() ? NULL : _request.data()); }
        inline void* context() const noexcept { return _context; }
        inline unsigned long type() const noexcept { return _type; }
        inline bool empty() const noexcept { return _request.empty(); }


    private:
        flt_request& operator = (const flt_request& other)
        {
            assert(FALSE);
            return *this;
        }

    private:
        std::vector<unsigned char> _request;
        //void*           _request;
        unsigned long   _type;
        void*           _context;
    };

public:
    drvflt_serv();
    ~drvflt_serv();

    void start(const void* key_blob, unsigned long size) noexcept;
    void stop() noexcept;

    void fltctl_set_saveas_forecast(unsigned long process_id, const wchar_t* source, const wchar_t* target);
    void fltctl_set_policy_changed();
    void fltctl_set_keys(const void* key_blob, unsigned long size);

    inline bool is_running() const noexcept { return _running; }

    static void worker(drvflt_serv* serv) noexcept;
    static unsigned long __stdcall drv_callback(unsigned long type, void* msg, unsigned long length, void* msg_context, void* user_context);

protected:
    // Handle request
    void on_check_rights(flt_request* request);
    void on_block_notification(flt_request* request);
    void on_key_error_notification(flt_request* request);
    void on_purge_cache_notification(flt_request* request);
    void on_process_notification(flt_request* request);

private:
    std::vector<std::thread>                 _threads;
    std::queue<std::shared_ptr<flt_request>> _request_queue;
    CRITICAL_SECTION                         _queue_lock;
    HANDLE                                   _h;
    HANDLE                                   _request_events[2];
    bool                                     _running;
    NX::drvflt_man_instance                  _dll;
};



#endif