#include "nxrmfltdef.h"
#include "nxrmfltwrite.h"
#include "nxrmfltnxlcachemgr.h"

extern DECLSPEC_CACHEALIGN PFLT_FILTER			gFilterHandle;
extern DECLSPEC_CACHEALIGN ULONG				gTraceFlags;
extern DECLSPEC_CACHEALIGN NXRMFLT_GLOBAL_DATA	Global;

FLT_PREOP_CALLBACK_STATUS
nxrmfltPreWrite(
_Inout_ PFLT_CALLBACK_DATA				Data,
_In_ PCFLT_RELATED_OBJECTS				FltObjects,
_Flt_CompletionContext_Outptr_ PVOID	*CompletionContext
)
{
	FLT_PREOP_CALLBACK_STATUS CallbackStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;

	NTSTATUS	Status = STATUS_SUCCESS;

	PFLT_INSTANCE			FltInstance = NULL;
	PFILE_OBJECT			FileObject = NULL;

	PNXRMFLT_STREAM_CONTEXT		Ctx = NULL;


	FltInstance = FltObjects->Instance;
	FileObject = FltObjects->FileObject;

	do 
	{
		Status = FltGetStreamContext(FltInstance, FileObject, &Ctx);

		if (!Ctx)
		{
			break;
		}

		*CompletionContext = Ctx;

		CallbackStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;

	} while (FALSE);

	return CallbackStatus;
}

FLT_POSTOP_CALLBACK_STATUS
nxrmfltPostWrite(
_Inout_ PFLT_CALLBACK_DATA		Data,
_In_ PCFLT_RELATED_OBJECTS		FltObjects,
_In_opt_ PVOID					CompletionContext,
_In_ FLT_POST_OPERATION_FLAGS	Flags
)
{
	FLT_POSTOP_CALLBACK_STATUS	CallbackStatus = FLT_POSTOP_FINISHED_PROCESSING;

	NTSTATUS	Status = STATUS_SUCCESS;
	ULONG_PTR	Information = 0;

	PFLT_INSTANCE			FltInstance = NULL;
	PFILE_OBJECT			FileObject = NULL;
	
	PNXRMFLT_STREAM_CONTEXT		Ctx = (PNXRMFLT_STREAM_CONTEXT)CompletionContext;

	NXL_CACHE_NODE	*pCacheNode = NULL;

	BOOLEAN	MediaEjected = FALSE;

	FltInstance = FltObjects->Instance;
	FileObject = FltObjects->FileObject;

	do
	{
		Status = Data->IoStatus.Status;
		Information = Data->IoStatus.Information;

		if ((!NT_SUCCESS(Status)) || (NT_SUCCESS(Status) && Information == 0))
		{
			break;
		}

		if (Ctx)
		{
			InterlockedExchange(&Ctx->ContentDirty, 1);

			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostWrite: Set content dirty flag.\n"));
		}
	
	} while (FALSE);

	if (Ctx)
	{
		FltReleaseContext(Ctx);
	}

	return CallbackStatus;
}
