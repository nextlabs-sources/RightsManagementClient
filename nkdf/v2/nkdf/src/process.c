

#include "stdafx.h"

#include <ntimage.h>

#include <nkdf/ntapi.h>
#include <nkdf/process.h>


//  Assign text sections for each routine.
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NkGetProcessImageName)
#pragma alloc_text(PAGE, NkGetProcessImageNameById)
#pragma alloc_text(PAGE, NkGetProcessImageNameByHandle)
#pragma alloc_text(PAGE, NkGetProcessInfo)
#pragma alloc_text(PAGE, NkOpenProcess)
#pragma alloc_text(PAGE, NkTerminateProcess)
#pragma alloc_text(PAGE, NkIsNativeProcess)
#pragma alloc_text(PAGE, NkIs64bitProcess)
#pragma alloc_text(PAGE, NkIsSystemProcess)
#pragma alloc_text(PAGE, NkGetProcessEntryPoint)
#endif



//
//  Get Internal APIs
//

_Check_return_
PEPROCESS
NkGetThreadProcess(
    _In_ PETHREAD Thread
    )
{
    return NkPsGetThreadProcess(Thread);
}

_Check_return_
HANDLE
NkGetThreadId(
    _In_ PETHREAD Thread
    )
{
    return NkPsGetThreadId(Thread);
}

_Check_return_
HANDLE
NkGetThreadProcessId(
    _In_ PETHREAD Thread
    )
{
    return NkPsGetThreadProcessId(Thread);
}

