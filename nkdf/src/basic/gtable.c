
#include <ntifs.h>
#include <Ntstrsafe.h>


#include <nkdf/basic/defines.h>
#include <nkdf/basic/gtable.h>



typedef struct _NKSAFE_GENERIC_TABLE {
    RTL_GENERIC_TABLE       NkTable;
    ERESOURCE               Lock;
    ULONG                   ElementSize;
    POOL_TYPE               PoolType;
    ULONG                   PoolTag;
    LOOKASIDE_LIST_EX       Lookaside;
} NKSAFE_GENERIC_TABLE, *PNKSAFE_GENERIC_TABLE;


_IRQL_requires_same_
_Function_class_(RTL_GENERIC_ALLOCATE_ROUTINE)
__drv_allocatesMem(Mem)
PVOID
NTAPI
TableAllocRoutine(
                  _In_ struct _RTL_GENERIC_TABLE *Table,
                  _In_ CLONG ByteSize
                  );

_IRQL_requires_same_
_Function_class_(RTL_GENERIC_FREE_ROUTINE)
VOID
NTAPI
TableFreeRoutine(
                 _In_ struct _RTL_GENERIC_TABLE *Table,
                 _In_ __drv_freesMem(Mem) _Post_invalid_ PVOID Buffer
                 );




//  Assign text sections for each routine.
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NkCreateSafeGenericTable)
#pragma alloc_text(PAGE, NkDestroySafeGenericTable)
#endif




_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCreateSafeGenericTable(
                         _Out_ PHANDLE TableHandle,
                         _In_ PRTL_GENERIC_COMPARE_ROUTINE CompareRoutine,
                         _In_ ULONG ElementSize,
                         _In_ POOL_TYPE PoolType,
                         _In_ ULONG PoolTag
                         )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    PNKSAFE_GENERIC_TABLE Table = NULL;


    PAGED_CODE();


    try {

        *TableHandle = NULL;

        Table = ExAllocatePoolWithTag(NonPagedPool, sizeof(NKSAFE_GENERIC_TABLE), TAG_TABLE);
        if (NULL == Table) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory(Table, sizeof(NKSAFE_GENERIC_TABLE));
        Status = ExInitializeLookasideListEx(&Table->Lookaside,
                                             NULL,
                                             NULL,
                                             PoolType,
                                             EX_LOOKASIDE_LIST_EX_FLAGS_FAIL_NO_RAISE,
                                             ElementSize,
                                             PoolTag,
                                             0);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = ExInitializeResourceLite(&Table->Lock);
        if (!NT_SUCCESS(Status)) {
            ExDeleteLookasideListEx(&Table->Lookaside);
            try_return(Status);
        }

        Table->ElementSize = ElementSize;
        Table->PoolType = PoolType;
        Table->PoolTag = PoolTag;

        RtlInitializeGenericTable(&Table->NkTable,
                                  CompareRoutine,
                                  TableAllocRoutine,
                                  TableFreeRoutine,
                                  NULL
                                  );


        *TableHandle = (HANDLE)Table;
        Table = NULL; // Transfer ownership
        Status = STATUS_SUCCESS;


try_exit: NOTHING;
    }
    finally {
        if (!NT_SUCCESS(Status)) {
            if (NULL != Table) {
                // free resources
                ExFreePool(Table);
                Table = NULL;
            }
        }
    }

    return Status;
}

_IRQL_requires_(PASSIVE_LEVEL)
VOID
NkDestroySafeGenericTable(
                          _In_ HANDLE TableHandle
                          )
{
    PNKSAFE_GENERIC_TABLE TableObject = (PNKSAFE_GENERIC_TABLE)TableHandle;
    PVOID Buffer = NULL;


    PAGED_CODE();


    ASSERT(NULL != TableHandle);
    ExEnterCriticalRegionAndAcquireResourceExclusive(&TableObject->Lock);
    // Clean table
    while (NULL != (Buffer = RtlGetElementGenericTable(&TableObject->NkTable, 0))) {
        RtlDeleteElementGenericTable(&TableObject->NkTable, Buffer);
        Buffer = NULL;
    }
    // Delete LookasideList
    ExDeleteLookasideListEx(&TableObject->Lookaside);
    ExReleaseResourceAndLeaveCriticalRegion(&TableObject->Lock);
    ExDeleteResourceLite(&TableObject->Lock);
    ExFreePool(TableObject);
}


