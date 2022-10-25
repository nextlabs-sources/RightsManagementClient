

#include <ntddk.h>

#include <nkdf/basic/list.h>
#include <nkdf/basic/defines.h>



typedef struct _NKLIST_HEAD {
    LIST_ENTRY  ListHead;
    ERESOURCE   Lock;
    NKLIST_COMPARE_ROUTINE Compare;
    NKLIST_FREE_ROUTINE Free;
} NKLIST_HEAD, *PNKLIST_HEAD;


_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkCreateList(
             _In_ NKLIST_COMPARE_ROUTINE CompareRoutine,
             _In_ NKLIST_FREE_ROUTINE FreeRoutine,
             _Out_ PHANDLE ListHandle
             )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    PNKLIST_HEAD NkList = NULL;

    *ListHandle = NULL;

    NkList = ExAllocatePoolWithTag(NonPagedPool, sizeof(NKLIST_HEAD), TAG_LIST);
    if (NULL == NkList) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(NkList, sizeof(NKLIST_HEAD));
    InitializeListHead(&NkList->ListHead);
    NkList->Compare = CompareRoutine;
    NkList->Free = FreeRoutine;
    Status = ExInitializeResourceLite(&NkList->Lock);
    if (!NT_SUCCESS(Status)) {
        ExFreePool(NkList);
        NkList = NULL;
        return Status;
    }

    *ListHandle = (HANDLE)NkList;
    return STATUS_SUCCESS;
}

