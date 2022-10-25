#pragma once

#include <fltKernel.h>
#include <ntstrsafe.h>
#include <dontuse.h>
#include <suppress.h>
#include "nkdf.h"
#include <nudf\shared\listentry.h>
#include <nudf\shared\rbtree.h>
#include <nudf\shared\rightsdef.h>

#define PTDBG_TRACE_CRITICAL			0x00000001
#define PTDBG_TRACE_OPERATION_STATUS    0x00000002
#define PTDBG_TRACE_CACHE_NODE			0x00000004
#define PTDBG_TRACE_TEARDOWN			0x00000008
#define PTDBG_TRACE_CACHE				0x00000010
#define PTDBG_TRACE_ROUTINES            0x00000020
#define PTDBG_TRACE_WRITE				0x00000040
#define PTDBG_TRACE_DIRCONTROL			0x00000080
#define PTDBG_TRACE_RENAME				0x00000100

#define NXRMFLT_MIN_SECTOR_SIZE					(512)
#define NXRMFLT_MAX_REPARSE_FAST_BUFFER_SIZE	(260)
#define NXRMFLT_MAX_PATH						(260)
#define NXRMFLT_MAX_SRC_FAST_BUFFER_SIZE		(260)
#define NXRMFLT_INVALID_SESSION_ID				(0xffffffff)
#define NXRMFLT_READFILE_BUFFER_SIZE			(512*1024)
#define NXRMFLT_FULLPATH_BUFFER_SIZE			(4*1024)
#define NXRMFLT_EXTENSION_MAX_SIZE				(64)
#define NXRMFLT_CONTENT_KEY_LENGTH				(32)

#define NXRMFLT_SAVEAS_EXPIRE_TIMEOUT_1S		(0L-((LONGLONG)1 * 10 * 1000 * 1000))

#define NXRMFLT_INSTANCE_CTX_TAG			'1MYT'
#define NXRMFLT_STREAM_CTX_TAG				'2MYT'
#define NXRMFLT_REPARSE_NAME_TAG			'3MYT'
#define NXRMFLT_READFILE_TAG				'4MYT'
#define NXRMFLT_NXLCACHE_TAG				'5MYT'
#define NXRMFLT_FULLPATH_TAG				'6MYT'
#define NXRMFLT_ECP_CTX_TAG					'7MYT'
#define NXRMFLT_SECURITY_TAG				'8MYT'
#define NXRMFLT_DIRCTLCTX_TAG				'9MYT'
#define NXRMFLT_RMINFO_TAG					'AMYT'
#define NXRMFLT_TMP_TAG						'BMYT'
#define NXRMFLT_PROCESSNODE_TAG				'CMYT'
#define NXRMFLT_STEAMHANDLE_CTX_TAG			'DMYT'
#define NXRMFLT_RENAMENODE_TAG				'EMYT'
#define NXRMFLT_RIGHTSNODE_TAG				'FMYT'
#define NXRMFLT_NOTIFICATION_TAG			'GMYT'
#define NXRMFLT_KEYCHAINNODE_TAG			'HMYT'
#define NXRMFLT_SAVEASNODE_TAG				'IMYT'
#define NXRMFLT_ECPSRCFILENAME_TAG			'JMYT'
#define NXRMFLT_SRCFILENAME_TAG				'KMYT'
#define NXRMFLT_SETINFORMATION_TAG			'LMYT'
#define NXRMFLT_NEWRENAMEINFO_TAG			'MMYT'
#define NXRMFLT_ADOBERENAME_NODE_TAG		'NMYT'
#define NXRMFLT_ADOBERENAME_NAME_TAG		'OMYT'
#define NXRMFLT_NXLFILECREATECTX_TAG		'PMYT'
#define NXRMFLT_SETREADONLYATTRCTX_TAG		'QMYT'

#define NXRMFLT_NXL_EXTENSION				L"NXL"

#define NXRMFLT_NXL_DOTEXT					L".nxl"

#define NXRMFLT_MUP_PREFIX					L"\\Device\\Mup\\"
#define NXRMFLT_DEVICE_PREFIX				L"\\Device\\"

#define NXRMFLT_FLAG_READ_ONLY				(0x0001)
#define NXRMFLT_FLAG_CTX_ATTACHED			(0x0002)
#define NXRMFLT_FLAG_ATTACHING_CTX			(0x0004)
#define NXRMFLT_FLAG_STOGRAGE_EJECTED		(0x0008)
#define NXRMFLT_FLAG_DECRYPTING				(0x0010)
//#define NXRMFLT_FLAG_NEWLYCREATED			(0x0020)