_Check_return_
_IRQL_requires_max_(APC_LEVEL)
BOOLEAN
NkIsGenericTableEmpty(
                      _In_ HANDLE TableHandle
                      )
{
    PNKSAFE_GENERIC_TABLE TableObject = (PNKSAFE_GENERIC_TABLE)TableHandle;
    BOOLEAN IsEmpty = TRUE;
    

    ASSERT(NULL != TableHandle);
    ExEnterCriticalRegionAndAcquireResourceShared(&TableObject->Lock);
    // Clean table
    IsEmpty = RtlIsGenericTableEmpty(&TableObject->NkTable);
    ExReleaseResourceAndLeaveCriticalRegion(&TableObject->Lock);

    return IsEmpty;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
ULONG
NkNumberGenericTableElements(
                             _In_ HANDLE TableHandle
                             )
{
    PNKSAFE_GENERIC_TABLE TableObject = (PNKSAFE_GENERIC_TABLE)TableHandle;
    ULONG Number = 0;
    

    ASSERT(NULL != TableHandle);
    ExEnterCriticalRegionAndAcquireResourceShared(&TableObject->Lock);
    // Clean table
    Number = RtlNumberGenericTableElements(&TableObject->NkTable);
    ExReleaseResourceAndLeaveCriticalRegion(&TableObject->Lock);

    return Number;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkInsertElementGenericTable(
                            _In_ HANDLE TableHandle,
                            _In_ PVOID Buffer,
                            _In_ CLONG BufferSize
                            )
{
    PNKSAFE_GENERIC_TABLE TableObject = (PNKSAFE_GENERIC_TABLE)TableHandle;
    BOOLEAN IsNew = TRUE;
    PVOID Element = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    

    ASSERT(NULL != TableHandle);

    try {

        ExEnterCriticalRegionAndAcquireResourceExclusive(&TableObject->Lock);
        // Clean table
        Element = RtlInsertElementGenericTable(&TableObject->NkTable, Buffer, BufferSize, &IsNew);
        if (NULL == Element) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }
        if (!IsNew) {
            try_return(Status = STATUS_OBJECTID_EXISTS);
        }

try_exit: NOTHING;
    }
    finally {
        ExReleaseResourceAndLeaveCriticalRegion(&TableObject->Lock);
    }

    return Status;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkDeleteElementGenericTable(
                            _In_ HANDLE TableHandle,
                            _In_ PVOID Buffer,
                            _In_opt_ PNKGT_OPERATE_ROUTINE OperateRoutine,
                            _Inout_opt_ PVOID OperateContext
                            )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    PNKSAFE_GENERIC_TABLE TableObject = (PNKSAFE_GENERIC_TABLE)TableHandle;
    BOOLEAN Removed = FALSE;


    ASSERT(NULL != TableHandle);

    try {

        ExEnterCriticalRegionAndAcquireResourceExclusive(&TableObject->Lock);

        if (NULL != OperateRoutine) {

            PVOID Element = NULL;
            Element = RtlLookupElementGenericTable(&TableObject->NkTable, Buffer);
            if (NULL == Element) {
                try_return(Status = STATUS_NOT_FOUND);
            }
            
            OperateRoutine(&TableObject->NkTable, Element, OperateContext);
        }

        Removed = RtlDeleteElementGenericTable(&TableObject->NkTable, Buffer);

try_exit: NOTHING;
    }
    finally {
        ExReleaseResourceAndLeaveCriticalRegion(&TableObject->Lock);
        if (NT_SUCCESS(Status)) {
            Status = Removed ? STATUS_SUCCESS : STATUS_NOT_FOUND;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
BOOLEAN
NkGetElementGenericTable(
                         _In_ HANDLE TableHandle,
                         _In_ ULONG Index,
                         _In_ BOOLEAN Exclusive,
                         _In_ PNKGT_OPERATE_ROUTINE OperateRoutine,
                         _Inout_opt_ PVOID OperateContext
                         )
{
    PNKSAFE_GENERIC_TABLE TableObject = (PNKSAFE_GENERIC_TABLE)TableHandle;
    BOOLEAN Status = FALSE;
    PVOID Element = NULL;


    ASSERT(NULL != TableHandle);

    try {

        if (Exclusive)
            ExEnterCriticalRegionAndAcquireResourceExclusive(&TableObject->Lock);
        else
            ExEnterCriticalRegionAndAcquireResourceShared(&TableObject->Lock);

        if (Index >= RtlNumberGenericTableElements(&TableObject->NkTable)) {
            try_return(Status = FALSE);
        }

        Element = RtlGetElementGenericTable(&TableObject->NkTable, Index);
        ASSERT(NULL != Element);
        Status = TRUE;

        if (NULL != OperateRoutine) {
            OperateRoutine(&TableObject->NkTable, Element, OperateContext);
        }

try_exit: NOTHING;
    }
    finally {
        ExReleaseResourceAndLeaveCriticalRegion(&TableObject->Lock);
    }

    return Status;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
BOOLEAN
NkLookupElementGenericTable(
                            _In_ HANDLE TableHandle,
                            _In_ PVOID Buffer,
                            _In_ BOOLEAN Exclusive,
                            _In_opt_ PNKGT_OPERATE_ROUTINE OperateRoutine,
                            _Inout_opt_ PVOID OperateContext
                            )
{
    PNKSAFE_GENERIC_TABLE TableObject = (PNKSAFE_GENERIC_TABLE)TableHandle;
    BOOLEAN Status = FALSE;
    PVOID Element = NULL;


    ASSERT(NULL != TableHandle);

    try {

        if (Exclusive)
            ExEnterCriticalRegionAndAcquireResourceExclusive(&TableObject->Lock);
        else
            ExEnterCriticalRegionAndAcquireResourceShared(&TableObject->Lock);

        Element = RtlLookupElementGenericTable(&TableObject->NkTable, Buffer);
        if (NULL == Element) {
            try_return(Status = FALSE);
        }

        // Found
        Status = TRUE;

        if (NULL != OperateRoutine) {
            OperateRoutine(&TableObject->NkTable, Element, OperateContext);
        }

try_exit: NOTHING;
    }
    finally {
        ExReleaseResourceAndLeaveCriticalRegion(&TableObject->Lock);
    }

    return Status;
}

_IRQL_requires_max_(APC_LEVEL)
VOID
NkEnumerateGenericTable(
                        _In_ HANDLE TableHandle,
                        _In_ BOOLEAN Exclusive,
                        _In_ PNKGT_TRAVERSE_ROUTINE TraverseRoutine,
                        _Inout_opt_ PVOID TraverseContext
                        )
{
    PNKSAFE_GENERIC_TABLE TableObject = (PNKSAFE_GENERIC_TABLE)TableHandle;
    PVOID Element = NULL;


    ASSERT(NULL != TableHandle);

    try {

        if (Exclusive)
            ExEnterCriticalRegionAndAcquireResourceExclusive(&TableObject->Lock);
        else
            ExEnterCriticalRegionAndAcquireResourceShared(&TableObject->Lock);

        Element = RtlEnumerateGenericTable(&TableObject->NkTable, TRUE);
        while (NULL != Element) {

            BOOLEAN Continue = TRUE;
            TraverseRoutine(&TableObject->NkTable, Element, &Continue, TraverseContext);
            if (!Continue) {
                break;
            }

            // Move to next
            Element = RtlEnumerateGenericTable(&TableObject->NkTable, FALSE);
        }
    }
    finally {
        ExReleaseResourceAndLeaveCriticalRegion(&TableObject->Lock);
    }
}



//
//  LOCAL ROUTINES
//

_IRQL_requires_same_
_Function_class_(RTL_GENERIC_ALLOCATE_ROUTINE)
__drv_allocatesMem(Mem)
PVOID
NTAPI
TableAllocRoutine(
                  _In_ struct _RTL_GENERIC_TABLE *Table,
                  _In_ CLONG ByteSize
                  )
{
    PNKSAFE_GENERIC_TABLE TableObject = NULL;
    PVOID Element = NULL;


    TableObject = CONTAINING_RECORD(Table, NKSAFE_GENERIC_TABLE, NkTable);
    ASSERT(TableObject->ElementSize >= ByteSize);

    Element = ExAllocateFromLookasideListEx(&TableObject->Lookaside);
    if (NULL != Element) {
        RtlZeroMemory(Element, ByteSize);
    }

    return Element;
}

_IRQL_requires_same_
_Function_class_(RTL_GENERIC_FREE_ROUTINE)
VOID
NTAPI
TableFreeRoutine(
                 _In_ struct _RTL_GENERIC_TABLE *Table,
                 _In_ __drv_freesMem(Mem) _Post_invalid_ PVOID Buffer
                 )
{
    PNKSAFE_GENERIC_TABLE TableObject = NULL;


    TableObject = CONTAINING_RECORD(Table, NKSAFE_GENERIC_TABLE, NkTable);
    ASSERT(NULL != Buffer);

    ExFreeToLookasideListEx(&TableObject->Lookaside, Buffer);
}