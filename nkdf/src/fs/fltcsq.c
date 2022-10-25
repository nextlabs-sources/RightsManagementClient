

#include <ntifs.h>

#include <nkdf/basic.h>
#include <nkdf/fs/fltcsq.h>




typedef struct _NKFLTCSQ {
    // Flt Filter
    PFLT_FILTER             Filter;
    // Flt Insatcne
    PFLT_INSTANCE           Instance;
    // Csq Callback Routine
    PNKFLTCSQ_PRE_CALLBACK  CsqPreCallback;
    PNKFLTCSQ_POST_CALLBACK CsqPostCallback;
    // FltCallbackDataQueue
    FLT_CALLBACK_DATA_QUEUE Cbdq;
    // Queue to hold work item
    LIST_ENTRY              CbdqHead;
    FAST_MUTEX              CbdqLock;
    // Thread indicator
    LONG volatile           CbdqThread;
} NKFLTCSQ, *PNKFLTCSQ;



VOID
_IRQL_requires_max_(APC_LEVEL)
_IRQL_raises_(APC_LEVEL)
CsqAcquire(
           _In_ PFLT_CALLBACK_DATA_QUEUE DataQueue,
           _Out_ PKIRQL Irql
           );

VOID
_IRQL_requires_max_(APC_LEVEL)
_IRQL_requires_min_(APC_LEVEL)
_IRQL_raises_(PASSIVE_LEVEL)
CsqRelease(
           _In_ PFLT_CALLBACK_DATA_QUEUE DataQueue,
           _In_ KIRQL Irql
           );

NTSTATUS
CsqInsertIo(
            _In_ PFLT_CALLBACK_DATA_QUEUE DataQueue,
            _In_ PFLT_CALLBACK_DATA Data,
            _In_opt_ PVOID Context
            );

VOID
CsqRemoveIo(
            _In_ PFLT_CALLBACK_DATA_QUEUE DataQueue,
            _In_ PFLT_CALLBACK_DATA Data
            );

PFLT_CALLBACK_DATA
CsqPeekNextIo(
              _In_ PFLT_CALLBACK_DATA_QUEUE DataQueue,
              _In_opt_ PFLT_CALLBACK_DATA Data,
              _In_opt_ PVOID PeekContext
              );

VOID
CsqCompleteCanceledIo(
                      _In_ PFLT_CALLBACK_DATA_QUEUE DataQueue,
                      _Inout_ PFLT_CALLBACK_DATA Data
                      );

_IRQL_raises_(PASSIVE_LEVEL)
VOID
CsqFltWorkerRoutine(
                 _In_ PFLT_GENERIC_WORKITEM WorkItem,
                 _In_ PFLT_FILTER Filter,
                 _In_ PVOID Context
                 );



//  Assign text sections for each routine.
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NkFltCsqCreate)
#pragma alloc_text(PAGE, NkFltCsqDestroy)
#pragma alloc_text(PAGE, CsqFltWorkerRoutine)
#endif




_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltCsqCreate(
                _In_ PFLT_FILTER Filter,
                _In_ PFLT_INSTANCE Instance,
                _In_ PNKFLTCSQ_PRE_CALLBACK CsqPreCallback,
                _In_ PNKFLTCSQ_POST_CALLBACK CsqPostCallback,
                _Out_ PNKFLTCSQ* Csq
                )
{
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    PNKFLTCSQ   NkFltCsq = NULL;

    PAGED_CODE();

    try {

        NkFltCsq = ExAllocatePoolWithTag(NonPagedPool, sizeof(NKFLTCSQ), TAG_CSQ);
        if (NULL == NkFltCsq) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory(NkFltCsq, sizeof(NKFLTCSQ));
        NkFltCsq->Filter = Filter;
        NkFltCsq->Instance = Instance;
        NkFltCsq->CsqPreCallback = CsqPreCallback;
        NkFltCsq->CsqPostCallback = CsqPostCallback;
        InitializeListHead(&NkFltCsq->CbdqHead);
        ExInitializeFastMutex(&NkFltCsq->CbdqLock);
        Status = FltCbdqInitialize(Instance,
                                   &NkFltCsq->Cbdq,
                                   CsqInsertIo,
                                   CsqRemoveIo,
                                   CsqPeekNextIo,
                                   CsqAcquire,
                                   CsqRelease,
                                   CsqCompleteCanceledIo);


try_exit: NOTHING;
    }
    finally {

        if (NT_SUCCESS(Status)) {
            *Csq = NkFltCsq;
            NkFltCsq = NULL;
        }
        else {

            if (NULL != NkFltCsq) {
                ExFreePool(NkFltCsq);
            }
        }
    }

    return Status;
}

