

#include <Windows.h>
#include <Shldisp.h>
#include <AccCtrl.h>
#include <Aclapi.h>

#include <nudf\shared\enginectl.h>
#include <nudf\exception.hpp>
#include <nudf\host.hpp>
#include <nudf\resutil.hpp>

#include <nudf\nxrmres.h>

#include "nxrmeng.h"
#include "engine.hpp"
#include "restclient.hpp"
#include "ipcserv.hpp"


using namespace nxrm::engine;

static LRESULT LogCollectRoutine(DWORD dwRequestorId);

CIpcServ::CIpcServ() : nudf::ipc::CPipeServer(NXRM_ENGINE_IPC_NAME, NXRMSERV_IPC_BUFSIZE, NXRMSERV_IPC_TIMEOUT)
{
    AllowEveryoneAccess();
}

CIpcServ::~CIpcServ()
{
}

VOID CIpcServ::Initialize()
{
    SetName(gpEngine->GetSessionGuid().c_str());
}

VOID CIpcServ::Cleanup()
{
}

VOID CIpcServ::OnRequest(_In_ const UCHAR* pbRequest, _In_ ULONG cbRequest, _In_ UCHAR* pbReply, _In_ ULONG cbReply, _Out_ PULONG pcbValidReply, _Out_ PBOOL pfClose)
{
    PCRM_REQUEST_HEADER Header = (PCRM_REQUEST_HEADER)pbRequest;
    PRM_REPLY_HEADER Reply = (PRM_REPLY_HEADER)pbReply;

    *pcbValidReply = 0UL;
    *pfClose = FALSE;

    if(cbRequest < sizeof(RM_REQUEST_HEADER)) {
        return;
    }
    if(Header->Size > cbRequest) {
        return;
    }

    DWORD dwType = DEVICE_TYPE_FROM_CTL_CODE(Header->CtlCode);

    switch(Header->CtlCode)
    {
        // Engine Control
    case ENGINE_CTL_QUERYSTATUS:
        OnQueryStatus((const RM_QUERYSTATUS_REQUEST*)pbRequest, &((PRM_QUERYSTATUS_REPLY)pbReply)->Status);
        Reply->Size = sizeof(RM_QUERYSTATUS_REPLY);
        Reply->CtlCode = ENGINE_CTL_QUERYSTATUS;
        Reply->Result = 1;
        *pcbValidReply = sizeof(RM_QUERYSTATUS_REPLY);
        break;
    case ENGINE_CTL_STOP:
        Reply->Size = sizeof(RM_REPLY_HEADER);
        Reply->CtlCode = ENGINE_CTL_STOP;
        Reply->Result = ERROR_INVALID_FUNCTION;
        *pcbValidReply = sizeof(RM_REPLY_HEADER);
        break;
    case ENGINE_CTL_START:
        Reply->Size = sizeof(RM_REPLY_HEADER);
        Reply->CtlCode = ENGINE_CTL_START;
        Reply->Result = ERROR_INVALID_FUNCTION;
        *pcbValidReply = sizeof(RM_REPLY_HEADER);
        break;
    case ENGINE_CTL_PAUSE:
        Reply->Size = sizeof(RM_REPLY_HEADER);
        Reply->CtlCode = ENGINE_CTL_PAUSE;
        Reply->Result = ERROR_INVALID_FUNCTION;
        *pcbValidReply = sizeof(RM_REPLY_HEADER);
        break;
    case ENGINE_CTL_CONTINUE:
        Reply->Size = sizeof(RM_REPLY_HEADER);
        Reply->CtlCode = ENGINE_CTL_CONTINUE;
        Reply->Result = ERROR_INVALID_FUNCTION;
        *pcbValidReply = sizeof(RM_REPLY_HEADER);
        break;
    case ENGINE_CTL_UPDATE:
        OnUpdatePolicy((const RM_REQUEST_HEADER*)pbRequest);
        Reply->Size = sizeof(RM_REPLY_HEADER);
        Reply->CtlCode = ENGINE_CTL_UPDATE;
        Reply->Result = 0;
        *pcbValidReply = sizeof(RM_REPLY_HEADER);
        break;
    case ENGINE_CTL_DBGCTL:
        gpEngine->GetLogServ()->SetAcceptLevel((LOGLEVEL)((const RM_SETDEBUG_REQUEST*)pbRequest)->LogLevel);
        Reply->Size = sizeof(RM_SETDEBUG_REPLY);
        Reply->CtlCode = ENGINE_CTL_DBGCTL;
        Reply->Result = 0;
        ((PRM_SETDEBUG_REPLY)pbRequest)->LogLevel = gpEngine->GetLogServ()->GetAcceptLevel();
        *pcbValidReply = sizeof(RM_SETDEBUG_REPLY);
        break;
    case ENGINE_CTL_DBGCOLLECT:
        if(!OnCollectLog((const RM_REQUEST_HEADER*)pbRequest)) {
            Reply->Result = -1;
        }
        else {
            Reply->Result = 0;
        }
        Reply->Size = sizeof(RM_REPLY_HEADER);
        Reply->CtlCode = ENGINE_CTL_DBGCOLLECT;
        *pcbValidReply = sizeof(RM_REPLY_HEADER);
        break;

        // Authentication
    case AUTHNENGINE_CTL_LOGON:
    case AUTHNENGINE_CTL_LOGOFF:
        Reply->Size = sizeof(RM_REPLY_HEADER);
        Reply->CtlCode = AUTHNENGINE_CTL_LOGON;
        Reply->Result = OnAuthn((const RM_AUTHN_REQUEST*)pbRequest);
        *pcbValidReply = sizeof(RM_REPLY_HEADER);
        break;

        // Authorization
    case AUTHZENGINE_CTL_EVAL:
        Reply->Size = sizeof(RM_REPLY_HEADER);
        Reply->CtlCode = AUTHZENGINE_CTL_EVAL;
        Reply->Result = ERROR_INVALID_FUNCTION;
        *pcbValidReply = sizeof(RM_REPLY_HEADER);
        break;
    case AUTHZENGINE_CTL_UPDATE:
        Reply->Size = sizeof(RM_REPLY_HEADER);
        Reply->CtlCode = AUTHZENGINE_CTL_UPDATE;
        Reply->Result = ERROR_INVALID_FUNCTION;
        *pcbValidReply = sizeof(RM_REPLY_HEADER);
        break;

        // Key Management
    case KEYENGINE_CTL_GETKEY:
        Reply->Size = sizeof(RM_REPLY_HEADER);
        Reply->CtlCode = KEYENGINE_CTL_GETKEY;
        Reply->Result = ERROR_INVALID_FUNCTION;
        *pcbValidReply = sizeof(RM_REPLY_HEADER);
        break;
    case KEYENGINE_CTL_REVOKEKEY:
        Reply->Size = sizeof(RM_REPLY_HEADER);
        Reply->CtlCode = KEYENGINE_CTL_REVOKEKEY;
        Reply->Result = ERROR_INVALID_FUNCTION;
        *pcbValidReply = sizeof(RM_REPLY_HEADER);
        break;
    case KEYENGINE_CTL_DISPKEY:
        Reply->Size = sizeof(RM_REPLY_HEADER);
        Reply->CtlCode = KEYENGINE_CTL_DISPKEY;
        Reply->Result = ERROR_INVALID_FUNCTION;
        *pcbValidReply = sizeof(RM_REPLY_HEADER);
        break;

        // Key Management
    case AUDITENGINE_CTL_AUDIT:
        Reply->Size = sizeof(RM_REPLY_HEADER);
        Reply->CtlCode = AUDITENGINE_CTL_AUDIT;
        Reply->Result = ERROR_INVALID_FUNCTION;
        *pcbValidReply = sizeof(RM_REPLY_HEADER);
        break;

        // Hook
    case HOOKENGINE_CTL_STOP:
        Reply->Size = sizeof(RM_REPLY_HEADER);
        Reply->CtlCode = HOOKENGINE_CTL_STOP;
        Reply->Result = ERROR_INVALID_FUNCTION;
        *pcbValidReply = sizeof(RM_REPLY_HEADER);
        break;
    case HOOKENGINE_CTL_START:
        Reply->Size = sizeof(RM_REPLY_HEADER);
        Reply->CtlCode = HOOKENGINE_CTL_START;
        Reply->Result = ERROR_INVALID_FUNCTION;
        *pcbValidReply = sizeof(RM_REPLY_HEADER);
        break;

        // VHD
    case VHDENGINE_CTL_SHOW:
        Reply->Size = sizeof(RM_REPLY_HEADER);
        Reply->CtlCode = VHDENGINE_CTL_SHOW;
        Reply->Result = ERROR_INVALID_FUNCTION;
        *pcbValidReply = sizeof(RM_REPLY_HEADER);
        break;
    case VHDENGINE_CTL_HIDE:
        Reply->Size = sizeof(RM_REPLY_HEADER);
        Reply->CtlCode = VHDENGINE_CTL_HIDE;
        Reply->Result = ERROR_INVALID_FUNCTION;
        *pcbValidReply = sizeof(RM_REPLY_HEADER);
        break;
    case VHDENGINE_CTL_LIST:
        Reply->Size = sizeof(RM_REPLY_HEADER);
        Reply->CtlCode = VHDENGINE_CTL_LIST;
        Reply->Result = ERROR_INVALID_FUNCTION;
        *pcbValidReply = sizeof(RM_REPLY_HEADER);
        break;

    default:
        break;
    }
}

