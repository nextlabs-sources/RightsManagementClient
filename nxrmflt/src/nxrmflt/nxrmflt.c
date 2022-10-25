#include "nxrmfltdef.h"
#include "nxrmfltCbdq.h"
#include "nxrmfltcreate.h"
#include "nxrmfltcleanup.h"
#include "nxrmfltclose.h"
#include "nxrmfltsetinformation.h"
#include "nxrmfltdirectorycontrol.h"
#include "nxrmfltwrite.h"
#include "nxrmfltsetea.h"
#include "nxrmfltsetsecurity.h"
#include "nxrmfltnxlcachemgr.h"
#include "nxrmfltqueryopen.h"
#include "nxrmfltqueryinformation.h"
#include "nxrmfltutils.h"
#include "nxrmflt.h"
#include "nxrmfltcommunication.h"
#include "nxrmfltqueryea.h"
#include "nxrmfltexpire.h"

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")


DECLSPEC_CACHEALIGN ULONG			gTraceFlags = PTDBG_TRACE_CRITICAL;
//DECLSPEC_CACHEALIGN PFLT_FILTER		gFilterHandle;

DECLSPEC_CACHEALIGN NXRMFLT_GLOBAL_DATA	Global;

/*************************************************************************
	Prototypes
*************************************************************************/

DRIVER_INITIALIZE DriverEntry;
NTSTATUS
DriverEntry (
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath
	);

NTSTATUS
nxrmfltInstanceSetup (
	_In_ PCFLT_RELATED_OBJECTS		FltObjects,
	_In_ FLT_INSTANCE_SETUP_FLAGS	Flags,
	_In_ DEVICE_TYPE				VolumeDeviceType,
	_In_ FLT_FILESYSTEM_TYPE		VolumeFilesystemType
	);

VOID
nxrmfltInstanceTeardownStart (
	_In_ PCFLT_RELATED_OBJECTS			FltObjects,
	_In_ FLT_INSTANCE_TEARDOWN_FLAGS	Flags
	);

VOID
nxrmfltInstanceTeardownComplete (
	_In_ PCFLT_RELATED_OBJECTS			FltObjects,
	_In_ FLT_INSTANCE_TEARDOWN_FLAGS	Flags
	);

NTSTATUS
nxrmfltUnload (
	_In_ FLT_FILTER_UNLOAD_FLAGS	Flags
	);

NTSTATUS
nxrmfltInstanceQueryTeardown (
	_In_ PCFLT_RELATED_OBJECTS				FltObjects,
	_In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS	Flags
	);

FLT_PREOP_CALLBACK_STATUS
nxrmfltPreShutdown(
_Inout_ PFLT_CALLBACK_DATA				Data,
_In_ PCFLT_RELATED_OBJECTS				FltObjects,
_Flt_CompletionContext_Outptr_ PVOID	*CompletionContext
);

NTSTATUS
nxrmfltGenerateFileNameCallback(
_In_      PFLT_INSTANCE			Instance,
_In_      PFILE_OBJECT			FileObject,
_In_opt_  PFLT_CALLBACK_DATA	Data,
_In_      FLT_FILE_NAME_OPTIONS NameOptions,
_Out_     PBOOLEAN				CacheFileNameInformation,
_Inout_     PFLT_NAME_CONTROL		FileName
);

NTSTATUS
nxrmfltNormalizeNameComponentExCallback(
_In_     PFLT_INSTANCE				Instance,
_In_     PFILE_OBJECT				FileObject,
_In_     PCUNICODE_STRING			ParentDirectory,
_In_     USHORT						VolumeNameLength,
_In_     PCUNICODE_STRING			Component,
_Out_    PFILE_NAMES_INFORMATION	ExpandComponentName,
_In_     ULONG						ExpandComponentNameLength,
_In_     FLT_NORMALIZE_NAME_FLAGS	Flags,
_Inout_  PVOID						*NormalizationContext
);

VOID nxrmfltCreateProcessNotifyEx(
	_Inout_   PEPROCESS					Process,
	_In_      HANDLE					ProcessId,
	_In_opt_  PPS_CREATE_NOTIFY_INFO	CreateInfo
	);

VOID CtxCleanup(
	_In_  PVOID				ctx,
	_In_  FLT_CONTEXT_TYPE	ContextType
	);

VOID InstCtxCleanup(
	_In_  PVOID				ctx,
	_In_  FLT_CONTEXT_TYPE	ContextType
	);

BOOLEAN InitializeGlobalData(void);

void CleanupGlobalData(void);

extern ULONG NTAPI PsGetProcessSessionId(PEPROCESS Process);
extern BOOLEAN is_app_in_real_name_access_list(PEPROCESS  Process);
extern BOOLEAN is_app_in_graphic_integration_list(PEPROCESS  Process);

//
//  Assign text sections for each routine.
//

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, nxrmfltUnload)
#endif

const FLT_CONTEXT_REGISTRATION ContextRegistration[] =
{
	{ FLT_INSTANCE_CONTEXT,
	  0,
	  InstCtxCleanup,
	  sizeof(NXRMFLT_INSTANCE_CONTEXT),
	  NXRMFLT_INSTANCE_CTX_TAG
	},
	
	{ FLT_STREAMHANDLE_CONTEXT,
	  0,
	  CtxCleanup,
	  sizeof(NXRMFLT_STREAMHANDLE_CONTEXT),
	  NXRMFLT_STEAMHANDLE_CTX_TAG
	},

	{ FLT_STREAM_CONTEXT,
	  0,
	  CtxCleanup,
	  sizeof(NXRMFLT_STREAM_CONTEXT),
	  NXRMFLT_STREAM_CTX_TAG
	},

	{ FLT_CONTEXT_END }
};

//
//  operation registration
//