_IRQL_requires_max_(APC_LEVEL)
VOID
NkDestoryList(
              _In_ HANDLE ListHandle
              )
{
    KeEnterCriticalRegion();
    (VOID)ExAcquireResourceExclusiveLite(&(((PNKLIST_HEAD)ListHandle)->Lock), TRUE);
    while (!IsListEmpty(&(((PNKLIST_HEAD)ListHandle)->ListHead))) {
        PLIST_ENTRY Entry = RemoveHeadList(&(((PNKLIST_HEAD)ListHandle)->ListHead));
        ASSERT(NULL != Entry);
        ((PNKLIST_HEAD)ListHandle)->Free(Entry);
    }
    ExReleaseResourceLite(&(((PNKLIST_HEAD)ListHandle)->Lock));
    KeLeaveCriticalRegion();
    ExDeleteResourceLite(&(((PNKLIST_HEAD)ListHandle)->Lock));
    ExFreePool(ListHandle);
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
BOOLEAN
NkIsListEmpty(
              _In_ HANDLE ListHandle
              )
{
    BOOLEAN IsEmpty = TRUE;

    KeEnterCriticalRegion();
    (VOID)ExAcquireResourceSharedLite(&(((PNKLIST_HEAD)ListHandle)->Lock), TRUE);
    IsEmpty = IsListEmpty(&(((PNKLIST_HEAD)ListHandle)->ListHead));
    ExReleaseResourceLite(&(((PNKLIST_HEAD)ListHandle)->Lock));
    KeLeaveCriticalRegion();

    return IsEmpty;
}

_IRQL_requires_max_(APC_LEVEL)
VOID
NkClearList(
            _In_ HANDLE ListHandle
            )
{
    KeEnterCriticalRegion();
    (VOID)ExAcquireResourceExclusiveLite(&(((PNKLIST_HEAD)ListHandle)->Lock), TRUE);
    while (!IsListEmpty(&(((PNKLIST_HEAD)ListHandle)->ListHead))) {
        PLIST_ENTRY Entry = RemoveHeadList(&(((PNKLIST_HEAD)ListHandle)->ListHead));
        ASSERT(NULL != Entry);
        ((PNKLIST_HEAD)ListHandle)->Free(Entry);
    }
    ExReleaseResourceLite(&(((PNKLIST_HEAD)ListHandle)->Lock));
    KeLeaveCriticalRegion();
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkInsertHeadList(
                 _In_ HANDLE ListHandle,
                 _In_ PLIST_ENTRY Entry,
                 _In_ BOOLEAN Unique
                 )
{
    NTSTATUS     Status = STATUS_NOT_FOUND;
    PNKLIST_HEAD NkList = (PNKLIST_HEAD)ListHandle;

    ASSERT(NULL != NkList);
    ASSERT(NULL != NkList->Compare);

    KeEnterCriticalRegion();
    (VOID)ExAcquireResourceExclusiveLite(&NkList->Lock, TRUE);
    if (Unique) {
        PLIST_ENTRY CurEntry = NULL;
        for (CurEntry = NkList->ListHead.Flink; CurEntry != &NkList->ListHead; CurEntry = CurEntry->Flink) {
            if (NkList->Compare(CurEntry, Entry)) {
                // Already exists
                Status = STATUS_OBJECTID_EXISTS;
                break;
            }
        }
    }
    if (STATUS_NOT_FOUND == Status) {
        InsertHeadList(&NkList->ListHead, Entry);
        Status = STATUS_SUCCESS;
    }
    ExReleaseResourceLite(&NkList->Lock);
    KeLeaveCriticalRegion();

    return Status;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkInsertTailList(
                 _In_ HANDLE ListHandle,
                 _In_ PLIST_ENTRY Entry,
                 _In_ BOOLEAN Unique
                 )
{
    NTSTATUS     Status = STATUS_NOT_FOUND;
    PNKLIST_HEAD NkList = (PNKLIST_HEAD)ListHandle;

    ASSERT(NULL != NkList);
    ASSERT(NULL != NkList->Compare);

    KeEnterCriticalRegion();
    (VOID)ExAcquireResourceExclusiveLite(&NkList->Lock, TRUE);
    if (Unique) {
        PLIST_ENTRY CurEntry = NULL;
        for (CurEntry = NkList->ListHead.Flink; CurEntry != &NkList->ListHead; CurEntry = CurEntry->Flink) {
            if (NkList->Compare(CurEntry, Entry)) {
                // Already exists
                Status = STATUS_OBJECTID_EXISTS;
                break;
            }
        }
    }
    if (STATUS_NOT_FOUND == Status) {
        InsertTailList(&NkList->ListHead, Entry);
        Status = STATUS_SUCCESS;
    }
    ExReleaseResourceLite(&NkList->Lock);
    KeLeaveCriticalRegion();

    return Status;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
PLIST_ENTRY
NkRemoveHeadList(
                 _In_ HANDLE ListHandle
                 )
{
    PLIST_ENTRY Entry = NULL;

    KeEnterCriticalRegion();
    (VOID)ExAcquireResourceExclusiveLite(&(((PNKLIST_HEAD)ListHandle)->Lock), TRUE);
    if (!IsListEmpty(&(((PNKLIST_HEAD)ListHandle)->ListHead))) {
        Entry = RemoveHeadList(&(((PNKLIST_HEAD)ListHandle)->ListHead));
    }
    ExReleaseResourceLite(&(((PNKLIST_HEAD)ListHandle)->Lock));
    KeLeaveCriticalRegion();

    return Entry;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
PLIST_ENTRY
NkRemoveTailList(
                 _In_ HANDLE ListHandle
                 )
{
    PLIST_ENTRY Entry = NULL;

    KeEnterCriticalRegion();
    (VOID)ExAcquireResourceExclusiveLite(&(((PNKLIST_HEAD)ListHandle)->Lock), TRUE);
    if (!IsListEmpty(&(((PNKLIST_HEAD)ListHandle)->ListHead))) {
        Entry = RemoveTailList(&(((PNKLIST_HEAD)ListHandle)->ListHead));
    }
    ExReleaseResourceLite(&(((PNKLIST_HEAD)ListHandle)->Lock));
    KeLeaveCriticalRegion();

    return Entry;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
BOOLEAN
NkRemoveEntryList(
                  _In_ HANDLE ListHandle,
                  _In_ PLIST_ENTRY Entry
                  )
{
    BOOLEAN Result = TRUE;

    KeEnterCriticalRegion();
    (VOID)ExAcquireResourceExclusiveLite(&(((PNKLIST_HEAD)ListHandle)->Lock), TRUE);
    Result = RemoveEntryList(Entry);
    ExReleaseResourceLite(&(((PNKLIST_HEAD)ListHandle)->Lock));
    KeLeaveCriticalRegion();

    return Result;
}



_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkTraverseList(
               _In_ HANDLE ListHandle,
               _In_ BOOLEAN Exclusive,
               _In_ NKLIST_TRAVERSE_ROUTINE Traverse
               )
{
    NTSTATUS     Status = STATUS_SUCCESS;
    PNKLIST_HEAD NkList = (PNKLIST_HEAD)ListHandle;
    PLIST_ENTRY  Entry = NULL;
    BOOLEAN      Continue = TRUE;

    ASSERT(NULL != NkList);
    ASSERT(NULL != NkList->Compare);

    KeEnterCriticalRegion();
    if (Exclusive) {
        (VOID)ExAcquireResourceExclusiveLite(&NkList->Lock, TRUE);
    }
    else {
        (VOID)ExAcquireResourceSharedLite(&NkList->Lock, TRUE);
    }

    for (Entry = NkList->ListHead.Flink; Entry != &NkList->ListHead; Entry = Entry->Flink) {        
        Status = Traverse(Entry, &Continue);
    }

    ExReleaseResourceLite(&NkList->Lock);
    KeLeaveCriticalRegion();

    return Status;
}