//#define NXRMFLT_DEBUG_TEMP_REPARSE_PATH		L"\\Device\\SecuOvLayVolumeK"
//#define NXRMFLT_DEBUG_TEMP_REPARSE_PATH		L"\\Device\\HarddiskVolume5"

#define NXRMFLT_NXLFILE_CREATE_CTX_FLAG_FILE_IS_IN_CACHE	(0x00000001)

#define NXRMFLT_LEGACY_THEMES_FOLDER		L"\\Windows\\Resources\\Ease of Access Themes\\"

#define PT_DBG_PRINT( _dbgLevel, _string )          \
	(FlagOn(gTraceFlags,(_dbgLevel)) ?              \
		DbgPrint _string :                          \
		((int)0))

// {01915FA3-58E8-4DF3-9566-3D869F35DA05}
DEFINE_GUID(GUID_ECP_NXRMFLT_REPARSE,
			0x1915fa3,
			0x58e8,
			0x4df3,
			0x95, 0x66, 0x3d, 0x86, 0x9f, 0x35, 0xda, 0x5);

// {C414D098-75BC-45C7-BB70-6DBC6C2052E1}
DEFINE_GUID(GUID_ECP_NXRMFLT_BLOCKING,
			0xc414d098, 
			0x75bc, 
			0x45c7, 
			0xbb, 0x70, 0x6d, 0xbc, 0x6c, 0x20, 0x52, 0xe1);

typedef struct _NXRMFLT_INSTANCE_CONTEXT{

	PFLT_VOLUME_PROPERTIES			VolumeProperties;

	UNICODE_STRING					VolDosName;

	ULONG							SectorSize;

	BOOLEAN							IsWritable;

	BOOLEAN							DisableFiltering;

	FLT_FILESYSTEM_TYPE				VolumeFilesystemType;

	ERESOURCE						Resource;

	LIST_ENTRY						Link;

	//
	//  Cancel safe queue members
	//

	FLT_CALLBACK_DATA_QUEUE			CallbackDataQueue;

	LIST_ENTRY						WorkItemQueue;

	KSPIN_LOCK						WorkItemQueueLock;

	//
	//  Flag to control the life/death of the work item thread
	//

	volatile LONG					WorkerThreadFlag;

	//
	//	Device name
	//	VendorID + ProductID swprintf_s(dev_name,NXRMFLT_MAX_PATH,L"%s %s",VendorId,ProductId);
	//
	WCHAR							DeviceName[NXRMFLT_MAX_PATH];		

	//
	// Device Serial number
	//
	WCHAR							SerialNumber[NXRMFLT_MAX_PATH];

	//
	// Bus type
	//
	STORAGE_BUS_TYPE				BusType;

	//
	//  Notify the worker thread that the instance is being torn down
	//

	KEVENT							TeardownEvent;

	PFLT_INSTANCE					Instance;

	WCHAR							VolPropBuffer[sizeof(FLT_VOLUME_PROPERTIES) + 512];

}NXRMFLT_INSTANCE_CONTEXT, *PNXRMFLT_INSTANCE_CONTEXT;

typedef struct _NXRMFLT_STREAM_CONTEXT{

	EX_PUSH_LOCK					CtxLock;

	PFLT_INSTANCE					OriginalInstance;

	ULONG							RequestorSessionId;

	ULONG							ContentDirty;

	UNICODE_STRING					FileName;			// FileName that PreCreate see. It's the same as what QueryDirectory make OS see

	UNICODE_STRING					FullPathParentDir;

	UNICODE_STRING					FinalComponent;

	BOOLEAN							ReleaseFileName;

	WCHAR							FileNameFastBuffer[NXRMFLT_MAX_REPARSE_FAST_BUFFER_SIZE];

}NXRMFLT_STREAM_CONTEXT, *PNXRMFLT_STREAM_CONTEXT;

typedef struct _NXRMFLT_STREAMHANDLE_CONTEXT{

	PFLT_FILE_NAME_INFORMATION		SourceFileNameInfo;

	PFLT_FILE_NAME_INFORMATION		DestinationFileNameInfo;

	BOOLEAN							SourceFileIsNxlFile;

	BOOLEAN							EncryptDestinationFile;

	BOOLEAN							DeleteOnClose;

	UCHAR							Reserved;

}NXRMFLT_STREAMHANDLE_CONTEXT, *PNXRMFLT_STREAMHANDLE_CONTEXT;

