

#include <Windows.h>
#include <assert.h>

#include <nudf\eh.hpp>
#include <nudf\debug.hpp>
#include <nudf\string.hpp>

// from drvman
#include "nxrmdrv.h"
#include "nxrmdrvman.h"

#include "nxrmserv.hpp"
#include "global.hpp"
#include "serv.hpp"
#include "serv_core.hpp"



//
//  class drvcore_serv::drv_request
//

drvcore_serv::drv_request::drv_request(unsigned long type, void* msg, unsigned long length, void* msg_context)
{
    unsigned long required_size = 0;

    switch (type)
    {
    case NXRMDRV_MSG_TYPE_CHECKOBLIGATION:
        required_size = (unsigned long)sizeof(CHECK_OBLIGATION_REQUEST);
        break;
    case NXRMDRV_MSG_TYPE_GET_CUSTOMUI:
        required_size = (unsigned long)sizeof(OFFICE_GET_CUSTOMUI_REQUEST);
        break;
    case NXRMDRV_MSG_TYPE_SAVEAS_FORECAST:
        required_size = (unsigned long)sizeof(SAVEAS_FORECAST_REQUEST);
        break;
    case NXRMDRV_MSG_TYPE_GET_CLASSIFYUI:
        required_size = (unsigned long)sizeof(GET_CLASSIFY_UI_REQUEST);
        break;
    case NXRMDRV_MSG_TYPE_BLOCK_NOTIFICATION:
        required_size = (unsigned long)sizeof(BLOCK_NOTIFICATION_REQUEST);
        break;
    case NXRMDRV_MSG_TYPE_CHECK_PROTECT_MENU:
        required_size = (unsigned long)sizeof(CHECK_PROTECT_MENU_REQUEST);
        break;
    case NXRMDRV_MSG_TYPE_UPDATE_PROTECTEDMODEAPPINFO:
        required_size = (unsigned long)sizeof(UPDATE_PROTECTEDMODEAPPINFO_REQUEST);
        break;
    case NXRMDRV_MSG_TYPE_QUERY_PROTECTEDMODEAPPINFO:
        required_size = (unsigned long)sizeof(QUERY_PROTECTEDMODEAPPINFO_REQUEST);
        break;
    case NXRMDRV_MSG_TYPE_GET_CTXMENUREGEX:
        required_size = (unsigned long)sizeof(QUERY_CTXMENUREGEX_REQUEST);
        break;
    case NXRMDRV_MSG_TYPE_UPDATE_CORE_CTX:
        required_size = (unsigned long)sizeof(UPDATE_CORE_CTX_REQUEST);
        break;
    case NXRMDRV_MSG_TYPE_QUERY_CORE_CTX:
        required_size = (unsigned long)sizeof(QUERY_CORE_CTX_REQUEST);
        break;
    case NXRMDRV_MSG_TYPE_QUERY_SERVICE:
        required_size = (unsigned long)sizeof(QUERY_SERVICE_REQUEST);
        break;
    case NXRMDRV_MSG_TYPE_UPDATE_DWM_WINDOW:
        required_size = (unsigned long)sizeof(UPDATE_DWM_WINDOW_REQUEST);
        break;
    case NXRMDRV_MSG_TYPE_UPDATE_OVERLAY_WINDOW:
        required_size = (unsigned long)sizeof(UPDATE_OVERLAY_WINDOW_REQUEST);
        break;
    case NXRMDRV_MSG_TYPE_CHECK_PROCESS_RIGHTS:
        required_size = (unsigned long)sizeof(CHECK_PROCESS_RIGHTS_REQUEST);
        break;
    default:
        break;
    }

    if (0 == required_size) {
        SetLastError(ERROR_INVALID_PARAMETER);
        LOGERROR(NX::string_formater(L"Unknown drv_core request type (%d)", type));
    }

    _request.resize(required_size, 0);
    memcpy(&_request[0], msg, min(required_size, length));
    _type    = type;
    _context = msg_context;
}



