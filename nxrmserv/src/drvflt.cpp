

#include <Windows.h>
#include <Shlobj.h>
#include <assert.h>
#include <Wtsapi32.h>
#include <Winternl.h>

#include <iostream>
#include <fstream>

#include <nudf\exception.hpp>
#include <nudf\string.hpp>
#include <nudf\path.hpp>
#include <nudf\shared\enginectl.h>
#include <nudf\shared\obutil.h>
#include <nudf\shared\officelayout.h>
#include <nudf\shared\nxlfmt.h>
#include <nudf\nxlutil.hpp>

#include <nudf\nxrmres.h>
#include <nudf\resutil.hpp>

// from fltman
#include "nxrmflt.h"
#include "nxrmfltman.h"

#include "nxrmserv.h"
#include "rest.hpp"
#include "session.hpp"
#include "nxlfile.hpp"
#include "drvflt.hpp"


using namespace NX;



class flt_man
{
typedef HANDLE(WINAPI* NXRMFLT_CREATE_MANAGER)(NXRMFLT_CALLBACK_NOTIFY, LOGAPI_LOG, LOGAPI_ACCEPT, const NXRM_KEY_BLOB*, ULONG, PVOID);
typedef ULONG(WINAPI* NXRMFLT_REPLY_MESSAGE)(HANDLE, PVOID, NXRMFLT_CHECK_RIGHTS_REPLY*);
typedef ULONG(WINAPI* NXRMFLT_START_FILTERING)(HANDLE);
typedef ULONG(WINAPI* NXRMFLT_STOP_FILTERING)(HANDLE);
typedef ULONG(WINAPI* NXRMFLT_CLOSE_MANAGER)(HANDLE);
typedef ULONG(WINAPI* NXRMFLT_SET_SAVEAS_FORECAST)(HANDLE, ULONG, const WCHAR*, const WCHAR*);
typedef ULONG(WINAPI* NXRMFLT_SET_POLICY_CHANGED)(HANDLE);
typedef ULONG(WINAPI* NXRMFLT_UPDATE_KEYCHAIN)(HANDLE, const NXRM_KEY_BLOB*, ULONG);
public:
    flt_man() : hMod(NULL),
        FnCreateManager(NULL),
        FnCloseManager(NULL),
        FnReplyMessage(NULL),
        FnStartFiltering(NULL),
        FnStopFiltering(NULL),
        FnSetSaveAsForecast(NULL),
        FnSetPolicyChanged(NULL),
        FnUpdateKeyChain(NULL)
    {
        load();
    }

    ~flt_man()
    {
        unload();
    }

    void load()
    {
        nudf::win::CModulePath mod(NULL);
        std::wstring wsDll = mod.GetParentDir();
        if (!wsDll.empty()) {
            wsDll += L"\\";
        }
        wsDll += L"nxrmfltman.dll";

        hMod = ::LoadLibraryW(wsDll.c_str());
        if (NULL != hMod) {
            // Refer to nxrmfltman.def to get ordinal
            FnCreateManager = (NXRMFLT_CREATE_MANAGER)GetProcAddress(hMod, MAKEINTRESOURCEA(1));
            FnReplyMessage = (NXRMFLT_REPLY_MESSAGE)GetProcAddress(hMod, MAKEINTRESOURCEA(2));
            FnStopFiltering = (NXRMFLT_STOP_FILTERING)GetProcAddress(hMod, MAKEINTRESOURCEA(3));
            FnStartFiltering = (NXRMFLT_START_FILTERING)GetProcAddress(hMod, MAKEINTRESOURCEA(4));
            FnCloseManager = (NXRMFLT_CLOSE_MANAGER)GetProcAddress(hMod, MAKEINTRESOURCEA(5));
            FnSetSaveAsForecast = (NXRMFLT_SET_SAVEAS_FORECAST)GetProcAddress(hMod, MAKEINTRESOURCEA(6));
            FnSetPolicyChanged = (NXRMFLT_SET_POLICY_CHANGED)GetProcAddress(hMod, MAKEINTRESOURCEA(7));
            FnUpdateKeyChain = (NXRMFLT_UPDATE_KEYCHAIN)GetProcAddress(hMod, MAKEINTRESOURCEA(8));
        }
    }
    void unload()
    {
        FnCreateManager = NULL;
        FnReplyMessage = NULL;
        FnStartFiltering = NULL;
        FnStopFiltering = NULL;
        FnCloseManager = NULL;
        FnSetSaveAsForecast = NULL;
        FnSetPolicyChanged = NULL;
        FnUpdateKeyChain = NULL;
        if (NULL != hMod) {
            FreeLibrary(hMod);
        }
    }
    bool is_valid() const noexcept
    {
        return (FnCreateManager != NULL &&
            FnReplyMessage != NULL &&
            FnStartFiltering != NULL &&
            FnStopFiltering != NULL &&
            FnCloseManager != NULL &&
            FnSetSaveAsForecast != NULL &&
            FnSetPolicyChanged != NULL &&
            FnUpdateKeyChain != NULL
            );
    }

    HANDLE CreateManager(NXRMFLT_CALLBACK_NOTIFY CbNotify, LOGAPI_LOG CbDbgDump, LOGAPI_ACCEPT CbDbgDumpCheckLevel, const NXRM_KEY_BLOB* KeyBlob, ULONG KeyBlobSize, PVOID Context)
    {
        if (NULL == FnCreateManager) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return NULL;
        }
        return FnCreateManager(CbNotify, CbDbgDump, CbDbgDumpCheckLevel, KeyBlob, KeyBlobSize, Context);
    }
    ULONG ReplyMessage(HANDLE MgrHandle, PVOID Context, NXRMFLT_CHECK_RIGHTS_REPLY* Reply)
    {
        if (NULL == FnReplyMessage) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnReplyMessage(MgrHandle, Context, Reply);
    }
    ULONG StartFiltering(HANDLE MgrHandle)
    {
        if (NULL == FnStartFiltering) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnStartFiltering(MgrHandle);
    }
    ULONG StopFiltering(HANDLE MgrHandle)
    {
        if (NULL == FnStopFiltering) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnStopFiltering(MgrHandle);
    }
    ULONG CloseManager(HANDLE MgrHandle)
    {
        if (NULL == FnCloseManager) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnCloseManager(MgrHandle);
    }
    ULONG SetSaveAsForecast(HANDLE MgrHandle, ULONG ProcessId, _In_opt_ const WCHAR* SrcFileName, _In_ const WCHAR* SaveAsFileName)
    {
        if (NULL == FnSetSaveAsForecast) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnSetSaveAsForecast(MgrHandle, ProcessId, SrcFileName, SaveAsFileName);
    }
    ULONG SetPolicyChanged(HANDLE MgrHandle)
    {
        if (NULL == FnSetPolicyChanged) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnSetPolicyChanged(MgrHandle);
    }
    ULONG UpdateKeyChain(HANDLE MgrHandle, const NXRM_KEY_BLOB* KeyBlob, ULONG KeyBlobSize)
    {
        if (NULL == FnUpdateKeyChain) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnUpdateKeyChain(MgrHandle, KeyBlob, KeyBlobSize);
    }

