#pragma once

#include "nxrmflt.h"

NTSTATUS nxrmfltDecryptFile(
	__in	PFLT_CALLBACK_DATA	Data,
	__in	PFLT_INSTANCE		SrcInstance,
	__in	PFLT_INSTANCE		DstInstance,
	__in	PUNICODE_STRING		SrcFileName,
	__in	PUNICODE_STRING		DstFileName
	);

NTSTATUS nxrmfltDeleteFileByName(
	__in	PFLT_INSTANCE		Instance, 
	__in	PUNICODE_STRING		FileName
	);

NTSTATUS nxrmfltSetReadOnlyAttrByName(
	__in	PFLT_INSTANCE		Instance,
	__in	PUNICODE_STRING		FileName,
	__in	BOOLEAN				ReadOnlyAttr
	);

NTSTATUS nxrmfltEncryptFile(
	__in	PFLT_CALLBACK_DATA			Data,
	__in	PFLT_INSTANCE				SrcInstance,
	__in	PFLT_INSTANCE				DstInstance,
	__in	PUNICODE_STRING				SrcFileName,
	__in	PUNICODE_STRING				DstFileName
	);

ULONG nxrmfltExceptionFilter(
	__in	NXRMFLT_STREAM_CONTEXT	*Ctx, 
	__in	PEXCEPTION_POINTERS		ExceptionPointer
	);

VOID nxrmfltECPCleanupCallback(
	_Inout_  PVOID EcpContext,
	_In_     LPCGUID EcpType
	);

NTSTATUS
	nxrmfltAddReparseECP(
	_Inout_		PFLT_CALLBACK_DATA			Data,
	__in		PFLT_FILE_NAME_INFORMATION	NameInfo,
	__in		PFLT_INSTANCE				Instance,
	_In_opt_	PUNICODE_STRING				SourceFileName
	);

PVOID nxrmfltGetReparseECP(
	__in PFLT_CALLBACK_DATA Data
	);

NTSTATUS nxrmfltDeleteAddedReparseECP(
	__in PFLT_CALLBACK_DATA Data
	);

NTSTATUS nxrmfltAddBlockingECP(
	_Inout_		PFLT_CALLBACK_DATA			Data,
	__in		PFLT_FILE_NAME_INFORMATION	NameInfo
	);

NXL_PROCESS_NODE* nxrmfltFindProcessNodeByProcessID(
	__in HANDLE		ProcessID
	);

BOOLEAN nxrmfltDoesFileExistEx(
	__in PFLT_INSTANCE					Instance, 
	__in PFLT_FILE_NAME_INFORMATION		NameInfo,
	__in BOOLEAN						IgnoreCase
	);

BOOLEAN nxrmfltDoesFileExist(
	__in PFLT_INSTANCE					Instance, 
	__in PUNICODE_STRING				FileName,
	__in BOOLEAN						IgnoreCase
	);

NXL_CACHE_NODE *nxrmfltFindFirstCachNodeByParentDirectoryHash(
	__in ULONG ParentDirectoryHash
	);

NTSTATUS nxrmfltRenameOnDiskNXLFile(
	__in PFLT_INSTANCE		Instance, 
	__in PUNICODE_STRING	CurrentName,
	__in PUNICODE_STRING	NewName
	);

NTSTATUS nxrmfltRenameOnDiskNXLFileEx(
	__in PFLT_INSTANCE					Instance, 
	__in PUNICODE_STRING				CurrentName,
	__in PUNICODE_STRING				NewName,
	__in BOOLEAN						ReplaceIfExists,
	__in HANDLE							RootDirectory,
	__out PFLT_FILE_NAME_INFORMATION	*NewNameInfo
	);

NTSTATUS nxrmfltRenameFile(
	__in PFLT_INSTANCE		Instance, 
	__in PUNICODE_STRING	CurrentName,
	__in PUNICODE_STRING	NewName
	);

NTSTATUS nxrmfltBuildNamesInStreamContext(
	__in NXRMFLT_STREAM_CONTEXT		*Ctx, 
	__in UNICODE_STRING				*FileName
	);

NTSTATUS nxrmfltBuildRenameNodeFromCcb(
	__in NXRMFLT_STREAMHANDLE_CONTEXT	*Ccb,
	__inout NXL_RENAME_NODE				*RenameNode
	);	

VOID nxrmfltFreeRenameNode(
	__in NXL_RENAME_NODE	*RenameNode
	);

NXL_RENAME_NODE *nxrmfltFindRenameNodeFromCcb(
	__in NXRMFLT_STREAMHANDLE_CONTEXT	*Ccb
	);

NTSTATUS nxrmfltCopyOnDiskNxlFile(
	__in PFLT_INSTANCE		SrcInstance,
	__in UNICODE_STRING		*SrcFileName,
	__in PFLT_INSTANCE		DstInstance,
	__in UNICODE_STRING		*DstFileName);

HANDLE nxrmfltReferenceReparseFile(
	__in PFLT_INSTANCE		Instance,
	__in UNICODE_STRING		*FileName);

