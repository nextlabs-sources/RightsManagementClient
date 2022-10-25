

#include <ntifs.h>
#include <Ntstrsafe.h>


#include <nkdf/basic/defines.h>
#include <nkdf/basic/csq.h>




typedef struct _NKCSQ {

    PDEVICE_OBJECT   DeviceObject;
    IO_CSQ           IoCsq;
    PDRIVER_DISPATCH IrpDispatch;
    KSPIN_LOCK       QueueLock;
    LIST_ENTRY       PendingIrpQueue;
    KSEMAPHORE       IrpQueueSemaphore;
    PETHREAD         QueueThread;
    BOOLEAN          Stop;

} NKCSQ, *PNKCSQ;



KSTART_ROUTINE  CsqWorkerRoutine;
_IRQL_requires_same_
_Function_class_(KSTART_ROUTINE)
VOID
CsqWorkerRoutine(
                 _In_ PVOID Context
                 );

VOID
NkCsqInsertIrp(
               _In_ PIO_CSQ Csq,
               _In_ PIRP Irp
               );

VOID
NkCsqRemoveIrp(
               _In_ PIO_CSQ Csq,
               _In_ PIRP Irp
               );

PIRP
NkCsqPeekNextIrp(
                 _In_ PIO_CSQ Csq,
                 _In_ PIRP Irp,
                 _In_ PVOID PeekContext
                 );

_IRQL_raises_(DISPATCH_LEVEL)
_IRQL_requires_max_(DISPATCH_LEVEL)
VOID
NkCsqAcquireLock(
                 _In_ PIO_CSQ Csq,
                 _Out_ __drv_out_deref(_IRQL_saves_) PKIRQL Irql
                 );

_IRQL_requires_(DISPATCH_LEVEL)
VOID
NkCsqReleaseLock(
                 _In_ PIO_CSQ Csq,
                 _In_ __drv_in(__drv_restoresIRQL) KIRQL   Irql
                 );

VOID
NkCsqCompleteCanceledIrp(
                         _In_ PIO_CSQ Csq,
                         _In_ PIRP Irp
                         );




//  Assign text sections for each routine.
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NkCsqCreate)
#pragma alloc_text(PAGE, NkCsqDestroy)
#pragma alloc_text(PAGE, CsqWorkerRoutine)
#endif




_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkCsqCreate(
            _In_ PDEVICE_OBJECT DeviceObject,
            _In_ PDRIVER_DISPATCH IrpDispatch,
            _Out_ PNKCSQ* Csq
            )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    PNKCSQ      NewCsq = NULL;
    HANDLE      ThreadHandle = NULL;

    PAGED_CODE();


    try {

        if (NULL == IrpDispatch) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        NewCsq = ExAllocatePoolWithTag(NonPagedPool, sizeof(NKCSQ), TAG_CSQ);
        if (NULL == NewCsq) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory(NewCsq, sizeof(NKCSQ));
        NewCsq->DeviceObject = DeviceObject;
        NewCsq->IrpDispatch = IrpDispatch;
        KeInitializeSpinLock(&NewCsq->QueueLock);
        KeInitializeSemaphore(&NewCsq->IrpQueueSemaphore, 0, MAXLONG);
        InitializeListHead(&NewCsq->PendingIrpQueue);
        IoCsqInitialize(&NewCsq->IoCsq,
                        NkCsqInsertIrp,
                        NkCsqRemoveIrp,
                        NkCsqPeekNextIrp,
                        NkCsqAcquireLock,
                        NkCsqReleaseLock,
                        NkCsqCompleteCanceledIrp);

        NewCsq->Stop = FALSE;

        Status = PsCreateSystemThread(&ThreadHandle,
                                      (ACCESS_MASK)0,
                                      NULL,
                                      (HANDLE)0,
                                      NULL,
                                      CsqWorkerRoutine,
                                      NewCsq);

        if (!NT_SUCCESS(Status)) {
            NewCsq->Stop = TRUE;
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        //
        // Convert the Thread object handle into a pointer to the Thread object
        // itself. Then close the handle.
        //
        ObReferenceObjectByHandle(ThreadHandle,
                                  THREAD_ALL_ACCESS,
                                  NULL,
                                  KernelMode,
                                  &NewCsq->QueueThread,
                                  NULL);
        
try_exit: NOTHING;
    }
    finally {

        if (NULL != ThreadHandle) {
            ZwClose(ThreadHandle);
        }

        if (!NT_SUCCESS(Status)) {
            if (NULL != NewCsq) {
                ExFreePool(NewCsq);
                NewCsq = NULL;
            }
        }

    }

    *Csq = NewCsq;
    return Status;

}

