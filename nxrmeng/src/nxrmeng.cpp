

#include <Windows.h>
#include <assert.h>

#include <nudf\exception.hpp>
#include <nudf\rwlock.hpp>
#include <nudf\path.hpp>
#include <nudf\shared\logdef.h>
#include <nudf\shared\moddef.h>

#include "nxrmeng.h"
#include "nxrminfo.hpp"
#include "engine.hpp"


nxrm::engine::CEngine*          nxrm::engine::gpEngine = NULL;
static nudf::util::CRwLock      gpEngineLock;
static nudf::win::CModulePath   gwsModulePath;
HINSTANCE                       _Instance = NULL;
CNxProdInfo                     _nxInfo;
CNxPaths                        _nxPaths;

DECLARE_NXRM_MODULE_ENGINE();


using namespace nxrm::engine;


BOOL WINAPI DllMain(HANDLE hInstance, DWORD dwReason, LPVOID lpvReserved)
{
    switch(dwReason)
    {
    case DLL_PROCESS_ATTACH:
        _Instance = (HINSTANCE)hInstance;
        ::DisableThreadLibraryCalls((HMODULE)hInstance);
        gwsModulePath.SetModule((HMODULE)hInstance);
        _nxInfo.Load();
        _nxPaths.Load();
        break;
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
        break;
    }

    return TRUE;
}

LONG WINAPI RmEngineCreate(_In_opt_ LPCWSTR wzWorkingRoot)
{
    LONG lRet = 0;
    nudf::win::CFilePath wsWorkingRoot;
    LPCWSTR TmpRoot = wzWorkingRoot;

    if(NULL != gpEngine) {
        return ERROR_SERVICE_EXISTS;
    }

    //
    //  Get Working Root
    //
    if(NULL != TmpRoot) {
        try {
            wsWorkingRoot.SetPath(TmpRoot);
        }
        catch(const nudf::CException& e) {
            UNREFERENCED_PARAMETER(e);
            TmpRoot = NULL;
        }
    }
    if(NULL == TmpRoot) {

        nudf::win::CFilePath wsParentDir;
        wsParentDir = gwsModulePath.GetParentDir();
        if(wsParentDir.IsRoot()) {
            wsWorkingRoot = wsParentDir;
        }
        else {
            if(0 == wsParentDir.CompareFileName(L"bin")) {
                wsWorkingRoot = wsParentDir.GetParentDir();
            }
            else {
                wsWorkingRoot = wsParentDir;
            }
        }
    }
        
    nudf::util::CRwExclusiveLocker Locker(&gpEngineLock);
    try {
        gpEngine = new nxrm::engine::CEngine();
        gpEngine->Initialize(wsWorkingRoot.GetPath().c_str());
    }
    catch(const nudf::CException& e) {
        lRet = e.GetCode();
        if(NULL != gpEngine) {
            delete gpEngine;
            gpEngine = NULL;
        }
    }

    return lRet;
}

LONG WINAPI RmEngineDestroy()
{
    nudf::util::CRwExclusiveLocker Locker(&gpEngineLock);
    LONG lRet = 0;
    
    if(gpEngine == NULL) {
        return ERROR_SERVICE_DOES_NOT_EXIST;
    }
    if(gpEngine->IsPending()) {
        return ERROR_SERVICE_CANNOT_ACCEPT_CTRL;
    }
    
    try {    
        if(gpEngine->IsRunning()) {
            gpEngine->Stop();
        }
        gpEngine->Cleanup();
        delete gpEngine;
        gpEngine = NULL;
    }
    catch(const nudf::CException& e) {
        lRet = e.GetCode();
    }

    return lRet;
}

