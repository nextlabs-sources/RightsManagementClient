
#include <SDKDDKVer.h>
#include <Windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <winioctl.h>
#include <WinCred.h>
#include <Objbase.h>
#include <Ntsecapi.h>

#include <sstream>

#include <nudf\shared\enginectl.h>
#include <nudf\exception.hpp>
#include <nudf\shared\logdef.h>
#include <nudf\string.hpp>
#include <nudf\asyncpipe.hpp>
#include <nudf\nxrmres.h>
#include <nudf\resutil.hpp>
#include <nudf\crypto.hpp>

#include "nxrmtray.h"
#include "clientipc.hpp"
#include "EngineUtils.h"



// *************************************************************************
//
//  Define Global Variables
//
// *************************************************************************
extern nxrm::tray::serv_status g_serv_status;
extern APPUI        g_APPUI;                           // Global Variables (most are handles of control )
extern APPSTATE     g_APPState;                        // Used to initial the program.(about UI or create UI)
extern nxrm::tray::logon_ui_flag g_logon_ui_flag;


BOOL EngineGetStatus()
{
    BOOL bRet = TRUE;

    if (!g_APPState.bActive) {
        return TRUE;
    }

    try {

        nxrm::tray::serv_control sc;
        nxrm::tray::serv_status st = sc.ctl_query_status();

        // update server status
        g_serv_status = st;
        UpdateServerStatus(st);
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        bRet = FALSE;
    }

    return bRet;
}

void EngineUpdatePolicy()
{
    try {
        std::wstring title = nudf::util::res::LoadMessage(g_APPUI.hResDll, IDS_PRODUCT_NAME, 1024, g_APPState.dwLangId, L"NextLabs Rights Management");
        std::wstring msg = nudf::util::res::LoadMessage(g_APPUI.hResDll, IDS_NOTIFY_UPDATING_POLICY, 1024, g_APPState.dwLangId, L"Checking for latest policy...");
        doShowTrayMsg(title.c_str(), msg.c_str());
        nxrm::tray::serv_control sc;
        sc.ctl_update_policy();
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
    }
}

void EngineEnableDebug(BOOL bEnable)
{
    try {
        nxrm::tray::serv_control sc;
        sc.ctl_enable_debug(bEnable?true:false);

        g_APPState.bIsDebugOn = bEnable ? true : false;
        std::wstring wsMenuText = nudf::util::res::LoadMessage(g_APPUI.hResDll, bEnable ? IDS_TRAY_MENU_DISABLE_DEBUG : IDS_TRAY_MENU_ENABLE_DEBUG, 1024);
        if (wsMenuText.empty()) {
            wsMenuText = bEnable ? L"Disable Debug" : L"Enable Debug";
        }
        ModifyMenuW(g_APPUI.hTrayMenu, TRAY_MENU_ENABLE_DEBUG_ID, MF_STRING | MF_BYCOMMAND, TRAY_MENU_ENABLE_DEBUG_ID, wsMenuText.c_str());
    }
    catch (const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
    }
}

void EngineCollectLog()
{
    try {
        nxrm::tray::serv_control sc;
        sc.ctl_collect_debug();
    }
    catch (const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
    }
}


BOOL GetAuthnPkgId(PULONG pid)
{
	LONG status = 0;
	HANDLE h = NULL;
	ULONG ulAuthnPackage = 0;
	LSA_STRING sAuthnPkgName = { 0 };

	*pid = 0;

	status = LsaConnectUntrusted(&h);
	if (0 != status) {
		return FALSE;
	}

#ifndef NTLMSP_NAME_A
#define NTLMSP_NAME_A            "NTLM"
#define NTLMSP_NAME              L"NTLM"        
#endif
	sAuthnPkgName.Buffer = NTLMSP_NAME_A;
	sAuthnPkgName.Length = (USHORT)strlen(NTLMSP_NAME_A);
	sAuthnPkgName.MaximumLength = sAuthnPkgName.Length;
	status = LsaLookupAuthenticationPackage(h, &sAuthnPkgName, pid);
	LsaDeregisterLogonProcess(h); 
	h = NULL;

	if (0 != status) {
		*pid = 0;
		return FALSE;
	}

	return TRUE;
}


