
#include <Windows.h>
#include <Shlobj.h>
#include <assert.h>

#include <nudf\exception.hpp>
#include <nudf\path.hpp>
#include <nudf\listentry.hpp>
#include <nudf\user.hpp>
#include <nudf\session.hpp>
#include <nudf\regex.hpp>
#include <nudf\time.hpp>
#include <nudf\resutil.hpp>
#include <nudf\bitmap.hpp>
#include <nudf\shared\enginectl.h>
#include <nudf\shared\obutil.h>
#include <nudf\shared\officelayout.h>

#include <nudf\nxrmres.h>

#include "nxrmeng.h"
#include "engine.hpp"
#include "drvserv.hpp"
#include "nxrmdrv.h"
#include "nxrmdrvman.h"


using namespace nxrm::engine;


typedef HANDLE (WINAPI* NXRMDRV_CREATE_MANAGER)(NXRMDRV_CALLBACK_NOTIFY, LOGAPI_LOG, LOGAPI_ACCEPT, PVOID);
typedef ULONG (WINAPI* NXRMDRV_POSTCHECK_OBLIGATIONS_RESPONSE)(HANDLE, PVOID, NXRM_OBLIGATION*, ULONG);
typedef ULONG (WINAPI* NXRMDRV_POST_OFFICE_GETCUSTOMUI_RESPONSE)(HANDLE, PVOID, OFFICE_GET_CUSTOMUI_RESPONSE*);
typedef ULONG (WINAPI* NXRMDRV_QUERY_PROCESSINFO)(HANDLE, ULONG, NXRM_PROCESS_ENTRY*);
typedef ULONG (WINAPI* NXRMDRV_IS_REQUEST_CANCELED)(HANDLE, PVOID);
typedef ULONG (WINAPI* NXRMDRV_IS_PROCESS_ALIVE)(HANDLE,  ULONG);
typedef ULONG (WINAPI* NXRMDRV_START)(HANDLE);
typedef ULONG (WINAPI* NXRMDRV_STOP)(HANDLE);
typedef ULONG (WINAPI* NXRMDRV_INCREASE_POLICY_SERIALNO)(HANDLE);
typedef ULONG (WINAPI* NXRMDRV_CLOSE_MANAGER)(HANDLE);
typedef ULONG (WINAPI* NXRMDRV_POST_SAVEAS_FORECAST_RESPONSE)(HANDLE,PVOID);
typedef ULONG (WINAPI* NXRMDRV_POST_GET_CLASSIFY_UI_RESPONSE)(HANDLE,PVOID,GET_CLASSIFY_UI_RESPONSE*);
typedef ULONG (WINAPI* NXRMDRV_POST_BLOCK_NOTIFICATION_RESPONSE)(HANDLE,PVOID);
typedef ULONG (WINAPI* NXRMDRV_POST_CHECK_PROTECT_MENU_RESPONSE)(HANDLE,PVOID,CHECK_PROTECT_MENU_RESPONSE*);
typedef ULONG (WINAPI* NXRMDRV_POST_GET_CONTEXT_MENU_REGEX_RESPONSE)(HANDLE,PVOID,QUERY_CTXMENUREGEX_RESPONSE*);
typedef ULONG (WINAPI* NXRMDRV_ENABLE_ANTITAMPERING)(BOOL);

static ULONG WINAPI NotificationCallback(ULONG Type, PVOID Msg, ULONG Length, PVOID MsgCtx, PVOID UserCtx);



class CDrvMan
{
public:
    CDrvMan();
    ~CDrvMan();

    VOID Load();
    VOID Unload();
    BOOL IsValid() const throw();

    HANDLE CreateManager(NXRMDRV_CALLBACK_NOTIFY CbNotify, LOGAPI_LOG CbDbgDump, LOGAPI_ACCEPT CbDbgDumpCheckLevel, PVOID Context);
    ULONG PostCheckObligationsResponse(HANDLE MgrHandle, PVOID Context, NXRM_OBLIGATION* Obs, ULONG ObsLength);
    ULONG PostOfficeGetCustomUIResponse(HANDLE MgrHandle, PVOID Context, OFFICE_GET_CUSTOMUI_RESPONSE* Response);
    ULONG QueryProcessInfo(HANDLE MgrHandle, ULONG ProcessId, NXRM_PROCESS_ENTRY* Info);
    ULONG IsRequestCanceled(HANDLE MgrHandle, PVOID Context);
    ULONG IsProcessAlive(HANDLE MgrHandle,  ULONG ProcessId);
    ULONG Start(HANDLE MgrHandle);
    ULONG Stop(HANDLE MgrHandle);
    ULONG PostSaveAsForecastResponse(HANDLE MgrHandle, PVOID Context);
    ULONG PostGetClassifyUIResponse(HANDLE MgrHandle, PVOID Context, GET_CLASSIFY_UI_RESPONSE* Response);
    ULONG PostBlockNotificationResponse(HANDLE MgrHandle, PVOID Context);
    ULONG PostCheckProtectMenuResponse(HANDLE MgrHandle, PVOID Context, CHECK_PROTECT_MENU_RESPONSE* Response);
    ULONG PostGetContextMenuRegexResponse(HANDLE MgrHandle, PVOID Context, QUERY_CTXMENUREGEX_RESPONSE* Response);
    ULONG CloseManager(HANDLE MgrHandle);
    ULONG EnableAntiTampering(BOOL Enable);
    ULONG IncreasePolicySerialNo(HANDLE MgrHandle);


private:
    HMODULE hMod;
    NXRMDRV_CREATE_MANAGER  FnCreateManager;
    NXRMDRV_POSTCHECK_OBLIGATIONS_RESPONSE FnPostCheckObligationsResponse;
    NXRMDRV_POST_OFFICE_GETCUSTOMUI_RESPONSE FnPostOfficeGetCustomUIResponse;
    NXRMDRV_QUERY_PROCESSINFO FnQueryProcessInfo;
    NXRMDRV_IS_REQUEST_CANCELED FnIsRequestCanceled;
    NXRMDRV_IS_PROCESS_ALIVE FnIsProcessAlive;
    NXRMDRV_START FnStart;
    NXRMDRV_STOP FnStop;
    NXRMDRV_INCREASE_POLICY_SERIALNO FnIncreasePolicySerialNo;
    NXRMDRV_POST_SAVEAS_FORECAST_RESPONSE FnPostSaveAsForecastResponse;
    NXRMDRV_POST_GET_CLASSIFY_UI_RESPONSE FnPostGetClassifyUIResponse;
    NXRMDRV_POST_BLOCK_NOTIFICATION_RESPONSE FnPostBlockNotificationResponse;
    NXRMDRV_POST_CHECK_PROTECT_MENU_RESPONSE FnPostCheckProtectMenuResponse;
    NXRMDRV_POST_GET_CONTEXT_MENU_REGEX_RESPONSE FnPostGetContextMenuRegexResponse;
    NXRMDRV_CLOSE_MANAGER FnCloseManager;
    NXRMDRV_ENABLE_ANTITAMPERING FnEnableAntiTampering;
};