private:
    HMODULE hMod;
    NXRMFLT_CREATE_MANAGER  FnCreateManager;
    NXRMFLT_REPLY_MESSAGE FnReplyMessage;
    NXRMFLT_START_FILTERING FnStartFiltering;
    NXRMFLT_STOP_FILTERING FnStopFiltering;
    NXRMFLT_CLOSE_MANAGER FnCloseManager;
    NXRMFLT_SET_SAVEAS_FORECAST FnSetSaveAsForecast;
    NXRMFLT_SET_POLICY_CHANGED FnSetPolicyChanged;
    NXRMFLT_UPDATE_KEYCHAIN FnUpdateKeyChain;
};



//
//  Local data
//
static flt_man FLTMAN;



//
//  class NX::drv::serv_flt::flt_request
//
NX::drv::serv_flt::flt_request::flt_request(unsigned long type, void* msg, unsigned long length, void* msg_context) : _type(0), _context(NULL)
{
    unsigned long required_size = 0;

    // check size
    switch (type)
    {
    case NXRMFLT_MSG_TYPE_CHECK_RIGHTS:
        if (length < sizeof(NXRM_CHECK_RIGHTS_NOTIFICATION)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return;
        }
        break;
    case NXRMFLT_MSG_TYPE_BLOCK_NOTIFICATION:
        if (length < sizeof(NXRM_BLOCK_NOTIFICATION)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return;
        }
        break;
    case NXRMFLT_MSG_TYPE_KEYCHAIN_ERROR_NOTIFICATION:
        if (length < sizeof(NXRM_KEYCHAIN_ERROR_NOTIFICATION)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return;
        }
        break;
    case NXRMFLT_MSG_TYPE_PURGE_CACHE_NOTIFICATION:
        if (length < sizeof(NXRM_PURGE_CACHE_NOTIFICATION)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return;
        }
        break;
    case NXRMFLT_MSG_TYPE_PROCESS_NOTIFICATION:
        if (length < sizeof(NXRM_PROCESS_NOTIFICATION)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return;
        }
        break;
    default:
        break;
    }

    _request.resize(length, 0);
    memcpy(&_request[0], msg, length);
    _type    = type;
    _context = msg_context;
}

//
//  class NX::drv::serv_flt
//
NX::drv::serv_flt::serv_flt() : _fltman(NULL), _stop_event(NULL), _started(false)
{
    ::InitializeCriticalSection(&_list_lock);
    _stop_event = ::CreateEventW(NULL, TRUE, FALSE, NULL);
    _request_event = ::CreateEventW(NULL, TRUE, FALSE, NULL);
}

NX::drv::serv_flt::~serv_flt()
{
    // stop
    stop();

    // cleanup
    ::DeleteCriticalSection(&_list_lock);
    if (NULL != _stop_event) {
        CloseHandle(_stop_event);
_stop_event = NULL;
    }
    if (NULL != _request_event) {
        CloseHandle(_request_event);
        _request_event = NULL;
    }
}

bool NX::drv::serv_flt::start(const void* key_blob, unsigned long size) noexcept
{
    DWORD dwResult = 0;

    assert(!_started);

    if (_started) {
        return false;
    }

    try {

        _fltman = FLTMAN.CreateManager(NX::drv::serv_flt::drv_callback, write_log, check_log_accept, (const NXRM_KEY_BLOB*)key_blob, size, this);
        if (NULL == _fltman) {
            LOGERR(GetLastError(), L"fail to start driver manager for nxrmflt.sys");
            throw std::exception("fail to create driver manager for nxrmflt.sys");
        }

        // enable nxrmflt manager
        dwResult = FLTMAN.StartFiltering(_fltman);
        if (0 != dwResult) {
            CloseHandle(_fltman);
            LOGERR(dwResult, L"fail to start driver manager for nxrmflt.sys");
            throw std::exception("fail to start driver manager for nxrmflt.sys");
        }

        // start worker thread
        for (int i = 0; i < 4; i++) {
            _threads.push_back(std::thread(NX::drv::serv_flt::worker, this));
        }

        _started = true;
        // use this function to purge cache in driver
        // it doesn't mean the policy change
        set_policy_changed();
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        _started = false;
        _threads.clear();
    }
    catch (NX::structured_exception& e) {
        std::wstring exception_msg = e.exception_message();
        LOGASSERT(exception_msg.c_str());
        e.dump(); // NOTHING
        e.raise();
    }

    return _started;
}

void NX::drv::serv_flt::stop() noexcept
{
    if (_started) {
        FLTMAN.StopFiltering(_fltman);
        SetEvent(_stop_event);
        std::for_each(_threads.begin(), _threads.end(), [&](std::thread& t) {
            if (t.joinable()) {
                t.join();
            }
        });
        _threads.clear();
        ResetEvent(_stop_event);
        _started = false;
    }
}

unsigned long NX::drv::serv_flt::set_saveas_forecast(unsigned long process_id, const wchar_t* source, const wchar_t* target) noexcept
{
    return FLTMAN.SetSaveAsForecast(_fltman, process_id, source, target);
}

unsigned long NX::drv::serv_flt::set_policy_changed() noexcept
{
    return FLTMAN.SetPolicyChanged(_fltman);
}

unsigned long NX::drv::serv_flt::set_keys(const void* key_blob, unsigned long size) noexcept
{
    return FLTMAN.UpdateKeyChain(_fltman, (const NXRM_KEY_BLOB*)key_blob, size);
}

