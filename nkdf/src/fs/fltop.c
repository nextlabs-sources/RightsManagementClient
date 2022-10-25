

#include <ntifs.h>

#include <nkdf/basic.h>
#include <nkdf/fs/fltop.h>




//  Assign text sections for each routine.
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NkFltCreateFile)
#pragma alloc_text(PAGE, NkFltOpenFile)
#pragma alloc_text(PAGE, NkFltCloseFile)
#pragma alloc_text(PAGE, NkFltDeleteFile)
#pragma alloc_text(PAGE, NkFltRenameFile)
#pragma alloc_text(PAGE, NkFltMoveFile)
#pragma alloc_text(PAGE, NkFltIsFileDirectory)
#pragma alloc_text(PAGE, NkFltIsFileExisting)
#pragma alloc_text(PAGE, NkFltGetFileAttributes)
#pragma alloc_text(PAGE, NkFltGetFileTime)
#pragma alloc_text(PAGE, NkFltGetFileSize)
#pragma alloc_text(PAGE, NkFltGetFilePointer)
#pragma alloc_text(PAGE, NkFltSetFileAttributes)
#pragma alloc_text(PAGE, NkFltSetFileTime)
#pragma alloc_text(PAGE, NkFltSetFileSize)
#pragma alloc_text(PAGE, NkFltSetFilePointer)
#pragma alloc_text(PAGE, NkFltZeroFileContent)
#pragma alloc_text(PAGE, NkFltSyncReadFile)
#pragma alloc_text(PAGE, NkFltSyncWriteFile)
#pragma alloc_text(PAGE, NkFltSyncRenameFile)
#pragma alloc_text(PAGE, NkFltSyncMoveFile)
#pragma alloc_text(PAGE, NkFltSyncGetFileAttributes)
#pragma alloc_text(PAGE, NkFltSyncGetFileTime)
#pragma alloc_text(PAGE, NkFltSyncGetFileSize)
#pragma alloc_text(PAGE, NkFltSyncGetFilePointer)
#pragma alloc_text(PAGE, NkFltSyncSetFileAttributes)
#pragma alloc_text(PAGE, NkFltSyncSetFileTime)
#pragma alloc_text(PAGE, NkFltSyncSetFileSize)
#pragma alloc_text(PAGE, NkFltSyncSetFilePointer)
#pragma alloc_text(PAGE, NkFltSyncZeroFileContent)
#pragma alloc_text(PAGE, NkFltSyncQueryDirectoryFile)
#endif



_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltCreateFile(
                _In_ PFLT_FILTER Filter,
                _In_opt_ PFLT_INSTANCE Instance,
                _Out_ PHANDLE FileHandle,
                _In_ ACCESS_MASK DesiredAccess,
                _In_ PCUNICODE_STRING FileName,
                _Out_opt_ PULONG Result,
                _In_ ULONG FileAttributes,
                _In_ ULONG ShareAccess,
                _In_ ULONG CreateDisposition,
                _In_ ULONG CreateOptions,
                _In_ ULONG Flags
                )
{
    NTSTATUS            Status;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;


    PAGED_CODE();


    if (0 == FileName->Length || NULL == FileName->Buffer) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Create file
    //
    InitializeObjectAttributes(&ObjectAttributes, (PUNICODE_STRING)FileName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);
    Status = FltCreateFile(Filter,
                           Instance,
                           FileHandle,
                           DesiredAccess,
                           &ObjectAttributes,
                           &IoStatusBlock,
                           NULL,
                           FileAttributes,
                           ShareAccess,
                           CreateDisposition,
                           CreateOptions,
                           NULL,
                           0,
                           Flags
                           );
    if (NT_SUCCESS(Status) && NULL != Result) {
        *Result = (ULONG)IoStatusBlock.Information;
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltOpenFile(
              _In_ PFLT_FILTER Filter,
              _In_opt_ PFLT_INSTANCE Instance,
              _Out_ PHANDLE FileHandle,
              _In_ ACCESS_MASK DesiredAccess,
              _In_ PCUNICODE_STRING FileName,
              _Out_opt_ PULONG Result,
              _In_ ULONG ShareAccess,
              _In_ ULONG OpenOptions,
              _In_ ULONG Flags
              )
{
    PAGED_CODE();
    return NkFltCreateFile(Filter,
                           Instance,
                           FileHandle,
                           DesiredAccess,
                           FileName,
                           Result,
                           FILE_ATTRIBUTE_NORMAL,
                           ShareAccess,
                           FILE_OPEN,
                           OpenOptions,
                           Flags);
}


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltOpenFileById(
                  _In_ PFLT_FILTER Filter,
                  _In_opt_ PFLT_INSTANCE Instance,
                  _Out_ PHANDLE FileHandle,
                  _In_ ACCESS_MASK DesiredAccess,
                  _In_ PUNICODE_STRING VolumeName,
                  _In_ LARGE_INTEGER FileId,
                  _Out_opt_ PULONG Result,
                  _In_ ULONG ShareAccess,
                  _In_ ULONG OpenOptions,
                  _In_ ULONG Flags
                  )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES   OaFile;
    IO_STATUS_BLOCK     IoStatusBlock;
    UNICODE_STRING      StrFileId = { 8, 8, (PWSTR)(&FileId) };
    OBJECT_ATTRIBUTES   OaRoot;
    UNICODE_STRING      StrRoot = { 0, 0, NULL };
    HANDLE              RootHandle = NULL;
    const UNICODE_STRING StrSlash = { 2, 2, L"\\" };

    PAGED_CODE();

    *FileHandle = NULL;
    if (NULL != Result) {
        *Result = 0;
    }

    try {

        StrRoot.MaximumLength = VolumeName->Length + 2 * sizeof(WCHAR);
        StrRoot.Length = 0;
        StrRoot.Buffer = ExAllocatePoolWithTag(PagedPool, StrRoot.MaximumLength, TAG_TEMP);
        if (NULL == StrRoot.Buffer) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlCopyUnicodeString(&StrRoot, VolumeName);
        (VOID)RtlAppendUnicodeStringToString(&StrRoot, &StrSlash);

        InitializeObjectAttributes(&OaRoot, &StrRoot, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);
        Status = FltCreateFile(Filter,
                               Instance,
                               &RootHandle,
                               GENERIC_READ,
                               &OaRoot,
                               &IoStatusBlock,
                               NULL,
                               FILE_ATTRIBUTE_NORMAL,
                               FILE_SHARE_VALID_FLAGS,
                               FILE_OPEN,
                               FILE_DIRECTORY_FILE,
                               NULL,
                               0,
                               Flags
                               );
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }
    
        InitializeObjectAttributes(&OaFile, &StrFileId, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, RootHandle, NULL);
        Status = FltCreateFile(Filter,
                               Instance,
                               FileHandle,
                               DesiredAccess,
                               &OaFile,
                               &IoStatusBlock,
                               NULL,
                               FILE_ATTRIBUTE_NORMAL,
                               ShareAccess,
                               FILE_OPEN,
                               OpenOptions | FILE_OPEN_BY_FILE_ID,
                               NULL,
                               0,
                               Flags
                               );
        if (NT_SUCCESS(Status) && NULL != Result) {
            *Result = (ULONG)IoStatusBlock.Information;
        }

    try_exit: NOTHING;
    }
    finally {
        if (NULL != StrRoot.Buffer) {
            ExFreePool(StrRoot.Buffer);
            StrRoot.Buffer = NULL;
            StrRoot.Length = 0;
            StrRoot.MaximumLength = 0;
        }
        if (NULL != RootHandle) {
            FltClose(RootHandle);
            RootHandle = NULL;
        }
    }

    return Status;
}

