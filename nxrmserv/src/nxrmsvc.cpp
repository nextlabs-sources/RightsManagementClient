
#include <Windows.h>
#include <assert.h>
#include <Wtsapi32.h>

#include <iostream>
#include <string>

#include <nudf\exception.hpp>
#include <nudf\shared\logdef.h>
#include <nudf\log.hpp>

#include "nxrmserv.h"
#include "nxrmsvc.hpp"
#include "servlog.hpp"



CRmService::CRmService() : nudf::win::svc::IService(NXRMSERV_NAME), _delay(0)
{
    ::InitializeCriticalSection(&_logon_lock);
}

CRmService::~CRmService()
{
    ::DeleteCriticalSection(&_logon_lock);
}
    
void CRmService::OnStart()
{
    try {

        LOGSYSINF(L"[SERVER] Starting ...");
        GLOBAL.change_status(NX::global::serv_start_pending);

        // Check delay
        if (_delay > 0) {
            _delay = min(180, _delay);
            LOGSYSINF(L"          Delay %d seconds ...", _delay);
            Sleep(_delay * 1000);
        }

        LOGSYSINF(L"[SERVER] Start communication service ...");
        if (!GLOBAL.listen()) {
            LOGASSERT(L"[SERVER] Failed (%d)", GetLastError());
            throw std::exception("fail to start communication service");
        }
        LOGSYSINF(L"[SERVER] Listening");


        LOGSYSINF(L"[SERVER] Connecting to core driver ...");
        if (!GLOBAL.drv_core().start()) {
            LOGASSERT(L"[SERVER] Failed (%d)", GetLastError());
            throw std::exception("fail to connect to core driver");
        }
        LOGSYSINF(L"[SERVER] Connected");


        NX::product prod;
#ifdef _AMD64_
        prod = NX::product(L"NextLabs Rights Management", NX::arch_amd64);
#else
        prod = NX::product(L"NextLabs Rights Management", NX::arch_i386);
#endif

        // Load configuration
        NX::web::json::value classify_bundle;
        NX::web::json::value keys_bundle;
        GLOBAL.load_configurations(classify_bundle, keys_bundle);
        //  --> if keys are ready, connect to filter driver
        if (!GLOBAL.agent_info().empty() && !keys_bundle.is_null()) {
            std::vector<unsigned char> keys_blob = GLOBAL.generate_keys_budnle(keys_bundle);
            if (!keys_blob.empty()) {
                LOGSYSINF(L"[SERVER] Connecting to filter driver ...");
                if (!GLOBAL.drv_flt().start(&keys_blob[0], (unsigned long)keys_blob.size())) {
                    LOGASSERT(L"[SERVER] Failed (%d)", GetLastError());
                    throw std::exception("fail to connect to filter driver");
                }
#ifdef _DEBUG
                GLOBAL._nxl_keys_blob = keys_blob;
#endif
                LOGSYSINF(L"[SERVER] Connected");
            }
        }
        //  --> make sure profiles for all users on this machine is ready
        NX::logged_on_users lou;
        std::for_each(lou.users().begin(), lou.users().end(), [&](const std::pair<std::wstring, std::wstring>& user) {
            // make sure profile folder exists
            if (!NX::global::create_profile_folder(user.first, false)) {
                LOGASSERT(L"Failed create profile folder for user %s (%s)", user.second.c_str(), user.first.c_str());
                return;
            }
            // make sure policy bundle exists
            // make sure policy bundle exists
        });

        // Try to add existing sessions
        LOGSYSINF(L"[SERVER] Starting session system ...");
        PWTS_SESSION_INFOW  ssinf = NULL;
        DWORD               count = 0;
        NX::serv_session::SessionMapType sessions;
        ::EnterCriticalSection(&_logon_lock);
        if (WTSEnumerateSessionsW(NULL, 0, 1, &ssinf, &count) && NULL != ssinf) {
            for (int i = 0; i < (int)count; i++) {
                if (ssinf[i].State == WTSActive) {
                    bool existing = false;
                    std::shared_ptr<NX::session> ss = GLOBAL.serv_session().add(ssinf[i].SessionId, &existing);
                    if (!existing) {
                        // new session logged on
                        LOGSYSINF(L"Session (%d) already logged on\r\n\t\tuser.id = %s\r\n\t\tuser.name = %s\r\n\t\tuser.domain = %s", ssinf[i].SessionId, ss->user().id().c_str(), ss->user().is_domain_user() ? ss->user().principle_name().c_str() : ss->user().name().c_str(), ss->user().domain().c_str());
                    }
                    else {
                        // session already logged on
                        LOGASSERT(L"Session (%d) logon again before logout", ssinf[i].SessionId);
                    }
                }
            }
            WTSFreeMemory(ssinf);
            ssinf = NULL;
        }
        // make a copy of existing sessions
        GLOBAL.serv_session().copy(sessions);
        ::LeaveCriticalSection(&_logon_lock);
        LOGSYSINF(L"[SERVER] Done");

        // start each session
        std::for_each(sessions.begin(), sessions.end(), [&](auto item) {
            // try to start session instance
            item.second->start();
        });

        // change service status
        GLOBAL.change_status(NX::global::serv_running);

        // run jobs
        LOGSYSINF(L"[SERVER] Starting job system ...");
        GLOBAL.run_jobs();
        LOGSYSINF(L"[SERVER] Done");

        LOGSYSINF(L"[SERVER] ACTIVE");
        LOGSYSINF(L" ");
    }
    catch (std::exception& e) {
        std::string s(e.what() ? e.what() : "unknown c++ exception");
        std::wstring ws(s.begin(), s.end());
        LOGASSERT(ws.c_str());
    }
    catch (NX::structured_exception& e) {
        std::wstring exception_msg = e.exception_message();
        LOGASSERT(exception_msg.c_str());
        e.dump(); // NOTHING
        e.raise();
    }
}