_IRQL_requires_max_(APC_LEVEL)
VOID
NkFltCsqDestroy(
                _In_ PNKFLTCSQ Csq
                )
{
    PFLT_CALLBACK_DATA  Data = NULL;

    PAGED_CODE();


    //
    // Disable the queue
    //
    FltCbdqDisable(&Csq->Cbdq);

    //
    // Cancel all requests in the queue
    //    
    do {

        PNKFLTCSQ_CONTEXT   CsqCtx = NULL;

        Data = FltCbdqRemoveNextIo(&Csq->Cbdq, NULL);
        if (NULL == Data) {
            continue;
        }

        CsqCtx = Data->QueueContext[0];

        // 
        //  Just complete the operation as canceled. 
        //
        if (CsqCtx->IsPreCb) {

            // Performed at pre operation
            Data->IoStatus.Status = STATUS_CANCELLED;
            Data->IoStatus.Information = 0;
            FltCompletePendedPreOperation(Data, FLT_PREOP_COMPLETE, NULL);
        }
        else {

            // Performed at post operation
            FltCompletePendedPostOperation(Data);
        }

        // 
        //  Free the extra storage that was allocated for this canceled I/O. 
        //
        if (NULL != CsqCtx->UserContext) {
            ASSERT(0 != CsqCtx->UserContextSize);
            ExFreePool(CsqCtx->UserContext);
            CsqCtx->UserContext = NULL;
            CsqCtx->UserContextSize = 0;
        }
        ExFreePool(CsqCtx);
        CsqCtx = NULL;

    } while (Data);
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkFltCsqInsert(
               _Inout_ PFLT_CALLBACK_DATA Data,
               _In_ PNKFLTCSQ Csq,
               _In_ BOOLEAN IsPreCb,
               _In_opt_ PVOID UserContext,
               _In_ ULONG UserContextSize
               )
{
    NTSTATUS        Status = STATUS_UNSUCCESSFUL;
    PNKFLTCSQ_CONTEXT Context = NULL;


    try {

        Context = ExAllocatePoolWithTag(NonPagedPool, sizeof(NKFLTCSQ_CONTEXT), TAG_CSQ);
        if (NULL == Context) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory(Context, sizeof(NKFLTCSQ_CONTEXT));
        Context->IsPreCb = IsPreCb;
        Context->UserContext = UserContext;
        Context->UserContextSize = UserContextSize;

        Data->QueueContext[0] = (PVOID)Context;
        Data->QueueContext[1] = NULL;

        Status = FltCbdqInsertIo(&Csq->Cbdq,
                                 Data,
                                 &Context->CbdqIoContext,
                                 0);

try_exit: NOTHING;
    }
    finally {

        if (!NT_SUCCESS(Status)) {

            if (NULL != Context) {
                ExFreePool(Context);
                Context = NULL;
            }
        }
    }

    return Status;
}




//*********************************************************************************
//
//      DEFINE LOCAL ROUTINES
//
//*********************************************************************************

//
//  Safe Cancel Queue
//
VOID
_IRQL_requires_max_(APC_LEVEL)
_IRQL_raises_(APC_LEVEL)
CsqAcquire(
           _In_ PFLT_CALLBACK_DATA_QUEUE DataQueue,
           _Out_ PKIRQL Irql
           )
{
    PNKFLTCSQ  Csq;


    UNREFERENCED_PARAMETER(Irql);

    // 
    //  Get a pointer to the instance context. 
    // 
    Csq = CONTAINING_RECORD(DataQueue, NKFLTCSQ, Cbdq);

    if (Irql)
    {

        RtlZeroMemory(Irql, sizeof(KIRQL));

    }
    // 
    //  Acquire the lock. 
    //
    ExAcquireFastMutex(&Csq->CbdqLock);
}

VOID
_IRQL_requires_max_(APC_LEVEL)
_IRQL_requires_min_(APC_LEVEL)
_IRQL_raises_(PASSIVE_LEVEL)
CsqRelease(
           _In_ PFLT_CALLBACK_DATA_QUEUE DataQueue,
           _In_ KIRQL Irql
           )
{
    PNKFLTCSQ  Csq;


    UNREFERENCED_PARAMETER(Irql);

    // 
    //  Get a pointer to the instance context. 
    //
    Csq = CONTAINING_RECORD(DataQueue, NKFLTCSQ, Cbdq);

    // 
    //  Release the lock. 
    //
    ExReleaseFastMutex(&Csq->CbdqLock);
}

NTSTATUS
CsqInsertIo(
            _In_ PFLT_CALLBACK_DATA_QUEUE DataQueue,
            _In_ PFLT_CALLBACK_DATA Data,
            _In_opt_ PVOID Context
            )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    PNKFLTCSQ   Csq;
    BOOLEAN     WasQueueEmpty;


    UNREFERENCED_PARAMETER(Context);


    // 
    //  Get a pointer to the instance context. 
    //
    Csq = CONTAINING_RECORD(DataQueue, NKFLTCSQ, Cbdq);

    // 
    //  Save the queue state before inserting to it. 
    //
    WasQueueEmpty = IsListEmpty(&Csq->CbdqHead);

    // 
    //  Insert the callback data entry into the queue. 
    //
    InsertTailList(&Csq->CbdqHead, &Data->QueueLinks);

    // 
    //  Queue a work item if no worker thread present. 
    //
    if (WasQueueEmpty && InterlockedIncrement(&Csq->CbdqThread) == 1) {

        PFLT_GENERIC_WORKITEM   WorkItem;

        WorkItem = FltAllocateGenericWorkItem();
        if (WorkItem) {

            Status = FltQueueGenericWorkItem(WorkItem,
                                             Csq->Filter,
                                             CsqFltWorkerRoutine,
                                             CriticalWorkQueue,
                                             Csq);
            if (!NT_SUCCESS(Status)) {
                FltFreeGenericWorkItem(WorkItem);
            }
        }
        else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

        if (!NT_SUCCESS(Status)) {
            //  Remove the callback data that was inserted into the queue. 
            RemoveTailList(&Csq->CbdqHead);
        }
    }


    return Status;
}


