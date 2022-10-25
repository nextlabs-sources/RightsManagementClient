
#include <SDKDDKVer.h>
#include <Windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <winioctl.h>
#include <WinCred.h>
#include <Objbase.h>
#include <Dwmapi.h>

#include <sstream>

#include <nudf\shared\enginectl.h>
#include <nudf\exception.hpp>
#include <nudf\shared\logdef.h>
#include <nudf\string.hpp>
#include <nudf\asyncpipe.hpp>
#include <nudf\nxrmres.h>
#include <nudf\resutil.hpp>
#include <nudf\crypto.hpp>
#include <nudf\bitmap.hpp>

#include "resource.h"

#include "nxrmtray.h"
#include "engineutils.h"
#include "main_popup.h"
#include "rotateicon.h"

namespace TrimMemory {
#define NXRM_MEMTUNING_MAX_WORKINGSET             (1024*1024)
#define NXRM_MEMTUNING_MIN_WORKINGSET             (500*1024)
#define NXRM_MEMTUNING_TIMER  					  555		//Make this ID odd enough that other IDs will not be conflict with this one
#define NXRM_MEMTUNING_TIMESLOT					  30000		//Make memtrim 30 seconds

    void __stdcall TrimWorkingSet(void)
    {
        CoFreeUnusedLibraries();

        SetProcessWorkingSetSize(GetCurrentProcess(),
            NXRM_MEMTUNING_MIN_WORKINGSET,
            NXRM_MEMTUNING_MAX_WORKINGSET);
    }
}

using namespace TrimMemory;

// Enable Visual Style: supported by ComCtrl32.dll version 6 and later, see the configure follow
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#pragma endregion


// *************************************************************************
//
//  Define Global Variables
//
// *************************************************************************
static std::wstring     gwsEngPipeName(NXRM_ENGINE_IPC_NAME);
nxrm::tray::CConnServ   connServ;
nxrm::tray::serv_status g_serv_status;


#ifndef RESFILENAME
#define RESFILENAME     L"nxrmres.dll"
#endif
#ifndef RESFILENAME2
#define RESFILENAME2    L"res\\nxrmres.dll"
#endif

#define MAX_LOADSTRING 100

#define COLOR_FONT_NORMAL                 (RGB(255, 255, 255))
#define COLOR_STATUS_OK                   (RGB(38, 166, 91))
#define COLOR_POLICY_OK                   (RGB(27, 116, 64))
#define COLOR_UPDATE_OK                   (RGB(19, 83, 45))
#define COLOR_REFRESH_OK_NORMAL           (RGB(24, 104, 47))
#define COLOR_REFRESH_OK_HOVER            (RGB(29, 150, 43))

#define COLOR_STATUS_BREAK                (RGB(181, 181, 181))
#define COLOR_POLICY_BREAK                (RGB(161, 161, 161))
#define COLOR_UPDATE_BREAK                (RGB(141, 141, 141))
#define COLOR_REFRESH_BREAK_NORMAL        (RGB(131, 131, 131))
#define COLOR_REFRESH_BREAK_HOVER         (RGB(151, 151, 151))

#define COLOR_SETTING_BK                  (RGB(255, 255, 255))
#define COLOR_SETTING_TEXT                (RGB(131, 131, 131))
#define COLOR_SETTING_HOVER               (RGB(181, 181, 181))
#define COLOR_SETTING_NORMAL              (RGB(161, 161, 161))

// Global Variables:
WCHAR szTitle[MAX_LOADSTRING];					// The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

APPUI        g_APPUI;                           // Global Variables (most are handles of control )
APPSTATE     g_APPState;                        // Used to initial the program.(about UI or create UI)
                
static BOOL  g_bMouseOverButton;                // Mark if the mouse is over the button
static int   g_iMouseOverButtonID;              // Button's ID of the mouse is hover
static int   g_iPopupStatus = POPUP_SHOW;       // Pop up status
static long  g_lPopupIncrement = 0;             // Used to move main windows
static int   g_expandedHeight = 0;				// Use this height to restore window height
LONG_PTR     g_BtnClosePreProc;                 // Store return value of SetWindowsLongPtr
LONG_PTR     g_BtnRefreshPreProc;
LONG_PTR     g_BtnSettingPreProc;

//Following are defined for icon rotation
enum RotateState
{
    NX_ICON_RT_STATE_STOPPED,
    NX_ICON_RT_STATE_ROTATING,
    NX_ICON_RT_STATE_STOPPING
};

RotateState		 g_updateIconRotate = NX_ICON_RT_STATE_STOPPED;		//Indcate that update icon rotate state

nxrm::tray::logon_ui_flag g_logon_ui_flag;

static UINT  WM_TASKBARCREATED = 0;

static int		 g_iAngle = 0;


// Forward declarations of functions included in this code module:
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    OwnerDrawBNProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
BOOL                InitMainDlg(HWND hMainDlg);
bool                InitTrayIcon(HWND hWnd, HINSTANCE hInstance);
void                InitTrayIconMenu();
void                doShowTrayMsg(_In_ LPCWSTR wzTitle, _In_ LPCWSTR wzInfo);
void                doDrawBtnItem(HWND hWnd, LPARAM lParam, COLORREF bkColor, COLORREF txtColor, UINT uFormat, LONG lMargin);
HWND                doCreateToolTip(HWND hDlg, int iItemID, LPWSTR pszText);
void                doMoveMainDialog(void);
void                onShowingOrHidingPopup(void);
void                onBtnRefreshClicked(void);
void                onBtnSettingClicked(void);
void                QuitMain();
void				StopRotatingUpdateIndicator();
BOOL                IsDwmCompositionEnabled();
VOID                UpdateDwmCompositionStatus();

// Prepare Resource
HMODULE             LoadResourceModule();
VOID                SetPolicyTimestamp(_In_ const FILETIME* timestamp);
VOID                SetUpdateTimestamp(_In_ const FILETIME* timestamp);
VOID                UpdateTrayMenu(bool debug_on, bool external_authn, const std::wstring& logon_user);
VOID                UpdateServerStatus(const nxrm::tray::serv_status& st);

VOID                ProtectOfficeAddinKeys();


