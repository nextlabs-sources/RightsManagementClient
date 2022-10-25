#pragma once

#include <nudf\shared\rbtree.h>
#include <nudf\shared\keydef.h>
#include <nudf\shared\nxlfmt.h>

//
//  Name of nxrmflt filter server ports
//

#define NXRMFLT_MSG_PORT_NAME                    L"\\nxrmfltPort"

#ifndef NXRMFLT_FULLPATH_BUFFER_SIZE
#define NXRMFLT_FULLPATH_BUFFER_SIZE			(4*1024)
#endif

#ifndef NXRMFLT_MAX_PATH
#define NXRMFLT_MAX_PATH						(260)
#endif

#define NXRMFLT_MSG_TYPE_CHECK_RIGHTS					(0x80000001)
#define NXRMFLT_MSG_TYPE_BLOCK_NOTIFICATION				(0x80000002)
#define NXRMFLT_MSG_TYPE_KEYCHAIN_ERROR_NOTIFICATION	(0x80000003)
#define NXRMFLT_MSG_TYPE_PURGE_CACHE_NOTIFICATION		(0x80000004)
#define NXRMFLT_MSG_TYPE_PROCESS_NOTIFICATION			(0x80000005)

//
//  Message timeout in 30 seconds
//
#define NXRMFLT_MSG_TIMEOUT_IN_MS				(30*1000)

#define NXRM_CEKEY_LEN							32

#define NXRM_ALGORITHM_AES256					2
		
#define NXRM_PROCESS_FLAG_HIDE_NXL_EXT				(0x0000000000000001)
#define NXRM_PROCESS_FLAG_HAS_OVERLAY_INTEGRATION	(0x0000000000000002)
#define NXRM_PROCESS_FLAG_WITH_NXL_OPENED	        (0x0000000000000004)
#define NXRM_PROCESS_FLAG_HAS_NO_OVERLAYPOLICY		(0x0000000000000008)	//user mode flag for overlay policy

#pragma pack(push, 8)

typedef enum _NXRMFLT_COMMAND {

	nxrmfltInvalidCommand = 0,
	nxrmfltSaveAsForecast,
	nxrmfltGetProtectedVolumes,
	nxrmfltPolicyChanged,
	nxrmfltSetKeys

}NXRMFLT_COMMAND;

typedef enum _NXRMFLT_BLOCK_REASON {

	nxrmfltDeniedWritesOpen = 0x1000,
	nxrmfltDeniedSaveAsOpen,
	nxrmfltSaveAsToUnprotectedVolume

}NXRMFLT_BLOCK_REASON;

//
//  Message: Kernel -> User Message
//
typedef struct _NXRM_PURGE_CACHE_NOTIFICATION {

	WCHAR					FileName[NXRMFLT_MAX_PATH];

}NXRM_PURGE_CACHE_NOTIFICATION, *PNXRM_PURGE_CACHE_NOTIFICATION;

typedef struct _NXRM_PROCESS_NOTIFICATION {

	ULONG					SessionId;

	ULONG					ProcessId;

	ULONG					Create;			// 0 means exit; 1 means create

	ULONGLONG				Flags;			// features flag

	WCHAR					ProcessImagePath[NXRMFLT_MAX_PATH];

}NXRM_PROCESS_NOTIFICATION, *PNXRM_PROCESS_NOTIFICATION;

typedef struct _NXRM_BLOCK_NOTIFICATION {

	ULONG					ProcessId;

	ULONG					ThreadId;

	ULONG					SessionId;

	NXRMFLT_BLOCK_REASON	Reason;

	WCHAR					FileName[NXRMFLT_MAX_PATH];

} NXRM_BLOCK_NOTIFICATION, *PNXRM_BLOCK_NOTIFICATION;

typedef struct _NXRM_KEYCHAIN_ERROR_NOTIFICATION {

	WCHAR					FileName[NXRMFLT_MAX_PATH];

	NXL_KEKEY_ID			KeyId;

	ULONG					SessionId;

} NXRM_KEYCHAIN_ERROR_NOTIFICATION, *PNXRM_KEYCHAIN_ERROR_NOTIFICATION;

typedef struct _NXRM_CHECK_RIGHTS_NOTIFICATION {

	ULONG						ProcessId;

	ULONG						ThreadId;

	WCHAR						FileName[NXRMFLT_FULLPATH_BUFFER_SIZE/sizeof(WCHAR)];

} NXRM_CHECK_RIGHTS_NOTIFICATION, *PNXRM_CHECK_RIGHTS_NOTIFICATION;

typedef struct _NXRMFLT_NOTIFICATION
{
	ULONG									Type;

	union
	{
		NXRM_CHECK_RIGHTS_NOTIFICATION		CheckRightsMsg;

		NXRM_BLOCK_NOTIFICATION				BlockMsg;

		NXRM_KEYCHAIN_ERROR_NOTIFICATION	KeyChainErrorMsg;

		NXRM_PURGE_CACHE_NOTIFICATION		PurgeCacheMsg;

		NXRM_PROCESS_NOTIFICATION			ProcessMsg;
	};

}NXRMFLT_NOTIFICATION, *PNXRMFLT_NOTIFICATION;

typedef struct _NXRMFLT_CONNECTION_CONTEXT {

	ULONG			HideNxlExtension;

	ULONG			NumberOfKey;

	NXRM_KEY_BLOB	KeyChain[1];

}NXRMFLT_CONNECTION_CONTEXT, *PNXRMFLT_CONNECTION_CONTEXT;

//
//	Message: User -> Kernel Message
//

typedef struct _NXRMFLT_CHECK_RIGHTS_REPLY {

	ULONGLONG		RightsMask;

	ULONGLONG		CustomRights;

	ULONGLONG		EvaluationId;

}NXRMFLT_CHECK_RIGHTS_REPLY, *PNXRMFLT_CHECK_RIGHTS_REPLY;

typedef struct _NXRMFLT_SAVEAS_FORECAST {

	ULONG						ProcessId;

	WCHAR						SaveAsFileName[NXRMFLT_MAX_PATH];

	WCHAR						SourceFileName[NXRMFLT_MAX_PATH];	// optional

}NXRMFLT_SAVEAS_FORECAST, *PNXRMFLT_SAVEAS_FORECAST;

typedef struct _NXRMFLT_COMMAND_MSG {

	NXRMFLT_COMMAND					Command;

	ULONG							Size;

	UCHAR							Data[1];

}NXRMFLT_COMMAND_MSG, *PNXRMFLT_COMMAND_MSG;

#pragma pack(pop)