VOID CIpcServ::OnQueryStatus(_In_ const RM_QUERYSTATUS_REQUEST* request, _Out_ RM_ENGINE_STATUS* status)
{
    DWORD     dwRequestorPid = request->Header.RequestorPid;

    memset(status, 0, sizeof(RM_ENGINE_STATUS));
    status->State = gpEngine->GetEngineState();
    status->LogLevel = gpEngine->GetLogServ()->GetAcceptLevel();
    status->Connected = gpEngine->IsConnectedToServer() ? 1 : 0;
    status->PolicyTimestamp = gpEngine->GetPolicyServ()->GetPolicyTimestamp();
    status->LastUpdateTime = gpEngine->GetLastHeartBeatTime();
    wcsncpy_s(status->ProductVersion, 64, _nxInfo.GetProductVersion().c_str(), _TRUNCATE);
    if(!gpEngine->GetRmsAgentProfile().GetRmsServer().empty()) {
        wcsncpy_s(status->CurrentServer, 64, gpEngine->GetRmsAgentProfile().GetRmsServer().c_str(), _TRUNCATE);
    }

    DWORD dwSessionId = -1;
    if(ProcessIdToSessionId(dwRequestorPid, &dwSessionId) && -1!=dwSessionId) {
        std::shared_ptr<nxrm::engine::CActiveSession> pSession = gpEngine->GetSessionServ()->FindSession(dwSessionId);
        if(NULL != pSession) {
            wcsncpy_s(status->CurrentUser,
                      64,
                      pSession->GetCurrentUser().GetAccountName().c_str(),
                      _TRUNCATE);
        }
    }
}