// *************************************************************************
//
//  WinMain
//
// *************************************************************************
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
    
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    MSG    msg;
    HACCEL hAccelTable;
    INITCOMMONCONTROLSEX icex;
    

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_NXRMUI, szWindowClass, MAX_LOADSTRING);
    
    std::wstring ws_port_seed;

    INT     nArgs = 0;
    LPCWSTR pwzCmdline = GetCommandLineW();
    LPWSTR* pwzArgs = CommandLineToArgvW(pwzCmdline, &nArgs);
    if(nArgs >= 1) {
        const WCHAR* pExeExt = wcsrchr(pwzArgs[0], L'.');
        if(NULL == pExeExt) pExeExt = pwzArgs[0];
        if(0 == _wcsicmp(pExeExt, L".exe")) {
            if(nArgs > 1) {
                ws_port_seed = pwzArgs[1];
            }
        }
        else {
            ws_port_seed = pwzArgs[0];
        }
    }

    // generate port
    LARGE_INTEGER li_seed = { 0, 0 };
    li_seed.QuadPart = (LONGLONG)nudf::crypto::ToCrc64(0, ws_port_seed.c_str(), (ULONG)(ws_port_seed.length() * sizeof(wchar_t)));
    swprintf_s(nudf::string::tempstr<wchar_t>(gwsEngPipeName, 128), 128, L"NXRM_SERV_IPC_%08X%08X", li_seed.HighPart, li_seed.LowPart);
    nxrm::tray::serv_control::set_serv_ctl_port(gwsEngPipeName);

    // Guarantee single instance
    nxrm::tray::session_guard   sguard;
    if (sguard.check_existence()) {
        HWND hWnTarget = FindWindowW(NULL, szTitle);
        if (NULL != hWnTarget) {
            ShowWindow(hWnTarget, SW_SHOWNORMAL);
            UpdateWindow(hWnTarget);
            SetForegroundWindow(hWnTarget);
        }
        return 0;
    }

    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TAB_CLASSES;
    InitCommonControlsEx(&icex);

    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, SW_HIDE)) {
        return FALSE;
    }
    SetWindowTextW(g_APPUI.hMainDlg, szTitle);

    WM_TASKBARCREATED = RegisterWindowMessageW(L"TaskbarCreated");

    // Main message loop:
    hAccelTable = NULL;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            if(g_APPUI.hHtmlNotifyDlg == 0 || ! IsDialogMessage(g_APPUI.hHtmlNotifyDlg, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    return (int) msg.wParam;
}

//Return the correct color according to whether is connected or mouse hover or not
COLORREF GetRefreshBtnBkColor()
{
    COLORREF bkColor;
    if (g_APPState.bConnected) {

        if (g_bMouseOverButton) {
            bkColor = COLOR_REFRESH_OK_HOVER;
        }
        else 
        {
            bkColor = COLOR_REFRESH_OK_NORMAL;
        }
    }
    else
    {
        if (g_bMouseOverButton) {
            bkColor = COLOR_REFRESH_BREAK_HOVER;
        }
        else {
            bkColor = COLOR_REFRESH_BREAK_NORMAL;
        }
    }
    return bkColor;

}

void DisplayIconInAngle(HWND hWnd, int iAngle)
{
    HWND hbtnWnd = GetDlgItem(hWnd, IDC_MAIN_POLICY_REFRESH);
    HDC hDc = GetDC(hbtnWnd);
    HICON hIcon = LoadIcon(g_APPUI.hInst, MAKEINTRESOURCE(IDI_REFRESH));
    DrawRotatedIconFromBmp(hDc, iAngle, hIcon, GetRefreshBtnBkColor());
    DeleteObject(hIcon);
    ReleaseDC(hbtnWnd, hDc);
}
// *************************************************************************
//
//  Local Routines
//
// *************************************************************************

//This function tried to find login window in the process of the nxrmtray.exe
//All strings will be localized when language changes, we do not use text as identifier
//We'll use class name only as identifier at this time.
HWND  FindPasswordWindow()
{
    HWND hChildWnd = NULL;
    WCHAR szClass[] = L"#32770";
    do
    {
        hChildWnd = FindWindowEx(NULL, hChildWnd, szClass, NULL);
        DWORD dwCurrThreadID = GetCurrentThreadId();
        DWORD dwWndProcID = 0;

        if (GetWindowThreadProcessId(hChildWnd, NULL) == dwCurrThreadID)
        {
            return hChildWnd;
        }

    } while (hChildWnd != NULL);

    return NULL;
}

//TODO: remove following test function:
//using namespace UN;
//void GenerateWaterMarkFile()
//{
//	nudf::image::CTextBitmap bitmap;
//	COLORREF rgbForground = RGB(255, 0, 0);
//
//	if (!bitmap.Create(L"User: W7x86UO13MSEa11\n2016-02-01 10:16:40\nnormal.pdf", L"Arial", 48, RGB(255, 255, 255), rgbForground)) {
//		MessageBox(NULL, L"Could not create bitmap", L"Bitmap Create Error", MB_OK);
//	}
//
//	bitmap.Rotate(NULL, 45); //Please change angle here
//	bitmap.SetTransparency(50);
//
//	if (!bitmap.ToPNGFile(L"C:\\USERS\\ELI\\WATERMARK.PNG")) {
//		MessageBox(NULL, L"Error", L"Save PNG Error", MB_OK);
//	}
//
//	if (!bitmap.ToFile(L"C:\\USERS\\ELI\\WATERMARK.BMP")) {
//		MessageBox(NULL, L"Error", L"Save BMP Error", MB_OK);
//	}
//
//}

//  Processes messages for the main window.
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LPDRAWITEMSTRUCT pdis = NULL;
    int              iItemID;
    HBRUSH           hBrush;
    HDC              hDc;
    RECT             rcClient;
    


    if (message == WM_TASKBARCREATED)
    {
        InitTrayIcon(hWnd, g_APPUI.hInst);
        return TRUE;
    }

    switch (message)
    {
    case WM_CREATE:
        g_APPState.bActive = TRUE;
        if(!EngineGetStatus()) {
            QuitMain();
        }

        SetTimer(hWnd, (UINT_PTR)KEEPALIVE_TIMER, (UINT)KEEPALIVE_ELAPSE_SHORT, NULL);
        SetTimer(hWnd, (UINT_PTR)KEYPROTECT_TIMER, (UINT)KEYPROTECT_ELAPSE, NULL);
        //Reduce Memory Usage timer
        SetTimer(hWnd, NXRM_MEMTUNING_TIMER, NXRM_MEMTUNING_TIMESLOT, NULL);

        ProtectOfficeAddinKeys();
        
        return TRUE;

    case WM_NCHITTEST:
        POINT pt;   
        pt.x = GET_X_LPARAM(lParam);   
        pt.y = GET_Y_LPARAM(lParam);    // The coordinate is relative to the upper-left corner of the screen.
        ScreenToClient(hWnd,&pt);       // Change to relative to the upper-left corner of the Client.
        if(MAINDLG_MOVABLE_HEIGHT > pt.y) {
            return HTCAPTION;
        }
        break;

    case WM_DWMCOMPOSITIONCHANGED:
        UpdateDwmCompositionStatus();
        break;
        
        // Process this message to change Main dialog's background color
    case WM_ERASEBKGND: 
        hDc = (HDC) wParam; 
        hBrush = CreateSolidBrush((g_APPState.bConnected ? COLOR_STATUS_OK : COLOR_STATUS_BREAK));
        GetClientRect(hWnd, &rcClient); 
        SetMapMode(hDc, MM_ANISOTROPIC); 
        SetWindowExtEx(hDc, 100, 100, NULL); 
        SetViewportExtEx(hDc, rcClient.right, rcClient.bottom, NULL); 
        FillRect(hDc, &rcClient, hBrush); 
        DeleteObject(hBrush);
        return TRUE; 

    case WM_COMMAND:
        iItemID = LOWORD(wParam);
        switch (iItemID)
        {
        case IDC_MAIN_BTN_CLOSE:
            if(POPUP_SHOWING == g_iPopupStatus || POPUP_HIDING == g_iPopupStatus) {
                return TRUE;
            }
            PostMessage(hWnd, WM_CLOSE, NULL, NULL);
            return TRUE;

        case IDC_MAIN_POLICY_REFRESH:
            if(POPUP_SHOWING == g_iPopupStatus || POPUP_HIDING == g_iPopupStatus || g_updateIconRotate != NX_ICON_RT_STATE_STOPPED) 
            {
                return TRUE;
            }

            g_iAngle  = 0; //Set start Angle
            SetTimer(hWnd, (UINT_PTR)ROTATE_REFRESH_TIMER, (UINT)ROTATE_REFRESH_TIME, NULL);
            g_updateIconRotate = NX_ICON_RT_STATE_ROTATING;

            onBtnRefreshClicked();
            return TRUE;

        case IDC_MAIN_SETTING:
            onBtnSettingClicked();
            return TRUE;

        default:
            break;
        }
        return TRUE;

    case WM_TIMER:
    {
        switch ((UINT_PTR)wParam)
        {
        case NXRM_MEMTUNING_TIMER:
            TrimWorkingSet();
            break;
        case KEEPALIVE_TIMER:
        {
            if (!EngineGetStatus()) {
                QuitMain();
            }
        }
        break;
        case KEYPROTECT_TIMER:
        {
            // Remove special keys
            ProtectOfficeAddinKeys();
        }
        case SHOW_TRAY_ICON_TIMER:
        {
            // show icon if succeed, we remove the timer
            if (InitTrayIcon(hWnd, g_APPUI.hInst))
            {
                KillTimer(hWnd, SHOW_TRAY_ICON_TIMER);
            }
        }
        break;
        case ROTATE_REFRESH_TIMER:
        {
            g_iAngle  += ROTATE_ANGLE_UNIT;
            if ((g_iAngle % 360) == 0)
            {
                if (g_updateIconRotate == NX_ICON_RT_STATE_STOPPING ||
                    g_iAngle / ROTATE_ANGLE_UNIT * ROTATE_REFRESH_TIME >= 7000) //If we exceeded 7 seconds we stop spinning the icon
                {
                    StopRotatingUpdateIndicator();
                }
            }
            DisplayIconInAngle(hWnd, g_iAngle);
        }
        break;
        }
    break;
    }
    case WM_DRAWITEM:
        {
            LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT) lParam;
            int              iButtonID = lpDIS->CtlID;

            switch (iButtonID)
            {
            case IDC_MAIN_BTN_CLOSE:
                // The third Parameter is Back ground color
                doDrawBtnItem(hWnd, lParam, g_APPState.bConnected ? COLOR_STATUS_OK : COLOR_STATUS_BREAK, COLOR_FONT_NORMAL, DT_RIGHT, 0);
                break;

            case IDC_MAIN_STATE_ICON:
                doDrawBtnItem(hWnd, lParam, g_APPState.bConnected ? COLOR_STATUS_OK : COLOR_STATUS_BREAK, COLOR_FONT_NORMAL, DT_RIGHT, 0);
                break;

            case IDC_MAIN_STATE_STR_CONNECT:
                doDrawBtnItem(hWnd, lParam, g_APPState.bConnected ? COLOR_STATUS_OK : COLOR_STATUS_BREAK, COLOR_FONT_NORMAL, DT_LEFT, 0);
                break;

            case IDC_MAIN_POLICY_LABEL:
                doDrawBtnItem(hWnd, lParam, g_APPState.bConnected ? COLOR_POLICY_OK : COLOR_POLICY_BREAK, COLOR_FONT_NORMAL, DT_RIGHT, 5);
                break;

            case IDC_MAIN_POLICY_VERSION:
                doDrawBtnItem(hWnd, lParam, g_APPState.bConnected ? COLOR_POLICY_OK : COLOR_POLICY_BREAK, COLOR_FONT_NORMAL, DT_LEFT, 5);
                break;

            case IDC_MAIN_LAST_UPDATE_LABEL:
                doDrawBtnItem(hWnd, lParam, g_APPState.bConnected ? COLOR_UPDATE_OK : COLOR_UPDATE_BREAK, COLOR_FONT_NORMAL, DT_LEFT, 10);
                break;

            case IDC_MAIN_LAST_UPDATE_TIME:
                doDrawBtnItem(hWnd, lParam, g_APPState.bConnected ? COLOR_UPDATE_OK : COLOR_UPDATE_BREAK, COLOR_FONT_NORMAL, DT_RIGHT, 10);
                break;

            case IDC_MAIN_POLICY_REFRESH:
                if (g_updateIconRotate == NX_ICON_RT_STATE_STOPPED)
                {
                    if (g_APPState.bConnected) {

                        if ((g_bMouseOverButton && IDC_MAIN_POLICY_REFRESH == g_iMouseOverButtonID)) {
                            doDrawBtnItem(hWnd, lParam, COLOR_REFRESH_OK_HOVER, COLOR_FONT_NORMAL, DT_RIGHT, 0);
                        }
                        else {
                            doDrawBtnItem(hWnd, lParam, COLOR_REFRESH_OK_NORMAL, COLOR_FONT_NORMAL, DT_RIGHT, 0);
                        }
                    }
                    else
                    {
                        if ((g_bMouseOverButton && IDC_MAIN_POLICY_REFRESH == g_iMouseOverButtonID)) {
                            doDrawBtnItem(hWnd, lParam, COLOR_REFRESH_BREAK_HOVER, COLOR_FONT_NORMAL, DT_RIGHT, 0);
                        }
                        else {
                            doDrawBtnItem(hWnd, lParam, COLOR_REFRESH_BREAK_NORMAL, COLOR_FONT_NORMAL, DT_RIGHT, 0);
                        }
                    }
                }
                else
                {//This one is needed when the animationWindow blocked.
                    DisplayIconInAngle(hWnd, g_iAngle);
                }
                break;

            case IDC_MAIN_VERTICAL:
                doDrawBtnItem(hWnd, lParam, COLOR_SETTING_BK, COLOR_SETTING_TEXT, DT_CENTER, 0);
                break;

            case IDC_MAIN_TITLE:
                doDrawBtnItem(hWnd, lParam, COLOR_SETTING_BK, COLOR_SETTING_TEXT, DT_LEFT, 0);
                break;

            case IDC_MAIN_BRAND_BMP:
                doDrawBtnItem(hWnd, lParam, COLOR_SETTING_BK, COLOR_SETTING_TEXT, DT_LEFT, 5);
                break;

            case IDC_MAIN_SETTING:
                if(TRUE == g_bMouseOverButton && IDC_MAIN_SETTING == g_iMouseOverButtonID) {
                    doDrawBtnItem(hWnd, lParam, COLOR_SETTING_HOVER, NULL, DT_RIGHT, 0);
                }
                else {
                    doDrawBtnItem(hWnd, lParam, COLOR_SETTING_NORMAL, NULL, DT_RIGHT, 0);
                }
                break;

            default:
                return TRUE;
            }

            return TRUE;
        }
        break;

    case WM_CTLCOLORSTATIC:
        iItemID = GetWindowLong ((HWND) lParam, GWL_ID);
        if(IDC_MAIN_STATE_STR_CONNECT == iItemID) {
            SetTextColor ((HDC) wParam, RGB(255, 255, 255)) ;
        }
        SetBkMode((HDC)wParam, TRANSPARENT);
        return (BOOL)(ULONG_PTR)((HBRUSH)GetStockObject(NULL_BRUSH));

    case WM_TRAY:
        switch (lParam)
        {
        case WM_RBUTTONUP:
            {
                POINT pt;
                int cmd;

                GetCursorPos(&pt);
                SetForegroundWindow(hWnd);
                cmd = TrackPopupMenu(g_APPUI.hTrayMenu, TPM_RETURNCMD, pt.x, pt.y, NULL, hWnd, NULL);
                switch(cmd)
                {
                case TRAY_MENU_OPEN_RMC_ID:
                    ShowWindow(hWnd, SW_SHOWNORMAL);
                    UpdateWindow(hWnd);
                    SetForegroundWindow(hWnd);
                    break;
                case TRAY_MENU_UPDATE_POLICY_ID:
                    EngineUpdatePolicy();
                    break;
                case TRAY_MENU_ENABLE_DEBUG_ID:
                    EngineEnableDebug(!g_APPState.bIsDebugOn);
                    break;
                case TRAY_MENU_COLLECT_LOGS_ID:
                    //TODO:TEST GENERATE TRANSPARENT BITMAP CODE:
                    //GenerateWaterMarkFile();
                    EngineCollectLog();
                    break;
                case TRAY_MENU_LOGINOUT_ID:
                    if (!g_logon_ui_flag.on()) {
                        g_APPState.bLoggedOn ? EngineLogOut() : EngineLogIn();
                    }
                    else
                    {
                        HWND hPswdWnd = FindPasswordWindow();
                        BringWindowToTop(hPswdWnd);
                        SetForegroundWindow(hPswdWnd);
                    }
                    break;
                default:
                    break;
                }
            }
            break;
        case WM_LBUTTONDOWN:
            ShowWindow(hWnd, SW_SHOWNORMAL);
            UpdateWindow(hWnd);
            SetForegroundWindow(hWnd);
            break;

        case WM_LBUTTONDBLCLK:
            break;
        }
        break;

    case WM_CLOSE:
        ShowWindow(hWnd, SW_MINIMIZE);
        ShowWindow(hWnd, SW_HIDE);
        return 0;

    case WM_DESTROY:
        g_APPUI.nid.uID = TRAY_ID;
        g_APPUI.nid.hWnd = hWnd;
        Shell_NotifyIcon(NIM_DELETE, &g_APPUI.nid);
        KillTimer(hWnd, (UINT)KEEPALIVE_TIMER);
        KillTimer(hWnd, (UINT)KEYPROTECT_TIMER);
        KillTimer(hWnd, (UINT)NXRM_MEMTUNING_TIMER);	//Kill this time for the memory tuning
        KillTimer(hWnd, (UINT)ROTATE_REFRESH_TIMER);
        EndDialog(hWnd, 0);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

//Owner draw button Proc
LRESULT CALLBACK OwnerDrawBNProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)  
{
    int      iCnt = 0;
    int      iControlID = 0;
    LONG_PTR PreProc = NULL;
    RECT     rect;
                           //ID of all window subclass's controls, whose PROC is  OwnerDrawBNProc
    //int      iButtonID[] = {IDC_TABBN_HOME, IDC_TABBN_UPDATE, IDC_TABBN_NOTIFY, IDC_TABBN_SETTING, IDC_HELP_BN};
    TRACKMOUSEEVENT tme;

    tme.cbSize = sizeof(tme);
    tme.hwndTrack = hWnd;
    tme.dwFlags = TME_LEAVE | TME_HOVER;
    tme.dwHoverTime = 1;

    iControlID = GetWindowLongW(hWnd, GWL_ID);
    //TODO: Get Current Child window control's ID and PreProc(Parent window's PROC normally)
    switch (iControlID)
    {
    case IDC_MAIN_BTN_CLOSE:
        PreProc = g_BtnClosePreProc;
        break;

    case IDC_MAIN_POLICY_REFRESH:
        PreProc = g_BtnRefreshPreProc;
        break;

    case IDC_MAIN_SETTING:
        PreProc = g_BtnSettingPreProc;
        break;

    default:
        return FALSE;
    }

    //Processing the message
    switch (message)
    {
    case WM_LBUTTONDBLCLK:
        PostMessage(hWnd, WM_LBUTTONDOWN, wParam, lParam);
        break;

    case WM_MOUSEMOVE:
        {
            TrackMouseEvent(&tme);  //"generate" WM_MOUSEHOVER and WM_MOUSELEAVE Message
            // return 0;
        }
        break;

    case WM_MOUSEHOVER:
        {
            if(POPUP_SHOWING == g_iPopupStatus || POPUP_HIDING == g_iPopupStatus)
            {
                return TRUE;
            }
            if(!g_bMouseOverButton )
            {
                g_bMouseOverButton = TRUE;
                g_iMouseOverButtonID = iControlID;
                GetClientRect(hWnd, &rect);
                if (iControlID != IDC_MAIN_POLICY_REFRESH || g_updateIconRotate == NX_ICON_RT_STATE_STOPPED)
                {
                    InvalidateRect(hWnd, &rect, TRUE);
                }
            }

            return 0;
        }// case WM_MOUSEHOVER

    case WM_MOUSELEAVE:
        {
            if(POPUP_SHOWING == g_iPopupStatus || POPUP_HIDING == g_iPopupStatus)
            {
                return TRUE;
            }
            if(g_bMouseOverButton)
            {
                g_bMouseOverButton = FALSE;
                GetClientRect(hWnd, &rect);
                if (iControlID != IDC_MAIN_POLICY_REFRESH || g_updateIconRotate == NX_ICON_RT_STATE_STOPPED)
                {
                    InvalidateRect(hWnd, &rect, TRUE);
                }
            }// if(g_bMouseOverButton)

            return 0;
        }// case WM_MOUSELEAVE

    default:
        break;
    }

    return CallWindowProc((WNDPROC)PreProc, hWnd, message, wParam, lParam);
}