CDrvMan drvMan;


//
//  class CDrvServ
//
CDrvServ::CDrvServ() : _drvman(NULL)
{
    nudf::util::InitializeListHead(&_list);
    ::InitializeCriticalSection(&_listlock);
    _hFont = CreateFontW(36, 0, 0, 0, FW_BOLD,
                         0, FALSE, FALSE,
                         ANSI_CHARSET,
                         OUT_DEFAULT_PRECIS,
                         CLIP_DEFAULT_PRECIS,
                         PROOF_QUALITY,
                         DEFAULT_PITCH | FF_DONTCARE,
                         L"Sitka Text");
}

CDrvServ::~CDrvServ()
{
    ::DeleteCriticalSection(&_listlock);
    if(NULL != _hFont) {
        ::DeleteObject(_hFont);
        _hFont = NULL;
    }
}

void CDrvServ::Start()
{
    DWORD dwResult = 0;
    BOOL bThreadStarted = FALSE;

    _drvman = drvMan.CreateManager(NotificationCallback, RmEngineLog, RmEngineLogAccept, this);
    if(NULL == _drvman) {
        throw WIN32ERROR();
    }

    try {

        // Start worker threads
        nudf::util::thread::CThreadExPool<CDrvWorkerThread>::Start(4, this);
        bThreadStarted = TRUE;

#ifdef _DEBUG
        // disable anti-tampering
        dwResult = drvMan.EnableAntiTampering(FALSE);
        if(0 != dwResult) {
            throw WIN32ERROR2(dwResult);
        }
#endif

        // enable nxrmdrv manager
        dwResult = drvMan.Start(_drvman);
        if(0 != dwResult) {
            throw WIN32ERROR2(dwResult);
        }
    }
    catch(const nudf::CException& e) {
        Stop();
        throw e;
    }
}

void CDrvServ::Stop() throw()
{
    if(_drvman != NULL) {
        drvMan.CloseManager(_drvman);
        _drvman = NULL;
    }

    nudf::util::thread::CThreadExPool<CDrvWorkerThread>::Stop();
    CleanRequestQueue();
}

PDRVREQUEST CDrvServ::PopRequest()
{
    PDRVREQUEST Request = NULL;

    ::EnterCriticalSection(&_listlock);
    if(!nudf::util::IsListEmpty(&_list)) {
        PLIST_ENTRY Entry = nudf::util::RemoveHeadList(&_list);
        if(NULL != Entry) {
            Request = CONTAINING_RECORD(Entry, DRVREQUEST, Link);
        }
    }
    ::LeaveCriticalSection(&_listlock);

    return Request;
}

VOID CDrvServ::PushRequest(PDRVREQUEST Request)
{
    ::EnterCriticalSection(&_listlock);
    nudf::util::InsertTailList(&_list, &Request->Link);
    ::LeaveCriticalSection(&_listlock);
}

VOID CDrvServ::CleanRequestQueue()
{
    ::EnterCriticalSection(&_listlock);
    while(!nudf::util::IsListEmpty(&_list)) {
        PLIST_ENTRY Entry = nudf::util::RemoveHeadList(&_list);
        PDRVREQUEST ReqItem = CONTAINING_RECORD(Entry, DRVREQUEST, Link);
        if(NULL != ReqItem->Request) {
            free(ReqItem->Request);
        }
        free(ReqItem);
    }
    ::LeaveCriticalSection(&_listlock);
}

bool CDrvServ::QueryProcessInfo(_In_ ULONG pid, _Out_ CDrvProcessInfo& info)
{
    NXRM_PROCESS_ENTRY entry = {0};

    if(0 != drvMan.QueryProcessInfo(_drvman, pid, &entry)) {
        return false;
    }

    info.SetImage(entry.process_path);
    info.SetPid(pid);
    info.SetParentPid((ULONG)(ULONG_PTR)entry.parent_id);
    info.SetSessionId(entry.session_id);
    info.SetPlatform(entry.platform);
    info.SetService(0 != entry.is_service);
    info.SetHooked(0 != entry.hooked);
    return true;
}

void CDrvServ::IncreasePolicySerianNo()
{
    (void)drvMan.IncreasePolicySerialNo(_drvman);
}

PDRVREQUEST CDrvServ::MakeGetCustomUIRequest(_In_ PVOID Msg, _In_ ULONG Length)
{
    PDRVREQUEST ReqItem = NULL;
    
    ReqItem = (PDRVREQUEST)malloc(sizeof(DRVREQUEST));
    if(NULL == ReqItem) {
        LOGERR(ERROR_OUTOFMEMORY, L"Fail to allocate DRVREQUEST object");
        return NULL;
    }
    memset(ReqItem, 0, sizeof(DRVREQUEST));
    
    ReqItem->Request = malloc(sizeof(OFFICE_GET_CUSTOMUI_REQUEST));
    if(NULL == ReqItem->Request) {
        free(ReqItem);
        ReqItem = NULL;
        LOGERR(ERROR_OUTOFMEMORY, L"Fail to allocate DRVREQUEST.Request object");
        return NULL;
    }
    
    ReqItem->Type = NXRMDRV_MSG_TYPE_GET_CUSTOMUI;
    memcpy(ReqItem->Request, Msg, min(sizeof(OFFICE_GET_CUSTOMUI_REQUEST), Length));
    return ReqItem;
}

PDRVREQUEST CDrvServ::MakeCheckObligationsRequest(_In_ PVOID Msg, _In_ ULONG Length)
{
    PDRVREQUEST ReqItem = NULL;
    
    ReqItem = (PDRVREQUEST)malloc(sizeof(DRVREQUEST));
    if(NULL == ReqItem) {
        LOGERR(ERROR_OUTOFMEMORY, L"Fail to allocate DRVREQUEST object");
        return NULL;
    }
    memset(ReqItem, 0, sizeof(DRVREQUEST));
    
    ReqItem->Request = malloc(sizeof(CHECK_OBLIGATION_REQUEST));
    if(NULL == ReqItem->Request) {
        free(ReqItem);
        ReqItem = NULL;
        LOGERR(ERROR_OUTOFMEMORY, L"Fail to allocate DRVREQUEST.Request object");
        return NULL;
    }
    
    ReqItem->Type = NXRMDRV_MSG_TYPE_CHECKOBLIGATION;
    memcpy(ReqItem->Request, Msg, min(sizeof(CHECK_OBLIGATION_REQUEST), Length));
    return ReqItem;
}

