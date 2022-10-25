

#include "stdafx.h"


#include <nkdf\registry.h>




_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegInterRead(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name,
    _Out_ PULONG Type,
    _Out_writes_opt_(*Size) PVOID Value,
    _Inout_ PULONG Size
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    PKEY_VALUE_PARTIAL_INFORMATION FullKpi = NULL;

    if (NULL == Name || NULL == Type || NULL == Size) {
        return STATUS_INVALID_PARAMETER;
    }
    

    try {

        const ULONG OutputSize = *Size;
        ULONG       LengthNeeded = 0;
        KEY_VALUE_PARTIAL_INFORMATION  BasicKpi = { 0 };

        RtlZeroMemory(&BasicKpi, sizeof(BasicKpi));
        Status = ZwQueryValueKey(KeyHandle,
            Name,
            KeyValuePartialInformation,
            &BasicKpi,
            sizeof(BasicKpi),
            &LengthNeeded);

        //
        //  Error except STATUS_BUFFER_OVERFLOW?
        //
        if (!NT_SUCCESS(Status) && STATUS_BUFFER_OVERFLOW != Status) {
            // Unexpected error
            try_return(Status/*= STATUS_UNEXPECTED_IO_ERROR*/);
        }

        // Get Value Information
        *Type = BasicKpi.Type;

        // Get size only?
        if (NULL == Value || 0 == OutputSize) {
            *Size = BasicKpi.DataLength;
            try_return(Status = STATUS_SUCCESS);    // The purpose is to get data length, and it succeeds
        }
        
        // The basic KPI buffer can hold all the data
        if (NT_SUCCESS(Status)) {
            // No data
            ASSERT(BasicKpi.DataLength <= 1);
            ASSERT(OutputSize >= 1);
            *Size = BasicKpi.DataLength;
            if (0 != BasicKpi.DataLength) {
                RtlCopyMemory(Value, FullKpi->Data, FullKpi->DataLength);
            }
            try_return(Status = STATUS_SUCCESS);
        }

        //
        // The basic KPI buffer cannot hold all the data
        //

        // If output buffer not enough to hold all the data, return an error
        if (BasicKpi.DataLength > OutputSize) {
            *Size = BasicKpi.DataLength;
            try_return(Status = STATUS_BUFFER_TOO_SMALL);
        }

        // Allocate full KPI
        FullKpi = ExAllocatePoolWithTag(NonPagedPool, LengthNeeded, TAG_TMP);
        if (NULL == FullKpi) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }

        // Get full KPI
        RtlZeroMemory(FullKpi, LengthNeeded);
        Status = ZwQueryValueKey(KeyHandle,
            Name,
            KeyValuePartialInformation,
            FullKpi,
            LengthNeeded,
            &LengthNeeded);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        // Return data
        ASSERT(FullKpi->DataLength <= OutputSize);
        RtlCopyMemory(Value, FullKpi->Data, FullKpi->DataLength);
        *Size = FullKpi->DataLength;
        Status = STATUS_SUCCESS;

    try_exit: NOTHING;
    }
    finally {

        if (NULL != FullKpi) {
            ExFreePool(FullKpi);
            FullKpi = NULL;
        }
    }

    return Status;
}


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegInterWrite(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name,
    _In_ ULONG Type,
    _In_ PVOID Value,
    _In_ ULONG Size
    )
{
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;

    if (NULL == Value || 0 == Size) {
        return STATUS_INVALID_PARAMETER;
    }

    try {
        
        Status = ZwSetValueKey(KeyHandle, Name, 0, Type, Value, Size);

    }
    finally {

        if (NULL != KeyHandle) {
            ZwClose(KeyHandle);
        }
    }

    return Status;
}

__forceinline
ULONG
EndianConvert(
    _In_ ULONG Value
    )
{
    ULONG  Ret = 0;
    const UCHAR* Ptr = (const UCHAR*)(&Value);
    UCHAR* PtrOut = (UCHAR*)(&Ret);
    PtrOut[0] = Ptr[3];
    PtrOut[1] = Ptr[2];
    PtrOut[2] = Ptr[1];
    PtrOut[3] = Ptr[0];
    return Ret;
}

