

#pragma once
#ifndef __NXRM_DRVCORE_SERV_HPP__
#define __NXRM_DRVCORE_SERV_HPP__


#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <thread>

#include <boost/noncopyable.hpp>

#include "drvcore.hpp"

class drvcore_serv : boost::noncopyable
{
public:
    class drv_request : boost::noncopyable
    {
    public:
        drv_request() : _type(0), _context(NULL) {}
        drv_request(unsigned long type, void* msg, unsigned long length, void* msg_context);
        ~drv_request() {}

        inline void* request() const noexcept { return (void*)(_request.empty() ? NULL : _request.data()); }
        inline void* context() const noexcept { return _context; }
        inline unsigned long type() const noexcept { return _type; }
        inline bool empty() const noexcept { return _request.empty(); }
        
    private:
        std::vector<unsigned char>  _request;
        unsigned long               _type;
        void*                       _context;
    };

public:
    drvcore_serv();
    ~drvcore_serv();
    
    void start();
    void stop();

    void drvctl_query_process_info(unsigned long process_id) noexcept;
    void drvctl_set_overlay_windows(unsigned long session_id, const std::vector<unsigned long>& hwnds);
    void drvctl_set_overlay_bitmap_status(unsigned long session_id, bool ready);

    inline bool is_running() const { return _running; }

    static void worker(drvcore_serv* serv) noexcept;
    static unsigned long __stdcall drv_callback(unsigned long type, void* msg, unsigned long length, void* msg_context, void* user_context);

protected:
    // Handle request
    virtual void on_check_obligations(drv_request* request);
    virtual void on_get_custom_ui(drv_request* request);
    virtual void on_save_as_forecast(drv_request* request);
    virtual void on_get_classify_ui(drv_request* request);
    virtual void on_block_notification(drv_request* request);
    virtual void on_check_protect_menu(drv_request* request);
    virtual void on_update_protected_mode_app_info(drv_request* request);
    virtual void on_query_protected_mode_app_info(drv_request* request);
    virtual void on_get_context_menu_regex(drv_request* request);
    virtual void on_update_core_context(drv_request* request);
    virtual void on_query_core_context(drv_request* request);
    virtual void on_query_service(drv_request* request);
    virtual void on_update_dwm_window(drv_request* request);
    virtual void on_update_overlay_window(drv_request* request);
    virtual void on_check_process_rights(drv_request* request);

private:
    bool    _running;
    std::queue<std::shared_ptr<drv_request>>    _request_list;
    CRITICAL_SECTION   _request_list_lock;
    std::vector<std::thread>    _threads;
    HANDLE _request_events[2];  // 0: stop event, 1: request coming event

    HANDLE  _h;
    NX::drvcore_man_instance _dll;
};



#endif