PDRVREQUEST CDrvServ::MakeSaveAsForecastRequest(_In_ PVOID Msg, _In_ ULONG Length)
{
    PDRVREQUEST ReqItem = NULL;
    
    ReqItem = (PDRVREQUEST)malloc(sizeof(DRVREQUEST));
    if(NULL == ReqItem) {
        LOGERR(ERROR_OUTOFMEMORY, L"Fail to allocate DRVREQUEST object");
        return NULL;
    }
    memset(ReqItem, 0, sizeof(DRVREQUEST));
    
    ReqItem->Request = malloc(sizeof(SAVEAS_FORECAST_REQUEST));
    if(NULL == ReqItem->Request) {
        free(ReqItem);
        ReqItem = NULL;
        LOGERR(ERROR_OUTOFMEMORY, L"Fail to allocate DRVREQUEST.Request object");
        return NULL;
    }
    
    ReqItem->Type = NXRMDRV_MSG_TYPE_SAVEAS_FORECAST;
    memcpy(ReqItem->Request, Msg, min(sizeof(SAVEAS_FORECAST_REQUEST), Length));
    return ReqItem;
}

PDRVREQUEST CDrvServ::MakeGetClassifyUIRequest(_In_ PVOID Msg, _In_ ULONG Length)
{
    PDRVREQUEST ReqItem = NULL;
    
    ReqItem = (PDRVREQUEST)malloc(sizeof(DRVREQUEST));
    if(NULL == ReqItem) {
        LOGERR(ERROR_OUTOFMEMORY, L"Fail to allocate DRVREQUEST object");
        return NULL;
    }
    memset(ReqItem, 0, sizeof(DRVREQUEST));
    
    ReqItem->Request = malloc(sizeof(GET_CLASSIFY_UI_REQUEST));
    if(NULL == ReqItem->Request) {
        free(ReqItem);
        ReqItem = NULL;
        LOGERR(ERROR_OUTOFMEMORY, L"Fail to allocate DRVREQUEST.Request object");
        return NULL;
    }
    
    ReqItem->Type = NXRMDRV_MSG_TYPE_GET_CLASSIFYUI;
    memcpy(ReqItem->Request, Msg, min(sizeof(GET_CLASSIFY_UI_REQUEST), Length));
    return ReqItem;
}

PDRVREQUEST CDrvServ::MakeNotificationRequest(_In_ PVOID Msg, _In_ ULONG Length)
{
    PDRVREQUEST ReqItem = NULL;
    
    ReqItem = (PDRVREQUEST)malloc(sizeof(DRVREQUEST));
    if(NULL == ReqItem) {
        LOGERR(ERROR_OUTOFMEMORY, L"Fail to allocate DRVREQUEST object");
        return NULL;
    }
    memset(ReqItem, 0, sizeof(DRVREQUEST));
    
    ReqItem->Request = malloc(sizeof(BLOCK_NOTIFICATION_REQUEST));
    if(NULL == ReqItem->Request) {
        free(ReqItem);
        ReqItem = NULL;
        LOGERR(ERROR_OUTOFMEMORY, L"Fail to allocate DRVREQUEST.Request object");
        return NULL;
    }
    
    ReqItem->Type = NXRMDRV_MSG_TYPE_BLOCK_NOTIFICATION;
    memcpy(ReqItem->Request, Msg, min(sizeof(BLOCK_NOTIFICATION_REQUEST), Length));
    return ReqItem;
}

PDRVREQUEST CDrvServ::MakeCheckProtectMenuRequest(_In_ PVOID Msg, _In_ ULONG Length)
{
    PDRVREQUEST ReqItem = NULL;
    
    ReqItem = (PDRVREQUEST)malloc(sizeof(DRVREQUEST));
    if(NULL == ReqItem) {
        LOGERR(ERROR_OUTOFMEMORY, L"Fail to allocate DRVREQUEST object");
        return NULL;
    }
    memset(ReqItem, 0, sizeof(DRVREQUEST));
    
    ReqItem->Request = malloc(sizeof(CHECK_PROTECT_MENU_REQUEST));
    if(NULL == ReqItem->Request) {
        free(ReqItem);
        ReqItem = NULL;
        LOGERR(ERROR_OUTOFMEMORY, L"Fail to allocate DRVREQUEST.Request object");
        return NULL;
    }
    
    ReqItem->Type = NXRMDRV_MSG_TYPE_CHECK_PROTECT_MENU;
    memcpy(ReqItem->Request, Msg, min(sizeof(CHECK_PROTECT_MENU_REQUEST), Length));
    return ReqItem;
}

PDRVREQUEST CDrvServ::MakeGetContextMenuRegexRequest(_In_ PVOID Msg, _In_ ULONG Length)
{
    PDRVREQUEST ReqItem = NULL;
    
    ReqItem = (PDRVREQUEST)malloc(sizeof(DRVREQUEST));
    if(NULL == ReqItem) {
        LOGERR(ERROR_OUTOFMEMORY, L"Fail to allocate DRVREQUEST object");
        return NULL;
    }
    memset(ReqItem, 0, sizeof(DRVREQUEST));
    
    ReqItem->Request = malloc(sizeof(QUERY_CTXMENUREGEX_REQUEST));
    if(NULL == ReqItem->Request) {
        free(ReqItem);
        ReqItem = NULL;
        LOGERR(ERROR_OUTOFMEMORY, L"Fail to allocate DRVREQUEST.Request object");
        return NULL;
    }
    
    ReqItem->Type = NXRMDRV_MSG_TYPE_GET_CTXMENUREGEX;
    memcpy(ReqItem->Request, Msg, min(sizeof(QUERY_CTXMENUREGEX_REQUEST), Length));
    return ReqItem;
}