CONST FLT_OPERATION_REGISTRATION Callbacks[] = {

	{ IRP_MJ_CREATE,
	  0,
	  nxrmfltPreCreate,
	  nxrmfltPostCreate },

	//{ IRP_MJ_CLOSE,
	//  0,
	//  nxrmfltPreClose,
	//  NULL },

	{ IRP_MJ_WRITE,							// 0x04
	  0,
	  nxrmfltPreWrite,
	  nxrmfltPostWrite },

	//{ IRP_MJ_QUERY_INFORMATION,
	//  0,
	//  nxrmfltPreQueryInformation,
	//  NULL },

	{ IRP_MJ_SET_INFORMATION,
	  0,
	  nxrmfltPreSetInformation,
	  nxrmfltPostSetInformation },

	{ IRP_MJ_QUERY_EA,						// 0x07
	  0,
	  nxrmfltPreQueryEA,
	  NULL },

	{ IRP_MJ_SET_EA,						// 0x08
	  0,
	  nxrmfltPreSetEA,
	  nxrmfltPostSetEA },

	{ IRP_MJ_DIRECTORY_CONTROL,				// 0x0c
	  0,
	  nxrmfltPreDirectoryControl,
	  nxrmfltPostDirectoryControl },

	{ IRP_MJ_SHUTDOWN,
	  0,
	  nxrmfltPreShutdown,
	  NULL },                               //post operations not supported

	{ IRP_MJ_CLEANUP,
	  0,
	  nxrmfltPreCleanup,
	  NULL },

	{ IRP_MJ_SET_SECURITY,					// 0x15
	  0,
	  nxrmfltPreSetSecurity,
	  nxrmfltPostSetSecurity },

	{ IRP_MJ_NETWORK_QUERY_OPEN,
	  0,
	  nxrmfltPreQueryOpen,
	  NULL },

	{ IRP_MJ_OPERATION_END }
};

//
//  This defines what we want to filter with FltMgr
//

CONST FLT_REGISTRATION FilterRegistration = {

	sizeof(FLT_REGISTRATION),					//  Size
	FLT_REGISTRATION_VERSION,					//  Version
	0,											//  Flags

	ContextRegistration,						//  Context
	Callbacks,									//  Operation callbacks

	nxrmfltUnload,								//  MiniFilterUnload

	nxrmfltInstanceSetup,						//  InstanceSetup
	nxrmfltInstanceQueryTeardown,				//  InstanceQueryTeardown
	nxrmfltInstanceTeardownStart,				//  InstanceTeardownStart
	nxrmfltInstanceTeardownComplete,			//  InstanceTeardownComplete

	nxrmfltGenerateFileNameCallback,			//  GenerateFileName
	NULL,										//  GenerateDestinationFileName
	NULL,										//  NormalizeNameComponent
	NULL,										//	TransactionNotificationCallback
	nxrmfltNormalizeNameComponentExCallback		//	NormalizeNameComponentExCallback			

};



NTSTATUS
nxrmfltInstanceSetup (
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_In_ FLT_INSTANCE_SETUP_FLAGS Flags,
	_In_ DEVICE_TYPE VolumeDeviceType,
	_In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
	)
{
	NTSTATUS status = STATUS_SUCCESS;

	NXRMFLT_INSTANCE_CONTEXT *InstCtx = NULL;
	NXRMFLT_INSTANCE_CONTEXT *OldCtx = NULL;

	ULONG retLen = 0;

	PDEVICE_OBJECT devObj = NULL;

	BOOLEAN RevertGlobalReparseInstanceWhenError = FALSE;

	PT_DBG_PRINT(PTDBG_TRACE_ROUTINES,
				 ("nxrmflt!nxrmfltInstanceSetup: Entered\n"));

	do
	{
		//if (VolumeDeviceType != FILE_DEVICE_DISK_FILE_SYSTEM ||
		//	VolumeFilesystemType == FLT_FSTYPE_RAW)
		//{
		//	status = STATUS_FLT_DO_NOT_ATTACH;
		//	break;
		//}

		if (VolumeFilesystemType == FLT_FSTYPE_RAW)
		{
			status = STATUS_FLT_DO_NOT_ATTACH;
			break;
		}

		//
		// Using instance context instead of using volume context for better performance
		//
		status = FltAllocateContext(Global.Filter,
									FLT_INSTANCE_CONTEXT,
									sizeof(NXRMFLT_INSTANCE_CONTEXT),
									NonPagedPool,
									&InstCtx);

		if (!NT_SUCCESS(status))
		{
			status = STATUS_FLT_DO_NOT_ATTACH;
			break;
		}

		memset(InstCtx, 0, sizeof(NXRMFLT_INSTANCE_CONTEXT));

		InstCtx->VolumeProperties = (PFLT_VOLUME_PROPERTIES)InstCtx->VolPropBuffer;

		status = FltGetVolumeProperties(FltObjects->Volume,
										InstCtx->VolumeProperties,
										sizeof(InstCtx->VolPropBuffer),
										&retLen);
		if (!NT_SUCCESS(status))
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL,
						 ("Failed to query volume properties!!"));
		}

		//
		// do it regardless status
		//
		InstCtx->SectorSize = max(InstCtx->SectorSize, NXRMFLT_MIN_SECTOR_SIZE);

		//
		// for 8.0, we ignore removable device. We should change following code in 8.1
		//
		do 
		{
			status = FltGetDiskDeviceObject(FltObjects->Volume, &devObj);

			if (!NT_SUCCESS(status))
			{
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL,
							 ("Failed to get volume device object! Error is %x. Disable filtering.\n", status));

				InstCtx->DisableFiltering = TRUE;
				break;
			}

			status = IoVolumeDeviceToDosName(devObj, &InstCtx->VolDosName);

			if (!NT_SUCCESS(status))
			{
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL,
							 ("Failed to query volume DOS device name! Error is %x\n", status));

				//
				// 8.0 ONLY
				//
				InstCtx->DisableFiltering = TRUE;
				break;
			}

			status = nxrmfltGetDeviceInfo(devObj, 
										  InstCtx->DeviceName, 
										  sizeof(InstCtx->DeviceName),
										  InstCtx->SerialNumber,
										  sizeof(InstCtx->SerialNumber),
										  &InstCtx->BusType);

			if (!NT_SUCCESS(status))
			{
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL,
							 ("Failed to get device information! Error is %x. Disable filtering\n", status));

				InstCtx->DisableFiltering = TRUE;
				break;
			}

			if (InstCtx->BusType == BusTypeUsb ||
				InstCtx->BusType == BusType1394)
			{
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL,
							 ("Device %S is removable device. Disable filtering\n", InstCtx->DeviceName));

				InstCtx->DisableFiltering = TRUE;
				break;
			}

			InstCtx->DisableFiltering = FALSE;

		} while (FALSE);

		ExInitializeResourceLite(&InstCtx->Resource);

		status = FltCbdqInitialize(FltObjects->Instance,
								   &InstCtx->CallbackDataQueue,
								   nxrmfltCbdqInsertIo,
								   nxrmfltCbdqRemoveIo,
								   nxrmfltCbdqPeekNextIo,
								   nxrmfltCbdqAcquire,
								   nxrmfltCbdqRelease,
								   nxrmfltCbdqCompleteCanceledIo);

		InitializeListHead(&InstCtx->WorkItemQueue);

		KeInitializeSpinLock(&InstCtx->WorkItemQueueLock);

		InstCtx->WorkerThreadFlag = 0;

		KeInitializeEvent(&InstCtx->TeardownEvent, NotificationEvent, FALSE);

		InstCtx->Instance = FltObjects->Instance;

		status = FltIsVolumeWritable((PVOID)FltObjects->Instance, &InstCtx->IsWritable);

		if (!NT_SUCCESS(status))
		{
			InstCtx->IsWritable = TRUE;
		}

		InstCtx->VolumeFilesystemType = VolumeFilesystemType;

		////
		//// let's skip removable volumes in 8.0
		////
		//if (InstCtx->DisableFiltering)
		//{
		//	status = STATUS_FLT_DO_NOT_ATTACH;
		//	break;
		//}

		status = FltSetInstanceContext(FltObjects->Instance,
									   FLT_SET_CONTEXT_REPLACE_IF_EXISTS,
									   InstCtx,
									   &OldCtx);

		if (!NT_SUCCESS(status))
		{
			status = STATUS_FLT_DO_NOT_ATTACH;
			
			if (RevertGlobalReparseInstanceWhenError)
			{
				Global.ReparseInstance = NULL;
			}

			break;
		}

		FltAcquirePushLockExclusive(&Global.AttachedInstancesListLock);

		InsertTailList(&Global.AttachedInstancesList, &InstCtx->Link);

		FltReleasePushLock(&Global.AttachedInstancesListLock);

	} while (FALSE);

	if (OldCtx)
	{
		FltReleaseContext(OldCtx);
		OldCtx = NULL;
	}

	if (InstCtx)
	{
		FltReleaseContext(InstCtx);
		InstCtx = NULL;
	}

	if (devObj)
	{
		ObDereferenceObject(devObj);
		devObj = NULL;
	}

	return status;
}


