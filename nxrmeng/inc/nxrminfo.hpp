

#ifndef __NXRM_ENGINE_DIRS_H__
#define __NXRM_ENGINE_DIRS_H__


#include <Windows.h>
#include <string>


class CNxProdInfo
{
public:
    CNxProdInfo();
    ~CNxProdInfo();

    void Load() throw();
    void Clear() throw();

    inline const std::wstring& GetGuid() const throw() {return _guid;}
    inline const std::wstring& GetPublisher() const throw() {return _publisher;}
    inline const std::wstring& GetProductName() const throw() {return _prodname;}
    inline const std::wstring& GetProductInitial() const throw() {return _prodinitial;}
    inline const std::wstring& GetProductVersion() const throw() {return _prodversion;}
    inline const std::wstring& GetInstallDir() const throw() {return _installdir;}
    inline const SYSTEMTIME& GetInstallDate() const throw() {return _installdate;}
    inline const std::wstring& GetLocaleName() const throw() {return _localename;}
    inline ULONG GetLanguageId() const throw() {return _langid;}

private:
    std::wstring    _guid;
    std::wstring    _publisher;
    std::wstring    _prodname;
    std::wstring    _prodinitial;
    std::wstring    _prodversion;
    std::wstring    _installdir;
    SYSTEMTIME      _installdate;
    std::wstring    _localename;
    ULONG           _langid;
};

class CNxPaths
{
public:
    CNxPaths();
    ~CNxPaths();

    void Load() throw();
    void Clear() throw();

    inline const std::wstring& GetWinDir() const throw() {return _win;}
    inline const std::wstring& GetWinTempDir() const throw() {return _wintemp;}
    inline const std::wstring& GetSys32Dir() const throw() {return _sys32;}

    inline const std::wstring& GetRootDir() const throw() {return _root;}
    inline const std::wstring& GetBinDir() const throw() {return _bin;}
    inline const std::wstring& GetConfDir() const throw() {return _conf;}
    inline const std::wstring& GetLogDir() const throw() {return _log;}
    inline const std::wstring& GetCacheDir() const throw() {return _cache;}

    inline const std::wstring& GetCoreDrv() const throw() {return _coredrv;}
    inline const std::wstring& GetFltDrv() const throw() {return _fltdrv;}
    inline const std::wstring& GetFltManDll() const throw() {return _fltmandll;}
    inline const std::wstring& GetDrvManDll() const throw() {return _drvmandll;}
    inline const std::wstring& GetCoreDll() const throw() {return _coredll;}
    inline const std::wstring& GetCoreAddinDll() const throw() {return _coreaddindll;}
    inline const std::wstring& GetEngineDll() const throw() {return _engdll;}
    inline const std::wstring& GetResDll() const throw() {return _resdll;}
    inline const std::wstring& GetOverlayDll() const throw() {return _overlaydll;}
    inline const std::wstring& GetShellDll() const throw() {return _shelldll;}
    inline const std::wstring& GetCommonUiDll() const throw() {return _cmudll;}
    inline const std::wstring& GetServExe() const throw() {return _servexe;}
    inline const std::wstring& GetTrayExe() const throw() {return _trayexe;}

    // Conf
    inline const std::wstring& GetRegisterConf() const throw() {return _regconf;}
    inline const std::wstring& GetClassifyConf() const throw() {return _classifyconf;}
    inline const std::wstring& GetProfile() const throw() {return _profile;}
    inline const std::wstring& GetKeyConf() const throw() {return _keyconf;}
    inline const std::wstring& GetPolicyConf() const throw() {return _policyconf;}
    inline const std::wstring& GetAgentCert() const throw() {return _agentcert;}
    inline const std::wstring& GetSecureBin() const throw() {return _securebin;}

#ifdef _WIN64
    inline const std::wstring& GetWowSys32Dir() const throw() {return _wowsys32;}
    inline const std::wstring& GetBin32Dir() const throw() {return _bin32;}
    inline const std::wstring& GetCore32Dll() const throw() {return _core32dll;}
    inline const std::wstring& GetCoreAddin32Dll() const throw() {return _coreaddin32dll;}
    inline const std::wstring& GetRes32Dll() const throw() {return _res32dll;}
    inline const std::wstring& GetCommonUi32Dll() const throw() {return _cmu32dll;}
#endif

protected:
    void LoadSysPaths() throw();

private:
    // Windows Dirs
    std::wstring    _win;
    std::wstring    _wintemp;
    std::wstring    _sys32;

    // NextLabs Dirs
    std::wstring    _root;
    std::wstring    _bin;
    std::wstring    _conf;
    std::wstring    _log;
    std::wstring    _cache;

    // NextLabs Files
    std::wstring    _coredrv;
    std::wstring    _fltdrv;
    std::wstring    _fltmandll;
    std::wstring    _drvmandll;
    std::wstring    _coredll;
    std::wstring    _coreaddindll;
    std::wstring    _engdll;
    std::wstring    _resdll;
    std::wstring    _overlaydll;
    std::wstring    _shelldll;
    std::wstring    _cmudll;
    std::wstring    _servexe;
    std::wstring    _trayexe;

    // NextLabs Conf Files
    std::wstring    _regconf;
    std::wstring    _classifyconf;
    std::wstring    _profile;
    std::wstring    _keyconf;
    std::wstring    _policyconf;
    std::wstring    _agentcert;
    std::wstring    _securebin;

#ifdef _WIN64
    // Windows Dirs
    std::wstring    _wowsys32;
    // NextLabs Dirs
    std::wstring    _bin32;
    // NextLabs Files
    std::wstring    _core32dll;
    std::wstring    _coreaddin32dll;
    std::wstring    _res32dll;
    std::wstring    _cmu32dll;
#endif
};


#endif // __NXRM_ENGINE_DIRS_H__

