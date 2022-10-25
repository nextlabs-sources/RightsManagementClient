

#include <Windows.h>
#include <assert.h>
#include <Wtsapi32.h>

#include <string>
#include <iostream>

#include <nudf\eh.hpp>
#include <nudf\debug.hpp>
#include <nudf\winutil.hpp>
#include <nudf\string.hpp>
#include <nudf\dbglog.hpp>

#include "nxrmserv.hpp"
#include "session.hpp"


using namespace NX::dbg;


//
//  class winsession
//

winsession::winsession() : _session_id(-1)
{
}

winsession::winsession(unsigned long session_id) : _session_id(session_id)
{
}

winsession::~winsession()
{
}



//
//  class winsession
//

winsession_manager::winsession_manager()
{
    ::InitializeCriticalSection(&_lock);
}

winsession_manager::~winsession_manager()
{
    ::DeleteCriticalSection(&_lock);
}

void winsession_manager::add_session(unsigned long session_id)
{
    ::EnterCriticalSection(&_lock);
    auto pos = _map.find(session_id);
    if (pos == _map.end()) {
        try {
            std::shared_ptr<winsession> sp(new winsession(session_id));
            _map[session_id] = std::shared_ptr<winsession>(sp);
        }
        catch (std::exception& e) {
            LOGERROR(NX::conversion::utf8_to_utf16(e.what()));
        }
    }
    ::LeaveCriticalSection(&_lock);
}

void winsession_manager::remove_session(unsigned long session_id)
{
    ::EnterCriticalSection(&_lock);
    auto pos = _map.find(session_id);
    if (pos != _map.end()) {
        _map.erase(pos);
    }
    ::LeaveCriticalSection(&_lock);
}

std::shared_ptr<winsession> winsession_manager::get_session(unsigned long session_id) const
{
    std::shared_ptr<winsession> result;

    ::EnterCriticalSection(&_lock);
    auto pos = _map.find(session_id);
    if (pos != _map.end()) {
        result = (*pos).second;
    }
    ::LeaveCriticalSection(&_lock);

    return result;
}

void winsession_manager::clear()
{
    ::EnterCriticalSection(&_lock);
    _map.clear();
    ::LeaveCriticalSection(&_lock);
}

bool winsession_manager::empty()
{
    bool result = true;
    ::EnterCriticalSection(&_lock);
    result = _map.empty();
    ::LeaveCriticalSection(&_lock);
    return result;
}

std::vector<unsigned long> winsession_manager::find_existing_session()
{
    PWTS_SESSION_INFOW  ssinf = NULL;
    DWORD               count = 0;
    std::vector<unsigned long> session_list;

    if (WTSEnumerateSessionsW(NULL, 0, 1, &ssinf, &count) && NULL != ssinf) {
        for (int i = 0; i < (int)count; i++) {
            if (ssinf[i].State == WTSActive && 0 != ssinf[i].SessionId) {
                session_list.push_back(ssinf[i].SessionId);
            }
        }
        WTSFreeMemory(ssinf);
        ssinf = NULL;
    }

    return std::move(session_list);
}


//
//  class rmsession
//

rmsession::rmsession()
{
}

rmsession::~rmsession()
{
    clear();
}

void rmsession::initialize(const std::wstring& list_file)
{
    rmprofile_list pflist;

    pflist.load_file(list_file);
}

void rmsession::clear()
{
}


//
//  class rmappinstance
//

// ERROR_MAX_SESSIONS_REACHED

rmappinstance::rmappinstance() : _process_handle(NULL), _process_id(0)
{
}

rmappinstance::rmappinstance(const std::wstring& app_path)
{
}

rmappinstance::~rmappinstance()
{
    quit();
}

void rmappinstance::run(unsigned long session_id)
{
    HANDLE          tk = NULL;
    STARTUPINFOW    si;
    PROCESS_INFORMATION pi;
    DWORD dwError = 0;

    if (is_running()) {
        return;
    }

    assert(session_id != 0);
    if (session_id == 0) {
        throw WIN32_ERROR_MSG(ERROR_INVALID_CLUSTER_IPV6_ADDRESS, "fail to get session token");
    }

    if (session_id != -1) {

        if (!WTSQueryUserToken(session_id, &tk)) {
            throw WIN32_ERROR_MSG(GetLastError(), "fail to get session token");
        }
        assert(NULL != tk);
        if (NULL == tk) {
            throw WIN32_ERROR_MSG(ERROR_INVALID_CLUSTER_IPV6_ADDRESS, "fail to get session token");
        }
    }

    memset(&pi, 0, sizeof(pi));
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    si.lpDesktop = L"WinSta0\\Default";
    if (!::CreateProcessAsUserW(tk, _image_path.c_str(), NULL, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        dwError = GetLastError();
        CloseHandle(tk);
        throw WIN32_ERROR_MSG(ERROR_INVALID_CLUSTER_IPV6_ADDRESS, "fail to create process");
    }

    _process_handle = pi.hProcess;
    _process_id = pi.dwProcessId;
    CloseHandle(pi.hThread);
    CloseHandle(tk);
}

void rmappinstance::quit(unsigned long wait_time)
{
    if (is_running()) {
        kill();
    }
}

void rmappinstance::kill()
{
    if (NULL != _process_handle) {
        TerminateProcess(_process_handle, 0);
        CloseHandle(_process_handle);
        _process_handle = NULL;
        _process_id = 0;
    }
}

void rmappinstance::attach(unsigned long process_id)
{
    _process_handle = ::OpenProcess(PROCESS_TERMINATE, FALSE, process_id);
    if (NULL != _process_handle) {
        _process_id = process_id;
    }
}

void rmappinstance::detach()
{
    if (NULL != _process_handle) {
        CloseHandle(_process_handle);
        _process_handle = NULL;
        _process_id = 0;
    }
}



//
//  class rmappmanager
//
rmappmanager::rmappmanager() : _shutdown_event(NULL)
{
    _shutdown_event = ::CreateEventW(NULL, FALSE, FALSE, NULL);
}

rmappmanager::~rmappmanager()
{
    shutdown();

    if (NULL != _shutdown_event) {
        CloseHandle(_shutdown_event);
        _shutdown_event = NULL;
    }
}

void rmappmanager::start(unsigned long session_id, const std::wstring& app_path)
{
    _session_id = session_id;
    _daemon_thread = std::thread(rmappmanager::daemon, this);
}

void rmappmanager::shutdown()
{
    if (_daemon_thread.joinable()) {
        SetEvent(_shutdown_event);
        _daemon_thread.join();
    }
}

void rmappmanager::daemon(rmappmanager* manager)
{
    bool    active = true;
    unsigned long wait_result = WAIT_TIMEOUT;
    HANDLE wait_objects[2] = { manager->_shutdown_event, NULL };

    wait_objects[0] = manager->get_shutdown_event();
    assert(NULL != wait_objects[0]);

    do {

        if (manager->get_instance() != nullptr) {
            manager->get_instance()->quit();
        }

        manager->get_instance()->run(manager->get_session_id());

        wait_result = ::WaitForMultipleObjects(2, wait_objects, FALSE, INFINITE);
        switch (wait_result)
        {
        case WAIT_OBJECT_0:
            // shutdown
            active = false;
            manager->get_instance()->quit();
            break;

        case (WAIT_OBJECT_0 + 1) :
            // process has been terminated
            manager->get_instance()->detach();
            break;

        default:
            // error
            active = false;
            manager->get_instance()->quit();
            break;
        }

    } while (active);
}
