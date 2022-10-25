
#pragma once
#ifndef __NXRMDRV_CORE_HPP__
#define __NXRMDRV_CORE_HPP__

#include <string>
#include "function_dll.hpp"


namespace NX {

// it exports 22 functions
#define FLTMAN_FUNCTION_NUMBER  23

class drvcore_man_instance : public dll_instance<FLTMAN_FUNCTION_NUMBER>
{
public:
    drvcore_man_instance();
    virtual ~drvcore_man_instance();

    virtual void load(const std::wstring& dll_file);

    HANDLE create_manager(void* callback_notify, void* callback_log, void* callback_logaccept, void* context);
    unsigned long start(HANDLE h);
    unsigned long stop(HANDLE h);
    unsigned long post_check_obligations_response(HANDLE h, void*, void* obligations, unsigned long size);
    unsigned long query_process_info(HANDLE h, unsigned long process_id, void* info);
    unsigned long is_request_canceled(HANDLE h, void* context);
    unsigned long is_process_alive(HANDLE h, unsigned long process_id);
    unsigned long post_office_get_custom_ui_response(HANDLE h, void* context, void* response);
    unsigned long increase_policy_serial_number(HANDLE h);
    unsigned long close_manager(HANDLE h);
    unsigned long post_save_as_forecast_response(HANDLE h, void* response);
    unsigned long post_get_classify_ui_response(HANDLE h, void* context, void* response);
    unsigned long post_block_notification_response(HANDLE h, void* response);
    unsigned long post_check_protect_menu_response(HANDLE h, void* context, void* response);
    unsigned long post_get_context_menu_regex_response(HANDLE h, void* context, void* response);
    unsigned long post_update_core_context_response(HANDLE h, void* response);
    unsigned long post_query_core_context_response(HANDLE h, void* context, void* response);
    unsigned long post_query_service_response(HANDLE h, void* context, void* response);
    unsigned long post_query_update_dwm_window_response(HANDLE h, void* response);
    unsigned long post_query_update_overlay_window_response(HANDLE h, void* response);
    unsigned long post_check_process_rights_response(HANDLE h, void* context, void* response);
    unsigned long enable_antitampering(BOOL b);
    unsigned long set_overlay_protected_windows(HANDLE h, unsigned long session_id, unsigned long* hwnds, unsigned long* cb_size);
    unsigned long set_overlay_bitmap_status(HANDLE h, unsigned long session_id, bool ready);

};


class drvcore_controller
{
public:
    drvcore_controller();
    virtual ~drvcore_controller();

    bool connect();
    void disconnect();

    inline bool is_connected() const { return (NULL != _h); }

    bool drvcore_is_good_version();
    bool drvcore_is_enabled();
    bool drvcore_is_channel_on(long channel_id);
    bool drvcore_is_process_protected(unsigned long process_id = 0);
    bool drvcore_is_overlay_bitmap_ready();

    unsigned long drvcore_get_policy_serial_number();
    std::vector<HWND> drvcore_get_overlay_windows();
    void drvcore_set_dwm_active_session(unsigned long session_id);

    // request


    // notify


protected:
    std::vector<unsigned char> send_request(unsigned long type, std::vector<unsigned char>& request_data, bool wait);
    void send_notify(unsigned long type, std::vector<unsigned char>& request_data);


private:
    // no copy/move is allowed
    drvcore_controller& operator = (const drvcore_controller& other) { return *this; }
    drvcore_controller& operator = (drvcore_controller&& other) { return *this; }
private:
    HANDLE  _h;
};


}


#endif