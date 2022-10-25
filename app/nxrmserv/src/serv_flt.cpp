
#include <Windows.h>
#include <assert.h>

#include <nudf\eh.hpp>
#include <nudf\debug.hpp>
#include <nudf\string.hpp>
#include <nudf\filesys.hpp>

// from fltman
#include "nxrmflt.h"
#include "nxrmfltman.h"

#include "nxrmserv.hpp"
#include "global.hpp"
#include "serv.hpp"
#include "serv_flt.hpp"



//
//  class drvflt_serv::drv_request
//

drvflt_serv::flt_request::flt_request(unsigned long type, void* msg, unsigned long length, void* msg_context) : _type(0), _context(NULL)
{
    unsigned long required_size = 0;

    // check size
    switch (type)
    {
    case NXRMFLT_MSG_TYPE_CHECK_RIGHTS:
        if (length < sizeof(NXRM_CHECK_RIGHTS_NOTIFICATION)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            LOGERROR(NX::string_formater(L"Invalid drv_flt request size: %d != sizeof(NXRM_CHECK_RIGHTS_NOTIFICATION)", length));
            return;
        }
        break;
    case NXRMFLT_MSG_TYPE_BLOCK_NOTIFICATION:
        if (length < sizeof(NXRM_BLOCK_NOTIFICATION)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            LOGERROR(NX::string_formater(L"Invalid drv_flt request size: %d != sizeof(NXRM_BLOCK_NOTIFICATION)", length));
            return;
        }
        break;
    case NXRMFLT_MSG_TYPE_KEYCHAIN_ERROR_NOTIFICATION:
        if (length < sizeof(NXRM_KEYCHAIN_ERROR_NOTIFICATION)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            LOGERROR(NX::string_formater(L"Invalid drv_flt request size: %d != sizeof(NXRM_KEYCHAIN_ERROR_NOTIFICATION)", length));
            return;
        }
        break;
    case NXRMFLT_MSG_TYPE_PURGE_CACHE_NOTIFICATION:
        if (length < sizeof(NXRM_PURGE_CACHE_NOTIFICATION)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            LOGERROR(NX::string_formater(L"Invalid drv_flt request size: %d != sizeof(NXRM_PURGE_CACHE_NOTIFICATION)", length));
            return;
        }
        break;
    case NXRMFLT_MSG_TYPE_PROCESS_NOTIFICATION:
        if (length < sizeof(NXRM_PROCESS_NOTIFICATION)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            LOGERROR(NX::string_formater(L"Invalid drv_flt request size: %d != sizeof(NXRM_PROCESS_NOTIFICATION)", length));
            return;
        }
        break;
    default:
        LOGERROR(NX::string_formater(L"Unknown drv_flt request type (%d)", type));
        break;
    }

    _request.resize(length, 0);
    memcpy(&_request[0], msg, length);
    _type = type;
    _context = msg_context;
}


//
//
//

drvflt_serv::drvflt_serv() : _h(NULL), _running(false)
{
    ::InitializeCriticalSection(&_queue_lock);
    _request_events[0] = ::CreateEventW(NULL, TRUE, FALSE, NULL);   // stop event
    _request_events[1] = ::CreateEventW(NULL, TRUE, FALSE, NULL);   // request coming event
}

drvflt_serv::~drvflt_serv()
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
    ::DeleteCriticalSection(&_queue_lock);
}