//
//  class drvcore_serv
//

drvcore_serv::drvcore_serv() : _running(false)
{
    ::InitializeCriticalSection(&_request_list_lock);
    _request_events[0] = ::CreateEventW(NULL, TRUE, FALSE, NULL);   // stop event
    _request_events[1] = ::CreateEventW(NULL, TRUE, FALSE, NULL);   // request coming event
}

drvcore_serv::~drvcore_serv()
{
    stop();

    if (NULL != _request_events[0]) {
        CloseHandle(_request_events[0]);
        _request_events[0] = NULL;
    }
    if (NULL != _request_events[1]) {
        CloseHandle(_request_events[1]);
        _request_events[1] = NULL;
    }
    ::DeleteCriticalSection(&_request_list_lock);
}

void drvcore_serv::start()
{
    static const std::wstring drvman_dll_file(GLOBAL.get_bin_dir() + L"\\nxrmdrvman.dll");

    try {

        unsigned long result = 0;

        // load dll
        _dll.load(drvman_dll_file);

        // create manager
        _h = _dll.create_manager(drvcore_serv::drv_callback, NxGlobalLogWrite, NxGlobalLogAccept, this);
        if (_h == NULL) {            
            throw NX::exception(ERROR_MSG2("DRVSERV", "fail to create drvman manager (%d)", GetLastError()));
        }

        // start nxrmdrv manager
        result = _dll.start(_h);
        if (0 != result) {
            CloseHandle(_h);
            _h = NULL;
            throw NX::exception(ERROR_MSG2("DRVSERV", "fail to start drvman manager (%d)", result));
        }

        // start worker thread
        int nthreads = 0;
        for (int i = 0; i < 4; i++) {
            try {
                _threads.push_back(std::thread(drvcore_serv::worker, this));
                nthreads++;
            }
            catch (std::exception& e) {
                LOGWARNING(ERROR_MSG2("DRVSERV", "fail to start worker thread #%d (%s)", i, e.what()));
            }
        }

        if (0 == nthreads) {
            throw NX::exception(ERROR_MSG2("DRVSERV", "none worker thread has been started"));
        }

        _running = true;
        LOGINFO(NX::string_formater(L"DRVSERV: %d worker threads have been started", nthreads));
    }
    catch (std::exception& e) {
        if (_h != NULL) {
            _dll.stop(_h);
            CloseHandle(_h);
            _h = NULL;
        }
        LOGERROR(e.what());
    }
}

void drvcore_serv::stop()
{
    if (!_running) {
        return;
    }

    assert(_running);
    _running = false;

    // stop all the request handler threads
    SetEvent(_request_events[0]);
    std::for_each(_threads.begin(), _threads.end(), [&](std::thread& t) {
        if (t.joinable()) {
            t.join();
        }
    });
    _threads.clear();
    ResetEvent(_request_events[0]);

    // stop drvman manager
    _dll.stop(_h);
    CloseHandle(_h);
    _h = NULL;

    // unload dll
    _dll.unload();
}