//  FUNCTION: MyRegisterClass()
//  PURPOSE: Registers the window class.
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style			= CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc	= WndProc;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= DLGWINDOWEXTRA;
    wcex.hInstance		= hInstance;
    wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NXRMUI));
    wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground	= (HBRUSH)NULL;
    wcex.lpszMenuName	= 0;                                    // If need a menu, use MAKEINTRESOURCE(IDC_NXRMUI)
    wcex.lpszClassName	= szWindowClass;
    wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_NXRMUI));

    return RegisterClassEx(&wcex);
}

//   FUNCTION: InitInstance(HINSTANCE, int)
//   PURPOSE:  Saves instance handle and creates main window
//   COMMENTS: In this function, we store the instance handle in a global variable and create and display the main dialog window.
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   g_APPUI.hInst = hInstance;               // Store instance handle in our global variable
   g_APPUI.hResDll = LoadResourceModule();  // Load Resource Dll

   //Initial g_APPState struct
   g_APPState.dwLangId = LANG_NEUTRAL;      // Language ID
   g_APPState.bConnected = true;        
   g_APPState.bPopupNotify = TRUE;
   g_APPState.bIsDebugOn = true;
   g_APPState.bIconOn = false;
   wsprintfW(g_APPState.wzPolicyCreatedTime, L"%s", L"N/A");
   wsprintfW(g_APPState.wzLastUpdateTime, L"%s", L"N/A");
   wsprintfW(g_APPState.wzPolicyVersion, L"%s", L"N/A");

   // Init TrayIcon Menu
   InitTrayIconMenu();

   //TODO: Create main dialog
   HWND hMainDlg = NULL;
   hMainDlg = CreateDialogW(g_APPUI.hInst, MAKEINTRESOURCE(IDD_MAIN_DLG), 0, 0);
   if (!hMainDlg)
   {
        MessageBoxW(NULL, L"Create main dialog failed!", L"ERROR", MB_OK | MB_ICONERROR);
        return FALSE;
   }
   g_APPUI.hMainDlg = hMainDlg;

   //Some other initializations
   InitMainDlg(hMainDlg);
   ShowWindow(hMainDlg, nCmdShow);
   UpdateWindow(hMainDlg);
   InitTrayIcon(hMainDlg, g_APPUI.hInst);
   if (!g_APPState.bIconOn)
   {
       SetTimer(hMainDlg, SHOW_TRAY_ICON_TIMER, SHOW_TRAY_ICON_TIME, NULL);
   }


   // Update Aero Status
   UpdateDwmCompositionStatus();

   return TRUE;
}


