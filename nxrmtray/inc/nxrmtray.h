

#ifndef __NXRMMAIN_H__
#define __NXRMMAIN_H__

#include <Shellapi.h>

#define TAB_NUM              4
#define MAX_WZMSGLEN         256
#define MAX_USEREMAIL        256
#define MAX_STRLEN           256
#define MAX_SERVER_NUM       8
#define MAX_AUTHPROFILE_NUM  8

#define MAINDLG_MOVABLE_HEIGHT    80          // The height from top when Right click can to move window
#define MAINGLG_STATUSBAR_HEIGHT  100
#define MAINDLG_POLICYBAR_HEIGHT  40
#define MAINDLG_UPDATEBAR_HEIGHT  30
#define MAINDLG_SETTINGBAR_HEIGHT 35

#define TABBN_UNSELECTED_HEIGTH  35           // when tab menu is selected
#define TABBN_SELECTED_HEIGTH    45           // when tab menu is unselected
#define TABBN_WITDH              80           // Tab button's width
#define TABBN_SPACE              5            // Space in two button
#define TABBN_COORDINATE_LEFT    20           // Coordinate of the first TABBN
#define TABBN_COORDINATE_BOTTOM  90           //

#define DLG_MARGIN_LEFT          20           // DLG_MARGIN_ is used to determine the location of Dialog windows
#define DLG_MARGIN_TOP           TABBN_COORDINATE_BOTTOM
#define DLG_MARGIN_RIGHT         20    
#define DLG_MARGIN_BOTTOM        20

#define STATEBAR_HEIGTH          30           // Height of status bar in the top of main dialog windows


// Global Variable struct
typedef struct tagAPPUI {
    HWND           hMainDlg;                      // Handle of main Dialog
    NOTIFYICONDATA nid;                           // Tray icon
    HMENU          hTrayMenu;                     // Tray Right click Menu
    HINSTANCE      hInst;                         // Handle of current instance
    HWND           hTab;                          // Handle of Tab control
    HWND           hMainPopupDlg;                 // Handle of Tabs Dialog in Main Dialog
    HWND		   hHtmlNotifyDlg;				// Handle of Notify Dlg for Customized HTML info	NEW ADDED 0518
    HMODULE        hResDll;                       // HMODULE of nxrmres.dll
    RECT           rcPopupDlg;                    // Store the RECT of pop up dialog, relative to MainDialog's client
} APPUI, *PAPPUI;

//add or remove items for UI display
typedef struct tagAPPState {
    DWORD      dwLangId;                      // Language ID
    bool       bConnected;                    // The result of test connection or not
    BOOL       bActive;
    //SETTING:GENERAL
    BOOL       bAutoRun;                      // Start RMC on system start or not
    BOOL       bDeskTopMsg;                   // Show desktop Notification or not
    BOOL       bPopupNotify;                  //
    WCHAR      wzServerList[MAX_SERVER_NUM][MAX_STRLEN];  //ICENET
    WCHAR      wzSelectedServer[MAX_STRLEN];  //Selected Server
    //SETTING:ACCOUNT
    bool       bLoggedOn;
    WCHAR      wzCurLoginUser[MAX_USEREMAIL]; // Current log in user
    BOOL       bRememberUserName;             // Remember login user or not
    BOOL       bPromptUser;                   // Prompt user or not
    WCHAR      wzAuthProfile[MAX_AUTHPROFILE_NUM][MAX_STRLEN];  //Authentication profile
    WCHAR      wzSelectedAuth[MAX_STRLEN];    //Selected Authentication profile
    //SETTING:ABOUT
    WCHAR      wzAppVersion[MAX_STRLEN];      // The version of application
    WCHAR      wzCopyRight[MAX_STRLEN];       // 
    //POLICY
    BOOL       bPoliyStatus;                    // the policy is lasted or not
    bool       bIsDebugOn;                      // Is Debug On?
    bool	   bIconOn;							// Icon has been added to taskbar
    bool       bRemoteAuthn;
    WCHAR      wzCurrentServer[64];             // Policy created time
    WCHAR      wzCurrentUser[64];               // Policy created time
    WCHAR      wzCurrentUserId[64];             // Policy created time
    WCHAR      wzPolicyCreatedTime[MAX_STRLEN]; // Policy created time
    WCHAR      wzLastUpdateTime[MAX_STRLEN];    // Policy last update time
    WCHAR      wzPolicyVersion[MAX_STRLEN];     // Policy version


} APPSTATE, *PAPPSTATE;


//custom tray message 
#define WM_TRAY             (WM_USER + 100)
#define WM_TABBNMOUSEMOVE   (WM_USER + 110)

//Tray IDni
enum UI_ID
{
    //Tray's ID
    TRAY_ID = 1
};

//Tray Right click menu's Message ID
enum TARY_MENU
{
    TRAY_MENU_OPEN_RMC_ID = 1,
    TRAY_MENU_UPDATE_POLICY_ID,
    TRAY_MENU_ENABLE_DEBUG_ID,
    TRAY_MENU_COLLECT_LOGS_ID,
    TRAY_MENU_LOGINOUT_ID
};

//About pop up
#define POPUP_HEIGHT          120
#define POPUP_INCREMENT       (POPUP_HEIGHT / POPUP_COUNT)
#define POPUP_COUNT           60

// Update Timer
#define KEEPALIVE_TIMER             (0x02)
#define KEEPALIVE_ELAPSE_SHORT      5000        // 50 seconds
#define KEEPALIVE_ELAPSE            30000       // 30 seconds

// Key Protection Timer
#define KEYPROTECT_TIMER         (0x03)
#define KEYPROTECT_ELAPSE        300000     // 5 minutes


// Update Timer
#define ROTATE_REFRESH_TIMER            (0x04)
#define ROTATE_REFRESH_TIME				50
#define ROTATE_ANGLE_UNIT				15 //This number need to be factor of 360. 

// Show tray Icon timer
#define SHOW_TRAY_ICON_TIMER		 (0X05)
#define SHOW_TRAY_ICON_TIME		 1000

#include "clientipc.hpp"

//The Status of pop up the setting menu
enum POPUP_STATUS
{
    POPUP_SHOW = 1,
    POPUP_SHOWING,
    POPUP_HIDE,
    POPUP_HIDING
};


void SetIconStopRotatingFlag();

void doShowTrayMsg(_In_ LPCWSTR wzTitle, _In_ LPCWSTR wzInfo);
VOID UpdateServerStatus(const nxrm::tray::serv_status& st);



#endif  // __NXRMMAIN_H__