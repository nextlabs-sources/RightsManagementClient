#pragma once

#include "commdlg.h"

#define SAVEASCTL_COCREATEINSTANCE_PROC_NAME	"CoCreateInstance"
#define SAVEASCTL_GETSAVEFILENAMEW_PROC_NAME	"GetSaveFileNameW"
#define SAVEASCTL_GETOPENFILENAMEW_PROC_NAME	"GetOpenFileNameW"
#define SAVEASCTL_MOVEFILEEXW_PROC_NAME			"MoveFileExW"
#define SAVEASCTL_COPYFILEEXW_PROC_NAME			"CopyFileExW"

typedef HRESULT (WINAPI *SAVEASCTL_COCREATEINSTANCE)(
	IN REFCLSID		rclsid, 
	IN LPUNKNOWN	pUnkOuter,
	IN DWORD		dwClsContext, 
	IN REFIID		riid, 
	OUT LPVOID FAR* ppv);

HRESULT WINAPI Core_SaveAsCtl_CoCreateInstance(
	IN REFCLSID		rclsid, 
	IN LPUNKNOWN	pUnkOuter,
	IN DWORD		dwClsContext, 
	IN REFIID		riid, 
	OUT LPVOID FAR* ppv);

typedef BOOL (WINAPI *GETSAVEFILENAMEW)(LPOPENFILENAMEW);

typedef BOOL (WINAPI *GETOPENFILENAMEW)(LPOPENFILENAME);

BOOL WINAPI Core_GetSaveFileNameW(LPOPENFILENAMEW);

BOOL WINAPI Core_GetOpenFileNameW(_Inout_  LPOPENFILENAME lpofn);

BOOL InitializeSaveAsCtlHook(void);
void CleanupSaveAsCtlHook(void);

BOOL InitializeLegacySaveAsCtlHook(void);
void CleanupLegacySaveAsCtlHook(void);

typedef BOOL (WINAPI *MOVEFILEEXW)(
	_In_     LPCWSTR lpExistingFileName,
	_In_opt_ LPCWSTR lpNewFileName,
	_In_     DWORD    dwFlags
	);

BOOL WINAPI Core_MoveFileExW(
	_In_     LPCWSTR lpExistingFileName,
	_In_opt_ LPCWSTR lpNewFileName,
	_In_     DWORD    dwFlags
	);

typedef BOOL (WINAPI *COPYFILEEXW)(
	_In_        LPCWSTR lpExistingFileName,
	_In_        LPCWSTR lpNewFileName,
	_In_opt_    LPPROGRESS_ROUTINE lpProgressRoutine,
	_In_opt_    LPVOID lpData,
	_When_(pbCancel != NULL, _Pre_satisfies_(*pbCancel == FALSE))
	_Inout_opt_ LPBOOL pbCancel,
	_In_        DWORD dwCopyFlags
	);

BOOL WINAPI	Core_CopyFileExW(
	_In_        LPCWSTR lpExistingFileName,
	_In_        LPCWSTR lpNewFileName,
	_In_opt_    LPPROGRESS_ROUTINE lpProgressRoutine,
	_In_opt_    LPVOID lpData,
	_When_(pbCancel != NULL, _Pre_satisfies_(*pbCancel == FALSE))
	_Inout_opt_ LPBOOL pbCancel,
	_In_        DWORD dwCopyFlags
	);

BOOL InitializeExplorerRenameHook(void);
void CleanupExplorerRenameHook(void);

BOOL InitializeCopyFileExWHook(void);
void CleanupCopyFileExWHook(void);