static std::wstring get_process_commandline(unsigned long process_id)
{
    HANDLE h = NULL;
    std::wstring commandline;

    typedef NTSTATUS (WINAPI* NtQueryInformationProcess_t)(
        _In_      HANDLE           ProcessHandle,
        _In_      PROCESSINFOCLASS ProcessInformationClass,
        _Out_     PVOID            ProcessInformation,
        _In_      ULONG            ProcessInformationLength,
        _Out_opt_ PULONG           ReturnLength
    );

    static NtQueryInformationProcess_t PtrNtQueryInformationProcess = (NtQueryInformationProcess_t)::GetProcAddress(LoadLibraryW(L"ntdll.dll"), "NtQueryInformationProcess");

    if (NULL == PtrNtQueryInformationProcess) {
        return commandline;
    }
    
    do {

        PROCESS_BASIC_INFORMATION pbi = {0};
        ULONG_PTR returned_length = 0;

        h = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id);
        if (NULL == h) {
            break;
        }

        LONG status = PtrNtQueryInformationProcess(h, ProcessBasicInformation, &pbi, sizeof(pbi), (PULONG)&returned_length);
        if (0 != status) {
            break;
        }

        PEB peb = {0};
        if (!ReadProcessMemory(h, pbi.PebBaseAddress, &peb, sizeof(PEB), &returned_length)) {
            break;
        }

        RTL_USER_PROCESS_PARAMETERS upp = { 0 };
        if (!ReadProcessMemory(h, peb.ProcessParameters, &upp, sizeof(RTL_USER_PROCESS_PARAMETERS), &returned_length)) {
            break;
        }

        if (0 == upp.CommandLine.Length) {
            break;
        }

        std::vector<wchar_t> buf;
        buf.resize((upp.CommandLine.Length + 3) / 2, 0);
        if (!ReadProcessMemory(h, upp.CommandLine.Buffer, buf.data(), upp.CommandLine.Length, &returned_length)) {
            break;
        }

        commandline = buf.data();

    } while(false);

    if (h != NULL) {
        CloseHandle(h);
        h = NULL;
    }

    return std::move(commandline);
}

