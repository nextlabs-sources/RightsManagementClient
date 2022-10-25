#pragma once

#include "nxrmdrv.h"
#include <nudf\shared\logdef.h>

typedef struct _NXRMDRV_MANAGER{

	PVOID						nxrmdrvsection;

	HANDLE						StopEvent;

	HANDLE						WorkerThread;

	ULONG						WorkerThreadId;

	ULONG						WorkerThreadRecyleTick;

	NXRMDRV_CALLBACK_NOTIFY		NotifyCallback;

	LOGAPI_LOG					DebugDumpCallback;
	
	LOGAPI_ACCEPT				DebugDumpCheckLevelCallback;

	BOOL						Stop;

	BOOL						DisableFiltering;

	PVOID						UserCtx; 

	rb_root						ProcessCache;

	CRITICAL_SECTION			ProcessCacheLock;

	NXRM_PROCESS_INFO			*ProcessCacheInfo;

	ULONG						ProcessCacheInfoSize;

	NXRM_PROCESS_ENTRY			LastCachedProcessNode;

	HANDLE						ProcessCacheThread;

	ULONG						ProcessCacheThreadId;

	LIST_ENTRY					ProtectedModeAppInfoList;

	CRITICAL_SECTION			ProtectedModeAppInfoListLock;

}NXRMDRV_MANAGER, *PNXRMDRV_MANAGER;

typedef struct _PROTECTEDMODEAPPINFO{

	ULONG						ProcessId;

	ULONG						ProtectedChildProcessId;

	WCHAR						ActiveDocFileName[MAX_PATH];

	ULONGLONG					RightsMask;

	ULONGLONG					CustomRights;

	ULONGLONG					EvaluationId;

	LIST_ENTRY					Link;

}PROTECTEDMODEAPPINFO, *PPROTECTEDMODEAPPINFO;