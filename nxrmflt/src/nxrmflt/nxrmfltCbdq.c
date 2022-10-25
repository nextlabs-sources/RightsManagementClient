#include "nxrmfltdef.h"
#include "nxrmfltCbdq.h"

extern DECLSPEC_CACHEALIGN ULONG				gTraceFlags;
extern DECLSPEC_CACHEALIGN NXRMFLT_GLOBAL_DATA	Global;

VOID
_IRQL_requires_max_(DISPATCH_LEVEL)
_IRQL_raises_(DISPATCH_LEVEL)
_Requires_lock_not_held_((CONTAINING_RECORD(DataQueue, NXRMFLT_INSTANCE_CONTEXT, CallbackDataQueue))->WorkItemQueueLock)
_Acquires_lock_((CONTAINING_RECORD(DataQueue, NXRMFLT_INSTANCE_CONTEXT, CallbackDataQueue))->WorkItemQueueLock)
nxrmfltCbdqAcquire(_In_ PFLT_CALLBACK_DATA_QUEUE DataQueue, _Out_ PKIRQL Irql)
{
	NXRMFLT_INSTANCE_CONTEXT *InstCtx = NULL;

	InstCtx = CONTAINING_RECORD(DataQueue, NXRMFLT_INSTANCE_CONTEXT, CallbackDataQueue);

	KeAcquireSpinLock(&InstCtx->WorkItemQueueLock, Irql);
}

VOID
_IRQL_requires_max_(DISPATCH_LEVEL)
_IRQL_requires_min_(DISPATCH_LEVEL)
_IRQL_raises_(PASSIVE_LEVEL)
_Requires_lock_held_((CONTAINING_RECORD(DataQueue, NXRMFLT_INSTANCE_CONTEXT, CallbackDataQueue))->WorkItemQueueLock)
_Releases_lock_((CONTAINING_RECORD(DataQueue, NXRMFLT_INSTANCE_CONTEXT, CallbackDataQueue))->WorkItemQueueLock)
nxrmfltCbdqRelease(_In_ PFLT_CALLBACK_DATA_QUEUE DataQueue, _In_ KIRQL Irql)
{
	NXRMFLT_INSTANCE_CONTEXT *InstCtx = NULL;

	InstCtx = CONTAINING_RECORD(DataQueue, NXRMFLT_INSTANCE_CONTEXT, CallbackDataQueue);

	KeReleaseSpinLock(&InstCtx->WorkItemQueueLock, Irql);
}

NTSTATUS nxrmfltCbdqInsertIo(_In_ PFLT_CALLBACK_DATA_QUEUE	DataQueue, _In_ PFLT_CALLBACK_DATA Data, _In_opt_ PVOID Context)
{
	NTSTATUS status = STATUS_SUCCESS;

	NXRMFLT_INSTANCE_CONTEXT *InstCtx = NULL;
	PFLT_GENERIC_WORKITEM WorkItem = NULL;
	BOOLEAN WasQueueEmpty = FALSE;

	InstCtx = CONTAINING_RECORD(DataQueue, NXRMFLT_INSTANCE_CONTEXT, CallbackDataQueue);


	WasQueueEmpty = IsListEmpty(&InstCtx->WorkItemQueue);

	//
	//  Insert the callback data entry into the queue.
	//

	InsertTailList(&InstCtx->WorkItemQueue, &Data->QueueLinks);

	//
	//  Queue a work item if no worker thread present.
	//

	if (WasQueueEmpty &&
		InterlockedIncrement(&InstCtx->WorkerThreadFlag) == 1)
	{

		WorkItem = FltAllocateGenericWorkItem();

		if (WorkItem)
		{

			status = FltQueueGenericWorkItem(WorkItem,
											 InstCtx->Instance,
											 nxrmfltCbdqWorkItemRoutine,
											 DelayedWorkQueue,
											 Context);
			if (!NT_SUCCESS(status))
			{
				FltFreeGenericWorkItem(WorkItem);
				WorkItem = NULL;
			}
		}
		else
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
		}

		if (!NT_SUCCESS(status))
		{
			RemoveTailList(&InstCtx->WorkItemQueue);
		}
	}

	return status;
}

VOID nxrmfltCbdqRemoveIo(_In_ PFLT_CALLBACK_DATA_QUEUE	DataQueue, _In_ PFLT_CALLBACK_DATA Data)
{
	RemoveEntryList(&Data->QueueLinks);
}

PFLT_CALLBACK_DATA	nxrmfltCbdqPeekNextIo(_In_ PFLT_CALLBACK_DATA_QUEUE DataQueue, _In_opt_ PFLT_CALLBACK_DATA Data, _In_opt_ PVOID PeekContext)
{
	NXRMFLT_INSTANCE_CONTEXT *InstCtx = NULL;
	PLIST_ENTRY NextEntry = NULL;
	PFLT_CALLBACK_DATA NextData = NULL;

	InstCtx = CONTAINING_RECORD(DataQueue, NXRMFLT_INSTANCE_CONTEXT, CallbackDataQueue);

	//
	//  If the supplied callback "Data" is NULL, the "NextIo" is the first entry
	//  in the queue; or it is the next list entry in the queue.
	//

	if (Data == NULL)
	{
		NextEntry = InstCtx->WorkItemQueue.Flink;
	}
	else
	{

		NextEntry = Data->QueueLinks.Flink;
	}

	//
	//  Return NULL if we hit the end of the queue or the queue is empty.
	//

	if (NextEntry == &InstCtx->WorkItemQueue)
	{
		return NULL;
	}

	NextData = CONTAINING_RECORD(NextEntry, FLT_CALLBACK_DATA, QueueLinks);

	return NextData;
}

