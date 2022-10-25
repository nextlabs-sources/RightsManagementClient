

#include <Windows.h>
#include <assert.h>


#include <nudf\exception.hpp>
#include <nudf\registry.hpp>
#include <nudf\string.hpp>

#include "nxrmeng.h"
#include "nxrminfo.hpp"



CNxProdInfo::CNxProdInfo() : _langid(0), _prodname(L"NextLabs Rights Management")
{
    memset(&_installdate, 0, sizeof(_installdate));
    _prodinitial = L"NXRMC/8.0";
}

CNxProdInfo::~CNxProdInfo()
{
    Clear();
}

void CNxProdInfo::Load() throw()
{
    WCHAR   wzSubKeyName[MAX_PATH] = {0};
    ULONG   i = 0;
    REGSAM  samDesired = KEY_READ;

    nudf::win::CRegKey regkey;
    nudf::win::CRegLocalMachine reglm;

    samDesired |= KEY_WOW64_64KEY;
    if(!regkey.Open(reglm, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", samDesired)) {
        return;
    }

    while(0 == RegEnumKeyW(regkey, i++, wzSubKeyName, MAX_PATH-1)) {

        nudf::win::CRegKey regsubkey;
        std::wstring name;
        std::wstring installdate;
        std::wstring installdir;
        std::wstring prodversion;
        std::wstring publisher;
        DWORD dwLangId = 0;
        WCHAR wzLocale[LOCALE_NAME_MAX_LENGTH+1] = {0};

        if(!regsubkey.Open(regkey, wzSubKeyName, samDesired)) {
            continue;
        }

        // Get following information from subkey
        regsubkey.GetValue(L"DisplayName", name, NULL);
        if(0 != _wcsicmp(name.c_str(), _prodname.c_str())) {
            continue;
        }
        regsubkey.GetValue(L"InstallLocation", installdir, NULL);
        if(installdir.empty()) {
            continue;
        }
        if(L'\\' == installdir.c_str()[installdir.length()-1]) {
            // end with L'\'
            installdir = installdir.substr(0, installdir.length()-1);
        }
        regsubkey.GetValue(L"DisplayVersion", prodversion, NULL);
        if(prodversion.empty()) {
            continue;
        }
        regsubkey.GetValue(L"Publisher", publisher, NULL);
        if(publisher.empty()) {
            continue;
        }
        regsubkey.GetValue(L"InstallDate", installdate, NULL);
        if(installdate.empty()) {
            continue;
        }
        if(!regsubkey.GetValue(L"Language", &dwLangId)) {
            continue;
        }
        if(0 != LCIDToLocaleName(MAKELCID(_langid, SORT_DEFAULT), wzLocale, LOCALE_NAME_MAX_LENGTH, 0)) {
            wzLocale[0] = L'\0';
        }

        // Good, we found this product
        _guid = wzSubKeyName;
        _prodversion = prodversion;
        _publisher = publisher;
        _installdir = installdir;
        _localename = wzLocale;
        _langid = dwLangId;
        if(installdate.length() == 8) {
            std::wstring s;
            int n = 0;
            // Year
            s = std::wstring(installdate.c_str(), 4);
            if(nudf::string::ToInt<wchar_t>(s, &n)) {
                _installdate.wYear = (WORD)n;
            }
            // Month
            s = std::wstring(installdate.c_str()+4, 2);
            if(nudf::string::ToInt<wchar_t>(s, &n)) {
                _installdate.wMonth = (WORD)n;
            }
            // Day
            s = std::wstring(installdate.c_str()+6, 2);
            if(nudf::string::ToInt<wchar_t>(s, &n)) {
                _installdate.wDay = (WORD)n;
            }
        }

        // 
        break;
    }
}

void CNxProdInfo::Clear() throw()
{
    _guid.clear();
    _publisher.clear();
    _prodname.clear();
    _prodversion.clear();
    _installdir.clear();
    memset(&_installdate, 0, sizeof(_installdate));
    _localename.clear();
    _langid = 0;
}


//
//  class CNxPaths
//

CNxPaths::CNxPaths()
{
}

CNxPaths::~CNxPaths()
{
    Clear();
}

void CNxPaths::Load() throw()
{
    WCHAR  wzPath[MAX_PATH + 1] = {0};
    WCHAR* pos = NULL;

    LoadSysPaths();

    GetModuleFileNameW((HMODULE)_Instance, wzPath, MAX_PATH);

    // Get Engine Dll Path
    pos = wcsrchr(wzPath, L'\\');
    assert(NULL != pos);
    assert(0 == _wcsicmp(pos, L"\\nxrmeng.dll"));
    _engdll = wzPath;
    *pos = L'\0';

    // Get Bin Path and Root Path
    pos = wcsrchr(wzPath, L'\\');
    assert(NULL != pos);
    assert(0 == _wcsicmp(pos, L"\\bin"));
    _bin = wzPath;
    *pos = L'\0';
    _root = wzPath;

    // Good, set others
    _conf   = _root + L"\\conf";
    _log    = _root + L"\\log";
    _cache  = _root + L"\\cache";
    
    // Make sure cache folder exists
    if(INVALID_FILE_ATTRIBUTES == GetFileAttributesW(_cache.c_str())) {
        ::CreateDirectoryW(_cache.c_str(), NULL);
    }

    _coredrv= _sys32 + L"\\drivers\\nxrmdrv.sys";
    _fltdrv = _sys32 + L"\\drivers\\nxrmflt.sys";
    _coredll= _sys32 + L"\\nxrmcore.dll";

    _drvmandll      = _bin + L"\\nxrmdrvman.dll";
    _fltmandll      = _bin + L"\\nxrmfltman.dll";
    _coreaddindll   = _bin + L"\\nxrmcoreaddin.dll";
    _engdll         = _bin + L"\\nxrmeng.dll";  
    _resdll         = _bin + L"\\nxrmres.dll"; 
    _overlaydll     = _bin + L"\\nxrmoverlay.dll";
    _shelldll       = _bin + L"\\nxrmshell.dll";
    _cmudll         = _bin + L"\\nxrmcmui.dll";
    _servexe        = _bin + L"\\nxrmserv.exe";
    _trayexe        = _bin + L"\\nxrmtray.exe";
    
    _regconf       = _conf + L"\\register.xml";
    _classifyconf   = _conf + L"\\classify.xml";
    _profile        = _conf + L"\\profile.xml";
    _keyconf        = _conf + L"\\keys.bin";
    _policyconf     = _conf + L"\\policy.bin";
    _agentcert      = _conf + L"\\agent.cer";
    _securebin      = _conf + L"\\secure.bin";
    
#ifdef _WIN64
    _coredll        = _sys32 + L"\\nxrmcore64.dll";
    _core32dll      = _wowsys32 + L"\\nxrmcore.dll";
    _bin32          = _bin + L"\\x86";
    _coreaddin32dll = _bin32 + L"\\nxrmcoreaddin.dll";
    _res32dll       = _bin32 + L"\\nxrmres.dll";
    _cmu32dll       = _bin32 + L"\\nxrmcmui.dll";
#endif
}

void CNxPaths::Clear() throw()
{
    _win.clear();
    _wintemp.clear();
    _sys32.clear();
    _root.clear();
    _bin.clear();
    _conf.clear();
    _log.clear();
    _cache.clear();
    _coredrv.clear();
    _fltdrv.clear();
    _fltmandll.clear();
    _drvmandll.clear();
    _coredll.clear();
    _coreaddindll.clear();
    _engdll.clear();
    _resdll.clear();
    _overlaydll.clear();
    _shelldll.clear();
    _cmudll.clear();
    _servexe.clear();
    _trayexe.clear();
    
    _regconf.clear();
    _classifyconf.clear();
    _profile.clear();
    _keyconf.clear();
    _policyconf.clear();
    _agentcert.clear();
    _securebin.clear();

#ifdef _WIN64
    _wowsys32.clear();
    _bin32.clear();
    _core32dll.clear();
    _coreaddin32dll.clear();
    _res32dll.clear();
    _cmu32dll.clear();
#endif
}

void CNxPaths::LoadSysPaths() throw()
{
    WCHAR wzPath[MAX_PATH + 1] = {0};

    // Get Windows Path
    GetWindowsDirectoryW(wzPath, MAX_PATH);
    _win = wzPath;
    _wintemp = _win + L"\\Temp";
    GetSystemDirectoryW(wzPath, MAX_PATH);
    _sys32 = wzPath;    
#ifdef _WIN64
    _wowsys32 = _win + L"\\SysWOW64";
#endif
    if(INVALID_FILE_ATTRIBUTES == GetFileAttributesW(_wintemp.c_str())) {
        ::CreateDirectoryW(_wintemp.c_str(), NULL);
    }
}