void drvcore_serv::worker(drvcore_serv* serv) noexcept
{
    while (serv->is_running()) {

        // wait
        unsigned wait_result = ::WaitForMultipleObjects(2, serv->_request_events, FALSE, INFINITE);

        if (wait_result == WAIT_OBJECT_0) {
            // stop event
            break;
        }

        //
        if (wait_result != (WAIT_OBJECT_0 + 1)) {
            // error
            break;
        }

        assert(wait_result == (WAIT_OBJECT_0 + 1));

        // new request comes
        std::shared_ptr<drvcore_serv::drv_request>  request;
        ::EnterCriticalSection(&serv->_request_list_lock);
        if (!serv->_request_list.empty()) {
            request = serv->_request_list.front();
            assert(request != nullptr);
            serv->_request_list.pop();
        }
        ::LeaveCriticalSection(&serv->_request_list_lock);

        // handle request
        try {

            switch (request->type())
            {
            case NXRMDRV_MSG_TYPE_CHECKOBLIGATION:
                serv->on_check_obligations(request.get());
                break;
            case NXRMDRV_MSG_TYPE_GET_CUSTOMUI:
                serv->on_get_custom_ui(request.get());
                break;
            case NXRMDRV_MSG_TYPE_SAVEAS_FORECAST:
                serv->on_save_as_forecast(request.get());
                break;
            case NXRMDRV_MSG_TYPE_GET_CLASSIFYUI:
                serv->on_get_classify_ui(request.get());
                break;
            case NXRMDRV_MSG_TYPE_BLOCK_NOTIFICATION:
                serv->on_block_notification(request.get());
                break;
            case NXRMDRV_MSG_TYPE_CHECK_PROTECT_MENU:
                serv->on_check_protect_menu(request.get());
                break;
            case NXRMDRV_MSG_TYPE_UPDATE_PROTECTEDMODEAPPINFO:
                serv->on_update_protected_mode_app_info(request.get());
                break;
            case NXRMDRV_MSG_TYPE_QUERY_PROTECTEDMODEAPPINFO:
                serv->on_query_protected_mode_app_info(request.get());
                break;
            case NXRMDRV_MSG_TYPE_GET_CTXMENUREGEX:
                serv->on_get_context_menu_regex(request.get());
                break;
            case NXRMDRV_MSG_TYPE_UPDATE_CORE_CTX:
                serv->on_update_core_context(request.get());
                break;
            case NXRMDRV_MSG_TYPE_QUERY_CORE_CTX:
                serv->on_query_core_context(request.get());
                break;
            case NXRMDRV_MSG_TYPE_QUERY_SERVICE:
                serv->on_query_service(request.get());
                break;
            case NXRMDRV_MSG_TYPE_UPDATE_DWM_WINDOW:
                serv->on_update_dwm_window(request.get());
                break;
            case NXRMDRV_MSG_TYPE_UPDATE_OVERLAY_WINDOW:
                serv->on_update_overlay_window(request.get());
                break;
            case NXRMDRV_MSG_TYPE_CHECK_PROCESS_RIGHTS:
                serv->on_check_process_rights(request.get());
                break;
            default:
                // unknown type
                throw NX::exception(ERROR_MSG("DRVSERV", "Unknown drvman request %08X", request->type()));
                break;
            }
        }
        catch (std::exception& e) {
            LOGWARNING(e.what());
        }
        catch (NX::structured_exception& e) {
            LOGCRITICAL(e.exception_message());
        }
        catch (...) {
            LOGCRITICAL(ERROR_MSG("DRVSERV", "Unknown exception when processing request"));
        }
    }
}

unsigned long __stdcall drvcore_serv::drv_callback(unsigned long type, void* msg, unsigned long length, void* msg_context, void* user_context)
{
    unsigned long result = 0;
    drvcore_serv* serv = (drvcore_serv*)user_context;
    
    std::shared_ptr<drv_request> request(new drv_request(type, msg, length, msg_context));
    if (request != NULL && !request->empty()) {
        ::EnterCriticalSection(&serv->_request_list_lock);
        serv->_request_list.push(request);
        SetEvent(serv->_request_events[1]);
        ::LeaveCriticalSection(&serv->_request_list_lock);
    }
    else {
        result = GetLastError();
        if (0 == result) result = ERROR_INVALID_PARAMETER;
    }

    return result;
}

void drvcore_serv::on_check_obligations(drv_request* request)
{
    const CHECK_OBLIGATION_REQUEST* req = (const CHECK_OBLIGATION_REQUEST*)request->request();

    assert(request->type() == NXRMDRV_MSG_TYPE_CHECKOBLIGATION);

    const bool request_canceled = (0 != _dll.is_request_canceled(_h, request->context()));

    LOGDETAIL(NX::string_formater(L"DRVSERV Request: CHECK_OBLIGATION%s", request_canceled ? L" (CANCELED)" : L" "));
    LOGDETAIL(NX::string_formater(L"  ProcessId:    %d", req->ProcessId));
    LOGDETAIL(NX::string_formater(L"  ThreadId:     %d", req->ThreadId));
    LOGDETAIL(NX::string_formater(L"  EvaluationId: %d", req->EvaluationId));
    LOGDETAIL(NX::string_formater(L"  FileName:     %s", req->FileName));
    LOGDETAIL(NX::string_formater(L"  TempPath:     %s", req->TempPath));
}