NTSTATUS
nxrmfltInstanceQueryTeardown (
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
	)
{
	UNREFERENCED_PARAMETER( FltObjects );
	UNREFERENCED_PARAMETER( Flags );

	PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
				  ("nxrmflt!nxrmfltInstanceQueryTeardown: Entered\n") );

	return STATUS_SUCCESS;
}


VOID
nxrmfltInstanceTeardownStart (
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
	)
{
	NXRMFLT_INSTANCE_CONTEXT *InstCtx = NULL;
	NTSTATUS Status = STATUS_SUCCESS;

	rb_node *ite = NULL;
	rb_node *tmp = NULL;

	LIST_ENTRY *list_ite = NULL;
	LIST_ENTRY *list_tmp = NULL;

	LIST_ENTRY	DelayFreeCacheNodeList = { 0 };

	PT_DBG_PRINT(PTDBG_TRACE_TEARDOWN,
				 ("nxrmflt!nxrmfltInstanceTeardownStart: Entered\n"));

	//
	//  Get a pointer to the instance context.
	//

	do
	{
		InitializeListHead(&DelayFreeCacheNodeList);

		Status = FltGetInstanceContext(FltObjects->Instance, &InstCtx);

		if (!NT_SUCCESS(Status))
		{
			FLT_ASSERT(!"Instance Context is missing");
			return;
		}

		//
		//  Disable the insert to the callback data queue.
		//

		FltCbdqDisable(&InstCtx->CallbackDataQueue);

		//
		//  Remove all callback data from the queue and complete them.
		//

		nxrmfltEmptyQueueAndComplete(InstCtx);

		//
		//  Signal the worker thread if it is pended.
		//

		KeSetEvent(&InstCtx->TeardownEvent, 0, FALSE);

		FltAcquirePushLockExclusive(&Global.NxlFileCacheLock);

		RB_EACH_NODE_SAFE(ite, tmp, &Global.NxlFileCache)
		{
			NXL_CACHE_NODE *pNode = CONTAINING_RECORD(ite, NXL_CACHE_NODE, Node);

			if (pNode->Instance == FltObjects->Instance)
			{
				if (FlagOn(pNode->Flags, NXRMFLT_FLAG_CTX_ATTACHED))
				{
					SetFlag(pNode->Flags, NXRMFLT_FLAG_STOGRAGE_EJECTED);
				}
				else
				{
					//
					// remove from Cache but not freeing it here because we don't
					// want to hold a lock while freeing. We insert it into a delay list
					//

					DeleteNXLNodeInCache(&Global.NxlFileCache, pNode);

					InsertTailList(&DelayFreeCacheNodeList, &pNode->Link);
				}
			}
		}

		FltReleasePushLock(&Global.NxlFileCacheLock);

		NT_ASSERT(DelayFreeCacheNodeList.Flink);

		FOR_EACH_LIST_SAFE(list_ite, list_tmp, &DelayFreeCacheNodeList)
		{
			NXL_CACHE_NODE *pNode = CONTAINING_RECORD(list_ite, NXL_CACHE_NODE, Link);

			RemoveEntryList(list_ite);

			FreeNXLCacheNode(pNode);
		}

		FltAcquirePushLockExclusive(&Global.AttachedInstancesListLock);

		RemoveEntryList(&InstCtx->Link);

		FltReleasePushLock(&Global.AttachedInstancesListLock);

	} while (FALSE);

	if (InstCtx)
	{
		FltReleaseContext(InstCtx);
	}
}


VOID
nxrmfltInstanceTeardownComplete (
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
	)
{
	UNREFERENCED_PARAMETER( FltObjects );
	UNREFERENCED_PARAMETER( Flags );

	PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
				  ("nxrmflt!nxrmfltInstanceTeardownComplete: Entered\n") );
}


/*************************************************************************
	MiniFilter initialization and unload routines.
*************************************************************************/

