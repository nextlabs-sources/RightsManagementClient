
#include <Windows.h>
#include <assert.h>
#include <time.h>
#include <Wtsapi32.h>
#define SECURITY_WIN32
#include <Sspi.h>

#include <algorithm>

#include <nudf\exception.hpp>
#include <nudf\string.hpp>
#include <nudf\registry.hpp>
#include <nudf\resutil.hpp>
#include <nudf\time.hpp>
#include <nudf\ziputil.hpp>
#include <nxrmpkg.hpp>

#include "nxrmeng.h"
#include "engine.hpp"
#include "diagnose.hpp"
#include "rmsutil.hpp"


using namespace nxrm::engine;

nxrm::engine::CLogServ nxrm::engine::_LogServ;

class CLogonLockGuard
{
public:
    CLogonLockGuard() : m_pcs(NULL)
    {
    }
    CLogonLockGuard(_In_ CRITICAL_SECTION* pcs) : m_pcs(pcs)
    {
        if(NULL != m_pcs) {
            ::EnterCriticalSection(m_pcs);
        }
    }
    ~CLogonLockGuard()
    {
        if(NULL != m_pcs) {
            ::LeaveCriticalSection(m_pcs);
        }
    }

private:
    CRITICAL_SECTION*   m_pcs;
};

//
//  class CEngine
//  

CEngine::CEngine() : _Status(EngineStopped), _Connected(FALSE), _Active(FALSE)
{
    nudf::crypto::Initialize();
    memset(&_lastHeartBeatTime, 0, sizeof(_lastHeartBeatTime));
    ::InitializeCriticalSection(&_LogOnLock);
}

CEngine::~CEngine()
{
    ::DeleteCriticalSection(&_LogOnLock);
}

