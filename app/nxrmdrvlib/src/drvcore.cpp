

#include <Windows.h>


#include <nudf\eh.hpp>
#include <nudf\debug.hpp>

// from drvman
#include "nxrmdrv.h"
#include "nxrmdrvman.h"
#include "nxrmcorehlp.h"

#include "drvcore.hpp"


using namespace NX;


#define ID_CREATE_MANAGER                               101
#define ID_START                                        102
#define ID_STOP                                         103
#define ID_POSTCHECK_OBLIGATIONS_RESPONSE               104
#define ID_QUERY_PROCESSINFO                            105
#define ID_IS_REQUEST_CANCELED                          106
#define ID_IS_PROCESS_ALIVE                             107
#define ID_POST_OFFICE_GETCUSTOMUI_RESPONSE             108
#define ID_INCREASE_POLICY_SERIALNO                     109
#define ID_POST_SAVEAS_FORECAST_RESPONSE                110
#define ID_POST_GET_CLASSIFY_UI_RESPONSE                111
#define ID_POST_BLOCK_NOTIFICATION_RESPONSE             112
#define ID_POST_CHECK_PROTECT_MENU_RESPONSE             113
#define ID_POST_GET_CONTEXT_MENU_REGEX_RESPONSE         114
#define ID_POST_UPDATE_CORE_CONTEXT_RESPONSE            115
#define ID_POST_QUERY_CORE_CONTEXT_RESPONSE             116
#define ID_POST_QUERY_SERVICE_RESPONSE                  117
#define ID_POST_QUERY_UPDATE_DWM_WINDOW_RESPONSE        118
#define ID_POST_QUERY_UPDATE_OVERLAY_WINDOW_RESPONSE    119
#define ID_POST_CHECK_PROCESS_RIGHTS_RESPONSE           120
#define ID_CLOSE_MANAGER                                199
#define ID_ENABLE_ANTITAMPERING                         200
#define ID_SET_OVERLAY_PROTECTED_WINDOWS                201
#define ID_SET_OVERLAY_SET_STATUS                       202



drvcore_man_instance::drvcore_man_instance() : dll_instance<FLTMAN_FUNCTION_NUMBER>(std::vector<function_item>({
    function_item(ID_CREATE_MANAGER, "CreateManager"),
    function_item(ID_START, "Start"),
    function_item(ID_STOP, "Stop"),
    function_item(ID_POSTCHECK_OBLIGATIONS_RESPONSE, "PostCheckObligationsResponse"),
    function_item(ID_QUERY_PROCESSINFO, "QueryProcessInfo"),
    function_item(ID_IS_REQUEST_CANCELED, "isRequestCanceled"),
    function_item(ID_IS_PROCESS_ALIVE, "isProcessAlive"),
    function_item(ID_POST_OFFICE_GETCUSTOMUI_RESPONSE, "PostOfficeGetCustomUIResponse"),
    function_item(ID_INCREASE_POLICY_SERIALNO, "InreasePolicySerialNumber"),
    function_item(ID_POST_SAVEAS_FORECAST_RESPONSE, "PostSaveAsForecastResponse"),
    function_item(ID_POST_GET_CLASSIFY_UI_RESPONSE, "PostGetClassifyUIResponse"),
    function_item(ID_POST_BLOCK_NOTIFICATION_RESPONSE, "PostBlockNotificationResponse"),
    function_item(ID_POST_CHECK_PROTECT_MENU_RESPONSE, "PostCheckProtectMenuResponse"),
    function_item(ID_POST_GET_CONTEXT_MENU_REGEX_RESPONSE, "PostGetContextMenuRegexResponse"),
    function_item(ID_POST_UPDATE_CORE_CONTEXT_RESPONSE, "PostUpdateCoreContextResponse"),
    function_item(ID_POST_QUERY_CORE_CONTEXT_RESPONSE, "PostQueryCoreContextResponse"),
    function_item(ID_POST_QUERY_SERVICE_RESPONSE, "PostQueryServiceResponse"),
    function_item(ID_POST_QUERY_UPDATE_DWM_WINDOW_RESPONSE, "PostQueryUpdateDwmWindowResponse"),
    function_item(ID_POST_QUERY_UPDATE_OVERLAY_WINDOW_RESPONSE, "PostQueryUpdateOverlayWindowResponse"),
    function_item(ID_POST_CHECK_PROCESS_RIGHTS_RESPONSE, "PostCheckProcessRightsResponse"),
    function_item(ID_CLOSE_MANAGER, "CloseManager"),
    function_item(ID_ENABLE_ANTITAMPERING, "EnableAntiTampering"),
    function_item(ID_SET_OVERLAY_PROTECTED_WINDOWS, "SetOverlayProtectedWindows")
}))
{
}