NTSTATUS
DriverEntry (
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath
	)
{
	NTSTATUS status;

	BOOLEAN UnregisterCallback = FALSE;

	OBJECT_ATTRIBUTES	ExpireThreadAttributes = {0};

	UNREFERENCED_PARAMETER( RegistryPath );

	PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
				  ("nxrmflt!DriverEntry: Entered\n") );

	//
	//  Register with FltMgr to tell it our callback routines
	//

	status = FltRegisterFilter(DriverObject,
							   &FilterRegistration,
							   &Global.Filter);

	FLT_ASSERT(NT_SUCCESS(status));

	if (NT_SUCCESS(status)) 
	{

		InitializeGlobalData();

		status = NkCryptoInit();

		if(NT_SUCCESS(status))
		{
			do 
			{
				status = PsSetCreateProcessNotifyRoutineEx(nxrmfltCreateProcessNotifyEx, FALSE);

				if(!NT_SUCCESS(status))
				{
					break;
				}

				InitializeObjectAttributes(&ExpireThreadAttributes,
										   NULL,
										   OBJ_KERNEL_HANDLE,
										   NULL,
										   NULL);

				status = PsCreateSystemThread(&Global.ExpireThreadHandle,
											  THREAD_ALL_ACCESS,
											  &ExpireThreadAttributes,
											  NULL,
											  NULL,
											  nxrmfltExpireThreadProc,
											  NULL);

				if (!NT_SUCCESS(status))
				{
					break;
				}

				//
				//  Start filtering i/o
				//

				status = FltStartFiltering(Global.Filter);

				UnregisterCallback = TRUE;

			} while (FALSE);

			if (!NT_SUCCESS(status)) 
			{
				//
				// close server port
				//
				nxrmfltCloseServerPort();

				FltUnregisterFilter(Global.Filter);

				if(UnregisterCallback)
				{
					PsSetCreateProcessNotifyRoutineEx(nxrmfltCreateProcessNotifyEx, TRUE);
				}

				if (Global.ExpireThreadHandle)
				{
					KeSetEvent(&Global.ExpireStopEvent, IO_NO_INCREMENT, FALSE);

					ZwWaitForSingleObject(&Global.ExpireThreadHandle, FALSE, NULL);
				}

				CleanupGlobalData();

				NkCryptoCleanup();
			}
			else
			{

			}
		}
	}

	return status;
}

NTSTATUS
nxrmfltUnload (
	_In_ FLT_FILTER_UNLOAD_FLAGS Flags
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	rb_node *ite = NULL;

	PAGED_CODE();

	PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
				  ("nxrmflt!nxrmfltUnload: Entered\n") );
	
	do 
	{
		if (Flags != FLTFL_FILTER_UNLOAD_MANDATORY && Global.ClientPort)
		{
			Status = STATUS_FLT_DO_NOT_DETACH;
			break;
		}

		//
		// Set flag to let PreCreate know that it does not need to do anything anymore
		// We have to do this because we delete files here and PreCreate may create these deleted files again
		//
		Global.DriverUnloading = TRUE;

		FltAcquirePushLockExclusive(&Global.NxlFileCacheLock);

		//
		//
		RB_EACH_NODE(ite, &Global.NxlFileCache)
		{
			NXL_CACHE_NODE *pNode = CONTAINING_RECORD(ite, NXL_CACHE_NODE, Node);

			Status = nxrmfltDeleteFileByName(pNode->Instance, &pNode->ReparseFileName);

			if(!NT_SUCCESS(Status))
			{
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltUnload: Can't delete file %wZ! Status is %x\n", &pNode->ReparseFileName, Status));
			}
		}

		FltReleasePushLock(&Global.NxlFileCacheLock);

		//
		// close server port
		//
		nxrmfltCloseServerPort();

		//
		// IMPORTANTANT: Unregister filter first to prevent OS from calling any callback
		//
		FltUnregisterFilter(Global.Filter);

		PsSetCreateProcessNotifyRoutineEx(nxrmfltCreateProcessNotifyEx, TRUE);

		KeSetEvent(&Global.ExpireStopEvent, IO_NO_INCREMENT, FALSE);

		ZwWaitForSingleObject(Global.ExpireThreadHandle, FALSE, NULL);

		CleanupGlobalData();

		NkCryptoCleanup();

		Status = STATUS_SUCCESS;

	} while (FALSE);

	return Status;
}


/*************************************************************************
	MiniFilter callback routines.
*************************************************************************/

VOID CtxCleanup(
	_In_  PVOID				Context,
	_In_  FLT_CONTEXT_TYPE	ContextType
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	NXRMFLT_STREAM_CONTEXT *ctx = NULL;
	NXRMFLT_STREAMHANDLE_CONTEXT *ccb = NULL;

	rb_node *ite = NULL;
	NXL_CACHE_NODE	*pCacheNode = NULL;

	do
	{
		if (ContextType == FLT_STREAM_CONTEXT)
		{
			ctx = (NXRMFLT_STREAM_CONTEXT*)Context;

			FltAcquirePushLockShared(&Global.NxlFileCacheLock);

			pCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &ctx->FileName);

			if (pCacheNode)
			{
				if (FlagOn(pCacheNode->Flags, NXRMFLT_FLAG_CTX_ATTACHED))
				{
					ClearFlag(pCacheNode->Flags, NXRMFLT_FLAG_CTX_ATTACHED);
				}
			}

			FltReleasePushLock(&Global.NxlFileCacheLock);
			
			//
			// only delete file when there is a cache node
			//
			if (pCacheNode)
			{
				Status = nxrmfltDeleteFileByName(ctx->OriginalInstance, &ctx->FileName);

				if(!NT_SUCCESS(Status) && Status != STATUS_OBJECT_NAME_NOT_FOUND)
				{
					PT_DBG_PRINT(PTDBG_TRACE_CRITICAL,
						("nxrmflt!CtxCleanup: Failed to delete %wZ. Status is %x\n",
						&ctx->FileName,
						Status));
				}
			}

			if(ctx->ReleaseFileName)
			{
				ExFreeToPagedLookasideList(&Global.FullPathLookaside, ctx->FileName.Buffer);
			}
		
			RtlInitUnicodeString(&ctx->FileName, NULL);

			RtlInitUnicodeString(&ctx->FullPathParentDir, NULL);

			RtlInitUnicodeString(&ctx->FinalComponent, NULL);

			FltDeletePushLock(&ctx->CtxLock);

			InterlockedDecrement(&Global.TotalContext);
		}
		else if(ContextType == FLT_STREAMHANDLE_CONTEXT)
		{
			ccb = (NXRMFLT_STREAMHANDLE_CONTEXT*)Context;

			if(ccb->SourceFileNameInfo)
			{
				FltReleaseFileNameInformation(ccb->SourceFileNameInfo);
			}

			if(ccb->DestinationFileNameInfo)
			{
				FltReleaseFileNameInformation(ccb->DestinationFileNameInfo);
			}
		}
		else
		{

		}

	} while (FALSE);

	return;
}