VOID nxrmfltFreeProcessNode(
	__in NXL_PROCESS_NODE *ProcessNode
	);

NTSTATUS nxrmfltCheckRights(
	__in HANDLE				ProcessId,
	__in HANDLE				ThreadId,
	__in NXL_CACHE_NODE		*pCacheNode,
	__in ULONG				IgnoreCache,
	__inout ULONGLONG		*RightsMask,
	__inout_opt ULONGLONG	*CustomRights,
	__inout_opt	ULONGLONG	*EvaluationId
	);

VOID NTAPI nxrmfltDeleteFileByNameWorkProc(
	__in PFLT_GENERIC_WORKITEM	FltWorkItem, 
	__in PVOID					FltObject, 
	__in_opt PVOID				Context);

VOID NTAPI nxrmfltSetReadOnlyAttrByNameWorkProc(
	__in PFLT_GENERIC_WORKITEM	FltWorkItem,
	__in PVOID					FltObject,
	__in_opt PVOID				Context);

NTSTATUS nxrmfltGetDeviceInfo(
	IN		PDEVICE_OBJECT				TargetDeviceObject,
	IN OUT  WCHAR						*DeviceName,
	IN		ULONG						DeviceNameLength,
	IN OUT	WCHAR						*SerialNumber,
	IN		ULONG						SerialNumberLength,
	IN OUT	STORAGE_BUS_TYPE			*BusType
	);

BOOLEAN nxrmfltIsProcessDirty(IN HANDLE ProcessId);

NTSTATUS nxrmfltGuessSourceFileFromProcessCache(IN HANDLE ProcessId, IN OUT PUNICODE_STRING SourceFileName);

NTSTATUS nxrmfltSendBlockNotificationMsg(IN PFLT_CALLBACK_DATA Data, IN PUNICODE_STRING FileName, IN NXRMFLT_BLOCK_REASON Reason);

NTSTATUS nxrmfltSendKeyChainErrorMsg(IN PFLT_INSTANCE Instance, IN PUNICODE_STRING FileName, IN NXL_KEKEY_ID *KeyId);

NTSTATUS nxrmfltCopyTags(
	__in PFLT_INSTANCE		SrcInstance,
	__in UNICODE_STRING		*SrcFileName,
	__in PFLT_INSTANCE		DstInstance,
	__in UNICODE_STRING		*DstFileName);

NTSTATUS nxrmfltPurgeRightsCache(IN PFLT_INSTANCE Instance, IN ULONG FileNameHash);

NTSTATUS nxrmfltForceAccessCheck(
	__in PFLT_INSTANCE		Instance,
	__in PUNICODE_STRING	FileName,  
	__in ACCESS_MASK		DesiredAccess, 
	__in ULONG				FileAttributes,
	__in ULONG				ShareAccess,
	__in ULONG				CreateOptions);

NTSTATUS nxrmfltCheckHideNXLExtsionByProcessId(
	__in HANDLE				ProcessId,
	__inout ULONG			*HideExt
);

NTSTATUS nxrmfltScanNotifyChangeDirectorySafe(
	__in PUNICODE_STRING			DirName, 
	__in PFILE_NOTIFY_INFORMATION	NotifyInfo, 
	__in ULONG						Length,
	__inout ULONG					*ContentDirty
	);

NTSTATUS nxrmfltDuplicateNXLFileAndItsRecords(
	__in ULONG						RequestorSessionId,
	__in PUNICODE_STRING			SrcFileName,
	__in PUNICODE_STRING			DstFileName,
	__in PFLT_INSTANCE				DstInstance,
	__in PUNICODE_STRING			DstDirName
	);

NTSTATUS nxrmfltBuildAdobeRenameNode(
	__in PUNICODE_STRING			SrcFileName,
	__in PUNICODE_STRING			DstFileName);

void nxrmfltFreeAdobeRenameNode(__in PADOBE_RENAME_NODE	pNode);

NTSTATUS nxrmfltBlockPreCreate(
	__inout PFLT_CALLBACK_DATA			Data,
	__in	PUNICODE_STRING				ReparseFileName,
	__in	PFLT_FILE_NAME_INFORMATION	NameInfo
	);

PFLT_INSTANCE nxrmfltFindInstanceByFileName(__in UNICODE_STRING *FileName);

LONG nxrmfltCompareFinalComponent(
	__in UNICODE_STRING *FileName1, 
	__in UNICODE_STRING *FileName2, 
	__in BOOLEAN		CaseInSensitive);

void FindFinalComponent(
	__in UNICODE_STRING *FileName, 
	__in UNICODE_STRING *FinalComponent);

NTSTATUS nxrmfltSendProcessNotification(
	__in HANDLE		ProcessId,
	__in ULONG		SessionId,
	__in BOOLEAN	Create,
	__in ULONGLONG	Flags,
	__in_opt PCUNICODE_STRING ImageFileName);

VOID NTAPI nxrmfltSendProcessNotificationWorkProc(
	__in PFLT_GENERIC_WORKITEM	FltWorkItem,
	__in PVOID					FltObject,
	__in_opt PVOID				Context);