void NX::drv::serv_flt::on_check_rights(flt_request* request)
{
    const NXRM_CHECK_RIGHTS_NOTIFICATION* req = (const NXRM_CHECK_RIGHTS_NOTIFICATION*)request->request();
    NXRMFLT_CHECK_RIGHTS_REPLY  reply = { 0 };
    NX::EVAL::eval_object eval_obj;
    bool evaluated = false;
    bool remote_eval = false;
    bool not_cache = false;

    NX::EVAL::rights init_app_rights;   // granted by app-white-list
    std::wstring     app_name;
    bool             session_0 = false;

    try {

        // get process/host information
        process_info pi = GLOBAL.drv_core().query_process_info(req->ProcessId);

        {
            unsigned long local_session_id = -1;
            if (!ProcessIdToSessionId(req->ProcessId, &local_session_id)) {
                LOGWARN(GetLastError(), L"fail to get session id from requestor's process id (%d, %s).", req->ProcessId, pi.image().c_str());
            }
            if(pi.session_id() != local_session_id) {
                LOGWARN(ERROR_NOT_FOUND, L"session id not match (%d v.s. %d)\r\n\t --> requestor's process id (%d, %s).", local_session_id, pi.session_id(), req->ProcessId, pi.image().c_str());
            }
        }

        if (!pi.is_session_id_valid()) {
            not_cache = true;
            LOGWARN(ERROR_NOT_FOUND, L"Evaluation is ignored because fail to get session id from requestor's process id (%d, %s)\r\n\t(file-->%s).", req->ProcessId, pi.image().c_str(), req->FileName);
            throw std::exception("fail to get session id from process id");
        }

        // check application-whitelist
        const wchar_t* pwz_app_name = wcsrchr(pi.image().c_str(), L'\\');
        app_name = (NULL == pwz_app_name) ? pi.image().c_str() : (pwz_app_name + 1);
        if (!GLOBAL.whitelist_info().empty()) {
            init_app_rights = GLOBAL.whitelist_info().check_app(app_name, pi.publisher());
        }

        if (0 == pi.session_id()) {
            // Currently we don't handle any service (which is in session 0)
            session_0 = true;
            eval_obj.result()->grant_rights(init_app_rights.get());
            const std::wstring& final_rights_list = eval_obj.result()->final_rights().serialize();
            LOGDBG(L"Evaluate:");
            LOGDBG(L"  type: local (session 0)");
            LOGDBG(L"  result: succeed");
            LOGDBG(L"  attributes");
            LOGDBG(L"    - application.pid : %d", req->ProcessId);
            LOGDBG(L"    - application.image : %s", pi.image().c_str());
            LOGDBG(L"    - application.publisher : %s", pi.publisher().c_str());
            LOGDBG(L"    - resource.fso.path : %s", req->FileName);
            LOGDBG(L"  grant rights: %s", final_rights_list.c_str());
            throw std::exception("session 0");
        }

        std::shared_ptr<NX::session> sp = GLOBAL.serv_session().get(pi.session_id());
        if (sp == NULL) {
            not_cache = true;
            LOGWARN(ERROR_NOT_FOUND, L"Evaluation is ignored because session (%d) not exist.\r\n\t process: %d, %s\r\n\t file:%s", pi.session_id(), req->ProcessId, pi.image().c_str(), req->FileName);
            throw std::exception("session not exist");
        }

#define DISABLE_DWM_CHECK   TRUE
#ifndef DISABLE_DWM_CHECK
        if (!sp->is_dwm_enabled()) {
            // If DWM is not enabled, our enforcer cannot show overlay on document
            // In this case, we don't allow user to view any document
            static std::wstring last_denied_file;
            not_cache = true;
            std::wstring title = nudf::util::res::LoadMessage(GLOBAL.res_module(), IDS_PRODUCT_NAME, 256, LANG_NEUTRAL, L"NextLabs Rights Management");
            std::wstring info = L"NextLabs Rights Management requires Windows Desktop Composition to be enabled.  Contact your system administrator.";
            if (0 != _wcsicmp(last_denied_file.c_str(), req->FileName)) {
                last_denied_file = req->FileName;
                LOGDBG(L"Desktop composition is off, fail to open file %s", req->FileName);
            }
            sp->notify(title, info);
            throw std::exception("Desktop composition is off");
        }
#endif

        bool is_system_app = (boost::iends_with(pi.image(), L"\\explorer.exe")
                              || boost::iends_with(pi.image(), L"\\iexplore.exe")
                              || boost::iends_with(pi.image(), L"\\svchost.exe")
                              || boost::iends_with(pi.image(), L"\\searchindexer.exe")
                              || boost::iends_with(pi.image(), L"\\nxrmserv.exe")
                              || boost::iends_with(pi.image(), L"\\nxrmtray.exe")
                              );

        // subject
        //  --> user
        if (sp->profile().is_external_authn()) {
            if (!sp->logged_on()) {
                not_cache = true;
                if (!is_system_app) {
                    std::wstring title = nudf::util::res::LoadMessage(GLOBAL.res_module(), IDS_PRODUCT_NAME, 64, LANG_NEUTRAL, L"NextLabs Rights Management");
                    std::wstring info = nudf::util::res::LoadMessageEx(GLOBAL.res_module(), IDS_NOTIFY_LOGON_REQUIRED, 1024, LANG_NEUTRAL, L"This document is protected by NextLabs Rights Management.\nTo access this document, you must log in.");
                    sp->notify(title, info);
                    sp->show_logon_ui();
                    LOGDBG(L"Evaluation is ignored because has not logged on.\r\n\t WinUser: %s\r\n\tProcess: %s\r\n\tFile: %s", sp->user().best_name().c_str(), pi.image().c_str(), req->FileName);
                }
                throw std::exception("session not log on yet");
            }
            eval_obj.attributes().insert(L"user.id", NX::EVAL::value_object(sp->profile().token().user_id(), false));
            eval_obj.attributes().insert(L"user.name", NX::EVAL::value_object(sp->profile().token().full_user_name(), false));
            if (!GLOBAL.register_info().tenant_id().empty()) {
                eval_obj.attributes().insert(L"user.tenant_id", NX::EVAL::value_object(GLOBAL.register_info().tenant_id(), false));
            }
            eval_obj.set_user_context(GLOBAL.policy_bundle().user_group(sp->profile().token().user_id()).first);
        }
        else {
            // internal authn
            eval_obj.attributes().insert(L"user.id", NX::EVAL::value_object(sp->user().id(), false));
            eval_obj.attributes().insert(L"user.name", NX::EVAL::value_object(sp->user().is_domain_user() ? sp->user().principle_name() : sp->user().name(), false));
            if (!GLOBAL.register_info().tenant_id().empty()) {
                eval_obj.attributes().insert(L"user.tenant_id", NX::EVAL::value_object(GLOBAL.register_info().tenant_id(), false));
            }
            eval_obj.set_user_context(GLOBAL.policy_bundle().user_group(sp->user().id()).first);
        }
        //  --> application
        eval_obj.attributes().insert(L"application.pid", NX::EVAL::value_object((int)req->ProcessId));
        eval_obj.attributes().insert(L"application.name", NX::EVAL::value_object(app_name, false));
        eval_obj.attributes().insert(L"application.path", NX::EVAL::value_object(pi.image(), false));
        eval_obj.attributes().insert(L"application.publisher", NX::EVAL::value_object(pi.publisher(), false));
        //  --> host
        std::vector<std::wstring> ipv4_list = NX::sys::hardware::get_active_ipv4();
        eval_obj.attributes().insert(L"host.name", NX::EVAL::value_object(GLOBAL.host().in_domain() ? GLOBAL.host().fully_qualified_domain_name() : GLOBAL.host().name(), false));
        for (auto it = ipv4_list.begin(); it != ipv4_list.end(); ++it) {
            eval_obj.attributes().insert(L"host.inet_addr", NX::EVAL::value_object(*it, false));
        }
        // environment
        FILETIME ft = { 0, 0 };
        GetSystemTimeAsFileTime(&ft);
        eval_obj.attributes().insert(L"environment.current_time.identity", NX::EVAL::value_object(ft, false));
        unsigned int   time_since_last_heartbeat = 0xFFFFFFFF;
        if (GLOBAL.agent_info().is_heartbeat_time_valid()) {
            nudf::time::CTime ct(&ft);
            nudf::time::CTime hbt(GLOBAL.agent_info().heartbeat_time());
            time_since_last_heartbeat = (unsigned long)(ct.ToSecondsSince1970Jan1st() - hbt.ToSecondsSince1970Jan1st());
        }
        eval_obj.attributes().insert(L"environment.time_since_last_heartbeat", NX::EVAL::value_object((long long)time_since_last_heartbeat));
        // resource
        std::wstring file_path = req->FileName;
        //   --> file path
        if (boost::algorithm::iends_with(file_path, L".nxl")) {
            file_path.substr(0, file_path.length() - 4); // remove .nxl extension
        }
        eval_obj.attributes().insert(L"resource.fso.path", NX::EVAL::value_object(file_path, false));
        const wchar_t* fnp = wcsrchr(file_path.c_str(), L'\\');
        std::wstring file_name = (NULL == fnp) ? file_path : (fnp + 1);
        eval_obj.attributes().insert(L"resource.fso.name", NX::EVAL::value_object(file_name, false));
        fnp = wcsrchr(file_name.c_str(), L'.');
        std::wstring file_extension = (NULL == fnp) ? L"" : (fnp + 1);
        eval_obj.attributes().insert(L"resource.fso.type", NX::EVAL::value_object(file_extension, false));
        //   --> file tags
        std::map<std::wstring, std::wstring>        nxl_attributes;
        std::wstring                                nxl_templates;
        std::multimap<std::wstring, std::wstring>   nxl_tags;
        if(load_file_attributes(pi.session_id(), req->FileName, nxl_attributes, nxl_templates, nxl_tags)) {
            for (auto it = nxl_tags.begin(); it != nxl_tags.end(); ++it) {
                std::wstring tag_name = L"resource.fso." + (*it).first;
                std::wstring tag_value = (*it).second;
                std::transform(tag_name.begin(), tag_name.end(), tag_name.begin(), tolower);
                std::transform(tag_value.begin(), tag_value.end(), tag_value.begin(), tolower);
                eval_obj.attributes().insert(tag_name, NX::EVAL::value_object(tag_value, false));
            }
        }
        else {
            not_cache = true;
        }

        try {
            if (!nxl_templates.empty()) {
                NX::web::json::value v = NX::web::json::value::parse(nxl_templates);
                if (v.is_object()) {
                    std::wstring type = v[L"type"].as_string();
                    if (0 == _wcsicmp(type.c_str(), L"remote only")) {
                        remote_eval = true;
                    }
                }
            }
        }
        catch (std::exception& e) {
            UNREFERENCED_PARAMETER(e);
            remote_eval = false;
        }

//#define FORCE_REMOTE_EVAL 1
#ifdef FORCE_REMOTE_EVAL
        remote_eval = true;
#endif
        const bool is_explorer = (0 == _wcsicmp(app_name.c_str(), L"explorer.exe")) ? true : false;

        // evaluation
        if (remote_eval) {
            if (GLOBAL.connected()) {
                if (GLOBAL.rest().request_eval(eval_obj, eval_obj.result().get())) {
                    evaluated = true;
                }
            }
            else {
                LOGWARN(0, L"remote evaluation failed because no connection to RMS");
            }
        }
        else {
            const bool is_rundll32 = (0 == _wcsicmp(app_name.c_str(), L"rundll32.exe")) ? true : false;
            bool is_nxrmshell = false;
            bool is_outlook_temp_file = false;
            bool is_ie_temp_file = false;
            bool is_chrome_temp_file = false;
            bool is_firefox_temp_file = false;

            if (is_rundll32) {
                // Check if the DLL being run is nxrmshell.dll.
                const std::wstring cmdline = get_process_commandline(req->ProcessId);
                if (!cmdline.empty()) {
                    int argc;
                    LPWSTR* const argv = CommandLineToArgvW(cmdline.c_str(), &argc);
                    if (NULL != argv) {
                        if (argc >= 2) {
                            const std::wstring dll_arg = argv[1];
                            is_nxrmshell = boost::ifind_first(dll_arg, L"nxrmshell.dll");
                        }
                        LocalFree(argv);
                    }
                }

                if (is_nxrmshell) {
                    HRESULT hResult;
                    const std::wstring defaultUserDirStr(L"\\Default\\");
                    size_t pos;

                    // Check if file is under Outlook temp folder.
                    PWSTR pszInternetCachePath;
                    hResult = SHGetKnownFolderPath(FOLDERID_InternetCache, KF_FLAG_DEFAULT, (HANDLE) -1, &pszInternetCachePath);
                    if (FAILED(hResult)) {
                        LOGWARN(hResult, L"Evaluation is ignored because fail to get InternetCache folder path, error 0x%08lX", hResult);
                        throw std::exception("fail to get InternetCache folder path");
                    }

                    const std::wstring internetCachePathStr(pszInternetCachePath);
                    pos = internetCachePathStr.find(defaultUserDirStr);
                    const std::wstring internetCachePathSuffix = internetCachePathStr.substr(pos + defaultUserDirStr.length() - 1);
                    CoTaskMemFree(pszInternetCachePath);
                    const std::wstring outlook_temp_relpath = internetCachePathSuffix + L"\\Content.Outlook\\";

                    is_outlook_temp_file = boost::ifind_first(file_path, outlook_temp_relpath);

                    // Check if file is under IE temp folder.  This is the folder that IE 11 uses for temp downloaded files.
                    const std::wstring ie_temp_relpath = internetCachePathSuffix + L"\\Content.IE5\\";

                    is_ie_temp_file = boost::ifind_first(file_path, ie_temp_relpath);

                    // Check if file is under Downloads folder.  This is the folder that Chrome 62 uses for temp downloaded files.
                    PWSTR pszDownloadsPath;
                    hResult = SHGetKnownFolderPath(FOLDERID_Downloads, KF_FLAG_DEFAULT, (HANDLE) -1, &pszDownloadsPath);
                    if (FAILED(hResult)) {
                        LOGWARN(hResult, L"Evaluation is ignored because fail to get Downloads folder path, error 0x%08lX", hResult);
                        throw std::exception("fail to get Downloads folder path");
                    }

                    const std::wstring downloadsPathStr(pszDownloadsPath);
                    pos = downloadsPathStr.find(defaultUserDirStr);
                    const std::wstring downloadsPathSuffix = downloadsPathStr.substr(pos + defaultUserDirStr.length() - 1);
                    CoTaskMemFree(pszDownloadsPath);
                    const std::wstring chrome_temp_relpath = downloadsPathSuffix + L"\\";

                    is_chrome_temp_file = boost::ifind_first(file_path, chrome_temp_relpath);

                    // Check if file is under user's temp folder.  This is the folder that Firefox 57 uses for temp downloaded files.
                    PWSTR pszLocalAppDataPath;
                    hResult = SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT, (HANDLE) -1, &pszLocalAppDataPath);
                    if (FAILED(hResult)) {
                        LOGWARN(hResult, L"Evaluation is ignored because fail to get LocalAppData folder path, error 0x%08lX", hResult);
                        throw std::exception("fail to get LocalAppData folder path");
                    }

                    const std::wstring localAppDataPathStr(pszLocalAppDataPath);
                    pos = localAppDataPathStr.find(defaultUserDirStr);
                    const std::wstring localAppDataPathSuffix = localAppDataPathStr.substr(pos + defaultUserDirStr.length() - 1);
                    CoTaskMemFree(pszLocalAppDataPath);
                    const std::wstring firefox_temp_relpath = localAppDataPathSuffix + L"\\Temp\\";

                    is_firefox_temp_file = boost::ifind_first(file_path, firefox_temp_relpath);
                }
            }

            if (is_rundll32 && is_nxrmshell && (is_outlook_temp_file || is_ie_temp_file || is_chrome_temp_file || is_firefox_temp_file)) {
                // Don't grant rights to rundll32.exe and nxrmshell.dll for
                // accessing Outlook / IE / Chrome / Firefox temp files, even
                // if the policies grants rights to rundll32.exe.  This way
                // when nxrmshell.dll is invoked by these apps to copy a
                // temporary protected attachment or temporary downloaded file
                // from their temp folder to NextLabs cache folder, it will
                // always copy the NXL header and ciphertext instead of
                // copying the plaintext.
                //
                // This special handling is needed for Bajaj who whitelists
                // rundll32.exe in order for their DLLs which are run by
                // rundll32.exe to be able to access protected files.
                // (Normally rundll32.exe should not be whitelisted because it
                // should not be trusted.)
                LOGDBG(L"Rights are not granted to rundll32.exe and nxrmshell.dll for accessing Outlook / IE / Chrome / Firefox temp file %s.", file_path.c_str());
            }
            else {
                if (!init_app_rights.empty() || is_explorer) {
                    // since application is in white list, don't need to evaluate it any more
                    eval_obj.set_mask(NX::EVAL::eval_object::eval_mask_user|NX::EVAL::eval_object::eval_mask_host | NX::EVAL::eval_object::eval_mask_res | NX::EVAL::eval_object::eval_mask_env);
                    // initiate rights for this application
                    eval_obj.result()->grant_rights(init_app_rights.get());
                }
                sp->evaluate(eval_obj);
            }
            evaluated = true;
        }

        // put into cache
        if (eval_obj.result()->final_rights().has(BUILTIN_RIGHT_VIEW) && !eval_obj.result()->hit_obligations().empty()) {
            // only store result with overlay obligation
            GLOBAL.eval_cache().set(eval_obj.result()->id(), eval_obj.result());
        }
        
		// update overlay policy flag
		std::shared_ptr<NX::process_status> sp_proc = GLOBAL.process_cache().get_process(req->ProcessId);
		if (nullptr != sp_proc) {
			std::wstring strOverlay = OB_NAME_OVERLAY;
			std::transform(strOverlay.begin(), strOverlay.end(), strOverlay.begin(), ::tolower);
			if (eval_obj.result()->hit_obligations().end() != eval_obj.result()->hit_obligations().find(strOverlay)) {
				sp_proc->remove_flags(NXRM_PROCESS_FLAG_HAS_NO_OVERLAYPOLICY);
			}
			else {
				sp_proc->add_flags(NXRM_PROCESS_FLAG_HAS_NO_OVERLAYPOLICY);
			}
		}
        // final
        reply.EvaluationId = eval_obj.id();
        reply.RightsMask   = eval_obj.result()->final_rights().get();
        if (!evaluated || not_cache) {
            reply.RightsMask |= RIGHTS_NOT_CACHE;
        }
        reply.CustomRights = 0;
        FLTMAN.ReplyMessage(_fltman, request->context(), &reply);

        // Actions for non-explorer processes
        if (!is_explorer) {
            // audit
            GLOBAL.audit_serv().audit(eval_obj);
            sp->audit(eval_obj);

            // change process status
            if (eval_obj.result()->final_rights().has(BUILTIN_RIGHT_VIEW)) {
                // give right to view
                if (sp_proc != nullptr) {
                    // set process rights
                    sp_proc->set_process_rights(eval_obj.result()->final_rights().get());
                    // Set flag
                    if (!sp_proc->is_flag_on(NXRM_PROCESS_FLAG_WITH_NXL_OPENED)) {
                        sp_proc->add_flags(NXRM_PROCESS_FLAG_WITH_NXL_OPENED);
                    }
                    // only update dwm-overlay windows for non-overlay-integrated process
					if (!sp_proc->is_flag_on(NXRM_PROCESS_FLAG_HAS_OVERLAY_INTEGRATION) && !sp_proc->is_flag_on(NXRM_PROCESS_FLAG_HAS_NO_OVERLAYPOLICY)) {
						const std::vector<unsigned long>& wndbuf = GLOBAL.process_cache().get_valid_overlay_windows(pi.session_id());
						if (!wndbuf.empty()) {
                            GLOBAL.drv_core().set_overlay_windows(pi.session_id(), wndbuf);
                        }
                    }
                }
            }
        }

        // Log
        LOGDBG(L"Evaluate:");
        LOGDBG(L"  type: %s", remote_eval ? L"remote" : L"local");
        LOGDBG(L"  result: %s", evaluated ? L"succeed" : L"failed");
        LOGDBG(L"  attributes");
        std::for_each(eval_obj.attributes().begin(), eval_obj.attributes().end(), [&](const NX::EVAL::attribute_multimap::value_type& v) {
            LOGDBG(L"    - %s : %s", v.first.c_str(), v.second.serialize().c_str());
        });
        const std::wstring& final_rights_list = eval_obj.result()->final_rights().serialize();
        LOGDBG(L"  grant rights: %s", final_rights_list.c_str());
        LOGDBG(L"  hit policies");
        if (!init_app_rights.empty()) {
            // initiate rights for this application
            LOGDBG(L"    - WHITELIST:\t %s is in application white list (%s)", app_name.c_str(), init_app_rights.serialize().c_str());
            eval_obj.result()->grant_rights(init_app_rights.get());
        }
        std::for_each(eval_obj.result()->hit_policies().begin(), eval_obj.result()->hit_policies().end(), [&](const std::pair<std::wstring, std::shared_ptr<NX::EVAL::policy>>& v) {
            LOGDBG(L"    - %s :\t%s", v.first.c_str(), v.second->name().c_str());
        });
        LOGDBG(L"  obligations");
        std::for_each(eval_obj.result()->hit_obligations().begin(), eval_obj.result()->hit_obligations().end(), [&](const std::pair<std::wstring, std::shared_ptr<NX::EVAL::obligation>>& v) {
            LOGDBG(L"    - %s", v.first.c_str());
            std::for_each(v.second->parameters().begin(), v.second->parameters().end(), [&](const std::pair<std::wstring, std::wstring>& ob) {
                LOGDBG(L"       {%s: %s}", ob.first.c_str(), ob.second.c_str());
            });
        });
        LOGDBG(L" ");
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        reply.EvaluationId = eval_obj.id();
        reply.RightsMask = 0;
        if (session_0) {
            reply.RightsMask = init_app_rights.get();
        }
        if (not_cache) {
            reply.RightsMask |= RIGHTS_NOT_CACHE;
        }
        reply.CustomRights = 0;
        FLTMAN.ReplyMessage(_fltman, request->context(), &reply);
    }
    
}