VOID
CsqRemoveIo(
            _In_ PFLT_CALLBACK_DATA_QUEUE DataQueue,
            _In_ PFLT_CALLBACK_DATA Data
            )
{
    UNREFERENCED_PARAMETER(DataQueue);

    // 
    //  Remove the callback data entry from the queue. 
    //
    RemoveEntryList(&Data->QueueLinks);
}


PFLT_CALLBACK_DATA
CsqPeekNextIo(
              _In_ PFLT_CALLBACK_DATA_QUEUE DataQueue,
              _In_opt_ PFLT_CALLBACK_DATA Data,
              _In_opt_ PVOID PeekContext
              )
{
    PNKFLTCSQ           Csq;
    PLIST_ENTRY         NextEntry;
    PFLT_CALLBACK_DATA  NextData;


    UNREFERENCED_PARAMETER(PeekContext);


    // 
    //  Get a pointer to the Csq
    //
    Csq = CONTAINING_RECORD(DataQueue, NKFLTCSQ, Cbdq);

    // 
    //  If the supplied callback "Data" is NULL, the "NextIo" is the first entry 
    //  in the queue; or it is the next list entry in the queue. 
    //
    if (Data == NULL) {

        NextEntry = Csq->CbdqHead.Flink;

    }
    else {

        NextEntry = Data->QueueLinks.Flink;
    }

    // 
    //  Return NULL if we hit the end of the queue or the queue is empty. 
    //
    if (NextEntry == &Csq->CbdqHead) {

        return NULL;
    }

    NextData = CONTAINING_RECORD(NextEntry, FLT_CALLBACK_DATA, QueueLinks);
    return NextData;
}