_IRQL_requires_(PASSIVE_LEVEL)
VOID
NkFltCloseFile(
               _In_ HANDLE FileHandle
               )
{
    FltClose(FileHandle);
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltDeleteFile(
                _In_ PFLT_FILTER Filter,
                _In_opt_ PFLT_INSTANCE Instance,
                _In_ PCUNICODE_STRING FileName
                )
{
    NTSTATUS Status;
    HANDLE   FileHandle;

    PAGED_CODE();

    Status = NkFltOpenFile(Filter, Instance, &FileHandle, DELETE, FileName, NULL, 0, FILE_DELETE_ON_CLOSE, 0);
    if (NT_SUCCESS(Status)) {
        FltClose(FileHandle);
        FileHandle = NULL;
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltRenameFile(
                _In_ PFLT_INSTANCE Instance,
                _In_ PFILE_OBJECT FileObject,
                _In_ PCUNICODE_STRING NewFileName,
                _In_ BOOLEAN ReplaceIfExists
                )
{
    NTSTATUS            Status;
    FILE_RENAME_INFORMATION* RenameInfo = NULL;
	USHORT i = 0;

    PAGED_CODE();

    // NewFileName is valid
    if (NULL == NewFileName || 0 == NewFileName->Length) {
        return STATUS_INVALID_PARAMETER;
    }
    // NewFileName is NOT path
    for (i = 0; i < NewFileName->Length / 2; i++) {
        if (L'\\' == NewFileName->Buffer[i]) {
            return STATUS_INVALID_PARAMETER;
        }
    }

    RenameInfo = ExAllocatePoolWithTag(NonPagedPool, sizeof(FILE_RENAME_INFORMATION) + NewFileName->Length, TAG_TEMP);
    if (NULL == RenameInfo) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(RenameInfo, sizeof(FILE_RENAME_INFORMATION) + NewFileName->Length);
    RenameInfo->ReplaceIfExists = ReplaceIfExists;
    RenameInfo->RootDirectory = NULL;
    RenameInfo->FileNameLength = NewFileName->Length;
    RtlCopyMemory(RenameInfo->FileName, NewFileName->Buffer, NewFileName->Length);
    Status = FltSetInformationFile(Instance, FileObject, RenameInfo, sizeof(FILE_RENAME_INFORMATION), FileRenameInformation);
    ExFreePool(RenameInfo);
    RenameInfo = NULL;
    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltMoveFile(
              _In_ PFLT_INSTANCE Instance,
              _In_ PFILE_OBJECT FileObject,
              _In_ PCUNICODE_STRING NewFilePath,
              _In_ BOOLEAN ReplaceIfExists
              )
{
    NTSTATUS            Status;
    FILE_RENAME_INFORMATION* RenameInfo = NULL;

    PAGED_CODE();

    // NewFileName is valid
    if (NULL == NewFilePath || 0 == NewFilePath->Length) {
        return STATUS_INVALID_PARAMETER;
    }

    RenameInfo = ExAllocatePoolWithTag(NonPagedPool, sizeof(FILE_RENAME_INFORMATION) + NewFilePath->Length, TAG_TEMP);
    if (NULL == RenameInfo) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(RenameInfo, sizeof(FILE_RENAME_INFORMATION) + NewFilePath->Length);
    RenameInfo->ReplaceIfExists = ReplaceIfExists;
    RenameInfo->RootDirectory = NULL;
    RenameInfo->FileNameLength = NewFilePath->Length;
    RtlCopyMemory(RenameInfo->FileName, NewFilePath->Buffer, NewFilePath->Length);
    Status = FltSetInformationFile(Instance, FileObject, RenameInfo, sizeof(FILE_RENAME_INFORMATION), FileRenameInformation);
    ExFreePool(RenameInfo);
    RenameInfo = NULL;
    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltIsFileDirectory(
                     _In_ PFLT_INSTANCE Instance,
                     _In_ PFILE_OBJECT FileObject,
                     _Out_ PBOOLEAN Directory
                     )
{
    NTSTATUS            Status;
    FILE_STANDARD_INFORMATION   StdInfo = { 0 };

    PAGED_CODE();

    *Directory = FALSE;
    Status = FltQueryInformationFile(Instance, FileObject, &StdInfo, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation, NULL);
    if (NT_SUCCESS(Status)) {
        *Directory = StdInfo.Directory;
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
BOOLEAN
NkFltIsFileExisting(
                    _In_ PFLT_FILTER Filter,
                    _In_opt_ PFLT_INSTANCE Instance,
                    _In_ PCUNICODE_STRING FileName
                    )
{
    NTSTATUS    Status;
    HANDLE      FileHandle = NULL;

    PAGED_CODE();

    Status = NkFltOpenFile(Filter,
                           Instance,
                           &FileHandle,
                           STANDARD_RIGHTS_READ | FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                           FileName,
                           NULL,
                           FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                           FILE_COMPLETE_IF_OPLOCKED,
                           IO_IGNORE_SHARE_ACCESS_CHECK);
    if (NT_SUCCESS(Status)) {
        ZwClose(FileHandle);
        FileHandle = NULL;
    }
    return ((STATUS_OBJECT_NAME_INVALID != Status)
            && (STATUS_OBJECT_NAME_NOT_FOUND != Status)
            && (STATUS_OBJECT_PATH_INVALID != Status)
            && (STATUS_OBJECT_PATH_NOT_FOUND != Status)
            );
}


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltGetFileAttributes(
                       _In_ PFLT_INSTANCE Instance,
                       _In_ PFILE_OBJECT FileObject,
                       _Out_ PULONG FileAttributes
                       )
{
    NTSTATUS            Status;
    FILE_BASIC_INFORMATION   BasicInfo = { 0 };

    PAGED_CODE();

    *FileAttributes = 0xFFFFFFFF;
    Status = FltQueryInformationFile(Instance, FileObject, &BasicInfo, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation, NULL);
    if (NT_SUCCESS(Status)) {
        *FileAttributes = BasicInfo.FileAttributes;
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltGetFileTime(
                 _In_ PFLT_INSTANCE Instance,
                 _In_ PFILE_OBJECT FileObject,
                 _Out_opt_ PLARGE_INTEGER CreationTime,
                 _Out_opt_ PLARGE_INTEGER LastAccessTime,
                 _Out_opt_ PLARGE_INTEGER LastWriteTime,
                 _Out_opt_ PLARGE_INTEGER ChangeTime
                 )
{
    NTSTATUS            Status;
    FILE_BASIC_INFORMATION   BasicInfo = { 0 };

    PAGED_CODE();

    Status = FltQueryInformationFile(Instance, FileObject, &BasicInfo, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation, NULL);
    if (NULL != CreationTime) {
        CreationTime->QuadPart = NT_SUCCESS(Status) ? BasicInfo.CreationTime.QuadPart : -1;
    }
    if (NULL != LastAccessTime) {
        LastAccessTime->QuadPart = NT_SUCCESS(Status) ? BasicInfo.LastAccessTime.QuadPart : -1;
    }
    if (NULL != LastWriteTime) {
        LastWriteTime->QuadPart = NT_SUCCESS(Status) ? BasicInfo.LastWriteTime.QuadPart : -1;
    }
    if (NULL != ChangeTime) {
        ChangeTime->QuadPart = NT_SUCCESS(Status) ? BasicInfo.ChangeTime.QuadPart : -1;
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltGetFileSize(
                 _In_ PFLT_INSTANCE Instance,
                 _In_ PFILE_OBJECT FileObject,
                 _Out_ PLARGE_INTEGER FileSize
                 )
{
    NTSTATUS            Status;
    FILE_STANDARD_INFORMATION   StdInfo = { 0 };

    PAGED_CODE();

    FileSize->QuadPart = 0;
    Status = FltQueryInformationFile(Instance, FileObject, &StdInfo, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation, NULL);
    if (NT_SUCCESS(Status)) {
        FileSize->QuadPart = StdInfo.EndOfFile.QuadPart;
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltGetFilePointer(
                    _In_ PFLT_INSTANCE Instance,
                    _In_ PFILE_OBJECT FileObject,
                    _Out_ PLARGE_INTEGER CurrentByteOffset
                    )
{
    NTSTATUS            Status;
    FILE_POSITION_INFORMATION   PosInfo = { 0 };

    PAGED_CODE();

    CurrentByteOffset->QuadPart = 0;
    Status = FltQueryInformationFile(Instance, FileObject, &PosInfo, sizeof(FILE_POSITION_INFORMATION), FilePositionInformation, NULL);
    if (NT_SUCCESS(Status)) {
        CurrentByteOffset->QuadPart = PosInfo.CurrentByteOffset.QuadPart;
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltSetFileAttributes(
                       _In_ PFLT_INSTANCE Instance,
                       _In_ PFILE_OBJECT FileObject,
                       _In_ ULONG FileAttributes
                       )
{
    FILE_BASIC_INFORMATION   BasicInfo = { 0 };

    PAGED_CODE();

    BasicInfo.CreationTime.QuadPart = -1;
    BasicInfo.LastAccessTime.QuadPart = -1;
    BasicInfo.LastWriteTime.QuadPart = -1;
    BasicInfo.ChangeTime.QuadPart = -1;
    BasicInfo.FileAttributes = FileAttributes;
    return FltSetInformationFile(Instance, FileObject, &BasicInfo, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltSetFileTime(
                 _In_ PFLT_INSTANCE Instance,
                 _In_ PFILE_OBJECT FileObject,
                 _In_ LONGLONG CreationTime,
                 _In_ LONGLONG LastAccessTime,
                 _In_ LONGLONG LastWriteTime,
                 _In_ LONGLONG ChangeTime
                 )
{
    FILE_BASIC_INFORMATION   BasicInfo = { 0 };

    PAGED_CODE();

    BasicInfo.CreationTime.QuadPart = CreationTime;
    BasicInfo.LastAccessTime.QuadPart = LastAccessTime;
    BasicInfo.LastWriteTime.QuadPart = LastWriteTime;
    BasicInfo.ChangeTime.QuadPart = ChangeTime;
    BasicInfo.FileAttributes = 0;
    return FltSetInformationFile(Instance, FileObject, &BasicInfo, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltSetFileSize(
                 _In_ PFLT_INSTANCE Instance,
                 _In_ PFILE_OBJECT FileObject,
                 _In_ LARGE_INTEGER FileSize
                 )
{
    FILE_END_OF_FILE_INFORMATION   EofInfo = { 0 };

    PAGED_CODE();

    EofInfo.EndOfFile.QuadPart = FileSize.QuadPart;
    return FltSetInformationFile(Instance, FileObject, &EofInfo, sizeof(FILE_END_OF_FILE_INFORMATION), FileEndOfFileInformation);
}


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltSetFilePointer(
                    _In_ PFLT_INSTANCE Instance,
                    _In_ PFILE_OBJECT FileObject,
                    _In_ LARGE_INTEGER CurrentByteOffset
                    )
{
    FILE_POSITION_INFORMATION   PosInfo = { 0 };

    PAGED_CODE();

    PosInfo.CurrentByteOffset.QuadPart = CurrentByteOffset.QuadPart;
    return FltSetInformationFile(Instance, FileObject, &PosInfo, sizeof(FILE_POSITION_INFORMATION), FilePositionInformation);
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltZeroFileContent(
                     _In_ PFLT_INSTANCE Instance,
                     _In_ PFILE_OBJECT FileObject,
                     _In_ PLARGE_INTEGER ByteOffset,
                     _In_ ULONG Length
                     )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    PUCHAR      ZeroBuffer = NULL;
    const ULONG BufferSize = 16384;  // 16K
    LARGE_INTEGER FileSize = { 0, 0 };
    LARGE_INTEGER CurrentOffset = { 0, 0 };


    PAGED_CODE();

    try {

        ZeroBuffer = ExAllocatePoolWithTag(PagedPool, BufferSize, TAG_TEMP);
        if (NULL == ZeroBuffer) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory(ZeroBuffer, BufferSize);

        Status = NkFltSyncGetFileSize(Instance, FileObject, &FileSize);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        if (FileSize.QuadPart <= ByteOffset->QuadPart) {
            try_return(Status = STATUS_SUCCESS);
        }

        if ((ByteOffset->QuadPart + Length) > FileSize.QuadPart) {
            Length = (ULONG)(FileSize.QuadPart - ByteOffset->QuadPart);
        }

        CurrentOffset.QuadPart = ByteOffset->QuadPart;
        while (0 != Length) {

            ULONG BytesToWrite = min(Length, BufferSize);
            ULONG BytesWritten;

            Status = FltWriteFile(Instance, FileObject, &CurrentOffset, BytesToWrite, ZeroBuffer, 0, &BytesWritten, NULL, NULL);
            if (!NT_SUCCESS(Status)) {
                try_return(Status);
            }

            Length -= BytesWritten;
            CurrentOffset.QuadPart += BytesWritten;
        }

try_exit: NOTHING;
    }
    finally {
        if (NULL != ZeroBuffer) {
            ExFreePool(ZeroBuffer);
        }
        ZeroBuffer = NULL;
    }

    return Status;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncReadFile(
                  _In_ PFLT_INSTANCE Instance,
                  _In_ PFILE_OBJECT FileObject,
                  _In_opt_ PLARGE_INTEGER ByteOffset,
                  _In_ ULONG Length,
                  _Out_ PVOID Buffer,
                  _In_ FLT_IO_OPERATION_FLAGS Flags,
                  _Out_opt_ PULONG BytesRead
                  )
{
    NTSTATUS                Status = STATUS_UNSUCCESSFUL;
    PFLT_CALLBACK_DATA      CallbackData = NULL;
    LARGE_INTEGER           OldByteOffset;


    PAGED_CODE();


    if (NULL != BytesRead) {
        *BytesRead = 0;
    }

    try {

        Status = FltAllocateCallbackData(Instance, FileObject, &CallbackData);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        OldByteOffset = FileObject->CurrentByteOffset;
        CallbackData->Iopb->MajorFunction = IRP_MJ_READ;
        CallbackData->Iopb->Parameters.Read.ByteOffset = (NULL == ByteOffset) ? FileObject->CurrentByteOffset : (*ByteOffset);
        CallbackData->Iopb->Parameters.Read.Length = Length;
        CallbackData->Iopb->Parameters.Read.ReadBuffer = Buffer;

        CallbackData->Iopb->IrpFlags = 0;
        SetFlag(CallbackData->Iopb->IrpFlags, IRP_READ_OPERATION);
        if (BooleanFlagOn(Flags, FLTFL_IO_OPERATION_NON_CACHED)) {
            SetFlag(CallbackData->Iopb->IrpFlags, IRP_NOCACHE);
        }
        if (BooleanFlagOn(Flags, FLTFL_IO_OPERATION_PAGING)) {
            SetFlag(CallbackData->Iopb->IrpFlags, IRP_PAGING_IO);
        }

        FltPerformSynchronousIo(CallbackData);
        Status = CallbackData->IoStatus.Status;
        if (NULL != BytesRead) {
            *BytesRead = (ULONG)CallbackData->IoStatus.Information;
        }
        if (NT_SUCCESS(Status)) {
            if (BooleanFlagOn(Flags, FLTFL_IO_OPERATION_DO_NOT_UPDATE_BYTE_OFFSET)) {
                FileObject->CurrentByteOffset = OldByteOffset;
            }
        }

try_exit: NOTHING;
    }
    finally {

        if (NULL != CallbackData) {
            FltFreeCallbackData(CallbackData);
            CallbackData = NULL;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncWriteFile(
                   _In_ PFLT_INSTANCE Instance,
                   _In_ PFILE_OBJECT FileObject,
                   _In_opt_ PLARGE_INTEGER ByteOffset,
                   _In_ ULONG Length,
                   _In_ PVOID Buffer,
                   _In_ FLT_IO_OPERATION_FLAGS Flags,
                   _Out_opt_ PULONG BytesWritten
                   )
{
    NTSTATUS                Status = STATUS_UNSUCCESSFUL;
    PFLT_CALLBACK_DATA      CallbackData = NULL;
    LARGE_INTEGER           OldByteOffset;


    PAGED_CODE();


    if (NULL != BytesWritten) {
        *BytesWritten = 0;
    }

    try {

        Status = FltAllocateCallbackData(Instance, FileObject, &CallbackData);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        OldByteOffset = FileObject->CurrentByteOffset;
        CallbackData->Iopb->MajorFunction = IRP_MJ_WRITE;
        CallbackData->Iopb->Parameters.Write.ByteOffset = (NULL == ByteOffset) ? FileObject->CurrentByteOffset : (*ByteOffset);
        CallbackData->Iopb->Parameters.Write.Length = Length;
        CallbackData->Iopb->Parameters.Write.WriteBuffer = Buffer;

        CallbackData->Iopb->IrpFlags = 0;
        SetFlag(CallbackData->Iopb->IrpFlags, IRP_WRITE_OPERATION);
        if (BooleanFlagOn(Flags, FLTFL_IO_OPERATION_NON_CACHED)) {
            SetFlag(CallbackData->Iopb->IrpFlags, IRP_NOCACHE);
        }
        if (BooleanFlagOn(Flags, FLTFL_IO_OPERATION_PAGING)) {
            SetFlag(CallbackData->Iopb->IrpFlags, IRP_PAGING_IO);
        }

        FltPerformSynchronousIo(CallbackData);
        Status = CallbackData->IoStatus.Status;
        if (NULL != BytesWritten) {
            *BytesWritten = (ULONG)CallbackData->IoStatus.Information;
        }
        if (NT_SUCCESS(Status)) {
            if (BooleanFlagOn(Flags, FLTFL_IO_OPERATION_DO_NOT_UPDATE_BYTE_OFFSET)) {
                FileObject->CurrentByteOffset = OldByteOffset;
            }
        }

    try_exit: NOTHING;
    }
    finally {

        if (NULL != CallbackData) {
            FltFreeCallbackData(CallbackData);
            CallbackData = NULL;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncQueryInformationFile(
                              _In_ PFLT_INSTANCE Instance,
                              _In_ PFILE_OBJECT FileObject,
                              _Out_ PVOID FileInformation,
                              _In_ ULONG Length,
                              _In_ FILE_INFORMATION_CLASS FileInformationClass,
                              _Out_opt_ PULONG LengthReturned
                              )
{
    NTSTATUS            Status = STATUS_UNSUCCESSFUL;
    PFLT_CALLBACK_DATA  CallbackData = NULL;

    PAGED_CODE();

    if (NULL != LengthReturned) {
        *LengthReturned = 0;
    }

    try {

        Status = FltAllocateCallbackData(Instance, FileObject, &CallbackData);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        CallbackData->Iopb->MajorFunction = IRP_MJ_QUERY_INFORMATION;
        CallbackData->Iopb->Parameters.QueryFileInformation.FileInformationClass = FileInformationClass;
        CallbackData->Iopb->Parameters.QueryFileInformation.InfoBuffer = FileInformation;
        CallbackData->Iopb->Parameters.QueryFileInformation.Length = Length;

        FltPerformSynchronousIo(CallbackData);
        Status = CallbackData->IoStatus.Status;
        if (NULL != LengthReturned) {
            *LengthReturned = (ULONG)CallbackData->IoStatus.Information;
        }

    try_exit: NOTHING;
    }
    finally {

        if (NULL != CallbackData) {
            FltFreeCallbackData(CallbackData);
            CallbackData = NULL;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncSetInformationFile(
                            _In_ PFLT_INSTANCE Instance,
                            _In_ PFILE_OBJECT FileObject,
                            _In_ PVOID FileInformation,
                            _In_ ULONG Length,
                            _In_ FILE_INFORMATION_CLASS FileInformationClass
                            )
{
    NTSTATUS            Status = STATUS_UNSUCCESSFUL;
    PFLT_CALLBACK_DATA  CallbackData = NULL;

    PAGED_CODE();


    //
    //  Sanity Check
    //
    ASSERT((FileAllocationInformation == FileInformationClass ||
            FileBasicInformation == FileInformationClass ||
            FileDispositionInformation == FileInformationClass ||
            FileEndOfFileInformation == FileInformationClass ||
            FileLinkInformation == FileInformationClass ||
            FilePositionInformation == FileInformationClass ||
            FileRenameInformation == FileInformationClass ||
            FileValidDataLengthInformation == FileInformationClass));

    if (!(FileAllocationInformation == FileInformationClass ||
          FileBasicInformation == FileInformationClass ||
          FileDispositionInformation == FileInformationClass ||
          FileEndOfFileInformation == FileInformationClass ||
          FileLinkInformation == FileInformationClass ||
          FilePositionInformation == FileInformationClass ||
          FileRenameInformation == FileInformationClass ||
          FileValidDataLengthInformation == FileInformationClass)) {
        return STATUS_INVALID_PARAMETER;
    }


    try {

        Status = FltAllocateCallbackData(Instance, FileObject, &CallbackData);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        CallbackData->Iopb->MajorFunction = IRP_MJ_SET_INFORMATION;
        CallbackData->Iopb->Parameters.SetFileInformation.FileInformationClass = FileInformationClass;
        CallbackData->Iopb->Parameters.SetFileInformation.Length = Length;
        CallbackData->Iopb->Parameters.SetFileInformation.ParentOfTarget = NULL;
        CallbackData->Iopb->Parameters.SetFileInformation.DeleteHandle = NULL;
        CallbackData->Iopb->Parameters.SetFileInformation.InfoBuffer = FileInformation;

        //
        //  Parameter Check
        //
        if (FileInformationClass == FileRenameInformation) {

            CallbackData->Iopb->Parameters.SetFileInformation.ReplaceIfExists = ((PFILE_RENAME_INFORMATION)FileInformation)->ReplaceIfExists;
            CallbackData->Iopb->Parameters.SetFileInformation.ParentOfTarget = ((PFILE_RENAME_INFORMATION)FileInformation)->RootDirectory;

        }
        else if (FileInformationClass == FileLinkInformation) {

            CallbackData->Iopb->Parameters.SetFileInformation.ReplaceIfExists = ((PFILE_LINK_INFORMATION)FileInformation)->ReplaceIfExists;
            CallbackData->Iopb->Parameters.SetFileInformation.ParentOfTarget = ((PFILE_LINK_INFORMATION)FileInformation)->RootDirectory;

        }
        else if( FileInformationClass == FileMoveClusterInformation ) {
            CallbackData->Iopb->Parameters.SetFileInformation.ClusterCount = ((PFILE_MOVE_CLUSTER_INFORMATION)FileInformation)->ClusterCount;
        }
        else {
            ; // Nothing need to do
        }

        FltPerformSynchronousIo(CallbackData);
        Status = CallbackData->IoStatus.Status;

try_exit: NOTHING;
    }
    finally {

        if (NULL != CallbackData) {
            FltFreeCallbackData(CallbackData);
            CallbackData = NULL;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncRenameFile(
                    _In_ PFLT_INSTANCE Instance,
                    _In_ PFILE_OBJECT FileObject,
                    _In_ PCUNICODE_STRING NewFileName,
                    _In_ BOOLEAN ReplaceIfExists
                    )
{
    NTSTATUS            Status;
    FILE_RENAME_INFORMATION* RenameInfo = NULL;
	USHORT i = 0;

    PAGED_CODE();

    // NewFileName is valid
    if (NULL == NewFileName || 0 == NewFileName->Length) {
        return STATUS_INVALID_PARAMETER;
    }
    // NewFileName is NOT path
    for (i = 0; i < NewFileName->Length / 2; i++) {
        if (L'\\' == NewFileName->Buffer[i]) {
            return STATUS_INVALID_PARAMETER;
        }
    }

    RenameInfo = ExAllocatePoolWithTag(NonPagedPool, sizeof(FILE_RENAME_INFORMATION) + NewFileName->Length, TAG_TEMP);
    if (NULL == RenameInfo) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(RenameInfo, sizeof(FILE_RENAME_INFORMATION) + NewFileName->Length);
    RenameInfo->ReplaceIfExists = ReplaceIfExists;
    RenameInfo->RootDirectory = NULL;
    RenameInfo->FileNameLength = NewFileName->Length;
    RtlCopyMemory(RenameInfo->FileName, NewFileName->Buffer, NewFileName->Length);
    Status = NkFltSyncSetInformationFile(Instance, FileObject, RenameInfo, sizeof(FILE_RENAME_INFORMATION), FileRenameInformation);
    ExFreePool(RenameInfo);
    RenameInfo = NULL;
    return Status;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncMoveFile(
                  _In_ PFLT_INSTANCE Instance,
                  _In_ PFILE_OBJECT FileObject,
                  _In_ PCUNICODE_STRING NewFilePath,
                  _In_ BOOLEAN ReplaceIfExists
                  )
{
    NTSTATUS            Status;
    FILE_RENAME_INFORMATION* RenameInfo = NULL;

    PAGED_CODE();

    // NewFileName is valid
    if (NULL == NewFilePath || 0 == NewFilePath->Length) {
        return STATUS_INVALID_PARAMETER;
    }

    RenameInfo = ExAllocatePoolWithTag(NonPagedPool, sizeof(FILE_RENAME_INFORMATION) + NewFilePath->Length, TAG_TEMP);
    if (NULL == RenameInfo) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(RenameInfo, sizeof(FILE_RENAME_INFORMATION) + NewFilePath->Length);
    RenameInfo->ReplaceIfExists = ReplaceIfExists;
    RenameInfo->RootDirectory = NULL;
    RenameInfo->FileNameLength = NewFilePath->Length;
    RtlCopyMemory(RenameInfo->FileName, NewFilePath->Buffer, NewFilePath->Length);
    Status = NkFltSyncSetInformationFile(Instance, FileObject, RenameInfo, sizeof(FILE_RENAME_INFORMATION), FileRenameInformation);
    ExFreePool(RenameInfo);
    RenameInfo = NULL;
    return Status;
}


_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncGetFileAttributes(
                           _In_ PFLT_INSTANCE Instance,
                           _In_ PFILE_OBJECT FileObject,
                           _Out_ PULONG FileAttributes
                           )
{
    NTSTATUS                Status;
    FILE_BASIC_INFORMATION  BasicInfo = { 0 };

    PAGED_CODE();

    *FileAttributes = 0xFFFFFFFF;
    Status = NkFltSyncQueryInformationFile(Instance, FileObject, &BasicInfo, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation, NULL);
    if (NT_SUCCESS(Status)) {
        *FileAttributes = BasicInfo.FileAttributes;
    }

    return Status;
}


_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncGetFileTime(
                     _In_ PFLT_INSTANCE Instance,
                     _In_ PFILE_OBJECT FileObject,
                     _Out_opt_ PLARGE_INTEGER CreationTime,
                     _Out_opt_ PLARGE_INTEGER LastAccessTime,
                     _Out_opt_ PLARGE_INTEGER LastWriteTime,
                     _Out_opt_ PLARGE_INTEGER ChangeTime
                     )
{
    NTSTATUS                Status;
    FILE_BASIC_INFORMATION  BasicInfo = { 0 };

    PAGED_CODE();

    Status = NkFltSyncQueryInformationFile(Instance, FileObject, &BasicInfo, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation, NULL);
    if (NULL != CreationTime) {
        CreationTime->QuadPart = NT_SUCCESS(Status) ? BasicInfo.CreationTime.QuadPart : -1;
    }
    if (NULL != LastAccessTime) {
        LastAccessTime->QuadPart = NT_SUCCESS(Status) ? BasicInfo.LastAccessTime.QuadPart : -1;
    }
    if (NULL != LastWriteTime) {
        LastWriteTime->QuadPart = NT_SUCCESS(Status) ? BasicInfo.LastWriteTime.QuadPart : -1;
    }
    if (NULL != ChangeTime) {
        ChangeTime->QuadPart = NT_SUCCESS(Status) ? BasicInfo.ChangeTime.QuadPart : -1;
    }

    return Status;
}


_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncGetFileSize(
                     _In_ PFLT_INSTANCE Instance,
                     _In_ PFILE_OBJECT FileObject,
                     _Out_ PLARGE_INTEGER FileSize
                     )
{
    NTSTATUS                    Status;
    FILE_STANDARD_INFORMATION   StdInfo = { 0 };

    PAGED_CODE();

    FileSize->QuadPart = 0;
    Status = NkFltSyncQueryInformationFile(Instance, FileObject, &StdInfo, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation, NULL);
    if (NT_SUCCESS(Status)) {
        FileSize->QuadPart = StdInfo.EndOfFile.QuadPart;
    }

    return Status;
}


_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncGetFilePointer(
                        _In_ PFLT_INSTANCE Instance,
                        _In_ PFILE_OBJECT FileObject,
                        _Out_ PLARGE_INTEGER CurrentByteOffset
                        )
{
    NTSTATUS                    Status;
    FILE_POSITION_INFORMATION   PosInfo = { 0 };

    PAGED_CODE();

    CurrentByteOffset->QuadPart = 0;
    Status = NkFltSyncQueryInformationFile(Instance, FileObject, &PosInfo, sizeof(FILE_POSITION_INFORMATION), FilePositionInformation, NULL);
    if (NT_SUCCESS(Status)) {
        CurrentByteOffset->QuadPart = PosInfo.CurrentByteOffset.QuadPart;
    }

    return Status;
}


_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncSetFileAttributes(
                           _In_ PFLT_INSTANCE Instance,
                           _In_ PFILE_OBJECT FileObject,
                           _In_ ULONG FileAttributes
                           )
{
    FILE_BASIC_INFORMATION   BasicInfo = { 0 };

    PAGED_CODE();

    BasicInfo.CreationTime.QuadPart = -1;
    BasicInfo.LastAccessTime.QuadPart = -1;
    BasicInfo.LastWriteTime.QuadPart = -1;
    BasicInfo.ChangeTime.QuadPart = -1;
    BasicInfo.FileAttributes = FileAttributes;
    return NkFltSyncSetInformationFile(Instance, FileObject, &BasicInfo, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);
}


_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncSetFileTime(
                     _In_ PFLT_INSTANCE Instance,
                     _In_ PFILE_OBJECT FileObject,
                     _In_ LONGLONG CreationTime,
                     _In_ LONGLONG LastAccessTime,
                     _In_ LONGLONG LastWriteTime,
                     _In_ LONGLONG ChangeTime
                     )
{
    FILE_BASIC_INFORMATION   BasicInfo = { 0 };

    PAGED_CODE();

    BasicInfo.CreationTime.QuadPart = CreationTime;
    BasicInfo.LastAccessTime.QuadPart = LastAccessTime;
    BasicInfo.LastWriteTime.QuadPart = LastWriteTime;
    BasicInfo.ChangeTime.QuadPart = ChangeTime;
    BasicInfo.FileAttributes = 0;
    return NkFltSyncSetInformationFile(Instance, FileObject, &BasicInfo, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);
}


_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncSetFileSize(
                     _In_ PFLT_INSTANCE Instance,
                     _In_ PFILE_OBJECT FileObject,
                     _In_ LARGE_INTEGER FileSize
                     )
{
    FILE_END_OF_FILE_INFORMATION   EofInfo = { 0 };

    PAGED_CODE();

    EofInfo.EndOfFile.QuadPart = FileSize.QuadPart;
    return NkFltSyncSetInformationFile(Instance, FileObject, &EofInfo, sizeof(FILE_END_OF_FILE_INFORMATION), FileEndOfFileInformation);
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncSetFilePointer(
                        _In_ PFLT_INSTANCE Instance,
                        _In_ PFILE_OBJECT FileObject,
                        _In_ LARGE_INTEGER CurrentByteOffset
                        )
{
    FILE_POSITION_INFORMATION   PosInfo = { 0 };

    PAGED_CODE();

    PosInfo.CurrentByteOffset.QuadPart = CurrentByteOffset.QuadPart;
    return NkFltSyncSetInformationFile(Instance, FileObject, &PosInfo, sizeof(FILE_POSITION_INFORMATION), FilePositionInformation);
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncZeroFileContent(
                         _In_ PFLT_INSTANCE Instance,
                         _In_ PFILE_OBJECT FileObject,
                         _In_ PLARGE_INTEGER ByteOffset,
                         _In_ ULONG Length
                         )
{
    NTSTATUS    Status      = STATUS_SUCCESS;
    PUCHAR      ZeroBuffer  = NULL;
    const ULONG BufferSize  = 16384;  // 16K
    LARGE_INTEGER FileSize = { 0, 0 };
    LARGE_INTEGER CurrentOffset = { 0, 0 };


    PAGED_CODE();

    try {

        ZeroBuffer = ExAllocatePoolWithTag(PagedPool, BufferSize, TAG_TEMP);
        if (NULL == ZeroBuffer) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory(ZeroBuffer, BufferSize);

        Status = NkFltSyncGetFileSize(Instance, FileObject, &FileSize);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        if (FileSize.QuadPart <= ByteOffset->QuadPart) {
            try_return(Status = STATUS_SUCCESS);
        }

        if ((ByteOffset->QuadPart + Length) > FileSize.QuadPart) {
            Length = (ULONG)(FileSize.QuadPart - ByteOffset->QuadPart);
        }

        CurrentOffset.QuadPart = ByteOffset->QuadPart;
        while (0 != Length) {

            ULONG BytesToWrite = min(Length, BufferSize);
            ULONG BytesWritten;

            Status = NkFltSyncWriteFile(Instance, FileObject, &CurrentOffset, BytesToWrite, ZeroBuffer, 0, &BytesWritten);
            if (!NT_SUCCESS(Status)) {
                try_return(Status);
            }

            Length -= BytesWritten;
            CurrentOffset.QuadPart += BytesWritten;
        }

try_exit: NOTHING;
    }
    finally {
        if (NULL != ZeroBuffer) {
            ExFreePool(ZeroBuffer);
        }
        ZeroBuffer = NULL;
    }

    return Status;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncQueryDirectoryFile(
                            _In_ PFLT_INSTANCE Instance,
                            _In_ PFILE_OBJECT FileObject,
                            _Out_ PVOID FileInformation,
                            _In_ ULONG Length,
                            _In_ FILE_INFORMATION_CLASS FileInformationClass,
                            _In_ BOOLEAN ReturnSingleEntry,
                            _In_opt_ PUNICODE_STRING FileName,
                            _In_ BOOLEAN RestartScan,
                            _Out_opt_ PULONG LengthReturned
                            )
{

    NTSTATUS			Status = STATUS_UNSUCCESSFUL;
    PFLT_PARAMETERS		Params;
    PFLT_CALLBACK_DATA	CallbackData = NULL;


    PAGED_CODE();

    try {

        Status = FltAllocateCallbackData(Instance, FileObject, &CallbackData);

        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }


        CallbackData->Iopb->MajorFunction = IRP_MJ_DIRECTORY_CONTROL;
        CallbackData->Iopb->MinorFunction = IRP_MN_QUERY_DIRECTORY;

        Params = &CallbackData->Iopb->Parameters;
        Params->DirectoryControl.QueryDirectory.Length = Length;
        Params->DirectoryControl.QueryDirectory.FileName = FileName;
        Params->DirectoryControl.QueryDirectory.FileInformationClass = FileInformationClass;
        Params->DirectoryControl.QueryDirectory.DirectoryBuffer = FileInformation;
        Params->DirectoryControl.QueryDirectory.MdlAddress = NULL;

        if (RestartScan)
            CallbackData->Iopb->OperationFlags |= SL_RESTART_SCAN;
        if (ReturnSingleEntry)
            CallbackData->Iopb->OperationFlags |= SL_RETURN_SINGLE_ENTRY;
        FltPerformSynchronousIo(CallbackData);
        Status = CallbackData->IoStatus.Status;
        if (LengthReturned && NT_SUCCESS(Status)) {
            *LengthReturned = (ULONG)CallbackData->IoStatus.Information;
        }

try_exit: NOTHING;
    }
    finally{

        if (NULL != CallbackData) {
            FltFreeCallbackData(CallbackData);
            CallbackData = NULL;
        }
    }

    return Status;
}