


#include <Windows.h>
#include <Shlobj.h>
#include <assert.h>

#include <iostream>
#include <fstream>

#include <nudf\exception.hpp>
#include <nudf\path.hpp>
#include <nudf\bitmap.hpp>
#include <nudf\shared\enginectl.h>
#include <nudf\shared\obutil.h>
#include <nudf\shared\officelayout.h>

#include <nudf\string.hpp>
#include <nudf\nxrmres.h>
#include <nudf\resutil.hpp>

// from drvman
#include "nxrmdrv.h"
#include "nxrmdrvman.h"

#include "nxrmflt.h"

#include "nxrmserv.h"
#include "sconfig.hpp"
#include "process_status.hpp"
#include "drvcore.hpp"

#include "debug.hpp"
#include "servctrl.hpp"


using namespace NX;



//
//  class drv_man
//
class drv_man
{
typedef HANDLE(WINAPI* NXRMDRV_CREATE_MANAGER)(NXRMDRV_CALLBACK_NOTIFY, LOGAPI_LOG, LOGAPI_ACCEPT, PVOID);
typedef ULONG(WINAPI* NXRMDRV_POSTCHECK_OBLIGATIONS_RESPONSE)(HANDLE, PVOID, NXRM_OBLIGATION*, ULONG);
typedef ULONG(WINAPI* NXRMDRV_POST_OFFICE_GETCUSTOMUI_RESPONSE)(HANDLE, PVOID, OFFICE_GET_CUSTOMUI_RESPONSE*);
typedef ULONG(WINAPI* NXRMDRV_QUERY_PROCESSINFO)(HANDLE, ULONG, NXRM_PROCESS_ENTRY*);
typedef ULONG(WINAPI* NXRMDRV_IS_REQUEST_CANCELED)(HANDLE, PVOID);
typedef ULONG(WINAPI* NXRMDRV_IS_PROCESS_ALIVE)(HANDLE, ULONG);
typedef ULONG(WINAPI* NXRMDRV_START)(HANDLE);
typedef ULONG(WINAPI* NXRMDRV_STOP)(HANDLE);
typedef ULONG(WINAPI* NXRMDRV_INCREASE_POLICY_SERIALNO)(HANDLE);
typedef ULONG(WINAPI* NXRMDRV_CLOSE_MANAGER)(HANDLE);
typedef ULONG(WINAPI* NXRMDRV_POST_SAVEAS_FORECAST_RESPONSE)(HANDLE, PVOID);
typedef ULONG(WINAPI* NXRMDRV_POST_GET_CLASSIFY_UI_RESPONSE)(HANDLE, PVOID, GET_CLASSIFY_UI_RESPONSE*);
typedef ULONG(WINAPI* NXRMDRV_POST_BLOCK_NOTIFICATION_RESPONSE)(HANDLE, PVOID);
typedef ULONG(WINAPI* NXRMDRV_POST_CHECK_PROTECT_MENU_RESPONSE)(HANDLE, PVOID, CHECK_PROTECT_MENU_RESPONSE*);
typedef ULONG(WINAPI* NXRMDRV_POST_GET_CONTEXT_MENU_REGEX_RESPONSE)(HANDLE, PVOID, QUERY_CTXMENUREGEX_RESPONSE*);
typedef ULONG(WINAPI* NXRMDRV_POST_UPDATE_CORE_CONTEXT_RESPONSE)(HANDLE, PVOID);
typedef ULONG(WINAPI* NXRMDRV_POST_QUERY_CORE_CONTEXT_RESPONSE)(HANDLE, PVOID, QUERY_CORE_CTX_RESPONSE*);
typedef ULONG(WINAPI* NXRMDRV_POST_QUERY_SERVICE_RESPONSE)(HANDLE, PVOID, QUERY_SERVICE_RESPONSE*);
typedef ULONG(WINAPI* NXRMDRV_POST_QUERY_UPDATE_DWM_WINDOW_RESPONSE)(HANDLE, PVOID);
typedef ULONG(WINAPI* NXRMDRV_POST_QUERY_UPDATE_OVERLAY_WINDOW_RESPONSE)(HANDLE, PVOID);
typedef ULONG(WINAPI* NXRMDRV_POST_CHECK_PROCESS_RIGHTS_RESPONSE)(HANDLE, PVOID, CHECK_PROCESS_RIGHTS_RESPONSE*);
typedef ULONG(WINAPI* NXRMDRV_ENABLE_ANTITAMPERING)(BOOL);
typedef ULONG(WINAPI* NXRMDRV_SET_OVERLAY_PROTECTED_WINDOWS)(HANDLE, ULONG, ULONG*, ULONG*);
typedef ULONG(WINAPI* NXRMDRV_SET_OVERLAY_BITMAP_STATUS)(HANDLE, ULONG, BOOL);
public:
    drv_man() : hMod(NULL),
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
        FnPostUpdateCoreContextResponse(NULL),
        FnPostQueryCoreContextResponse(NULL),
        FnPostQueryServiceResponse(NULL),
        FnPostUpdateDwmWindowResponse(NULL),
        FnPostUpdateOverlayWindowResponse(NULL),
        FnPostCheckProcessRightsResponse(NULL),
        FnCloseManager(NULL),
        FnEnableAntiTampering(NULL),
        FnSetOverlayProtectedWindows(NULL),
        FnSetOverlayBitmapStatus(NULL)
    {
        load();
    }
    ~drv_man()
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
        wsDll += L"nxrmdrvman.dll";