_IRQL_requires_max_(APC_LEVEL)
VOID
NkCsqDestroy(
             _In_ PNKCSQ Csq
             )
{
    PAGED_CODE();

    // Stop thread
    Csq->Stop = TRUE;

    //
    // Make sure the thread wakes up
    //
#pragma prefast(suppress: 28160, "Ignore this warning because this is Windows API")
    KeReleaseSemaphore(&Csq->IrpQueueSemaphore,
                       0,       // No priority boost
                       1,       // Increment semaphore by 1
                       TRUE);   // WaitForXxx after this call

    //
    // Wait for the thread to terminate
    //
    KeWaitForSingleObject(Csq->QueueThread,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);
    ObDereferenceObject(Csq->QueueThread);
    Csq->QueueThread = NULL;

    // Destroy Queue
    ExFreePool(Csq);
    Csq = NULL;
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkCsqInsert(
             _In_ PNKCSQ Csq,
             _Inout_ PIRP Irp
             )
{
    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);
    KeEnterCriticalRegion();

    //
    // Queue the IRP and return STATUS_PENDING after signalling the
    // polling thread.
    // Note: IoCsqInsertIrp marks the IRP pending.
    //
    IoCsqInsertIrp(&Csq->IoCsq, Irp, NULL);

    //
    // Do not touch the IRP once it has been queued because another thread
    // could remove the IRP and complete it before this one gets to run.
    //

    //
    // A semaphore remains signaled as long as its count is greater than
    // zero, and non-signaled when the count is zero. Following function
    // increments the semaphore count by 1.
    //

    KeReleaseSemaphore(&Csq->IrpQueueSemaphore,
                       0,// No priority boost
                       1,// Increment semaphore by 1
                       FALSE);// No WaitForXxx after this call
    KeLeaveCriticalRegion();

    return STATUS_SUCCESS;
}



//
//  LOCAL ROUTINES
//
_IRQL_requires_same_
_Function_class_(KSTART_ROUTINE)
VOID
CsqWorkerRoutine(
                 _In_ PVOID Context
                 )
{
    PIRP        Irp = NULL;
    PNKCSQ      Csq = Context;

    KeSetPriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY);

    //
    // Now enter the main IRP-processing loop
    //
    for (;;)
    {
        //
        // Wait indefinitely for an IRP to appear in the work queue or for
        // the Unload routine to stop the thread. Every successful return
        // from the wait decrements the semaphore count by 1.
        //
        KeWaitForSingleObject(&Csq->IrpQueueSemaphore,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        //
        // See if thread was awakened because driver is unloading itself...
        //
        if (Csq->Stop) {
            Irp = IoCsqRemoveNextIrp(&Csq->IoCsq, NULL);
            while (Irp) {
                //
                // Cancel the IRP
                //
                Irp->IoStatus.Information = 0;
                Irp->IoStatus.Status = STATUS_CANCELLED;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);

                Irp = IoCsqRemoveNextIrp(&Csq->IoCsq, NULL);
            }
            PsTerminateSystemThread(STATUS_SUCCESS);
            return;
        }

        //
        // Remove a pending IRP from the queue.
        //
        Irp = IoCsqRemoveNextIrp(&Csq->IoCsq, NULL);
        if (!Irp) {
            continue; // go back to waiting
        }

        //
        // Perform I/O
        //
        (VOID)Csq->IrpDispatch(Csq->DeviceObject, Irp);

        //
        // Go back to the top of the loop to see if there's another request waiting.
        //

    } // end of while-loop
}


VOID
NkCsqInsertIrp(
               _In_ PIO_CSQ Csq,
               _In_ PIRP Irp
               )
{
    PNKCSQ   NkCsq = NULL;

    NkCsq = CONTAINING_RECORD(Csq, NKCSQ, IoCsq);
    InsertTailList(&NkCsq->PendingIrpQueue, &Irp->Tail.Overlay.ListEntry);
}