void CIpcServ::OnUpdatePolicy(_In_ const RM_REQUEST_HEADER* request)
{
    DWORD dwSessionId = -1;
    DWORD dwRequestorPid = request->RequestorPid;
    
    std::wstring wsTitle;
    std::wstring wsInfo;
    
    wsTitle = nudf::util::res::LoadMessage(gpEngine->GetResModule(), IDS_TRAY_INFO_TITLE, 256, LANG_NEUTRAL, L"NextLabs Rights Management");
    
    // Get requestor
    if(gpEngine->GetRmsAgentProfile().GetAgentId().empty()) {
        gpEngine->TriggerRegister();
        if(ProcessIdToSessionId(dwRequestorPid, &dwSessionId) && -1!=dwSessionId) {
            wsInfo = nudf::util::res::LoadMessage(gpEngine->GetResModule(), IDS_NOTIFY_REGISTERING, 1024, LANG_NEUTRAL, L"Registering");
            RmNotify(dwSessionId, wsTitle.c_str(), wsInfo.c_str());
        }
    }
    else {
        gpEngine->TriggerHeartBeat();
        if(ProcessIdToSessionId(dwRequestorPid, &dwSessionId) && -1!=dwSessionId) {
            wsInfo = nudf::util::res::LoadMessage(gpEngine->GetResModule(), IDS_NOTIFY_UPDATING_POLICY, 1024, LANG_NEUTRAL, L"Checking for latest policy ...");
            RmNotify(dwSessionId, wsTitle.c_str(), wsInfo.c_str());
        }
    }
}

DWORD CIpcServ::OnAuthn(_In_ const RM_AUTHN_REQUEST* request)
{
    DWORD dwResult = 0;

    if (request->Header.CtlCode == AUTHNENGINE_CTL_LOGON) {
        if (request->Logon.Name[0] == L'\0' || request->Logon.Domain[0] == L'\0') {
            return ERROR_INVALID_PARAMETER;
        }
        if (request->Logon.Password[0] == L'\0') {
            return ERROR_INVALID_PASSWORD;
        }
        // Talk to RMS
        NX::rest::request_authn req(request->Logon.Name, request->Logon.Domain, request->Logon.Password);
        NX::rest::response_authn res;
        NX::rest::rmclient client;
        client.request(&req, &res);
        if (0 == res.result()) {
            // update session's user information
        }
        return (DWORD)res.result();
    }
    else if (request->Header.CtlCode == AUTHNENGINE_CTL_LOGOFF) {
        // Succeed
        dwResult = 0;
    }
    else {
        assert(FALSE);
        __assume(0);
    }

    return dwResult;
}