void NX::drv::serv_flt::on_block_notification(flt_request* request)
{
    const NXRM_BLOCK_NOTIFICATION* req = (const NXRM_BLOCK_NOTIFICATION*)request->request();
    unsigned long   session_id = -1;
    std::wstring    title;
    std::wstring    info;
    std::wstring    file;
    std::wstring    operation;
    std::wstring    user_operation_name;
    std::wstring    app_name;


    const WCHAR* pwzFileName = wcsrchr(req->FileName, L'\\');
    if (NULL == pwzFileName) pwzFileName = req->FileName;
    else pwzFileName++;
    file = pwzFileName;


    if (!ProcessIdToSessionId(req->ProcessId, &session_id) || ((ULONG)-1) == session_id) {
        return;
    }


    try {
        process_info pi = GLOBAL.drv_core().query_process_info(req->ProcessId);
        app_name = pi.image();
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }

    switch (req->Reason)
    {
    case nxrmfltDeniedWritesOpen:
        title = nudf::util::res::LoadMessage(GLOBAL.res_module(), IDS_PRODUCT_NAME, 256, LANG_NEUTRAL, L"Open in ReadOnly Mode");
        info = nudf::util::res::LoadMessageEx(GLOBAL.res_module(), IDS_NOTIFY_READONLY_MODE, 1024, LANG_NEUTRAL, L"The file (%s) will be opened in read only mode because you don't have write permission", file.c_str());
        user_operation_name = RIGHT_DISP_EDIT;
        break;
    case nxrmfltDeniedSaveAsOpen:
        title = nudf::util::res::LoadMessage(GLOBAL.res_module(), IDS_PRODUCT_NAME, 256, LANG_NEUTRAL, L"Operation Denied");
        operation = nudf::util::res::LoadMessage(GLOBAL.res_module(), IDS_OPERATION_OVERWRITE, 256, LANG_NEUTRAL, L"overwrite");
        info = nudf::util::res::LoadMessageEx(GLOBAL.res_module(), IDS_NOTIFY_OPERATION_DENIED, 1024, LANG_NEUTRAL, L"You don't have permission to %s file %s", operation.c_str(), file.c_str());
        user_operation_name = RIGHT_DISP_SAVEAS;
        break;
    case nxrmfltSaveAsToUnprotectedVolume:
        title = nudf::util::res::LoadMessage(GLOBAL.res_module(), IDS_PRODUCT_NAME, 256, LANG_NEUTRAL, L"Operation Denied");
        info = nudf::util::res::LoadMessageEx(GLOBAL.res_module(), IDS_NOTIFY_OPERATION_SAVEAS_DENIED, 1024, LANG_NEUTRAL, L"File (%s) cannot be saved to unprotected location", file.c_str());
        user_operation_name = RIGHT_DISP_SAVEAS;
        break;
    default:
        title = nudf::util::res::LoadMessage(GLOBAL.res_module(), IDS_PRODUCT_NAME, 256, LANG_NEUTRAL, L"Operation Denied");
        operation = nudf::util::res::LoadMessage(GLOBAL.res_module(), IDS_OPERATION_DEFAULT, 256, LANG_NEUTRAL, L"operate");
        info = nudf::util::res::LoadMessageEx(GLOBAL.res_module(), IDS_NOTIFY_OPERATION_DENIED, 1024, LANG_NEUTRAL, L"You don't have permission to %s file %s", operation.c_str(), file.c_str());
        break;
    }

    std::shared_ptr<NX::session> sp = GLOBAL.serv_session().get(session_id);
    if (sp != NULL) {
        sp->notify(title, info);
        sp->audit(user_operation_name, false, req->FileName, app_name, info);
    }
    else {
        LOGERR(ERROR_INVALID_PARAMETER, L"session (%d) object not found", session_id);
    }

    LOGINF(info.c_str());
}