void drvflt_serv::start(const void* key_blob, unsigned long size) noexcept
{
    static const std::wstring drvman_dll_file(GLOBAL.get_bin_dir() + L"\\nxrmfltman.dll");

    try {

        unsigned long result = 0;

        // load dll
        _dll.load(drvman_dll_file);

        // create manager
        _h = _dll.create_manager(drvflt_serv::drv_callback, NxGlobalLogWrite, NxGlobalLogAccept, (void*)key_blob, size, this);
        if (_h == NULL) {
            throw NX::exception(ERROR_MSG2("FLTSERV", "fail to create fltman manager (%d)", GetLastError()));
        }

        // start nxrmdrv manager
        result = _dll.start_filtering(_h);
        if (0 != result) {
            CloseHandle(_h);
            _h = NULL;
            throw NX::exception(ERROR_MSG2("FLTSERV", "fail to start fltman manager (%d)", result));
        }

        // start worker thread
        int nthreads = 0;
        for (int i = 0; i < 4; i++) {
            try {
                _threads.push_back(std::thread(drvflt_serv::worker, this));
                nthreads++;
            }
            catch (std::exception& e) {
                LOGWARNING(ERROR_MSG2("FLTSERV", "fail to start worker thread #%d (%s)", i, e.what()));
            }
        }

        if (0 == nthreads) {
            throw NX::exception(ERROR_MSG2("FLTSERV", "none worker thread has been started"));
        }

        _running = true;
        LOGINFO(NX::string_formater(L"FLTSERV: %d worker threads have been started", nthreads));
    }
    catch (std::exception& e) {
        if (_h != NULL) {
            _dll.stop_filtering(_h);
            CloseHandle(_h);
            _h = NULL;
        }
        LOGERROR(e.what());
    }
}

void drvflt_serv::stop() noexcept
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
    _dll.stop_filtering(_h);
    CloseHandle(_h);
    _h = NULL;

    // unload dll
    _dll.unload();
}

void drvflt_serv::worker(drvflt_serv* serv) noexcept
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
        std::shared_ptr<drvflt_serv::flt_request>  request;
        ::EnterCriticalSection(&serv->_queue_lock);
        if (!serv->_request_queue.empty()) {
            request = serv->_request_queue.front();
            assert(request != nullptr);
            serv->_request_queue.pop();
        }
        ::LeaveCriticalSection(&serv->_queue_lock);

        // handle request
        try {

            switch (request->type())
            {
            case NXRMFLT_MSG_TYPE_CHECK_RIGHTS:
                serv->on_check_rights(request.get());
                break;
            case NXRMFLT_MSG_TYPE_BLOCK_NOTIFICATION:
                serv->on_block_notification(request.get());
                break;
            case NXRMFLT_MSG_TYPE_KEYCHAIN_ERROR_NOTIFICATION:
                serv->on_key_error_notification(request.get());
                break;
            case NXRMFLT_MSG_TYPE_PURGE_CACHE_NOTIFICATION:
                serv->on_purge_cache_notification(request.get());
                break;
            case NXRMFLT_MSG_TYPE_PROCESS_NOTIFICATION:
                serv->on_process_notification(request.get());
                break;
            default:
                // unknown type
                throw NX::exception(ERROR_MSG("FLTSERV", "Unknown fltman request %08X", request->type()));
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
            LOGCRITICAL(ERROR_MSG("FLTSERV", "Unknown exception when processing request"));
        }
    }
}

unsigned long __stdcall drvflt_serv::drv_callback(unsigned long type, void* msg, unsigned long length, void* msg_context, void* user_context)
{
    unsigned long result = 0;
    drvflt_serv* serv = (drvflt_serv*)user_context;
    
    std::shared_ptr<flt_request> request(new flt_request(type, msg, length, msg_context));
    if (request != NULL && !request->empty()) {
        ::EnterCriticalSection(&serv->_queue_lock);
        serv->_request_queue.push(request);
        SetEvent(serv->_request_events[1]);
        ::LeaveCriticalSection(&serv->_queue_lock);
    }
    else {
        result = GetLastError();
        if (0 == result) result = ERROR_INVALID_PARAMETER;
    }

    return result;
}