//TODO: Initial Main Dialog's Tab Control
BOOL InitMainDlg(HWND hMainDlg)
{
    int      iCnt;
    std::wstring wsUiText;      // Store Msg string Loaded from ResDLL
    RECT     rcClient1;
    RECT     rcClient2;

    LONG wBtnCaptionResID[] = {
        IDS_MAIN_LABEL_POLICY, 
        IDS_MAIN_LABEL_LAST_UPDATE, 
        IDS_MAIN_TITLE};

    int iBtnWithCapID[] = {
        IDC_MAIN_POLICY_LABEL, 
        IDC_MAIN_LAST_UPDATE_LABEL, 
        IDC_MAIN_TITLE};

    int iBtnClickable[] = {
        IDC_MAIN_BTN_CLOSE, 
        IDC_MAIN_POLICY_REFRESH, 
        IDC_MAIN_SETTING};

    LONG lTipResID[] = {
        IDS_MAIN_CLOSE_TIP, 
        IDS_MAIN_REFRESH_TIP, 
        IDS_MAIN_SETTING_TIP};
    
    //
    //  Initialize Pipe
    //
    try {
        std::wstring tray_app_port;
        unsigned long session_id = 0;
        ProcessIdToSessionId(GetCurrentProcessId(), &session_id);
        swprintf_s(nudf::string::tempstr<wchar_t>(tray_app_port, MAX_PATH), MAX_PATH, L"nxrmtray-%d", session_id);
        connServ.listen(tray_app_port);
    }
    catch(const nudf::CException& e) {
        char szInfo[4096] = {0};
        sprintf_s(szInfo, "Fail to start pipe (%d).\n  Function=%s.\n  File=%s.\n  Line=%d.", e.GetCode(), e.GetFunction(), e.GetFile(), e.GetLine());
        MessageBoxA(hMainDlg, szInfo, "NextLabs", MB_OK|MB_ICONERROR);
    }

    // Try to get Engine Information
    EngineGetStatus();

    //TODO: subclass the button so we can know when the mouse was moved over it
    //Close button
    g_BtnClosePreProc = SetWindowLongPtr(GetDlgItem(hMainDlg, IDC_MAIN_BTN_CLOSE), GWLP_WNDPROC, 
        (LONG_PTR)OwnerDrawBNProc);
    //Refresh button
    g_BtnRefreshPreProc = SetWindowLongPtr(GetDlgItem(hMainDlg, IDC_MAIN_POLICY_REFRESH), GWLP_WNDPROC, 
        (LONG_PTR)OwnerDrawBNProc);
    //Setting button
    g_BtnSettingPreProc = SetWindowLongPtr(GetDlgItem(hMainDlg, IDC_MAIN_SETTING), GWLP_WNDPROC, 
        (LONG_PTR)OwnerDrawBNProc);



    //TODO: Initial the Caption of button
    for (iCnt = 0; iCnt < sizeof(wBtnCaptionResID) / sizeof(wBtnCaptionResID[0]); ++iCnt)
    {
        // Set the caption of control
        wsUiText = nudf::util::res::LoadMessage(g_APPUI.hResDll, (DWORD)wBtnCaptionResID[iCnt], 1024, g_APPState.dwLangId);
        SendMessageW(GetDlgItem(g_APPUI.hMainDlg, iBtnWithCapID[iCnt]), WM_SETTEXT, NULL, (LPARAM)(LPWSTR)wsUiText.c_str());
    }
    // Connect status string
    wsUiText = nudf::util::res::LoadMessage(g_APPUI.hResDll, 
        (g_APPState.bConnected ? IDS_MAIN_STATE_CONNECTED : IDS_MAIN_STATE_DISCONNECTED), 1024,
        g_APPState.dwLangId);
    SendMessageW(GetDlgItem(g_APPUI.hMainDlg, IDC_MAIN_STATE_STR_CONNECT), WM_SETTEXT, NULL, (LPARAM)(LPWSTR)wsUiText.c_str());

    //Policy Version
    if(g_APPState.wzPolicyVersion != '\0')
    {
        SendMessageW(GetDlgItem(g_APPUI.hMainDlg,IDC_MAIN_POLICY_VERSION), WM_SETTEXT, NULL, (LPARAM)g_APPState.wzPolicyVersion);
    }
    //Last updated time
    if(g_APPState.wzLastUpdateTime != '\0')
    {
        SendMessageW(GetDlgItem(g_APPUI.hMainDlg, IDC_MAIN_LAST_UPDATE_TIME), WM_SETTEXT, NULL, (LPARAM)g_APPState.wzLastUpdateTime);
    }


    //TODO: Set the tooltip for button that can be click
    for (iCnt = 0; iCnt < sizeof(lTipResID) / sizeof(lTipResID[0]); ++iCnt)
    {
        // Set the caption of control
        wsUiText = nudf::util::res::LoadMessage(g_APPUI.hResDll, (DWORD)lTipResID[iCnt], 1024, g_APPState.dwLangId);
        doCreateToolTip(g_APPUI.hMainDlg, iBtnClickable[iCnt], (LPWSTR)wsUiText.c_str());
    }

    InitMainPopupDlg();
    ShowWindow(g_APPUI.hMainPopupDlg,SW_SHOW);
    
    //TODO: Calculate Move windows increment
    GetWindowRect(GetDlgItem(g_APPUI.hMainDlg, IDC_MAIN_SETTING), &rcClient1);
    GetWindowRect(GetDlgItem(g_APPUI.hMainDlg, IDC_MAIN_LAST_UPDATE_LABEL), &rcClient2);
    g_lPopupIncrement = rcClient1.top - rcClient2.bottom;
    GetWindowRect(g_APPUI.hMainDlg, &rcClient1);
    g_expandedHeight = rcClient1.bottom - rcClient1.top;

    onBtnSettingClicked();

    return TRUE;
}