void NX::drv::serv_flt::on_key_error_notification(flt_request* request)
{
    const NXRM_KEYCHAIN_ERROR_NOTIFICATION* req = (const NXRM_KEYCHAIN_ERROR_NOTIFICATION*)request->request();

    std::wstring title;
    std::wstring info;
    std::wstring file;
    std::wstring key_id = nudf::string::FromBytes<wchar_t>(req->KeyId.Id, req->KeyId.IdSize);
    
    const WCHAR* pwzFileName = wcsrchr(req->FileName, L'\\');
    if (NULL == pwzFileName) pwzFileName = req->FileName;
    else pwzFileName++;
    file = pwzFileName;

    title = nudf::util::res::LoadMessage(GLOBAL.res_module(), IDS_PRODUCT_NAME, 256, LANG_NEUTRAL, L"NextLabs Rights Management");
    info = nudf::util::res::LoadMessageEx(GLOBAL.res_module(), IDS_MSG_UNRECOGNIZED_KEY_ID, 1024, LANG_NEUTRAL, L"File (%s) has an unrecognized key id: %s", file.c_str(), key_id.c_str());

    std::shared_ptr<NX::session> sp = GLOBAL.serv_session().get(req->SessionId);
    if (sp != NULL) {
        sp->notify(title, info);
        sp->audit(RIGHT_DISP_VIEW, false, req->FileName, L"N/A", info);
    }
    else {
        LOGERR(ERROR_INVALID_PARAMETER, L"session (%d) object not found", req->SessionId);
    }

    LOGWARN(ERROR_LOCAL_USER_SESSION_KEY, L"KeyError: %s", info.c_str());
}