void EngineLogIn(LPCWSTR pwzDefaultDomain/* = NULL*/)
{
    nxrm::tray::logon_ui_flag_guard guard(&g_logon_ui_flag);

    assert(!g_APPState.bLoggedOn);

    DWORD           dwResult = 0;
    CREDUI_INFOW    UiInfo;
    DWORD           dwAuthError = 0;
	ULONG			ulPkgNo = 0;
    ULONG           ulAuthPackage = GetAuthnPkgId(&ulPkgNo) ? ulPkgNo : 0;
    LPVOID          pvInAuthBuffer = NULL;
    ULONG           ulInAuthBufferSize = 0;
    LPVOID          pvOutAuthBuffer = NULL;
    ULONG           ulOutAuthBufferSize = 0;
    BOOL            fSave = FALSE;
    DWORD           dwFlags = CREDUIWIN_AUTHPACKAGE_ONLY;


    std::wstring default_user;
    std::wstring default_domain;

    std::wstring title = nudf::util::res::LoadMessage(g_APPUI.hResDll, IDS_PRODUCT_NAME, 1024, g_APPState.dwLangId, L"NextLabs Rights Management");
    std::wstring msg = nudf::util::res::LoadMessage(g_APPUI.hResDll, IDS_NOTIFY_USER_LOGON_FAILURE, 1024, g_APPState.dwLangId, L"User (%s) was unable to log in due to error (%d: %s)");
    std::wstring error_msg;

    if (NULL != pwzDefaultDomain && L'\0' != pwzDefaultDomain[0]) {
        default_domain = pwzDefaultDomain;
        default_domain += L"\\";
    }
    else {
        try {
            nxrm::tray::serv_control sc;
            sc.query_default_authn_info(default_user, default_domain);
            if (!default_domain.empty()) {
                default_domain += L"\\";
            }
        }
        catch (std::exception& e) {
            UNREFERENCED_PARAMETER(e);
        }
    }

    try {

        WCHAR   wzUserName[1024] = { 0 };
        DWORD   dwMaxUserName = 1024;
        WCHAR   wzDomainName[1024] = { 0 };
        DWORD   dwMaxDomainName = 1024;
        WCHAR   wzPassword[1024] = { 0 };
        DWORD   dwMaxPassword = 1024;
        unsigned char pbDefaultCred[1024];

        std::wstring user_name;
        std::wstring domain_name;


    _retry:
        memset(&UiInfo, 0, sizeof(UiInfo));
        UiInfo.cbSize = sizeof(UiInfo);
        UiInfo.hwndParent = NULL;
        UiInfo.pszMessageText = L"Log in with your Rights Management credentials.";
        UiInfo.pszCaptionText = L"NextLabs Rights Management";
        UiInfo.hbmBanner = NULL;

        std::wstring default_logon_info = default_domain + user_name;

        memset(pbDefaultCred, 0, 1024);
        ulInAuthBufferSize = 1024;
        if (!CredPackAuthenticationBufferW(0, //CRED_PACK_GENERIC_CREDENTIALS,
            default_logon_info.empty() ? L"" : default_logon_info.c_str(),
            L"",
            pbDefaultCred,
            &ulInAuthBufferSize
            )) {
            ulInAuthBufferSize = 0;
        }

        dwMaxUserName = 1024;
        dwMaxDomainName = 1024;
        dwMaxPassword = 1024;
        dwResult = CredUIPromptForWindowsCredentialsW(&UiInfo,
            dwAuthError,
            &ulAuthPackage,
            (0 == ulInAuthBufferSize) ? NULL : pbDefaultCred,
            ulInAuthBufferSize,
            &pvOutAuthBuffer,
            &ulOutAuthBufferSize,
            &fSave,
            dwFlags
            );
        if (dwResult == ERROR_CANCELLED) {
            throw dwResult;
        }

        if (0 != dwResult) {
            throw dwResult;
        }

        if (!CredUnPackAuthenticationBufferW(CRED_PACK_PROTECTED_CREDENTIALS,
            pvOutAuthBuffer,
            ulOutAuthBufferSize,
            wzUserName,
            &dwMaxUserName,
            wzDomainName,
            &dwMaxDomainName,
            wzPassword,
            &dwMaxPassword))
        {
            throw (dwResult = GetLastError());
        }

        CoTaskMemFree(pvOutAuthBuffer);
        pvOutAuthBuffer = NULL;

        //
        // Good, tell service to send logon request to RMS
        //
        try {

            std::wstring full_logon_name(wzUserName);
            std::wstring password(wzPassword);

            std::wstring::size_type pos = full_logon_name.find_first_of(L"@\\");
            if (pos == std::wstring::npos) {
                user_name = full_logon_name;
                domain_name = wzDomainName;
            }
            else {
                if (full_logon_name.at(pos) == L'@') {
                    // in format: user@full_domain_name
                    user_name = full_logon_name.substr(0, pos);
                    domain_name = full_logon_name.substr(pos+1);
                }
                else {
                    // in format: domain\user
                    domain_name = full_logon_name.substr(0, pos);
                    user_name = full_logon_name.substr(pos+1);
                }
            }

            if (user_name.empty()) {
                dwResult = ERROR_NOT_AUTHENTICATED;
                error_msg = nudf::util::res::LoadMessage(g_APPUI.hResDll, IDS_NOTIFY_LOGON_ERROR_EMPTY_NAME, 1024, g_APPState.dwLangId, L"empty user name");
                msg = nudf::util::res::LoadMessageEx(g_APPUI.hResDll, IDS_NOTIFY_USER_LOGON_FAILURE, 1024, g_APPState.dwLangId, L"User %s was unable to log in due to error %d (%s)", L"", ERROR_NOT_AUTHENTICATED, error_msg.c_str());
                doShowTrayMsg(title.c_str(), msg.c_str());
                // doShowTrayMsg(L"NextLabs Rights Management", L"Logon failed because of empty password");
                throw WIN32ERROR2(dwResult);
            }
            if (password.empty()) {
                dwResult = ERROR_NOT_AUTHENTICATED;
                error_msg = nudf::util::res::LoadMessage(g_APPUI.hResDll, IDS_NOTIFY_LOGON_ERROR_EMPTY_PASSWORD, 1024, g_APPState.dwLangId, L"empty password");
                msg = nudf::util::res::LoadMessageEx(g_APPUI.hResDll, IDS_NOTIFY_USER_LOGON_FAILURE, 1024, g_APPState.dwLangId, L"User %s was unable to log in due to error %d (%s)", user_name.c_str(), ERROR_NOT_AUTHENTICATED, error_msg.c_str());
                doShowTrayMsg(title.c_str(), msg.c_str());
                throw WIN32ERROR2(dwResult);
            }

            nxrm::tray::serv_control sc;
            if (sc.ctl_login(user_name, domain_name, password, &dwResult) && 0 == dwResult) {
                //
                // Good, now user has logged on
                //
                g_APPState.bLoggedOn = TRUE;
                std::wstring wsMenuText = nudf::util::res::LoadMessage(g_APPUI.hResDll, IDS_TRAY_MENU_LOGOUT, 1024);
                if (wsMenuText.empty()) {
                    wsMenuText = L"Log out";
                }
                ModifyMenuW(g_APPUI.hTrayMenu, TRAY_MENU_LOGINOUT_ID, MF_BYCOMMAND | MF_STRING, TRAY_MENU_LOGINOUT_ID, wsMenuText.c_str());
                EngineGetStatus();
            }
            else {
                switch (dwResult)
                {
                case ERROR_INVALID_PASSWORD:
                    error_msg = nudf::util::res::LoadMessage(g_APPUI.hResDll, IDS_NOTIFY_LOGON_ERROR_INVALID_PASSWORD, 1024, g_APPState.dwLangId, L"wrong password");
                    msg = nudf::util::res::LoadMessageEx(g_APPUI.hResDll, IDS_NOTIFY_USER_LOGON_FAILURE, 1024, g_APPState.dwLangId, L"User %s was unable to log in due to error %d (%s)", user_name.c_str(), dwResult, error_msg.c_str());
                    doShowTrayMsg(title.c_str(), msg.c_str()); 
                    break;
                case ERROR_NOT_AUTHENTICATED:
                    error_msg = nudf::util::res::LoadMessage(g_APPUI.hResDll, IDS_NOTIFY_LOGON_ERROR_NOT_AUTHENTICATED, 1024, g_APPState.dwLangId, L"authentication failed - wrong user name or password");
                    msg = nudf::util::res::LoadMessageEx(g_APPUI.hResDll, IDS_NOTIFY_USER_LOGON_FAILURE, 1024, g_APPState.dwLangId, L"User %s was unable to log in due to error %d (%s)", user_name.c_str(), dwResult, error_msg.c_str());
                    doShowTrayMsg(title.c_str(), msg.c_str());
                    break;
                case ERROR_INVALID_DOMAINNAME:
                    error_msg = nudf::util::res::LoadMessage(g_APPUI.hResDll, IDS_NOTIFY_LOGON_ERROR_INVALID_DOMAIN, 1024, g_APPState.dwLangId, L"unrecognized domain name");
                    msg = nudf::util::res::LoadMessageEx(g_APPUI.hResDll, IDS_NOTIFY_USER_LOGON_FAILURE, 1024, g_APPState.dwLangId, L"User %s was unable to log in due to error %d (%s)", user_name.c_str(), dwResult, error_msg.c_str());
                    doShowTrayMsg(title.c_str(), msg.c_str());
                    break;
                default:
                    error_msg = nudf::util::res::LoadMessage(g_APPUI.hResDll, IDS_NOTIFY_LOGON_ERROR_UNKNOWN, 1024, g_APPState.dwLangId, L"unknown");
                    msg = nudf::util::res::LoadMessageEx(g_APPUI.hResDll, IDS_NOTIFY_USER_LOGON_FAILURE, 1024, g_APPState.dwLangId, L"User %s was unable to log in due to error %d (%s)", user_name.c_str(), dwResult, error_msg.c_str());
                    doShowTrayMsg(title.c_str(), msg.c_str());
                    break;
                }
            }
        }
        catch (const nudf::CException& e) {
            UNREFERENCED_PARAMETER(e);
            dwResult = e.GetCode();
        }

        // If the password is not correct
        // Pop up cred dialog box again
        if (dwResult == ERROR_INVALID_DOMAINNAME || dwResult == ERROR_INVALID_PASSWORD || dwResult == ERROR_NOT_AUTHENTICATED) {
            // goto re-try
            dwAuthError = dwResult;
            goto _retry;
        }
    }
    catch (DWORD dwErr) {
        UNREFERENCED_PARAMETER(dwErr);
        ; // Nothing
    }
	catch (...) {
        ; // Nothing
	}

    if (NULL != pvOutAuthBuffer) {
        CoTaskMemFree(pvOutAuthBuffer);
        pvOutAuthBuffer = NULL;
    }
}

