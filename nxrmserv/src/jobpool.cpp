

#include <Windows.h>
#include <assert.h>


#include <nudf\web\json.hpp>
#include <nudf\nxrmres.h>
#include <nudf\resutil.hpp>

#include "nxrmserv.h"
#include "global.hpp"
#include "rest.hpp"
#include "sconfig.hpp"
#include "autoupgrade.hpp"
#include "jobpool.hpp"




const std::wstring NX::job::jn_register(L"register");
const std::wstring NX::job::jn_heartbeat(L"heartbeat");
const std::wstring NX::job::jn_audit(L"audit");
const std::wstring NX::job::jn_checkupdate(L"checkupdate");

NX::job::job() : _repeat_interval(0), _evt(NULL), _stopping(false)
{
    _evt = ::CreateEventW(NULL, FALSE, FALSE, NULL);
}

NX::job::job(const std::wstring& name, unsigned long repeat_interval) : _name(name), _repeat_interval(repeat_interval), _evt(NULL), _stopping(false)
{
    _evt = ::CreateEventW(NULL, FALSE, FALSE, NULL);
}

NX::job::~job()
{
    if (NULL != _evt) {
        stop();
        ::CloseHandle(_evt);
        _evt = NULL;
    }
}

void NX::job::start()
{
    // If no repeat (0 == repeat_interval), set stop event (on)
    // Otherwise, reset stop event (off)
    _stopping = (0 == repeat_interval()) ? true : false;
    _worker = std::thread(NX::job::worker, this);
}