drvcore_man_instance::~drvcore_man_instance()
{
}

void drvcore_man_instance::load(const std::wstring& dll_file)
{
    dll_instance::load(dll_file);
}

HANDLE drvcore_man_instance::create_manager(void* callback_notify, void* callback_log, void* callback_logaccept, void* context)
{
    typedef HANDLE (WINAPI* NXRMDRV_CREATE_MANAGER)(void*, void*, void*, void*);
    return EXECUTE(NXRMDRV_CREATE_MANAGER, *this, ID_CREATE_MANAGER, callback_notify, callback_log, callback_logaccept, context);
}

unsigned long drvcore_man_instance::start(HANDLE h)
{
    typedef unsigned long (WINAPI* NXRMDRV_START)(HANDLE);
    return EXECUTE(NXRMDRV_START, *this, ID_START, h);
}

unsigned long drvcore_man_instance::stop(HANDLE h)
{
    typedef unsigned long (WINAPI* NXRMDRV_STOP)(HANDLE);
    return EXECUTE(NXRMDRV_STOP, *this, ID_STOP, h);
}

unsigned long drvcore_man_instance::post_check_obligations_response(HANDLE h, void*context , void* obligations, unsigned long size)
{
    typedef unsigned long (WINAPI* NXRMDRV_POSTCHECK_OBLIGATIONS_RESPONSE)(HANDLE, void*, void*, unsigned long);
    return EXECUTE(NXRMDRV_POSTCHECK_OBLIGATIONS_RESPONSE, *this, ID_POSTCHECK_OBLIGATIONS_RESPONSE, h, context, obligations, size);
}

unsigned long drvcore_man_instance::query_process_info(HANDLE h, unsigned long process_id, void* info)
{
    typedef unsigned long (WINAPI* NXRMDRV_QUERY_PROCESSINFO)(HANDLE, unsigned long, void*);
    return EXECUTE(NXRMDRV_QUERY_PROCESSINFO, *this, ID_QUERY_PROCESSINFO, h, process_id, info);
}

unsigned long drvcore_man_instance::is_request_canceled(HANDLE h, void* context)
{
    typedef unsigned long (WINAPI* NXRMDRV_IS_REQUEST_CANCELED)(HANDLE, void*);
    return EXECUTE(NXRMDRV_IS_REQUEST_CANCELED, *this, ID_IS_REQUEST_CANCELED, h, context);
}

unsigned long drvcore_man_instance::is_process_alive(HANDLE h, unsigned long process_id)
{
    typedef unsigned long (WINAPI* NXRMDRV_IS_PROCESS_ALIVE)(HANDLE, unsigned long);
    return EXECUTE(NXRMDRV_IS_PROCESS_ALIVE, *this, ID_IS_PROCESS_ALIVE, h, process_id);
}

unsigned long drvcore_man_instance::post_office_get_custom_ui_response(HANDLE h, void* context, void* response)
{
    typedef unsigned long (WINAPI* NXRMDRV_POST_OFFICE_GETCUSTOMUI_RESPONSE)(HANDLE, PVOID, void*);
    return EXECUTE(NXRMDRV_POST_OFFICE_GETCUSTOMUI_RESPONSE, *this, ID_POST_OFFICE_GETCUSTOMUI_RESPONSE, h, context, response);
}