void drvcore_serv::on_get_custom_ui(drv_request* request)
{
    const OFFICE_GET_CUSTOMUI_REQUEST* req = (const OFFICE_GET_CUSTOMUI_REQUEST*)request->request();

    assert(request->type() == NXRMDRV_MSG_TYPE_GET_CUSTOMUI);

    const bool request_canceled = (0 != _dll.is_request_canceled(_h, request->context()));

    LOGDETAIL(NX::string_formater(L"DRVSERV Request: GET_CUSTOMUI%s", request_canceled ? L" (CANCELED)" : L" "));
    LOGDETAIL(NX::string_formater(L"  ProcessId:      %d", req->ProcessId));
    LOGDETAIL(NX::string_formater(L"  ThreadId:       %d", req->ThreadId));
    LOGDETAIL(NX::string_formater(L"  OfficeVersion:  %s", req->OfficeVersion));
    LOGDETAIL(NX::string_formater(L"  OfficeLanguage: %04X", req->OfficeLanguageId));
    LOGDETAIL(NX::string_formater(L"  OfficeProduct:  %04X", req->OfficeProduct));
    LOGDETAIL(NX::string_formater(L"  TempPath:       %s", req->TempPath));
}

void drvcore_serv::on_save_as_forecast(drv_request* request)
{
    const SAVEAS_FORECAST_REQUEST* req = (const SAVEAS_FORECAST_REQUEST*)request->request();

    assert(request->type() == NXRMDRV_MSG_TYPE_SAVEAS_FORECAST);

    const bool request_canceled = (0 != _dll.is_request_canceled(_h, request->context()));

    LOGDETAIL(NX::string_formater(L"DRVSERV Request: SAVEAS_FORECAST%s", request_canceled ? L" (CANCELED)" : L" "));
    LOGDETAIL(NX::string_formater(L"  ProcessId:%d", req->ProcessId));
    LOGDETAIL(NX::string_formater(L"  ThreadId: %d", req->ThreadId));
    LOGDETAIL(NX::string_formater(L"  Source:   %s", req->SourceFileName));
    LOGDETAIL(NX::string_formater(L"  Target:   %s", req->SaveAsFileName));
}

void drvcore_serv::on_get_classify_ui(drv_request* request)
{
    const GET_CLASSIFY_UI_REQUEST* req = (const GET_CLASSIFY_UI_REQUEST*)request->request();

    assert(request->type() == NXRMDRV_MSG_TYPE_GET_CLASSIFYUI);

    const bool request_canceled = (0 != _dll.is_request_canceled(_h, request->context()));

    LOGDETAIL(NX::string_formater(L"DRVSERV Request: GET_CLASSIFYUI%s", request_canceled ? L" (CANCELED)" : L" "));
    LOGDETAIL(NX::string_formater(L"  ProcessId:    %d", req->ProcessId));
    LOGDETAIL(NX::string_formater(L"  ThreadId:     %d", req->ThreadId));
    LOGDETAIL(NX::string_formater(L"  SessionId:    %d", req->SessionId));
    LOGDETAIL(NX::string_formater(L"  LanguageId:   %04X", req->LanguageId));
    LOGDETAIL(NX::string_formater(L"  TempPath:     %s", req->TempPath));
}

