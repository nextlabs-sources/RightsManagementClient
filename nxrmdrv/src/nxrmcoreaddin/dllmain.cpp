// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "nxrmcoreaddin.h"
#include "nxrmdrv.h"
#include "nxrmdrvman.h"
#include "nxrmcorehlp.h"

CACHE_ALIGN HMODULE g_hModule = NULL;
CACHE_ALIGN BOOL	g_bIsOffice2010 = FALSE;
CACHE_ALIGN PVOID	g_Section = NULL;
CACHE_ALIGN CRITICAL_SECTION g_SectionLock;

static BOOL is_office2010(void);
static PIMAGE_NT_HEADERS RtlImageNtHeader (IN PVOID BaseAddress);

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:

		g_hModule = hModule;
		g_bIsOffice2010 = is_office2010();
		
		InitializeCriticalSectionAndSpinCount(&g_SectionLock, 300);

		DisableThreadLibraryCalls(hModule);
		break;

	case DLL_THREAD_ATTACH:
		
		break;
	case DLL_THREAD_DETACH:
		
		break;

	case DLL_PROCESS_DETACH:
		
		if (g_Section)
		{
			close_transporter_client(g_Section);
			g_Section = NULL;
		}

		DeleteCriticalSection(&g_SectionLock);

		break;
	}

	return TRUE;
}

static BOOL is_office2010(void)
{
	BOOL bRet = FALSE;

	IMAGE_NT_HEADERS		*pNtHdr = NULL;
	IMAGE_OPTIONAL_HEADER	*pOptHdr = NULL;

	HMODULE		hOfficeApp = NULL;

	do 
	{
		hOfficeApp = GetModuleHandleW(NULL);

		if(!hOfficeApp)
		{
			break;
		}

		pNtHdr = (IMAGE_NT_HEADERS *)RtlImageNtHeader(hOfficeApp);

		if(!pNtHdr)
		{
			break;
		}

		pOptHdr = &pNtHdr->OptionalHeader;

		if(!pOptHdr)
		{
			break;
		}

		if(pOptHdr->MajorLinkerVersion == 9 && pOptHdr->MinorLinkerVersion == 0)
		{
			bRet = TRUE;
			break;
		}

	} while (FALSE);

	return bRet;
}

static PIMAGE_NT_HEADERS RtlImageNtHeader (IN PVOID BaseAddress)
{
	PIMAGE_NT_HEADERS NtHeader;
	PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)BaseAddress;

	if(DosHeader && DosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return NULL;
	}

	if(DosHeader && DosHeader->e_magic == IMAGE_DOS_SIGNATURE)
	{
		NtHeader = (PIMAGE_NT_HEADERS)((ULONG_PTR)BaseAddress + DosHeader->e_lfanew);

		if(NtHeader->Signature == IMAGE_NT_SIGNATURE)
			return NtHeader;
	}

	return NULL;
}

BOOL init_rm_section_safe(void)
{
	BOOL bRet = FALSE;

	do
	{
		if (!g_Section)
		{
			EnterCriticalSection(&g_SectionLock);

			g_Section = init_transporter_client();

			LeaveCriticalSection(&g_SectionLock);
		}

		if (!g_Section)
		{
			break;
		}

		bRet = is_transporter_enabled(g_Section);

	} while (FALSE);

	return bRet;
}