unsigned long drvcore_man_instance::increase_policy_serial_number(HANDLE h)
{
    typedef unsigned long (WINAPI* NXRMDRV_INCREASE_POLICY_SERIALNO)(HANDLE);
    return EXECUTE(NXRMDRV_INCREASE_POLICY_SERIALNO, *this, ID_INCREASE_POLICY_SERIALNO, h);
}

unsigned long drvcore_man_instance::post_save_as_forecast_response(HANDLE h, void* response)
{
    typedef unsigned long (WINAPI* NXRMDRV_POST_SAVEAS_FORECAST_RESPONSE)(HANDLE, void*);
    return EXECUTE(NXRMDRV_POST_SAVEAS_FORECAST_RESPONSE, *this, ID_POST_SAVEAS_FORECAST_RESPONSE, h, response);
}

unsigned long drvcore_man_instance::post_get_classify_ui_response(HANDLE h, void* context, void* response)
{
    typedef unsigned long (WINAPI* NXRMDRV_POST_GET_CLASSIFY_UI_RESPONSE)(HANDLE, void*, void*);
    return EXECUTE(NXRMDRV_POST_GET_CLASSIFY_UI_RESPONSE, *this, ID_POST_GET_CLASSIFY_UI_RESPONSE, h, context, response);
}

unsigned long drvcore_man_instance::post_block_notification_response(HANDLE h, void* response)
{
    typedef unsigned long (WINAPI* NXRMDRV_POST_BLOCK_NOTIFICATION_RESPONSE)(HANDLE, void*);
    return EXECUTE(NXRMDRV_POST_BLOCK_NOTIFICATION_RESPONSE, *this, ID_POST_BLOCK_NOTIFICATION_RESPONSE, h, response);
}

unsigned long drvcore_man_instance::post_check_protect_menu_response(HANDLE h, void* context, void* response)
{
    typedef unsigned long (WINAPI* NXRMDRV_POST_CHECK_PROTECT_MENU_RESPONSE)(HANDLE, void*, void*);
    return EXECUTE(NXRMDRV_POST_CHECK_PROTECT_MENU_RESPONSE, *this, ID_POST_CHECK_PROTECT_MENU_RESPONSE, h, context, response);
}

unsigned long drvcore_man_instance::post_get_context_menu_regex_response(HANDLE h, void* context, void* response)
{
    typedef unsigned long (WINAPI* NXRMDRV_POST_GET_CONTEXT_MENU_REGEX_RESPONSE)(HANDLE, void*, void*);
    return EXECUTE(NXRMDRV_POST_GET_CONTEXT_MENU_REGEX_RESPONSE, *this, ID_POST_GET_CONTEXT_MENU_REGEX_RESPONSE, h, context, response);
}

unsigned long drvcore_man_instance::post_update_core_context_response(HANDLE h, void* response)
{
    typedef unsigned long (WINAPI* NXRMDRV_POST_UPDATE_CORE_CONTEXT_RESPONSE)(HANDLE, void*);
    return EXECUTE(NXRMDRV_POST_UPDATE_CORE_CONTEXT_RESPONSE, *this, ID_POST_UPDATE_CORE_CONTEXT_RESPONSE, h, response);
}

unsigned long drvcore_man_instance::post_query_core_context_response(HANDLE h, void* context, void* response)
{
    typedef unsigned long (WINAPI* NXRMDRV_POST_QUERY_CORE_CONTEXT_RESPONSE)(HANDLE, void*, void*);
    return EXECUTE(NXRMDRV_POST_QUERY_CORE_CONTEXT_RESPONSE, *this, ID_POST_QUERY_CORE_CONTEXT_RESPONSE, h, context, response);
}

unsigned long drvcore_man_instance::post_query_service_response(HANDLE h, void* context, void* response)
{
    typedef unsigned long (WINAPI* NXRMDRV_POST_QUERY_SERVICE_RESPONSE)(HANDLE, void*, void*);
    return EXECUTE(NXRMDRV_POST_QUERY_SERVICE_RESPONSE, *this, ID_POST_QUERY_SERVICE_RESPONSE, h, context, response);
}