VOID InstCtxCleanup(
	_In_  PVOID				ctx,
	_In_  FLT_CONTEXT_TYPE	ContextType
	)
{
	PNXRMFLT_INSTANCE_CONTEXT	InstCtx = NULL;

	do
	{
		if (ContextType != FLT_INSTANCE_CONTEXT)
		{
			break;
		}

		InstCtx = (PNXRMFLT_INSTANCE_CONTEXT)ctx;

		if (InstCtx->VolDosName.Buffer)
		{
			ExFreePool(InstCtx->VolDosName.Buffer);
		}

		ExDeleteResourceLite(&InstCtx->Resource);

	} while (FALSE);

	return;
}

BOOLEAN InitializeGlobalData(void)
{
	BOOLEAN bRet = TRUE;

	//const UCHAR keyId[0x2c] = {0x4e, 0x4c, 0x5f, 0x53, 0x48, 0x41, 0x52, 0x45, 0x76, 0x07, 0x5b, 0xb7, 0x6a, 0x2d, 0x7f, 0xda, 0x00, 0xea, 0xdc, 0x3e, 0xef, 0x07, 0x2d, 0x3a, 0x8d, 0x27, 0x56, 0xe1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb7, 0x48, 0x07, 0x54};
	//const UCHAR Key[32] = {0xfa, 0x1b, 0x3b, 0x89, 0x5b, 0xf6, 0x02, 0x7a, 0x06, 0xe0, 0x76, 0x57, 0xb5, 0x6f, 0xa2, 0x39, 0x42, 0xfb, 0x23, 0x31, 0x0d, 0x14, 0x9d, 0x44, 0xd3, 0xfb, 0xb7, 0x2a, 0xba, 0xe3, 0x6b, 0xdb};

	do
	{
		Global.TotalContext = 0;

		ExInitializePagedLookasideList(&Global.NXLCacheLookaside, 
									   NULL, 
									   NULL, 
									   0, 
									   sizeof(NXL_CACHE_NODE), 
									   NXRMFLT_NXLCACHE_TAG, 
									   0);

		ExInitializePagedLookasideList(&Global.FullPathLookaside,
									   NULL,
									   NULL,
									   0,
									   NXRMFLT_FULLPATH_BUFFER_SIZE,
									   NXRMFLT_FULLPATH_TAG,
									   0);

		ExInitializePagedLookasideList(&Global.DirCtlCtxLookaside,
									   NULL,
									   NULL,
									   0,
									   sizeof(NXRMFLT_DIRCTL_CONTEXT),
									   NXRMFLT_DIRCTLCTX_TAG,
									   0);

		ExInitializePagedLookasideList(&Global.NXLProcessCacheLookaside,
									   NULL,
									   NULL,
									   0,
									   sizeof(NXL_PROCESS_NODE),
									   NXRMFLT_PROCESSNODE_TAG,
									   0);

		ExInitializePagedLookasideList(&Global.RenameCacheLookaside,
									   NULL,
									   NULL,
									   0,
									   sizeof(NXL_RENAME_NODE),
									   NXRMFLT_RENAMENODE_TAG,
									   0);

		ExInitializePagedLookasideList(&Global.NXLRightsCacheLookaside,
									   NULL,
									   NULL,
									   0,
									   sizeof(NXL_RIGHTS_CACHE_NODE),
									   NXRMFLT_RIGHTSNODE_TAG,
									   0);

		ExInitializePagedLookasideList(&Global.NotificationLookaside,
									   NULL,
									   NULL,
									   0,
									   sizeof(NXRMFLT_NOTIFICATION),
									   NXRMFLT_NOTIFICATION_TAG,
									   0);

		ExInitializePagedLookasideList(&Global.SaveAsExpireLookaside,
									   NULL,
									   NULL,
									   0,
									   sizeof(NXL_SAVEAS_NODE),
									   NXRMFLT_SAVEASNODE_TAG,
									   0);

		ExInitializePagedLookasideList(&Global.SetInformationCtxLookaside,
									   NULL,
									   NULL,
									   0,
									   sizeof(NXRMFLT_SETINFORMATION_CONTEXT),
									   NXRMFLT_SETINFORMATION_TAG,
									   0);

		ExInitializePagedLookasideList(&Global.AdobeRenameLookaside,
									   NULL,
									   NULL,
									   0,
									   sizeof(ADOBE_RENAME_NODE),
									   NXRMFLT_ADOBERENAME_NODE_TAG,
									   0);

		ExInitializePagedLookasideList(&Global.NXLFileCreateCtxLookaside,
									   NULL,
									   NULL,
									   0,
									   sizeof(NXLFILE_CREATE_CONTEXT),
									   NXRMFLT_NXLFILECREATECTX_TAG,
									   0);

		ExInitializePagedLookasideList(&Global.SetReadOnlyAttrCtxLookaside,
									   NULL,
									   NULL,
									   0,
									   sizeof(SET_READONLY_ATTR_CONTEXT),
									   NXRMFLT_SETREADONLYATTRCTX_TAG,
									   0);

		RtlInitUnicodeString(&Global.NXLFileDotExtsion, NXRMFLT_NXL_DOTEXT);

		RtlInitUnicodeString(&Global.NXLFileExtsion, NXRMFLT_NXL_EXTENSION);

		Global.HideNXLExtension = TRUE;

		//
		// communication related
		//

		nxrmfltPrepareServerPort();

		//
		// NXL file cache related
		//
		Global.NxlFileCount = 0;

		Global.NxlFileCache.rb_node = NULL;

		FltInitializePushLock(&Global.NxlFileCacheLock);

		//
		// Driver unloading related
		//
		Global.DriverUnloading = FALSE;

		memset(&Global.PrimaryKey, 0, sizeof(Global.PrimaryKey));

		//Global.PrimaryKey.KeyId.Algorithm	= 0x2;
		//Global.PrimaryKey.KeyId.IdSize		= 0x2c;
		//memcpy(Global.PrimaryKey.KeyId.Id, keyId, min(sizeof(Global.PrimaryKey.KeyId.Id),sizeof(keyId)));
		//
		//Global.PrimaryKey.keySize			= 0x20;
		//memcpy(Global.PrimaryKey.Key, Key, min(sizeof(Global.PrimaryKey.Key),sizeof(Key)));

		FltInitializePushLock(&Global.PrimaryKeyLock);

		InitializeListHead(&Global.KeyChain);

		FltInitializePushLock(&Global.KeyChainLock);

		InitializeListHead(&Global.NxlProcessList);

		FltInitializePushLock(&Global.NxlProcessListLock);

		InitializeListHead(&Global.RenameList);

		InitializeListHead(&Global.ExpireTable);
		
		FltInitializePushLock(&Global.ExpireTableLock);

		InitializeListHead(&Global.AdobeRenameExpireTable);

		FltInitializePushLock(&Global.AdobeRenameExpireTableLock);

		InitializeListHead(&Global.AttachedInstancesList);

		FltInitializePushLock(&Global.AttachedInstancesListLock);

		Global.ExpireThreadHandle = NULL;

		KeInitializeEvent(&Global.ExpireStopEvent, NotificationEvent, FALSE);

	} while (FALSE);

	return bRet;
}

