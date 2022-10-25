

#include <Windows.h>
#include <assert.h>
#include <Wtsapi32.h>

#include <string>
#include <iostream>

#include <nudf\eh.hpp>
#include <nudf\debug.hpp>
#include <nudf\filesys.hpp>
#include <nudf\winutil.hpp>
#include <nudf\string.hpp>
#include <nudf\dbglog.hpp>


#include "global.hpp"


nxrm_global GLOBAL;


nxrm_global::nxrm_global()
{
}

nxrm_global::~nxrm_global()
{
    clear();
}

void nxrm_global::initialize()
{
    init_dirs();

    _file_version = NX::win::file_version(_module_path);
    assert(_file_version.is_application());
}

void nxrm_global::clear()
{
}

void nxrm_global::init_dirs()
{
    NX::fs::module_path mod_path(NULL);
    NX::fs::dos_filepath parent_dir(mod_path.file_dir());

    _module_path = mod_path.path();
    _product_dir = parent_dir.file_dir();
    _bin_dir = parent_dir.path();
#ifdef _AMD64_
    _bin32_dir = _bin_dir + L"\\x86";
#endif
    _working_dir = _bin_dir;
    _config_dir = _product_dir + L"\\conf";
    _profiles_dir = _product_dir + L"\\profiles";
}