VOID CDrvServ::OnGetCustomUI(_In_ PDRVREQUEST ReqItem)
{
	BOOL bRet = TRUE;
	UINT uRet = 0;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD BytesWriten = 0;
    OFFICE_GET_CUSTOMUI_REQUEST*  req = (OFFICE_GET_CUSTOMUI_REQUEST*)ReqItem->Request;
	OFFICE_GET_CUSTOMUI_RESPONSE resp = {0};
	NXRM_PROCESS_ENTRY ProcessInfo = {0};
	ULONG ulRet = 0;


    if(0 != drvMan.IsRequestCanceled(_drvman, ReqItem->Context)) {
        LOGWARN(0, L"Request has been canceled");
        return;
    }

    ulRet = drvMan.QueryProcessInfo(_drvman, req->ProcessId, &ProcessInfo);
    if (ulRet != ERROR_SUCCESS) {
        LOGERR(ulRet, L"Fail to get process information");
    }

    const wchar_t* process_name = wcsrchr(ProcessInfo.process_path, L'\\');
    if (NULL == process_name) {
        process_name = ProcessInfo.process_path;
    }
    else {
        process_name++;
    }

    std::string xml = OFFICE_LAYOUT_XML;

    if (0 == _wcsnicmp(req->OfficeVersion, L"14", 2)) {
        if (0 == _wcsicmp(process_name, L"WINWORD.EXE")) {
            xml = WORD_LAYOUT_XML_14;
        }
        else if (0 == _wcsicmp(process_name, L"EXCEL.EXE")) {
            xml = EXCEL_LAYOUT_XML_14;
        }
        else if (0 == _wcsicmp(process_name, L"POWERPNT.EXE")) {
            xml = POWERPNT_LAYOUT_XML_14;
        }
        else {
            xml = OFFICE_LAYOUT_XML;
        }
    }
    else if (0 == _wcsnicmp(req->OfficeVersion, L"15", 2)) {
        if (0 == _wcsicmp(process_name, L"WINWORD.EXE")) {
            xml = WORD_LAYOUT_XML_15;
        }
        else if (0 == _wcsicmp(process_name, L"EXCEL.EXE")) {
            xml = EXCEL_LAYOUT_XML_15;
        }
        else if (0 == _wcsicmp(process_name, L"POWERPNT.EXE")) {
            xml = POWERPNT_LAYOUT_XML_15;
        }
        else {
            xml = OFFICE_LAYOUT_XML;
        }
    }
    else {
        if (0 == _wcsicmp(process_name, L"WINWORD.EXE")) {
            xml = WORD_LAYOUT_XML_15;
        }
        else if (0 == _wcsicmp(process_name, L"EXCEL.EXE")) {
            xml = EXCEL_LAYOUT_XML_15;
        }
        else if (0 == _wcsicmp(process_name, L"POWERPNT.EXE")) {
            xml = POWERPNT_LAYOUT_XML_15;
        }
        else {
            xml = OFFICE_LAYOUT_XML;
        }
    }

    uRet = GetTempFileNameW(req->TempPath, L"nxrm", 0, resp.CustomUIFileName);
    if(!uRet) {
        LOGERR(GetLastError(), L"Fail to get temp file name");
        return;
    }
    
    hFile = CreateFileW(resp.CustomUIFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        LOGERR(GetLastError(), L"Fail to create temp Custom UI XML file");
        return;
    }
    
    if (!WriteFile(hFile, xml.c_str(), (DWORD)xml.length(), &BytesWriten, NULL)) {
        LOGERR(GetLastError(), L"Fail to write data to temp Custom UI XML file");
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
        return;
    }		

    CloseHandle(hFile);
    hFile = INVALID_HANDLE_VALUE;
    ulRet = drvMan.PostOfficeGetCustomUIResponse(_drvman, ReqItem->Context, &resp);
    if(ERROR_SUCCESS != ulRet) {
        LOGERR(GetLastError(), L"Fail to post office get custom UI response");
    }
}

VOID CDrvServ::OnCheckObligations(_In_ PDRVREQUEST ReqItem)
{
    PCHECK_OBLIGATION_REQUEST  req = (PCHECK_OBLIGATION_REQUEST)ReqItem->Request;
    ULONG ulResult = 0;
    DWORD dwSessionId = -1;
    nudf::util::CObligations vobs;
    
    if(0 != drvMan.IsRequestCanceled(_drvman, ReqItem->Context)) {
        LOGWARN(0, L"Request has been canceled");
        return;
    }
       
    //
    //  Unknown Session ?
    //
    if(!ProcessIdToSessionId(req->ProcessId, &dwSessionId) || ((ULONG)-1) == dwSessionId) {
        drvMan.PostCheckObligationsResponse(_drvman, ReqItem->Context, NULL, 0);
        return;
    }
    
    // try to find obligation
    LOGDBG(L"CheckObligation: EvaluationId=%d", req->EvaluationId);
    if(!gpEngine->GetPolicyServ()->FindObs(dwSessionId, req->EvaluationId, vobs) || vobs.IsEmpty()) {
        drvMan.PostCheckObligationsResponse(_drvman, ReqItem->Context, NULL, 0);
        LOGDBG(L"  - No obligation");
        return;
    }

    
    std::vector<UCHAR>  obs_block;
    std::vector<std::vector<UCHAR>> obs_array;
    size_t obs_block_size = 0;

    for(nudf::util::OBS::const_iterator it=vobs.GetObligations().begin(); it!=vobs.GetObligations().end(); ++it) {

        std::vector<UCHAR> ob_block;
        ULONG              ob_block_size = 0;
        NXRM_OBLIGATION*   ob = NULL;
        switch((*it).first)
        {
        case OB_ID_CLASSIFY:
            ob_block_size = sizeof(NXRM_OBLIGATION);
            ob_block.resize(ob_block_size, 0);
            ob = (NXRM_OBLIGATION*)(&ob_block[0]);
            ob->NextOffset = ob_block_size;
            ob->Id = OB_ID_CLASSIFY;
            obs_array.push_back(ob_block);
            obs_block_size += ob_block.size();
            LOGDBG(L"  - Classify");
            break;

        case OB_ID_OVERLAY:
            {
                LOGDBG(L"  - Overlay");
                CWmTextInfo ti;
                nudf::util::OBPARAMS::const_iterator param;

                // Get all the parameters
                param = (*it).second.GetParams().find(L"text");
                if(param != (*it).second.GetParams().end() && (*param).second.length()!=0) {
                    ti.SetText((*param).second);
                }
                param = (*it).second.GetParams().find(L"transparency");
                if(param != (*it).second.GetParams().end() && (*param).second.length()!=0) {
                    int nTrans = 10;
                    if(nudf::string::ToInt<wchar_t>((*param).second, &nTrans)) {
                        ti.SetTransparency(nTrans);
                    }
                }
                param = (*it).second.GetParams().find(L"fontname");
                if(param != (*it).second.GetParams().end() && (*param).second.length()!=0) {
                    ti.SetFontName((*param).second);
                }
                param = (*it).second.GetParams().find(L"fontsize");
                if(param != (*it).second.GetParams().end() && (*param).second.length()!=0) {
                    int nSize = 0;
                    if(nudf::string::ToInt<wchar_t>((*param).second, &nSize)) {
                        ti.SetFontSize(abs(nSize));
                    }
                }
                param = (*it).second.GetParams().find(L"textcolor");
                if(param != (*it).second.GetParams().end() && (*param).second.length()!=0) {
                    ti.SetTextColor((*param).second);
                }
                param = (*it).second.GetParams().find(L"rotation");
                if(param != (*it).second.GetParams().end() && (*param).second.length()!=0) {
                    ti.SetRotationAngle((*param).second);
                }

                // Generate Watermark
                std::wstring wsImage = GenerateWaterMarkText(req->ProcessId, req->FileName, req->TempPath, ti);
                if(!wsImage.empty()) {

                    LOGDBG(L"    WaterMark: %s", wsImage.c_str());
                    std::wstring wsParamImage = OB_OVERLAY_PARAM_IMAGE;
                    wsParamImage += L"=";
                    wsParamImage += wsImage;
                    std::wstring wsParamTrans = OB_OVERLAY_PARAM_TRANSPARENCY;
                    wsParamTrans += L"=";
                    wsParamTrans += nudf::string::FromInt<wchar_t>(ti.GetTransparency());

                    ob_block_size = (ULONG)(sizeof(NXRM_OBLIGATION) + (sizeof(wchar_t) * (wsParamImage.length() + 1 + wsParamTrans.length() + 1)));
                    ob_block.resize(ob_block_size, 0);
                    ob = (NXRM_OBLIGATION*)(&ob_block[0]);
                    ob->NextOffset = ob_block_size;
                    ob->Id = OB_ID_OVERLAY;
                    WCHAR* pos = ob->Params;
                    memcpy(pos, wsParamImage.c_str(), sizeof(wchar_t)*wsParamImage.length());
                    pos += (ULONG)wsParamImage.length();
                    *pos = L'\0'; ++pos;
                    memcpy(pos, wsParamTrans.c_str(), sizeof(wchar_t)*wsParamTrans.length());
                    pos += (ULONG)wsParamTrans.length();
                    *pos = L'\0'; ++pos;
                    *pos = L'\0'; ++pos;

                    obs_array.push_back(ob_block);
                    obs_block_size += ob_block.size();
                }
            }
            break;
        default:
            break;
        }
    }
    
    if(0 == obs_block_size) {
        drvMan.PostCheckObligationsResponse(_drvman, ReqItem->Context, NULL, 0);
        return;
    }

    // Make sure the last item's NextOffset == 0
    ((NXRM_OBLIGATION*)(&(obs_array[obs_array.size()-1][0])))->NextOffset = 0;
    // Fill obs block
    obs_block.resize(obs_block_size, 0);
    UCHAR* p = &obs_block[0];
    LOGDBG(L"Post check obligations response:");
    for(std::vector<std::vector<UCHAR>>::const_iterator it=obs_array.begin(); it!=obs_array.end(); ++it) {
        NXRM_OBLIGATION* pOb = (NXRM_OBLIGATION*)(&(*it)[0]);
        if(OB_ID_OVERLAY == pOb->Id) {
            LOGDBG(L"  > Overlay: %s", pOb->Params);
        }
        else {
            LOGDBG(L"  > Classify");
        }
        memcpy(p, &((*it)[0]), (*it).size());
        p += (*it).size();
    }

    assert(0 != obs_block.size());

    ulResult = drvMan.PostCheckObligationsResponse(_drvman, ReqItem->Context, (NXRM_OBLIGATION*)(&obs_block[0]), (ULONG)obs_block.size());
    if(ERROR_SUCCESS != ulResult) {
        LOGWARN(ulResult, L"Fail to post check obligations response");
    }
}

