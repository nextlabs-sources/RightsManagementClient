#include "nxrmfltdef.h"
#include "nxrmfltnxlcachemgr.h"
#include "nxrmfltqueryopen.h"

extern DECLSPEC_CACHEALIGN PFLT_FILTER			gFilterHandle;
extern DECLSPEC_CACHEALIGN ULONG				gTraceFlags;
extern DECLSPEC_CACHEALIGN NXRMFLT_GLOBAL_DATA	Global;

FLT_PREOP_CALLBACK_STATUS
nxrmfltPreQueryOpen(
_Inout_ PFLT_CALLBACK_DATA				Data,
_In_ PCFLT_RELATED_OBJECTS				FltObjects,
_Flt_CompletionContext_Outptr_ PVOID	*CompletionContext
)
{
	FLT_PREOP_CALLBACK_STATUS CallbackStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;

	PFLT_INSTANCE	FltInstance = NULL;

	NTSTATUS Status = STATUS_SUCCESS;

	PFLT_FILE_NAME_INFORMATION pNameInfo = NULL;
	NXL_CACHE_NODE	*pCacheNode = NULL;

	NXRMFLT_INSTANCE_CONTEXT	*InstCtx = NULL;

	FltInstance = FltObjects->Instance;

	NT_ASSERT(FLT_IS_FASTIO_OPERATION(Data));
	NT_ASSERT(!FlagOn(Data->Iopb->OperationFlags, SL_OPEN_TARGET_DIRECTORY));

	do 
	{
		if (FlagOn(Data->Iopb->OperationFlags, SL_OPEN_PAGING_FILE))
		{
			break;
		}

		if (!Global.ClientPort)
		{
			break;
		}

		Status = FltGetInstanceContext(FltInstance, &InstCtx);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		if (InstCtx->DisableFiltering)
		{
			break;
		}

		//if (FltObjects->Instance == Global.ReparseInstance)
		//{
		//	break;
		//}

		Status = FltGetFileNameInformation(Data,
										   FLT_FILE_NAME_NORMALIZED,
										   &pNameInfo);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		Status = FltParseFileNameInformation(pNameInfo);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		FltAcquirePushLockShared(&Global.NxlFileCacheLock);

		pCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &pNameInfo->Name);

		if (pCacheNode)
		{
			CallbackStatus = FLT_PREOP_DISALLOW_FASTIO;
		}

		FltReleasePushLock(&Global.NxlFileCacheLock);

	} while (FALSE);

	if (pNameInfo)
	{
		FltReleaseFileNameInformation(pNameInfo);
	}

	if (InstCtx)
	{
		FltReleaseContext(InstCtx);
		InstCtx = NULL;
	}

	return CallbackStatus;
}