typedef struct _NXRMFLT_GLOBAL_DATA{

	ULONG					TotalContext;

	PAGED_LOOKASIDE_LIST	NXLCacheLookaside;

	PAGED_LOOKASIDE_LIST	FullPathLookaside;

	PAGED_LOOKASIDE_LIST	DirCtlCtxLookaside;

	PAGED_LOOKASIDE_LIST	NXLProcessCacheLookaside;

	PAGED_LOOKASIDE_LIST	RenameCacheLookaside;

	PAGED_LOOKASIDE_LIST	NXLRightsCacheLookaside;

	PAGED_LOOKASIDE_LIST	NotificationLookaside;

	PAGED_LOOKASIDE_LIST	SaveAsExpireLookaside;

	PAGED_LOOKASIDE_LIST	SetInformationCtxLookaside;

	PAGED_LOOKASIDE_LIST	AdobeRenameLookaside;

	PAGED_LOOKASIDE_LIST	NXLFileCreateCtxLookaside;

	PAGED_LOOKASIDE_LIST	SetReadOnlyAttrCtxLookaside;

	PFLT_INSTANCE			ReparseInstance;

	UNICODE_STRING			NXLFileDotExtsion;

	UNICODE_STRING			NXLFileExtsion;

	PFLT_FILTER				Filter;

	PFLT_PORT				ServerPort;

	PFLT_PORT				ClientPort;

	HANDLE					PortProcessId;

	ULONG					HideNXLExtension;

	ULONG					DriverUnloading;

	ULONG					NxlFileCount;

	rb_root					NxlFileCache;

	EX_PUSH_LOCK			NxlFileCacheLock;

	NXL_KEKEY_BLOB			PrimaryKey;		// key for encryption

	EX_PUSH_LOCK			PrimaryKeyLock;

	LIST_ENTRY				KeyChain;		// key chain for decryption

	EX_PUSH_LOCK			KeyChainLock;	

	LIST_ENTRY				NxlProcessList;

	EX_PUSH_LOCK			NxlProcessListLock;

	LIST_ENTRY				RenameList;

	EX_PUSH_LOCK			RenameListLock;

	LIST_ENTRY				ExpireTable;

	EX_PUSH_LOCK			ExpireTableLock;

	LIST_ENTRY				AdobeRenameExpireTable;

	EX_PUSH_LOCK			AdobeRenameExpireTableLock;

	LIST_ENTRY				AttachedInstancesList;

	EX_PUSH_LOCK			AttachedInstancesListLock;

	HANDLE					ExpireThreadHandle;

	KEVENT					ExpireStopEvent;

}NXRMFLT_GLOBAL_DATA, *PNXRMFLT_GLOBAL_DATA;

typedef struct _NXL_CACHE_NODE{

	rb_node					Node;

	LIST_ENTRY				Link;

	UNICODE_STRING			FileName;			// fake name without extension

	UNICODE_STRING			OriginalFileName;	// original on disk file name (with extension)

	UNICODE_STRING			ReparseFileName;	// file name on VHD

	UNICODE_STRING			SourceFileName;		// source file name if it's on remote or removable media

	LARGE_INTEGER			FileID;

	ULONG					FileAttributes;

	ULONG					Flags;

	ULONG					ParentDirectoryHash;

	ULONG					FileNameHash;

	EX_RUNDOWN_REF			NodeRundownRef;

	PFLT_INSTANCE			Instance;

	BOOLEAN					OnRemoveOrRemovableMedia;

	BOOLEAN					ReleaseReparseName;

	BOOLEAN					ReleaseFileName;

	BOOLEAN					ReleaseSourceFileName;

	WCHAR					FileNameFastBuffer[NXRMFLT_MAX_REPARSE_FAST_BUFFER_SIZE];

	WCHAR					ReparseFileNameFastBuffer[NXRMFLT_MAX_REPARSE_FAST_BUFFER_SIZE];

	WCHAR					SourceFileNameFastBuffer[NXRMFLT_MAX_SRC_FAST_BUFFER_SIZE];

}NXL_CACHE_NODE, *PNXL_CACHE_NODE;

typedef struct _NXRMFLT_REPARSE_ECP_CONTEXT{

	FLT_FILE_NAME_INFORMATION	*NameInfo;

	PFLT_INSTANCE				OriginalInstance;

	ULONGLONG					RightsMask;

	ULONGLONG					CustomRightsMask;

	UNICODE_STRING				SourceFileName;

	BOOLEAN						ReleaseSourceFileName;

	WCHAR						SourceFileNameBuffer[NXRMFLT_MAX_PATH];

}NXRMFLT_REPARSE_ECP_CONTEXT, *PNXRMFLT_REPARSE_ECP_CONTEXT;

typedef struct _NXRMFLT_BLOCKING_ECP_CONTEXT {

	FLT_FILE_NAME_INFORMATION	*NameInfo;

}NXRMFLT_BLOCKING_ECP_CONTEXT, *PNXRMFLT_BLOCKING_ECP_CONTEXT;