VOID CDrvServ::OnSaveAsForecast(_In_ PDRVREQUEST ReqItem)
{
    PSAVEAS_FORECAST_REQUEST  req = (PSAVEAS_FORECAST_REQUEST)ReqItem->Request;
    ULONG ulResult = 0;
    
    if(0 != drvMan.IsRequestCanceled(_drvman, ReqItem->Context)) {
        LOGWARN(0, L"Request has been canceled");
        return;
    }

    gpEngine->GetFltServ()->SetSaveAsForecast(req->ProcessId, req->SourceFileName, req->SaveAsFileName);
    LOGDBG(L"OnSaveAsForecast: Pid=%d, Source=%s, Target=%s", req->ProcessId, req->SourceFileName, req->SaveAsFileName);

    ulResult = drvMan.PostSaveAsForecastResponse(_drvman, ReqItem->Context);
    if(ERROR_SUCCESS != ulResult) {
        LOGWARN(ulResult, L"Fail to post check obligations response");
    }
}

VOID CDrvServ::OnGetClassifyUI(_In_ PDRVREQUEST ReqItem)
{
    PGET_CLASSIFY_UI_REQUEST  req = (PGET_CLASSIFY_UI_REQUEST)ReqItem->Request;
    GET_CLASSIFY_UI_RESPONSE  response = {0};
    NXRM_PROCESS_ENTRY        procinfo = {0};
    ULONG ulResult = 0;
    
    if(0 != drvMan.IsRequestCanceled(_drvman, ReqItem->Context)) {
        LOGWARN(0, L"Request has been canceled");
        return;
    }

    // Set Response
    memset(&response, 0, sizeof(response));
    GetTempFileNameW(req->TempPath, L"CS", 0, response.ClassifyUIFileName);
    CopyFileW(_nxPaths.GetClassifyConf().c_str(), response.ClassifyUIFileName, FALSE);
    
    std::wstring wsGroup;
    if(gpEngine->GetPolicyServ()->GetClassifyProfileGroup(req->SessionId, wsGroup) && !wsGroup.empty()) {
        wcsncpy_s(response.GroupName, MAX_PATH, wsGroup.c_str(), _TRUNCATE);
    }

    ulResult = drvMan.PostGetClassifyUIResponse(_drvman, ReqItem->Context, &response);
    if(ERROR_SUCCESS != ulResult) {
        LOGWARN(ulResult, L"Fail to post check obligations response");
    }
}

VOID CDrvServ::OnNotification(_In_ PDRVREQUEST ReqItem)
{
    const BLOCK_NOTIFICATION_REQUEST* req = (const BLOCK_NOTIFICATION_REQUEST*)ReqItem->Request;
    ULONG ulResult = 0;
    
    if(0 != drvMan.IsRequestCanceled(_drvman, ReqItem->Context)) {
        LOGWARN(0, L"Request has been canceled");
        return;
    }

    std::wstring wsTitle = nudf::util::res::LoadMessage(gpEngine->GetResModule(), IDS_NOTIFY_OPERATION_DENIED_TITLE, 256, LANG_NEUTRAL, L"Operation Denied");
    std::wstring wsOperation;
    std::wstring wsFile;
    std::wstring wsInfo;

    const WCHAR* pwzFileName = wcsrchr(req->FileName, L'\\');
    if(NULL == pwzFileName) pwzFileName = req->FileName;
    else pwzFileName++;
    wsFile = pwzFileName;
    
    // Send Notification
    switch(req->Type)
    {
    case NxrmdrvSaveFileBlocked:
        wsOperation = nudf::util::res::LoadMessage(gpEngine->GetResModule(), IDS_NOTIFY_OPERATION_SAVE, 64, LANG_NEUTRAL, L"save");
        break;
    case NXrmdrvPrintingBlocked:
        wsOperation = nudf::util::res::LoadMessage(gpEngine->GetResModule(), IDS_NOTIFY_OPERATION_PRINT, 64, LANG_NEUTRAL, L"print");
        break;
    case NxrmdrvEmbeddedOleObjBlocked:
        wsOperation = nudf::util::res::LoadMessage(gpEngine->GetResModule(), IDS_NOTIFY_OPERATION_INSERT, 64, LANG_NEUTRAL, L"insert");
        break;
    case NxrmdrvSendMailBlocked:
        wsOperation = nudf::util::res::LoadMessage(gpEngine->GetResModule(), IDS_NOTIFY_OPERATION_EMAIL, 64, LANG_NEUTRAL, L"email");
        break;
    case NxrmdrvExportSlidesBlocked:
        wsOperation = nudf::util::res::LoadMessage(gpEngine->GetResModule(), IDS_NOTIFY_OPERATION_EXPORT, 64, LANG_NEUTRAL, L"export");
        break;
    default:
        break;
    }

    // Post
    drvMan.PostBlockNotificationResponse(_drvman, ReqItem->Context);

    // Send notify
    if(!wsOperation.empty()) {
        wsInfo = nudf::util::res::LoadMessageEx(gpEngine->GetResModule(), IDS_NOTIFY_OPERATION_DENIED, 1024, LANG_NEUTRAL, L"You don't have permission to %s this file (%s)", wsOperation.c_str(), wsFile.c_str());
        RmNotify(req->SessionId, wsTitle.c_str(), wsInfo.c_str());
    }
}