VOID nxrmfltCbdqCompleteCanceledIo(_In_ PFLT_CALLBACK_DATA_QUEUE DataQueue, _Inout_ PFLT_CALLBACK_DATA	Data)
{
	PVOID QueueCtx = NULL;

	QueueCtx = Data->QueueContext[0];

	//
	//  Just complete the operation as canceled.
	//

	Data->IoStatus.Status = STATUS_CANCELLED;
	Data->IoStatus.Information = 0;

	FltCompletePendedPostOperation(Data);
}

VOID nxrmfltEmptyQueueAndComplete(
	_In_ 	NXRMFLT_INSTANCE_CONTEXT	*InstCtx
	)
{
	NTSTATUS Status	= STATUS_CANCELLED;
	FLT_PREOP_CALLBACK_STATUS callbackStatus = FLT_PREOP_COMPLETE;
	PFLT_CALLBACK_DATA Data = NULL;

	PVOID QueueCtx = NULL;

	FLT_FILE_NAME_INFORMATION *pNameInfo = NULL;
	NXRMFLT_DIRCTL_CONTEXT *DirCtlCtx = NULL;

	do
	{
		Data = FltCbdqRemoveNextIo(&InstCtx->CallbackDataQueue, NULL);

		if (Data)
		{
			QueueCtx = Data->QueueContext[0];

			//
			// just return STATUS_CANCELLED
			//
			Data->IoStatus.Status = Status;

			FltCompletePendedPreOperation(Data, callbackStatus, NULL);
		}

	} while (Data);

	return;
}

VOID
nxrmfltCbdqWorkItemRoutine(
__in PFLT_GENERIC_WORKITEM	FltWorkItem,
__in PVOID					FltObject,
__in_opt PVOID				Context
)
{
	NXRMFLT_INSTANCE_CONTEXT *InstCtx = NULL;
	PFLT_CALLBACK_DATA Data = NULL;
	PFLT_INSTANCE Instance = (PFLT_INSTANCE)FltObject;
	NTSTATUS Status = STATUS_SUCCESS;
	FLT_POSTOP_CALLBACK_STATUS	callbackStatus = FLT_POSTOP_FINISHED_PROCESSING;
	//
	//  Get a pointer to the instance context.
	//

	Status = FltGetInstanceContext(Instance, &InstCtx);

	if (!NT_SUCCESS(Status))
	{
		PT_DBG_PRINT(PTDBG_TRACE_CRITICAL,
					 ("nxrmflt!nxrmfltCbdqWorkItemRoutine: Instance Context is missing\n"));
		return;
	}

	//
	//  Process all the pended I/O in the cancel safe queue
	//

	for (;;)
	{

		//
		//  WorkerThreadFlag >= 1;
		//  Here we reduce it to 1.
		//
		InterlockedExchange(&InstCtx->WorkerThreadFlag, 1);

		//
		//  Remove an I/O from the cancel safe queue.
		//

		Data = FltCbdqRemoveNextIo(&InstCtx->CallbackDataQueue, NULL);

		if (Data)
		{
			if (!FlagOn(Data->Flags, FLTFL_CALLBACK_DATA_SYSTEM_BUFFER) &&
				(Data->Iopb->MajorFunction == IRP_MJ_READ ||
				Data->Iopb->MajorFunction == IRP_MJ_WRITE ||
				Data->Iopb->MajorFunction == IRP_MJ_DEVICE_CONTROL ||
				Data->Iopb->MajorFunction == IRP_MJ_DIRECTORY_CONTROL ||
				Data->Iopb->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL ||
				Data->Iopb->MajorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL ||
				Data->Iopb->MajorFunction == IRP_MJ_QUERY_EA ||
				Data->Iopb->MajorFunction == IRP_MJ_QUERY_QUOTA ||
				Data->Iopb->MajorFunction == IRP_MJ_QUERY_SECURITY ||
				Data->Iopb->MajorFunction == IRP_MJ_SET_EA ||
				Data->Iopb->MajorFunction == IRP_MJ_SET_QUOTA))
			{

				Status = FltLockUserBuffer(Data);

				if (!NT_SUCCESS(Status))
				{
					Data->IoStatus.Status = Status;
				}
			}

			switch (Data->Iopb->MajorFunction)
			{
			//case IRP_MJ_READ:

			//	break;
			//case IRP_MJ_WRITE:

			//	break;
			//case IRP_MJ_DIRECTORY_CONTROL:

			//	break;
			//case IRP_MJ_SET_INFORMATION:

			//	break;
			default:

				break;
			}

			//
			//  Complete the I/O
			//

			FltCompletePendedPostOperation(Data);

		}
		else
		{

			//
			//  At this moment it is possible that a new IO is being inserted
			//  into the queue in the CsqInsertIo routine. Now that the queue is
			//  empty, CsqInsertIo needs to make a decision on whether to create
			//  a new worker thread. The decision is based on the race between
			//  the InterlockedIncrement in CsqInsertIo and the
			//  InterlockedDecrement as below. There are two situations:
			//
			//  (1) If the decrement executes earlier before the increment,
			//      the flag will be decremented to 0 so this worker thread
			//      will return. Then CsqInsertIo will increment the flag
			//      from 0 to 1, and therefore create a new worker thread.
			//  (2) If the increment executes earlier before the decrement,
			//      the flag will be first incremented to 2 in CsqInsertIo
			//      so a new worker thread will not be satisfied. Then the
			//      decrement as below will lower the flag down to 1, and
			//      therefore continue this worker thread.
			//

			if (InterlockedDecrement(&InstCtx->WorkerThreadFlag) == 0)
			{
				break;
			}

		}
	}

	//
	//  Clean up
	//
	if (InstCtx)
		FltReleaseContext(InstCtx);

	FltFreeGenericWorkItem(FltWorkItem);

	return;
}
