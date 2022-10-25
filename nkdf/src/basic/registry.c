

#include <ntifs.h>
#include <Ntstrsafe.h>


#include <nkdf/basic/defines.h>
#include <nkdf/basic/registry.h>


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegOpenKey(
             _Out_ PHANDLE KeyHandle,
             _In_ PUNICODE_STRING KeyPath,
             _In_ ACCESS_MASK DesiredAccess
             )
{
    OBJECT_ATTRIBUTES   KeyAttributes;

    InitializeObjectAttributes( &KeyAttributes,
                                KeyPath,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL);
    return ZwOpenKey(KeyHandle, DesiredAccess, &KeyAttributes);
}



_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegCreateKey(
               _In_ PUNICODE_STRING KeyPath,
               _Out_ PHANDLE KeyHandle,
               _Out_opt_ PBOOLEAN CreateNew
               )
{
    NTSTATUS            Status;
    OBJECT_ATTRIBUTES   KeyAttributes;
    ACCESS_MASK         DesiredAccess = KEY_READ | KEY_WRITE;
    ULONG               Disposition = 0;

    InitializeObjectAttributes( &KeyAttributes,
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

    if (NULL != CreateNew) {
        *CreateNew = (NT_SUCCESS(Status) && REG_CREATED_NEW_KEY == Disposition) ? TRUE : FALSE;
    }
    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegKeyExist(
              _In_ PUNICODE_STRING KeyPath,
              _Out_ PBOOLEAN Exist
              )
{
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    HANDLE      KeyHandle = NULL;


    *Exist = FALSE;

    try {

        Status = NkRegOpenKey(&KeyHandle, KeyPath, KEY_READ);
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
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegDeleteKey(
               _In_ PUNICODE_STRING KeyPath
               )
{
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    HANDLE  KeyHandle = NULL;


    try {

        Status = NkRegOpenKey(&KeyHandle, KeyPath, KEY_ALL_ACCESS | DELETE);
        if (!NT_SUCCESS(Status)) {

            try_return(Status);
        }

        Status = ZwDeleteKey(KeyHandle);

try_exit: NOTHING;
    }
    finally {

        if (NULL != KeyHandle) {
            ZwClose(KeyHandle);
        }

        if (STATUS_OBJECT_NAME_NOT_FOUND == Status) {
            Status = STATUS_SUCCESS;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegDeleteValue(
                 _In_ PUNICODE_STRING KeyPath,
                 _In_ PUNICODE_STRING ValueName
                 )
{
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    HANDLE  KeyHandle = NULL;


    try {

        Status = NkRegOpenKey(&KeyHandle, KeyPath, KEY_ALL_ACCESS);
        if (!NT_SUCCESS(Status)) {

            try_return(Status);
        }

        Status = ZwDeleteValueKey(KeyHandle, ValueName);

try_exit: NOTHING;
    }
    finally {

        if (NULL != KeyHandle) {
            ZwClose(KeyHandle);
        }

        if (STATUS_OBJECT_NAME_NOT_FOUND == Status) {
            Status = STATUS_SUCCESS;
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
NkRegReadIntValue(
                  _In_ PUNICODE_STRING KeyPath,
                  _In_ PUNICODE_STRING ValueName,
                  _Out_ PULONG ValueData
                  )
{
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    ULONG       ValueType;
    ULONGLONG   Data;
    ULONG       DataLength = sizeof(ULONGLONG);


    *ValueData = 0;

    Status = NkRegReadBinaryValue(KeyPath, ValueName, &ValueType, &Data, &DataLength);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    switch (ValueType)
    {
    case REG_DWORD:
    case REG_QWORD:
        ASSERT(sizeof(ULONG) == DataLength || sizeof(ULONGLONG) == DataLength);
        *ValueData = (ULONG)Data;
        break;
    case REG_DWORD_BIG_ENDIAN:
        ASSERT(sizeof(ULONG) == DataLength);
        *ValueData = EndianConvert((ULONG)Data);
        break;
    default:
        Status = STATUS_OBJECT_TYPE_MISMATCH;
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegReadInt64Value(
                    _In_ PUNICODE_STRING KeyPath,
                    _In_ PUNICODE_STRING ValueName,
                    _Out_ PULONGLONG ValueData
                    )
{
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    ULONG       ValueType;
    ULONGLONG   Data;
    ULONG       DataLength = sizeof(ULONGLONG);


    *ValueData = 0;

    Status = NkRegReadBinaryValue(KeyPath, ValueName, &ValueType, &Data, &DataLength);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    switch (ValueType)
    {
    case REG_DWORD:
        ASSERT(sizeof(ULONG) == DataLength);
        *ValueData = (ULONG)Data;
        break;
    case REG_DWORD_BIG_ENDIAN:
        ASSERT(sizeof(ULONG) == DataLength);
        *ValueData = EndianConvert((ULONG)Data);
        break;
    case REG_QWORD:
        ASSERT(sizeof(ULONGLONG) == DataLength);
        *ValueData = Data;
        break;
    default:
        Status = STATUS_INVALID_PARAMETER;
    }

    return Status;
}


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegReadBooleanValue(
                      _In_ PUNICODE_STRING KeyPath,
                      _In_ PUNICODE_STRING ValueName,
                      _Out_ PBOOLEAN ValueData
                      )
{
    NTSTATUS    Status;
    ULONG IntValue = 0;

    Status = NkRegReadIntValue(KeyPath, ValueName, &IntValue);
    if (NT_SUCCESS(Status)) {
        *ValueData = (0 == IntValue) ? FALSE : TRUE;
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegReadStringValue(
                       _In_ PUNICODE_STRING KeyPath,
                       _In_ PUNICODE_STRING ValueName,
                       _In_ BOOLEAN PagePool,
                       _In_ ULONG PoolTag,
                       _Out_ PUNICODE_STRING ValueData
                       )
{
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    ULONG       ValueType;
    ULONG       DataLength = 0;


    RtlZeroMemory(ValueData, sizeof(UNICODE_STRING));

    try {

        Status = NkRegReadBinaryValue(KeyPath, ValueName, &ValueType, NULL, &DataLength);
        if (STATUS_BUFFER_TOO_SMALL != Status) {
            ASSERT(STATUS_SUCCESS != Status);
            try_return(Status);
        }

        if (REG_SZ != ValueType && REG_EXPAND_SZ != ValueType) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        if (0 != DataLength) {

            WCHAR* Buffer = NULL;
            USHORT MaxLength = NKROUND_TO_SIZE(USHORT, DataLength, sizeof(WCHAR));
            Buffer = (WCHAR*)ExAllocatePoolWithTag(PagePool ? PagedPool : NonPagedPool, MaxLength, PoolTag);
            if (NULL == Buffer) {
                try_return(STATUS_INSUFFICIENT_RESOURCES);
            }

            RtlZeroMemory(Buffer, MaxLength);
            DataLength = MaxLength;
            Status = NkRegReadBinaryValue(KeyPath, ValueName, &ValueType, Buffer, &DataLength);
            if (!NT_SUCCESS(Status)) {
                ExFreePool(Buffer);
                Buffer = NULL;
                try_return(Status);
            }

            // Succeed
            RtlInitUnicodeString(ValueData, Buffer); Buffer = NULL;
            ValueData->MaximumLength = MaxLength;
        }

try_exit: NOTHING;
    }
    finally {
        ; // NOTHING
    }

    return Status;
}

#pragma warning(push)
#pragma warning(disable: 28196 6387 6101)
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegReadBinaryValue(
                     _In_ PUNICODE_STRING KeyPath,
                     _In_ PUNICODE_STRING ValueName,
                     _Out_ PULONG ValueType,
                     _Out_writes_opt_(*ValueDataLength) PVOID ValueData,
                     _Inout_ PULONG ValueDataLength
                     )
{
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    HANDLE      KeyHandle = NULL;
    ULONG       Length = 0;
    ULONG       LengthNeeded = 0;
    KEY_VALUE_PARTIAL_INFORMATION  Kpi = { 0 };
    PKEY_VALUE_PARTIAL_INFORMATION Pkpi = NULL;

    if (NULL == KeyPath || NULL == ValueName || NULL == ValueType || NULL == ValueDataLength) {
        return STATUS_INVALID_PARAMETER;
    }
    if (NULL == ValueData && 0 != *ValueDataLength) {
        return STATUS_INVALID_PARAMETER;
    }
    if (NULL != ValueData && 0 == *ValueDataLength) {
        return STATUS_INVALID_PARAMETER;
    }

    // Init out
    if (NULL != ValueData && 0 != *ValueDataLength) {
        RtlZeroMemory(ValueData, *ValueDataLength);
    }    

    try {

        RtlZeroMemory(&Kpi, sizeof(Kpi));

        Status = NkRegOpenKey(&KeyHandle, KeyPath, KEY_READ);
        if (!NT_SUCCESS(Status)) {
            // Unexpected error
            try_return(Status);
        }

        Status = ZwQueryValueKey(KeyHandle,
                                 ValueName,
                                 KeyValuePartialInformation,
                                 &Kpi,
                                 sizeof(Kpi),
                                 &LengthNeeded);

        //
        //  Error except STATUS_BUFFER_OVERFLOW?
        //
        if (!NT_SUCCESS(Status) && STATUS_BUFFER_OVERFLOW != Status) {
            // Unexpected error
            try_return(Status = STATUS_UNEXPECTED_IO_ERROR);
        }

        //
        // Okay, get value type here
        //
        *ValueType = Kpi.Type;
        if (NULL == ValueData) {
            *ValueDataLength = Kpi.DataLength;
            try_return(Status = STATUS_BUFFER_TOO_SMALL);
        }

        //
        // SUCCEED CASE
        //
        if (STATUS_SUCCESS == Status) {

            if (0 != Kpi.DataLength) {

                if (Kpi.DataLength > *ValueDataLength) {
                    RtlCopyMemory(ValueData, Kpi.Data, *ValueDataLength);
                    *ValueDataLength = Kpi.DataLength;
                    Status = STATUS_BUFFER_OVERFLOW;
                }
                else {
                    RtlCopyMemory(ValueData, Kpi.Data, Kpi.DataLength);
                    *ValueDataLength = Kpi.DataLength;
                }
            }
            else {
                *ValueDataLength = 0;
            }

            try_return(Status);
        }



        ASSERT(STATUS_BUFFER_OVERFLOW == Status);
        //
        //  STATUS_BUFFER_OVERFLOW -- we need to allocate more
        //
        Length = LengthNeeded + 32; // Allocate extra 32 bytes to ensure enough buffer
        Pkpi = ExAllocatePoolWithTag(NonPagedPool, Length, TAG_TEMP);
        if (NULL == Pkpi) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory(Pkpi, Length);
        Status = ZwQueryValueKey(KeyHandle,
                                 ValueName,
                                 KeyValuePartialInformation,
                                 Pkpi,
                                 Length,
                                 &LengthNeeded);

        // Shouldn't fail
        ASSERT(NT_SUCCESS(Status));
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        // check length
        if (0 != Pkpi->DataLength) {

            if (Pkpi->DataLength > *ValueDataLength) {
                RtlCopyMemory(ValueData, Pkpi->Data, *ValueDataLength);
                *ValueDataLength = Pkpi->DataLength;
                Status = STATUS_BUFFER_OVERFLOW;
            }
            else {
                RtlCopyMemory(ValueData, Pkpi->Data, Pkpi->DataLength);
                *ValueDataLength = Pkpi->DataLength;
            }
        }
        else {
            *ValueDataLength = 0;
        }

try_exit: NOTHING;
    }
    finally {

        if (NULL != Pkpi) {
            ExFreePool(Pkpi);
        }

        if (NULL != KeyHandle) {
            ZwClose(KeyHandle);
        }
    }

    return Status;
}
#pragma warning(pop)

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegWriteIntValue(
                   _In_ PUNICODE_STRING KeyPath,
                   _In_ PUNICODE_STRING ValueName,
                   _In_ ULONG Data
                   )
{
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    HANDLE      KeyHandle = NULL;


    try {

        Status = NkRegCreateKey(KeyPath, &KeyHandle, NULL);
        if (!NT_SUCCESS(Status)) {

            try_return(Status);
        }

        Status = ZwSetValueKey(KeyHandle, ValueName, 0, REG_DWORD, &Data, sizeof(ULONG));

try_exit: NOTHING;
    }
    finally {

        if (NULL != KeyHandle) {
            ZwClose(KeyHandle);
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegWriteInt64Value(
                     _In_ PUNICODE_STRING KeyPath,
                     _In_ PUNICODE_STRING ValueName,
                     _In_ ULONGLONG Data
                     )
{
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    HANDLE      KeyHandle = NULL;


    try {

        Status = NkRegCreateKey(KeyPath, &KeyHandle, NULL);
        if (!NT_SUCCESS(Status)) {

            try_return(Status);
        }

        Status = ZwSetValueKey(KeyHandle, ValueName, 0, REG_QWORD, &Data, sizeof(ULONGLONG));

    try_exit: NOTHING;
    }
    finally {

        if (NULL != KeyHandle) {
            ZwClose(KeyHandle);
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegWriteBooleanValue(
                       _In_ PUNICODE_STRING KeyPath,
                       _In_ PUNICODE_STRING ValueName,
                       _In_ BOOLEAN Data
                       )
{
    return NkRegWriteIntValue(KeyPath, ValueName, Data ? 1 : 0);
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegWriteStringValue(
                      _In_ PUNICODE_STRING KeyPath,
                      _In_ PUNICODE_STRING ValueName,
                      _In_opt_ const WCHAR* Data
                      )
{
    NTSTATUS     Status = STATUS_UNSUCCESSFUL;
    HANDLE       KeyHandle = NULL;
    const WCHAR* String = NULL;
    ULONG        StringSize = 0;



    try {

        Status = NkRegCreateKey(KeyPath, &KeyHandle, NULL);
        if (!NT_SUCCESS(Status)) {

            try_return(Status);
        }

        if (NULL == Data || L'\0' == Data[0]) {

            // Empty String
            String = L"";
            StringSize = sizeof(WCHAR);
        }
        else {

            UNICODE_STRING TempString = { 0, 0, NULL };

            ASSERT(NULL != Data);
            RtlInitUnicodeString(&TempString, Data);
            String = Data;
            StringSize = TempString.Length + sizeof(WCHAR);
        }

        Status = ZwSetValueKey(KeyHandle, ValueName, 0, REG_SZ, (PVOID)String, StringSize);

try_exit: NOTHING;
    }
    finally {

        if (NULL != KeyHandle) {
            ZwClose(KeyHandle);
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegWriteBinaryValue(
                      _In_ PUNICODE_STRING KeyPath,
                      _In_ PUNICODE_STRING ValueName,
                      _In_ PVOID Data,
                      _In_ ULONG DataSize
                      )
{
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    HANDLE      KeyHandle = NULL;

    if (NULL == Data || 0 == DataSize) {
        return STATUS_INVALID_PARAMETER;
    }


    try {

        Status = NkRegCreateKey(KeyPath, &KeyHandle, NULL);
        if (!NT_SUCCESS(Status)) {

            try_return(Status);
        }

        Status = ZwSetValueKey(KeyHandle, ValueName, 0, REG_BINARY, Data, DataSize);

try_exit: NOTHING;
    }
    finally {

        if (NULL != KeyHandle) {
            ZwClose(KeyHandle);
        }
    }

    return Status;
}