        hMod = ::LoadLibraryW(wsDll.c_str());
        if (NULL != hMod) {
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
            FnPostUpdateCoreContextResponse = (NXRMDRV_POST_UPDATE_CORE_CONTEXT_RESPONSE)GetProcAddress(hMod, MAKEINTRESOURCEA(115));
            FnPostQueryCoreContextResponse = (NXRMDRV_POST_QUERY_CORE_CONTEXT_RESPONSE)GetProcAddress(hMod, MAKEINTRESOURCEA(116));
            FnPostQueryServiceResponse = (NXRMDRV_POST_QUERY_SERVICE_RESPONSE)GetProcAddress(hMod, MAKEINTRESOURCEA(117));
            FnPostUpdateDwmWindowResponse = (NXRMDRV_POST_QUERY_UPDATE_DWM_WINDOW_RESPONSE)GetProcAddress(hMod, MAKEINTRESOURCEA(118));
            FnPostUpdateOverlayWindowResponse = (NXRMDRV_POST_QUERY_UPDATE_OVERLAY_WINDOW_RESPONSE)GetProcAddress(hMod, MAKEINTRESOURCEA(119));
            FnPostCheckProcessRightsResponse = (NXRMDRV_POST_CHECK_PROCESS_RIGHTS_RESPONSE)GetProcAddress(hMod, MAKEINTRESOURCEA(120));
            FnCloseManager = (NXRMDRV_CLOSE_MANAGER)GetProcAddress(hMod, MAKEINTRESOURCEA(199));
            FnEnableAntiTampering = (NXRMDRV_ENABLE_ANTITAMPERING)GetProcAddress(hMod, MAKEINTRESOURCEA(200));
            FnSetOverlayProtectedWindows = (NXRMDRV_SET_OVERLAY_PROTECTED_WINDOWS)GetProcAddress(hMod, MAKEINTRESOURCEA(201));
            FnSetOverlayBitmapStatus = (NXRMDRV_SET_OVERLAY_BITMAP_STATUS)GetProcAddress(hMod, MAKEINTRESOURCEA(202));
        }
    }
    void unload()
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
        FnPostUpdateCoreContextResponse = NULL;
        FnPostQueryCoreContextResponse = NULL;
        FnPostQueryServiceResponse = NULL;
        FnPostUpdateDwmWindowResponse = NULL;
        FnPostUpdateOverlayWindowResponse = NULL;
        FnPostCheckProcessRightsResponse = NULL;
        FnCloseManager = NULL;
        FnEnableAntiTampering = NULL;
        FnSetOverlayProtectedWindows = NULL;
        FnSetOverlayBitmapStatus = NULL;
        if (NULL != hMod) {
            FreeLibrary(hMod);
        }
    }
    bool is_valid() const noexcept
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
            FnPostUpdateCoreContextResponse != NULL &&
            FnPostQueryCoreContextResponse != NULL &&
            FnPostQueryServiceResponse != NULL &&
            FnPostUpdateDwmWindowResponse != NULL &&
            FnPostUpdateOverlayWindowResponse != NULL &&
            FnPostCheckProcessRightsResponse != NULL &&
            FnCloseManager != NULL &&
            FnEnableAntiTampering != NULL &&
            FnSetOverlayProtectedWindows != NULL &&
            FnSetOverlayBitmapStatus != NULL);
    }

    HANDLE CreateManager(NXRMDRV_CALLBACK_NOTIFY CbNotify, LOGAPI_LOG CbDbgDump, LOGAPI_ACCEPT CbDbgDumpCheckLevel, PVOID Context)
    {
        if (NULL == FnCreateManager) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return NULL;
        }
        return FnCreateManager(CbNotify, CbDbgDump, CbDbgDumpCheckLevel, Context);
    }
    ULONG PostCheckObligationsResponse(HANDLE MgrHandle, PVOID Context, NXRM_OBLIGATION* Obs, ULONG ObsLength)
    {
        if (NULL == FnPostCheckObligationsResponse) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnPostCheckObligationsResponse(MgrHandle, Context, Obs, ObsLength);
    }
    ULONG PostOfficeGetCustomUIResponse(HANDLE MgrHandle, PVOID Context, OFFICE_GET_CUSTOMUI_RESPONSE* Response)
    {
        if (NULL == FnPostOfficeGetCustomUIResponse) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnPostOfficeGetCustomUIResponse(MgrHandle, Context, Response);
    }
    ULONG QueryProcessInfo(HANDLE MgrHandle, ULONG ProcessId, NXRM_PROCESS_ENTRY* Info)
    {
        if (NULL == FnQueryProcessInfo) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnQueryProcessInfo(MgrHandle, ProcessId, Info);
    }
    ULONG IsRequestCanceled(HANDLE MgrHandle, PVOID Context)
    {
        if (NULL == FnIsRequestCanceled) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnIsRequestCanceled(MgrHandle, Context);
    }
    ULONG IsProcessAlive(HANDLE MgrHandle, ULONG ProcessId)
    {
        if (NULL == FnIsProcessAlive) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnIsProcessAlive(MgrHandle, ProcessId);
    }
    ULONG Start(HANDLE MgrHandle)
    {
        if (NULL == FnStart) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnStart(MgrHandle);
    }
    ULONG Stop(HANDLE MgrHandle)
    {
        if (NULL == FnStop) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnStop(MgrHandle);
    }
    ULONG PostSaveAsForecastResponse(HANDLE MgrHandle, PVOID Context)
    {
        if (NULL == FnPostSaveAsForecastResponse) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnPostSaveAsForecastResponse(MgrHandle, Context);
    }
    ULONG PostGetClassifyUIResponse(HANDLE MgrHandle, PVOID Context, GET_CLASSIFY_UI_RESPONSE* Response)
    {
        if (NULL == FnPostGetClassifyUIResponse) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnPostGetClassifyUIResponse(MgrHandle, Context, Response);
    }
    ULONG PostBlockNotificationResponse(HANDLE MgrHandle, PVOID Context)
    {
        if (NULL == FnPostBlockNotificationResponse) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnPostBlockNotificationResponse(MgrHandle, Context);
    }
    ULONG PostCheckProtectMenuResponse(HANDLE MgrHandle, PVOID Context, CHECK_PROTECT_MENU_RESPONSE* Response)
    {
        if (NULL == FnPostBlockNotificationResponse) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnPostCheckProtectMenuResponse(MgrHandle, Context, Response);
    }
    ULONG PostGetContextMenuRegexResponse(HANDLE MgrHandle, PVOID Context, QUERY_CTXMENUREGEX_RESPONSE* Response)
    {
        if (NULL == FnPostGetContextMenuRegexResponse) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnPostGetContextMenuRegexResponse(MgrHandle, Context, Response);
    }
    ULONG PostUpdateCoreContextResponse(HANDLE MgrHandle, PVOID Context)
    {
        if (NULL == FnPostUpdateCoreContextResponse) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnPostUpdateCoreContextResponse(MgrHandle, Context);
    }
    ULONG PostQueryCoreContextResponse(HANDLE MgrHandle, PVOID Context, QUERY_CORE_CTX_RESPONSE* Response)
    {
        if (NULL == FnPostQueryCoreContextResponse) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnPostQueryCoreContextResponse(MgrHandle, Context, Response);
    }
    ULONG PostQueryServiceResponse(HANDLE MgrHandle, PVOID Context, QUERY_SERVICE_RESPONSE* Response)
    {
        if (NULL == FnPostQueryServiceResponse) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnPostQueryServiceResponse(MgrHandle, Context, Response);
    }
    ULONG PostUpdateDwmWindowResponse(HANDLE MgrHandle, PVOID Context)
    {
        if (NULL == FnPostUpdateDwmWindowResponse) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnPostUpdateDwmWindowResponse(MgrHandle, Context);
    }
    ULONG PostUpdateOverlayWindowResponse(HANDLE MgrHandle, PVOID Context)
    {
        if (NULL == FnPostUpdateOverlayWindowResponse) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnPostUpdateOverlayWindowResponse(MgrHandle, Context);
    }
    ULONG PostCheckProcessRightsResponse(HANDLE MgrHandle, PVOID Context, CHECK_PROCESS_RIGHTS_RESPONSE* Response)
    {
        if (NULL == FnPostCheckProcessRightsResponse) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnPostCheckProcessRightsResponse(MgrHandle, Context, Response);
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
    ULONG EnableAntiTampering(BOOL Enable)
    {
        if (NULL == FnEnableAntiTampering) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        return FnEnableAntiTampering(Enable);
    }
    ULONG SetOverlayProtectedWindows(HANDLE MgrHandle, ULONG SessionId, ULONG* hWnd, ULONG* cbSize)
    {
        if (NULL == FnSetOverlayProtectedWindows) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        return FnSetOverlayProtectedWindows(MgrHandle, SessionId, hWnd, cbSize);
    }
    ULONG SetOverlayBitmapStatus(HANDLE MgrHandle, ULONG SessionId, BOOL bReady)
    {
        if (NULL == FnSetOverlayBitmapStatus) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        return FnSetOverlayBitmapStatus(MgrHandle, SessionId, bReady);
    }
    ULONG IncreasePolicySerialNo(HANDLE MgrHandle)
    {
        if (NULL == FnIncreasePolicySerialNo) {
            SetLastError(ERROR_INVALID_FUNCTION);
            return ERROR_INVALID_FUNCTION;
        }
        if (NULL == MgrHandle) {
            SetLastError(ERROR_INVALID_HANDLE);
            return ERROR_INVALID_HANDLE;
        }
        return FnIncreasePolicySerialNo(MgrHandle);
    }


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
    NXRMDRV_POST_UPDATE_CORE_CONTEXT_RESPONSE FnPostUpdateCoreContextResponse;
    NXRMDRV_POST_QUERY_CORE_CONTEXT_RESPONSE FnPostQueryCoreContextResponse;
    NXRMDRV_POST_QUERY_SERVICE_RESPONSE FnPostQueryServiceResponse;
    NXRMDRV_POST_QUERY_UPDATE_DWM_WINDOW_RESPONSE FnPostUpdateDwmWindowResponse;
    NXRMDRV_POST_QUERY_UPDATE_OVERLAY_WINDOW_RESPONSE FnPostUpdateOverlayWindowResponse;
    NXRMDRV_POST_CHECK_PROCESS_RIGHTS_RESPONSE FnPostCheckProcessRightsResponse;
    NXRMDRV_CLOSE_MANAGER FnCloseManager;
    NXRMDRV_ENABLE_ANTITAMPERING FnEnableAntiTampering;
    NXRMDRV_SET_OVERLAY_PROTECTED_WINDOWS FnSetOverlayProtectedWindows;
    NXRMDRV_SET_OVERLAY_BITMAP_STATUS FnSetOverlayBitmapStatus;
};


//
//  class 
//
class overlay_text_info
{
public:
    overlay_text_info() : _text(L"$(User)\\n$(Time)"), _transratio(90), _font(L"Sitka Text"), _size(16), _color(RGB(0, 0, 0)), _rotation(-45)
    {
    }
    virtual ~overlay_text_info() {}

    void load(const std::map<std::wstring,std::wstring>& parameters)
    {

        auto pos = parameters.find(L"text");
        if (pos != parameters.end() && (*pos).second.length() != 0) {
            SetText((*pos).second);
        }
        pos = parameters.find(L"transparency");
        if (pos != parameters.end() && (*pos).second.length() != 0) {
            int nTrans = 10;
            if (nudf::string::ToInt<wchar_t>((*pos).second, &nTrans)) {
                SetTransparency(nTrans);
            }
        }
        pos = parameters.find(L"fontname");
        if (pos != parameters.end() && (*pos).second.length() != 0) {
            SetFontName((*pos).second);
        }
        pos = parameters.find(L"fontsize");
        if (pos != parameters.end() && (*pos).second.length() != 0) {
            int nSize = 0;
            if (nudf::string::ToInt<wchar_t>((*pos).second, &nSize)) {
                SetFontSize(abs(nSize));
            }
        }
        pos = parameters.find(L"textcolor");
        if (pos != parameters.end() && (*pos).second.length() != 0) {
            SetTextColor((*pos).second);
        }
        pos = parameters.find(L"rotation");
        if (pos != parameters.end() && (*pos).second.length() != 0) {
            SetRotationAngle((*pos).second);
        }
    }