void NX::drv::serv_flt::on_purge_cache_notification(flt_request* request)
{
    const NXRM_PURGE_CACHE_NOTIFICATION* req = (const NXRM_PURGE_CACHE_NOTIFICATION*)request->request();
    std::wstring file(req->FileName);
    if (!file.empty()) {
        std::transform(file.begin(), file.end(), file.begin(), tolower);
        //gpEngine->GetPolicyServ()->RemoveResource(wsFile);
    }
}

void NX::drv::serv_flt::on_process_notification(flt_request* request)
{
    const NXRM_PROCESS_NOTIFICATION* req = (const NXRM_PROCESS_NOTIFICATION*)request->request();
    if (0 == req->Create) {

        const bool is_overlay_related = GLOBAL.process_cache().process_has_overlay_window(req->ProcessId);

        // Process Exit
        GLOBAL.process_cache().remove_process(req->ProcessId);

        if (0 != req->SessionId && is_overlay_related) {
            const std::vector<unsigned long>& wndbuf = GLOBAL.process_cache().get_valid_overlay_windows(req->SessionId);
            GLOBAL.drv_core().set_overlay_windows(req->SessionId, wndbuf);
        }
    }
    else {
        // Process Created
        GLOBAL.process_cache().add_process(req->ProcessId, req->SessionId, req->Flags, std::wstring(), std::wstring());
    }
}

