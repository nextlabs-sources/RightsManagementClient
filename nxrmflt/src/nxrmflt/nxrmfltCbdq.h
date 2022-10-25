#pragma once

VOID
_IRQL_requires_max_(DISPATCH_LEVEL)
_IRQL_raises_(DISPATCH_LEVEL)
_Requires_lock_not_held_((CONTAINING_RECORD(DataQueue, NXRMFLT_INSTANCE_CONTEXT, CallbackDataQueue))->WorkItemQueueLock)
_Acquires_lock_((CONTAINING_RECORD(DataQueue, NXRMFLT_INSTANCE_CONTEXT, CallbackDataQueue))->WorkItemQueueLock)
nxrmfltCbdqAcquire(
_In_ PFLT_CALLBACK_DATA_QUEUE	DataQueue,
_Out_ PKIRQL					Irql
);

VOID
_IRQL_requires_max_(DISPATCH_LEVEL)
_IRQL_requires_min_(DISPATCH_LEVEL)
_IRQL_raises_(PASSIVE_LEVEL)
_Requires_lock_held_((CONTAINING_RECORD(DataQueue, NXRMFLT_INSTANCE_CONTEXT, CallbackDataQueue))->WorkItemQueueLock)
_Releases_lock_((CONTAINING_RECORD(DataQueue, NXRMFLT_INSTANCE_CONTEXT, CallbackDataQueue))->WorkItemQueueLock)
nxrmfltCbdqRelease(
_In_ PFLT_CALLBACK_DATA_QUEUE	DataQueue,
_In_ KIRQL						Irql
);

NTSTATUS
nxrmfltCbdqInsertIo(
_In_ PFLT_CALLBACK_DATA_QUEUE	DataQueue,
_In_ PFLT_CALLBACK_DATA			Data,
_In_opt_ PVOID					Context
);

VOID
nxrmfltCbdqRemoveIo(
_In_ PFLT_CALLBACK_DATA_QUEUE	DataQueue,
_In_ PFLT_CALLBACK_DATA			Data
);

PFLT_CALLBACK_DATA
nxrmfltCbdqPeekNextIo(
_In_ PFLT_CALLBACK_DATA_QUEUE	DataQueue,
_In_opt_ PFLT_CALLBACK_DATA		Data,
_In_opt_ PVOID					PeekContext
);

VOID
nxrmfltCbdqCompleteCanceledIo(
_In_ PFLT_CALLBACK_DATA_QUEUE	DataQueue,
_Inout_ PFLT_CALLBACK_DATA		Data
);

VOID
nxrmfltCbdqWorkItemRoutine(
__in PFLT_GENERIC_WORKITEM	FltWorkItem,
__in PVOID					FltObject,
__in_opt PVOID				Context
);

VOID 
nxrmfltEmptyQueueAndComplete(
_In_ 	NXRMFLT_INSTANCE_CONTEXT	*InstCtx
);
