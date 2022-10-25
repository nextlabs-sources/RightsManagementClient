#pragma once

#define CLIPBOARDCTL_OPENCLIPBOARD_PROC_NAME	"OpenClipboard"

typedef	BOOL (WINAPI *OPENCLIPBOARD)(_In_opt_ HWND hWndNewOwner);

BOOL WINAPI Core_OpenClipboard(_In_opt_ HWND hWndNewOwner);

BOOL InitializeClipboardCtlHook(void);
void CleanupClipboardCtlHook(void);