void drvflt_serv::on_check_rights(flt_request* request)
{
    const NXRM_CHECK_RIGHTS_NOTIFICATION* req = (const NXRM_CHECK_RIGHTS_NOTIFICATION*)request->request();

    assert(request->type() == NXRMFLT_MSG_TYPE_CHECK_RIGHTS);

    LOGDETAIL(NX::string_formater(L"FLTSERV Request: CHECK_RIGHTS"));
    LOGDETAIL(NX::string_formater(L"  ProcessId:    %d", req->ProcessId));
    LOGDETAIL(NX::string_formater(L"  ThreadId:     %d", req->ThreadId));
    LOGDETAIL(NX::string_formater(L"  FileName:     %s", req->FileName));
}

void drvflt_serv::on_block_notification(flt_request* request)
{
    const NXRM_BLOCK_NOTIFICATION* req = (const NXRM_BLOCK_NOTIFICATION*)request->request();

    assert(request->type() == NXRMFLT_MSG_TYPE_BLOCK_NOTIFICATION);

    LOGDETAIL(NX::string_formater(L"FLTSERV Request: BLOCK_NOTIFICATION"));
    LOGDETAIL(NX::string_formater(L"  SessionId:    %d", req->SessionId));
    LOGDETAIL(NX::string_formater(L"  ProcessId:    %d", req->ProcessId));
    LOGDETAIL(NX::string_formater(L"  ThreadId:     %d", req->ThreadId));
    LOGDETAIL(NX::string_formater(L"  Reason:       %08X", req->Reason));
    LOGDETAIL(NX::string_formater(L"  FileName:     %s", req->FileName));
}

void drvflt_serv::on_key_error_notification(flt_request* request)
{
    const NXRM_KEYCHAIN_ERROR_NOTIFICATION* req = (const NXRM_KEYCHAIN_ERROR_NOTIFICATION*)request->request();

    assert(request->type() == NXRMFLT_MSG_TYPE_KEYCHAIN_ERROR_NOTIFICATION);

    LOGDETAIL(NX::string_formater(L"FLTSERV Request: KEYCHAIN_ERROR_NOTIFICATION"));
    LOGDETAIL(NX::string_formater(L"  SessionId:    %d", req->SessionId));
    //LOGDETAIL(NX::string_formater(L"  Keyid:        %d", req->ThreadId));
    LOGDETAIL(NX::string_formater(L"  FileName:     %s", req->FileName));
}

void drvflt_serv::on_purge_cache_notification(flt_request* request)
{
    const NXRM_PURGE_CACHE_NOTIFICATION* req = (const NXRM_PURGE_CACHE_NOTIFICATION*)request->request();

    assert(request->type() == NXRMFLT_MSG_TYPE_PURGE_CACHE_NOTIFICATION);

    LOGDETAIL(NX::string_formater(L"FLTSERV Request: PURGE_CACHE_NOTIFICATION"));
    LOGDETAIL(NX::string_formater(L"  FileName:     %s", req->FileName));
}

void drvflt_serv::on_process_notification(flt_request* request)
{
    const NXRM_PROCESS_NOTIFICATION* req = (const NXRM_PROCESS_NOTIFICATION*)request->request();

    assert(request->type() == NXRMFLT_MSG_TYPE_PROCESS_NOTIFICATION);

    NX::fs::dos_filepath image_path(req->ProcessImagePath);

    LOGDETAIL(NX::string_formater(L"FLTSERV Request: PROCESS_NOTIFICATION"));
    LOGDETAIL(NX::string_formater(L"  SessionId:    %d", req->SessionId));
    LOGDETAIL(NX::string_formater(L"  ProcessId:    %d", req->ProcessId));
    LOGDETAIL(NX::string_formater(L"  Operation:    %s", req->Create ? L"Create" : L"Destroy"));
    LOGDETAIL(NX::string_formater(L"  Flags:        %08X", req->Flags));
    LOGDETAIL(NX::string_formater(L"  ImageName:    %s", req->ProcessImagePath));

    if (req->Create) {
        GLOBAL.get_process_cache().insert(process_record(req->ProcessId, req->SessionId, image_path.path(), 0));
    }
    else {
        GLOBAL.get_process_cache().remove(req->ProcessId);
    }
}

