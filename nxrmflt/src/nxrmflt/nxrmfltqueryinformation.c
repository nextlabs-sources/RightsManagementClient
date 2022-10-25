#include "nxrmfltdef.h"
#include "nxrmfltnxlcachemgr.h"
#include "nxrmfltqueryinformation.h"
#include "nxrmfltutils.h"

extern DECLSPEC_CACHEALIGN PFLT_FILTER			gFilterHandle;
extern DECLSPEC_CACHEALIGN ULONG				gTraceFlags;
extern DECLSPEC_CACHEALIGN NXRMFLT_GLOBAL_DATA	Global;

FLT_PREOP_CALLBACK_STATUS
	nxrmfltPreQueryInformation(
	_Inout_ PFLT_CALLBACK_DATA				Data,
	_In_ PCFLT_RELATED_OBJECTS				FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID	*CompletionContext
	)
{
	FLT_PREOP_CALLBACK_STATUS	CallbackStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;

	//NTSTATUS Status = STATUS_SUCCESS;

	//PFLT_INSTANCE			FltInstance = NULL;
	//PFILE_OBJECT			FileObject = NULL;

	//FILE_INFORMATION_CLASS FileInformationClass = 0;
	//FILE_NAME_INFORMATION	*FileNameInfo = NULL;

	//ULONG Length = 0;
	//PCHAR InfoBuffer = NULL;

	//PNXRMFLT_STREAM_CONTEXT		Ctx = NULL;

	//NXL_CACHE_NODE	*pCacheNode = NULL;

	//BOOLEAN	MediaEjected = FALSE;

	//ULONG LengthToCopy = 0;
	//
	//FltInstance = FltObjects->Instance;
	//FileObject = FltObjects->FileObject;

	//FileInformationClass	= Data->Iopb->Parameters.QueryFileInformation.FileInformationClass;
	//Length					= Data->Iopb->Parameters.QueryFileInformation.Length;
	//InfoBuffer				= Data->Iopb->Parameters.QueryFileInformation.InfoBuffer;

	//do 
	//{
	//	if (FileInformationClass != FileNameInformation)
	//	{
	//		break;
	//	}

	//	Status = FltGetStreamContext(FltInstance, FileObject, &Ctx);

	//	if (!Ctx)
	//	{
	//		break;
	//	}

	//	if(Length < sizeof(FILE_NAME_INFORMATION))
	//	{
	//		break;
	//	}
	//	
	//	FileNameInfo = (FILE_NAME_INFORMATION *)InfoBuffer;

	//	FltAcquirePushLockShared(&Global.NxlFileCacheLock);

	//	pCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &Ctx->FileName);

	//	if (pCacheNode)
	//	{
	//		if (FlagOn(pCacheNode->Flags, NXRMFLT_FLAG_STOGRAGE_EJECTED))
	//		{
	//			MediaEjected = TRUE;
	//		}

	//		ExAcquireRundownProtection(&pCacheNode->NodeRundownRef);
	//	}
	//	else
	//	{
	//		PT_DBG_PRINT(PTDBG_TRACE_CRITICAL,("nxrmflt!FATAL ERROR!!! Can't find cached record --> PreQueryInformation"));
	//	}

	//	FltReleasePushLock(&Global.NxlFileCacheLock);

	//	if(MediaEjected)
	//	{
	//		Data->IoStatus.Status		= STATUS_VOLUME_DISMOUNTED;
	//		Data->IoStatus.Information	= 0;

	//		CallbackStatus = FLT_PREOP_COMPLETE;

	//		break;
	//	}
	//	
	//	__try
	//	{

	//		FileNameInfo->FileNameLength = LengthToCopy = Ctx->ParentDir.Length + Ctx->FinalComponent.Length;

	//		Length -= sizeof(ULONG);

	//		if(LengthToCopy > Length)
	//		{
	//			LengthToCopy = Length;

	//			Data->IoStatus.Status = STATUS_BUFFER_OVERFLOW;
	//		}

	//		memcpy(FileNameInfo->FileName, 
	//			   Ctx->ParentDir.Buffer, 
	//			   LengthToCopy);

	//		Data->IoStatus.Information = LengthToCopy + sizeof(ULONG);

	//		CallbackStatus = FLT_PREOP_COMPLETE;
	//	}
	//	__except (nxrmfltExceptionFilter(NULL, GetExceptionInformation()))
	//	{

	//	}

	//} while (FALSE);

	//if(pCacheNode)
	//{
	//	ExReleaseRundownProtection(&pCacheNode->NodeRundownRef);
	//}

	//if(Ctx)
	//{
	//	FltReleaseContext(Ctx);
	//}

	return CallbackStatus;
}