void CleanupGlobalData(void)
{
	rb_node *rb_tmp = NULL;
	rb_node *rb_ite = NULL;

	LIST_ENTRY *tmp = NULL;
	LIST_ENTRY *ite = NULL;

	if (Global.ExpireThreadHandle)
	{
		ZwClose(Global.ExpireThreadHandle);
		Global.ExpireThreadHandle = NULL;
	}

	//
	// clean up NxlFileCache
	//
	RB_EACH_NODE_SAFE(rb_ite, rb_tmp, &Global.NxlFileCache)
	{
		NXL_CACHE_NODE *pNode = CONTAINING_RECORD(rb_ite, NXL_CACHE_NODE, Node);

		DeleteNXLNodeInCache(&Global.NxlFileCache, pNode);

		FreeNXLCacheNode(pNode);
	}

	//
	// delete lookaside after freeing all records to lookaside
	//
	ExDeletePagedLookasideList(&Global.NXLCacheLookaside);

	FltDeletePushLock(&Global.NxlFileCacheLock);

	//
	// clean up PrimayKey
	//
	FltDeletePushLock(&Global.PrimaryKeyLock);

	//
	// clean up process cache
	//
	FOR_EACH_LIST_SAFE(ite, tmp, &Global.NxlProcessList)
	{
		NXL_PROCESS_NODE *pNode = CONTAINING_RECORD(ite, NXL_PROCESS_NODE, Link);

		RemoveEntryList(ite);

		ExWaitForRundownProtectionRelease(&pNode->NodeRundownRef);

		ExRundownCompleted(&pNode->NodeRundownRef);

		nxrmfltFreeProcessNode(pNode);
	}

	FltDeletePushLock(&Global.NxlProcessListLock);

	ite = NULL;
	tmp = NULL;

	//
	// clean up RenameList
	//
	FOR_EACH_LIST_SAFE(ite, tmp, &Global.RenameList)
	{
		NXL_RENAME_NODE *pNode = CONTAINING_RECORD(ite, NXL_RENAME_NODE, Link);

		NT_ASSERT(ite->Flink);

		RemoveEntryList(ite);

		nxrmfltFreeRenameNode(pNode);

		ExFreeToPagedLookasideList(&Global.RenameCacheLookaside, pNode);
	}

	FltDeletePushLock(&Global.RenameListLock);

	ite = NULL;
	tmp = NULL;

	//
	// cleanup Expire table
	//
	FOR_EACH_LIST_SAFE(ite, tmp, &Global.ExpireTable)
	{
		NXL_SAVEAS_NODE *pNode = CONTAINING_RECORD(ite, NXL_SAVEAS_NODE, Link);

		NT_ASSERT(ite->Flink);

		RemoveEntryList(ite);

		//
		// Wait for all other threads rundown
		//
		ExWaitForRundownProtectionRelease(&pNode->NodeRundownRef);

		ExRundownCompleted(&pNode->NodeRundownRef);

		ExFreeToPagedLookasideList(&Global.SaveAsExpireLookaside, pNode);
	}

	FltDeletePushLock(&Global.ExpireTableLock);

	ite = NULL;
	tmp = NULL;

	//
	// cleanup adobe rename expire table
	//
	FOR_EACH_LIST_SAFE(ite, tmp, &Global.AdobeRenameExpireTable)
	{
		ADOBE_RENAME_NODE *pNode = CONTAINING_RECORD(ite, ADOBE_RENAME_NODE, Link);

		NT_ASSERT(ite->Flink);

		RemoveEntryList(ite);

		//
		// Wait for all other threads rundown
		//
		ExWaitForRundownProtectionRelease(&pNode->NodeRundownRef);

		ExRundownCompleted(&pNode->NodeRundownRef);

		ExFreeToPagedLookasideList(&Global.AdobeRenameLookaside, pNode);
	}

	FltDeletePushLock(&Global.AdobeRenameExpireTableLock);

	//
	// Cleanup AttachedInstancesList
	//
	NT_ASSERT(IsListEmpty(&Global.AttachedInstancesList));

	FltDeletePushLock(&Global.AttachedInstancesListLock);

	//
	// delete lookaside after freeing all records to lookaside
	//
	ExDeletePagedLookasideList(&Global.NXLProcessCacheLookaside);

	ExDeletePagedLookasideList(&Global.RenameCacheLookaside);

	ExDeletePagedLookasideList(&Global.FullPathLookaside);

	ExDeletePagedLookasideList(&Global.DirCtlCtxLookaside);

	ExDeletePagedLookasideList(&Global.NXLRightsCacheLookaside);

	ExDeletePagedLookasideList(&Global.NotificationLookaside);

	ExDeletePagedLookasideList(&Global.SaveAsExpireLookaside);

	ExDeletePagedLookasideList(&Global.SetInformationCtxLookaside);
	
	ExDeletePagedLookasideList(&Global.AdobeRenameLookaside);

	ExDeletePagedLookasideList(&Global.NXLFileCreateCtxLookaside);

	ExDeletePagedLookasideList(&Global.SetReadOnlyAttrCtxLookaside);

	return;
}