VOID
NkCsqRemoveIrp(
               _In_ PIO_CSQ Csq,
               _In_ PIRP Irp
               )
{
    UNREFERENCED_PARAMETER(Csq);
    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
}


PIRP
NkCsqPeekNextIrp(
                 _In_ PIO_CSQ Csq,
                 _In_ PIRP Irp,
                 _In_ PVOID PeekContext
                 )
{
    PNKCSQ              NkCsq = NULL;
    PIRP                NextIrp = NULL;
    PLIST_ENTRY         NextEntry;
    PLIST_ENTRY         ListHead;
    PIO_STACK_LOCATION  IrpStack;

    NkCsq = CONTAINING_RECORD(Csq, NKCSQ, IoCsq);
    ListHead = &NkCsq->PendingIrpQueue;

    //
    // If the IRP is NULL, we will start peeking from the listhead, else
    // we will start from that IRP onwards. This is done under the
    // assumption that new IRPs are always inserted at the tail.
    //
    if (Irp == NULL) {
        NextEntry = ListHead->Flink;
    }
    else {
        NextEntry = Irp->Tail.Overlay.ListEntry.Flink;
    }

    while (NextEntry != ListHead) {

        NextIrp = CONTAINING_RECORD(NextEntry, IRP, Tail.Overlay.ListEntry);
        IrpStack = IoGetCurrentIrpStackLocation(NextIrp);

        //
        // If context is present, continue until you find a matching one.
        // Else you break out as you got next one.
        //
        if (PeekContext) {
            if (IrpStack->FileObject == (PFILE_OBJECT)PeekContext) {
                break;
            }
        }
        else {
            break;
        }
        NextIrp = NULL;
        NextEntry = NextEntry->Flink;
    }

    return NextIrp;

}

//
// NkCsqAcquireLock modifies the execution level of the current processor.
// 
// KeAcquireSpinLock raises the execution level to Dispatch Level and stores
// the current execution level in the Irql parameter to be restored at a later
// time.  KeAcqurieSpinLock also requires us to be running at no higher than
// Dispatch level when it is called.
//
// The annotations reflect these changes and requirments.
//

_IRQL_raises_(DISPATCH_LEVEL)
_IRQL_requires_max_(DISPATCH_LEVEL)
VOID
NkCsqAcquireLock(
                 _In_ PIO_CSQ Csq,
                 _Out_ __drv_out_deref(__drv_savesIRQL) PKIRQL Irql
                 )
{
    PNKCSQ              NkCsq = NULL;

    NkCsq = CONTAINING_RECORD(Csq, NKCSQ, IoCsq);
    //
    // Suppressing because the address below csq is valid since it's
    // part of DEVICE_EXTENSION structure.
    //
#pragma prefast(suppress: __WARNING_BUFFER_UNDERFLOW, "Underflow using expression 'NkCsq->QueueLock'")
    KeAcquireSpinLock(&NkCsq->QueueLock, Irql);
}

//
// NkCsqReleaseLock modifies the execution level of the current processor.
// 
// KeReleaseSpinLock assumes we already hold the spin lock and are therefore
// running at Dispatch level.  It will use the Irql parameter saved in a
// previous call to KeAcquireSpinLock to return the thread back to it's original
// execution level.
//
// The annotations reflect these changes and requirments.
//

_IRQL_requires_(DISPATCH_LEVEL)
VOID
NkCsqReleaseLock(
                 _In_ PIO_CSQ Csq,
                 _In_ __drv_in(__drv_restoresIRQL) KIRQL   Irql
                 )
{
    PNKCSQ              NkCsq = NULL;

    NkCsq = CONTAINING_RECORD(Csq, NKCSQ, IoCsq);

    //
    // Suppressing because the address below csq is valid since it's
    // part of DEVICE_EXTENSION structure.
    //
#pragma prefast(suppress: __WARNING_BUFFER_UNDERFLOW, "Underflow using expression 'NkCsq->QueueLock'")
    KeReleaseSpinLock(&NkCsq->QueueLock, Irql);
}

VOID
NkCsqCompleteCanceledIrp(
                         _In_ PIO_CSQ Csq,
                         _In_ PIRP Irp
                         )
{

    UNREFERENCED_PARAMETER(Csq);
    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
}