BOOL CIpcServ::OnCollectLog(_In_ const RM_REQUEST_HEADER* request)
{
    DWORD dwThreadId = 0;
    HANDLE hThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)LogCollectRoutine, (LPVOID)(ULONG_PTR)request->RequestorPid, 0, &dwThreadId);
    if(NULL == hThread) {
        return FALSE;
    }

    CloseHandle(hThread);
    return TRUE;
}

void FindAllLogFiles(std::vector<std::wstring>& logfiles)
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW fd;
    std::wstring wsPattern = _nxPaths.GetRootDir() + L"\\DebugDump*.txt";

    hFind = FindFirstFileW(wsPattern.c_str(), &fd);
    if(INVALID_HANDLE_VALUE == hFind) {
        return;
    }

    do {
        std::wstring wsFile = fd.cFileName;
        logfiles.push_back(wsFile);
    } while(FindNextFileW(hFind, &fd));

    FindClose(hFind);
}

void FindSubFiles(const std::wstring& folder, std::vector<std::wstring>& subfiles)
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW fd;
    std::wstring wsPattern = folder + L"\\*";

    hFind = FindFirstFileW(wsPattern.c_str(), &fd);
    if(INVALID_HANDLE_VALUE == hFind) {
        return;
    }

    do {
        std::wstring wsFile = fd.cFileName;
        subfiles.push_back(wsFile);
    } while(FindNextFileW(hFind, &fd));

    FindClose(hFind);
}

BOOL DeleteFolder(const std::wstring& folder)
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW fd;
    std::wstring wsPattern = folder + L"\\*";

    std::vector<std::wstring> vFolders;
    std::vector<std::wstring> vFiles;

    // Find all sub folders and files
    hFind = FindFirstFileW(wsPattern.c_str(), &fd);
    if(INVALID_HANDLE_VALUE != hFind) {
        do {

            if(0 == _wcsicmp(L".", fd.cFileName) || 0 == _wcsicmp(L"..", fd.cFileName)) {
                continue;
            }
            if(FILE_ATTRIBUTE_SYSTEM == (FILE_ATTRIBUTE_SYSTEM & fd.dwFileAttributes)) {
                continue;
            }

            if(FILE_ATTRIBUTE_DIRECTORY == (FILE_ATTRIBUTE_DIRECTORY & fd.dwFileAttributes)) {
                vFolders.push_back(fd.cFileName);
            }
            else {
                vFiles.push_back(fd.cFileName);
            }
        } while(FindNextFileW(hFind, &fd));
        FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }

    // Delete sub-files
    for(std::vector<std::wstring>::const_iterator it=vFiles.begin(); it!=vFiles.end(); ++it) {
        std::wstring file = folder + L"\\" + (*it);
        ::SetFileAttributesW(file.c_str(), FILE_ATTRIBUTE_NORMAL);
        ::DeleteFileW(file.c_str());
    }
    
    // Delete sub-folders
    for(std::vector<std::wstring>::const_iterator it=vFolders.begin(); it!=vFolders.end(); ++it) {
        std::wstring file = folder + L"\\" + (*it);
        DeleteFolder(file);
    }

    // Finally remove current folder
    return ::RemoveDirectoryW(folder.c_str());
}