LONG WINAPI RmEngineStart()
{
    nudf::util::CRwExclusiveLocker Locker(&gpEngineLock);
    LONG lRet = 0;
    
    if(gpEngine == NULL) {
        return ERROR_SERVICE_DOES_NOT_EXIST;
    }
    if(gpEngine->IsPending()) {
        return ERROR_SERVICE_CANNOT_ACCEPT_CTRL;
    }
    if(!gpEngine->IsStopped()) {
        return ERROR_SERVICE_ALREADY_RUNNING;
    }
    
    try {
        gpEngine->Start();
    }
    catch(const nudf::CException& e) {
        lRet = e.GetCode();
    }

    return lRet;
}

LONG WINAPI RmEngineStop()
{
    nudf::util::CRwExclusiveLocker Locker(&gpEngineLock);
    LONG lRet = 0;
    
    if(gpEngine == NULL) {
        return ERROR_SERVICE_DOES_NOT_EXIST;
    }
    if(gpEngine->IsPending()) {
        return ERROR_SERVICE_CANNOT_ACCEPT_CTRL;
    }
    if(!gpEngine->IsRunning() && !gpEngine->IsPaused()) {
        return ERROR_SERVICE_NOT_ACTIVE;
    }
    
    try {
        gpEngine->Stop();
    }
    catch(const nudf::CException& e) {
        lRet = e.GetCode();
    }

    return lRet;
}

LONG WINAPI RmEnginePause()
{
    nudf::util::CRwExclusiveLocker Locker(&gpEngineLock);
    LONG lRet = 0;
    
    if(gpEngine == NULL) {
        return ERROR_SERVICE_DOES_NOT_EXIST;
    }
    if(gpEngine->IsPending()) {
        return ERROR_SERVICE_CANNOT_ACCEPT_CTRL;
    }
    if(gpEngine->IsStopped()) {
        return ERROR_SERVICE_NOT_ACTIVE;
    }
    if(gpEngine->IsPaused()) {
        return 0L;
    }

    assert(gpEngine->IsRunning());
    
    try {
        gpEngine->Pause();
    }
    catch(const nudf::CException& e) {
        lRet = e.GetCode();
    }

    return lRet;
}

LONG WINAPI RmEngineContinue()
{
    nudf::util::CRwExclusiveLocker Locker(&gpEngineLock);
    LONG lRet = 0;
    
    if(gpEngine == NULL) {
        return ERROR_SERVICE_DOES_NOT_EXIST;
    }
    if(gpEngine->IsPending()) {
        return ERROR_SERVICE_CANNOT_ACCEPT_CTRL;
    }
    if(!gpEngine->IsStopped()) {
        return ERROR_SERVICE_NOT_ACTIVE;
    }
    if(gpEngine->IsRunning()) {
        return 0L;
    }
    assert(gpEngine->IsPaused());
    
    try {
        gpEngine->Continue();
    }
    catch(const nudf::CException& e) {
        lRet = e.GetCode();
    }

    return lRet;
}

BOOL WINAPI RmEngineLogAccept(_In_ ULONG Level)
{
    if(gpEngine == NULL) {
        return FALSE;
    }
    if(nxrm::engine::_LogServ.IsRunning()) {
        return nxrm::engine::_LogServ.AcceptLevel((LOGLEVEL)Level);
    }
    return FALSE;
}

LONG WINAPI RmEngineLog(_In_ LPCWSTR Info)
{
    LONG lRet = 0;
    
    if(gpEngine == NULL) {
        return ERROR_SERVICE_DOES_NOT_EXIST;
    }

    if(nxrm::engine::_LogServ.IsRunning()) {
        if(!nxrm::engine::_LogServ.Push(Info)) {
            lRet = ERROR_OUTOFMEMORY;
        }
    }

    return lRet;
}

LONG WINAPI RmOnSessionLogon(_In_ ULONG dwSessionId)
{
    LONG lRet = 0;
    
    if(gpEngine == NULL) {
        return ERROR_SERVICE_DOES_NOT_EXIST;
    }

    try {
        gpEngine->OnSessionLogon(dwSessionId);
    }
    catch(const nudf::CException& e) {
        lRet = e.GetCode();
    }

    return lRet;
}