VOID CDrvServ::OnCheckProtectMenu(_In_ PDRVREQUEST ReqItem)
{
    const CHECK_PROTECT_MENU_REQUEST* req = (const CHECK_PROTECT_MENU_REQUEST*)ReqItem->Request;
    CHECK_PROTECT_MENU_RESPONSE       response = {0};
    ULONG ulResult = 0;
    
    if(0 != drvMan.IsRequestCanceled(_drvman, ReqItem->Context)) {
        LOGWARN(0, L"Request has been canceled");
        return;
    }

    std::wstring wsGroup;
    if(gpEngine->GetPolicyServ()->GetClassifyProfileGroup(req->SessionId, wsGroup)) {
        response.EnableProtectMenu = wsGroup.empty() ? 0 : 1;
    }

    drvMan.PostCheckProtectMenuResponse(_drvman, ReqItem->Context, &response);
}

VOID CDrvServ::OnGetContextMenuRegex(_In_ PDRVREQUEST ReqItem)
{
    const QUERY_CTXMENUREGEX_REQUEST* req = (const QUERY_CTXMENUREGEX_REQUEST*)ReqItem->Request;
    QUERY_CTXMENUREGEX_RESPONSE       response = {0};
    ULONG ulResult = 0;
    
    if(0 != drvMan.IsRequestCanceled(_drvman, ReqItem->Context)) {
        LOGWARN(0, L"Request has been canceled");
        return;
    }

    //
    //  Currently we just simply return a regex string which matches anything
    //
    std::wstring wsRegex(L".*");    // Anything

    wcsncpy_s(response.CtxMenuRegEx, 1024, wsRegex.c_str(), _TRUNCATE);

    // Post
    drvMan.PostGetContextMenuRegexResponse(_drvman, ReqItem->Context, &response);
}