LRESULT LogCollectRoutine(DWORD dwRequestorId)
{
    DWORD dwSessionId = -1;
    DWORD dwRequestorPid = dwRequestorId;
    std::wstring wsDesktop;
    std::wstring wsDbgDir;
    std::wstring wsDbgZipFile;
    WCHAR        wzDbgTime[128] = {0};
    SYSTEMTIME   st;

    std::wstring wsPolicyXml;
    std::wstring wsOrigPolicyXml;
    std::wstring wsTargetFile;

    std::wstring wsTitle;
    std::wstring wsInfo;

    // Get requestor
    if(!ProcessIdToSessionId(dwRequestorPid, &dwSessionId) || -1==dwSessionId) {
        return 0;
    }

    std::shared_ptr<nxrm::engine::CActiveSession> pSession = gpEngine->GetSessionServ()->FindSession(dwSessionId);
    assert(NULL != pSession);
    wsDesktop = pSession->GetKnownDirs().GetDesktopDir();

    nudf::win::CHost host;
        

    GetLocalTime(&st);
    swprintf_s(wzDbgTime, 128, L"%04d%02d%02d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    wsDbgDir     = wsDesktop + L"\\NxDbg-" + host.GetHostName() + L"-" + wzDbgTime;
    wsDbgZipFile = wsDesktop + L"\\NxDbg-" + host.GetHostName() + L"-" + wzDbgTime + L".zip";

    // 1. Decrypt data
    wsPolicyXml = _nxPaths.GetConfDir();
    wsPolicyXml += L"\\policy.xml";
    // Try to decrypt policy
    // Only if the policy file exist, and the agent profiel has been loaded
    if( INVALID_FILE_ATTRIBUTES!=GetFileAttributesW(_nxPaths.GetPolicyConf().c_str())
        && !gpEngine->GetRmsAgentProfile().GetAgentId().empty()
        && gpEngine->GetRmsAgentProfile().GetSessionKey().GetBlobSize() != 0) {

        if(!gpEngine->DecryptPolicyBundle(wsPolicyXml, false)) {
            LOGWARN(GetLastError(), L"LogCollecting: Fail to decrypt policy bundle");
        }
    }

    wsOrigPolicyXml = _nxPaths.GetConfDir();
    wsOrigPolicyXml += L"\\policy-orig.xml";
    if( INVALID_FILE_ATTRIBUTES!=GetFileAttributesW(_nxPaths.GetPolicyConf().c_str())
        && !gpEngine->GetRmsAgentProfile().GetAgentId().empty()
        && gpEngine->GetRmsAgentProfile().GetSessionKey().GetBlobSize() != 0) {

        if(!gpEngine->DecryptPolicyBundle(wsOrigPolicyXml, true)) {
            LOGWARN(GetLastError(), L"LogCollecting: Fail to decrypt original policy bundle");
        }
    }

    
    SECURITY_ATTRIBUTES     sa = {0};
    PSECURITY_DESCRIPTOR    sd = NULL;
    EXPLICIT_ACCESS         ea;
    PACL                    acl = NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
    PSID                    pEveryoneSid = NULL;

    // 2. Zip all the files
    CoInitialize(NULL);
    try {

        HRESULT hr;
        CComPtr<IShellDispatch> spISD;
        CComPtr<Folder>         spToFolder;
        CComVariant vDir;
        CComVariant vFile;
        CComVariant vOpt;

        
        if(!::CreateDirectoryW(wsDbgDir.c_str(), NULL)) {
            throw WIN32ERROR();
        }

        std::vector<std::wstring> conffiles;
        CopyFileW(_nxPaths.GetConfDir().c_str(), wsTargetFile.c_str(), FALSE);
        std::vector<std::wstring> logfiles;
        FindSubFiles(_nxPaths.GetConfDir(), conffiles);
        for(std::vector<std::wstring>::const_iterator it=conffiles.begin(); it!=conffiles.end(); ++it) {
            std::wstring wsSource = _nxPaths.GetConfDir() + L"\\" + (*it);
            wsTargetFile = wsDbgDir + L"\\" + (*it);
            CopyFileW(wsSource.c_str(), wsTargetFile.c_str(), FALSE);
        }

        FindAllLogFiles(logfiles);
        for(std::vector<std::wstring>::const_iterator it=logfiles.begin(); it!=logfiles.end(); ++it) {
            std::wstring wsSource = _nxPaths.GetRootDir() + L"\\" + (*it);
            wsTargetFile = wsDbgDir + L"\\" + (*it);
            CopyFileW(wsSource.c_str(), wsTargetFile.c_str(), FALSE);
        }

        
        AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pEveryoneSid);
        ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
        ea.grfAccessPermissions = GENERIC_READ | GENERIC_WRITE;
        ea.grfAccessMode        = SET_ACCESS;
        ea.grfInheritance       = NO_INHERITANCE;
        ea.Trustee.TrusteeForm  = TRUSTEE_IS_SID;
        ea.Trustee.TrusteeType  = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ea.Trustee.ptstrName    = (LPWSTR)pEveryoneSid;
        SetEntriesInAclW(1, &ea, NULL, &acl);
        sd = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
        InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(sd, TRUE, acl, FALSE);
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = sd;
        sa.bInheritHandle = FALSE;
        HANDLE hZip = ::CreateFileW(wsDbgZipFile.c_str(), GENERIC_WRITE|GENERIC_READ, 0, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if(INVALID_HANDLE_VALUE == hZip) {
            throw WIN32ERROR();
        }
        CloseHandle(hZip);
        hZip = INVALID_HANDLE_VALUE;


        vDir.vt = VT_BSTR;
        vDir.bstrVal = ::SysAllocStringLen(NULL, (UINT)wsDbgZipFile.length() + 3);
        RtlSecureZeroMemory(vDir.bstrVal, sizeof(WCHAR)*(wsDbgZipFile.length() + 3));
        memcpy(vDir.bstrVal, wsDbgZipFile.c_str(), sizeof(WCHAR)*wsDbgZipFile.length());

        vFile.vt = VT_BSTR;
        vFile.bstrVal = ::SysAllocStringLen(NULL, (UINT)wsDbgDir.length() + 3);
        RtlSecureZeroMemory(vFile.bstrVal, sizeof(WCHAR)*(wsDbgDir.length() + 3));
        memcpy(vFile.bstrVal, wsDbgDir.c_str(), sizeof(WCHAR)*wsDbgDir.length());

        vOpt.vt = VT_I4;
        vOpt.lVal = 0x0614; //FOF_NO_UI;  //Do not display a progress dialog box, not useful in compression
        
        hr = ::CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_IShellDispatch, (void **)&spISD);
        if (SUCCEEDED(hr) && NULL!=spISD.p) {

            // Destination is our zip file
            hr= spISD->NameSpace(vDir, &spToFolder);
            if (SUCCEEDED(hr) && NULL != spToFolder.p) {
                
                // Copying and compressing the source files to our zip
                hr = spToFolder->CopyHere(vFile, vOpt);

                // CopyHere() creates a separate thread to copy files and 
                // it may happen that the main thread exits before the 
                // copy thread is initialized. So we put the main thread to sleep 
                // for a second to give time for the copy thread to start.
                while(TRUE) {
                    Sleep(500);
                    hZip = ::CreateFileW(wsDbgZipFile.c_str(), GENERIC_WRITE|GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                    if(INVALID_HANDLE_VALUE != hZip) {
                        CloseHandle(hZip);
                        hZip = INVALID_HANDLE_VALUE;
                        break;
                    }
                }

                // Done
                spToFolder.Release();
            }

            spISD.Release();
        }

        ::DeleteFileW(wsPolicyXml.c_str());
        DeleteFolder(wsDbgDir);

        // Notify user that Debug Log has been collected
        const WCHAR* dbgFileName = wcsrchr(wsDbgZipFile.c_str(), L'\\');
        dbgFileName = (NULL==dbgFileName) ? wsDbgZipFile.c_str() : (dbgFileName+1);
        wsTitle = nudf::util::res::LoadMessage(gpEngine->GetResModule(), IDS_TRAY_INFO_TITLE, 256, LANG_NEUTRAL, L"NextLabs Rights Management");
        wsInfo = nudf::util::res::LoadMessageEx(gpEngine->GetResModule(), IDS_NOTIFY_DBGLOG_COLLECTED, 1024, LANG_NEUTRAL, L"Debug data file (%s) has been generated on your desktop", dbgFileName);
        RmNotify(dwSessionId, wsTitle.c_str(), wsInfo.c_str());
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        if(NULL != acl) {LocalFree(acl); acl = NULL;}
        if(NULL != sd) {LocalFree(sd); sd = NULL;}
        if(NULL != pEveryoneSid) {FreeSid(pEveryoneSid); pEveryoneSid = NULL;}
        ::DeleteFileW(wsPolicyXml.c_str());
        DeleteFolder(wsDbgDir);
        wsTitle = nudf::util::res::LoadMessage(gpEngine->GetResModule(), IDS_TRAY_INFO_TITLE, 256, LANG_NEUTRAL, L"NextLabs Rights Management");
        wsInfo = nudf::util::res::LoadMessage(gpEngine->GetResModule(), IDS_NOTIFY_DBGLOG_COLLECT_FAILED, 1024, LANG_NEUTRAL, L"Fail to collect debug data");
        RmNotify(dwSessionId, wsTitle.c_str(), wsInfo.c_str());
    }
    CoUninitialize();

    return 0;
}