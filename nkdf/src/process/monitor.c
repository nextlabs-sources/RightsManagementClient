
#include <ntifs.h>
#include <Ntstrsafe.h>


#include <nkdf/basic/defines.h>
#include <nkdf/basic/ntapi.h>
#include <nkdf/process/utility.h>
#include <nkdf/process/monitor.h>




typedef struct _CONTEXT_BLOCK {
    ERESOURCE         Lock;
    NKPROCESS_CONTEXT Context;
} CONTEXT_BLOCK, *PCONTEXT_BLOCK;

typedef struct _PROCESS_ITEM {
    LIST_ENTRY      Link;
    PROCESS_INFOW   Info;
    CONTEXT_BLOCK   Cb;
} PROCESS_ITEM, *PPROCESS_ITEM;


//
// Process Monitor Control Block
//
typedef struct _PROCESS_MONITOR_CTLBLOCK {
    BOOLEAN                     Active;
    ULONG                       Flags;
    ULONG                       ContextSize;
    ULONG                       ContextTag;
    PROCESS_CREATE_CALLBACK     CreateCallback;
    PROCESS_DESTROY_CALLBACK    DestroyCallback;
    ULONG                       ProcessCount;
    LIST_ENTRY                  ProcessList;
    ERESOURCE                   ProcessListLock;
} PROCESS_MONITOR_CTLBLOCK;

static PROCESS_MONITOR_CTLBLOCK pmcb = { FALSE, 0, 0, 0, NULL, NULL, 0 };



//
//  Local Routines
//

_IRQL_requires_(PASSIVE_LEVEL)
VOID
NkProcessMonitorCleanup(
                        );

_IRQL_requires_(PASSIVE_LEVEL)
VOID
NkProcessNotify(
                _Inout_ PEPROCESS Process,
                _In_ HANDLE ProcessId,
                _In_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo
                );

_IRQL_requires_(PASSIVE_LEVEL)
VOID
NkProcessOnCreate(
                  _In_ PEPROCESS Process,
                  _In_ HANDLE ProcessId,
                  _In_ PBOOLEAN  Deny
                  );

_IRQL_requires_(PASSIVE_LEVEL)
VOID
NkProcessOnDestroy(
                   _In_ PEPROCESS Process,
                   _In_ HANDLE ProcessId
                   );

_Check_return_
PPROCESS_ITEM
AllocProcessItem(
                 _In_ HANDLE ProcessId
                 );


VOID
FreeProcessItem(
                _In_ PPROCESS_ITEM Item
                );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkEnumProcess(
              );

//  Assign text sections for each routine.
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NkProcessMonitorStart)
#pragma alloc_text(PAGE, NkProcessMonitorStop)
#pragma alloc_text(PAGE, NkProcessMonitorCleanup)
#pragma alloc_text(PAGE, NkFindProcessInformation)
#pragma alloc_text(PAGE, NkGetProcessFlags)
#pragma alloc_text(PAGE, NkSetProcessFlags)
#pragma alloc_text(PAGE, NkRemoveProcessFlags)
#pragma alloc_text(PAGE, NkAcquireProcessContext)
#pragma alloc_text(PAGE, NkReleaseProcessContext)
#pragma alloc_text(PAGE, NkProcessNotify)
#pragma alloc_text(PAGE, NkProcessOnCreate)
#pragma alloc_text(PAGE, NkProcessOnDestroy)
#pragma alloc_text(PAGE, NkEnumProcess)
#endif





