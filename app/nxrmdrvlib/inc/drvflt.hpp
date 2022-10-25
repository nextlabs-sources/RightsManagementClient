
#pragma once
#ifndef __NXRMDRV_FLT_HPP__
#define __NXRMDRV_FLT_HPP__

#include <string>
#include "function_dll.hpp"

namespace NX {

// it exports 8 functions
#define FLTMAN_FUNCTION_NUMBER  8


class drvflt_man_instance : public dll_instance<FLTMAN_FUNCTION_NUMBER>
{
public:
    drvflt_man_instance();
    virtual ~drvflt_man_instance();

    virtual void load(const std::wstring& file);
    
    // functions
    HANDLE create_manager(void* notify_callback, void* log_callback, void* loglevel_callback, void* key_blob, unsigned long key_blob_size, void* context);
    unsigned long reply_message(HANDLE h, void* context, void* reply);
    unsigned long start_filtering(HANDLE h);
    unsigned long stop_filtering(HANDLE h);
    unsigned long close_manager(HANDLE h);
    unsigned long set_save_as_forecast(HANDLE h, unsigned long process_id, _In_opt_ const wchar_t* source_file, _In_ const wchar_t* target_file);
    unsigned long set_policy_changed(HANDLE h);
    unsigned long update_key_chain(HANDLE h, void* key_blob, unsigned long key_blob_size);
};


}


#endif