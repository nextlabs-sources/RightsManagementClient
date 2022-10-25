


#pragma once
#ifndef __NKDF_REGISTRY_H__
#define __NKDF_REGISTRY_H__



_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegOpen(
    _Out_ PHANDLE KeyHandle,
    _In_ PUNICODE_STRING KeyPath,
    _In_ ACCESS_MASK DesiredAccess
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegCreate(
    _In_ PUNICODE_STRING KeyPath,
    _Out_ PHANDLE KeyHandle,
    _Out_opt_ PBOOLEAN Created
    );

VOID
NkRegClose(
    _In_opt_ HANDLE KeyHandle
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegExists(
    _In_ PUNICODE_STRING KeyPath,
    _Out_ PBOOLEAN Exist
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegDelete(
    _In_ PUNICODE_STRING KeyPath,
    _In_ BOOLEAN SucceedOnNonExist
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegRemoveValue(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegGetValueInfo(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name,
    _Out_opt_ PULONG Type,
    _Out_opt_ PULONG Size
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegReadInt(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name,
    _Out_ PULONG Value
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegReadInt64(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name,
    _Out_ PULONGLONG Value
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegReadString(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name,
    _Out_writes_opt_(*Size) WCHAR* Buffer,
    _Inout_ PULONG Size
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegReadMultiString(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name,
    _Out_writes_opt_z_(*Size) WCHAR* Buffer,
    _Inout_ PULONG Size
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegReadBinary(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name,
    _Out_writes_opt_(*Size) PVOID Value,
    _Inout_ PULONG Size
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegWriteInt(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name,
    _In_ ULONG Data
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegWriteInt64(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name,
    _In_ ULONGLONG Data
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegWriteString(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name,
    _In_opt_ const WCHAR* Data
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegWriteMultiString(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name,
    _In_opt_ const WCHAR* Data
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegWriteBinary(
    _In_ HANDLE KeyHandle,
    _In_ PUNICODE_STRING Name,
    _In_ PVOID Value,
    _In_ ULONG Size
    );


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegQuickGetValueInfo(
    _In_ PUNICODE_STRING KeyPath,
    _In_ PUNICODE_STRING Name,
    _Out_opt_ PULONG Type,
    _Out_opt_ PULONG Size
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegQuickReadInt(
    _In_ PUNICODE_STRING KeyPath,
    _In_ PUNICODE_STRING Name,
    _Out_ PULONG Value
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegQuickReadInt64(
    _In_ PUNICODE_STRING KeyPath,
    _In_ PUNICODE_STRING Name,
    _Out_ PULONGLONG Value
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegQuickReadString(
    _In_ PUNICODE_STRING KeyPath,
    _In_ PUNICODE_STRING Name,
    _Out_writes_opt_(*Size) WCHAR* Buffer,
    _Inout_ PULONG Size
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegQuickReadMultiString(
    _In_ PUNICODE_STRING KeyPath,
    _In_ PUNICODE_STRING Name,
    _Out_writes_opt_z_(*Size) WCHAR* Buffer,
    _Inout_ PULONG Size
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegQuickReadBinary(
    _In_ PUNICODE_STRING KeyPath,
    _In_ PUNICODE_STRING Name,
    _Out_writes_opt_(*Size) PVOID Value,
    _Inout_ PULONG Size
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegQuickWriteInt(
    _In_ PUNICODE_STRING KeyPath,
    _In_ PUNICODE_STRING Name,
    _In_ ULONG Value
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegQuickWriteInt64(
    _In_ PUNICODE_STRING KeyPath,
    _In_ PUNICODE_STRING Name,
    _In_ ULONGLONG Value
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegQuickWriteString(
    _In_ PUNICODE_STRING KeyPath,
    _In_ PUNICODE_STRING Name,
    _In_opt_ const WCHAR* Value
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegQuickWriteMultiString(
    _In_ PUNICODE_STRING KeyPath,
    _In_ PUNICODE_STRING Name,
    _In_opt_ const WCHAR* Value
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegQuickWriteBinary(
    _In_ PUNICODE_STRING KeyPath,
    _In_ PUNICODE_STRING Name,
    _In_ PVOID Value,
    _In_ ULONG Size
    );



#endif