std::wstring CDrvServ::GenerateWaterMarkText(_In_ ULONG pid, _In_ LPCWSTR file, _In_ LPCWSTR wzTempFolder, _In_ CWmTextInfo& ti)
{
    std::wstring wsText;
    std::wstring wsAccount;
    std::wstring wsFile;
    std::wstring wsDate;
    std::wstring wsTime;
    std::wstring wsZone;
    DWORD dwSessionId = -1;


    UNREFERENCED_PARAMETER(file);

    if(NULL != file && L'\0'!=file[0]) {
        const wchar_t* filename = wcsrchr(file, L'\\');
        wsFile = (NULL==filename) ? file : (filename+1);
    }

    if(!ProcessIdToSessionId(pid, &dwSessionId)) {
        return L"";
    }


    std::shared_ptr<CActiveSession> pActiveSession = gpEngine->GetSessionServ()->FindSession(dwSessionId);
    nudf::win::CUser u;
    if(NULL==pActiveSession || !pActiveSession->GetUser(u)) {
        return L"";
    }
    wsAccount = u.GetAccountName();

    SYSTEMTIME st = {0};
    GetLocalTime(&st);
    nudf::time::CTimeZone zone;
    zone.BiasToString(wsZone);
    swprintf_s(nudf::string::tempstr<wchar_t>(wsDate, 64), 64, L"%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
    swprintf_s(nudf::string::tempstr<wchar_t>(wsTime, 64), 64, L"%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
    wsTime += wsZone;

    wsText = ti.GetText();
    if(wsText.empty()) {
        wsText = wsAccount;
        wsText += L"\n";
        wsText += wsDate;
        wsText += L" ";
        wsText += wsTime;
        wsText += L"\n";
        wsText += wsFile;
    }
    else {        
        wsText = nudf::util::regex::ReplaceAll<wchar_t>(wsText, L"\\$\\(User\\)", wsAccount, true);
        wsText = nudf::util::regex::ReplaceAll<wchar_t>(wsText, L"\\$\\(Date\\)", wsDate, true);
        wsText = nudf::util::regex::ReplaceAll<wchar_t>(wsText, L"\\$\\(Time\\)", wsTime, true);
        wsText = nudf::util::regex::ReplaceAll<wchar_t>(wsText, L"\\$\\(Document\\)", wsFile, true);
    }
    ti.SetText(wsText);

    LOGDBG(L"    Text: %s", wsText.c_str());
    return GenerateWaterMarkImage(ti, wzTempFolder);
}

std::wstring CDrvServ::GenerateWaterMarkImage(_In_ const CWmTextInfo& ti, _In_ const std::wstring& folder)
{
    std::wstring wsImage;
    nudf::image::CTextBitmap bmp;

    WCHAR wzImageName[MAX_PATH] = {0};
    GetTempFileNameW(folder.c_str(), L"WM", 0, wzImageName);

    if(!bmp.Create(ti.GetText().c_str(), ti.GetFontName().c_str(), ti.GetFontSize(), RGB(255,255,255), ti.GetFontColor())) {
        return L"";
    }

    if(ti.GetRotationAngle() != 0) {
        bmp.Rotate(NULL, ti.GetRotationAngle());
    }

    if(!bmp.ToFile(wzImageName)) {
        return L"";
    }

    return wzImageName;
}

//
//  class CDrvWorkerThread
//
CDrvWorkerThread::CDrvWorkerThread()
{
}

CDrvWorkerThread::~CDrvWorkerThread()
{
}

DWORD CDrvWorkerThread::OnStart(_In_opt_ PVOID Context)
{
    return 0L;
}

DWORD CDrvWorkerThread::OnRunning(_In_opt_ PVOID Context)
{
    CDrvServ* serv = (CDrvServ*)Context;
    PDRVREQUEST ReqItem = NULL;

    assert(NULL != Context);

    ReqItem = serv->PopRequest();
    if(NULL == ReqItem) {
        // No more request, resetthe signal
        serv->ResetJobEvent();
        return 0;
    }

    switch(ReqItem->Type)
    {
    case NXRMDRV_MSG_TYPE_GET_CUSTOMUI:
        serv->OnGetCustomUI(ReqItem);
        break;
    case NXRMDRV_MSG_TYPE_CHECKOBLIGATION:
        serv->OnCheckObligations(ReqItem);
        break;
    case NXRMDRV_MSG_TYPE_SAVEAS_FORECAST:
        serv->OnSaveAsForecast(ReqItem);
        break;
    case NXRMDRV_MSG_TYPE_GET_CLASSIFYUI:
        serv->OnGetClassifyUI(ReqItem);
        break;
    case NXRMDRV_MSG_TYPE_BLOCK_NOTIFICATION:
        serv->OnNotification(ReqItem);
        break;
    case NXRMDRV_MSG_TYPE_CHECK_PROTECT_MENU:
        serv->OnCheckProtectMenu(ReqItem);
        break;
    case NXRMDRV_MSG_TYPE_GET_CTXMENUREGEX:
        serv->OnGetContextMenuRegex(ReqItem);
        break;
    default:
        break;
    }

    if(NULL != ReqItem) {
        if(NULL != ReqItem->Request) {
            free(ReqItem->Request);
        }
        free(ReqItem);
        ReqItem = NULL;
    }

    return 0;
}

//
//
//
ULONG WINAPI NotificationCallback(ULONG Type, PVOID Msg, ULONG Length, PVOID MsgCtx, PVOID UserCtx)
{
    CDrvServ* serv = (CDrvServ*)UserCtx;
	PDRVREQUEST ReqItem = NULL;

    switch (Type)
    {
    case NXRMDRV_MSG_TYPE_GET_CUSTOMUI:
        ReqItem = serv->MakeGetCustomUIRequest(Msg, Length);
        break;

    case NXRMDRV_MSG_TYPE_CHECKOBLIGATION:
        ReqItem = serv->MakeCheckObligationsRequest(Msg, Length);
        break;

    case NXRMDRV_MSG_TYPE_SAVEAS_FORECAST:
        ReqItem = serv->MakeSaveAsForecastRequest(Msg, Length);
        break;

    case NXRMDRV_MSG_TYPE_GET_CLASSIFYUI:
        ReqItem = serv->MakeGetClassifyUIRequest(Msg, Length);
        break;

    case NXRMDRV_MSG_TYPE_BLOCK_NOTIFICATION:
        ReqItem = serv->MakeNotificationRequest(Msg, Length);
        break;

    case NXRMDRV_MSG_TYPE_CHECK_PROTECT_MENU:
        ReqItem = serv->MakeCheckProtectMenuRequest(Msg, Length);
        break;

    case NXRMDRV_MSG_TYPE_GET_CTXMENUREGEX:
        ReqItem = serv->MakeGetContextMenuRegexRequest(Msg, Length);
        break;

    default:
        break;
    }

    if(ReqItem == NULL) {
        return 0;
    }
    
    ReqItem->Context = MsgCtx;
    serv->PushRequest(ReqItem);
    serv->SetJobEvent();
	return 0;
}




//
//  class CDrvMan
//
CDrvMan::CDrvMan() : hMod(NULL),
    FnCreateManager(NULL),
    FnPostCheckObligationsResponse(NULL),
    FnPostOfficeGetCustomUIResponse(NULL),
    FnQueryProcessInfo(NULL),
    FnIsRequestCanceled(NULL),
    FnIsProcessAlive(NULL),
    FnStart(NULL),
    FnStop(NULL),
    FnIncreasePolicySerialNo(NULL),
    FnPostSaveAsForecastResponse(NULL),
    FnPostGetClassifyUIResponse(NULL),
    FnPostBlockNotificationResponse(NULL),
    FnPostCheckProtectMenuResponse(NULL),
    FnPostGetContextMenuRegexResponse(NULL),
    FnCloseManager(NULL),
    FnEnableAntiTampering(NULL)
{
    Load();
}

CDrvMan::~CDrvMan()
{
    Unload();
}

VOID CDrvMan::Load()
{
    nudf::win::CModulePath mod(NULL);
    std::wstring wsDll = mod.GetParentDir();
    if(!wsDll.empty()) {
        wsDll += L"\\";
    }
    wsDll += L"nxrmdrvman.dll";

    hMod = ::LoadLibraryW(wsDll.c_str());
    if(NULL != hMod) {
        // Refer to nxrmdrvman.def to get ordinal
        FnCreateManager = (NXRMDRV_CREATE_MANAGER)GetProcAddress(hMod, MAKEINTRESOURCEA(101));
        FnStart = (NXRMDRV_START)GetProcAddress(hMod, MAKEINTRESOURCEA(102));
        FnStop = (NXRMDRV_STOP)GetProcAddress(hMod, MAKEINTRESOURCEA(103));
        FnPostCheckObligationsResponse = (NXRMDRV_POSTCHECK_OBLIGATIONS_RESPONSE)GetProcAddress(hMod, MAKEINTRESOURCEA(104));
        FnQueryProcessInfo = (NXRMDRV_QUERY_PROCESSINFO)GetProcAddress(hMod, MAKEINTRESOURCEA(105));
        FnIsRequestCanceled = (NXRMDRV_IS_REQUEST_CANCELED)GetProcAddress(hMod, MAKEINTRESOURCEA(106));
        FnIsProcessAlive = (NXRMDRV_IS_PROCESS_ALIVE)GetProcAddress(hMod, MAKEINTRESOURCEA(107));
        FnPostOfficeGetCustomUIResponse = (NXRMDRV_POST_OFFICE_GETCUSTOMUI_RESPONSE)GetProcAddress(hMod, MAKEINTRESOURCEA(108));
        FnIncreasePolicySerialNo = (NXRMDRV_INCREASE_POLICY_SERIALNO)GetProcAddress(hMod, MAKEINTRESOURCEA(109));
        FnPostSaveAsForecastResponse = (NXRMDRV_POST_SAVEAS_FORECAST_RESPONSE)GetProcAddress(hMod, MAKEINTRESOURCEA(110));
        FnPostGetClassifyUIResponse = (NXRMDRV_POST_GET_CLASSIFY_UI_RESPONSE)GetProcAddress(hMod, MAKEINTRESOURCEA(111));
        FnPostBlockNotificationResponse = (NXRMDRV_POST_BLOCK_NOTIFICATION_RESPONSE)GetProcAddress(hMod, MAKEINTRESOURCEA(112));
        FnPostCheckProtectMenuResponse = (NXRMDRV_POST_CHECK_PROTECT_MENU_RESPONSE)GetProcAddress(hMod, MAKEINTRESOURCEA(113));
        FnPostGetContextMenuRegexResponse = (NXRMDRV_POST_GET_CONTEXT_MENU_REGEX_RESPONSE)GetProcAddress(hMod, MAKEINTRESOURCEA(114));
        FnCloseManager = (NXRMDRV_CLOSE_MANAGER)GetProcAddress(hMod, MAKEINTRESOURCEA(199));
        FnEnableAntiTampering = (NXRMDRV_ENABLE_ANTITAMPERING)GetProcAddress(hMod, MAKEINTRESOURCEA(200));
    }
}

VOID CDrvMan::Unload()
{
    FnCreateManager = NULL;
    FnPostCheckObligationsResponse = NULL;
    FnPostOfficeGetCustomUIResponse = NULL;
    FnQueryProcessInfo = NULL;
    FnIsRequestCanceled = NULL;
    FnIsProcessAlive = NULL;
    FnStart = NULL;
    FnStop = NULL;
    FnIncreasePolicySerialNo = NULL;
    FnPostSaveAsForecastResponse = NULL;
    FnPostGetClassifyUIResponse = NULL;
    FnPostBlockNotificationResponse = NULL;
    FnPostCheckProtectMenuResponse = NULL;
    FnPostGetContextMenuRegexResponse = NULL;
    FnCloseManager = NULL;
    FnEnableAntiTampering = NULL;
    if(NULL != hMod) {
        FreeLibrary(hMod);
    }
}

BOOL CDrvMan::IsValid() const throw()
{
    return (FnCreateManager != NULL &&
            FnPostCheckObligationsResponse != NULL &&
            FnPostOfficeGetCustomUIResponse != NULL &&
            FnQueryProcessInfo != NULL &&
            FnIsRequestCanceled != NULL &&
            FnIsProcessAlive != NULL &&
            FnStart != NULL &&
            FnStop != NULL &&
            FnIncreasePolicySerialNo != NULL &&
            FnPostSaveAsForecastResponse != NULL &&
            FnPostGetClassifyUIResponse != NULL &&
            FnPostBlockNotificationResponse != NULL &&
            FnPostCheckProtectMenuResponse != NULL &&
            FnPostGetContextMenuRegexResponse != NULL &&
            FnCloseManager != NULL &&
            FnEnableAntiTampering != NULL);
}

HANDLE CDrvMan::CreateManager(NXRMDRV_CALLBACK_NOTIFY CbNotify, LOGAPI_LOG CbDbgDump, LOGAPI_ACCEPT CbDbgDumpCheckLevel, PVOID Context)
{
    if(NULL == FnCreateManager) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return NULL;
    }
    return FnCreateManager(CbNotify, CbDbgDump, CbDbgDumpCheckLevel, Context);
}

ULONG CDrvMan::PostCheckObligationsResponse(HANDLE MgrHandle, PVOID Context, NXRM_OBLIGATION* Obs, ULONG ObsLength)
{
    if(NULL == FnPostCheckObligationsResponse) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return ERROR_INVALID_FUNCTION;
    }
    return FnPostCheckObligationsResponse(MgrHandle, Context, Obs, ObsLength);
}