typedef struct _NXRMFLT_DIRCTL_CONTEXT{

	PFLT_FILE_NAME_INFORMATION		NameInfo;

	UNICODE_STRING					FileName;

	PVOID							OriginalFileName;

	ULONG							HideExt;

}NXRMFLT_DIRCTL_CONTEXT, *PNXRMFLT_DIRCTL_CONTEXT;

typedef struct _NXRMFLT_SETINFORMATION_CONTEXT{

	PFLT_FILE_NAME_INFORMATION		NameInfo;

	PNXRMFLT_STREAM_CONTEXT			Ctx;

}NXRMFLT_SETINFORMATION_CONTEXT, *PNXRMFLT_SETINFORMATION_CONTEXT;

typedef struct _NXL_PROCESS_NODE{

	LIST_ENTRY						Link;

	EX_RUNDOWN_REF					NodeRundownRef;

	HANDLE							ProcessId;

	ULONG							SessionId;

	BOOLEAN							AlwaysGrantAcess;

	BOOLEAN							AlwaysDenyAccess;

	BOOLEAN							HideXNLExtension;

	BOOLEAN							HasGraphicIntegration;

	rb_root							RightsCache;

	EX_PUSH_LOCK					RightsCacheLock;

}NXL_PROCESS_NODE, *PNXL_PROCESS_NODE;

typedef struct _NXL_RENAME_NODE{

	LIST_ENTRY						Link;

	UNICODE_STRING					SourceFileName;

	UNICODE_STRING					DestinationFileName;

	BOOLEAN							SourceFileIsNxlFile;

	BOOLEAN							ReleaseSourceFileName;

	BOOLEAN							ReleaseDestinationFileName;

	WCHAR							SourceFileNameFastBuffer[NXRMFLT_MAX_PATH];

	WCHAR							DestinationFileNameFastBuffer[NXRMFLT_MAX_PATH];

}NXL_RENAME_NODE,*PNXL_RENAME_NODE;

typedef struct _NXL_RIGHTS_CACHE_NODE{

	rb_node							Node;

	ULONG							FileNameHash;

	WCHAR							FileNameBuf[NXRMFLT_MAX_PATH];

	UNICODE_STRING					FileName;

	ULONGLONG						RightsMask;

	ULONGLONG						CustomRights;

	ULONGLONG						EvaluationId;

}NXL_RIGHTS_CACHE_NODE,*PNXL_RIGHTS_CACHE_NODE;

typedef struct _NXL_KEYCHAIN_NODE{

	LIST_ENTRY						Link;

	NXL_KEKEY_BLOB					KeyBlob;

}NXL_KEYCHAIN_NODE, *PNXL_KEYCHAIN_NODE;

typedef struct _NXL_SAVEAS_NODE{

	LIST_ENTRY				Link;

	WCHAR					SaveAsFileNameBuf[NXRMFLT_MAX_PATH];

	WCHAR					SourceFileNameBuf[NXRMFLT_MAX_PATH];

	UNICODE_STRING			SaveAsFileName;

	UNICODE_STRING			SourceFileName;

	HANDLE					ProcessId;

	EX_RUNDOWN_REF			NodeRundownRef;

	ULONG					ExpireTick;

}NXL_SAVEAS_NODE, *PNXL_SAVEAS_NODE;

typedef struct _ADOBE_RENAME_NODE{

	LIST_ENTRY				Link;

	UNICODE_STRING			SourceFileName;

	UNICODE_STRING			DestinationFileName;

	BOOLEAN					ReleaseSourceFileName;

	BOOLEAN					ReleaseDestinationFileName;

	WCHAR					SourceFileNameFastBuffer[NXRMFLT_MAX_PATH];

	WCHAR					DestinationFileNameFastBuffer[NXRMFLT_MAX_PATH];

	EX_RUNDOWN_REF			NodeRundownRef;

	ULONG					ExpireTick;

}ADOBE_RENAME_NODE,*PADOBE_RENAME_NODE;

typedef struct _NXLFILE_CREATE_CONTEXT{

	PFLT_FILE_NAME_INFORMATION		NameInfo;

	UNICODE_STRING					FileNameWithoutNXLExtension;

	ULONG							Flags;

}NXLFILE_CREATE_CONTEXT, *PNXLFILE_CREATE_CONTEXT;

typedef struct _SET_READONLY_ATTR_CONTEXT{

	WCHAR					FileNameBuf[NXRMFLT_FULLPATH_BUFFER_SIZE];

	BOOLEAN					ReadOnlyAttr;

}SET_READONLY_ATTR_CONTEXT, *PSET_READONLY_ATTR_CONTEXT;