void NX::job::stop(bool wait)
{
    try {
        _stopping = true;
        LOGDMP(L"Stopping job: %s", name().c_str())
        SetEvent(_evt);
        if (wait) {
            if (_worker.joinable()) {
                _worker.join();
            }
        }
        else {
            _worker.detach();
        }
        LOGDMP(L"Job (%s) stopped", name().c_str())
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

void NX::job::trigger()
{
    if (NULL != _evt) {
        SetEvent(_evt);
    }
}

void NX::job::worker(job* j) noexcept
{
    unsigned long wait_result = WAIT_TIMEOUT;

    try {
        do {
            try {
                j->do_job();
            }
            catch (std::exception& e) {
                std::string s(e.what() ? e.what() : "unknown c++ exception");
                std::wstring ws(s.begin(), s.end());
                LOGASSERT(ws.c_str());
            }
            wait_result = ::WaitForSingleObject(j->_evt, j->repeat_interval());
            if (wait_result != WAIT_OBJECT_0 && wait_result != WAIT_TIMEOUT) {
                // error happened
                break;
            }
        } while (!j->stopping() && GLOBAL.is_running());
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
    catch (NX::structured_exception& e) {
        std::wstring exception_msg = e.exception_message();
        LOGASSERT(exception_msg.c_str());
        e.dump(); // NOTHING
        e.raise();
    }
}



NX::job_pool::job_pool()
{
    ::InitializeCriticalSection(&_queue_lock);
}

NX::job_pool::~job_pool()
{
    ::DeleteCriticalSection(&_queue_lock);
}

bool NX::job_pool::add(std::shared_ptr<job>& p)
{
    bool result = false;
    ::EnterCriticalSection(&_queue_lock);
    auto it = _queue.begin();
    for (; it != _queue.end(); ++it) {
        if (0 == _wcsicmp(p->name().c_str(), (*it)->name().c_str())) {
            break;
        }
    }
    if (it == _queue.end()) {
        // Not exist, insert it
        _queue.push_back(p);
        result = true;
        p->start();
    }
    ::LeaveCriticalSection(&_queue_lock);
    return result;
}

std::shared_ptr<NX::job> NX::job_pool::remove(const std::wstring& name)
{
    std::shared_ptr<NX::job> sp;
    ::EnterCriticalSection(&_queue_lock);
    auto pos = std::find_if(_queue.begin(), _queue.end(), [&](const std::shared_ptr<job>& it) {
        assert(it != __nullptr);
        return (0 == _wcsicmp(name.c_str(), it->name().c_str()));
    });
    if (pos != _queue.end()) {
        sp = (*pos);
        _queue.erase(pos);
    }
    ::LeaveCriticalSection(&_queue_lock);
    return sp;
}

std::shared_ptr<NX::job> NX::job_pool::get(const std::wstring& name)
{
    std::shared_ptr<NX::job> sp;
    ::EnterCriticalSection(&_queue_lock);
    auto pos = std::find_if(_queue.begin(), _queue.end(), [&](const std::shared_ptr<job>& it) {
        assert(it != __nullptr);
        return (0 == _wcsicmp(name.c_str(), it->name().c_str()));
    });
    if (pos != _queue.end()) {
        sp = (*pos);
    }
    ::LeaveCriticalSection(&_queue_lock);
    return sp;
}

void NX::job_pool::clear()
{
    ::EnterCriticalSection(&_queue_lock);
    std::for_each(_queue.begin(), _queue.end(), [&](std::shared_ptr<NX::job>& sp) {
        if (sp != NULL) {
            sp->stop();
        }
    });
    _queue.clear();
    ::LeaveCriticalSection(&_queue_lock);
}

//
//  jobs
//
void NX::job_register::do_job() noexcept
{
    // make sure agent_info is empty
    assert(GLOBAL.agent_info().empty());

    if (GLOBAL.rest().register_agent(GLOBAL.agent_info())) {

        // Generate agent key
        GLOBAL.generate_agent_key();

        // Save agent info to local file
        std::wstring agent_info_file = GLOBAL.dir_conf() + L"\\agent_info.sjs";
        GLOBAL.agent_info().save(agent_info_file);
    }

    // Fail to register?
    if (GLOBAL.agent_info().empty()) {
        return;
    }

    // succeed
    // add jobs:
    //    - heartbeat
    GLOBAL.jobs().add(std::shared_ptr<NX::job>(new NX::job_heartbeat(GLOBAL.agent_info().heartbeat_frequency()*1000)));
    //    - audit
    GLOBAL.audit_serv().start(GLOBAL.dir_auditlogs());
    GLOBAL.jobs().add(std::shared_ptr<NX::job>(new NX::job_audit(GLOBAL.agent_info().log_frequency()*1000)));
    //    - checkupdate
    GLOBAL.jobs().add(std::shared_ptr<NX::job>(new NX::job_checkupdate()));
    // stop this job
    std::shared_ptr<NX::job> sp_register = GLOBAL.jobs().remove(NX::job::jn_register);
    if (sp_register != nullptr) {
        // removed, stop without waiting
        sp_register->stop(false);
    }
}

void NX::job_heartbeat::do_job() noexcept
{
    bool policy_changed = false;

    // make sure agent_info is NOT empty
    assert(!GLOBAL.agent_info().empty());

    // send heartbeat request

    try {

        bool changed = false;
        NX::rest_heartbeat_result res;

        if (!GLOBAL.rest().request_heartbeat(res)) {
            throw std::exception("fail to send heart beat request");
        }

        if (res.info().empty()) {
            throw std::exception("empty agent information");
        }

        if (GLOBAL.agent_info().id() != res.info().id()) {
            LOGASSERT(L"agent id doesn't match (existing: %s v.s. response: %s)", GLOBAL.agent_info().id().c_str(), res.info().id().c_str());
            throw std::exception("agent id doesn't match");
        }

        if (res.force_update() && GLOBAL.policy_need_force_update()) {
            GLOBAL.reset_policy_force_update_flag();
        }

        if (!res.classify_bundle().is_null()) {
            std::wstring v = res.classify_bundle().serialize();
            if (GLOBAL.update_classify_bundle(v)) {
                changed = true;
            }
        }
        if (!res.keys_bundle().is_null()) {
            std::wstring v = res.keys_bundle().serialize();
            if (GLOBAL.update_keys_bundle(v)) {
                changed = true;
            }
        }
        if (!res.agent_whitelist().is_null()) {
            if(GLOBAL.update_agent_whitelist(res.agent_whitelist())) {
                changed = true;
            }
        }
        if (!res.policy_bundle().empty()) {
            if(GLOBAL.update_policy_bundle(res.policy_bundle(), res.policy_bundle_time(), res.force_update())) {
                changed = true;
                policy_changed = true;
                // let driver know that policy has been changed
                GLOBAL.drv_flt().set_policy_changed();
                GLOBAL.drv_core().increase_policy_sn();
                // need to update all the policies in session
                NX::serv_session::SessionMapType ss;
                GLOBAL.serv_session().copy(ss);
                std::wstring title = nudf::util::res::LoadMessage(GLOBAL.res_module(), IDS_PRODUCT_NAME, 256, LANG_NEUTRAL, L"NextLabs Rights Management");
                std::wstring msg = nudf::util::res::LoadMessage(GLOBAL.res_module(), IDS_NOTIFY_POLICY_UPDATED, 1024, LANG_NEUTRAL, L"Policy has been updated");
                std::for_each(ss.begin(), ss.end(), [&](auto s) {
                    s.second->set_session_policy_bundle(GLOBAL.policy_bundle());
                    s.second->set_policy_time(res.policy_bundle_time());
                    s.second->notify(title, msg);
                });
            }
        }

        GLOBAL.update_agent_info(res.info(), changed);

        // finally, send request_acknowledge
        if (changed) {
            if (!GLOBAL.rest().send_acknowledge()) {
                LOGWARN(0, L"heartbeat succeed, but acknowledge failed");
            }
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }

    unsigned long requestor_session_id = GLOBAL.get_requestor_session_id();
    if (0 != requestor_session_id) {
        GLOBAL.set_requestor_session_id(0);
        // send response
        std::shared_ptr<NX::session> sp = GLOBAL.serv_session().get(requestor_session_id);
        if (sp != NULL) {
            // let requestor know that update has been done
            sp->send_update_response(policy_changed);
        }
    }

    // Trim Working Set
//#ifndef _DEBUG
//    NX::sys::trim_working_set();
//#endif
}

void NX::job_audit::do_job() noexcept
{
    // make sure agent_info is NOT empty
    assert(!GLOBAL.agent_info().empty());

    // send audit request
    GLOBAL.audit_serv().upload();

    // Trim Working Set
//#ifndef _DEBUG
//    NX::sys::trim_working_set();
//#endif
}

void NX::job_checkupdate::do_job() noexcept
{
    // make sure agent_info is NOT empty
    assert(!GLOBAL.agent_info().empty());
    if (NX::auto_upgrade()) {
        // stop this job
        std::shared_ptr<NX::job> sp_checkupdate = GLOBAL.jobs().remove(NX::job::jn_checkupdate);
        if (sp_checkupdate != nullptr) {
            // removed, stop without waiting
            sp_checkupdate->stop(false);
        }
    }

    // Trim Working Set
//#ifndef _DEBUG
//    NX::sys::trim_working_set();
//#endif
}