void drvcore_serv::on_block_notification(drv_request* request)
{
    const BLOCK_NOTIFICATION_REQUEST* req = (const BLOCK_NOTIFICATION_REQUEST*)request->request();

    assert(request->type() == NXRMDRV_MSG_TYPE_BLOCK_NOTIFICATION);

    const bool request_canceled = (0 != _dll.is_request_canceled(_h, request->context()));

    LOGDETAIL(NX::string_formater(L"DRVSERV Request: BLOCK_NOTIFICATION%s", request_canceled ? L" (CANCELED)" : L" "));
    LOGDETAIL(NX::string_formater(L"  ProcessId:    %d", req->ProcessId));
    LOGDETAIL(NX::string_formater(L"  ThreadId:     %d", req->ThreadId));
    LOGDETAIL(NX::string_formater(L"  SessionId:    %d", req->SessionId));
    LOGDETAIL(NX::string_formater(L"  NoteType:     %d", req->Type));
    LOGDETAIL(NX::string_formater(L"  FileName1:    %s", req->FileName));
    LOGDETAIL(NX::string_formater(L"  FileName2:    %s", req->FileName2));
}

void drvcore_serv::on_check_protect_menu(drv_request* request)
{
    const CHECK_PROTECT_MENU_REQUEST* req = (const CHECK_PROTECT_MENU_REQUEST*)request->request();

    assert(request->type() == NXRMDRV_MSG_TYPE_CHECK_PROTECT_MENU);

    const bool request_canceled = (0 != _dll.is_request_canceled(_h, request->context()));

    LOGDETAIL(NX::string_formater(L"DRVSERV Request: CHECK_PROTECT_MENU%s", request_canceled ? L" (CANCELED)" : L" "));
    LOGDETAIL(NX::string_formater(L"  ProcessId:    %d", req->ProcessId));
    LOGDETAIL(NX::string_formater(L"  ThreadId:     %d", req->ThreadId));
    LOGDETAIL(NX::string_formater(L"  SessionId:    %d", req->SessionId));
}

void drvcore_serv::on_update_protected_mode_app_info(drv_request* request)
{
    const UPDATE_PROTECTEDMODEAPPINFO_REQUEST* req = (const UPDATE_PROTECTEDMODEAPPINFO_REQUEST*)request->request();
    UPDATE_PROTECTEDMODEAPPINFO_RESPONSE       response = { 0 };

    memset(&response, 0, sizeof(response));

    assert(request->type() == NXRMDRV_MSG_TYPE_UPDATE_PROTECTEDMODEAPPINFO);

    const bool request_canceled = (0 != _dll.is_request_canceled(_h, request->context()));

    LOGDETAIL(NX::string_formater(L"DRVSERV Request: UPDATE_PROTECTEDMODEAPPINFO%s", request_canceled ? L" (CANCELED)" : L" "));
    LOGDETAIL(NX::string_formater(L"  ProcessId:    %d", req->ProcessId));
    LOGDETAIL(NX::string_formater(L"  ThreadId:     %d", req->ThreadId));
    LOGDETAIL(NX::string_formater(L"  SessionId:    %d", req->SessionId));
    LOGDETAIL(NX::string_formater(L"  EvaluationId: %d", req->EvaluationId));
    LOGDETAIL(NX::string_formater(L"  RightsMask:   %08X%08X", (unsigned long)(req->RightsMask >> 32), (unsigned long)req->RightsMask));
    LOGDETAIL(NX::string_formater(L"  CustomRights: %08X%08X", (unsigned long)(req->CustomRights >> 32), (unsigned long)req->CustomRights));
    LOGDETAIL(NX::string_formater(L"  ActiveDocFileName: %s", req->ActiveDocFileName));
}

void drvcore_serv::on_query_protected_mode_app_info(drv_request* request)
{
    const QUERY_PROTECTEDMODEAPPINFO_REQUEST* req = (const QUERY_PROTECTEDMODEAPPINFO_REQUEST*)request->request();
    QUERY_PROTECTEDMODEAPPINFO_RESPONSE       response = { 0 };

    memset(&response, 0, sizeof(response));

    assert(request->type() == NXRMDRV_MSG_TYPE_QUERY_PROTECTEDMODEAPPINFO);

    const bool request_canceled = (0 != _dll.is_request_canceled(_h, request->context()));

    LOGDETAIL(NX::string_formater(L"DRVSERV Request: QUERY_PROTECTEDMODEAPPINFO%s", request_canceled ? L" (CANCELED)" : L" "));
    LOGDETAIL(NX::string_formater(L"  ProcessId:    %d", req->ProcessId));
    LOGDETAIL(NX::string_formater(L"  ThreadId:     %d", req->ThreadId));
    LOGDETAIL(NX::string_formater(L"  SessionId:    %d", req->SessionId));
}