    inline const std::wstring& GetText() const throw() { return _text; }
    inline int GetTransparency() const throw() { return _transratio; }
    inline const std::wstring& GetFontName() const throw() { return _font; }
    inline int GetFontSize() const throw() { return _size; }
    inline COLORREF GetFontColor() const throw() { return _color; }
    inline int GetRotationAngle() const throw() { return _rotation; }

    inline void SetText(const std::wstring& text) throw() { _text = text; }
    inline void SetTransparency(int ratio) throw()
    {
        _transratio = (100 - ((abs(ratio) <= 100) ? abs(ratio) : 100));
    }
    inline void SetFontName(const std::wstring& name) throw() { _font = name; }
    inline void SetFontSize(int size) throw() { _size = abs(size); }
    inline void SetTextColor(COLORREF color) throw() { _color = color; }
    inline void SetTextColor(const std::wstring& name) throw()
    {
        //
        // See RGB color table:
        //    http://www.rapidtables.com/web/color/RGB_Color.htm
        //
        if (0 == _wcsicmp(name.c_str(), L"Red")) {
            _color = RGB(255, 0, 0);
        }
        else if (0 == _wcsicmp(name.c_str(), L"Lime")) {
            _color = RGB(0, 255, 0);
        }
        else if (0 == _wcsicmp(name.c_str(), L"Blue")) {
            _color = RGB(0, 0, 255);
        }
        else if (0 == _wcsicmp(name.c_str(), L"Yellow")) {
            _color = RGB(255, 255, 0);
        }
        else if (0 == _wcsicmp(name.c_str(), L"Cyan / Aqua")) {
            _color = RGB(0, 255, 255);
        }
        else if (0 == _wcsicmp(name.c_str(), L"Magenta / Fuchsia")) {
            _color = RGB(255, 0, 255);
        }
        else if (0 == _wcsicmp(name.c_str(), L"Gray")) {
            _color = RGB(128, 128, 128);
        }
        else if (0 == _wcsicmp(name.c_str(), L"Dim Gray")) {
            _color = RGB(105, 105, 105);
        }
        else if (0 == _wcsicmp(name.c_str(), L"Maroon")) {
            _color = RGB(128, 0, 0);
        }
        else if (0 == _wcsicmp(name.c_str(), L"Olive")) {
            _color = RGB(128, 128, 0);
        }
        else if (0 == _wcsicmp(name.c_str(), L"Green")) {
            _color = RGB(0, 128, 0);
        }
        else if (0 == _wcsicmp(name.c_str(), L"Purple")) {
            _color = RGB(128, 0, 128);
        }
        else if (0 == _wcsicmp(name.c_str(), L"Teal")) {
            _color = RGB(0, 128, 128);
        }
        else if (0 == _wcsicmp(name.c_str(), L"Navy")) {
            _color = RGB(0, 0, 128);
        }
        else {
            // Black
            _color = RGB(0, 0, 0);
        }
    }
    inline void SetRotationAngle(int angle) throw() { _rotation = (angle % 180); }
    inline void SetRotationAngle(const std::wstring& angle) throw()
    {
        if (0 == _wcsicmp(angle.c_str(), L"None")) {
            _rotation = 0;
        }
        else if (0 == _wcsicmp(angle.c_str(), L"Clockwise")) {
            _rotation = -45;
        }
        else {
            _rotation = 45;
        }
    }

    overlay_text_info& operator = (const overlay_text_info& ti)
    {
        if (this != (&ti)) {
            _text = ti.GetText();
            _transratio = ti.GetTransparency();
            _font = ti.GetFontName();
            _size = ti.GetFontSize();
            _color = ti.GetFontColor();
            _rotation = ti.GetRotationAngle();
        }
        return *this;
    }

private:
    std::wstring    _text;
    int             _transratio;
    std::wstring    _font;
    int             _size;
    COLORREF        _color;
    int             _rotation;
};



//
//  Local data
//
static drv_man DRVMAN;


//
//  class NX::drv::serv_drv
//
NX::drv::serv_drv::drv_request::drv_request(unsigned long type, void* msg, unsigned long length, void* msg_context) : _type(0), _context(NULL)
{
    unsigned long required_size = 0;

    switch (type)
    {
    case NXRMDRV_MSG_TYPE_GET_CUSTOMUI:
        required_size = (unsigned long)sizeof(OFFICE_GET_CUSTOMUI_REQUEST);
        break;
    case NXRMDRV_MSG_TYPE_CHECKOBLIGATION:
        required_size = (unsigned long)sizeof(CHECK_OBLIGATION_REQUEST);
        break;
    case NXRMDRV_MSG_TYPE_SAVEAS_FORECAST:
        required_size = (unsigned long)sizeof(SAVEAS_FORECAST_REQUEST);
        break;
    case NXRMDRV_MSG_TYPE_GET_CLASSIFYUI:
        required_size = (unsigned long)sizeof(GET_CLASSIFY_UI_REQUEST);
        break;
    case NXRMDRV_MSG_TYPE_BLOCK_NOTIFICATION:
        required_size = (unsigned long)sizeof(BLOCK_NOTIFICATION_REQUEST);
        break;
    case NXRMDRV_MSG_TYPE_CHECK_PROTECT_MENU:
        required_size = (unsigned long)sizeof(CHECK_PROTECT_MENU_REQUEST);
        break;
    case NXRMDRV_MSG_TYPE_UPDATE_PROTECTEDMODEAPPINFO:
        required_size = (unsigned long)sizeof(UPDATE_PROTECTEDMODEAPPINFO_REQUEST);
        break;
    case NXRMDRV_MSG_TYPE_QUERY_PROTECTEDMODEAPPINFO:
        required_size = (unsigned long)sizeof(QUERY_PROTECTEDMODEAPPINFO_REQUEST);
        break;
    case NXRMDRV_MSG_TYPE_GET_CTXMENUREGEX:
        required_size = (unsigned long)sizeof(QUERY_CTXMENUREGEX_REQUEST);
        break;
	case NXRMDRV_MSG_TYPE_QUERY_CORE_CTX:
		required_size = (unsigned long)sizeof(QUERY_CORE_CTX_REQUEST);
		break;
	case NXRMDRV_MSG_TYPE_UPDATE_CORE_CTX:
		required_size = (unsigned long)sizeof(UPDATE_CORE_CTX_REQUEST);
		break;
	case NXRMDRV_MSG_TYPE_QUERY_SERVICE:
		required_size = (unsigned long)sizeof(QUERY_SERVICE_REQUEST);
		break;
    case NXRMDRV_MSG_TYPE_UPDATE_DWM_WINDOW:
		required_size = (unsigned long)sizeof(UPDATE_DWM_WINDOW_REQUEST);
        break;
    case NXRMDRV_MSG_TYPE_UPDATE_OVERLAY_WINDOW:
        required_size = (unsigned long)sizeof(UPDATE_OVERLAY_WINDOW_REQUEST);
        break;
    case NXRMDRV_MSG_TYPE_CHECK_PROCESS_RIGHTS:
        required_size = (unsigned long)sizeof(CHECK_PROCESS_RIGHTS_REQUEST);
        break;
    default:
        break;
    }

    if (0 == required_size) {
        SetLastError(ERROR_INVALID_PARAMETER);
        LOGWARN(0, L"unknown driver request type (%x)", type);
    }

    _request.resize(required_size, 0);
    memcpy(&_request[0], msg, min(required_size, length));
    _type = type;
    _context = msg_context;
}

//
//  class NX::drv
//
NX::drv::serv_drv::serv_drv() : _drvman(NULL), _stop_event(NULL), _started(false)
{
    ::InitializeCriticalSection(&_list_lock);
    _stop_event = ::CreateEventW(NULL, TRUE, FALSE, NULL);
    _request_event = ::CreateEventW(NULL, TRUE, FALSE, NULL);
}

NX::drv::serv_drv::~serv_drv()
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

