
#ifndef __NX_DRV_CORE_H__
#define __NX_DRV_CORE_H__


#include <string>
#include <vector>
#include <list>
#include <memory>
#include <thread>

#include <nudf\thread.hpp>

namespace NX {
namespace drv {
    


class serv_drv
{
public:
    class drv_request
    {
    public:
        drv_request() : _type(0), _context(NULL)
        {
        }
        drv_request(unsigned long type, void* msg, unsigned long length, void* msg_context);
        ~drv_request() {}

        inline void* request() const noexcept { return (void*)(_request.empty() ? NULL : _request.data()); }
        inline void* context() const noexcept { return _context; }
        inline unsigned long type() const noexcept { return _type; }
        inline bool empty() const noexcept { return _request.empty(); }


    private:
        drv_request& operator = (const drv_request& other)
        {
            assert(FALSE);
            return *this;
        }

    private:
        std::vector<unsigned char> _request;
        unsigned long   _type;
        void*           _context;
    };

public:
    serv_drv();
    ~serv_drv();

    bool start() noexcept;
    void stop() noexcept;

    inline bool started() const noexcept { return _started; }

    process_info query_process_info(unsigned long process_id) noexcept;
    void set_overlay_windows(unsigned long session_id, const std::vector<unsigned long>& wnds);
    void set_overlay_bitmap_status(unsigned long session_id, bool ready);
	void increase_policy_sn();

    static void worker(serv_drv* serv) noexcept;
    static unsigned long __stdcall drv_callback(unsigned long type, void* msg, unsigned long length, void* msg_context, void* user_context);
    static BOOL __stdcall check_log_accept(unsigned long level);
    static LONG __stdcall write_log(const wchar_t* msg);

protected:
    // Handle request
    void on_get_custom_ui(drv_request* request);
    void on_check_obligations(drv_request* request);
    void on_save_as_forecast(drv_request* request);
    void on_get_classify_ui(drv_request* request);
    void on_notification(drv_request* request);
    void on_check_protect_menu(drv_request* request);
    void on_update_protect_mode_app_info(drv_request* request);
    void on_query_protect_mode_app_info(drv_request* request);
    void on_get_context_menu_regex(drv_request* request);
    void on_update_core_context(drv_request* request);
    void on_query_core_context(drv_request* request);
    void on_query_service(drv_request* request);
    void on_update_dwm_window(drv_request* request, bool overlay);
    void on_check_process_rights(drv_request* request);

    void on_unknown_request(drv_request* request);

private:
    std::vector<std::thread>                _threads;
    std::list<std::shared_ptr<drv_request>> _list;
    CRITICAL_SECTION                        _list_lock;
    HANDLE                                  _drvman;
    HANDLE                                  _stop_event;
    HANDLE                                  _request_event;
    bool                                    _started;
};
       
}   // namespace NX::drv
}   // namespace NX


#endif  // #ifndef __NX_DRV_CORE_H__