void drvcore_serv::on_get_context_menu_regex(drv_request* request)
{
    const QUERY_CTXMENUREGEX_REQUEST* req = (const QUERY_CTXMENUREGEX_REQUEST*)request->request();

    assert(request->type() == NXRMDRV_MSG_TYPE_GET_CTXMENUREGEX);

    const bool request_canceled = (0 != _dll.is_request_canceled(_h, request->context()));

    LOGDETAIL(NX::string_formater(L"DRVSERV Request: GET_CTXMENUREGEX%s", request_canceled ? L" (CANCELED)" : L" "));
    LOGDETAIL(NX::string_formater(L"  ProcessId:    %d", req->ProcessId));
    LOGDETAIL(NX::string_formater(L"  ThreadId:     %d", req->ThreadId));
    LOGDETAIL(NX::string_formater(L"  SessionId:    %d", req->SessionId));
}

void drvcore_serv::on_update_core_context(drv_request* request)
{
    const UPDATE_CORE_CTX_REQUEST* req = (const UPDATE_CORE_CTX_REQUEST*)request->request();

    assert(request->type() == NXRMDRV_MSG_TYPE_UPDATE_CORE_CTX);

    const bool request_canceled = (0 != _dll.is_request_canceled(_h, request->context()));

    LOGDETAIL(NX::string_formater(L"DRVSERV Request: UPDATE_CORE_CTX%s", request_canceled ? L" (CANCELED)" : L" "));
    LOGDETAIL(NX::string_formater(L"  ProcessId:    %d", req->ProcessId));
    LOGDETAIL(NX::string_formater(L"  ThreadId:     %d", req->ThreadId));
    LOGDETAIL(NX::string_formater(L"  SessionId:    %d", req->SessionId));
    LOGDETAIL(NX::string_formater(L"  ModulePath:   %s", req->ModuleFullPath));
    LOGDETAIL(NX::string_formater(L"  RightsChecksum: %08X%08X", (unsigned long)(req->ModuleChecksum >> 32), (unsigned long)req->ModuleChecksum));
}

void drvcore_serv::on_query_core_context(drv_request* request)
{
    const QUERY_CORE_CTX_REQUEST* req = (const QUERY_CORE_CTX_REQUEST*)request->request();

    assert(request->type() == NXRMDRV_MSG_TYPE_QUERY_CORE_CTX);

    const bool request_canceled = (0 != _dll.is_request_canceled(_h, request->context()));

    LOGDETAIL(NX::string_formater(L"DRVSERV Request: QUERY_CORE_CTX%s", request_canceled ? L" (CANCELED)" : L" "));
    LOGDETAIL(NX::string_formater(L"  ProcessId:    %d", req->ProcessId));
    LOGDETAIL(NX::string_formater(L"  ThreadId:     %d", req->ThreadId));
    LOGDETAIL(NX::string_formater(L"  SessionId:    %d", req->SessionId));
    LOGDETAIL(NX::string_formater(L"  ModulePath:   %s", req->ModuleFullPath));
    LOGDETAIL(NX::string_formater(L"  RightsChecksum: %08X%08X", (unsigned long)(req->ModuleChecksum >> 32), (unsigned long)req->ModuleChecksum));
}