VOID CEngine::Initialize(_In_ LPCWSTR wzWorkingRoot)
{
    HRESULT hr = S_OK;
    if(NULL == wzWorkingRoot || L'\0' == wzWorkingRoot[0]) {
        SetLastError(ERROR_INVALID_PARAMETER);
        throw WIN32ERROR();
    }

    // Initialize Engine Session Key and Session Guid
    hr = _sessionKey.Generate(256);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }
    
    assert(NULL != _sessionKey.GetKey());
    _sessionGuid = nudf::string::FromGuid<wchar_t>((const GUID*)_sessionKey.GetKey(), false);
    assert(!_sessionGuid.empty());
    

    // Initialize SSPI before the driver protect our process
    InitializeSSPI();

    // Start logging
    try {

        _LogServ.Initialize();
        _LogServ.Start();


        _LogServ.Push(L"\r\n");
        _LogServ.Push(L"\r\n");
        LOGINF(L" ");
        LOGINF(L">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
        LOGINF(L">                                                 >");
        LOGINF(L">       NEXTLABS RIGHTS MANAGEMENT SYSTEM         >");
        LOGINF(L">                                                 >");
        LOGINF(L">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
        LOGSYSINF(L" ");
        LOGSYSINF(L"SYSTEM BOOTING ...");
        LOGSYSINF(L" ");
        LOGSYSINF(L">>>> Checking Environment Information ...");
        diagnose::DumpHwInfo();
        diagnose::DumpOsInfo();

        LOGSYSINF(L" ");
        LOGSYSINF(L">>>> Initializing Engine ...");

        // Validate Environment Variables
        LOGDBG(L"        --> Validate Environment Variables");
        if(!EnvValidate()) {
            throw WIN32ERROR2(ERROR_FILE_NOT_FOUND);
        }
        LOGDBG(L"            Done");

        // Load Resource Dll
        LOGDBG(L"        --> Load MUI Resource");
        _ResDll = ::LoadLibraryW(_nxPaths.GetResDll().c_str());
        if(NULL == _ResDll) {
            throw EXCEPTION(ERROR_FILE_NOT_FOUND, "MUI Resource Dll doesn't exist");
        }
        LOGDBG(L"            Done");

        // Start IPC
        LOGDBG(L"        --> Starting IPC Service");
        _IpcServ.Initialize();
        _IpcServ.Start();
        LOGDBG(L"            Done");

        // Start IPC
        LOGDBG(L"        --> Starting Session Service");
        try {
            CLogonLockGuard guard(&_LogOnLock);
            _SessionServ.Start();
        }
        catch(const nudf::CException& e) {
            throw e;
        }
        LOGDBG(L"            Done");
        
        // Log Start Information
        LOGSYSINF(L"        Engine has been initialized");
    }
    catch(const nudf::CException& e) {
        LOG_EXCEPTION(e);
        _SessionServ.Stop();
        _IpcServ.Shutdown();
        _LogServ.Stop();
        // Free MUI Resource DLL
        if(NULL != _ResDll) {
            ::FreeLibrary(_ResDll);
            _ResDll = NULL;
        }
        LOGSYSINF(L" ");
        LOGSYSINF(L"SYSTEM FAIL TO INITIALIZE");
        LOGSYSINF(L" ");
        throw e;
    }
}

VOID CEngine::Cleanup()
{
    LOGSYSINF(L" ");
    LOGSYSINF(L">>>> 2. Shutdown ...");
    
    LOGDBG(L"        --> Shutdown Session Service");
    _SessionServ.Stop();
    LOGDBG(L"            Done");
    
    LOGDBG(L"        --> Shutdown IPC Service");
    _IpcServ.Shutdown();
    _IpcServ.Cleanup();
    LOGDBG(L"            Done");

    LOGSYSINF(L">>>> SYSTEM HAS BEEN SHUTDOWN");
    LOGSYSINF(L" ");
    _LogServ.Push(L"\r\n");

    // Shutdown logging service at last
    _LogServ.Stop();
    _LogServ.Cleanup();

    // Free MUI Resource DLL
    if(NULL != _ResDll) {
        ::FreeLibrary(_ResDll);
        _ResDll = NULL;
    }
}


VOID CEngine::Start()
{
    try {
        
        // Set time for checking/performing registration
        // Every 5 minutes        
        _TimerRegister.Set(300000, TRUE);
        _Status = EngineRunning;

        // Log Start Information
        LOGSYSINF(L"        Engine Started");
        LOGSYSINF(L" ");
        LOGSYSINF(L">>>> SYSTEM BOOT SUCCESSFULLY");
        LOGSYSINF(L" ");
    }
    catch(const nudf::CException& e) {
        LOG_EXCEPTION(e);
        LOGSYSINF(L" ");
        LOGSYSINF(L"SYSTEM FAIL TO START");
        LOGSYSINF(L" ");
        throw e;
    }

}

VOID CEngine::Stop()
{
    LOGSYSINF(L" ");
    LOGSYSINF(L"SYSTEM SHUTTING DOWN ...");
    LOGSYSINF(L" ");
    LOGSYSINF(L">>>> 1. Stopping Engine ...");

    _TimerRegister.Kill();
    _TimerHeartBeat.Kill();
    _TimerLog.Kill();
    _TimerCheckUpdate.Kill(FALSE);

    // Shutting down session service
    InterStopEngine();
    
    LOGSYSINF(L"        Engine Stopped");
    _Status = EngineStopped;
}

BOOL CEngine::InterStartEngine()
{
    CoInitialize(NULL);
    try {
        if(!_Active) {
            
            LOGSYSINF(L"        Starting Engine");

            _AuditObject.Create(L"N/A", _agentProfile.GetAgentId());
            LOGSYSINF(L"        Audit Engine Started");

            _DrvServ.Start();
            LOGSYSINF(L"        Core Driver Started");
            assert(_keyBundle.GetKeyBlobSize() != 0);
            assert(_PolicyServ.IsLoaded());
            _FltServ.Start(_keyBundle.GetKeyBlobs(), _keyBundle.GetKeyBlobSize());
            LOGSYSINF(L"        Filter Driver Started");
            if(_LogServ.AcceptLevel(LOGDEBUG)) {
                const NXRM_KEY_BLOB* pb = _keyBundle.GetKeyBlobs();
                int nKeyCount = (int)(_keyBundle.GetKeyBlobSize() / ((ULONG)sizeof(NXRM_KEY_BLOB)));
                _LogServ.Push(nudf::util::log::CLogEntry(LOGDEBUG, NXMODNAME(), L"Init keys - %d %s set:", nKeyCount, (nKeyCount>1)?L"keys are":L"key is"));
                for(int i=0; i<nKeyCount; i++) {
                    std::wstring wsKeyId = nudf::string::FromBytes<wchar_t>(pb[i].KeKeyId.Id, pb[i].KeKeyId.IdSize);
                    _LogServ.Push(nudf::util::log::CLogEntry(LOGDEBUG, NXMODNAME(), L"  Key%04d: %s", i, wsKeyId.c_str()));
                }
            }

            _Active = TRUE;
            LOGSYSINF(L">>>> SYSTEM IS ACTIVE NOW");
        }
    }
    catch(const nudf::CException& e) {
        LOGERR(e.GetCode(), L"InterStartEngine Failed. (err:%d, file:%S, func:%S, line:%d)", e.GetCode(), e.GetFile(), e.GetFunction(), e.GetLine());
        _FltServ.Stop();
        _DrvServ.Stop();
        _Active = FALSE;
    }
    CoUninitialize();

    return _Active;
}

VOID CEngine::InterStopEngine()
{
    if(_Active) {
        _FltServ.Stop();
        _DrvServ.Stop();
        DumpAudit();
    }
}

VOID CEngine::Pause()
{
    _Status = EnginePaused;
}

VOID CEngine::Continue()
{
    _Status = EngineRunning;
}

VOID CEngine::Audit(const CAuditItem& ai) throw()
{
    CoInitialize(NULL);
    try {
        _AuditObject.InsertAuditItem(ai);
        if(_AuditObject.GetCount() >= 1024) {
            DumpAudit();
        }
    }
    catch(const nudf::CException& e) {
        LOGERR(e.GetCode(), L"InterStopEngine: Fail to dump audit logs (err:%d, file:%S, func:%S, line:%d)", e.GetCode(), e.GetFile(), e.GetFunction(), e.GetLine());
    }
    CoUninitialize();
}

VOID CEngine::DumpAudit() throw()
{
    std::wstring wsFile;
    nudf::time::CTime tm;
    SYSTEMTIME st;

    if(_AuditObject.GetCount() == 0) {
        return;
    }

    tm.Now();
    tm.ToLocalTime(NULL);
    tm.ToSystemTime(&st);
    swprintf_s(nudf::string::tempstr<wchar_t>(wsFile, MAX_PATH), MAX_PATH, L"%s\\Audit%04d%02d%02d%02d%02d%02d%03d.xml",
        _nxPaths.GetLogDir().c_str(), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    try {
        _AuditObject.Dump(wsFile);
    }
    catch(const nudf::CException& e) {
        LOGERR(e.GetCode(), L"DumpAudit: Fail to dump audit logs (err:%d, file:%S, func:%S, line:%d)", e.GetCode(), e.GetFile(), e.GetFunction(), e.GetLine());
    }
}

VOID CEngine::UploadAudit() throw()
{
    std::wstring wsPattern;
    HANDLE       hFind = INVALID_HANDLE_VALUE;

    try {

        wsPattern = _nxPaths.GetLogDir();
        wsPattern += L"\\Audit*.xml";
        WIN32_FIND_DATAW wfd = {0};

        memset(&wfd, 0, sizeof(wfd));
        hFind = FindFirstFileW(wsPattern.c_str(), &wfd);
        if(INVALID_HANDLE_VALUE == hFind) {
            throw WIN32ERROR();
        }

        do {

            std::wstring wsFile = _nxPaths.GetLogDir() + L"\\";
            std::wstring wsPostFile = _nxPaths.GetLogDir() + L"\\Post";

            if(FILE_ATTRIBUTE_DIRECTORY==(wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) || 0 == _wcsicmp(wfd.cFileName, L".") || 0 == _wcsicmp(wfd.cFileName, L"..")) {
                continue;
            }

            wsFile +=wfd.cFileName;
            wsPostFile +=wfd.cFileName;

            CRmsLogService logsvc;
            logsvc.LogAudit(GetRmsAgentProfile(), wsFile);

            // If succeed, rename the file
            if(!::MoveFileW(wsFile.c_str(), wsPostFile.c_str())) {
                LOGWARN(GetLastError(), L"Fail to rename audit log (%d): %s", GetLastError(), wsFile.c_str());
            }

        } while(FindNextFileW(hFind, &wfd));

        FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }
    catch(const nudf::CException& e) {
        if(INVALID_HANDLE_VALUE != hFind) {
            FindClose(hFind);
            hFind = INVALID_HANDLE_VALUE;
        }
        if(ERROR_FILE_NOT_FOUND != e.GetCode()) {
            LOG_EXCEPTION2(e, L"UploadAudit Failed");
        }
    }
}

BOOL CEngine::DecryptPolicyBundle(const std::wstring& file, bool orig)
{
    BOOL bRet = FALSE;

    try {
        CPolicyBundle bundle;

        bundle.Load(this->GetRmsAgentProfile().GetSessionKey(), orig);
        ::DeleteFileW(file.c_str());
        bundle.SaveAsPlainFile(file);
        bRet = TRUE;
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        bRet = FALSE;
    }

    return bRet;
}

VOID CEngine::OnSessionLogon(_In_ ULONG dwSessionId)
{
    LOGINF(L"Session Logon: %d", dwSessionId);

    try {
        CLogonLockGuard guard(&_LogOnLock);
        if(!_SessionServ.Exists(dwSessionId)) {
            _SessionServ.AddSession(dwSessionId);
        }
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        throw e;
    }
}

VOID CEngine::OnSessionLogoff(_In_ ULONG dwSessionId)
{
    try {
        CLogonLockGuard guard(&_LogOnLock);
        _SessionServ.RemoveSession(dwSessionId);
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        throw e;
    }
    LOGINF(L"Session Logoff: %d", dwSessionId);
}

VOID CEngine::OnSessionConn(_In_ ULONG dwSessionId)
{
    LOGINF(L"Session Connect: %d", dwSessionId);
}

VOID CEngine::OnSessionDisconn(_In_ ULONG dwSessionId)
{
    LOGINF(L"Session Disconnect: %d", dwSessionId);
}

VOID CEngine::OnSessionRemoteConn(_In_ ULONG dwSessionId)
{
}

VOID CEngine::OnSessionRemoteDisconn(_In_ ULONG dwSessionId)
{
}


//
//  Protected Routines
//

BOOL CEngine::CheckFolderExistence(_In_ const std::wstring& path)
{
    DWORD dwAttributes = INVALID_FILE_ATTRIBUTES;
    if(!CheckFileExistence(path.c_str(), &dwAttributes)) {
        return FALSE;
    }
    return (FILE_ATTRIBUTE_DIRECTORY == (FILE_ATTRIBUTE_DIRECTORY & dwAttributes));
}

BOOL CEngine::CheckFileExistence(_In_ const std::wstring& path, _Out_opt_ PDWORD attributes)
{
    DWORD dwAttributes = GetFileAttributesW(path.c_str());
    if(attributes) *attributes = dwAttributes;
    if(INVALID_FILE_ATTRIBUTES == dwAttributes) {
        return FALSE;
    }
    return TRUE;
}

VOID CEngine::InitializeSSPI()
{
    PSecurityFunctionTableW pSft = InitSecurityInterfaceW();
    if(NULL != pSft) {
        ULONG           cPackages = 0;
        PSecPkgInfoW    pPackageInfo = NULL;
        SECURITY_STATUS status = EnumerateSecurityPackagesW(&cPackages, &pPackageInfo);
        if(SEC_E_OK == status && NULL != pPackageInfo) {
            (VOID)FreeContextBuffer(pPackageInfo);
        }
    }
}

BOOL CEngine::EnvValidate()
{
    //
    //  Check Directories
    //

    // [root]\bin
    if(!CheckFolderExistence(_nxPaths.GetBinDir())) {
        LOGERR(ERROR_FILE_NOT_FOUND, L"Folder Not Exist: %s", _nxPaths.GetBinDir().c_str());
        return FALSE;
    }
#ifdef _WIN64
    // [root]\bin\x86
    if(!CheckFolderExistence(_nxPaths.GetBin32Dir())) {
        LOGERR(ERROR_FILE_NOT_FOUND, L"Folder Not Exist: %s", _nxPaths.GetBin32Dir().c_str());
        return FALSE;
    }
#endif

    // [root]\conf
    if(!CheckFolderExistence(_nxPaths.GetConfDir())) {
        LOGERR(ERROR_FILE_NOT_FOUND, L"Folder Not Exist: %s", _nxPaths.GetConfDir().c_str());
        return FALSE;
    }

    // [root]\log
    if(!CheckFolderExistence(_nxPaths.GetLogDir())) {
        LOGERR(ERROR_FILE_NOT_FOUND, L"Folder Not Exist: %s", _nxPaths.GetLogDir().c_str());
        return FALSE;
    }

    //
    //  Check Files
    //

    // %system%\drivers\nxrmflt.sys
    if(!CheckFileExistence(_nxPaths.GetFltDrv())) {
        LOGERR(ERROR_FILE_NOT_FOUND, L"File Not Exist: %s", _nxPaths.GetFltDrv().c_str());
        return FALSE;
    }

    // %system%\drivers\nxrmdrv.sys
    if(!CheckFileExistence(_nxPaths.GetCoreDrv())) {
        LOGERR(ERROR_FILE_NOT_FOUND, L"File Not Exist: %s", _nxPaths.GetCoreDrv().c_str());
        return FALSE;
    }

    // %system%\nxrmcore.dll
    if(!CheckFileExistence(_nxPaths.GetCoreDll())) {
        LOGERR(ERROR_FILE_NOT_FOUND, L"File Not Exist: %s", _nxPaths.GetCoreDll().c_str());
        return FALSE;
    }

    // [root]\bin\nxrmserv.exe
    if(!CheckFileExistence(_nxPaths.GetServExe())) {
        LOGERR(ERROR_FILE_NOT_FOUND, L"File Not Exist: %s", _nxPaths.GetServExe().c_str());
        return FALSE;
    }

    // [root]\bin\nxrmtray.exe
    if(!CheckFileExistence(_nxPaths.GetTrayExe())) {
        LOGERR(ERROR_FILE_NOT_FOUND, L"File Not Exist: %s", _nxPaths.GetTrayExe().c_str());
        return FALSE;
    }

    // [root]\bin\nxrmcmui.exe
    if(!CheckFileExistence(_nxPaths.GetCommonUiDll())) {
        LOGERR(ERROR_FILE_NOT_FOUND, L"File Not Exist: %s", _nxPaths.GetCommonUiDll().c_str());
        return FALSE;
    }

    // [root]\bin\nxrmcoreaddin.dll
    if(!CheckFileExistence(_nxPaths.GetCoreAddinDll())) {
        LOGERR(ERROR_FILE_NOT_FOUND, L"File Not Exist: %s", _nxPaths.GetCoreAddinDll().c_str());
        return FALSE;
    }

    // [root]\bin\nxrmfltman.dll
    if(!CheckFileExistence(_nxPaths.GetFltManDll())) {
        LOGERR(ERROR_FILE_NOT_FOUND, L"File Not Exist: %s", _nxPaths.GetFltManDll().c_str());
        return FALSE;
    }

    // [root]\bin\nxrmdrvman.dll
    if(!CheckFileExistence(_nxPaths.GetDrvManDll())) {
        LOGERR(ERROR_FILE_NOT_FOUND, L"File Not Exist: %s", _nxPaths.GetDrvManDll().c_str());
        return FALSE;
    }

    // [root]\bin\nxrmoverlay.dll
    if(!CheckFileExistence(_nxPaths.GetOverlayDll())) {
        LOGERR(ERROR_FILE_NOT_FOUND, L"File Not Exist: %s", _nxPaths.GetOverlayDll().c_str());
        return FALSE;
    }

    // [root]\bin\nxrmshell.dll
    if(!CheckFileExistence(_nxPaths.GetShellDll())) {
        LOGERR(ERROR_FILE_NOT_FOUND, L"File Not Exist: %s", _nxPaths.GetShellDll().c_str());
        return FALSE;
    }

    // [root]\bin\nxrmres.dll
    if(!CheckFileExistence(_nxPaths.GetResDll())) {
        LOGERR(ERROR_FILE_NOT_FOUND, L"File Not Exist: %s", _nxPaths.GetResDll().c_str());
        return FALSE;
    }
    
#ifdef _WIN64

    // %SysWow64%\nxrmcore32.dll
    if(!CheckFileExistence(_nxPaths.GetCore32Dll())) {
        LOGERR(ERROR_FILE_NOT_FOUND, L"File Not Exist: %s", _nxPaths.GetCore32Dll().c_str());
        return FALSE;
    }

    // [root]\bin\x86\nxrmcmui.exe
    if(!CheckFileExistence(_nxPaths.GetCommonUi32Dll())) {
        LOGERR(ERROR_FILE_NOT_FOUND, L"File Not Exist: %s", _nxPaths.GetCommonUi32Dll().c_str());
        return FALSE;
    }

    // [root]\bin\x86\nxrmcoreaddin.dll
    if(!CheckFileExistence(_nxPaths.GetCoreAddin32Dll())) {
        LOGERR(ERROR_FILE_NOT_FOUND, L"File Not Exist: %s", _nxPaths.GetCoreAddin32Dll().c_str());
        return FALSE;
    }

    // [root]\bin\x86\nxrmres.dll
    if(!CheckFileExistence(_nxPaths.GetRes32Dll())) {
        LOGERR(ERROR_FILE_NOT_FOUND, L"File Not Exist: %s", _nxPaths.GetRes32Dll().c_str());
        return FALSE;
    }

#endif

    return TRUE;
}

void CEngine::CleanSidCache()
{
    HANDLE  hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW    wfd = { 0 };

    std::wstring wssearch = _nxPaths.GetCacheDir();
    std::vector<std::wstring> files;

    memset(&wfd, 0, sizeof(WIN32_FIND_DATAW));
    wssearch += L"\\*";
    hFind = FindFirstFileW(wssearch.c_str(), &wfd);
    if (INVALID_HANDLE_VALUE == hFind) {
        return;
    }

    do {
        if (FILE_ATTRIBUTE_DIRECTORY == (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            continue;
        }
        if (0 == _wcsicmp(L".", wfd.cFileName) || 0 == _wcsicmp(L"..", wfd.cFileName)) {
            continue;
        }
        files.push_back(std::wstring(_nxPaths.GetCacheDir() + L"\\" + wfd.cFileName));
    } while (FindNextFileW(hFind, &wfd));
    FindClose(hFind); hFind = INVALID_HANDLE_VALUE;

    // Delete all the files
    std::for_each(files.begin(), files.end(), [&](const std::wstring& file) {
        (VOID)::DeleteFileW(file.c_str());
    });
}

VOID CEngine::Register()
{
    CRmsRegisterServer  regserv;
    CRmsRegisterService regsvc;
    std::wstring wsRegisterData;

    // Delete all the files under cache dir
    CleanSidCache();
    
    // Load register.xml
    regserv.Load(_nxPaths.GetRegisterConf());
    regsvc.Register(regserv, wsRegisterData);

    // Good
    _agentProfile.SetRmsServer(regserv.GetServer());
    _agentProfile.SetRmsPath(regserv.GetServicePath());
    _agentProfile.SetRmsPort(regserv.GetPort());
    _agentProfile.LoadFromRegisterData(wsRegisterData);
    _agentProfile.Save();
    // Load Classify Profile
    _classifyProfile.LoadFromRegisterData(wsRegisterData, _agentProfile.GetSignKey());
    _classifyProfile.Save(_nxPaths.GetClassifyConf());
}

VOID CEngine::HeartBeat(_Out_ BOOL* bKeyChanged, _Out_ BOOL* bPolicyChanged)
{
    CRmsHeartBeatService hbsvc;
    std::wstring         wsHbResponse;
    nudf::util::CXmlDocument doc;
    bool bIsAgentProfileDirty = false;
    static bool bFirstheartBeat = true;

    *bKeyChanged = FALSE;
    *bPolicyChanged = FALSE;

    if(_agentProfile.GetAgentId().empty()) {
        return;
    }

    try {
        hbsvc.Beat(_agentProfile, wsHbResponse);
        // HeartBeat Succeed
        if(!_Connected) {
            // Set Connection Status
            _Connected = TRUE;
            GetSessionServ()->NotifyClientServerStatus(_agentProfile.GetRmsServer(), _Connected);
            LOGINF(L"RMS Connection: Disconnected ==> Connected");
            // Disconnected ==> Connected: We reset timer to Profile defined time elapse
            _TimerHeartBeat.SetElapse((_agentProfile.GetHeartBeatFrequency()) == 0 ? 86400000 /*1 day*/ : (_agentProfile.GetHeartBeatFrequency()*1000));
        }
        if(bFirstheartBeat) {
            bFirstheartBeat = false;
            LOGINF(L"First HeartBeat Succeed");
        }
    }
    catch(const nudf::CException& e) {
        if(_Connected) {
            _Connected = FALSE;
            GetSessionServ()->NotifyClientServerStatus(_agentProfile.GetRmsServer(), _Connected);
            LOGINF(L"RMS Connection: Connected ==> Disconnected");
            // Connected ==> Disconnected: We reset timer to 1 minutes
            _TimerHeartBeat.SetElapse(60000);
        }
        else {
            if(bFirstheartBeat) {
                LOGERR(e.GetCode(), L"First HeartBeat Failed: error:%d(0x%X), file:%S, function:%S, line:%d", e.GetCode(), e.GetCode(), e.GetFile(), e.GetFunction(), e.GetLine());
            }
        }
        if(bFirstheartBeat) {
            bFirstheartBeat = false;
        }
        throw e;
    }

    try {

        std::wstring            rootName;
        CComPtr<IXMLDOMElement> spRoot;
        CComPtr<IXMLDOMNode>    spAgentUpdates;
        CComPtr<IXMLDOMNode>    spClassification;
        CComPtr<IXMLDOMNode>    spkeyRings;
        CComPtr<IXMLDOMNode>    spPolicyBundle;

        doc.LoadFromXml(wsHbResponse.c_str());
        if(!doc.GetDocRoot(&spRoot)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        rootName = doc.GetNodeName(spRoot);
        if(0 != _wcsicmp(L"HeartBeatResponse", rootName.c_str())) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        SetHeartBeatTime(NULL);

        if(doc.FindChildElement(spRoot, L"AgentUpdates", &spAgentUpdates)) {

            CComPtr<IXMLDOMNode>    spAgentProfile;
            CComPtr<IXMLDOMNode>    spCommProfile;
            // Update AgentProfile
            if(doc.FindChildElement(spAgentUpdates, L"agentProfile", &spAgentProfile)) {
                CComPtr<IXMLDOMNode> spSubNode;
                if(doc.FindChildElement(spAgentProfile, L"name", &spSubNode)) {
                    std::wstring wsName = doc.GetNodeText(spSubNode);
                    if(!wsName.empty() && 0 != _wcsicmp(wsName.c_str(), _agentProfile.GetRmsAgentProfileName().c_str())) {
                        _agentProfile.SetRmsAgentProfileName(wsName);
                        bIsAgentProfileDirty = true;
                    }
                    spSubNode.Release();
                }
                if(doc.FindChildElement(spAgentProfile, L"modifiedDate", &spSubNode)) {
                    std::wstring wsModifiedDate = doc.GetNodeText(spSubNode);
                    if(!wsModifiedDate.empty() && 0 != _wcsicmp(wsModifiedDate.c_str(), _agentProfile.GetRmsAgentProfileTimestamp().c_str())) {
                        _agentProfile.SetRmsAgentProfileTimestamp(wsModifiedDate);
                        bIsAgentProfileDirty = true;
                    }
                    spSubNode.Release();
                }
            }
            if(doc.FindChildElement(spAgentUpdates, L"commProfile", &spCommProfile)) {
                CComPtr<IXMLDOMNode> spSubNode;
                if(doc.FindChildElement(spCommProfile, L"name", &spSubNode)) {
                    std::wstring wsName = doc.GetNodeText(spSubNode);
                    if(!wsName.empty() && 0 != _wcsicmp(wsName.c_str(), _agentProfile.GetRmsCommProfileName().c_str())) {
                        _agentProfile.SetRmsCommProfileName(wsName);
                        bIsAgentProfileDirty = true;
                    }
                    spSubNode.Release();
                }
                if(doc.FindChildElement(spCommProfile, L"modifiedDate", &spSubNode)) {
                    std::wstring wsModifiedDate = doc.GetNodeText(spSubNode);
                    if(!wsModifiedDate.empty() && 0 != _wcsicmp(wsModifiedDate.c_str(), _agentProfile.GetRmsCommProfileTimestamp().c_str())) {
                        _agentProfile.SetRmsCommProfileTimestamp(wsModifiedDate);
                        bIsAgentProfileDirty = true;
                    }
                    spSubNode.Release();
                }
                if(doc.FindChildElement(spCommProfile, L"heartBeatFrequency", &spSubNode)) {
                    CComPtr<IXMLDOMNode> spTimeNode;
                    CComPtr<IXMLDOMNode> spTimeUnitNode;
                    if(doc.FindChildElement(spSubNode, L"time", &spTimeNode) && doc.FindChildElement(spSubNode, L"time-unit", &spTimeUnitNode)) {
                        std::wstring wsTime = doc.GetNodeText(spTimeNode);
                        std::wstring wsTimeUnit = doc.GetNodeText(spTimeUnitNode);
                        int nTime = 0;
                        if(nudf::string::ToInt<wchar_t>(wsTime, &nTime) && nTime > 0) {
                            
                            if(0 == _wcsicmp(wsTimeUnit.c_str(), L"days")) {
                                nTime *= 86400;
                            }
                            else if(0 == _wcsicmp(wsTimeUnit.c_str(), L"hours")) {
                                nTime *= 3600;
                            }
                            else if(0 == _wcsicmp(wsTimeUnit.c_str(), L"minutes")) {
                                nTime *= 60;
                            }
                            else if(0 == _wcsicmp(wsTimeUnit.c_str(), L"milliseconds")) {
                                nTime /= 1000;
                                if(0 == nTime) {
                                    nTime = 1;
                                }
                            }
                            else {
                                ; // Nothing
                            }

                            if((ULONG)nTime != _agentProfile.GetHeartBeatFrequency()) {
                                _agentProfile.SetHeartBeatFrequency((ULONG)nTime);
                                bIsAgentProfileDirty = true;
                            }
                        }
                    }
                    spSubNode.Release();
                }
                if(doc.FindChildElement(spCommProfile, L"logFrequency", &spSubNode)) {
                    CComPtr<IXMLDOMNode> spTimeNode;
                    CComPtr<IXMLDOMNode> spTimeUnitNode;
                    if(doc.FindChildElement(spSubNode, L"time", &spTimeNode) && doc.FindChildElement(spSubNode, L"time-unit", &spTimeUnitNode)) {
                        std::wstring wsTime = doc.GetNodeText(spTimeNode);
                        std::wstring wsTimeUnit = doc.GetNodeText(spTimeUnitNode);
                        int nTime = 0;
                        if(nudf::string::ToInt<wchar_t>(wsTime, &nTime) && nTime > 0) {
                            
                            if(0 == _wcsicmp(wsTimeUnit.c_str(), L"days")) {
                                nTime *= 86400;
                            }
                            else if(0 == _wcsicmp(wsTimeUnit.c_str(), L"hours")) {
                                nTime *= 3600;
                            }
                            else if(0 == _wcsicmp(wsTimeUnit.c_str(), L"minutes")) {
                                nTime *= 60;
                            }
                            else if(0 == _wcsicmp(wsTimeUnit.c_str(), L"milliseconds")) {
                                nTime /= 1000;
                                if(0 == nTime) {
                                    nTime = 1;
                                }
                            }
                            else {
                                ; // Nothing
                            }

                            if((ULONG)nTime != _agentProfile.GetLogFrequency()) {
                                _agentProfile.SetLogFrequency((ULONG)nTime);
                                bIsAgentProfileDirty = true;
                            }
                        }
                    }
                    spSubNode.Release();
                }
            }

            if(bIsAgentProfileDirty) {
                try {
                    _agentProfile.Save();
                    LOGINF(L"New AgentProfile has been saved");
                }
                catch(const nudf::CException& e) {
                    UNREFERENCED_PARAMETER(e);
                    LOGERR(e.GetCode(), L"Fail to save new AgentProfile (code:0x%08X, file:%S, function:%S, line:%d)", e.GetCode(), e.GetFile(), e.GetFunction(), e.GetLine());
                }
            }

            // Update Classification Profile
            if(doc.FindChildElement(spAgentUpdates, L"ClassificationProfile", &spClassification)) {
                try {
                    CRmsAgentClassify   newClassifyProfile;
                    newClassifyProfile.LoadFromNode(&doc, spClassification, _agentProfile.GetSignKey());
                    if(0 != _wcsicmp(newClassifyProfile.GetSignature().c_str(), _classifyProfile.GetSignature().c_str())) {
                        // Need to update it
                        _classifyProfile = newClassifyProfile;
                        _classifyProfile.Save(_nxPaths.GetClassifyConf());
                        LOGINF(L"New Classification Profile has been saved");
                        // Let driver know
                        _DrvServ.IncreasePolicySerianNo();
                    }
                }
                catch(const nudf::CException& e) {
                    // Nothing
                    UNREFERENCED_PARAMETER(e);
                    LOGERR(e.GetCode(), L"Fail to save new Classification Profile (code:0x%08X, file:%S, function:%S, line:%d)", e.GetCode(), e.GetFile(), e.GetFunction(), e.GetLine());
                }
            }

            // Update Keys
            if(doc.FindChildElement(spAgentUpdates, L"keyRings", &spkeyRings)) {

                CKeyBundle kbundle;
                kbundle.Load(spkeyRings);
                if(kbundle.GetHash() != _keyBundle.GetHash()) {
                    // Update hash
                    kbundle.Save(_agentProfile.GetSessionKey());
                    _keyBundle = kbundle;
                    *bKeyChanged = TRUE;
                    // Let driver know
                    _DrvServ.IncreasePolicySerianNo();
                }
            }

            // Update Policy
            if(doc.FindChildElement(spAgentUpdates, L"policyDeploymentBundle", &spPolicyBundle)) {

                CComPtr<IXMLDOMNode> spPolicyProfile;

                if(!nudf::util::XmlUtil::FindChildElement(spPolicyBundle, PROFILE_POLICY_NODENAME, &spPolicyProfile) || NULL == spPolicyProfile.p) {
                    CComPtr<IXMLDOMNode> spNxPackage;
                    if(nudf::util::XmlUtil::FindChildElement(spPolicyBundle, L"NXPACKAGE", &spNxPackage) && NULL != spNxPackage.p) {
                        CComPtr<IXMLDOMNode> spNxData;
                        if(nudf::util::XmlUtil::FindChildElement(spNxPackage, L"DATA", &spNxData) && NULL != spNxData.p) {
                            nudf::util::XmlUtil::FindChildElement(spPolicyBundle, PROFILE_POLICY_NODENAME, &spPolicyProfile);
                        }
                    }
                }

                if(NULL != spPolicyProfile.p) {

                    CPolicyBundle bundle;

                    try {
                        bundle.Load(spPolicyProfile, true);
                        if(bundle.GetTimestampUll() > _PolicyServ.GetPolicyTimestampUll()) {
                            bundle.Save(_agentProfile.GetSessionKey(), true);
                            bundle.Clear();
                            bundle.Load(spPolicyProfile, false);
                            bundle.Save(_agentProfile.GetSessionKey(), false);
                            _PolicyServ.Load(_agentProfile.GetSessionKey());
                            *bPolicyChanged = TRUE;
                            _PolicyServ.UpdateLoggedOnUserCacheFile(hbsvc.GetLoggedOnUsersInBeat());
                            // Let driver know
                            _DrvServ.IncreasePolicySerianNo();
                        }
                    }
                    catch(const nudf::CException& e) {
                        UNREFERENCED_PARAMETER(e);
                    }
                }
            }

            // Update Comm Profile
            if(bIsAgentProfileDirty || *bPolicyChanged || *bKeyChanged) {
                try {
                    hbsvc.Acknowledge(_agentProfile);
                }
                catch(const nudf::CException& e) {
                    LOGERR(e.GetCode(), L"AcknowledgeHeartBeat: failed (error=%d, file:%S, function:%S, line:%d)", e.GetCode(), e.GetFile(), e.GetFunction(), e.GetLine());
                }
            }
        }

    }
    catch(const nudf::CException& e) {
        throw e;
    }
}

BOOL CEngine::NeedHeartBeat()
{
    // No key bundle
    if(INVALID_FILE_ATTRIBUTES == GetFileAttributesW(_nxPaths.GetKeyConf().c_str())) {
        return TRUE;
    }
    // No policy bundle
    if(INVALID_FILE_ATTRIBUTES == GetFileAttributesW(_nxPaths.GetPolicyConf().c_str())) {
        return TRUE;
    }
    // Check Time
    FILETIME CurrentTime = {0, 0};
    GetHeartBeatTime(&_lastHeartBeatTime);
    if(_lastHeartBeatTime.dwHighDateTime==0 && _lastHeartBeatTime.dwLowDateTime==0) {
        // No heart Beat Record?
        return TRUE;
    }
    GetSystemTimeAsFileTime(&CurrentTime);
    // Get Time span
    LONGLONG span = *((const LONGLONG*)(&CurrentTime)) - *((const LONGLONG*)(&_lastHeartBeatTime));
    if(span < 0) {
        return TRUE;
    }

    // 100-nano seconds to seconds
    span /= 10000000;
    if(span > (LONGLONG)_agentProfile.GetHeartBeatFrequency()) {
        return TRUE;
    }

    return FALSE;
}

BOOL CEngine::GetHeartBeatTime(_Out_ FILETIME* timestamp)
{
    nudf::win::CRegLocalMachine reglm;
    nudf::win::CRegKey  rkserv;

    memset(timestamp, 0, sizeof(FILETIME));
    
    if(!rkserv.Open(reglm, L"SYSTEM\\CurrentControlSet\\services\\nxrmserv", KEY_READ)) {
        return FALSE;
    }
    if(!rkserv.GetValue(L"LastHeartBeat", (ULONGLONG*)timestamp)) {
        memset(timestamp, 0, sizeof(FILETIME));
        return FALSE;
    }
    return TRUE;
}

BOOL CEngine::SetHeartBeatTime(_In_opt_ const FILETIME* timestamp)
{
    nudf::win::CRegLocalMachine reglm;
    nudf::win::CRegKey  rkserv;
    
        
    if(!rkserv.Open(reglm, L"SYSTEM\\CurrentControlSet\\services\\nxrmserv", KEY_READ|KEY_WRITE)) {
        return FALSE;
    }
    
    if(NULL == timestamp) {
        GetSystemTimeAsFileTime(&_lastHeartBeatTime);
    }
    else {
        memcpy(&_lastHeartBeatTime, timestamp, sizeof(FILETIME));
    }
    if(!rkserv.SetValue(L"LastHeartBeat", *((const ULONGLONG*)(&_lastHeartBeatTime)))) {
        return FALSE;
    }

    return TRUE;
}

VOID CEngine::OnTimerRegister()
{    
    // Try to load profile        
    try {
        LOGDBG(L"OnRegister: Loading profile ...");
        _agentProfile.Load();
        nudf::crypto::CRsaPubKeyBlob pubkey;
        _agentProfile.GetSignKey().GetPublicKeyBlob(pubkey);
        _classifyProfile.Load(_nxPaths.GetClassifyConf().c_str(), pubkey);
        LOGDBG(L"OnRegister: Profile is loaded (AgentId: %s)", _agentProfile.GetAgentId().c_str());
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        _agentProfile.Clear();
        LOGDBG(L"OnRegister: Profile not exist");
    }

    // Fail to load?
    // Try registration
    if(_agentProfile.GetAgentId().empty()) {
        try {
            LOGINF(L"OnRegister: Registering");
            Register();
            LOGINF(L"OnRegister: Registered(AgentId: %s)", _agentProfile.GetAgentId().c_str());
            _agentProfile.Load();
            nudf::crypto::CRsaPubKeyBlob pubkey;
            _agentProfile.GetSignKey().GetPublicKeyBlob(pubkey);
            _classifyProfile.Load(_nxPaths.GetClassifyConf().c_str(), pubkey);
        }
        catch(const nudf::CException& e) {
            LOGDBG(L"OnRegister: Fail to register agent (%08X, %S, %d, %S)", e.GetCode(), e.GetFile(), e.GetLine(), e.GetFunction());
            _agentProfile.Clear();
        }
    }

    // Load successfully?
    // Start heart beat to get bundles
    if(!_agentProfile.GetAgentId().empty()) {
        // Kill Rgister Timer
        _TimerRegister.Kill(FALSE);
        LOGSYSINF(L"Profile is loaded successfully");
        // Set HeartBeat Timer
        // At begining, we set timer to 1 minutes, if heartbeat succeed, reset it to profile defined time
        if(!_TimerHeartBeat.Set(5000 /*One Minute by Default*/, TRUE)) {
            LOGASSERT(L"OnRegister: Fail to set heartbeat timer (%d)", GetLastError());
        }
    }
}

VOID CEngine::OnTimerHeartBeat()
{
    static bool bInitialCheck = true;
    BOOL bKeyChanged = FALSE;
    BOOL bPolicyChanged = FALSE;

    assert(!_agentProfile.GetAgentId().empty());
    assert(0 != _agentProfile.GetSessionKey().GetBlobSize());

    try {

        if(bInitialCheck) {
            // This is first time heartbeat?
            LOGINF(L"Initial HeartBeat Check");

            // try to load from disk first
            try {
                _keyBundle.Load(_agentProfile.GetSessionKey());
                LOGINF(L"OnInitialHeartBeat: Key bundle is loaded");
            }
            catch(const nudf::CException& e) {
                UNREFERENCED_PARAMETER(e);
                _keyBundle.Clear();
                LOGINF(L"OnInitialHeartBeat: Key bundle not exist yet, waiting for heart beat.");
            }
            try {
                _PolicyServ.Load(_agentProfile.GetSessionKey());
                LOGINF(L"OnInitialHeartBeat: Policy bundle is loaded");
            }
            catch(const nudf::CException& e) {
                UNREFERENCED_PARAMETER(e);
                LOGINF(L"OnInitialHeartBeat: Policy bundle not exist yet, waiting for heart beat.");
            }
            
            // If key bundle and policy bundle are ready
            // Activate Engine
            if(_keyBundle.GetKeyBlobSize() != 0 && _PolicyServ.IsLoaded()) {
                

                LOGSYSINF(L"OnInitialHeartBeat: Key/Policy bundles have been loaded ...");
                if(!InterStartEngine()) {
                    LOGERR(GetLastError(), L"OnHeartBeat: Fail to activate engine.");
                }

                GetHeartBeatTime(&_lastHeartBeatTime);
                GetSessionServ()->NotifyClientTimeStamp(&GetPolicyServ()->GetPolicyTimestamp(), &_lastHeartBeatTime);

                _TimerLog.Set((_agentProfile.GetLogFrequency()) == 0 ? 3600000 /*1 hour*/ : (_agentProfile.GetLogFrequency()*1000), FALSE);
                _TimerCheckUpdate.Set(86400000 /*1 day*/, TRUE);
            }

            bInitialCheck = false;
        }

        // HeartBest to get latest data
        try {
            HeartBeat(&bKeyChanged, &bPolicyChanged);
        }
        catch(const nudf::CException& e) {
            bKeyChanged = FALSE;
            bPolicyChanged = FALSE;
            // If no valid bundles, throw exception
            if(_keyBundle.GetKeyBlobSize()==0 || !_PolicyServ.IsLoaded()) {
                LOGDBG(L"HeartBeat failed. (err=%d, file=%S, function=%S, line=%d)", e.GetCode(), e.GetFile(), e.GetFunction(), e.GetLine());
                throw e;
            }
        }

        if(!_Active) {

            //
            //  Try to load from file
            //
            if(_keyBundle.GetKeyBlobSize() == 0) {
                LOGDBG(L"OnHeartBeat: Fail to load key bundle.");
                throw WIN32ERROR2(ERROR_INVALID_DATA);
            }
            if(!_PolicyServ.IsLoaded()) {
                LOGDBG(L"OnHeartBeat: Fail to load policy bundle.");
                throw WIN32ERROR2(ERROR_INVALID_DATA);
            }

            LOGSYSINF(L"        Key/Policy bundles have been loaded ...");

            if(!InterStartEngine()) {
                LOGERR(GetLastError(), L"OnHeartBeat: Fail to activate engine.");
                throw WIN32ERROR();
            }
            
            GetHeartBeatTime(&_lastHeartBeatTime);
            GetSessionServ()->NotifyClientTimeStamp(&GetPolicyServ()->GetPolicyTimestamp(), &_lastHeartBeatTime);
            
            _TimerLog.Set((_agentProfile.GetLogFrequency()) == 0 ? 3600000 /*1 hour*/ : (_agentProfile.GetLogFrequency()*1000), FALSE);
            _TimerCheckUpdate.Set(86400000 /*1 day*/, TRUE);
        }
        else {
            if(bKeyChanged) {
                if(0 != _keyBundle.GetKeyBlobSize()) {
                    _FltServ.UpdateKeyChain(_keyBundle.GetKeyBlobs(), _keyBundle.GetKeyBlobSize());
                    if(_LogServ.AcceptLevel(LOGDEBUG)) {
                        const NXRM_KEY_BLOB* pb = _keyBundle.GetKeyBlobs();
                        int nKeyCount = (int)(_keyBundle.GetKeyBlobSize() / ((ULONG)sizeof(NXRM_KEY_BLOB)));
                        _LogServ.Push(nudf::util::log::CLogEntry(LOGDEBUG, NXMODNAME(), L"Keys are updated - %d %s set:", nKeyCount, (nKeyCount>1)?L"keys are":L"key is"));
                        for(int i=0; i<nKeyCount; i++) {
                            std::wstring wsKeyId = nudf::string::FromBytes<wchar_t>(pb[i].KeKeyId.Id, pb[i].KeKeyId.IdSize);
                            _LogServ.Push(nudf::util::log::CLogEntry(LOGDEBUG, NXMODNAME(), L"  Key%04d: %s", i, wsKeyId.c_str()));
                        }
                    }
                }
            }
            if(bPolicyChanged) {
                _FltServ.SetPolicyChanged();
                GetSessionServ()->NotifyClientTimeStamp(&GetPolicyServ()->GetPolicyTimestamp(), &_lastHeartBeatTime);
            }
        }
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
    }
}

VOID CEngine::OnTimerLog()
{
    CoInitialize(NULL);
    // Dump logs
    DumpAudit();

    // Upload Logs
    UploadAudit();

    CoUninitialize();
}

VOID CEngine::OnTimerCheckUpdate()
{
    static bool bInitCheck = true;
    CRmsUpdateService upsvc;

    if(bInitCheck) {

        DWORD dwDebug = 0;
        // Remove Init Flag
        bInitCheck = false;
        // GetRand Time
        srand((UINT)time(NULL));
        nudf::win::CRegKey nxrmkey;
        if(nxrmkey.Open(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\services\\nxrmserv", KEY_READ)) {
            if(!nxrmkey.GetValue(L"DebugUpdate", &dwDebug)) {
                dwDebug = 0;
            }
        }

        DWORD dwDelay = 5;
        //if(0 == dwDebug) {
        //    dwDelay = 120 + rand()%180;     // 2 - 5 hours (count in minutes, 120 to 300 minutes)
        //}
        //else {
            dwDelay = 2 + rand()%4;         // 2 -5 minutes
        //}

        LOGINF(L"AutoUpdate: initial update check will start after %d minutes",dwDelay);
        dwDelay *= 60000;
        Sleep(dwDelay);
    }

    try {

        std::wstring    pkg_version;
        std::wstring    pkg_url;
        std::wstring    pkg_checksum;
        WCHAR           wzUnzipDir[MAX_PATH] = {0};

        upsvc.Update(_agentProfile, pkg_version, pkg_url, pkg_checksum);
        if(pkg_version.empty() || pkg_url.empty()) { // || 40 != pkg_checksum.length()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        LOGINF(L"AutoUpdate: A new version (%s) Rights Management Client is available", pkg_version.c_str());

        // Prepare download folder
        GetTempFileNameW(_nxPaths.GetWinTempDir().c_str(), L"NXU", 0, wzUnzipDir);
        ::DeleteFileW(wzUnzipDir);
        if(!CreateDirectoryW(wzUnzipDir, NULL)) {
            LOGERR(GetLastError(), L"AutoUpdate: Fail to create temp folder for downloading (%s)", wzUnzipDir);
            throw WIN32ERROR();
        }


        // Download
        std::wstring wsZipFile;
        const WCHAR* name = wcsrchr(pkg_url.c_str(), L'/');
        if(NULL == name) {
            LOGERR(GetLastError(), L"AutoUpdate: Invalid URL (%s)", pkg_url.c_str());
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        wsZipFile = wzUnzipDir;
        wsZipFile += L"\\";
        wsZipFile +=(name + 1);
        ::DeleteFileW(wsZipFile.c_str());
        LOGINF(L"AutoUpdate: Downloading ...");
        HRESULT hr = URLDownloadToFileW(NULL, pkg_url.c_str(), wsZipFile.c_str(), 0, NULL);
        if(FAILED(hr) || INVALID_FILE_ATTRIBUTES == GetFileAttributesW(wsZipFile.c_str())) {
            LOGERR(hr, L"AutoUpdate: Download failed (0x%08X)", hr);
            throw WIN32ERROR2(hr);
        }

        LOGINF(L"AutoUpdate: Download succeed (%s)", wsZipFile.c_str());

        // Unpack
        nudf::util::CZip zip;
        if(!zip.Unzip(wsZipFile, wzUnzipDir)) {
            LOGERR(GetLastError(), L"AutoUpdate: Fail to unzip installer package (%d)", GetLastError());
            throw WIN32ERROR();
        }

        // Good, start installer now
        std::wstring wsSetUpCmd = L"C:\\Windows\\System32\\msiexec.exe /i \"";
        wsSetUpCmd += wzUnzipDir;
        wsSetUpCmd += L"\\NextLabs Rights Management.msi";
        wsSetUpCmd += L"\" /quiet /norestart /L*V \"";
        // The update.log cannot be put into RMC install folder because it will block installation process.
        wsSetUpCmd += L"C:\\Windows\\Temp\\NextLabs-RMC-Update.log\"";

        // Create Process
        STARTUPINFOW si;
        PROCESS_INFORMATION pi;
        memset(&si, 0, sizeof(si));
        memset(&pi, 0, sizeof(pi));
        si.cb = sizeof(si);
        if(!::CreateProcessW(NULL, (LPWSTR)wsSetUpCmd.c_str(), NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, wzUnzipDir, &si, &pi)) {
            LOGERR(GetLastError(), L"AutoUpdate: Fail to initiate install process (%d)", GetLastError());
            throw WIN32ERROR();
        }
        LOGINF(L"AutoUpdate: Installing ...");
        ResumeThread(pi.hThread);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
    }
}