__forceinline
ULONGLONG
EndianConvert64(
    _In_ ULONGLONG Value
    )
{
    ULONGLONG  Ret = 0;
    const UCHAR* Ptr = (const UCHAR*)(&Value);
    UCHAR* PtrOut = (UCHAR*)(&Ret);
    PtrOut[0] = Ptr[7];
    PtrOut[1] = Ptr[6];
    PtrOut[2] = Ptr[5];
    PtrOut[3] = Ptr[4];
    PtrOut[4] = Ptr[3];
    PtrOut[5] = Ptr[2];
    PtrOut[6] = Ptr[1];
    PtrOut[7] = Ptr[0];
    return Ret;
}


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegOpen(
    _Out_ PHANDLE KeyHandle,
    _In_ PUNICODE_STRING KeyPath,
    _In_ ACCESS_MASK DesiredAccess
    )
{
    OBJECT_ATTRIBUTES   KeyAttributes = { 0 };

    InitializeObjectAttributes(&KeyAttributes,
        KeyPath,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL);
    return ZwOpenKey(KeyHandle, DesiredAccess, &KeyAttributes);
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegCreate(
    _In_ PUNICODE_STRING KeyPath,
    _Out_ PHANDLE KeyHandle,
    _Out_opt_ PBOOLEAN Created
    )
{
    NTSTATUS            Status;
    OBJECT_ATTRIBUTES   KeyAttributes;
    ACCESS_MASK         DesiredAccess = KEY_READ | KEY_WRITE;
    ULONG               Disposition = 0;

    InitializeObjectAttributes(&KeyAttributes,
        KeyPath,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL);
    Status = ZwCreateKey(KeyHandle,
        DesiredAccess,
        &KeyAttributes,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        &Disposition);

    if (NULL != Created) {
        *Created = (NT_SUCCESS(Status) && REG_CREATED_NEW_KEY == Disposition) ? TRUE : FALSE;
    }
    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegExists(
    _In_ PUNICODE_STRING KeyPath,
    _Out_ PBOOLEAN Exist
    )
{
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    HANDLE      KeyHandle = NULL;

    *Exist = FALSE;

    try {

        Status = NkRegOpen(&KeyHandle, KeyPath, KEY_READ);
        if (NT_SUCCESS(Status)) {
            *Exist = TRUE;
        }
        else {
            if (STATUS_OBJECT_NAME_NOT_FOUND == Status) {
                Status = STATUS_SUCCESS;
            }
        }
    }
    finally {
        if (NULL != KeyHandle) {
            ZwClose(KeyHandle);
            KeyHandle = NULL;
        }
    }

    return Status;
}

VOID
NkRegClose(
    _In_opt_ HANDLE KeyHandle
    )
{
    if (NULL != KeyHandle) {
        ZwClose(KeyHandle);
    }
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegDelete(
    _In_ PUNICODE_STRING KeyPath,
    _In_ BOOLEAN SucceedOnNonExist
    )
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    HANDLE KeyHandle = NULL;

    try {

        Status = NkRegOpen(&KeyHandle, KeyPath, KEY_ALL_ACCESS | DELETE);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = ZwDeleteKey(KeyHandle);

    try_exit: NOTHING;
    }
    finally {
        if (NULL != KeyHandle) {
            ZwClose(KeyHandle);
            KeyHandle = NULL;
        }
        if (STATUS_OBJECT_NAME_NOT_FOUND == Status && SucceedOnNonExist) {
            Status = STATUS_SUCCESS;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegRemoveValue(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name
    )
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    try {

        Status = ZwDeleteValueKey(KeyHandle, Name);

    }
    finally {
        if (STATUS_OBJECT_NAME_NOT_FOUND == Status) {
            Status = STATUS_SUCCESS;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegGetValueInfo(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name,
    _Out_opt_ PULONG Type,
    _Out_opt_ PULONG Size
    )
{
    ULONG ValueType = REG_NONE;
    ULONG ValueSize = 0;

    NTSTATUS Status = NkRegInterRead(KeyHandle, Name, &ValueType, NULL, &ValueSize);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    if (NULL != Type) *Type = ValueType;
    if (NULL != Size) *Size = ValueSize;
    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegReadInt(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name,
    _Out_ PULONG Value
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Type = REG_NONE;
    ULONG Size = sizeof(ULONGLONG);
    ULONGLONG Value64 = 0;

    Status = NkRegInterRead(KeyHandle, Name, &Type, &Value64, &Size);
    if (!NT_SUCCESS(Status)) {
        *Value = 0;
        return Status;
    }

    switch (Type)
    {
    case REG_DWORD:
    case REG_QWORD:
        *Value = (ULONG)Value64;
        break;
    case REG_DWORD_BIG_ENDIAN:
        *Value = EndianConvert((ULONG)Value64);
        break;
    default:
        *Value = 0;
        Status = STATUS_OBJECT_TYPE_MISMATCH;
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegReadInt64(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name,
    _Out_ PULONGLONG Value
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Type = REG_NONE;
    ULONG Size = sizeof(ULONGLONG);
    ULONGLONG Value64 = 0;

    Status = NkRegInterRead(KeyHandle, Name, &Type, &Value64, &Size);
    if (!NT_SUCCESS(Status)) {
        *Value = 0;
        return Status;
    }

    switch (Type)
    {
    case REG_DWORD:
    case REG_QWORD:
        *Value = Value64;
        break;
    case REG_DWORD_BIG_ENDIAN:
        *Value = EndianConvert((ULONG)Value64);
        break;
    default:
        *Value = 0;
        Status = STATUS_OBJECT_TYPE_MISMATCH;
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegReadString(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name,
    _Out_writes_opt_(*Size) WCHAR* Buffer,
    _Inout_ PULONG Size
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Type = REG_NONE;
    const ULONG OutputSize = *Size;

    try {

        Status = NkRegInterRead(KeyHandle, Name, &Type, Buffer, Size);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        if (Type != REG_SZ && Type != REG_EXPAND_SZ && Type != REG_MULTI_SZ) {
            try_return(Status = STATUS_OBJECT_TYPE_MISMATCH);
        }

    try_exit: NOTHING;
    }
    finally {
        RtlZeroMemory(Buffer, OutputSize);
        *Size = 0;
    }

    return STATUS_SUCCESS;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegReadMultiString(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name,
    _Out_writes_opt_z_(*Size) WCHAR* Buffer,
    _Inout_ PULONG Size
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Type = REG_NONE;
    const ULONG OutputSize = *Size;

    try {

        Status = NkRegInterRead(KeyHandle, Name, &Type, Buffer, Size);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        if (Type != REG_MULTI_SZ) {
            try_return(Status = STATUS_OBJECT_TYPE_MISMATCH);
        }

    try_exit: NOTHING;
    }
    finally {
        RtlZeroMemory(Buffer, OutputSize);
        *Size = 0;
    }

    return STATUS_SUCCESS;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegReadBinary(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name,
    _Out_writes_opt_(*Size) PVOID Value,
    _Inout_ PULONG Size
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Type = REG_NONE;
    const ULONG OutputSize = *Size;

    try {

        Status = NkRegInterRead(KeyHandle, Name, &Type, Value, Size);
        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        if (Type != REG_BINARY) {
            try_return(Status = STATUS_OBJECT_TYPE_MISMATCH);
        }

    try_exit: NOTHING;
    }
    finally {
        RtlZeroMemory(Value, OutputSize);
        *Size = 0;
    }

    return STATUS_SUCCESS;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegWriteInt(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name,
    _In_ ULONG Value
    )
{
    return NkRegInterWrite(KeyHandle, Name, REG_QWORD, &Value, sizeof(ULONG));
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegWriteInt64(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name,
    _In_ ULONGLONG Value
    )
{
    return NkRegInterWrite(KeyHandle, Name, REG_QWORD, &Value, sizeof(ULONGLONG));
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegWriteString(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name,
    _In_opt_ const WCHAR* Value
    )
{
    if (NULL == Value) {
        static const WCHAR EmptyString[1] = { 0 };
        return NkRegInterWrite(KeyHandle, Name, REG_SZ, (PVOID)EmptyString, sizeof(WCHAR));
    }
    else {
        UNICODE_STRING Str = { 0, 0, NULL };
        RtlInitUnicodeString(&Str, Value);
        return NkRegInterWrite(KeyHandle, Name, REG_SZ, Str.Buffer, Str.Length + sizeof(WCHAR));
    }
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegWriteMultiString(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name,
    _In_opt_ const WCHAR* Value
    )
{
    if (NULL == Value) {
        static const WCHAR EmptyString[1] = { 0 };
        return NkRegInterWrite(KeyHandle, Name, REG_MULTI_SZ, (PVOID)EmptyString, sizeof(WCHAR));
    }
    else {
        UNICODE_STRING Str = { 0, 0, NULL };
        RtlInitUnicodeString(&Str, Value);
        return NkRegInterWrite(KeyHandle, Name, REG_MULTI_SZ, Str.Buffer, Str.Length + sizeof(WCHAR));
    }
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegWriteBinary(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name,
    _In_ PVOID Value,
    _In_ ULONG Size
    )
{
    return NkRegInterWrite(KeyHandle, Name, REG_BINARY, Value, Size);
}


//
//  Reg Quick
//

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegQuickGetValueInfo(
    _In_ PUNICODE_STRING KeyPath,
    _In_ PUNICODE_STRING Name,
    _Out_opt_ PULONG Type,
    _Out_opt_ PULONG Size
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE KeyHandle = NULL;

    try {

        Status = NkRegOpen(&KeyHandle, KeyPath, KEY_READ);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = NkRegGetValueInfo(KeyHandle, Name, Type, Size);

    try_exit: NOTHING;
    }
    finally {
        if (NULL != KeyHandle) {
            ZwClose(KeyHandle);
            KeyHandle = NULL;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegQuickReadInt(
    _In_ PUNICODE_STRING KeyPath,
    _In_ PUNICODE_STRING Name,
    _Out_ PULONG Value
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE KeyHandle = NULL;

    try {

        Status = NkRegOpen(&KeyHandle, KeyPath, KEY_READ);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = NkRegReadInt(KeyHandle, Name, Value);

    try_exit: NOTHING;
    }
    finally {
        if (NULL != KeyHandle) {
            ZwClose(KeyHandle);
            KeyHandle = NULL;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegQuickReadInt64(
    _In_ PUNICODE_STRING KeyPath,
    _In_ PUNICODE_STRING Name,
    _Out_ PULONGLONG Value
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE KeyHandle = NULL;

    try {

        Status = NkRegOpen(&KeyHandle, KeyPath, KEY_READ);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = NkRegReadInt64(KeyHandle, Name, Value);

    try_exit: NOTHING;
    }
    finally {
        if (NULL != KeyHandle) {
            ZwClose(KeyHandle);
            KeyHandle = NULL;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegQuickReadString(
    _In_ PUNICODE_STRING KeyPath,
    _In_ PUNICODE_STRING Name,
    _Out_writes_opt_(*Size) WCHAR* Buffer,
    _Inout_ PULONG Size
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE KeyHandle = NULL;

    try {

        Status = NkRegOpen(&KeyHandle, KeyPath, KEY_READ);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = NkRegReadString(KeyHandle, Name, Buffer, Size);

    try_exit: NOTHING;
    }
    finally {
        if (NULL != KeyHandle) {
            ZwClose(KeyHandle);
            KeyHandle = NULL;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegQuickReadMultiString(
    _In_ PUNICODE_STRING KeyPath,
    _In_ PUNICODE_STRING Name,
    _Out_writes_opt_z_(*Size) WCHAR* Buffer,
    _Inout_ PULONG Size
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE KeyHandle = NULL;

    try {

        Status = NkRegOpen(&KeyHandle, KeyPath, KEY_READ);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = NkRegReadMultiString(KeyHandle, Name, Buffer, Size);

    try_exit: NOTHING;
    }
    finally {
        if (NULL != KeyHandle) {
            ZwClose(KeyHandle);
            KeyHandle = NULL;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegQuickReadBinary(
    _In_ PUNICODE_STRING KeyPath,
    _In_ PUNICODE_STRING Name,
    _Out_writes_opt_(*Size) PVOID Value,
    _Inout_ PULONG Size
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE KeyHandle = NULL;

    try {

        Status = NkRegOpen(&KeyHandle, KeyPath, KEY_READ);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = NkRegReadBinary(KeyHandle, Name, Value, Size);

    try_exit: NOTHING;
    }
    finally {
        if (NULL != KeyHandle) {
            ZwClose(KeyHandle);
            KeyHandle = NULL;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegQuickWriteInt(
    _In_ PUNICODE_STRING KeyPath,
    _In_ PUNICODE_STRING Name,
    _In_ ULONG Value
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE KeyHandle = NULL;

    try {

        Status = NkRegCreate(KeyPath, &KeyHandle, NULL);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = NkRegWriteInt(KeyHandle, Name, Value);

    try_exit: NOTHING;
    }
    finally {
        if (NULL != KeyHandle) {
            ZwClose(KeyHandle);
            KeyHandle = NULL;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegQuickWriteInt64(
    _In_ PUNICODE_STRING KeyPath,
    _In_ PUNICODE_STRING Name,
    _In_ ULONGLONG Value
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE KeyHandle = NULL;

    try {

        Status = NkRegCreate(KeyPath, &KeyHandle, NULL);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = NkRegWriteInt64(KeyHandle, Name, Value);

    try_exit: NOTHING;
    }
    finally {
        if (NULL != KeyHandle) {
            ZwClose(KeyHandle);
            KeyHandle = NULL;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegQuickWriteString(
    _In_ PUNICODE_STRING KeyPath,
    _In_ PUNICODE_STRING Name,
    _In_opt_ const WCHAR* Value
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE KeyHandle = NULL;

    try {

        Status = NkRegCreate(KeyPath, &KeyHandle, NULL);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = NkRegWriteString(KeyHandle, Name, Value);

    try_exit: NOTHING;
    }
    finally {
        if (NULL != KeyHandle) {
            ZwClose(KeyHandle);
            KeyHandle = NULL;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegQuickWriteMultiString(
    _In_ PUNICODE_STRING KeyPath,
    _In_ PUNICODE_STRING Name,
    _In_opt_ const WCHAR* Value
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE KeyHandle = NULL;

    try {

        Status = NkRegCreate(KeyPath, &KeyHandle, NULL);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = NkRegWriteMultiString(KeyHandle, Name, Value);

    try_exit: NOTHING;
    }
    finally {
        if (NULL != KeyHandle) {
            ZwClose(KeyHandle);
            KeyHandle = NULL;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegQuickWriteBinary(
    _In_ PUNICODE_STRING KeyPath,
    _In_ PUNICODE_STRING Name,
    _In_ PVOID Value,
    _In_ ULONG Size
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE KeyHandle = NULL;

    try {

        Status = NkRegCreate(KeyPath, &KeyHandle, NULL);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = NkRegWriteBinary(KeyHandle, Name, Value, Size);

    try_exit: NOTHING;
    }
    finally {
        if (NULL != KeyHandle) {
            ZwClose(KeyHandle);
            KeyHandle = NULL;
        }
    }

    return Status;
}