VOID UpdateServerStatus(const nxrm::tray::serv_status& st)
{
    if (g_APPState.bConnected != st.connected()) {
        g_APPState.bConnected = st.connected();
        // reset timer --- more/less often
        KillTimer(g_APPUI.hMainDlg, (UINT)KEEPALIVE_TIMER);
        SetTimer(g_APPUI.hMainDlg, (UINT_PTR)KEEPALIVE_TIMER, (UINT)(g_APPState.bConnected ? KEEPALIVE_ELAPSE : KEEPALIVE_ELAPSE_SHORT), NULL);
        std::wstring wsUiText = nudf::util::res::LoadMessage(g_APPUI.hResDll,
            (g_APPState.bConnected ? IDS_MAIN_STATE_CONNECTED : IDS_MAIN_STATE_DISCONNECTED), 1024,
            g_APPState.dwLangId);
        SendMessageW(GetDlgItem(g_APPUI.hMainDlg, IDC_MAIN_STATE_STR_CONNECT), WM_SETTEXT, NULL, (LPARAM)(LPWSTR)wsUiText.c_str());
        RedrawWindow(g_APPUI.hMainDlg, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
    }

    if (0 != _wcsicmp(g_APPState.wzCurrentServer, st.logon_server().c_str())) {
        wcsncpy_s(g_APPState.wzCurrentServer, 64, st.logon_server().c_str(), _TRUNCATE);
    }

    UpdateTrayMenu(g_serv_status.debug(), g_serv_status.external_authn(), g_serv_status.logon_user());

    bool user_or_policy_changed = false;
    if (0 != _wcsicmp(g_APPState.wzCurrentUser, st.logon_user().c_str())) {
        wcsncpy_s(g_APPState.wzCurrentUser, 64, st.logon_user().c_str(), _TRUNCATE);
        user_or_policy_changed = true;
        wcsncpy_s(g_APPState.wzCurrentUserId, 64, st.logon_user_id().empty() ? L"" : st.logon_user_id().c_str(), _TRUNCATE);
    }
    if (!st.policy_time().empty() && 0 != _wcsicmp(g_APPState.wzPolicyCreatedTime, st.policy_time().c_str())) {
        bool bInit = (0 == _wcsicmp(g_APPState.wzPolicyCreatedTime, L"N/A") || 0 == g_APPState.wzPolicyCreatedTime[0]) ? true : false;
        wcsncpy_s(g_APPState.wzPolicyCreatedTime, 256, st.policy_time().c_str(), _TRUNCATE);
        wcsncpy_s(g_APPState.wzPolicyVersion, 256, g_APPState.wzPolicyCreatedTime, _TRUNCATE);
        SendMessageW(GetDlgItem(g_APPUI.hMainDlg, IDC_MAIN_POLICY_VERSION), WM_SETTEXT, NULL, (LPARAM)g_APPState.wzPolicyCreatedTime);
        RedrawWindow(GetDlgItem(g_APPUI.hMainDlg, IDC_MAIN_POLICY_VERSION), NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
        // Notify: Policy is updated
        //if (!bInit) {
        //    std::wstring wsTitle = nudf::util::res::LoadMessage(g_APPUI.hResDll, IDS_TRAY_INFO_TITLE, 256, LANG_NEUTRAL, L"NextLabs Rights Management");
        //    std::wstring wsInfo = nudf::util::res::LoadMessage(g_APPUI.hResDll, IDS_NOTIFY_POLICY_UPDATED, 1024, LANG_NEUTRAL, L"Policy has been updated");
        //    doShowTrayMsg(wsTitle.c_str(), wsInfo.c_str());
        //}
        user_or_policy_changed = true;
    }
    if (user_or_policy_changed) {
        // Change tray-icon tip
        std::wstring wsTipFmt = nudf::util::res::LoadMessage(g_APPUI.hResDll, (DWORD)IDS_TRAY_TIP, 1024, g_APPState.dwLangId);
        std::wstring wsTip;
        swprintf_s(nudf::string::tempstr<wchar_t>(wsTip, 1024), 1023, wsTipFmt.c_str(), g_APPState.wzPolicyCreatedTime, g_APPState.wzCurrentUser);
        wcsncpy_s(g_APPUI.nid.szTip, 127, wsTip.c_str(), _TRUNCATE);
        Shell_NotifyIcon(NIM_MODIFY, &g_APPUI.nid);
    }
    if (!st.heartbeat_time().empty() && 0 != _wcsicmp(g_APPState.wzLastUpdateTime, st.heartbeat_time().c_str())) {
        // Show new updated information
        wcsncpy_s(g_APPState.wzLastUpdateTime, 256, st.heartbeat_time().c_str(), _TRUNCATE);
        SendMessageW(GetDlgItem(g_APPUI.hMainDlg, IDC_MAIN_LAST_UPDATE_TIME), WM_SETTEXT, NULL, (LPARAM)g_APPState.wzLastUpdateTime);
        RedrawWindow(GetDlgItem(g_APPUI.hMainDlg, IDC_MAIN_LAST_UPDATE_TIME), NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
    }
}

//  Show Tray icon
void InitTrayIconMenu()
{
    if(NULL == g_APPUI.hTrayMenu) {

        g_APPUI.hTrayMenu = CreatePopupMenu();
        if(NULL == g_APPUI.hTrayMenu) {
            return ;
        }
        // Add menu item
        std::wstring menu_text;  // Msg Loaded from ResDLL
        menu_text = nudf::util::res::LoadMessageEx(g_APPUI.hResDll, IDS_TRAY_MENU_OPEN_RMC, 1024, g_APPState.dwLangId, L"NextLabs Rights Management...");
        AppendMenuW(g_APPUI.hTrayMenu, MF_STRING, TRAY_MENU_OPEN_RMC_ID, menu_text.c_str());
        menu_text = nudf::util::res::LoadMessageEx(g_APPUI.hResDll, IDS_TRAY_MENU_UPDATE_POLICY, 1024, g_APPState.dwLangId, L"Update Rights Management Policy");
        AppendMenuW(g_APPUI.hTrayMenu, MF_STRING, TRAY_MENU_UPDATE_POLICY_ID, menu_text.c_str());
        menu_text = nudf::util::res::LoadMessageEx(g_APPUI.hResDll, g_APPState.bIsDebugOn ? IDS_TRAY_MENU_DISABLE_DEBUG : IDS_TRAY_MENU_ENABLE_DEBUG, 1024, g_APPState.dwLangId, g_APPState.bIsDebugOn ? L"Disable Debug" : L"Enable Debug");
        AppendMenuW(g_APPUI.hTrayMenu, MF_STRING, TRAY_MENU_ENABLE_DEBUG_ID, menu_text.c_str());
        menu_text = nudf::util::res::LoadMessageEx(g_APPUI.hResDll, IDS_TRAY_MENU_COLLECT_LOGS, 1024, g_APPState.dwLangId, L"Collect Debug Logs...");
        AppendMenuW(g_APPUI.hTrayMenu, MF_STRING, TRAY_MENU_COLLECT_LOGS_ID, menu_text.c_str());
        InsertMenuW(g_APPUI.hTrayMenu, 3, MF_SEPARATOR, NULL, NULL);  //Draw a horizontal dividing line.
    }
}

bool InitTrayIcon(HWND hMainDlg, HINSTANCE hInstance)
{
    int   iRet;

    g_APPUI.nid.cbSize = sizeof(NOTIFYICONDATA);
    g_APPUI.nid.hWnd = hMainDlg;
    g_APPUI.nid.uID = TRAY_ID;
    g_APPUI.nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
    g_APPUI.nid.uCallbackMessage = WM_TRAY;
    g_APPUI.nid.uTimeout = 1000;                   //The timeout value, in milliseconds, for notification.
    static const GUID BalloonGUID = { 0xb113fe22, 0x21a1, 0x43a8, { 0x8c, 0x62, 0xfb, 0x22, 0x13, 0xd, 0x3, 0x94 } };
    g_APPUI.nid.guidItem = BalloonGUID;
    g_APPUI.nid.dwInfoFlags = NIIF_USER;
    g_APPUI.nid.uVersion = NOTIFYICON_VERSION_4;
    LoadIconMetric(hInstance, MAKEINTRESOURCE(IDI_NXRMUI), LIM_SMALL, &(g_APPUI.nid.hIcon));  // Load the icon for high DPI.

    // Tray Icon Tip
    std::wstring wsTipFmt = nudf::util::res::LoadMessage(g_APPUI.hResDll, (DWORD)IDS_TRAY_TIP, 1024, g_APPState.dwLangId);
    swprintf_s(g_APPUI.nid.szTip, 128, wsTipFmt.c_str(), g_APPState.wzPolicyCreatedTime, g_APPState.wzCurrentUser);
    // Message bubble title
    std::wstring bubble_title = nudf::util::res::LoadMessage(g_APPUI.hResDll, (DWORD)IDS_PRODUCT_NAME, 1024, g_APPState.dwLangId);
    lstrcpy(g_APPUI.nid.szInfoTitle, bubble_title.empty() ? L"" : bubble_title.c_str());

    //TODO: Show the tray icon
    Shell_NotifyIconW(NIM_DELETE, &g_APPUI.nid);    // Always remove tray-icon before we add a new one
    iRet = Shell_NotifyIcon(NIM_ADD, &g_APPUI.nid);
    if(FALSE == iRet)
    {
        
        return false;
    }
    g_APPState.bIconOn = true;
    return true;
}

//TODO: show pop-up message when needed, it is not used currently(just for test)
void doShowTrayMsg(_In_ LPCWSTR wzTitle, _In_ LPCWSTR wzInfo)
{
    if(g_APPState.bPopupNotify) {
        g_APPUI.nid.uFlags |= NIF_INFO;
        lstrcpy(g_APPUI.nid.szInfoTitle, wzTitle);
        lstrcpy(g_APPUI.nid.szInfo, wzInfo);
        LoadIconMetric(g_APPUI.hInst, MAKEINTRESOURCE(IDI_NXRMUI), LIM_LARGE, &(g_APPUI.nid.hIcon));
        Shell_NotifyIcon(NIM_MODIFY, &g_APPUI.nid);
        g_APPUI.nid.uFlags &= (~NIF_INFO);
        g_APPUI.nid.szInfoTitle[0] = L'\0';
        g_APPUI.nid.szInfo[0] = L'\0';
    }
}

HFONT CreateControlFont(int iButtonID)
{
    HFONT hFont = NULL;

    int nFontHeight = 0;
    int nFontStyle = 0;

    switch (iButtonID)
    {
    case IDC_MAIN_STATE_STR_CONNECT:
        nFontHeight = 24;
        nFontStyle = FW_REGULAR;
        break;
    case IDC_MAIN_POLICY_VERSION:
        nFontHeight = 24;
        nFontStyle = FW_BOLD;
        break;
    case IDC_MAIN_POLICY_LABEL:
        nFontHeight = 16;
        nFontStyle = FW_BOLD;
        break;
    case IDC_MAIN_LAST_UPDATE_TIME:
        nFontHeight = 16;
        nFontStyle = FW_BOLD;
        break;
    default:
        return NULL;
    }

    hFont = CreateFont(nFontHeight, 0, 0, 45, nFontStyle, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_STROKE_PRECIS, CLIP_STROKE_PRECIS,
        DRAFT_QUALITY, FF_DONTCARE, TEXT("Segoe UI"));
    return hFont;
}

// Description:
//   Draw the button item in WM_DRAWITEM
// Parameters:
//   hDc - handle of Device contents
//   rcBtn - REVT of button
//   bkColor - The background color of button.
//   hIcon - The handle of en icon for the button, if the button has one.
//   wzCaption -The caption of the button, if it has a caption
//   uFormat - The method of formatting the text. But should just be DT_CENTER/DT_LEFT/DT_RIGHT
// Returns:
//   void.
void doDrawBtnItem(HWND hWnd, LPARAM lParam, COLORREF bkColor, COLORREF txtColor, UINT uFormat, LONG lMargin)
{
    HWND             hWndItem;
    HICON            hIcon = NULL;
    HBITMAP          hBmp = NULL;
    HBITMAP          hbmOld = NULL;
    HDC              hdcMem = NULL;
    BITMAP           bm;
    POINT            pt;
    WCHAR            wzCaption[MAX_STRLEN] = {0};
    HBRUSH           brBackground = NULL;

    LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT) lParam;
    int              iButtonID   = lpDIS->CtlID;
    HDC              hDc         = lpDIS->hDC; 
    BOOL             bIsPressed  = (lpDIS->itemState & ODS_SELECTED);
    BOOL             bIsFocused  = (lpDIS->itemState & ODS_FOCUS);
    BOOL             bIsDisabled = (lpDIS->itemState & ODS_DISABLED);
    BOOL             bDrawFocusRect = !(lpDIS->itemState & ODS_NOFOCUSRECT);
    RECT             rcBtn     = lpDIS->rcItem;


    LONG centerRectWidth   = rcBtn.right - rcBtn.left;   // temp of original ButtonRect when draw button control's text
    LONG centerRectHeight  = rcBtn.bottom - rcBtn.top;

    hWndItem = GetDlgItem(hWnd, iButtonID);
    GetWindowText(hWndItem, wzCaption, MAX_STRLEN);

    //Paint the background color
    SetBkMode(hDc, TRANSPARENT);
    brBackground = CreateSolidBrush(bkColor);
    FillRect(hDc, &rcBtn, brBackground);
    DeleteObject(brBackground);

    if (lstrlen(wzCaption) != 0 && hIcon == NULL)
    {
        if(DT_LEFT == uFormat)
        {
            rcBtn.left += lMargin;
        }
        else if(DT_RIGHT == uFormat)
        {
            rcBtn.right -= lMargin;
        }
        HFONT hFont = CreateControlFont(iButtonID);
        SetBkMode(hDc, TRANSPARENT);
        SetTextColor(hDc, txtColor);
        SetBkColor(hDc, ::GetSysColor(COLOR_BTNFACE));
        HFONT hOldFont;
        if (hFont)
        {
            hOldFont = SelectFont(hDc, hFont);
        }
        DrawText(hDc, wzCaption, -1, &rcBtn, DT_SINGLELINE  | DT_VCENTER | uFormat);
        if (hFont)
        {
            SelectFont(hDc, hOldFont);	//Select back the font.
            DeleteObject(hFont);
        }
    }
    
    //Draw Icon
    switch (iButtonID)
    {
    case IDC_MAIN_BTN_CLOSE:
        if(TRUE == g_bMouseOverButton && IDC_MAIN_BTN_CLOSE == g_iMouseOverButtonID)
        {
            hIcon = LoadIcon(g_APPUI.hInst, MAKEINTRESOURCE(IDI_CLOSE_HOVER));
        }
        else
        {
            hIcon = LoadIcon(g_APPUI.hInst, MAKEINTRESOURCE(IDI_CLOSE_NAORMAL));
        }
        DrawIconEx (hDc, rcBtn.left, rcBtn.top, hIcon, 24, 24, 0, NULL, DI_NORMAL);
        DeleteObject(hIcon);
        break;

    case IDC_MAIN_STATE_ICON:
        if(g_APPState.bConnected)
        {
            hIcon = LoadIcon(g_APPUI.hInst, MAKEINTRESOURCE(IDI_STATE_OK));
        }
        else
        {
            hIcon = LoadIcon(g_APPUI.hInst, MAKEINTRESOURCE(IDI_STATE_WARNING));
        }
        DrawIconEx(hDc, (rcBtn.left + rcBtn.right) / 2 - 16,  rcBtn.top + (rcBtn.right - rcBtn.left) / 2 - 16,
                   hIcon, 32, 32, 0, NULL, DI_NORMAL);
        DeleteObject(hIcon);
        break;

    case IDC_MAIN_POLICY_LABEL:
        hIcon = LoadIcon(g_APPUI.hInst, MAKEINTRESOURCE(IDI_POLICY));
        DrawIconEx (hDc, rcBtn.left + 10, (rcBtn.top + rcBtn.bottom) / 2 - 8, 
                    hIcon, 16, 16, 0, NULL, DI_NORMAL);
        DeleteObject(hIcon);
        break;

    case IDC_MAIN_POLICY_REFRESH:
        hIcon = LoadIcon(g_APPUI.hInst, MAKEINTRESOURCE(IDI_REFRESH));
        DrawIconEx (hDc, (rcBtn.left + rcBtn.right) / 2 - 12 , (rcBtn.top + rcBtn.bottom) / 2 - 12, 
                    hIcon, 24, 24, 0, NULL, DI_NORMAL);
        DeleteObject(hIcon);
        break;

    case IDC_MAIN_BRAND_BMP:
        hBmp = LoadBitmap(g_APPUI.hInst, MAKEINTRESOURCE(IDB_BRAND));
        hdcMem = CreateCompatibleDC(hDc);
        hbmOld = (HBITMAP)SelectObject(hdcMem, hBmp);
        GetObject(hBmp, sizeof(bm), &bm);
        pt.x = rcBtn.left;
        pt.y = ((rcBtn.bottom - rcBtn.top) - bm.bmHeight) / 2 + rcBtn.top;
        BitBlt(hDc, pt.x, pt.y, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);
        SelectObject(hdcMem, hbmOld);
        DeleteDC(hdcMem);
        DeleteObject(hBmp);
        break;

    case IDC_MAIN_SETTING:
        hIcon = LoadIcon(g_APPUI.hInst, MAKEINTRESOURCE(IDI_SETTING));
        DrawIconEx(hDc, (rcBtn.left + rcBtn.right) / 2 - 12, (rcBtn.top + rcBtn.bottom) / 2 - 12, 
                   hIcon, 24, 24, 0, NULL, DI_NORMAL);
        DeleteObject(hIcon);
        break;
    
    default:
        break;
    }
    
    return ;

}

// Description:
//   Creates a tooltip for an item in a dialog box. 
// Parameters:
//   iItemID - identifier of an dialog box item.
//   nDlg - window handle of the dialog box.
//   pszText - string to use as the tooltip text.
// Returns:
//   The handle to the tooltip.
HWND doCreateToolTip(HWND hDlg, int iItemID, LPWSTR pszText)
{
    HWND hwndTip;
    HWND hwndTool;

    if (!iItemID || !hDlg || !pszText)
    {
        return FALSE;
    }

    // Get the window of the tool.
    hwndTool = GetDlgItem(hDlg, iItemID);

    //TODO: Create the tooltip.
    hwndTip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
                             WS_POPUP |TTS_ALWAYSTIP | TTS_NOANIMATE,
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             hDlg, NULL, 
                             g_APPUI.hInst, NULL);

    if (!hwndTool || !hwndTip)
    {
        return (HWND)NULL;
    }                              

    // Associate the tooltip with the tool.
    TOOLINFO toolInfo = { 0 };
    toolInfo.cbSize = sizeof(toolInfo);
    toolInfo.hwnd = hDlg;
    toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    toolInfo.uId = (UINT_PTR)hwndTool;
    toolInfo.lpszText = pszText;
    SendMessageW(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);

    return hwndTip;
}



void doMoveMainDialog(void)
{
    int   iCnt;
    HWND  hWndItem;
    RECT  rcMainDlg;
    RECT  rcClient;
    SIZE  szClient;
    POINT ptNewClient;     //Right-Down corner's point of child window control
    LONG  lItemNewTop;
    int   iItemID[] = {IDC_MAIN_BRAND_BMP, IDC_MAIN_VERTICAL, IDC_MAIN_TITLE, IDC_MAIN_SETTING};

    //Change y coordinate of main dialog and setting bar's controls
    GetWindowRect(g_APPUI.hMainDlg, &rcMainDlg);
    GetWindowRect(GetDlgItem(g_APPUI.hMainDlg, IDC_MAIN_SETTING), &rcClient);   //Relative to Screen
    ptNewClient.x = rcClient.left;
    ptNewClient.y = rcClient.top;
    ScreenToClient(g_APPUI.hMainDlg, &ptNewClient);

    if(POPUP_SHOWING == g_iPopupStatus)
    {
        rcMainDlg.top = rcMainDlg.bottom - g_expandedHeight;
        ptNewClient.y += g_lPopupIncrement;
    }
    else if(POPUP_HIDING == g_iPopupStatus)
    {
        rcMainDlg.top = rcMainDlg.bottom - g_expandedHeight + g_lPopupIncrement;  
        ptNewClient.y -= g_lPopupIncrement;
    }
    else
    {
        return ;
    }
    lItemNewTop = ptNewClient.y;

    //Move main window
    szClient.cx   = rcMainDlg.right  - rcMainDlg.left;
    szClient.cy   = rcMainDlg.bottom - rcMainDlg.top;
    MoveWindow(g_APPUI.hMainDlg, rcMainDlg.left, rcMainDlg.top, szClient.cx, szClient.cy, TRUE);


    //Move child window control
    
    for(iCnt = 0; iCnt < sizeof(iItemID) / sizeof(iItemID[0]); iCnt++)
    {
        hWndItem = GetDlgItem(g_APPUI.hMainDlg, iItemID[iCnt]);
        GetWindowRect(hWndItem, &rcClient);   //Relative to Screen
        ptNewClient.x = rcClient.left;
        ScreenToClient(g_APPUI.hMainDlg, &ptNewClient);
        szClient.cx = rcClient.right  - rcClient.left;
        szClient.cy = rcClient.bottom - rcClient.top; 

        HWND hCtrl = GetDlgItem(g_APPUI.hMainDlg, iItemID[iCnt]);
        RECT ctrlRect = { ptNewClient.x, lItemNewTop, ptNewClient.x + szClient.cx, lItemNewTop + szClient.cy };
        MoveWindow(GetDlgItem(g_APPUI.hMainDlg, iItemID[iCnt]), ptNewClient.x, lItemNewTop, szClient.cx, szClient.cy, TRUE);
    }

    GetWindowRect(g_APPUI.hMainDlg, &rcMainDlg);
    InvalidateRect(g_APPUI.hMainDlg, &rcMainDlg, TRUE);
    return ;
}

void onShowingOrHidingPopup(void)
{
    RECT  rcPopupDlg;
    SIZE  szPopupDlg;
    POINT ptNewClient;

    GetWindowRect(g_APPUI.hMainPopupDlg, &rcPopupDlg);
    
    if(POPUP_SHOWING == g_iPopupStatus) {
        rcPopupDlg.top -= POPUP_INCREMENT;  //Move down to hide
    }
    else if(POPUP_HIDING == g_iPopupStatus) {
        rcPopupDlg.top += POPUP_INCREMENT;  //Move up to show
    }
    else {
        return ;
    }

    ptNewClient.x = rcPopupDlg.left;
    ptNewClient.y = rcPopupDlg.top;
    ScreenToClient(g_APPUI.hMainDlg, &ptNewClient);
    szPopupDlg.cx   = rcPopupDlg.right  - rcPopupDlg.left;
    szPopupDlg.cy   = rcPopupDlg.bottom - rcPopupDlg.top;
    MoveWindow(g_APPUI.hMainPopupDlg, ptNewClient.x, ptNewClient.y, szPopupDlg.cx, szPopupDlg.cy, TRUE);

    return ;
}

void onShowingOrHidingPopupStyle2(void)
{
    BOOL  bRes;
    int   iCnt;
    HWND  hWndItem;
    RECT  rcClient;
    RECT  rcMainDlg;
    //HRGN  hRgnMainDlg;
    SIZE  szClient;
    POINT ptNewClient;     //Right-Down corner's point of child window control
    HDWP  hWinPosInfo;
    int   iItemID[] = {IDC_MAIN_BRAND_BMP, IDC_MAIN_VERTICAL, IDC_MAIN_TITLE, IDC_MAIN_SETTING};

    //Move main window
    GetWindowRect(g_APPUI.hMainDlg, &rcClient);
    if(POPUP_SHOW == g_iPopupStatus || POPUP_SHOWING == g_iPopupStatus)
    {
        rcClient.top -= POPUP_INCREMENT; //POPUP_HEIGHT / 40;     //Move down to hide
    }
    else if(POPUP_HIDE == g_iPopupStatus || POPUP_HIDING == g_iPopupStatus)
    {
        rcClient.top += POPUP_INCREMENT; //POPUP_HEIGHT / 40;     //Move up to show
    }

    szClient.cx   = rcClient.right  - rcClient.left;
    szClient.cy   = rcClient.bottom - rcClient.top;
    MoveWindow(g_APPUI.hMainDlg, rcClient.left, rcClient.top, szClient.cx, szClient.cy, TRUE);

    //Move child window control
    hWinPosInfo = BeginDeferWindowPos(sizeof(iItemID) / sizeof(iItemID[0]));   //Initial
    if(NULL == hWinPosInfo)
    {
        MessageBoxW(NULL, L"Initial HDWP failed!", L"ERROR", MB_OK | MB_ICONERROR);
    }
    
    GetClientRect(g_APPUI.hMainDlg, &rcMainDlg);
    for(iCnt = 0; iCnt < sizeof(iItemID) / sizeof(iItemID[0]); iCnt++)
    {
        hWndItem = GetDlgItem(g_APPUI.hMainDlg, iItemID[iCnt]);
        GetWindowRect(hWndItem, &rcClient);   //Relative to Screen
        ptNewClient.x = rcClient.left;
        ptNewClient.y = rcClient.top;
        ScreenToClient(g_APPUI.hMainDlg, &ptNewClient);

        GetWindowRect(GetDlgItem(g_APPUI.hMainDlg, iItemID[iCnt]), &rcClient);  //Relative to Main dialog
        szClient.cx = rcClient.right  - rcClient.left;
        szClient.cy = rcClient.bottom - rcClient.top; 

        if(POPUP_SHOW == g_iPopupStatus || POPUP_SHOWING == g_iPopupStatus)
        {
            ptNewClient.y += POPUP_INCREMENT; // POPUP_HEIGHT / 40;     //Move Setting bar up
        }
        else if(POPUP_HIDE == g_iPopupStatus || POPUP_HIDING == g_iPopupStatus)
        {
            ptNewClient.y -= POPUP_INCREMENT; //POPUP_HEIGHT / 40;     //Move Setting bar down
        }
        
        hWinPosInfo = DeferWindowPos(hWinPosInfo, hWndItem, HWND_TOP, 
                                     ptNewClient.x, ptNewClient.y, szClient.cx, szClient.cy, 
                                     SWP_NOSIZE | SWP_NOREDRAW | SWP_SHOWWINDOW);
        if(NULL == hWinPosInfo)
        {
            MessageBoxW(NULL, L"Add HDWP failed!", L"ERROR", MB_OK | MB_ICONERROR);
        }
    }
    bRes = EndDeferWindowPos(hWinPosInfo);
    if(NULL == bRes)
    {
        MessageBoxW(NULL, L"EndDeferWindowPos failed!", L"ERROR", MB_OK | MB_ICONERROR);
    }
    
    return ;
}

//processing function when click refresh button
void onBtnRefreshClicked(void)
{
    EngineUpdatePolicy();
}

//processing function when click setting button
void onBtnSettingClicked(void)
{
    switch(g_iPopupStatus)
    {
    case POPUP_SHOW:
        g_iPopupStatus = POPUP_HIDING;
        g_bMouseOverButton = FALSE;
        ::AnimateWindow(g_APPUI.hMainPopupDlg, 1000, AW_VER_POSITIVE | AW_SLIDE | AW_HIDE);
        doMoveMainDialog();
        g_iPopupStatus = POPUP_HIDE;
        ShowWindow(g_APPUI.hMainPopupDlg, SW_HIDE);
        break;

    case POPUP_SHOWING:
        ; // Do nothing
        break;

    case POPUP_HIDE:
        g_iPopupStatus = POPUP_SHOWING;
        g_bMouseOverButton = FALSE;
        doMoveMainDialog();
        ::AnimateWindow(g_APPUI.hMainPopupDlg, 1000, AW_VER_NEGATIVE | AW_SLIDE | AW_ACTIVATE);
        g_iPopupStatus = POPUP_SHOW;
        break;

    case POPUP_HIDING:
        ; // Do nothing
        break;

    default:
        break;
    }
}

HMODULE LoadResourceModule()
{
    HMODULE hMod = NULL;
    WCHAR   wzPath[MAX_PATH] = {0};
    std::wstring wsMod1;
    std::wstring wsMod2;

    //TODO: Load Resource DLL
    if(!GetModuleFileNameW(NULL, wzPath, MAX_PATH)) {
        return NULL;
    }

    WCHAR* pos = wcsrchr(wzPath, L'\\');

    if(NULL == pos) {
        return ::LoadLibraryW(RESFILENAME);
    }

    *pos = L'\0';
    wsMod1 = wzPath;
    wsMod1 += L"\\";
    wsMod1 += RESFILENAME;

    pos = wcsrchr(wzPath, L'\\');
    if(NULL != pos) {
        *(pos+1) = L'\0';
        wsMod2 = wzPath;
        wsMod2 += RESFILENAME2;
    }

    hMod = ::LoadLibraryW(wsMod1.c_str());
    if(NULL == hMod && !wsMod2.empty()) {
        hMod = ::LoadLibraryW(wsMod2.c_str());
    }

    return hMod;
}

void QuitMain()
{
    g_APPState.bActive = FALSE;
    ::PostMessageW(g_APPUI.hMainDlg, WM_DESTROY, 0, 0);
}

VOID SetPolicyTimestamp(const std::wstring& time)
{
    if (time.empty()) {
        return;
    }
    wcsncpy_s(g_APPState.wzPolicyCreatedTime, 256, time.c_str(), _TRUNCATE);
    std::wstring wsTipFmt = nudf::util::res::LoadMessage(g_APPUI.hResDll, (DWORD)IDS_TRAY_TIP, 1024, g_APPState.dwLangId);
    swprintf_s(g_APPUI.nid.szTip, 128, wsTipFmt.c_str(), g_APPState.wzPolicyCreatedTime, g_APPState.wzCurrentUser);
    Shell_NotifyIcon(NIM_MODIFY, &g_APPUI.nid);
    wcsncpy_s(g_APPState.wzPolicyVersion, 256, g_APPState.wzPolicyCreatedTime, _TRUNCATE);    
    SendMessageW(GetDlgItem(g_APPUI.hMainDlg,IDC_MAIN_POLICY_VERSION), WM_SETTEXT, NULL, (LPARAM)g_APPState.wzPolicyVersion);
    RedrawWindow(GetDlgItem(g_APPUI.hMainDlg, IDC_MAIN_POLICY_VERSION), NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
}

VOID SetPolicyTimestamp(_In_ const FILETIME* timestamp)
{
    FILETIME    ft;
    SYSTEMTIME  st;

    if(timestamp->dwHighDateTime == 0 && timestamp->dwLowDateTime == 0) {
        wsprintfW(g_APPState.wzPolicyCreatedTime, L"%s", L"N/A");
    }
    else {
        FileTimeToLocalFileTime(timestamp, &ft);
        FileTimeToSystemTime(&ft, &st);
        wsprintfW(g_APPState.wzPolicyCreatedTime, L"%04d-%02d-%02d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
        // Change tray-icon tip
        std::wstring wsTipFmt = nudf::util::res::LoadMessage(g_APPUI.hResDll, (DWORD)IDS_TRAY_TIP, 1024, g_APPState.dwLangId);
        swprintf_s(g_APPUI.nid.szTip, 128, wsTipFmt.c_str(), g_APPState.wzPolicyCreatedTime, g_APPState.wzCurrentUser);
        Shell_NotifyIcon(NIM_MODIFY, &g_APPUI.nid);
    }
    wcsncpy_s(g_APPState.wzPolicyVersion, 256, g_APPState.wzPolicyCreatedTime, _TRUNCATE);
    
    SendMessageW(GetDlgItem(g_APPUI.hMainDlg,IDC_MAIN_POLICY_VERSION), WM_SETTEXT, NULL, (LPARAM)g_APPState.wzPolicyVersion);
    RedrawWindow(GetDlgItem(g_APPUI.hMainDlg, IDC_MAIN_POLICY_VERSION), NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
}

void StopRotatingUpdateIndicator()
{
    
    KillTimer(g_APPUI.hMainDlg, (UINT_PTR)ROTATE_REFRESH_TIMER);
    HWND hbtnWnd = GetDlgItem(g_APPUI.hMainDlg, IDC_MAIN_POLICY_REFRESH);
    InvalidateRect(hbtnWnd, NULL, TRUE); //Force to repaint after rotate
    g_updateIconRotate = NX_ICON_RT_STATE_STOPPED;

}

void SetIconStopRotatingFlag()
{
    if (g_updateIconRotate == NX_ICON_RT_STATE_ROTATING)
    {
        g_updateIconRotate = NX_ICON_RT_STATE_STOPPING;
    }
}

BOOL IsDwmCompositionEnabled()
{
    BOOL enabled = FALSE;
    HRESULT hr = DwmIsCompositionEnabled(&enabled);
    if (FAILED(hr)) {
        enabled = FALSE;
    }
    return enabled;
}

VOID UpdateDwmCompositionStatus()
{
    bool enabled = IsDwmCompositionEnabled() ? true : false;
    nxrm::tray::serv_control sc;
    sc.set_aero_status(enabled);
}


VOID SetUpdateTimestamp(const std::wstring& time)
{
    if (time.empty()) {
        return;
    }
    wcsncpy_s(g_APPState.wzLastUpdateTime, 256, time.c_str(), _TRUNCATE);
    SendMessageW(GetDlgItem(g_APPUI.hMainDlg, IDC_MAIN_LAST_UPDATE_TIME), WM_SETTEXT, NULL, (LPARAM)g_APPState.wzLastUpdateTime);
    RedrawWindow(GetDlgItem(g_APPUI.hMainDlg, IDC_MAIN_LAST_UPDATE_TIME), NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
}

VOID SetUpdateTimestamp(_In_ const FILETIME* timestamp)
{
    FILETIME    ft;
    SYSTEMTIME  st;

    if(timestamp->dwHighDateTime == 0 && timestamp->dwLowDateTime == 0) {
        wsprintfW(g_APPState.wzLastUpdateTime, L"%s", L"N/A");
    }
    else {
        FileTimeToLocalFileTime(timestamp, &ft);
        FileTimeToSystemTime(&ft, &st);
        const WCHAR* pwzMon = L"???";
        switch(st.wMonth)
        {
        case 1: pwzMon = L"Jan"; break;
        case 2: pwzMon = L"Feb"; break;
        case 3: pwzMon = L"Mar"; break;
        case 4: pwzMon = L"Apr"; break;
        case 5: pwzMon = L"May"; break;
        case 6: pwzMon = L"Jun"; break;
        case 7: pwzMon = L"Jul"; break;
        case 8: pwzMon = L"Aug"; break;
        case 9: pwzMon = L"Sep"; break;
        case 10: pwzMon = L"Oct"; break;
        case 11: pwzMon = L"Nov"; break;
        case 12: pwzMon = L"Dec"; break;
        default: pwzMon = L"???"; break;
        }
        LPCWSTR sInfo = L"AM";
        if(st.wHour > 12) {
            st.wHour -= 12;
            sInfo = L"PM";
        }
        wsprintfW(g_APPState.wzLastUpdateTime, L"%02d %s %04d, %02d:%02d %s", st.wDay, pwzMon, st.wYear, st.wHour, st.wMinute, sInfo);
    }
    
    SendMessageW(GetDlgItem(g_APPUI.hMainDlg, IDC_MAIN_LAST_UPDATE_TIME), WM_SETTEXT, NULL, (LPARAM)g_APPState.wzLastUpdateTime);
    RedrawWindow(GetDlgItem(g_APPUI.hMainDlg, IDC_MAIN_LAST_UPDATE_TIME), NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
}


static void InsertLogonMenu(bool logged_on)
{
    MENUITEMINFO    mii = { 0 };
    memset(&mii, 0, sizeof(MENUITEMINFO));
    mii.cbSize = sizeof(MENUITEMINFO);
    BOOL exist = GetMenuItemInfoW(g_APPUI.hTrayMenu, TRAY_MENU_LOGINOUT_ID, FALSE, &mii);
    if (exist) {
        return;
    }

    std::wstring wsMenuText = nudf::util::res::LoadMessage(g_APPUI.hResDll, logged_on ? IDS_TRAY_MENU_LOGOUT : IDS_TRAY_MENU_LOGIN, 1024);
    if (wsMenuText.empty()) {
        wsMenuText = logged_on ? L"Logout" : L"Log in ...";
    }
    AppendMenuW(g_APPUI.hTrayMenu, MF_STRING, TRAY_MENU_LOGINOUT_ID, wsMenuText.c_str());
    InsertMenuW(g_APPUI.hTrayMenu, 5, MF_SEPARATOR, NULL, NULL);
}

static void UpdateLogonMenu(bool logged_on)
{
    std::wstring wsMenuText = nudf::util::res::LoadMessage(g_APPUI.hResDll, logged_on ? IDS_TRAY_MENU_LOGOUT : IDS_TRAY_MENU_LOGIN, 1024);
    if (wsMenuText.empty()) {
        wsMenuText = logged_on ? L"Logout" : L"Log in ...";
    }
    MENUITEMINFO    mii = { 0 };
    memset(&mii, 0, sizeof(MENUITEMINFO));
    mii.cbSize = sizeof(MENUITEMINFO);
    BOOL exist = GetMenuItemInfoW(g_APPUI.hTrayMenu, TRAY_MENU_LOGINOUT_ID, FALSE, &mii);
    if (exist) {
        ModifyMenuW(g_APPUI.hTrayMenu, TRAY_MENU_LOGINOUT_ID, MF_STRING | MF_BYCOMMAND, TRAY_MENU_LOGINOUT_ID, wsMenuText.c_str());
    }
    else {
        AppendMenuW(g_APPUI.hTrayMenu, MF_STRING, TRAY_MENU_LOGINOUT_ID, wsMenuText.c_str());
        InsertMenuW(g_APPUI.hTrayMenu, 5, MF_SEPARATOR, NULL, NULL);
    }
}

static void RemoveLogonMenu()
{
    MENUITEMINFO    mii = { 0 };
    memset(&mii, 0, sizeof(MENUITEMINFO));
    mii.cbSize = sizeof(MENUITEMINFO);
    BOOL exist = GetMenuItemInfoW(g_APPUI.hTrayMenu, TRAY_MENU_LOGINOUT_ID, FALSE, &mii);
    if (!exist) {
        return;
    }
    DeleteMenu(g_APPUI.hTrayMenu, TRAY_MENU_LOGINOUT_ID, MF_BYCOMMAND);
    DeleteMenu(g_APPUI.hTrayMenu, 5, MF_BYPOSITION);
}

VOID UpdateTrayMenu(bool debug_on, bool external_authn, const std::wstring& logon_user)
{
    if (NULL == g_APPUI.hTrayMenu) {
        return;
    }

    if (g_APPState.bIsDebugOn != debug_on) {
        g_APPState.bIsDebugOn = debug_on;
        std::wstring wsMenuText = nudf::util::res::LoadMessage(g_APPUI.hResDll, debug_on ? IDS_TRAY_MENU_DISABLE_DEBUG : IDS_TRAY_MENU_ENABLE_DEBUG, 1024);
        if (wsMenuText.empty()) {
            wsMenuText = debug_on ? L"Disable Debug" : L"Enable Debug";
        }
        ModifyMenuW(g_APPUI.hTrayMenu, TRAY_MENU_ENABLE_DEBUG_ID, MF_STRING | MF_BYCOMMAND, TRAY_MENU_ENABLE_DEBUG_ID, wsMenuText.c_str());
    }

    if (g_APPState.bRemoteAuthn != external_authn) {
        g_APPState.bRemoteAuthn = external_authn;
        if (g_APPState.bRemoteAuthn) {
            // from local to remote logon
            // insert menu
            bool logged_on = (!logon_user.empty() && 0 != _wcsicmp(logon_user.c_str(), L"N/A"));
            g_APPState.bLoggedOn = logged_on;
            UpdateLogonMenu(logged_on);
        }
        else {
            // from remote to local logon
            // remove menu
            RemoveLogonMenu();
        }
    }
    else {
        if (g_APPState.bRemoteAuthn) {
            bool logged_on = (!logon_user.empty() && 0 != _wcsicmp(logon_user.c_str(), L"N/A"));
            if (g_APPState.bLoggedOn != logged_on) {
                g_APPState.bLoggedOn = logged_on;
                UpdateLogonMenu(logged_on);
                if (!logged_on) {
                    // Let user know that his/her authn expired
                    std::wstring title = nudf::util::res::LoadMessage(g_APPUI.hResDll, IDS_PRODUCT_NAME, 256, LANG_NEUTRAL, L"NextLabs Rights Management");
                    std::wstring msg = nudf::util::res::LoadMessage(g_APPUI.hResDll, IDS_NOTIFY_LOGON_EXPIRED, 1024, LANG_NEUTRAL, L"Current session expired, please logon again.");
                    doShowTrayMsg(title.c_str(), msg.c_str());
                }
            }
        }
    }
}

VOID ProtectOfficeAddinKeys()
{
#ifdef _AMD64_
    ::RegDeleteKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Office\\Word\\Addins\\NxRMAddin", KEY_WOW64_32KEY, 0);
    ::RegDeleteKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Office\\Excel\\Addins\\NxRMAddin", KEY_WOW64_32KEY, 0);
    ::RegDeleteKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Office\\PowerPoint\\Addins\\NxRMAddin", KEY_WOW64_32KEY, 0);
    ::RegDeleteKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Office\\Word\\Addins\\NxRMAddin", KEY_WOW64_64KEY, 0);
    ::RegDeleteKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Office\\Excel\\Addins\\NxRMAddin", KEY_WOW64_64KEY, 0);
    ::RegDeleteKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Office\\PowerPoint\\Addins\\NxRMAddin", KEY_WOW64_64KEY, 0);
#else
    ::RegDeleteKeyW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Office\\Word\\Addins\\NxRMAddin");
    ::RegDeleteKeyW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Office\\Excel\\Addins\\NxRMAddin");
    ::RegDeleteKeyW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Office\\PowerPoint\\Addins\\NxRMAddin");
#endif
}