_Check_return_
NTSTATUS
NkProcessHandleToId(
    _In_ HANDLE ProcessHandle,
    _Out_ PHANDLE ProcessId
    )
{
    NTSTATUS        Status = STATUS_UNSUCCESSFUL;
    PROCESS_BASIC_INFORMATION pbi = { 0 };

    *ProcessId = NULL;

    Status = NkZwQueryInformationProcess(ProcessHandle, ProcessBasicInformation, &pbi, sizeof(pbi), NULL);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    *ProcessId = (HANDLE)pbi.UniqueProcessId;
    return STATUS_SUCCESS;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkGetProcessImageNameByHandle(
    _In_ HANDLE Handle,
    _Out_writes_z_(MAX_IMAGE_NAME) PWCH ImageName
    )
{

    NTSTATUS        Status = STATUS_UNSUCCESSFUL;
    ULONG           LengthNeeded = 0;
    ULONG           InfoLength;
    PUNICODE_STRING ImageFileName = NULL;


    PAGED_CODE();


    RtlZeroMemory(ImageName, sizeof(WCHAR) * MAX_IMAGE_NAME);
    ImageName[0] = L'\0';   // Have to add this line to avoid prefast warning

    try {

        Status = NkZwQueryInformationProcess(Handle,
            0x1B,   // ProcessImageFileName
            NULL,
            0,
            &LengthNeeded);
        if (STATUS_INFO_LENGTH_MISMATCH != Status) {
            try_return(Status);
        }

        ASSERT(LengthNeeded != 0);


        InfoLength = sizeof(UNICODE_STRING) + LengthNeeded + sizeof(WCHAR);
        ImageFileName = (PUNICODE_STRING)ExAllocatePoolWithTag(NonPagedPool, InfoLength, TAG_TMP);
        if (NULL == ImageFileName) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory(ImageFileName, InfoLength);
        ImageFileName->MaximumLength = (USHORT)LengthNeeded + sizeof(WCHAR);
        ImageFileName->Length = 0;
        ImageFileName->Buffer = (PWCH)(((PUCHAR)ImageFileName) + sizeof(UNICODE_STRING));

        Status = NkZwQueryInformationProcess(Handle,
            0x1B,   // ProcessImageFileName
            ImageFileName,
            InfoLength,
            &LengthNeeded);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        if (0 == ImageFileName->Length) {
            RtlCopyMemory(ImageName, L"System", 12);
            ImageName[6] = L'\0';
        }
        else {
            USHORT ToCopy = min(ImageFileName->Length, ((MAX_IMAGE_NAME - 1) * sizeof(WCHAR)));    // maximum buffer is 279*2
            RtlCopyMemory(ImageName, ImageFileName->Buffer, ToCopy);
            ImageName[ToCopy / 2] = L'\0';
        }

    try_exit: NOTHING;
    }
    finally {

        if (NULL != ImageFileName) {
            ExFreePool(ImageFileName);
            ImageFileName = NULL;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkGetProcessImageNameById(
    _In_ HANDLE ProcessId,
    _Out_writes_z_(MAX_IMAGE_NAME) PWCH ImageName
    )
{
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    PEPROCESS   Process = NULL;

    PAGED_CODE();


    try {

        Status = PsLookupProcessByProcessId(ProcessId, &Process);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = NkGetProcessImageName(Process, ImageName);

    try_exit: NOTHING;
    }
    finally {

        if (NULL != Process) {
            ObDereferenceObject(Process);
            Process = NULL;
        }
    }


    return Status;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkGetProcessImageName(
    _In_ PEPROCESS Process,
    _Out_writes_z_(MAX_IMAGE_NAME) PWCH ImageName
    )
{
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    HANDLE      Handle = NULL;


    PAGED_CODE();


    try {

        //  Get process Handle
        Status = ObOpenObjectByPointer(Process,
            OBJ_KERNEL_HANDLE,
            NULL,
            GENERIC_READ,
            0,
            KernelMode,
            &Handle);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        ASSERT(NULL != Handle);

        Status = NkGetProcessImageNameByHandle(Handle, ImageName);

    try_exit: NOTHING;
    }
    finally {

        if (NULL != Handle) {
            ZwClose(Handle);
            Handle = NULL;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkGetProcessInfo(
    _In_ HANDLE ProcessId,
    _Out_ PULONG SessionId,
    _Out_writes_z_(MAX_USER_NAME) PWCH UserName,
    _Out_writes_z_(MAX_SID_NAME) PWCH UserSid,
    _Out_writes_z_(MAX_IMAGE_NAME) PWCH ImageName
    )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    PACCESS_TOKEN       AccessToken = NULL;
    TOKEN_USER*         TokenUserInfo = NULL;
    PEPROCESS           Process = NULL;
    PSecurityUserData   UserData = NULL;
    UNICODE_STRING      SidStr = { 0, 0, NULL };

    PAGED_CODE();


    RtlZeroMemory(UserName, MAX_USER_NAME*sizeof(WCHAR));
    RtlZeroMemory(UserSid, MAX_SID_NAME*sizeof(WCHAR));
    RtlZeroMemory(ImageName, MAX_IMAGE_NAME*sizeof(WCHAR));
    // Avoid warnings
    UserName[0] = L'\0';
    UserSid[0] = L'\0';
    ImageName[0] = L'\0';
    SidStr.Length = 0;
    SidStr.MaximumLength = 128;
    SidStr.Buffer = UserSid;

    try {

        Status = PsLookupProcessByProcessId(ProcessId, &Process);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = NkGetProcessImageName(Process, ImageName);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }


        // Get Token
        AccessToken = PsReferencePrimaryToken(Process);


        //  Get SID
        Status = SeQueryInformationToken(AccessToken, TokenUser, (PVOID)&TokenUserInfo);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        if (!RtlValidSid(TokenUserInfo->User.Sid)) {
            try_return(Status = STATUS_INVALID_SID);
        }

        Status = RtlConvertSidToUnicodeString(&SidStr, TokenUserInfo->User.Sid, FALSE);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        if (RtlEqualSid(TokenUserInfo->User.Sid, SeExports->SeLocalServiceSid)) {

            // This is local service user
            RtlCopyMemory(UserName, L"LOCAL SERVICE", 28);
            UserName[14] = 0;
        }
        else if (RtlEqualSid(TokenUserInfo->User.Sid, SeExports->SeNetworkServiceSid)) {

            // This is network service user
            RtlCopyMemory(UserName, L"NETWORK SERVICE", 32);
            UserName[16] = 0;
        }
        else if (RtlEqualSid(TokenUserInfo->User.Sid, SeExports->SeLocalSystemSid)) {

            // This is network service user
            RtlCopyMemory(UserName, L"SYSTEM", 14);
            UserName[7] = 0;
        }
        else {

            LUID    Luid;
            ULONG   ToCopy = 0;

            Status = SeQueryAuthenticationIdToken(AccessToken, &Luid);
            if (!NT_SUCCESS(Status)) {
                try_return(Status);
            }

            Status = GetSecurityUserInfo(&Luid, UNDERSTANDS_LONG_NAMES, &UserData);
            if (!NT_SUCCESS(Status)) {
                try_return(Status);
            }

            ASSERT(NULL != UserData);

            if (0 == UserData->UserName.Length) {
                Status = STATUS_NO_SUCH_USER;
            }

            ToCopy = min(UserData->UserName.Length, ((MAX_USER_NAME - 1)*sizeof(WCHAR)));
            RtlCopyMemory(UserName, UserData->UserName.Buffer, ToCopy);
            UserName[ToCopy / 2] = 0;
        }

        // Get session Id
        if (NULL != SessionId) {
            Status = SeQuerySessionIdToken(AccessToken, SessionId);
            if (!NT_SUCCESS(Status)) {
                *SessionId = 0;
                Status = STATUS_SUCCESS;
            }
        }

        // Finally, succeed
        Status = STATUS_SUCCESS;


    try_exit: NOTHING;
    }
    finally {

        if (NULL != UserData) {
            LsaFreeReturnBuffer(UserData);
            UserData = NULL;
        }

        if (NULL != AccessToken) {
            PsDereferencePrimaryToken(AccessToken);
        }

        if (NULL != Process) {
            ObDereferenceObject(Process);
        }
    }

    return Status;
}


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkOpenProcess(
    _In_ HANDLE ProcessId,
    _In_ ACCESS_MASK DesiredAccess,
    _Out_ PHANDLE ProcessHandle
    )
{
    OBJECT_ATTRIBUTES   ObjAttributes;
    CLIENT_ID           ClientId;

    PAGED_CODE();

    RtlZeroMemory(&ObjAttributes, sizeof(OBJECT_ATTRIBUTES));
    ObjAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
    ClientId.UniqueProcess = ProcessId;
    ClientId.UniqueThread = 0;

    return ZwOpenProcess(ProcessHandle, DesiredAccess, &ObjAttributes, &ClientId);
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkTerminateProcess(
    _In_ HANDLE ProcessId,
    _In_ NTSTATUS ExitStatus
    )
{
    NTSTATUS            Status = STATUS_UNSUCCESSFUL;
    HANDLE              Handle = NULL;

    PAGED_CODE();

    try {

        Status = NkOpenProcess(ProcessId, GENERIC_ALL, &Handle);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = ZwTerminateProcess(Handle, ExitStatus);

    try_exit: NOTHING;
    }
    finally {

        if (NULL != Handle) {
            ZwClose(Handle);
            Handle = NULL;
        }
    }

    return Status;
}



_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkIsNativeProcess(
    _In_ HANDLE ProcessHandle,
    _Out_ PBOOLEAN Result
    )
{
    NTSTATUS                  Status = STATUS_SUCCESS;
    PROCESS_BASIC_INFORMATION BasicInfo = { 0 };
    ULONG_PTR                 SubsysIndex = 0;
    ULONG                     Subsys = 0;
    ULONG                     BytesRead = 0;


    PAGED_CODE();

    *Result = FALSE;

#ifdef _WIN64
    if (!NkIs64bitProcess(ProcessHandle))
        return FALSE;
    SubsysIndex = 0x128;
#else
    SubsysIndex = 0x0B4;
#endif



    // slightly ugly, but I don't know any "official" way to do this
    // (1) get PEB (process environment block) address
    // (2) get PEB->SubSystem
    Status = NkZwQueryInformationProcess(ProcessHandle, ProcessBasicInformation, &BasicInfo, sizeof(BasicInfo), NULL);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = NkZwReadVirtualMemory(ProcessHandle, (PVOID)((ULONG_PTR)BasicInfo.PebBaseAddress + SubsysIndex), &Subsys, 4, &BytesRead);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    *Result = (IMAGE_SUBSYSTEM_NATIVE == Subsys) ? TRUE : FALSE;
    return STATUS_SUCCESS;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
BOOLEAN
NkIs64bitProcess(
    _In_ HANDLE ProcessHandle
    )
{
    BOOLEAN Result = FALSE;
#ifdef _WIN64
    NTSTATUS Status = STATUS_SUCCESS;
    ULONGLONG IsWow64 = 0;
#endif

    PAGED_CODE();


#ifdef _WIN64

    Status = NkZwQueryInformationProcess(ProcessHandle, ProcessWow64Information, &IsWow64, sizeof(IsWow64), 0);
    if (NT_SUCCESS(Status)) {
        if (0 == IsWow64) {
            Result = TRUE;
        }
    }
#else
    UNREFERENCED_PARAMETER(ProcessHandle);
#endif

    return Result;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkIsSystemProcess(
    _In_ HANDLE ProcessHandle,
    _Out_ PBOOLEAN Result
    )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    HANDLE              Token = NULL;
    ULONG               ReturnedLength = 0;
    TOKEN_USER*         TokenUserInfo = NULL;


    *Result = FALSE;

    try {

        Status = ZwOpenProcessTokenEx(ProcessHandle, TOKEN_QUERY, OBJ_KERNEL_HANDLE, &Token);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = ZwQueryInformationToken(Token, TokenUser, NULL, 0, &ReturnedLength);
        if (0 == ReturnedLength) {
            try_return(Status);
        }

        ReturnedLength *= 2;
        TokenUserInfo = ExAllocatePoolWithTag(NonPagedPool, ReturnedLength, TAG_TMP);
        if (NULL == TokenUserInfo) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }
        RtlZeroMemory(TokenUserInfo, ReturnedLength);

        Status = ZwQueryInformationToken(Token, TokenUser, TokenUserInfo, ReturnedLength, &ReturnedLength);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        *Result = (TokenUserInfo->User.Sid == NULL || ((PUCHAR)(TokenUserInfo->User.Sid))[1] <= 1) ? TRUE : FALSE;

    try_exit: NOTHING;
    }
    finally {

        if (NULL != TokenUserInfo) {
            ExFreePool(TokenUserInfo);
            TokenUserInfo = NULL;
        }

        if (NULL != Token) {
            ZwClose(Token);
            Token = NULL;
        }
    }


    return Status;
}


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkGetProcessEntryPoint(
    _In_ HANDLE ProcessHandle,
    _Out_ PVOID* EntryPoint
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;
#ifdef _WIN64
    PVOID buf[8];
#else
    PVOID buf[12];
#endif



    PAGED_CODE();


    *EntryPoint = NULL;


    Status = NkZwQueryInformationProcess(ProcessHandle, ProcessImageInformation, buf, sizeof(buf), 0);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    if (buf[0] == NULL) {

        // Vista should never reach this
        // in XP and 2003 "ProcessImageInformation" doesn't work the normal way
        // it works only if you attach to the target process first
        HANDLE ProcessId = 0;
        PEPROCESS Process = NULL;
        KAPC_STATE ApcState;

        Status = NkProcessHandleToId(ProcessHandle, &ProcessId);
        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        Status = PsLookupProcessByProcessId(ProcessId, &Process);
        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        KeStackAttachProcess(Process, &ApcState);
        Status = NkZwQueryInformationProcess(NtCurrentProcess(), ProcessImageInformation, buf, sizeof(buf), 0);
        if (!NT_SUCCESS(Status)) {
            buf[0] = NULL;
        }
        KeUnstackDetachProcess(&ApcState);
        ObDereferenceObject(Process);
        Process = NULL;
    }


    *EntryPoint = buf[0];
    return STATUS_SUCCESS;
}