unsigned long drvcore_man_instance::post_query_update_dwm_window_response(HANDLE h, void* response)
{
    typedef unsigned long (WINAPI* NXRMDRV_POST_QUERY_UPDATE_DWM_WINDOW_RESPONSE)(HANDLE, void*);
    return EXECUTE(NXRMDRV_POST_QUERY_UPDATE_DWM_WINDOW_RESPONSE, *this, ID_POST_QUERY_UPDATE_DWM_WINDOW_RESPONSE, h, response);
}

unsigned long drvcore_man_instance::post_query_update_overlay_window_response(HANDLE h, void* response)
{
    typedef unsigned long (WINAPI* NXRMDRV_POST_QUERY_UPDATE_OVERLAY_WINDOW_RESPONSE)(HANDLE, void*);
    return EXECUTE(NXRMDRV_POST_QUERY_UPDATE_OVERLAY_WINDOW_RESPONSE, *this, ID_POST_QUERY_UPDATE_OVERLAY_WINDOW_RESPONSE, h, response);
}

unsigned long drvcore_man_instance::post_check_process_rights_response(HANDLE h, void* context, void* response)
{
    typedef unsigned long (WINAPI* NXRMDRV_POST_CHECK_PROCESS_RIGHTS_RESPONSE)(HANDLE, void*, void*);
    return EXECUTE(NXRMDRV_POST_CHECK_PROCESS_RIGHTS_RESPONSE, *this, ID_POST_CHECK_PROCESS_RIGHTS_RESPONSE, h, context, response);
}

unsigned long drvcore_man_instance::close_manager(HANDLE h)
{
    typedef unsigned long (WINAPI* NXRMDRV_CLOSE_MANAGER)(HANDLE);
    return EXECUTE(NXRMDRV_CLOSE_MANAGER, *this, ID_CLOSE_MANAGER, h);
}

unsigned long drvcore_man_instance::enable_antitampering(BOOL b)
{
    typedef unsigned long (WINAPI* NXRMDRV_ENABLE_ANTITAMPERING)(BOOL);
    return EXECUTE(NXRMDRV_ENABLE_ANTITAMPERING, *this, ID_ENABLE_ANTITAMPERING, b);
}

unsigned long drvcore_man_instance::set_overlay_protected_windows(HANDLE h, unsigned long session_id, unsigned long* hwnds, unsigned long* cb_size)
{
    typedef unsigned long (WINAPI* NXRMDRV_SET_OVERLAY_PROTECTED_WINDOWS)(HANDLE, unsigned long, unsigned long*, unsigned long*);
    return EXECUTE(NXRMDRV_SET_OVERLAY_PROTECTED_WINDOWS, *this, ID_SET_OVERLAY_PROTECTED_WINDOWS, h, session_id, hwnds, cb_size);
}

unsigned long drvcore_man_instance::set_overlay_bitmap_status(HANDLE h, unsigned long session_id, bool ready)
{
    typedef unsigned long (WINAPI* NXRMDRV_SET_OVERLAY_BITMAP_STATUS)(HANDLE, unsigned long, BOOL);
    return EXECUTE(NXRMDRV_SET_OVERLAY_BITMAP_STATUS, *this, ID_SET_OVERLAY_SET_STATUS, h, session_id, ready ? TRUE : FALSE);
}


//
//  drvcore_controller
//

drvcore_controller::drvcore_controller() : _h(NULL)
{
}

drvcore_controller::~drvcore_controller()
{
    disconnect();
}

bool drvcore_controller::connect()
{
    _h = init_transporter_client();
    return is_connected();
}

void drvcore_controller::disconnect()
{
    if (NULL != _h) {
        close_transporter_client(_h);
        _h = NULL;
    }
}

bool drvcore_controller::drvcore_is_good_version()
{
    if (!is_connected()) {
        throw WIN32_ERROR_MSG(ERROR_SERVICE_NOT_ACTIVE, "Not connect to nxrm driver yet");
    }
    return (is_good_version(_h) ? true : false);
}

bool drvcore_controller::drvcore_is_enabled()
{
    if (!is_connected()) {
        throw WIN32_ERROR_MSG(ERROR_SERVICE_NOT_ACTIVE, "Not connect to nxrm driver yet");
    }
    return (is_transporter_enabled(_h) ? true : false);
}

