#pragma once

#define PRINTINGCTL_STARTDOCW_PROC_NAME	"StartDocW"

typedef int (WINAPI *STARTDOCW)(_In_ HDC hdc, _In_ CONST DOCINFOW *lpdi);

int WINAPI Core_StartDocW(_In_ HDC hdc, _In_ CONST DOCINFOW *lpdi);
int WINAPI Core_StartDocW2(_In_ HDC hdc, _In_ CONST DOCINFOW *lpdi);

BOOL InitializePrintingCtlHook(void);
void CleanupPrintingCtlHook(void);
