
// Windows Header Files:
#include <windows.h>

#include <nudf\shared\enginectl.h>
#include <nudf\exception.hpp>
#include <nudf\pipe.hpp>
#include <nudf\nxrmres.h>
#include <nudf\resutil.hpp>


// user-defined Header Files:
#include "resource.h"
#include "nxrmtray.h"
#include "main_popup.h"

extern APPUI      g_APPUI;
extern APPSTATE   g_APPState;

static HBRUSH     g_hBrDlgBk = NULL;

//TODO: Initial setting page 
void InitMainPopupDlg(void)
{
	RECT     rcClient;
	RECT     rcMainDlg;
	RECT     rcSetting;
	POINT    ptClientStart;
	POINT    ptClientStop;
	HFONT    hFontMedium;
	std::wstring wsUiText;      // Store Msg string Loaded from ResDLL

	GetWindowRect(GetDlgItem(g_APPUI.hMainDlg, IDC_MAIN_LAST_UPDATE_LABEL), &rcClient);
	ptClientStart.x = rcClient.left;
	ptClientStart.y = rcClient.bottom;
	ScreenToClient(g_APPUI.hMainDlg, &ptClientStart);
	GetClientRect(g_APPUI.hMainDlg, &rcMainDlg);
	GetWindowRect(GetDlgItem(g_APPUI.hMainDlg, IDC_MAIN_SETTING), &rcSetting);
	ptClientStop.x = rcSetting.left;
	ptClientStop.y = rcSetting.top;
	ScreenToClient(g_APPUI.hMainDlg, &ptClientStop);

	//TODO: Create General Page and make it fit Tab control's client Rect
	g_APPUI.hMainPopupDlg = CreateDialogW(g_APPUI.hInst, MAKEINTRESOURCEW(IDD_MAIN_POPUP), g_APPUI.hMainDlg, (DLGPROC)MainPopupDlgProc);

	if(NULL == g_APPUI.hMainPopupDlg){
		MessageBoxW(NULL, L"Create pop up dialog failed.", L"ERROR", MB_ICONERROR | MB_OK);
		return ;
	}
	MoveWindow(g_APPUI.hMainPopupDlg, ptClientStart.x, ptClientStart.y, 
			   rcMainDlg.right - rcMainDlg.left, ptClientStop.y - ptClientStart.y, TRUE);
	
	// Store the RECT of main pop up dialog, will be use when showing or hiding this dialog
	g_APPUI.rcPopupDlg.left = ptClientStart.x;
	g_APPUI.rcPopupDlg.top = ptClientStart.y;
	g_APPUI.rcPopupDlg.right = ptClientStop.x;
	g_APPUI.rcPopupDlg.bottom = ptClientStop.y;

	hFontMedium = CreateFont(20, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, 
		DEFAULT_CHARSET, OUT_STROKE_PRECIS, CLIP_STROKE_PRECIS, 
		DRAFT_QUALITY, FF_DONTCARE, TEXT("Segoe UI"));

	SendMessageW(GetDlgItem(g_APPUI.hMainPopupDlg, IDC_MAIN_POPUP_LABEL), WM_SETFONT, (WPARAM)hFontMedium, TRUE);
	wsUiText = nudf::util::res::LoadMessage(g_APPUI.hResDll, (DWORD)IDS_MAIN_POPUP_LABEL, 1024, g_APPState.dwLangId);
	SendMessageW(GetDlgItem(g_APPUI.hMainPopupDlg, IDC_MAIN_POPUP_LABEL), WM_SETTEXT, NULL, (LPARAM)(LPWSTR)wsUiText.c_str());

	return ;
}

//Setting page's PROC
BOOL WINAPI MainPopupDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND   hWndItem;
	HICON  hIcon;
	int    iItemID;
	RECT   rcClient;

	switch(uMsg) 
	{     
	case WM_INITDIALOG:
		g_hBrDlgBk = CreateSolidBrush(RGB(181, 181, 181));
		return TRUE;

	case WM_COMMAND:

		iItemID = LOWORD(wParam);
		if(IDC_MAIN_POPUP_SWITCH == iItemID){
			g_APPState.bPopupNotify = !g_APPState.bPopupNotify;
			GetWindowRect(GetDlgItem(g_APPUI.hMainPopupDlg, IDC_MAIN_POPUP_SWITCH), &rcClient);
			InvalidateRect(g_APPUI.hMainDlg, &rcClient, TRUE);
		}

		break;

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT) lParam;
			int              iButtonID   = lpDIS->CtlID;
			HDC              hDc         = lpDIS->hDC; 
			BOOL             bIsPressed  = (lpDIS->itemState & ODS_SELECTED);
			BOOL             bIsFocused  = (lpDIS->itemState & ODS_FOCUS);
			BOOL             bIsDisabled = (lpDIS->itemState & ODS_DISABLED);
			BOOL             bDrawFocusRect = !(lpDIS->itemState & ODS_NOFOCUSRECT);
			RECT             rcBtn     = lpDIS->rcItem;


			if(IDC_MAIN_POPUP_SWITCH == iButtonID){
				hWndItem = GetDlgItem(hWnd, iButtonID);
				SetBkMode(hDc, TRANSPARENT);
				FillRect(hDc, &rcBtn, g_hBrDlgBk);

				if(TRUE == g_APPState.bPopupNotify){
					hIcon = (HICON)LoadImageW(g_APPUI.hInst, MAKEINTRESOURCE(IDI_BTN_YES), IMAGE_ICON, 76, 32, LR_DEFAULTCOLOR);
				}
				else{
					hIcon = (HICON)LoadImageW(g_APPUI.hInst, MAKEINTRESOURCE(IDI_BTN_NO), IMAGE_ICON, 76, 32, LR_DEFAULTCOLOR);
				}
				DrawIconEx (hDc, (rcBtn.left + rcBtn.right) / 2 - 38, (rcBtn.top + rcBtn.bottom) / 2 - 16, hIcon, 76, 32, 0, NULL, DI_NORMAL);

				return TRUE;
			} //if

		} //case
		break;

	case WM_CTLCOLORDLG:
		return (BOOL)(ULONG_PTR)g_hBrDlgBk;

	case WM_CTLCOLORSTATIC:
		if(IDC_MAIN_POPUP_LABEL == GetWindowLong ((HWND) lParam, GWL_ID)){
			SetTextColor ((HDC) wParam, RGB(255, 255, 255)) ;
		}

		SetBkMode((HDC)wParam, TRANSPARENT);
		return (BOOL)(ULONG_PTR)((HBRUSH)GetStockObject(NULL_BRUSH));

	case WM_CLOSE:
		DeleteObject(g_hBrDlgBk);
		EndDialog(hWnd,0);
		return FALSE;
	}

	return (BOOL)(DefWindowProcW(hWnd, uMsg, wParam, lParam));
}