_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkProcessMonitorStart(
                      _In_ ULONG Flags,
                      _In_ ULONG ContextSize,
                      _In_ ULONG ContextTag,
                      _In_opt_ PROCESS_CREATE_CALLBACK CreateCallback,
                      _In_opt_ PROCESS_DESTROY_CALLBACK DestroyCallback
                      )
{
    NTSTATUS    Status = STATUS_SUCCESS;


    PAGED_CODE();


    if (pmcb.Active) {
        return STATUS_ALREADY_REGISTERED;
    }

    pmcb.Flags = (0 == Flags) ? PROCESS_MONITOR_USER_PROCESS : Flags;
    pmcb.ContextSize = ContextSize;
    pmcb.ContextTag = ContextTag;
    pmcb.CreateCallback = CreateCallback;
    pmcb.DestroyCallback = DestroyCallback;
    pmcb.ProcessCount = 0;
    InitializeListHead(&pmcb.ProcessList);
    Status = ExInitializeResourceLite(&pmcb.ProcessListLock);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    try {

        Status = NkEnumProcess();
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = PsSetCreateProcessNotifyRoutineEx(NkProcessNotify, FALSE);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        pmcb.Active = TRUE;

try_exit: NOTHING;
    }
    finally {
        if (!NT_SUCCESS(Status)) {
            NkProcessMonitorCleanup();
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkProcessMonitorStop(
                     )
{
    PAGED_CODE();

    if (!pmcb.Active) {
        return STATUS_SUCCESS;
    }

    pmcb.Active = FALSE;
    (VOID)PsSetCreateProcessNotifyRoutineEx(NkProcessNotify, TRUE);
    NkProcessMonitorCleanup();
    return STATUS_SUCCESS;
}

_IRQL_requires_(PASSIVE_LEVEL)
VOID
NkProcessMonitorCleanup(
                        )
{
    PAGED_CODE();

    ASSERT(!pmcb.Active);

    // Free all the items in list
    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(&pmcb.ProcessListLock, TRUE);
    while (!IsListEmpty(&pmcb.ProcessList)) {
        PLIST_ENTRY   Entry = NULL;
        PPROCESS_ITEM Item = NULL;
        Entry = RemoveHeadList(&pmcb.ProcessList);
        ASSERT(NULL != Entry);
        Item = CONTAINING_RECORD(Entry, PROCESS_ITEM, Link);
        ASSERT(NULL != Item);
        FreeProcessItem(Item);
        Item = NULL;
    }
    ExReleaseResourceLite(&pmcb.ProcessListLock);
    KeLeaveCriticalRegion();

    // Delete the lock
    (VOID)ExDeleteResourceLite(&pmcb.ProcessListLock);

    // Clean memory
    RtlZeroMemory(&pmcb, sizeof(pmcb));
    pmcb.Active = FALSE;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFindProcessInformation(
                         _In_ HANDLE ProcessId,
                         _Out_writes_bytes_opt_(sizeof(PROCESS_INFOW)) PPROCESS_INFOW Info
                         )
{
    NTSTATUS        Status = STATUS_NOT_FOUND;
    PLIST_ENTRY     Entry = NULL;

    PAGED_CODE();

    ASSERT(pmcb.Active);

    // Remove Item from list
    KeEnterCriticalRegion();
    (VOID)ExAcquireResourceSharedLite(&pmcb.ProcessListLock, TRUE);
    for (Entry = pmcb.ProcessList.Flink; Entry != &pmcb.ProcessList; Entry = Entry->Flink) {
        PPROCESS_ITEM   Item = NULL;
        Item = CONTAINING_RECORD(Entry, PROCESS_ITEM, Link);
        ASSERT(NULL != Item);
        if ((HANDLE)Item->Info.ProcessId == ProcessId) {
            if (NULL != Info) {
                RtlCopyMemory(Info, &Item->Info, sizeof(PROCESS_INFOW));
            }
            Status = STATUS_SUCCESS;
            break;
        }
    }
    ExReleaseResourceLite(&pmcb.ProcessListLock);
    KeLeaveCriticalRegion();

    return Status;
}


_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkGetProcessFlags(
                  _In_ HANDLE ProcessId,
                  _Out_ PULONG Flags
                  )
{
    NTSTATUS        Status = STATUS_NOT_FOUND;
    PLIST_ENTRY     Entry = NULL;

    PAGED_CODE();

    ASSERT(pmcb.Active);

    *Flags = 0;

    // Remove Item from list
    KeEnterCriticalRegion();
    (VOID)ExAcquireResourceSharedLite(&pmcb.ProcessListLock, TRUE);
    for (Entry = pmcb.ProcessList.Flink; Entry != &pmcb.ProcessList; Entry = Entry->Flink) {
        PPROCESS_ITEM   Item = NULL;
        Item = CONTAINING_RECORD(Entry, PROCESS_ITEM, Link);
        ASSERT(NULL != Item);
        if ((HANDLE)Item->Info.ProcessId == ProcessId) {
            // Set flags
            *Flags = Item->Info.Flags;
            Status = STATUS_SUCCESS;
            break;
        }
    }
    ExReleaseResourceLite(&pmcb.ProcessListLock);
    KeLeaveCriticalRegion();

    return Status;
}


_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkSetProcessFlags(
                  _In_ HANDLE ProcessId,
                  _In_ ULONG NewFlags
                  )
{
    NTSTATUS        Status = STATUS_NOT_FOUND;
    PLIST_ENTRY     Entry = NULL;

    PAGED_CODE();

    ASSERT(pmcb.Active);

    // Remove Item from list
    KeEnterCriticalRegion();
    (VOID)ExAcquireResourceExclusiveLite(&pmcb.ProcessListLock, TRUE);
    for (Entry = pmcb.ProcessList.Flink; Entry != &pmcb.ProcessList; Entry = Entry->Flink) {
        PPROCESS_ITEM   Item = NULL;
        Item = CONTAINING_RECORD(Entry, PROCESS_ITEM, Link);
        ASSERT(NULL != Item);
        if ((HANDLE)Item->Info.ProcessId == ProcessId) {
            // Set flags
            Item->Info.Flags = (0 == NewFlags) ? 0 : (Item->Info.Flags | NewFlags);
            Status = STATUS_SUCCESS;
            break;
        }
    }
    ExReleaseResourceLite(&pmcb.ProcessListLock);
    KeLeaveCriticalRegion();

    return Status;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkRemoveProcessFlags(
                     _In_ HANDLE ProcessId,
                     _In_ ULONG Flags
                     )
{
    NTSTATUS        Status = STATUS_NOT_FOUND;
    PLIST_ENTRY     Entry = NULL;

    PAGED_CODE();

    ASSERT(pmcb.Active);

    // Remove Item from list
    KeEnterCriticalRegion();
    (VOID)ExAcquireResourceExclusiveLite(&pmcb.ProcessListLock, TRUE);
    for (Entry = pmcb.ProcessList.Flink; Entry != &pmcb.ProcessList; Entry = Entry->Flink) {
        PPROCESS_ITEM   Item = NULL;
        Item = CONTAINING_RECORD(Entry, PROCESS_ITEM, Link);
        ASSERT(NULL != Item);
        if ((HANDLE)Item->Info.ProcessId == ProcessId) {
            // Set flags
            Item->Info.Flags &= ~(Item->Info.Flags);
            Status = STATUS_SUCCESS;
            break;
        }
    }
    ExReleaseResourceLite(&pmcb.ProcessListLock);
    KeLeaveCriticalRegion();

    return Status;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkAcquireProcessContext(
                        _In_ HANDLE ProcessId,
                        _In_ BOOLEAN Exclusive,
                        _Out_ PNKPROCESS_CONTEXT* PtrContext
                        )
{
    NTSTATUS            Status = STATUS_NOT_FOUND;
    PLIST_ENTRY         Entry = NULL;

    PAGED_CODE();

    ASSERT(pmcb.Active);

    *PtrContext = NULL;

    // Remove Item from list
    KeEnterCriticalRegion();
    ExAcquireResourceSharedLite(&pmcb.ProcessListLock, TRUE);
    for (Entry = pmcb.ProcessList.Flink; Entry != &pmcb.ProcessList; Entry = Entry->Flink) {
        PPROCESS_ITEM   Item = NULL;
        Item = CONTAINING_RECORD(Entry, PROCESS_ITEM, Link);
        ASSERT(NULL != Item);
        if ((HANDLE)Item->Info.ProcessId == ProcessId) {
            if (Exclusive) {
                (VOID)ExAcquireResourceExclusiveLite(&Item->Cb.Lock, TRUE);
            }
            else {
                (VOID)ExAcquireResourceSharedLite(&Item->Cb.Lock, TRUE);
            }
            *PtrContext = &(Item->Cb.Context);
            Status = STATUS_SUCCESS;
            break;
        }
    }
    ExReleaseResourceLite(&pmcb.ProcessListLock);
    KeLeaveCriticalRegion();

    return Status;
}

_IRQL_requires_max_(DISPATCH_LEVEL)
VOID
NkReleaseProcessContext(
                        _In_ PNKPROCESS_CONTEXT ProcContext
                        )
{
    PCONTEXT_BLOCK Pcb = NULL;

    PAGED_CODE();

    Pcb = CONTAINING_RECORD(ProcContext, CONTEXT_BLOCK, Context);
    ASSERT(NULL != Pcb);
    KeEnterCriticalRegion();
    ExReleaseResourceLite(&Pcb->Lock);
    KeLeaveCriticalRegion();
}




//
//  Local Routines
//

_IRQL_requires_(PASSIVE_LEVEL)
VOID
NkProcessNotify(
                _Inout_ PEPROCESS Process,
                _In_ HANDLE ProcessId,
                _In_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo
                )
{
    PAGED_CODE();

    if (NULL != CreateInfo) {

        BOOLEAN Deny = FALSE;

        NkProcessOnCreate(Process, ProcessId, &Deny);
        if (Deny) {
            CreateInfo->CreationStatus = STATUS_ACCESS_DENIED;
        }
    }
    else {
        NkProcessOnDestroy(Process, ProcessId);
    }
}

_IRQL_requires_(PASSIVE_LEVEL)
VOID
NkProcessOnCreate(
                  _In_ PEPROCESS Process,
                  _In_ HANDLE ProcessId,
                  _In_ PBOOLEAN Deny
                  )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    PPROCESS_ITEM   Item = NULL;

    PAGED_CODE();


    ASSERT(pmcb.Active);

    try {

        Item = AllocProcessItem(ProcessId);
        if (NULL == Item) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }

        Status = NkGetProcessInfo(ProcessId, &Item->Info.SessionId, Item->Info.UserName, Item->Info.UserSid, Item->Info.ImageFileName);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        if (!BooleanFlagOn(pmcb.Flags, PROCESS_MONITOR_SYSTEM_PROCESS)) {
            if (28 == RtlCompareMemory(Item->Info.UserName, L"LOCAL SERVICE", 28) ||
                32 == RtlCompareMemory(Item->Info.UserName, L"NETWORK SERVICE", 32) ||
                14 == RtlCompareMemory(Item->Info.UserName, L"SYSTEM", 14)) {

                try_return(Status = STATUS_NOT_SUPPORTED);  // Don't need to monitor system process
            }
        }

        if (NULL != pmcb.CreateCallback) {
            pmcb.CreateCallback(ProcessId, &Item->Info, Item->Cb.Context.Data, Item->Cb.Context.Size, Deny);
            if (*Deny) {
                try_return(Status = STATUS_ACCESS_DENIED);
            }
        }

        KeEnterCriticalRegion();
        ExAcquireResourceExclusiveLite(&pmcb.ProcessListLock, TRUE);
        InsertTailList(&pmcb.ProcessList, &Item->Link);
        ExReleaseResourceLite(&pmcb.ProcessListLock);
        KeLeaveCriticalRegion();


try_exit: NOTHING;
    }
    finally {
        if (!NT_SUCCESS(Status)) {
            if (NULL != Item) {
                FreeProcessItem(Item);
                Item = NULL;
            }
        }
    }
}

_IRQL_requires_(PASSIVE_LEVEL)
VOID
NkProcessOnDestroy(
                   _In_ PEPROCESS Process,
                   _In_ HANDLE ProcessId
                   )
{
    PPROCESS_ITEM   Item = NULL;
    PLIST_ENTRY     Entry = NULL;

    PAGED_CODE();

    ASSERT(pmcb.Active);

    // Remove Item from list
    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(&pmcb.ProcessListLock, TRUE);
    for (Entry = pmcb.ProcessList.Flink; Entry != &pmcb.ProcessList; Entry = Entry->Flink) {
        PPROCESS_ITEM   CurItem = NULL;
        CurItem = CONTAINING_RECORD(Entry, PROCESS_ITEM, Link);
        ASSERT(NULL != CurItem);
        if ((HANDLE)CurItem->Info.ProcessId == ProcessId) {
            RemoveEntryList(&CurItem->Link);
            Item = CurItem;
            break;
        }
    }
    ExReleaseResourceLite(&pmcb.ProcessListLock);
    KeLeaveCriticalRegion();

    // Free Item
    if (NULL != Item) {

        if (NULL != pmcb.DestroyCallback) {
            ASSERT(pmcb.ContextSize == Item->Cb.Context.Size);
            pmcb.DestroyCallback(ProcessId, &Item->Info, Item->Cb.Context.Data, Item->Cb.Context.Size);
        }

        FreeProcessItem(Item);
    }
}

_Check_return_
PPROCESS_ITEM
AllocProcessItem(
                 _In_ HANDLE ProcessId
                 )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    PPROCESS_ITEM   Item = NULL;

    Item = ExAllocatePoolWithTag(NonPagedPool, sizeof(PROCESS_ITEM), pmcb.ContextTag);
    if (NULL == Item) {
        return NULL;
    }

    RtlZeroMemory(Item, sizeof(PROCESS_ITEM));
    InitializeListHead(&Item->Link);
    Item->Info.ProcessId = (ULONG)(ULONG_PTR)ProcessId;
    Status = ExInitializeResourceLite(&Item->Cb.Lock);
    if (!NT_SUCCESS(Status)) {
        ExFreePool(Item);
        return NULL;
    }

    Item->Cb.Context.Size = pmcb.ContextSize;
    if (0 != pmcb.ContextSize) {
        Item->Cb.Context.Data = ExAllocatePoolWithTag(PagedPool, pmcb.ContextSize, pmcb.ContextTag);
        if (NULL == Item->Cb.Context.Data) {
            (VOID)ExDeleteResourceLite(&Item->Cb.Lock);
            ExFreePool(Item);
            return NULL;
        }
        RtlZeroMemory(Item->Cb.Context.Data, Item->Cb.Context.Size);
    }

    return Item;
}

VOID
FreeProcessItem(
                _In_ PPROCESS_ITEM Item
                )
{
    if (NULL != Item) {
        if (0 != pmcb.ContextSize) {
            KeEnterCriticalRegion();
            ExAcquireResourceExclusiveLite(&Item->Cb.Lock, TRUE);
            if (NULL != Item->Cb.Context.Data) {
                ExFreePool(Item->Cb.Context.Data);
                Item->Cb.Context.Data = NULL;
                Item->Cb.Context.Size = 0;
            }
            ExReleaseResourceLite(&Item->Cb.Lock);
            KeLeaveCriticalRegion();
        }
        (VOID)ExDeleteResourceLite(&Item->Cb.Lock);
        ExFreePool(Item);
    }
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkEnumProcess(
              )
{
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    PUCHAR      ProcInfo = NULL;
    ULONG       ProcInfoSize;
    ULONG       InfoOffset = 0;


    PAGED_CODE();



    try {

        Status = NkZwQuerySystemInformation(SystemProcessInformation,
                                            NULL,
                                            0,
                                            &ProcInfoSize);
        if (Status != STATUS_INFO_LENGTH_MISMATCH) {
            try_return(Status);
        }

        ASSERT(0 != ProcInfoSize);

        // Allocate more bytes to ensure enough buffer
        ProcInfoSize += 128;
        ProcInfo = ExAllocatePoolWithTag(NonPagedPool, ProcInfoSize, TAG_TEMP);
        if (NULL == ProcInfo) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory(ProcInfo, ProcInfoSize);
        Status = NkZwQuerySystemInformation(SystemProcessInformation,
                                            ProcInfo,
                                            ProcInfoSize,
                                            &ProcInfoSize);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        //
        //  Good, we got all existing processes
        //  We need to handle them one by one
        //
        do {

            NTSTATUS                    InfoStatus = STATUS_SUCCESS;
            SYSTEM_PROCESS_INFORMATION* Info = NULL;
            PPROCESS_ITEM               Item = NULL;


            // Get current Info
            Info = (SYSTEM_PROCESS_INFORMATION*)(ProcInfo + InfoOffset);

            // Get next Info's offset
            InfoOffset = (0 == Info->NextEntryOffset) ? 0 : (InfoOffset + Info->NextEntryOffset);

            //
            // Handle current info
            //

            // Ignore Idle process
            if (0 == Info->UniqueProcessId) {
                continue;
            }

            try {

                BOOLEAN Deny = FALSE;

                Item = AllocProcessItem(Info->UniqueProcessId);
                if (NULL == Item) {
                    try_return2(InfoStatus = STATUS_INSUFFICIENT_RESOURCES);
                }

                InfoStatus = NkGetProcessInfo(Info->UniqueProcessId, &Item->Info.SessionId, Item->Info.UserName, Item->Info.UserSid, Item->Info.ImageFileName);
                if (!NT_SUCCESS(InfoStatus)) {
                    try_return2(InfoStatus);
                }

                if (!BooleanFlagOn(pmcb.Flags, PROCESS_MONITOR_SYSTEM_PROCESS)) {
                    if (28 == RtlCompareMemory(Item->Info.UserName, L"LOCAL SERVICE", 28) ||
                        32 == RtlCompareMemory(Item->Info.UserName, L"NETWORK SERVICE", 32) ||
                        14 == RtlCompareMemory(Item->Info.UserName, L"SYSTEM", 14)) {

                        try_return2(InfoStatus = STATUS_NOT_SUPPORTED);  // Don't need to monitor system process
                    }
                }

                if (NULL != pmcb.CreateCallback) {
                    pmcb.CreateCallback(Info->UniqueProcessId, &Item->Info, Item->Cb.Context.Data, Item->Cb.Context.Size, &Deny);
                    if (Deny) {
                        (VOID)NkTerminateProcess(Info->UniqueProcessId, 0);
                        try_return2(InfoStatus = STATUS_ACCESS_DENIED);
                    }
                }

                KeEnterCriticalRegion();
                ExAcquireResourceExclusiveLite(&pmcb.ProcessListLock, TRUE);
                InsertTailList(&pmcb.ProcessList, &Item->Link);
                Item = NULL; // transfer the ownership
                ExReleaseResourceLite(&pmcb.ProcessListLock);
                KeLeaveCriticalRegion();

try_exit2: NOTHING;
            }
            finally {

                if (!NT_SUCCESS(InfoStatus)) {
                    if (NULL != Item) {
                        FreeProcessItem(Item);
                        Item = NULL;
                    }
                }
            }

        } while (0 != InfoOffset);


try_exit: NOTHING;
    }
    finally {

        if (NULL != ProcInfo) {

            ExFreePool(ProcInfo);
            ProcInfo = NULL;
        }
    }

    return Status;
}