void EngineLogOut()
{
    nxrm::tray::logon_ui_flag_guard guard(&g_logon_ui_flag);
    try {
        nxrm::tray::serv_control sc;
        sc.ctl_logout();
        // Change tray-icon tip
        g_APPState.bLoggedOn = FALSE;
        wcsncpy_s(g_APPState.wzCurrentUser, 255, L"N/A", _TRUNCATE);
        g_APPState.wzCurrentUserId[0] = L'\0';
        std::wstring wsTipFmt = nudf::util::res::LoadMessage(g_APPUI.hResDll, (DWORD)IDS_TRAY_TIP, 1024, g_APPState.dwLangId);
        std::wstring wsTip;
        swprintf_s(nudf::string::tempstr<wchar_t>(wsTip, 1024), 1023, wsTipFmt.c_str(), g_APPState.wzPolicyCreatedTime, g_APPState.wzCurrentUser);
        wcsncpy_s(g_APPUI.nid.szTip, 127, wsTip.c_str(), _TRUNCATE);
        Shell_NotifyIcon(NIM_MODIFY, &g_APPUI.nid);

        std::wstring wsMenuText = nudf::util::res::LoadMessage(g_APPUI.hResDll, IDS_TRAY_MENU_LOGIN, 1024);
        if (wsMenuText.empty()) {
            wsMenuText = L"Log in ...";
        }
        ModifyMenuW(g_APPUI.hTrayMenu, TRAY_MENU_LOGINOUT_ID, MF_BYCOMMAND | MF_STRING, TRAY_MENU_LOGINOUT_ID, wsMenuText.c_str());
        EngineGetStatus();
    }
    catch (const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
    }
}