FLT_PREOP_CALLBACK_STATUS
nxrmfltPreShutdown(
_Inout_ PFLT_CALLBACK_DATA				Data,
_In_ PCFLT_RELATED_OBJECTS				FltObjects,
_Flt_CompletionContext_Outptr_ PVOID	*CompletionContext
)
{
	NTSTATUS Status = STATUS_SUCCESS;

	rb_node *ite = NULL;

	PT_DBG_PRINT(PTDBG_TRACE_TEARDOWN,
				 ("nxrmflt!nxrmfltPreShutdown: Entered. Total ctx is %d\n", Global.TotalContext));

	//
	// Nodes are deleted when stopping the driver
	// However, in case of shutdown, we delete them here
	//
	RB_EACH_NODE(ite, &Global.NxlFileCache)
	{
		NXL_CACHE_NODE *pNode = CONTAINING_RECORD(ite, NXL_CACHE_NODE, Node);

		Status = nxrmfltDeleteFileByName(pNode->Instance, &pNode->ReparseFileName);

		if(!NT_SUCCESS(Status))
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPreShutdown: Can't delete file %wZ! Status is %x\n", &pNode->ReparseFileName, Status));
		}
	}

	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

NTSTATUS
nxrmfltGenerateFileNameCallback(
_In_      PFLT_INSTANCE			Instance,
_In_      PFILE_OBJECT			FileObject,
_In_opt_  PFLT_CALLBACK_DATA	Data,
_In_      FLT_FILE_NAME_OPTIONS NameOptions,
_Out_     PBOOLEAN				CacheFileNameInformation,
_Inout_   PFLT_NAME_CONTROL		FileName
)
{
	PFLT_FILE_NAME_INFORMATION userFileNameInfo = NULL;
	PUNICODE_STRING userFileName;
	NTSTATUS Status = STATUS_SUCCESS;

	do 
	{
		//
		//  Clear FLT_FILE_NAME_REQUEST_FROM_CURRENT_PROVIDER from the name options
		//  We pass the same name options when we issue a name query to satisfy this
		//  name query. We want that name query to be targeted below nxrmflt.sys and 
		//  not recurse into nxrmflt.sys
		//

		ClearFlag(NameOptions, FLT_FILE_NAME_REQUEST_FROM_CURRENT_PROVIDER);

		if (FileObject->FsContext == NULL) 
		{


			//
			//  This file object has not yet been opened.  We will query the filter 
			//  manager for the name and return that name. We must use the original
			//  NameOptions we received in the query. If we were to swallow flags 
			//  such as FLT_FILE_NAME_QUERY_FILESYSTEM_ONLY or 
			//  FLT_FILE_NAME_DO_NOT_CACHE we could corrupt the name cache.
			//

			NT_ASSERT(Data);

			Status = FltGetFileNameInformation(Data,
											   NameOptions,
											   &userFileNameInfo);

			if (!NT_SUCCESS(Status)) 
			{

				break;
			}

			userFileName = &userFileNameInfo->Name;

		}
		else 
		{

			//
			//  The file has been opened. If the call is not in the context of an IO 
			//  operation (we don't have a callback data), we have to get the 
			//  filename with FltGetFilenameInformationUnsafe using the fileobject.
			//  Note, the only way we won't have a callback is if someone called 
			//  FltGetFileNameInformationUnsafe already.
			//

			if (ARGUMENT_PRESENT(Data)) 
			{

				Status = FltGetFileNameInformation(Data,
												   NameOptions,
												   &userFileNameInfo);

			}
			else
			{

				Status = FltGetFileNameInformationUnsafe(FileObject,
														 Instance,
														 NameOptions,
														 &userFileNameInfo);

			}

			if (!NT_SUCCESS(Status)) 
			{
				break;
			}

			userFileName = &userFileNameInfo->Name;

		}

		Status = FltCheckAndGrowNameControl(FileName,
											userFileName->Length);

		if (!NT_SUCCESS(Status)) 
		{
			break;
		}

		RtlCopyUnicodeString(&FileName->Name, userFileName);

		//
		//  If the file object is unopened then the name of the stream represented by 
		//  the file object may change from pre-create to post-create. 
		//  For example, the name being opened could actually be a symbolic link
		//

		*CacheFileNameInformation = (FileObject->FsContext != NULL);

	} while (FALSE);


	if (userFileNameInfo != NULL) 
	{
		FltReleaseFileNameInformation(userFileNameInfo);
	}

	if (!NT_SUCCESS(Status))
	{

		PT_DBG_PRINT(PTDBG_TRACE_CRITICAL,
					 ("nxrmflt!nxrmfltGenerateFileNameCallback: failed %x\n",Status));
	}

	return Status;

}

NTSTATUS
nxrmfltNormalizeNameComponentExCallback(
_In_     PFLT_INSTANCE				Instance,
_In_     PFILE_OBJECT				FileObject,
_In_     PCUNICODE_STRING			ParentDirectory,
_In_     USHORT						VolumeNameLength,
_In_     PCUNICODE_STRING			Component,
_Out_    PFILE_NAMES_INFORMATION	ExpandComponentName,
_In_     ULONG						ExpandComponentNameLength,
_In_     FLT_NORMALIZE_NAME_FLAGS	Flags,
_Inout_  PVOID						*NormalizationContext
)
{
	NTSTATUS					Status = STATUS_SUCCESS;
	HANDLE						directoryHandle = NULL;
	PFILE_OBJECT				directoryFileObject = NULL;
	OBJECT_ATTRIBUTES			objAttributes = { 0 };
	IO_STATUS_BLOCK				ioStatusBlock = { 0 };
	BOOLEAN						ignoreCase = !BooleanFlagOn(Flags, FLTFL_NORMALIZE_NAME_CASE_SENSITIVE);

	//
	//  Validate the buffer is big enough
	//

	if (ExpandComponentNameLength < sizeof(FILE_NAMES_INFORMATION))
	{
		return STATUS_INVALID_PARAMETER;
	}

	InitializeObjectAttributes(&objAttributes,
		(PUNICODE_STRING)ParentDirectory,
		OBJ_KERNEL_HANDLE | (ignoreCase ? OBJ_CASE_INSENSITIVE : 0),
		NULL,
		NULL);

	Status = FltCreateFileEx2(Global.Filter,
		Instance,
		&directoryHandle,
		&directoryFileObject,
		FILE_LIST_DIRECTORY | SYNCHRONIZE, // DesiredAccess
		&objAttributes,
		&ioStatusBlock,
		NULL,                              // AllocationSize
		FILE_ATTRIBUTE_DIRECTORY
		| FILE_ATTRIBUTE_NORMAL,         // FileAttributes
		FILE_SHARE_READ
		| FILE_SHARE_WRITE
		| FILE_SHARE_DELETE,             // ShareAccess
		FILE_OPEN,                         // CreateDisposition
		FILE_DIRECTORY_FILE
		| FILE_SYNCHRONOUS_IO_NONALERT
		| FILE_OPEN_FOR_BACKUP_INTENT,   // CreateOptions
		NULL,                              // EaBuffer
		0,                                 // EaLength
		IO_IGNORE_SHARE_ACCESS_CHECK,      // Flags
		NULL);


	if (!NT_SUCCESS(Status))
	{

		goto nxrmfltNormalizeNameComponentExCleanup;
	}

	//
	//  Query the file entry to get the long name
	//

	Status = FltQueryDirectoryFile(Instance,
		directoryFileObject,
		ExpandComponentName,
		ExpandComponentNameLength,
		FileNamesInformation,
		TRUE,					/* ReturnSingleEntry */
		(PUNICODE_STRING)Component,
		TRUE,					/* restartScan */
		NULL);


nxrmfltNormalizeNameComponentExCleanup:

	if (NULL != directoryHandle)
	{
		FltClose(directoryHandle);
	}

	if (NULL != directoryFileObject)
	{
		ObDereferenceObject(directoryFileObject);
	}

	return Status;

}