bool  NX::drv::serv_flt::load_file_attributes(unsigned long session_id,
                                              const std::wstring& file,
                                              std::map<std::wstring, std::wstring>& attributes,
                                              std::wstring& templates,
                                              std::multimap<std::wstring, std::wstring>& tags
                                              )
{
    bool result = false;
    HANDLE  h = INVALID_HANDLE_VALUE;
    unsigned long last_error = 0;
    HANDLE token = NULL;
    bool impersonated = false;

    try {

        h = ::CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (INVALID_HANDLE_VALUE == h) {
            last_error = GetLastError();
            if (ERROR_ACCESS_DENIED != last_error) {
                LOGWARN(last_error, L"Fail to open target file (%d): %s", last_error, file.c_str());
                throw std::exception("Fail to open file");
            }
            // try to impersonate
            if (!WTSQueryUserToken(session_id, &token)) {
                // Fail to get current session token
                // log & Exit
                last_error = GetLastError();
                LOGWARN(GetLastError(), L"Fail to get session token");
                throw std::exception("Fail to get session token");
            }
            if (!ImpersonateLoggedOnUser(token)) {
                // Fail to impersonate current user
                // log & Exit
                LOGWARN(GetLastError(), L"Fail to impersonate logon user");
                throw std::exception("Fail to impersonate logon user");
            }

            impersonated = true;

            h = ::CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (INVALID_HANDLE_VALUE == h) {
                last_error = GetLastError();
                LOGWARN(last_error, L"Fail to open target file (%d): %s", last_error, file.c_str());
                throw std::exception("Fail to open file");
            }
        }

        assert(INVALID_HANDLE_VALUE != h);

        // load tags
        NX::NXL::nxl_header header;
        try {
            header.load(h);

#ifdef _DEBUG
            // Print detail NXL Header
            const  NXRM_KEY_BLOB* blob = (const  NXRM_KEY_BLOB*)GLOBAL._nxl_keys_blob.data();
            const int blob_count = (int)(GLOBAL._nxl_keys_blob.size()/ sizeof(NXRM_KEY_BLOB));
            std::vector<unsigned char> k_e_key_id;
            std::vector<unsigned char> k_e_key;
            std::vector<unsigned char> c_e_key;
            unsigned long st_checksum_infile = 0;
            unsigned long st_checksum_real = header.calc_sections_checksum();
            for (int i = 0; i < blob_count; i++) {
                if (blob[i].KeKeyId.IdSize == header.primary_key_id().size() && 0 == memcmp(blob[i].KeKeyId.Id, header.primary_key_id().id(), header.primary_key_id().size())) {
                    k_e_key_id = std::vector<unsigned char>(blob[i].KeKeyId.Id, blob[i].KeKeyId.Id + blob[i].KeKeyId.IdSize);
                    k_e_key = std::vector<unsigned char>(blob[i].Key, blob[i].Key+32);
                    c_e_key = header.decrypt_content_key(k_e_key);
                    st_checksum_infile = header.decrypt_sections_checksum(c_e_key);
                }
            }

            std::wstring s_k_e_key_id = nudf::string::FromBytes<wchar_t>(k_e_key_id.data(), (unsigned long)k_e_key_id.size());
            std::wstring s_k_e_key = nudf::string::FromBytes<wchar_t>(k_e_key.data(), (unsigned long)k_e_key.size());
            std::wstring s_c_e_key = nudf::string::FromBytes<wchar_t>(c_e_key.data(), (unsigned long)c_e_key.size());

            LOGDBG(L"NXL HEADER");
            LOGDBG(L"  Basic");
            LOGDBG(L"    - Thumbprint: %s", header.thumbprint().c_str());
            LOGDBG(L"    - Version: %08X", header.version());
            LOGDBG(L"    - Flags: %08X", header.flags());
            LOGDBG(L"  Crypto");
            LOGDBG(L"    - Algorithm: %s", header.is_content_key_aes256() ? L"AES 256" : L"AES 128");
            LOGDBG(L"    - Primary Key:");
            LOGDBG(L"       * Id: %s", s_k_e_key_id.c_str());
            LOGDBG(L"       * Key: %s", s_k_e_key.c_str());
            LOGDBG(L"    - Content Key:");
            LOGDBG(L"       * Key: %s", s_c_e_key.c_str());
            LOGDBG(L"  Sections");
            LOGDBG(L"    - Count: %d", (int)header.sections().size());
            LOGDBG(L"    - Checksum: %08X (Real: %08X)", st_checksum_infile, st_checksum_real);
#endif

        }
        catch (std::exception& e) {
            LOGDBG(L"fail to load NXL header from file: %s:%s", file.c_str(), e.what());
            header.clear();
            throw e;
        }
		try {
			if (!header.empty()) {
				bool validated = false;
				attributes = header.load_section_attributes(h, &validated);
				if (!validated) {
					LOGERR(ERROR_INVALID_DATA, L"wrong checksum in section 'attributes' (%s)", file.c_str());
				}
				templates = header.load_section_templates(h, &validated);
				if (!validated) {
					LOGERR(ERROR_INVALID_DATA, L"wrong checksum in section 'templates' (%s)", file.c_str());
				}
				tags = header.load_section_tags(h, &validated);
				if (!validated) {
					LOGERR(ERROR_INVALID_DATA, L"wrong checksum in section 'tags' (%s)", file.c_str());
				}
			}

		}
		catch (const std::exception& e) {
			LOGDBG(L"fail to load NXL section data from file: %s", file.c_str());
			attributes.clear();
			templates.clear();
			tags.clear();
			throw e;
		}

        // succeed
        result = true;
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        result = false;
        attributes.clear();
    }

    if (NULL != token) {
        if (impersonated) {
            RevertToSelf();
            impersonated = false;
        }
        CloseHandle(token);
        token = NULL;
    }

    if (INVALID_HANDLE_VALUE != h) {
        CloseHandle(h);
        h = INVALID_HANDLE_VALUE;
    }

    return result;
}

void NX::drv::serv_flt::worker(serv_flt* serv) noexcept
{
    HANDLE events[2] = { serv->_stop_event, serv->_request_event };

    try {

        while (true) {

            unsigned wait_result = ::WaitForMultipleObjects(2, events, FALSE, INFINITE);
            if (wait_result == WAIT_OBJECT_0) {
                // stop event
                return;
            }

            assert(wait_result == (WAIT_OBJECT_0 + 1));
            if (wait_result != (WAIT_OBJECT_0 + 1)) {
                // should never reach here!!!
                return;
            }

            // a new request comes
            std::shared_ptr<flt_request> request;
            ::EnterCriticalSection(&serv->_list_lock);
            if (!serv->_list.empty()) {
                request = serv->_list.front();
                serv->_list.pop_front();
            }
            else {
                ResetEvent(serv->_request_event);
            }
            ::LeaveCriticalSection(&serv->_list_lock);

            if (request == NULL) {
                continue;
            }
            if (request->empty()) {
                continue;
            }

            // handle this request
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
                break;
            }

            // done
        }
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

unsigned long __stdcall NX::drv::serv_flt::drv_callback(unsigned long type, void* msg, unsigned long length, void* msg_context, void* user_context)
{
    unsigned long result = 0;
    NX::drv::serv_flt* serv = (NX::drv::serv_flt*)user_context;

    if (!GLOBAL.is_running()) {
        LOGDBG(L"filetr driver request (type == %d) comes in when service is not fully started", type);
        return ERROR_SERVICE_NOT_ACTIVE;
    }

    std::shared_ptr<flt_request> request(new flt_request(type, msg, length, msg_context));
    if (request != NULL && !request->empty()) {
        ::EnterCriticalSection(&serv->_list_lock);
        serv->_list.push_back(request);
        SetEvent(serv->_request_event);
        ::LeaveCriticalSection(&serv->_list_lock);
    }
    else {
        result = GetLastError();
        if (0 == result) result = ERROR_INVALID_PARAMETER;
    }

    return result;
}

BOOL __stdcall NX::drv::serv_flt::check_log_accept(unsigned long level)
{
    return GLOBAL.log().AcceptLevel((LOGLEVEL)level);
}

LONG __stdcall NX::drv::serv_flt::write_log(const wchar_t* msg)
{
    return GLOBAL.log().Push(msg);
}



