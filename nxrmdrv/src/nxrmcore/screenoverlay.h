#pragma once

#define WNDMSG_DISPATCHMESSAGEW_PROC_NAME			"DispatchMessageW"
#define WNDMSG_DISPATCHMESSAGEA_PROC_NAME			"DispatchMessageA"

typedef LRESULT(WINAPI *WNDMSG_DISPATCHMESSAGEW)(__in CONST MSG *lpMsg);

typedef LRESULT(WINAPI *WNDMSG_DISPATCHMESSAGEA)(__in CONST MSG *lpMsg);

LRESULT	WINAPI Core_DispatchMessageW(__in CONST MSG *lpMsg);

LRESULT	WINAPI Core_DispatchMessageA(__in CONST MSG *lpMsg);

BOOL InitializeScreenOverlayFunctions(void);

BOOL InitializeWndMsgHooks(void);

void CleanupWndMsgHooks(void);