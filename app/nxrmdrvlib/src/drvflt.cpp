

#include <Windows.h>
#include <assert.h>

// from fltman
#include "nxrmflt.h"
#include "nxrmfltman.h"

#include "drvflt.hpp"


using namespace NX;


#define ID_CREATE_MANAGER       1
#define ID_REPLY_MESSAGE        2
#define ID_START_FILTERING      3
#define ID_STOP_FILTERING       4
#define ID_CLOSE_MANAGER        5
#define ID_SET_SAVEAS_FORECAST  6
#define ID_SET_POLICY_CHANGED   7
#define ID_UPDATE_KEYCHAIN      8


drvflt_man_instance::drvflt_man_instance() : dll_instance<FLTMAN_FUNCTION_NUMBER>(std::vector<function_item>({
    function_item(ID_CREATE_MANAGER, "CreateManager"),
    function_item(ID_REPLY_MESSAGE, "ReplyMessage"),
    function_item(ID_START_FILTERING, "StartFiltering"),
    function_item(ID_STOP_FILTERING, "StopFiltering"),
    function_item(ID_CLOSE_MANAGER, "CloseManager"),
    function_item(ID_SET_SAVEAS_FORECAST, "SetSaveAsForecast"),
    function_item(ID_SET_POLICY_CHANGED, "SetPolicyChanged"),
    function_item(ID_UPDATE_KEYCHAIN, "UpdateKeyChain")
}))
{
}

drvflt_man_instance::~drvflt_man_instance()
{
}

void drvflt_man_instance::load(const std::wstring& dll_file)
{
    dll_instance::load(dll_file);
}

HANDLE drvflt_man_instance::create_manager(void* notify_callback, void* log_callback, void* loglevel_callback, void* key_blob, unsigned long key_blob_size, void* context)
{
    typedef HANDLE(WINAPI* NXRMFLT_CREATE_MANAGER)(void*, void*, void*, void*, ULONG, void*);
    return EXECUTE(NXRMFLT_CREATE_MANAGER, *this, ID_CREATE_MANAGER, notify_callback, log_callback, loglevel_callback, key_blob, key_blob_size, context);
}

unsigned long drvflt_man_instance::reply_message(HANDLE h, void* context, void* reply)
{
    typedef ULONG(WINAPI* NXRMFLT_REPLY_MESSAGE)(HANDLE, PVOID, void*);
    return EXECUTE(NXRMFLT_REPLY_MESSAGE, *this, ID_REPLY_MESSAGE, h, context, reply);
}

unsigned long drvflt_man_instance::start_filtering(HANDLE h)
{
    typedef ULONG(WINAPI* NXRMFLT_START_FILTERING)(HANDLE);
    return EXECUTE(NXRMFLT_START_FILTERING, *this, ID_START_FILTERING, h);
}

unsigned long drvflt_man_instance::stop_filtering(HANDLE h)
{
    typedef ULONG(WINAPI* NXRMFLT_STOP_FILTERING)(HANDLE);
    return EXECUTE(NXRMFLT_STOP_FILTERING, *this, ID_STOP_FILTERING, h);
}

unsigned long drvflt_man_instance::close_manager(HANDLE h)
{
    typedef ULONG(WINAPI* NXRMFLT_CLOSE_MANAGER)(HANDLE);
    return EXECUTE(NXRMFLT_CLOSE_MANAGER, *this, ID_CLOSE_MANAGER, h);
}

unsigned long drvflt_man_instance::set_save_as_forecast(HANDLE h, unsigned long process_id, _In_opt_ const wchar_t* source_file, _In_ const wchar_t* target_file)
{
    typedef ULONG(WINAPI* NXRMFLT_SET_SAVEAS_FORECAST)(HANDLE, ULONG, const WCHAR*, const WCHAR*);
    return EXECUTE(NXRMFLT_SET_SAVEAS_FORECAST, *this, ID_SET_SAVEAS_FORECAST, h, process_id, source_file, target_file);
}

unsigned long drvflt_man_instance::set_policy_changed(HANDLE h)
{
    typedef ULONG(WINAPI* NXRMFLT_SET_POLICY_CHANGED)(HANDLE);
    return EXECUTE(NXRMFLT_SET_POLICY_CHANGED, *this, ID_SET_POLICY_CHANGED, h);
}

unsigned long drvflt_man_instance::update_key_chain(HANDLE h, void* key_blob, unsigned long key_blob_size)
{
    typedef ULONG(WINAPI* NXRMFLT_UPDATE_KEYCHAIN)(HANDLE, void*, ULONG);
    return EXECUTE(NXRMFLT_UPDATE_KEYCHAIN, *this, ID_UPDATE_KEYCHAIN, h, key_blob, key_blob_size);
}