void CRmService::OnStop() noexcept
{
    try {

        LOGSYSINF(L" ");
        LOGSYSINF(L"[SERVER] Stopping ...");
        GLOBAL.change_status(NX::global::serv_stop_pending);

        if (GLOBAL.drv_flt().started()) {
            LOGSYSINF(L"[SERVER] Disconnecting from filter driver ...");
            GLOBAL.drv_flt().stop();
            LOGSYSINF(L"[SERVER] Disconnected");
        }
        if (GLOBAL.drv_core().started()) {
            LOGSYSINF(L"[SERVER] Disconnecting from core driver ...");
            GLOBAL.drv_core().stop();
            LOGSYSINF(L"[SERVER] Disconnected");
        }

        LOGSYSINF(L"[SERVER] Shutting down audit system");
        GLOBAL.audit_serv().stop();
        LOGSYSINF(L"[SERVER] Done");

        LOGSYSINF(L"[SERVER] Shutting down job system");
        GLOBAL.jobs().clear();
        LOGSYSINF(L"[SERVER] Done");

        LOGSYSINF(L"[SERVER] Shutting down session system");
        GLOBAL.serv_session().clear();
        LOGSYSINF(L"[SERVER] Done");

        GLOBAL.change_status(NX::global::serv_stopped);
        LOGSYSINF(L"[SERVER] INACTIVE");
        LOGSYSINF(L"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
        LOGSYSINF(L" ");
        LOGSYSINF(L" ");
    }
    catch (std::exception& e) {
        std::string s(e.what() ? e.what() : "unknown c++ exception");
        std::wstring ws(s.begin(), s.end());
        LOGASSERT(ws.c_str());
    }
    catch (...) {
        LOGASSERT(L"Unknown error happened during stopping service");
    }
}

void CRmService::OnPause()
{
    LOGSYSINF(L"[SERVER] Pausing ...");
    GLOBAL.change_status(NX::global::serv_paused);
    LOGSYSINF(L"[SERVER] PAUSED");
}

void CRmService::OnResume()
{
    LOGSYSINF(L"[SERVER] Resuming ...");
    assert(GLOBAL.is_paused());
    GLOBAL.change_status(NX::global::serv_running);
    LOGSYSINF(L"[SERVER] RESUMED");
}

LONG CRmService::OnSessionLogon(_In_ WTSSESSION_NOTIFICATION* wtsn) noexcept
{
    bool existing = false;
    std::shared_ptr<NX::session> ss;

    if (!GLOBAL.is_running()) {
        return 0L;
    }

    try {
        ::EnterCriticalSection(&_logon_lock);
        ss = GLOBAL.serv_session().add(wtsn->dwSessionId, &existing);
        ::LeaveCriticalSection(&_logon_lock);
        if (!existing) {
            // new session logged on
            LOGSYSINF(L"New session (%d) logged on\r\n\t\tuser.id = %s\r\n\t\tuser.name = %s\r\n\t\tuser.domain = %s", wtsn->dwSessionId, ss->user().id().c_str(), ss->user().is_domain_user() ? ss->user().principle_name().c_str() : ss->user().name().c_str(), ss->user().domain().c_str());
            // try to start session instance
            ss->start();
        }
        else {
            // session already logged on
            LOGASSERT(L"Session (%d) logon again before logout", wtsn->dwSessionId);
        }
    }
    catch (std::exception& e) {
        std::string s(e.what() ? e.what() : "unknown c++ exception");
        std::wstring ws(s.begin(), s.end());
        LOGASSERT(ws.c_str());
    }
    catch (NX::structured_exception& e) {
        std::wstring exception_msg = e.exception_message();
        LOGASSERT(exception_msg.c_str());
        e.dump(); // NOTHING
        e.raise();
    }


    return 0L;
}

LONG CRmService::OnSessionLogoff(_In_ WTSSESSION_NOTIFICATION* wtsn) noexcept
{
    std::shared_ptr<NX::session> ss;

    if (!GLOBAL.is_running()) {
        return 0L;
    }

    try {
        ::EnterCriticalSection(&_logon_lock);
        ss = GLOBAL.serv_session().del(wtsn->dwSessionId);
        ::LeaveCriticalSection(&_logon_lock);

        if (ss != nullptr) {
            // a valid session is removed
            LOGSYSINF(L"Session logged off\r\n\t\tuser.id = %s\r\n\t\tuser.name = %s\r\n\t\tuser.domain = %s", ss->user().id().c_str(), ss->user().is_domain_user() ? ss->user().principle_name().c_str() : ss->user().name().c_str(), ss->user().domain().c_str());
            ss->stop();
        }
    }
    catch (std::exception& e) {
        std::string s(e.what() ? e.what() : "unknown c++ exception");
        std::wstring ws(s.begin(), s.end());
        LOGASSERT(ws.c_str());
    }
    catch (NX::structured_exception& e) {
        std::wstring exception_msg = e.exception_message();
        LOGASSERT(exception_msg.c_str());
        e.dump(); // NOTHING
        e.raise();
    }
    catch (...) {
        LOGASSERT(L"Unknown exception");
    }
    return 0L;
}