void drvcore_serv::on_query_service(drv_request* request)
{
    const QUERY_SERVICE_REQUEST* req = (const QUERY_SERVICE_REQUEST*)request->request();

    assert(request->type() == NXRMDRV_MSG_TYPE_QUERY_SERVICE);

    const bool request_canceled = (0 != _dll.is_request_canceled(_h, request->context()));

    LOGDETAIL(NX::string_formater(L"DRVSERV Request: QUERY_SERVICE%s", request_canceled ? L" (CANCELED)" : L" "));
    LOGDETAIL(NX::string_formater(L"  ProcessId:    %d", req->ProcessId));
    LOGDETAIL(NX::string_formater(L"  ThreadId:     %d", req->ThreadId));
    LOGDETAIL(NX::string_formater(L"  SessionId:    %d", req->SessionId));
    LOGDETAIL(NX::string_formater(L"  Query:        %S", req->Data));
}

void drvcore_serv::on_update_dwm_window(drv_request* request)
{
    const UPDATE_DWM_WINDOW_REQUEST* req = (const UPDATE_DWM_WINDOW_REQUEST*)request->request();

    assert(request->type() == NXRMDRV_MSG_TYPE_UPDATE_DWM_WINDOW);

    const bool request_canceled = (0 != _dll.is_request_canceled(_h, request->context()));

    LOGDETAIL(NX::string_formater(L"DRVSERV Request: UPDATE_DWM_WINDOW%s", request_canceled ? L" (CANCELED)" : L" "));
    LOGDETAIL(NX::string_formater(L"  ProcessId:    %d", req->ProcessId));
    LOGDETAIL(NX::string_formater(L"  ThreadId:     %d", req->ThreadId));
    LOGDETAIL(NX::string_formater(L"  SessionId:    %d", req->SessionId));
    LOGDETAIL(NX::string_formater(L"  HWND:         %08X", req->hWnd));
    LOGDETAIL(NX::string_formater(L"  Operation:    %s", (0 == req->Op) ? L"Add" : L"Delete"));
}

void drvcore_serv::on_update_overlay_window(drv_request* request)
{
    const UPDATE_OVERLAY_WINDOW_REQUEST* req = (const UPDATE_OVERLAY_WINDOW_REQUEST*)request->request();

    assert(request->type() == NXRMDRV_MSG_TYPE_UPDATE_OVERLAY_WINDOW);

    const bool request_canceled = (0 != _dll.is_request_canceled(_h, request->context()));

    LOGDETAIL(NX::string_formater(L"DRVSERV Request: UPDATE_OVERLAY_WINDOW%s", request_canceled ? L" (CANCELED)" : L" "));
    LOGDETAIL(NX::string_formater(L"  ProcessId:    %d", req->ProcessId));
    LOGDETAIL(NX::string_formater(L"  ThreadId:     %d", req->ThreadId));
    LOGDETAIL(NX::string_formater(L"  SessionId:    %d", req->SessionId));
    LOGDETAIL(NX::string_formater(L"  HWND:         %08X", req->hWnd));
    LOGDETAIL(NX::string_formater(L"  Operation:    %s", (0 == req->Op) ? L"Add" : L"Delete"));
}

void drvcore_serv::on_check_process_rights(drv_request* request)
{
    const CHECK_PROCESS_RIGHTS_REQUEST* req = (const CHECK_PROCESS_RIGHTS_REQUEST*)request->request();
    CHECK_PROCESS_RIGHTS_RESPONSE response = { 0 };

    assert(request->type() == NXRMDRV_MSG_TYPE_CHECK_PROCESS_RIGHTS);
    memset(&response, 0, sizeof(response));

    const bool request_canceled = (0 != _dll.is_request_canceled(_h, request->context()));

    LOGDETAIL(NX::string_formater(L"DRVSERV Request: UPDATE_OVERLAY_WINDOW%s", request_canceled ? L" (CANCELED)" : L" "));
    LOGDETAIL(NX::string_formater(L"  SessionId:    %d", req->SessionId));
    LOGDETAIL(NX::string_formater(L"  ProcessId:    %d", req->ProcessId));

    // finally
    _dll.post_check_process_rights_response(_h, request->context(), &response);
}
