#pragma once

#define LOADMODULE_LOADLIBRARYEXW_PROC_NAME	"LoadLibraryExW"
#define LOADMODULE_FREELIBRARY_PROC_NAME	"FreeLibrary"

typedef HMODULE (WINAPI *LOADLIBRARYEXW)(LPCWSTR lpFileName, HANDLE hFile, DWORD dwFlags);

HANDLE WINAPI Core_LoadLibraryExW(LPCWSTR lpFileName, HANDLE hFile, DWORD dwFlags);

typedef HMODULE (WINAPI *LOADLIBRARYW)(LPCWSTR lpFileName);

HMODULE WINAPI Core_LoadLibraryW(LPCWSTR lpFileName);

typedef BOOL (WINAPI *FREELIBRARY)(HMODULE hModule);

//BOOL WINAPI Core_FreeLibrary(HMODULE hModule);

BOOL InitializeLoadModuleHook(void);
void CleanupLoadModuleHook(void);