bool drvcore_controller::drvcore_is_channel_on(long channel_id)
{
    if (!is_connected()) {
        throw WIN32_ERROR_MSG(ERROR_SERVICE_NOT_ACTIVE, "Not connect to nxrm driver yet");
    }
    return (is_xxx_channel_on(_h, channel_id) ? true : false);
}

bool drvcore_controller::drvcore_is_process_protected(unsigned long process_id)
{
    if (!is_connected()) {
        throw WIN32_ERROR_MSG(ERROR_SERVICE_NOT_ACTIVE, "Not connect to nxrm driver yet");
    }
    return (is_protected_process(_h, (0 == process_id) ? GetCurrentProcessId() : process_id) ? true : false);
}

bool drvcore_controller::drvcore_is_overlay_bitmap_ready()
{
    if (!is_connected()) {
        throw WIN32_ERROR_MSG(ERROR_SERVICE_NOT_ACTIVE, "Not connect to nxrm driver yet");
    }
    return (is_overlay_bitmap_ready(_h) ? true : false);
}

unsigned long drvcore_controller::drvcore_get_policy_serial_number()
{
    if (!is_connected()) {
        throw WIN32_ERROR_MSG(ERROR_SERVICE_NOT_ACTIVE, "Not connect to nxrm driver yet");
    }
    return (unsigned long)get_rm_policy_sn(_h);
}

std::vector<HWND> drvcore_controller::drvcore_get_overlay_windows()
{
    std::vector<unsigned long> buf;
    std::vector<HWND> hwnds;
    unsigned long size = NXRM_MAX_DWM_WND;

    buf.resize(NXRM_MAX_DWM_WND, 0);
    get_overlay_windows(_h, buf.data(), &size);
    for (int i = 0; i < (int)size; i++) {
        hwnds.push_back((HWND)(ULONG_PTR)buf[i]);
    }

    return std::move(hwnds);
}

void drvcore_controller::drvcore_set_dwm_active_session(unsigned long session_id)
{
    if (!is_connected()) {
        throw WIN32_ERROR_MSG(ERROR_SERVICE_NOT_ACTIVE, "Not connect to nxrm driver yet");
    }
    if (session_id == 0 || session_id == 0xFFFFFFFF) {
        throw WIN32_ERROR_MSG(ERROR_INVALID_PARAMETER, "Invalid session id");
    }
    
    set_dwm_active_session(_h, session_id);
}

std::vector<unsigned char> drvcore_controller::send_request(unsigned long type, std::vector<unsigned char>& request_data, bool wait)
{
    if (!is_connected()) {
        throw WIN32_ERROR_MSG(ERROR_SERVICE_NOT_ACTIVE, "Not connect to nxrm driver yet");
    }

    HANDLE request = submit_request(_h, type, request_data.data(), (unsigned long)request_data.size());
    if (NULL == request) {
        throw WIN32_ERROR_MSG(GetLastError(), "fail to submit request");
    }
    if (!wait) {
        return std::vector<unsigned char>();
    }

    std::vector<unsigned char> response_data;
    unsigned long bytes_returned = 0;

    response_data.resize(4096, 0);
    if (!wait_for_response(request, _h, response_data.data(), (unsigned long)response_data.size(), &bytes_returned)) {
        throw WIN32_ERROR_MSG(GetLastError(), "fail to submit request");
    }

    if (bytes_returned != 4096) {
        response_data.resize(bytes_returned);
    }

    return std::move(response_data);
}

void drvcore_controller::send_notify(unsigned long type, std::vector<unsigned char>& request_data)
{
    if (!is_connected()) {
        throw WIN32_ERROR_MSG(ERROR_SERVICE_NOT_ACTIVE, "Not connect to nxrm driver yet");
    }

    if (!submit_notify(_h, type, request_data.data(), (unsigned long)request_data.size())) {
        throw WIN32_ERROR_MSG(ERROR_SERVICE_NOT_ACTIVE, "fail to submit notification");
    }
}