LONG WINAPI RmOnSessionLogoff(_In_ ULONG dwSessionId)
{
    LONG lRet = 0;
    
    if(gpEngine == NULL) {
        return ERROR_SERVICE_DOES_NOT_EXIST;
    }

    try {
        gpEngine->OnSessionLogoff(dwSessionId);
    }
    catch(const nudf::CException& e) {
        lRet = e.GetCode();
    }

    return lRet;
}

LONG WINAPI RmOnSessionConn(_In_ ULONG dwSessionId)
{
    LONG lRet = 0;
    
    if(gpEngine == NULL) {
        return ERROR_SERVICE_DOES_NOT_EXIST;
    }
    if(gpEngine->IsPending()) {
        return ERROR_SERVICE_CANNOT_ACCEPT_CTRL;
    }
    if(!gpEngine->IsRunning()) {
        return ERROR_SERVICE_NOT_ACTIVE;
    }

    try {
        gpEngine->OnSessionConn(dwSessionId);
    }
    catch(const nudf::CException& e) {
        lRet = e.GetCode();
    }

    return lRet;
}

LONG WINAPI RmOnSessionDisconn(_In_ ULONG dwSessionId)
{
    LONG lRet = 0;
    
    if(gpEngine == NULL) {
        return ERROR_SERVICE_DOES_NOT_EXIST;
    }
    if(gpEngine->IsPending()) {
        return ERROR_SERVICE_CANNOT_ACCEPT_CTRL;
    }
    if(!gpEngine->IsRunning()) {
        return ERROR_SERVICE_NOT_ACTIVE;
    }

    try {
        gpEngine->OnSessionDisconn(dwSessionId);
    }
    catch(const nudf::CException& e) {
        lRet = e.GetCode();
    }

    return lRet;
}

LONG WINAPI RmOnSessionRemoteConn(_In_ ULONG dwSessionId)
{
    LONG lRet = 0;
    
    if(gpEngine == NULL) {
        return ERROR_SERVICE_DOES_NOT_EXIST;
    }
    if(gpEngine->IsPending()) {
        return ERROR_SERVICE_CANNOT_ACCEPT_CTRL;
    }
    if(!gpEngine->IsRunning()) {
        return ERROR_SERVICE_NOT_ACTIVE;
    }

    try {
        gpEngine->OnSessionRemoteConn(dwSessionId);
    }
    catch(const nudf::CException& e) {
        lRet = e.GetCode();
    }

    return lRet;
}

LONG WINAPI RmOnSessionRemoteDisconn(_In_ ULONG dwSessionId)
{
    LONG lRet = 0;
    
    if(gpEngine == NULL) {
        return ERROR_SERVICE_DOES_NOT_EXIST;
    }
    if(gpEngine->IsPending()) {
        return ERROR_SERVICE_CANNOT_ACCEPT_CTRL;
    }
    if(!gpEngine->IsRunning()) {
        return ERROR_SERVICE_NOT_ACTIVE;
    }

    try {
        gpEngine->OnSessionRemoteDisconn(dwSessionId);
    }
    catch(const nudf::CException& e) {
        lRet = e.GetCode();
    }

    return lRet;
}

LONG WINAPI RmNotify(_In_ ULONG dwSessionId, _In_ LPCWSTR wzTitle, _In_ LPCWSTR wzNotification)
{
    LONG lRet = 0;
    
    if(gpEngine == NULL) {
        return ERROR_SERVICE_DOES_NOT_EXIST;
    }
    if(gpEngine->IsPending()) {
        return ERROR_SERVICE_CANNOT_ACCEPT_CTRL;
    }
    if(!gpEngine->IsRunning()) {
        return ERROR_SERVICE_NOT_ACTIVE;
    }

    try {
        gpEngine->GetSessionServ()->NotifyClient(dwSessionId, wzTitle, wzNotification);
    }
    catch(const nudf::CException& e) {
        lRet = e.GetCode();
    }

    return lRet;
}