VOID nxrmfltCreateProcessNotifyEx(
	_Inout_   PEPROCESS					Process,
	_In_      HANDLE					ProcessId,
	_In_opt_  PPS_CREATE_NOTIFY_INFO	CreateInfo
	)
{
	NXL_PROCESS_NODE *pNode = NULL;
	NXL_PROCESS_NODE *pTmpNode = NULL;

	LIST_ENTRY *ite = NULL;

	ULONGLONG	ProcessFlags = 0;

	FltAcquirePushLockShared(&Global.NxlProcessListLock);

	FOR_EACH_LIST(ite, &Global.NxlProcessList)
	{
		pNode = CONTAINING_RECORD(ite, NXL_PROCESS_NODE, Link);

		if(pNode->ProcessId == ProcessId)
		{
			break;
		}
		else
		{
			pNode = NULL;
		}
	}

	if(pNode)
	{
		if (!ExAcquireRundownProtection(&pNode->NodeRundownRef))
		{
			pNode = NULL;
		}
	}

	FltReleasePushLock(&Global.NxlProcessListLock);

	if(pNode && CreateInfo == NULL)
	{
		NXL_SAVEAS_NODE	*pSaveAsNode = NULL;

		if (pNode->HideXNLExtension)
			SetFlag(ProcessFlags, NXRM_PROCESS_FLAG_HIDE_NXL_EXT);

		if (pNode->HasGraphicIntegration)
			SetFlag(ProcessFlags, NXRM_PROCESS_FLAG_HAS_OVERLAY_INTEGRATION);

		ExReleaseRundownProtection(&pNode->NodeRundownRef);

		ExWaitForRundownProtectionRelease(&pNode->NodeRundownRef);
		
		ExRundownCompleted(&pNode->NodeRundownRef);

		FltAcquirePushLockExclusive(&Global.NxlProcessListLock);

		RemoveEntryList(&pNode->Link);

		FltReleasePushLock(&Global.NxlProcessListLock);

		nxrmfltFreeProcessNode(pNode);

		FltAcquirePushLockShared(&Global.ExpireTableLock);

		FOR_EACH_LIST(ite, &Global.ExpireTable)
		{
			pSaveAsNode = CONTAINING_RECORD(ite, NXL_SAVEAS_NODE, Link);

			if (pSaveAsNode->ProcessId == ProcessId)
			{
				pSaveAsNode->ExpireTick = MAX_ULONG;	// make it expire
			}
		}

		FltReleasePushLock(&Global.ExpireTableLock);
	}
	else if(pNode == NULL && CreateInfo)
	{
		pNode = ExAllocateFromPagedLookasideList(&Global.NXLProcessCacheLookaside);

		if(pNode)
		{
			pNode->AlwaysGrantAcess		= FALSE;
			pNode->AlwaysDenyAccess		= FALSE;
			pNode->ProcessId			= ProcessId;
			pNode->SessionId			= PsGetProcessSessionId(Process);
			pNode->RightsCache.rb_node	= NULL;
			pNode->HideXNLExtension		= is_app_in_real_name_access_list(Process) ? FALSE : TRUE;
			pNode->HasGraphicIntegration= is_app_in_graphic_integration_list(Process);

			FltInitializePushLock(&pNode->RightsCacheLock);

			ExInitializeRundownProtection(&pNode->NodeRundownRef);

			//
			// search the list again because we released lock
			//
			FltAcquirePushLockExclusive(&Global.NxlProcessListLock);

			FOR_EACH_LIST(ite, &Global.NxlProcessList)
			{
				pTmpNode = CONTAINING_RECORD(ite, NXL_PROCESS_NODE, Link);

				if(pTmpNode->ProcessId == ProcessId)
				{
					break;
				}
				else
				{
					pTmpNode = NULL;
				}
			}

			if(!pTmpNode)
			{
				if (pNode->HideXNLExtension)
					SetFlag(ProcessFlags, NXRM_PROCESS_FLAG_HIDE_NXL_EXT);

				if (pNode->HasGraphicIntegration)
					SetFlag(ProcessFlags, NXRM_PROCESS_FLAG_HAS_OVERLAY_INTEGRATION);

				InsertHeadList(&Global.NxlProcessList, &pNode->Link);
			}

			FltReleasePushLock(&Global.NxlProcessListLock);

			//
			// there is a same record in the list already
			if(pTmpNode)
			{
				nxrmfltFreeProcessNode(pNode);
			}
		}
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		//
		// there is one process existing but it's not in my cache
		// this is possible when the existing process created before
		// nxrmflt start running
		//
		//////////////////////////////////////////////////////////////////////////
	}

	if (!pTmpNode)
	{
		nxrmfltSendProcessNotification(ProcessId,
									   PsGetProcessSessionId(Process),
									   CreateInfo ? TRUE : FALSE,
									   ProcessFlags,
									   CreateInfo ? CreateInfo->ImageFileName : NULL);
	}

	return;
}