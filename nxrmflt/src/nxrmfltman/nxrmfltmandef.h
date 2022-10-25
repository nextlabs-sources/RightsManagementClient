#pragma once

#include "nxrmflt.h"

#define NXRMFLTMAN_MAX_PORT_THREADS		8

typedef struct _NXRMFLT_MANAGER{

	HANDLE						ConnectionPort;

	HANDLE						Completion;

	HANDLE						StopEvent;

	HANDLE						WorkerThreadHandle[NXRMFLTMAN_MAX_PORT_THREADS];

	ULONG						WorkerThreadId[NXRMFLTMAN_MAX_PORT_THREADS];

	NXRMFLT_CALLBACK_NOTIFY		NotifyCallback;

	LOGAPI_LOG					DebugDumpCallback;

	LOGAPI_ACCEPT				DebugDumpCheckLevelCallback;

	BOOL						Stop;

	BOOL						DisableFiltering;

	BOOL						HideNxlExtension;
	
	PVOID						UserCtx; 

	LIST_ENTRY					MessageList;

}NXRMFLT_MANAGER, *PNXRMFLT_MANAGER;

typedef struct _NXRMFLT_MESSAGE {

	FILTER_MESSAGE_HEADER			Header;

	NXRMFLT_NOTIFICATION			Notification;

	OVERLAPPED						Ovlp;

	LIST_ENTRY						Link;

} NXRMFLT_MESSAGE, *PNXRMFLT_MESSAGE;

typedef struct _NXRM_CHECK_RIGHTS_REPLY {

	FILTER_REPLY_HEADER			ReplyHeader;

	NXRMFLT_CHECK_RIGHTS_REPLY	CheckRigtsReply;

}NXRM_CHECK_RIGHTS_REPLY, *PNXRM_CHECK_RIGHTS_REPLY;
