#pragma once

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct _SHELL_GLOBAL_DATA{

		CACHE_ALIGN	PVOID				Section;

		CACHE_ALIGN	CRITICAL_SECTION	SectionLock;

		CACHE_ALIGN HMODULE				hModule;

		CACHE_ALIGN	LONG				nxrmshellInstanceCount;

		CACHE_ALIGN	LONG				ContextMenuInstanceCount;

	}SHELL_GLOBAL_DATA,*PSHELL_GLOBAL_DATA;

#ifdef __cplusplus
}
#endif