VOID
CsqCompleteCanceledIo(
_In_ PFLT_CALLBACK_DATA_QUEUE DataQueue,
_Inout_ PFLT_CALLBACK_DATA Data
)
{
    PNKFLTCSQ       Csq;
    PNKFLTCSQ_CONTEXT CsqCtx = NULL;



    // 
    //  Get a pointer to the Csq
    //
    Csq = CONTAINING_RECORD(DataQueue, NKFLTCSQ, Cbdq);

    CsqCtx = (PNKFLTCSQ_CONTEXT)Data->QueueContext[0];
    Data->QueueContext[0] = NULL;

    ASSERT(NULL != CsqCtx);


    // 
    //  Just complete the operation as canceled. 
    //
    if (CsqCtx->IsPreCb) {

        // Performed at pre operation
        Data->IoStatus.Status = STATUS_CANCELLED;
        Data->IoStatus.Information = 0;
        FltCompletePendedPreOperation(Data, FLT_PREOP_COMPLETE, NULL);
    }
    else {

        // Performed at post operation
        FltCompletePendedPostOperation(Data);
    }

    // 
    //  Free the extra storage that was allocated for this canceled I/O. 
    //
    if (NULL != CsqCtx->UserContext) {
        ASSERT(0 != CsqCtx->UserContextSize);
        ExFreePool(CsqCtx->UserContext);
        CsqCtx->UserContext = NULL;
        CsqCtx->UserContextSize = 0;
    }
    ExFreePool(CsqCtx);
    CsqCtx = NULL;
}

_IRQL_raises_(PASSIVE_LEVEL)
VOID
CsqFltWorkerRoutine(
                 _In_ PFLT_GENERIC_WORKITEM WorkItem,
                 _In_ PFLT_FILTER Filter,
                 _In_ PVOID Context
                 )
{
    PNKFLTCSQ   Csq;


    PAGED_CODE();

    UNREFERENCED_PARAMETER(Filter);

    Csq = (PNKFLTCSQ)Context;
    ASSERT(NULL != Csq);

    for (;;) {

        PFLT_CALLBACK_DATA  Data = NULL;
        PNKFLTCSQ_CONTEXT   CsqCtx = NULL;

        //  Remove an I/O from the cancel safe queue.
        Data = FltCbdqRemoveNextIo(&Csq->Cbdq, NULL);
        if (NULL == Data) {

            if (InterlockedDecrement(&Csq->CbdqThread) == 0) {
                break;
            }

            continue;
        }

        CsqCtx = Data->QueueContext[0];
        ASSERT(NULL != CsqCtx);
        Data->QueueContext[0] = NULL;

        try {

            if (CsqCtx == NULL) {
                try_return(NOTHING);
            }

            if (CsqCtx->IsPreCb) {

                FLT_PREOP_CALLBACK_STATUS CbStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
                PVOID CbContext = NULL;

                CbStatus = Csq->CsqPreCallback(Data, Csq, CsqCtx->UserContext, CsqCtx->UserContextSize, &CbContext);

                ASSERT(FLT_PREOP_PENDING != CbStatus);
                if (FLT_PREOP_SUCCESS_WITH_CALLBACK != CbStatus) { ASSERT(NULL == CbContext); }

                FltCompletePendedPreOperation(Data, CbStatus, CbContext);
            }
            else {

                Csq->CsqPostCallback(Data, Csq, CsqCtx->UserContext, CsqCtx->UserContextSize);
                FltCompletePendedPostOperation(Data);
            }

try_exit: NOTHING;
        }
        finally {

            // Free CsqCtx
            if (NULL != CsqCtx) {

                if (NULL != CsqCtx->UserContext) {
                    ASSERT(0 != CsqCtx->UserContextSize);
                    ExFreePool(CsqCtx->UserContext);
                    CsqCtx->UserContext = NULL;
                    CsqCtx->UserContextSize = 0;
                }
                ExFreePool(CsqCtx);
                CsqCtx = NULL;
            }
        }
    }


    // Free Work Item
    FltFreeGenericWorkItem(WorkItem);
}