bool NX::drv::serv_drv::start() noexcept
{
    DWORD dwResult = 0;

    assert(!_started);

    if (_started) {
        return false;
    }


    try {
        _drvman = DRVMAN.CreateManager(NX::drv::serv_drv::drv_callback, write_log, check_log_accept, this);
        if (NULL == _drvman) {
            LOGERR(GetLastError(), L"fail to create driver manager for nxrmdrv.sys");
            throw std::exception("fail to create driver manager for nxrmdrv.sys");
        }

//#ifdef _DEBUG
        // disable anti-tampering
        dwResult = DRVMAN.EnableAntiTampering(FALSE);
        if (0 != dwResult) {
            LOGWARN(dwResult, L"fail to disable anti-tampering");
        }
//#endif

        // enable nxrmdrv manager
        dwResult = DRVMAN.Start(_drvman);
        if (0 != dwResult) {
            CloseHandle(_drvman);
            LOGERR(dwResult, L"fail to start driver manager for nxrmdrv.sys");
            throw std::exception("fail to start driver manager for nxrmdrv.sys");
        }

        // start worker thread
        for (int i = 0; i < 4; i++) {
            _threads.push_back(std::thread(NX::drv::serv_drv::worker, this));
        }

        _started = true;
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

void NX::drv::serv_drv::stop() noexcept
{
    if (_started) {
        DRVMAN.Stop(_drvman);
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

void NX::drv::serv_drv::on_get_custom_ui(drv_request* request)
{
    OFFICE_GET_CUSTOMUI_REQUEST*    req = (OFFICE_GET_CUSTOMUI_REQUEST*)request->request();
    OFFICE_GET_CUSTOMUI_RESPONSE    resp = { 0 };

    if (0 != DRVMAN.IsRequestCanceled(_drvman, request->context())) {
        LOGWARN(0, L"request (GET_CUSTOMUI) has been canceled");
        return;
    }
    
    try {

        const char* xml = OFFICE_LAYOUT_XML;

        NX::process_info pi = GLOBAL.drv_core().query_process_info(req->ProcessId);

        if (boost::algorithm::istarts_with(req->OfficeVersion, L"14")) {
            if (boost::algorithm::iends_with(pi.image(), L"\\WINWORD.EXE")) {
                xml = WORD_LAYOUT_XML_14;
            }
            else if (boost::algorithm::iends_with(pi.image(), L"\\EXCEL.EXE")) {
                xml = EXCEL_LAYOUT_XML_14;
            }
            else if (boost::algorithm::iends_with(pi.image(), L"\\POWERPNT.EXE")) {
                xml = POWERPNT_LAYOUT_XML_14;
            }
            else {
                xml = OFFICE_LAYOUT_XML;
            }
        }
        else if (boost::algorithm::istarts_with(req->OfficeVersion, L"15")) {
            if (boost::algorithm::iends_with(pi.image(), L"\\WINWORD.EXE")) {
                xml = WORD_LAYOUT_XML_15;
            }
            else if (boost::algorithm::iends_with(pi.image(), L"\\EXCEL.EXE")) {
                xml = EXCEL_LAYOUT_XML_15;
            }
            else if (boost::algorithm::iends_with(pi.image(), L"\\POWERPNT.EXE")) {
                xml = POWERPNT_LAYOUT_XML_15;
            }
            else {
                xml = OFFICE_LAYOUT_XML;
            }
        }
        else {
            if (boost::algorithm::iends_with(pi.image(), L"\\WINWORD.EXE")) {
                xml = WORD_LAYOUT_XML_15;
            }
            else if (boost::algorithm::iends_with(pi.image(), L"\\EXCEL.EXE")) {
                xml = EXCEL_LAYOUT_XML_15;
            }
            else if (boost::algorithm::iends_with(pi.image(), L"\\POWERPNT.EXE")) {
                xml = POWERPNT_LAYOUT_XML_15;
            }
            else {
                xml = OFFICE_LAYOUT_XML;
            }
        }

        if (0 == GetTempFileNameW(req->TempPath, L"nxrm", 0, resp.CustomUIFileName)) {
            LOGERR(GetLastError(), L"fail to get temp file name");
            throw std::exception("fail to get temp file name");
        }

        std::ofstream fp;
        fp.open(resp.CustomUIFileName, std::ofstream::binary | std::ofstream::trunc);
        if (!fp.is_open() || !fp.good()) {
            LOGERR(GetLastError(), L"fail to create temp file");
            throw std::exception("fail to create temp file");
        }

        fp.write(xml, strlen(xml));
        if (fp.fail() || fp.bad()) {
            throw std::exception("fail to write xml content");
        }

    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        resp.CustomUIFileName[0] = L'\0';
    }

    if (ERROR_SUCCESS != DRVMAN.PostOfficeGetCustomUIResponse(_drvman, request->context(), &resp)) {
        LOGERR(GetLastError(), L"Fail to post office get custom UI response");
    }
}

static std::wstring generate_watermark_image(_In_ const overlay_text_info& oti, _In_ const std::wstring& folder)
{
    std::wstring image_file;
    nudf::image::CTextBitmap bitmap;

    GetTempFileNameW(folder.c_str(), L"WM", 0, nudf::string::tempstr<wchar_t>(image_file, MAX_PATH));

    if (!bitmap.Create(oti.GetText().c_str(), oti.GetFontName().c_str(), oti.GetFontSize(), RGB(255, 255, 255), oti.GetFontColor())) {
        return std::wstring();
    }

    if (oti.GetRotationAngle() != 0) {
        bitmap.Rotate(NULL, oti.GetRotationAngle());
    }

    if (!bitmap.ToFile(image_file.c_str())) {
        return std::wstring();
    }

    return image_file;
}

static void remove_escape_string(std::wstring& s)
{
    if (s.length() != 0) {

        std::wstring ns;
        bool changed = false;
        const wchar_t* const end_pos = s.c_str() + s.length();
        const wchar_t * p = s.c_str();
        while (p != end_pos) {

            wchar_t c = *(p++);
            if (c == L'\\') {
                switch (*p)
                {
                case L'n':
                    c = L'\n';
                    ++p;
                    changed = true;
                    break;
                case L'r':
                    c = L'\r';
                    ++p;
                    changed = true;
                    break;
                case L't':
                    c = L'\t';
                    ++p;
                    changed = true;
                    break;
                case L'\\':
                    c = L'\\';
                    ++p;
                    changed = true;
                    break;
                default:
                    break;
                }
            }
            ns.push_back(c);
        }

        if (changed) {
            s = ns;
        }
    }
}

static std::wstring generate_watermark_text_image(const std::wstring& user, const std::wstring& file, const std::wstring& folder, overlay_text_info& oti)
{
    std::wstring file_name;

    const wchar_t* fnp = wcsrchr(file.c_str(), L'\\');
    file_name = (NULL != fnp) ? (fnp + 1) : file;

    std::wstring date_info;
    std::wstring time_info;
    std::wstring time_zone;
    SYSTEMTIME st = { 0 };
    GetLocalTime(&st);
    nudf::time::CTimeZone zone;
    zone.BiasToString(time_zone);
    swprintf_s(nudf::string::tempstr<wchar_t>(date_info, 64), 64, L"%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
    swprintf_s(nudf::string::tempstr<wchar_t>(time_info, 64), 64, L"%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
    
    std::wstring watermark_text;

    watermark_text = oti.GetText();
    if (watermark_text.empty()) {
        watermark_text = user;
        watermark_text += L"\n";
        watermark_text += date_info;
        watermark_text += L" ";
        watermark_text += time_info;
        watermark_text += L"\n";
        watermark_text += file_name;
    }
    else {
        remove_escape_string(watermark_text);
        boost::algorithm::replace_all(watermark_text, L"$(User)", user);
        boost::algorithm::replace_all(watermark_text, L"$(Date)", date_info);
        boost::algorithm::replace_all(watermark_text, L"$(Time)", time_info);
        boost::algorithm::replace_all(watermark_text, L"$(Document)", file_name);
    }
    oti.SetText(watermark_text);

    return generate_watermark_image(oti, folder);
}

void NX::drv::serv_drv::on_check_obligations(drv_request* request)
{
    PCHECK_OBLIGATION_REQUEST   req = (PCHECK_OBLIGATION_REQUEST)request->request();
    unsigned long   session_id = -1;
    std::wstring    account_info;
    std::vector<unsigned char> obs_block;

    if (0 != DRVMAN.IsRequestCanceled(_drvman, request->context())) {
        LOGWARN(0, L"request (CHECK_OBLIGATIONS) has been canceled");
        return;
    }

    if (!ProcessIdToSessionId(req->ProcessId, &session_id)) {
        LOGWARN(GetLastError(), L"fail to get requestor process session id");
        if (ERROR_SUCCESS != DRVMAN.PostCheckObligationsResponse(_drvman, request->context(), NULL, 0)) {
            LOGERR(GetLastError(), L"Fail to post check obligation response");
        }
        return;
    }

    std::shared_ptr<NX::session> sp = GLOBAL.serv_session().get(session_id);
    if (sp == NULL) {
        LOGWARN(GetLastError(), L"fail to get requestor process session object");
        if (ERROR_SUCCESS != DRVMAN.PostCheckObligationsResponse(_drvman, request->context(), NULL, 0)) {
            LOGERR(GetLastError(), L"Fail to post check obligation response");
        }
        return;
    }

    if (req->EvaluationId == 0xFFFFFFFFFFFFFFFF) {

        // this request comes from DWM Manager
        if (!sp->get_overlay_image().image_path().empty()) {

            std::wstring param_image = OB_OVERLAY_PARAM_IMAGE;
            param_image += L"=";
            param_image += sp->get_overlay_image().image_path();
            std::wstring param_trans_ratio = OB_OVERLAY_PARAM_TRANSPARENCY;
            param_trans_ratio += L"=";
            param_trans_ratio += nudf::string::FromInt<wchar_t>(sp->get_overlay_image().transparency_ratio());

            const unsigned long block_size = (ULONG)(sizeof(NXRM_OBLIGATION) + (sizeof(wchar_t) * (param_image.length() + 1 + param_trans_ratio.length() + 1)));
            obs_block.resize(block_size, 0);
            NXRM_OBLIGATION* pob = (NXRM_OBLIGATION*)obs_block.data();
            pob->NextOffset = 0;
            pob->Id = OB_ID_OVERLAY;
            wchar_t* pos = pob->Params;
            memcpy(pos, param_image.c_str(), sizeof(wchar_t)*param_image.length());
            pos += (unsigned long)param_image.length();
            *pos = L'\0'; ++pos;
            memcpy(pos, param_trans_ratio.c_str(), sizeof(wchar_t)*param_trans_ratio.length());
            pos += (unsigned long)param_trans_ratio.length();
            *pos = L'\0'; ++pos;
            *pos = L'\0'; ++pos;

            LOGDBG(L"DWM Query Overlay");
            LOGDBG(L"    {%s:        %s}", OB_OVERLAY_PARAM_IMAGE, sp->get_overlay_image().image_path().c_str());
            LOGDBG(L"    {%s: %d}", OB_OVERLAY_PARAM_TRANSPARENCY, sp->get_overlay_image().transparency_ratio());
        }
        else {
            LOGDBG(L"DWM Query Overlay");
            LOGDBG(L"    {%s:        None}", OB_OVERLAY_PARAM_IMAGE);
            LOGDBG(L"    {%s: None}", OB_OVERLAY_PARAM_TRANSPARENCY);
        }

        if (ERROR_SUCCESS != DRVMAN.PostCheckObligationsResponse(_drvman, request->context(), (NXRM_OBLIGATION*)(obs_block.empty() ? NULL : (&obs_block[0])), (unsigned long)obs_block.size())) {
            LOGERR(GetLastError(), L"Fail to post check obligation response");
        }
        return;
    }

    try {


        // try to find obligation
        std::shared_ptr<NX::EVAL::eval_result> result = GLOBAL.eval_cache().get(req->EvaluationId);
        if (result == NULL) {
            throw std::exception("result not present");
        }
        if (result->hit_obligations().empty()) {
            throw std::exception("obligation not present");
        }
        assert(!result->hit_obligations().empty());

        NXRM_OBLIGATION* pob = NULL;
        size_t index = 0;

        LOGDBG(L"CheckObligation: EvaluationId=%d", req->EvaluationId);

        std::for_each(result->hit_obligations().begin(), result->hit_obligations().end(), [&](const std::pair<std::wstring,std::shared_ptr<NX::EVAL::obligation>>& v) {
            bool is_last_one = ((++index) == result->hit_obligations().size()) ? true : false;
            std::vector<unsigned char> buf;
            if (0 == _wcsicmp(v.first.c_str(), OB_NAME_CLASSIFY)) {
                buf.resize(sizeof(NXRM_OBLIGATION), 0);
                pob = (NXRM_OBLIGATION*)(&buf[0]);
                pob->NextOffset = is_last_one ? 0 : (unsigned long)buf.size();
                pob->Id = OB_ID_CLASSIFY;
                LOGDBG(L"  - CLASSIFY");
            }
            else if (0 == _wcsicmp(v.first.c_str(), OB_NAME_OVERLAY)) {

                //overlay_text_info oti;
                //std::wstring water_amrk_inage;

                LOGDBG(L"  - OVERLAY");

                // Get all the parameters
                //oti.load(v.second->parameters());

                // Generate Watermark
                // water_amrk_inage = generate_watermark_text_image(account_info, req->FileName, req->TempPath, oti);
                //if (!water_amrk_inage.empty()) {
                if (!sp->get_overlay_image().image_path().empty()) {

                    std::wstring param_image = OB_OVERLAY_PARAM_IMAGE;
                    param_image += L"=";
                    param_image += sp->get_overlay_image().image_path();
                    std::wstring param_trans_ratio = OB_OVERLAY_PARAM_TRANSPARENCY;
                    param_trans_ratio += L"=";
                    param_trans_ratio += nudf::string::FromInt<wchar_t>(sp->get_overlay_image().transparency_ratio());

                    LOGDBG(L"    {%s:        %s}", OB_OVERLAY_PARAM_IMAGE, sp->get_overlay_image().image_path().c_str());
                    LOGDBG(L"    {%s: %d}", OB_OVERLAY_PARAM_TRANSPARENCY, sp->get_overlay_image().transparency_ratio());

                    const unsigned long block_size = (ULONG)(sizeof(NXRM_OBLIGATION) + (sizeof(wchar_t) * (param_image.length() + 1 + param_trans_ratio.length() + 1)));
                    buf.resize(block_size, 0);
                    pob = (NXRM_OBLIGATION*)(&buf[0]);
                    pob->NextOffset = is_last_one ? 0 : block_size;
                    pob->Id = OB_ID_OVERLAY;
                    wchar_t* pos = pob->Params;
                    memcpy(pos, param_image.c_str(), sizeof(wchar_t)*param_image.length());
                    pos += (unsigned long)param_image.length();
                    *pos = L'\0'; ++pos;
                    memcpy(pos, param_trans_ratio.c_str(), sizeof(wchar_t)*param_trans_ratio.length());
                    pos += (unsigned long)param_trans_ratio.length();
                    *pos = L'\0'; ++pos;
                    *pos = L'\0'; ++pos;
                }
            }
            else {
                ; // unknown obligation, don't handle
            }

            if (!buf.empty()) {
                for (auto it = buf.begin(); it != buf.end(); ++it) {
                    obs_block.push_back(*it);
                }
            }
        });
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }

    if (ERROR_SUCCESS != DRVMAN.PostCheckObligationsResponse(_drvman, request->context(), (NXRM_OBLIGATION*)(obs_block.empty() ? NULL : (&obs_block[0])), (unsigned long)obs_block.size())) {
        LOGERR(GetLastError(), L"Fail to post check obligation response");
    }
}

void NX::drv::serv_drv::on_save_as_forecast(drv_request* request)
{
    PSAVEAS_FORECAST_REQUEST  req = (PSAVEAS_FORECAST_REQUEST)request->request();

    if (0 != DRVMAN.IsRequestCanceled(_drvman, request->context())) {
        LOGWARN(0, L"request (SAVEAS_FORECAST) has been canceled");
        return;
    }

    GLOBAL.drv_flt().set_saveas_forecast(req->ProcessId, req->SourceFileName, req->SaveAsFileName);
    LOGDBG(L"SAVEAS_FORECAST: process_id=%d, source = %s, target = %s", req->ProcessId, req->SourceFileName, req->SaveAsFileName);

    if (ERROR_SUCCESS != DRVMAN.PostSaveAsForecastResponse(_drvman, request->context())) {
        LOGWARN(0, L"Fail to post save-as forecast response");
    }
}

void NX::drv::serv_drv::on_get_classify_ui(drv_request* request)
{
    PGET_CLASSIFY_UI_REQUEST  req = (PGET_CLASSIFY_UI_REQUEST)request->request();
    GET_CLASSIFY_UI_RESPONSE  response = { 0 };
    NXRM_PROCESS_ENTRY        procinfo = { 0 };
    ULONG ulResult = 0;

    if (0 != DRVMAN.IsRequestCanceled(_drvman, request->context())) {
        LOGWARN(0, L"request (GET_CLASSIFY_UI) has been canceled");
        return;
    }


    try {
        std::shared_ptr<NX::session> sp = GLOBAL.serv_session().get(req->SessionId);
        memset(&response, 0, sizeof(response));
        if (NULL != sp) {
            // valid only if user has a valid group
            if (!sp->classify_group().empty()) {
                wcsncpy_s(response.GroupName, MAX_PATH, sp->classify_group().c_str(), _TRUNCATE);
                GetTempFileNameW(req->TempPath, L"CS", 0, response.ClassifyUIFileName);
                ::DeleteFileW(response.ClassifyUIFileName);
                std::wstring file = GLOBAL.dir_conf() + L"\\agent_classify.sjs";
                std::string s = NX::sconfig::load(file, GLOBAL.agent_key().decrypt());

                std::ofstream fp;
                fp.open(response.ClassifyUIFileName, std::ofstream::binary | std::ofstream::trunc);
                if (!fp.is_open() || !fp.good()) {
                    LOGERR(GetLastError(), L"fail to create temp file");
                    throw std::exception("fail to create temp file");
                }

                fp.write(s.c_str(), s.length());
                if (fp.fail() || fp.bad()) {
                    LOGERR(GetLastError(), L"fail to write classify json content");
                    throw std::exception("fail to write classify json content");
                }
                fp.close();
            }
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }

    if (ERROR_SUCCESS != DRVMAN.PostGetClassifyUIResponse(_drvman, request->context(), &response)) {
        LOGWARN(0, L"Fail to post check obligations response");
    }
}

void NX::drv::serv_drv::on_notification(drv_request* request)
{
    const BLOCK_NOTIFICATION_REQUEST* req = (const BLOCK_NOTIFICATION_REQUEST*)request->request();

    if (0 != DRVMAN.IsRequestCanceled(_drvman, request->context())) {
        LOGWARN(0, L"Request (NOTIFICATION) has been canceled");
        return;
    }

    std::wstring title = nudf::util::res::LoadMessage(GLOBAL.res_module(), IDS_PRODUCT_NAME, 256, LANG_NEUTRAL, L"NextLabs Rights Management");
    std::wstring operation;
    std::wstring file;
    std::wstring info;

    const WCHAR* pwzFileName = wcsrchr(req->FileName, L'\\');
    if (NULL == pwzFileName) pwzFileName = req->FileName;
    else pwzFileName++;
    file = pwzFileName;

    // Send Notification
    switch (req->Type)
    {
    case NxrmdrvSaveFileBlocked:
        operation = nudf::util::res::LoadMessage(GLOBAL.res_module(), IDS_OPERATION_SAVE, 64, LANG_NEUTRAL, L"save");
        info = nudf::util::res::LoadMessageEx(GLOBAL.res_module(), IDS_NOTIFY_OPERATION_DENIED, 1024, LANG_NEUTRAL, L"You don't have permission to %s this file (%s)", operation.c_str(), file.c_str());
        break;
    case NXrmdrvPrintingBlocked:
        operation = nudf::util::res::LoadMessage(GLOBAL.res_module(), IDS_OPERATION_PRINT, 64, LANG_NEUTRAL, L"print");
        info = nudf::util::res::LoadMessageEx(GLOBAL.res_module(), IDS_NOTIFY_OPERATION_DENIED, 1024, LANG_NEUTRAL, L"You don't have permission to %s this file (%s)", operation.c_str(), file.c_str());
        break;
    case NxrmdrvEmbeddedOleObjBlocked:
        operation = nudf::util::res::LoadMessage(GLOBAL.res_module(), IDS_OPERATION_INSERT, 64, LANG_NEUTRAL, L"insert content");
        info = nudf::util::res::LoadMessageEx(GLOBAL.res_module(), IDS_NOTIFY_OPERATION_DENIED_FROM, 1024, LANG_NEUTRAL, L"You don't have permission to %s from this file (%s)", operation.c_str(), file.c_str());
        break;
    case NxrmdrvSendMailBlocked:
        operation = nudf::util::res::LoadMessage(GLOBAL.res_module(), IDS_OPERATION_EMAIL, 64, LANG_NEUTRAL, L"email");
        info = nudf::util::res::LoadMessageEx(GLOBAL.res_module(), IDS_NOTIFY_OPERATION_DENIED, 1024, LANG_NEUTRAL, L"You don't have permission to %s this file (%s)", operation.c_str(), file.c_str());
        break;
    case NxrmdrvExportSlidesBlocked:
        operation = nudf::util::res::LoadMessage(GLOBAL.res_module(), IDS_OPERATION_EXPORT, 64, LANG_NEUTRAL, L"export");
        info = nudf::util::res::LoadMessageEx(GLOBAL.res_module(), IDS_NOTIFY_OPERATION_DENIED, 1024, LANG_NEUTRAL, L"You don't have permission to %s this file (%s)", operation.c_str(), file.c_str());
        break;
    case NxrmdrvSaveAsToUnprotectedVolume:
        info = nudf::util::res::LoadMessageEx(GLOBAL.res_module(), IDS_NOTIFY_OPERATION_SAVEAS_DENIED, 1024, LANG_NEUTRAL, L"File (%s) cannot be saved to unprotected location", file.c_str());
        break;
    case NxrmdrvAdobeHookIsNotReady:
        info = nudf::util::res::LoadMessageEx(GLOBAL.res_module(), IDS_NOTIFY_ADOBE_PLUGIN_NOT_READY, 1024, LANG_NEUTRAL, L"Rights Management for Adobe is not ready, please wait...");
        break;
	case NxrmdrvShellPrintBlocked:
		info = nudf::util::res::LoadMessageEx(GLOBAL.res_module(), IDS_NOTIFY_SHELL_PRINT_DENY, 1024, LANG_NEUTRAL, L"The print operation is denied. \nPlease open the file to print.%0");
		break;
	case NxrmdrvNotAuthorized:
		operation = nudf::util::res::LoadMessage(GLOBAL.res_module(), IDS_OPERATION_DECRYPT, 64, LANG_NEUTRAL, L"remove protection");
		info = nudf::util::res::LoadMessageEx(GLOBAL.res_module(), IDS_NOTIFY_OPERATION_DENIED_ON, 1024, LANG_NEUTRAL, L"You don't have permission to %s on this file (%s)", operation.c_str(), file.c_str());
		break;
    default:
        info = nudf::util::res::LoadMessageEx(GLOBAL.res_module(), IDS_NOTIFY_OPERATION_DENIED, 1024, LANG_NEUTRAL, L"You don't have permission to %s this file (%s)", operation.c_str(), file.c_str());
        break;
    }

    // Post
    DRVMAN.PostBlockNotificationResponse(_drvman, request->context());

    // Send notify
    if (!info.empty()) {

        LOGINF(info.c_str());

        std::shared_ptr<NX::session> sp = GLOBAL.serv_session().get(req->SessionId);
        if (sp != NULL) {
            sp->notify(title, info);
        }
        else {
            LOGERR(ERROR_INVALID_PARAMETER, L"session (%d) object not found", req->SessionId);
        }
    }
}

void NX::drv::serv_drv::on_check_protect_menu(drv_request* request)
{
    const CHECK_PROTECT_MENU_REQUEST* req = (const CHECK_PROTECT_MENU_REQUEST*)request->request();
    CHECK_PROTECT_MENU_RESPONSE       response = { 0 };

    if (0 != DRVMAN.IsRequestCanceled(_drvman, request->context())) {
        LOGWARN(0, L"request (CHECK_PROTECT_MENU) has been canceled");
        return;
    }

    std::shared_ptr<NX::session> sp = GLOBAL.serv_session().get(req->SessionId);
    memset(&response, 0, sizeof(response));
    if (NULL != sp) {
        // valid only if user has a valid group
        if (sp->logged_on() && !sp->classify_group().empty()) {
            response.EnableProtectMenu = 1;
        }
        else {
            response.EnableProtectMenu = 0;
        }
    }
    else {
        LOGWARN(0, L"session object not found (%d)", req->SessionId);
    }

    DRVMAN.PostCheckProtectMenuResponse(_drvman, request->context(), &response);
}

void NX::drv::serv_drv::on_update_protect_mode_app_info(drv_request* request)
{
    const UPDATE_PROTECTEDMODEAPPINFO_REQUEST* req = (const UPDATE_PROTECTEDMODEAPPINFO_REQUEST*)request->request();
    UPDATE_PROTECTEDMODEAPPINFO_RESPONSE       response = { 0 };

    memset(&response, 0, sizeof(response));

    if (0 != DRVMAN.IsRequestCanceled(_drvman, request->context())) {
        LOGWARN(0, L"request (UPDATE_PROTECTMODE_APPINFO) has been canceled");
        return;
    }

    try {
        response.Ack = 1;
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
}

void NX::drv::serv_drv::on_query_protect_mode_app_info(drv_request* request)
{
    const QUERY_PROTECTEDMODEAPPINFO_REQUEST* req = (const QUERY_PROTECTEDMODEAPPINFO_REQUEST*)request->request();
    QUERY_PROTECTEDMODEAPPINFO_RESPONSE       response = { 0 };

    memset(&response, 0, sizeof(response));

    if (0 != DRVMAN.IsRequestCanceled(_drvman, request->context())) {
        LOGWARN(0, L"request (QUERY_PROTECTMODE_APPINFO) has been canceled");
        return;
    }

    try {
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
}

void NX::drv::serv_drv::on_get_context_menu_regex(drv_request* request)
{
    const QUERY_CTXMENUREGEX_REQUEST* req = (const QUERY_CTXMENUREGEX_REQUEST*)request->request();
    QUERY_CTXMENUREGEX_RESPONSE       response = { 0 };

    if (0 != DRVMAN.IsRequestCanceled(_drvman, request->context())) {
        LOGWARN(0, L"request (GET_CONTEXT_MENU_REGEX) has been canceled");
        return;
    }

    //
    //  Currently we just simply return a regex string which matches anything
    //
    static const std::wstring reg_anything(L".*");    // Anything
    memset(&response, 0, sizeof(response));
    wcsncpy_s(response.CtxMenuRegEx, 1024, GLOBAL.whitelist_info().ext_list().empty() ? reg_anything.c_str() : GLOBAL.whitelist_info().ext_list().c_str(), _TRUNCATE);

    // Post
    DRVMAN.PostGetContextMenuRegexResponse(_drvman, request->context(), &response);
}

void NX::drv::serv_drv::on_update_core_context(drv_request* request)
{
    const UPDATE_CORE_CTX_REQUEST* req = (const UPDATE_CORE_CTX_REQUEST*)request->request();
    std::vector<unsigned __int64> context_buffer;
    for (int i = 0; i < MAX_CTX_COUNT; i++) {
        if (req->CtxData[i] != 0) {
            context_buffer.push_back(req->CtxData[i]);
        }
    }
    GLOBAL.update_core_context(req->ModuleFullPath, req->ModuleChecksum, context_buffer.empty() ? NULL : context_buffer.data(), (unsigned long)context_buffer.size());
    DRVMAN.PostUpdateCoreContextResponse(_drvman, request->context());
}

void NX::drv::serv_drv::on_query_core_context(drv_request* request)
{
    const QUERY_CORE_CTX_REQUEST* req = (const QUERY_CORE_CTX_REQUEST*)request->request();
    QUERY_CORE_CTX_RESPONSE       response = { 0 };
    memset(&response, 0, sizeof(response));
    NX::global::CoreContextType context = GLOBAL.query_core_context(req->ModuleFullPath);
    if (context.first != 0 && context.first == req->ModuleChecksum) {
        // found it
        response.ModuleChecksum = context.first;
        for (int i = 0; i < (int)context.second.size(); i++) {
            assert(i < MAX_CTX_COUNT);
            if (i >= MAX_CTX_COUNT) {
                break;
            }
            response.CtxData[i] = context.second[i];
        }
    }
    DRVMAN.PostQueryCoreContextResponse(_drvman, request->context(), &response);
}

void NX::drv::serv_drv::on_query_service(drv_request* request)
{
    const QUERY_SERVICE_REQUEST* req = (const QUERY_SERVICE_REQUEST*)request->request();
    QUERY_SERVICE_RESPONSE       response = { 0 };
    memset(&response, 0, sizeof(response));
    std::string str_result;

    try {

        std::string s((const char*)req->Data, ((const char*)req->Data) + MAX_SERVICE_DATA_LENGTH);
        std::wstring ws = NX::utility::conversions::utf8_to_utf16(s.c_str());
        NX::web::json::value json_request = NX::web::json::value::parse(ws);
        NX::web::json::value json_result = NX::web::json::value::object();

        std::shared_ptr<session> sp = GLOBAL.serv_session().get(req->SessionId);
        if (sp == nullptr) {
            LOGWARN(ERROR_NOT_FOUND, L"Query service requesy comes from unknown session (%d)", req->SessionId);
            throw std::exception("session not found");
        }

        switch (json_request[NXSERV_REQUEST_PARAM_CODE].as_integer())
        {
        case NXSERV::CTL_SERV_QUERY_STATUS:
            LOGDBG(L"QUERYSERVICE: CTL_SERV_QUERY_STATUS");
            json_result[NXSERV_REQUEST_PARAM_RESULT] = NX::web::json::value::number(ERROR_SUCCESS);
            json_result = sp->build_serv_status_response2();
            break;
        case NXSERV::CTL_SERV_UPDATE_POLICY:
            LOGDBG(L"QUERYSERVICE: CTL_SERV_UPDATE_POLICY");
            json_result[NXSERV_REQUEST_PARAM_RESULT] = NX::web::json::value::number(ERROR_SUCCESS);
            GLOBAL.set_requestor_session_id(req->SessionId);
            {
                std::shared_ptr<NX::job> sp = GLOBAL.jobs().get(NX::job::jn_heartbeat);
                sp->trigger();
            }
            break;
        case NXSERV::CTL_SERV_ENABLE_DEBUG:
            LOGDBG(L"QUERYSERVICE: CTL_SERV_ENABLE_DEBUG");
            GLOBAL.log().SetAcceptLevel(LOGDEBUG);
            json_result[NXSERV_REQUEST_PARAM_RESULT] = NX::web::json::value::number(ERROR_SUCCESS);
            break;
        case NXSERV::CTL_SERV_DISABLE_DEBUG:
            LOGDBG(L"QUERYSERVICE: CTL_SERV_DISABLE_DEBUG");
            GLOBAL.log().SetAcceptLevel(LOGINFO);
            json_result[NXSERV_REQUEST_PARAM_RESULT] = NX::web::json::value::number(ERROR_SUCCESS);
            break;
        case NXSERV::CTL_SERV_COLLECT_DEBUGLOG:
            LOGDBG(L"QUERYSERVICE: CTL_SERV_COLLECT_DEBUGLOG");
            json_result[NXSERV_REQUEST_PARAM_RESULT] = NX::web::json::value::number(ERROR_SUCCESS);
            try {
                NX::debug_object dbg(req->SessionId);
                if (!dbg.create()) {
                    json_result[NXSERV_REQUEST_PARAM_RESULT] = NX::web::json::value::number(-1);
                }
            }
            catch (std::exception& e) {
                UNREFERENCED_PARAMETER(e);
            }
            break;
        case NXSERV::CTL_SERV_QUERY_AUTHN_INFO:
            LOGDBG(L"QUERYSERVICE: CTL_SERV_QUERY_AUTHN_INFO");
            json_result[NXSERV_REQUEST_PARAM_RESULT] = NX::web::json::value::number(ERROR_SUCCESS);
            json_result[NXSERV_REQUEST_PARAM_AUTHN_SERVER_URL] = NX::web::json::value::string(L"");
            json_result[NXSERV_REQUEST_PARAM_AUTHN_RETURN_URL] = NX::web::json::value::string(L"");
            break;
        case NXSERV::CTL_SERV_LOGIN:
            LOGDBG(L"QUERYSERVICE: CTL_SERV_LOGIN");
            json_result[NXSERV_REQUEST_PARAM_RESULT] = NX::web::json::value::number(ERROR_SUCCESS);
            break;
        case NXSERV::CTL_SERV_LOGOUT:
            LOGDBG(L"QUERYSERVICE: CTL_SERV_LOGOUT");
            json_result[NXSERV_REQUEST_PARAM_RESULT] = NX::web::json::value::number(ERROR_SUCCESS);
            sp->logoff();
            break;
        case NXSERV::CTL_SERV_SET_DWM_STATUS:
            LOGDBG(L"QUERYSERVICE: CTL_SERV_SET_DWM_STATUS");
            json_result[NXSERV_REQUEST_PARAM_RESULT] = NX::web::json::value::number(ERROR_SUCCESS);
            sp->set_dwm_enabled(json_request[NXSERV_REQUEST_PARAM_EMABLED].as_bool());
            break;
        case NXSERV::CTL_SERV_EXPORT_ACTIVITY_LOG:
            LOGDBG(L"QUERYSERVICE: CTL_SERV_EXPORT_ACTIVITY_LOG");
            json_result[NXSERV_REQUEST_PARAM_RESULT] = NX::web::json::value::number(ERROR_SUCCESS);
            sp->export_activity_log(json_request[NXSERV_REQUEST_PARAM_FILE].as_string());
            break;
        case NXSERV::CTL_SERV_WRITE_LOG:
            LOGDBG(L"QUERYSERVICE: CTL_SERV_WRITE_LOG");
            json_result[NXSERV_REQUEST_PARAM_RESULT] = NX::web::json::value::number(ERROR_SUCCESS);
            LOG((LOGLEVEL)json_request[NXSERV_REQUEST_PARAM_LEVEL].as_integer(), json_request[NXSERV_REQUEST_PARAM_MESSAGE].as_string().c_str());
            break;
        case NXSERV::CTL_SERV_UNKNOWN:
        default:
            LOGWARN(ERROR_INVALID_PARAMETER, L"QUERYSERVICE: Unknown Code");
            json_result[NXSERV_REQUEST_PARAM_RESULT] = NX::web::json::value::number((int)ERROR_INVALID_PARAMETER);
            break;
        }

        str_result = NX::utility::conversions::utf16_to_utf8(json_result.serialize());
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        sprintf_s(nudf::string::tempstr<char>(str_result, MAX_PATH), MAX_PATH, "{\"%S\" = %d}", NXSERV_REQUEST_PARAM_RESULT, ERROR_BAD_COMMAND);
    }

    // set data
    memcpy(response.Data, str_result.c_str(), str_result.length());
    DRVMAN.PostQueryServiceResponse(_drvman, request->context(), &response);
}

void NX::drv::serv_drv::on_update_dwm_window(drv_request* request, bool overlay)
{
    if (!overlay) {
        // The top window is being created or destroyed
        const UPDATE_DWM_WINDOW_REQUEST* req = (const UPDATE_DWM_WINDOW_REQUEST*)request->request();
        std::shared_ptr<NX::process_status> sp = GLOBAL.process_cache().get_process(req->ProcessId);
        if (sp != nullptr) {

            if (req->Op == NXRMDRV_DWM_WINDOW_ADD || req->Op == NXRMDRV_DWM_WINDOW_DELETE) {
                // add or remove
                if (req->Op == NXRMDRV_DWM_WINDOW_ADD) {
                    sp->add_wnd(req->hWnd);
                }
                else {
                    sp->remove_wnd(req->hWnd);
                }
                // update
				if (sp->is_flag_on(NXRM_PROCESS_FLAG_WITH_NXL_OPENED) && !sp->is_flag_on(NXRM_PROCESS_FLAG_HAS_NO_OVERLAYPOLICY)) {
                    const std::vector<unsigned long>& wndbuf = GLOBAL.process_cache().get_valid_overlay_windows(req->SessionId);
                    set_overlay_windows(req->SessionId, wndbuf);
                }
            }
            else {
                // Unknown operator
                LOGWARN(ERROR_INVALID_PARAMETER, L"NXRMDRV_MSG_TYPE_UPDATE_DWM_WINDOW: invalid operator (%d), process (%d, %s)", req->Op, req->ProcessId, sp->get_image().c_str());
            }
        }

        // set data
        DRVMAN.PostUpdateDwmWindowResponse(_drvman, request->context());
    }
    else {
        // Enable/Disable overlay for an existing window 
        const UPDATE_OVERLAY_WINDOW_REQUEST* req = (const UPDATE_OVERLAY_WINDOW_REQUEST*)request->request();
        std::shared_ptr<NX::process_status> sp = GLOBAL.process_cache().get_process(req->ProcessId);
        if (sp != nullptr) {

            if (req->Op == NXRMDRV_OVERLAY_WINDOW_ADD || req->Op == NXRMDRV_OVERLAY_WINDOW_DELETE) {
                // add or remove
                if (req->Op == NXRMDRV_OVERLAY_WINDOW_ADD) {
                    if (!sp->is_flag_on(NXRM_PROCESS_FLAG_WITH_NXL_OPENED)) {
                        sp->add_flags(NXRM_PROCESS_FLAG_WITH_NXL_OPENED);
                    }
                    sp->add_wnd(req->hWnd, NX::dwm_window::WndStatusTrue);
                }
                else {
                    sp->remove_wnd(req->hWnd);
                }
                // update
                const std::vector<unsigned long>& wndbuf = GLOBAL.process_cache().get_valid_overlay_windows(req->SessionId);
                set_overlay_windows(req->SessionId, wndbuf);
            }
            else {
                // Unknown operator
                LOGWARN(ERROR_INVALID_PARAMETER, L"NXRMDRV_MSG_TYPE_UPDATE_OVERLAY_WINDOW: invalid operator (%d), process (%d, %s)", req->Op, req->ProcessId, sp->get_image().c_str());
            }
        }

        // set data
        DRVMAN.PostUpdateOverlayWindowResponse(_drvman, request->context());
    }
}

void NX::drv::serv_drv::on_check_process_rights(drv_request* request)
{
    const CHECK_PROCESS_RIGHTS_REQUEST* req = (const CHECK_PROCESS_RIGHTS_REQUEST*)request->request();
    CHECK_PROCESS_RIGHTS_RESPONSE       response = { 0 };

    if (0 != DRVMAN.IsRequestCanceled(_drvman, request->context())) {
        LOGWARN(0, L"request (GET_CONTEXT_MENU_REGEX) has been canceled");
        return;
    }

    try {

        std::shared_ptr<NX::process_status> sp_proc = GLOBAL.process_cache().get_process(req->ProcessId);
        if (sp_proc != nullptr) {
            // set process rights
            response.Rights = sp_proc->get_process_rights();
        }
        else {
            LOGWARN(0, L"Fail to handle drv request (CHECK_PROCESS_RIGHTS: process not found)");
        }
    }
    catch (const std::exception& e) {
        LOGWARN(0, L"Fail to handle drv request (CHECK_PROCESS_RIGHTS: %S)", e.what());
    }

    // set data
    DRVMAN.PostCheckProcessRightsResponse(_drvman, request->context(), &response);
}

void NX::drv::serv_drv::on_unknown_request(drv_request* request)
{
    LOGWARN(ERROR_INVALID_PARAMETER, L"Unknown drv request (0x%08X)", request->type());
}

process_info NX::drv::serv_drv::query_process_info(unsigned long process_id) noexcept
{
    NXRM_PROCESS_ENTRY entry = { 0 };
    if (0 != DRVMAN.QueryProcessInfo(_drvman, process_id, &entry)) {
        return process_info();
    }
    return process_info(entry.session_id, entry.process_path);
}

void NX::drv::serv_drv::set_overlay_windows(unsigned long session_id, const std::vector<unsigned long>& wnds)
{
    unsigned long count = (unsigned long)(wnds.size() * sizeof(unsigned long));
    unsigned long no_wnd = 0;
    ULONG* p = wnds.empty() ? (&no_wnd) : ((ULONG*)wnds.data());
    DRVMAN.SetOverlayProtectedWindows(_drvman, session_id, p, &count);
}

void NX::drv::serv_drv::set_overlay_bitmap_status(unsigned long session_id, bool ready)
{
    DRVMAN.SetOverlayBitmapStatus(_drvman, session_id, ready ? TRUE : FALSE);
}

void NX::drv::serv_drv::increase_policy_sn()
{
	DRVMAN.IncreasePolicySerialNo(_drvman);
}


void NX::drv::serv_drv::worker(serv_drv* serv) noexcept
{
    HANDLE events[2] = { serv->_stop_event, serv->_request_event };

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
        std::shared_ptr<drv_request> request;
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
        try {

            switch (request->type())
            {
            case NXRMDRV_MSG_TYPE_CHECKOBLIGATION:
                serv->on_check_obligations(request.get());
                break;
            case NXRMDRV_MSG_TYPE_GET_CUSTOMUI:
                serv->on_get_custom_ui(request.get());
                break;
            case NXRMDRV_MSG_TYPE_SAVEAS_FORECAST:
                serv->on_save_as_forecast(request.get());
                break;
            case NXRMDRV_MSG_TYPE_GET_CLASSIFYUI:
                serv->on_get_classify_ui(request.get());
                break;
            case NXRMDRV_MSG_TYPE_BLOCK_NOTIFICATION:
                serv->on_notification(request.get());
                break;
            case NXRMDRV_MSG_TYPE_CHECK_PROTECT_MENU:
                serv->on_check_protect_menu(request.get());
                break;
            case NXRMDRV_MSG_TYPE_UPDATE_PROTECTEDMODEAPPINFO:
                serv->on_update_protect_mode_app_info(request.get());
                break;
            case NXRMDRV_MSG_TYPE_QUERY_PROTECTEDMODEAPPINFO:
                serv->on_query_protect_mode_app_info(request.get());
                break;
            case NXRMDRV_MSG_TYPE_GET_CTXMENUREGEX:
                serv->on_get_context_menu_regex(request.get());
                break;
            case NXRMDRV_MSG_TYPE_UPDATE_CORE_CTX:
                serv->on_update_core_context(request.get());
                break;
            case NXRMDRV_MSG_TYPE_QUERY_CORE_CTX:
                serv->on_query_core_context(request.get());
                break;
            case NXRMDRV_MSG_TYPE_QUERY_SERVICE:
                serv->on_query_service(request.get());
                break;
            case NXRMDRV_MSG_TYPE_UPDATE_DWM_WINDOW:
                serv->on_update_dwm_window(request.get(), false);
                break;
            case NXRMDRV_MSG_TYPE_UPDATE_OVERLAY_WINDOW:
                serv->on_update_dwm_window(request.get(), true);
                break;
            case NXRMDRV_MSG_TYPE_CHECK_PROCESS_RIGHTS:
                serv->on_check_process_rights(request.get());
                break;
            default:
                serv->on_unknown_request(request.get());
                break;
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
        catch (...) {
            LOGASSERT(L"Unknown exception");
        }

        // done
    }
}

unsigned long __stdcall NX::drv::serv_drv::drv_callback(unsigned long type, void* msg, unsigned long length, void* msg_context, void* user_context)
{
    unsigned long result = 0;
    serv_drv* serv = (serv_drv*)user_context;

    if (!GLOBAL.is_running()) {
        LOGDBG(L"core driver request (type == %d) comes in when service is not fully started", type);
        return ERROR_SERVICE_NOT_ACTIVE;
    }

    std::shared_ptr<drv_request> request(new drv_request(type, msg, length, msg_context));
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

BOOL __stdcall NX::drv::serv_drv::check_log_accept(unsigned long level)
{
    return GLOBAL.log().AcceptLevel((LOGLEVEL)level);
}

LONG __stdcall NX::drv::serv_drv::write_log(const wchar_t* msg)
{
    return GLOBAL.log().Push(msg);
}