ULONG CDrvMan::PostOfficeGetCustomUIResponse(HANDLE MgrHandle, PVOID Context, OFFICE_GET_CUSTOMUI_RESPONSE* Response)
{
    if(NULL == FnPostOfficeGetCustomUIResponse) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return ERROR_INVALID_FUNCTION;
    }
    return FnPostOfficeGetCustomUIResponse(MgrHandle, Context, Response);
}

ULONG CDrvMan::QueryProcessInfo(HANDLE MgrHandle, ULONG ProcessId, NXRM_PROCESS_ENTRY* Info)
{
    if(NULL == FnQueryProcessInfo) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return ERROR_INVALID_FUNCTION;
    }
    return FnQueryProcessInfo(MgrHandle, ProcessId, Info);
}

ULONG CDrvMan::IsRequestCanceled(HANDLE MgrHandle, PVOID Context)
{
    if(NULL == FnIsRequestCanceled) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return ERROR_INVALID_FUNCTION;
    }
    return FnIsRequestCanceled(MgrHandle, Context);
}

ULONG CDrvMan::IsProcessAlive(HANDLE MgrHandle,  ULONG ProcessId)
{
    if(NULL == FnIsProcessAlive) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return ERROR_INVALID_FUNCTION;
    }
    return FnIsProcessAlive(MgrHandle, ProcessId);
}

ULONG CDrvMan::Start(HANDLE MgrHandle)
{
    if(NULL == FnStart) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return ERROR_INVALID_FUNCTION;
    }
    return FnStart(MgrHandle);
}

ULONG CDrvMan::Stop(HANDLE MgrHandle)
{
    if(NULL == FnStop) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return ERROR_INVALID_FUNCTION;
    }
    return FnStop(MgrHandle);
}

ULONG CDrvMan::IncreasePolicySerialNo(HANDLE MgrHandle)
{
    if(NULL == FnIncreasePolicySerialNo) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return ERROR_INVALID_FUNCTION;
    }
    return FnIncreasePolicySerialNo(MgrHandle);
}

ULONG CDrvMan::PostSaveAsForecastResponse(HANDLE MgrHandle, PVOID Context)
{
    if(NULL == FnPostSaveAsForecastResponse) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return ERROR_INVALID_FUNCTION;
    }
    return FnPostSaveAsForecastResponse(MgrHandle, Context);
}

ULONG CDrvMan::PostGetClassifyUIResponse(HANDLE MgrHandle, PVOID Context, GET_CLASSIFY_UI_RESPONSE* Response)
{
    if(NULL == FnPostGetClassifyUIResponse) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return ERROR_INVALID_FUNCTION;
    }
    return FnPostGetClassifyUIResponse(MgrHandle, Context, Response);
}

ULONG CDrvMan::PostBlockNotificationResponse(HANDLE MgrHandle, PVOID Context)
{
    if(NULL == FnPostBlockNotificationResponse) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return ERROR_INVALID_FUNCTION;
    }
    return FnPostBlockNotificationResponse(MgrHandle, Context);
}

ULONG CDrvMan::PostCheckProtectMenuResponse(HANDLE MgrHandle, PVOID Context, CHECK_PROTECT_MENU_RESPONSE* Response)
{
    if(NULL == FnPostBlockNotificationResponse) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return ERROR_INVALID_FUNCTION;
    }
    return FnPostCheckProtectMenuResponse(MgrHandle, Context, Response);
}

ULONG CDrvMan::PostGetContextMenuRegexResponse(HANDLE MgrHandle, PVOID Context, QUERY_CTXMENUREGEX_RESPONSE* Response)
{
    if(NULL == FnPostGetContextMenuRegexResponse) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return ERROR_INVALID_FUNCTION;
    }
    return FnPostGetContextMenuRegexResponse(MgrHandle, Context, Response);
}

ULONG CDrvMan::CloseManager(HANDLE MgrHandle)
{
    if(NULL == FnCloseManager) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return ERROR_INVALID_FUNCTION;
    }
    return FnCloseManager(MgrHandle);
}

ULONG CDrvMan::EnableAntiTampering(BOOL Enable)
{
    if(NULL == FnEnableAntiTampering) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return ERROR_INVALID_FUNCTION;
    }
    return FnEnableAntiTampering(Enable);
}
