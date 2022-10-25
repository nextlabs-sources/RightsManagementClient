

#include <ntifs.h>

#include <nkdf/basic.h>
#include <nkdf/fs/fsop.h>







//  Assign text sections for each routine.
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NkIsReservedFile)
#pragma alloc_text(PAGE, NkCreateFile)
#pragma alloc_text(PAGE, NkOpenFile)
#pragma alloc_text(PAGE, NkCloseFile)
#pragma alloc_text(PAGE, NkDeleteFile)
#pragma alloc_text(PAGE, NkRenameFile)
#pragma alloc_text(PAGE, NkMoveFile)
#pragma alloc_text(PAGE, NkIsFileDirectory)
#pragma alloc_text(PAGE, NkIsFileExisting)
#pragma alloc_text(PAGE, NkGetFileAttributes)
#pragma alloc_text(PAGE, NkGetFileTime)
#pragma alloc_text(PAGE, NkGetFileSize)
#pragma alloc_text(PAGE, NkGetFilePointer)
#pragma alloc_text(PAGE, NkSetFileAttributes)
#pragma alloc_text(PAGE, NkSetFileTime)
#pragma alloc_text(PAGE, NkSetFileSize)
#pragma alloc_text(PAGE, NkSetFilePointer)
#pragma alloc_text(PAGE, NkZeroFileContent)
#pragma alloc_text(PAGE, NkCreateDirectory)
#endif



_Check_return_
_IRQL_requires_max_(APC_LEVEL)
BOOLEAN
NkIsReservedFile(
                 _In_ PCUNICODE_STRING FileName
                 )
{
#define SYSFN_MFT       L"\\$Mft"
#define SYSFN_MFTMIRR   L"\\$MftMirr"
#define SYSFN_LOGFILE   L"\\$LogFile"
#define SYSFN_VOLUME    L"\\$Volume"
#define SYSFN_ATTRDEF   L"\\$AttrDef"
#define SYSFN_BITMAP    L"\\$Bitmap"
#define SYSFN_BOOT      L"\\$Boot"
#define SYSFN_BADCLUS   L"\\$BadClus"
#define SYSFN_SECURE    L"\\$Secure"
#define SYSFN_UPCASE    L"\\$UpCase"
#define SYSFN_EXTEND    L"\\$Extend"
#define SYSFN_VOLINFO   L"\\System Volume Information"

    static UNICODE_STRING SystemNames[] = {
            { sizeof(SYSFN_MFT) - sizeof(WCHAR), sizeof(SYSFN_MFT), SYSFN_MFT },
            { sizeof(SYSFN_MFTMIRR) - sizeof(WCHAR), sizeof(SYSFN_MFTMIRR), SYSFN_MFTMIRR },
            { sizeof(SYSFN_LOGFILE) - sizeof(WCHAR), sizeof(SYSFN_LOGFILE), SYSFN_LOGFILE },
            { sizeof(SYSFN_VOLUME) - sizeof(WCHAR), sizeof(SYSFN_VOLUME), SYSFN_VOLUME },
            { sizeof(SYSFN_ATTRDEF) - sizeof(WCHAR), sizeof(SYSFN_ATTRDEF), SYSFN_ATTRDEF },
            { sizeof(SYSFN_BITMAP) - sizeof(WCHAR), sizeof(SYSFN_BITMAP), SYSFN_BITMAP },
            { sizeof(SYSFN_BOOT) - sizeof(WCHAR), sizeof(SYSFN_BOOT), SYSFN_BOOT },
            { sizeof(SYSFN_BADCLUS) - sizeof(WCHAR), sizeof(SYSFN_BADCLUS), SYSFN_BADCLUS },
            { sizeof(SYSFN_SECURE) - sizeof(WCHAR), sizeof(SYSFN_SECURE), SYSFN_SECURE },
            { sizeof(SYSFN_UPCASE) - sizeof(WCHAR), sizeof(SYSFN_UPCASE), SYSFN_UPCASE },
            { sizeof(SYSFN_EXTEND) - sizeof(WCHAR), sizeof(SYSFN_EXTEND), SYSFN_EXTEND },
            { sizeof(SYSFN_VOLINFO) - sizeof(WCHAR), sizeof(SYSFN_VOLINFO), SYSFN_VOLINFO },
            { 0, 0, NULL }
    };

    ULONG   i = 0;

    PAGED_CODE();

    do {
        if (RtlPrefixUnicodeString(&SystemNames[i], FileName, TRUE))
            return TRUE;
    } while (SystemNames[++i].Length > 0);

    return FALSE;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCreateFile(
             _Out_ PHANDLE FileHandle,
             _In_ ACCESS_MASK DesiredAccess,
             _In_ PCUNICODE_STRING FileName,
             _Out_opt_ PULONG Result,
             _In_ ULONG FileAttributes,
             _In_ ULONG ShareAccess,
             _In_ ULONG CreateDisposition,
             _In_ ULONG CreateOptions
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
    Status = ZwCreateFile(FileHandle,
                          DesiredAccess,
                          &ObjectAttributes,
                          &IoStatusBlock,
                          NULL,
                          FileAttributes,
                          ShareAccess,
                          CreateDisposition,
                          CreateOptions,
                          NULL,
                          0);
    if (NT_SUCCESS(Status) && NULL != Result) {
        *Result = (ULONG)IoStatusBlock.Information;
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkOpenFile(
           _Out_ PHANDLE FileHandle,
           _In_ ACCESS_MASK DesiredAccess,
           _In_ PCUNICODE_STRING FileName,
           _Out_opt_ PULONG Result,
           _In_ ULONG ShareAccess,
           _In_ ULONG OpenOptions
           )
{
    PAGED_CODE();
    return NkCreateFile(FileHandle, DesiredAccess, FileName, Result, FILE_ATTRIBUTE_NORMAL, ShareAccess, FILE_OPEN, OpenOptions);

}

_IRQL_requires_(PASSIVE_LEVEL)
VOID
NkCloseFile(
            _In_ HANDLE FileHandle
            )
{
    PAGED_CODE();
    ZwClose(FileHandle);
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkOpenFileById(
               _Out_ PHANDLE FileHandle,
               _In_ ACCESS_MASK DesiredAccess,
               _In_ PUNICODE_STRING VolumeName,
               _In_ LARGE_INTEGER FileId,
               _Out_opt_ PULONG Result,
               _In_ ULONG ShareAccess,
               _In_ ULONG OpenOptions
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
        Status = ZwCreateFile(FileHandle,
                              GENERIC_READ,
                              &OaRoot,
                              &IoStatusBlock,
                              NULL,
                              FILE_ATTRIBUTE_NORMAL,
                              FILE_SHARE_VALID_FLAGS,
                              FILE_OPEN,
                              FILE_DIRECTORY_FILE,
                              NULL,
                              0);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }
    
        InitializeObjectAttributes(&OaFile, &StrFileId, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, RootHandle, NULL);
        Status = ZwCreateFile(FileHandle,
                              DesiredAccess,
                              &OaFile,
                              &IoStatusBlock,
                              NULL,
                              FILE_ATTRIBUTE_NORMAL,
                              ShareAccess,
                              FILE_OPEN,
                              OpenOptions | FILE_OPEN_BY_FILE_ID,
                              NULL,
                              0);
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
            ZwClose(RootHandle);
            RootHandle = NULL;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkDeleteFile(
             _In_ PCUNICODE_STRING FileName
             )
{
    NTSTATUS Status;
    HANDLE   FileHandle;

    PAGED_CODE();
    
    Status = NkOpenFile(&FileHandle, DELETE, FileName, NULL, 0, FILE_DELETE_ON_CLOSE);
    if (NT_SUCCESS(Status)) {
        ZwClose(FileHandle);
        FileHandle = NULL;
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRenameFile(
             _In_ HANDLE FileHandle,
             _In_ PCUNICODE_STRING NewFileName,
             _In_ BOOLEAN ReplaceIfExists
             )
{
    NTSTATUS            Status;
    IO_STATUS_BLOCK     IoStatusBlock;
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
    Status = ZwSetInformationFile(FileHandle, &IoStatusBlock, RenameInfo, sizeof(FILE_RENAME_INFORMATION), FileRenameInformation);
    ExFreePool(RenameInfo);
    RenameInfo = NULL;
    return Status;
}


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkMoveFile(
           _In_ HANDLE FileHandle,
           _In_ PCUNICODE_STRING NewFilePath,
           _In_ BOOLEAN ReplaceIfExists
           )
{
    NTSTATUS            Status;
    IO_STATUS_BLOCK     IoStatusBlock;
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
    Status = ZwSetInformationFile(FileHandle, &IoStatusBlock, RenameInfo, sizeof(FILE_RENAME_INFORMATION), FileRenameInformation);
    ExFreePool(RenameInfo);
    RenameInfo = NULL;
    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkIsFileDirectory(
                  _In_ HANDLE FileHandle,
                  _Out_ PBOOLEAN Directory
                  )
{
    NTSTATUS            Status;
    IO_STATUS_BLOCK     IoStatusBlock;
    FILE_STANDARD_INFORMATION   StdInfo = { 0 };

    PAGED_CODE();

    *Directory = FALSE;
    Status = ZwQueryInformationFile(FileHandle, &IoStatusBlock, &StdInfo, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);
    if (NT_SUCCESS(Status)) {
        *Directory = StdInfo.Directory;
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
BOOLEAN
NkIsFileExisting(
                 _In_ PCUNICODE_STRING FileName
                 )
{
    NTSTATUS    Status;
    HANDLE      FileHandle = NULL;

    PAGED_CODE();

    Status = NkOpenFile(&FileHandle,
                        STANDARD_RIGHTS_READ | FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                        FileName,
                        NULL,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        FILE_COMPLETE_IF_OPLOCKED);
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
NkGetFileAttributes(
                    _In_ HANDLE FileHandle,
                    _Out_ PULONG FileAttributes
                    )
{
    NTSTATUS            Status;
    IO_STATUS_BLOCK     IoStatusBlock;
    FILE_BASIC_INFORMATION   BasicInfo = { 0 };

    PAGED_CODE();

    *FileAttributes = 0xFFFFFFFF;
    Status = ZwQueryInformationFile(FileHandle, &IoStatusBlock, &BasicInfo, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);
    if (NT_SUCCESS(Status)) {
        *FileAttributes = BasicInfo.FileAttributes;
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkGetFileTime(
              _In_ HANDLE FileHandle,
              _Out_opt_ PLARGE_INTEGER CreationTime,
              _Out_opt_ PLARGE_INTEGER LastAccessTime,
              _Out_opt_ PLARGE_INTEGER LastWriteTime,
              _Out_opt_ PLARGE_INTEGER ChangeTime
              )
{
    NTSTATUS            Status;
    IO_STATUS_BLOCK     IoStatusBlock;
    FILE_BASIC_INFORMATION   BasicInfo = { 0 };

    PAGED_CODE();

    Status = ZwQueryInformationFile(FileHandle, &IoStatusBlock, &BasicInfo, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);
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
NkGetFileSize(
              _In_ HANDLE FileHandle,
              _Out_ PLARGE_INTEGER FileSize
              )
{
    NTSTATUS            Status;
    IO_STATUS_BLOCK     IoStatusBlock;
    FILE_STANDARD_INFORMATION   StdInfo = { 0 };

    FileSize->QuadPart = 0;
    Status = ZwQueryInformationFile(FileHandle, &IoStatusBlock, &StdInfo, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);
    if (NT_SUCCESS(Status)) {
        FileSize->QuadPart = StdInfo.EndOfFile.QuadPart;
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkGetFilePointer(
                 _In_ HANDLE FileHandle,
                 _Out_ PLARGE_INTEGER CurrentByteOffset
                 )
{
    NTSTATUS            Status;
    IO_STATUS_BLOCK     IoStatusBlock;
    FILE_POSITION_INFORMATION   PosInfo = { 0 };

    CurrentByteOffset->QuadPart = 0;
    Status = ZwQueryInformationFile(FileHandle, &IoStatusBlock, &PosInfo, sizeof(FILE_POSITION_INFORMATION), FilePositionInformation);
    if (NT_SUCCESS(Status)) {
        CurrentByteOffset->QuadPart = PosInfo.CurrentByteOffset.QuadPart;
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkSetFileAttributes(
                    _In_ HANDLE FileHandle,
                    _In_ ULONG FileAttributes
                    )
{
    IO_STATUS_BLOCK     IoStatusBlock;
    FILE_BASIC_INFORMATION   BasicInfo = { 0 };

    PAGED_CODE();

    BasicInfo.CreationTime.QuadPart = -1;
    BasicInfo.LastAccessTime.QuadPart = -1;
    BasicInfo.LastWriteTime.QuadPart = -1;
    BasicInfo.ChangeTime.QuadPart = -1;
    BasicInfo.FileAttributes = FileAttributes;
    return ZwSetInformationFile(FileHandle, &IoStatusBlock, &BasicInfo, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkSetFileTime(
              _In_ HANDLE FileHandle,
              _In_ LONGLONG CreationTime,
              _In_ LONGLONG LastAccessTime,
              _In_ LONGLONG LastWriteTime,
              _In_ LONGLONG ChangeTime
              )
{
    IO_STATUS_BLOCK     IoStatusBlock;
    FILE_BASIC_INFORMATION   BasicInfo = { 0 };

    PAGED_CODE();

    BasicInfo.CreationTime.QuadPart = CreationTime;
    BasicInfo.LastAccessTime.QuadPart = LastAccessTime;
    BasicInfo.LastWriteTime.QuadPart = LastWriteTime;
    BasicInfo.ChangeTime.QuadPart = ChangeTime;
    BasicInfo.FileAttributes = 0;
    return ZwSetInformationFile(FileHandle, &IoStatusBlock, &BasicInfo, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkSetFileSize(
              _In_ HANDLE FileHandle,
              _In_ LARGE_INTEGER FileSize
              )
{
    IO_STATUS_BLOCK     IoStatusBlock;
    FILE_END_OF_FILE_INFORMATION   EofInfo = { 0 };

    PAGED_CODE();

    EofInfo.EndOfFile.QuadPart = FileSize.QuadPart;
    return ZwQueryInformationFile(FileHandle, &IoStatusBlock, &EofInfo, sizeof(FILE_END_OF_FILE_INFORMATION), FileEndOfFileInformation);
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkSetFilePointer(
                 _In_ HANDLE FileHandle,
                 _In_ LARGE_INTEGER CurrentByteOffset
                 )
{
    IO_STATUS_BLOCK     IoStatusBlock;
    FILE_POSITION_INFORMATION   PosInfo = { 0 };

    PAGED_CODE();

    PosInfo.CurrentByteOffset.QuadPart = CurrentByteOffset.QuadPart;
    return ZwSetInformationFile(FileHandle, &IoStatusBlock, &PosInfo, sizeof(FILE_POSITION_INFORMATION), FilePositionInformation);
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkZeroFileContent(
                  _In_ HANDLE FileHandle,
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

        IO_STATUS_BLOCK IoStatusBlock;

        ZeroBuffer = ExAllocatePoolWithTag(PagedPool, BufferSize, TAG_TEMP);
        if (NULL == ZeroBuffer) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory(ZeroBuffer, BufferSize);

        Status = NkGetFileSize(FileHandle, &FileSize);
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

            Status = ZwWriteFile(FileHandle,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &IoStatusBlock,
                                 ZeroBuffer,
                                 BytesToWrite,
                                 &CurrentOffset,
                                 NULL);
            if (!NT_SUCCESS(Status)) {
                try_return(Status);
            }

            Length -= (ULONG)IoStatusBlock.Information;
            CurrentOffset.QuadPart += (ULONG)IoStatusBlock.Information;
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
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCreateDirectory(
                  _In_ PCUNICODE_STRING DirectoryName
                  )
{
    NTSTATUS    Status;
    HANDLE      FileHandle;
    ULONG       Result;

    PAGED_CODE();

    Status = NkCreateFile(&FileHandle,
                          GENERIC_READ|GENERIC_WRITE,
                          DirectoryName,
                          &Result,
                          FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_READ|FILE_SHARE_WRITE,
                          FILE_OPEN_IF,
                          FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_WRITE_THROUGH);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    ZwClose(FileHandle);
    return STATUS_SUCCESS;
}


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCreateDirectoryEx(
                    _In_ PCUNICODE_STRING VolumeName,
                    _In_ PCUNICODE_STRING DirectoryName
                    )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    UNICODE_STRING  Path = { 0, 0, NULL };

    PAGED_CODE();

    try {

        Path.MaximumLength = VolumeName->Length + DirectoryName->Length + sizeof(WCHAR);
        Path.Buffer = ExAllocatePoolWithTag(PagedPool, Path.MaximumLength, TAG_TEMP);
        if (NULL == Path.Buffer) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory(Path.Buffer, Path.MaximumLength);
        RtlCopyUnicodeString(&Path, VolumeName);
        (VOID)RtlAppendUnicodeStringToString(&Path, DirectoryName);
        Status = NkCreateDirectory(&Path);

try_exit: NOTHING;
    }
    finally {
        if (Path.Buffer) {
            ExFreePool(Path.Buffer);
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCreateDirectorySafe(
                      _In_ PCUNICODE_STRING DirectoryName
                      )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    UNICODE_STRING  Path = { 0, 0, NULL };
    UNICODE_STRING  DeviceName = { 0, 0, NULL };
    UNICODE_STRING  GlobalDosName = { 0, 0, NULL };

    UNICODE_STRING  RemainPart = { 0, 0, NULL };
    UNICODE_STRING  FirstPart = { 0, 0, NULL };

    PAGED_CODE();

    RtlInitUnicodeString(&DeviceName, L"\\Device\\");
    RtlInitUnicodeString(&GlobalDosName, L"\\??\\");
    RtlCopyMemory(&Path, DirectoryName, sizeof(UNICODE_STRING));

    if (DirectoryName->Length <= 14) {
        return STATUS_INVALID_PARAMETER;
    }

    RtlCopyMemory(&RemainPart, DirectoryName, sizeof(UNICODE_STRING));
    if (NkStartsWithUnicodeString(DirectoryName, &DeviceName, TRUE)) {
        // Remove L"\\Device\\"
        RemainPart.Length -= 8 * sizeof(WCHAR);
        RemainPart.Buffer += 8;
        RemainPart.MaximumLength = RemainPart.Length;
        // Remove Volume Name
        NkSplitUnicodeString(RemainPart, L'\\', &FirstPart, &RemainPart);
    }
    else if (NkIsGlobalDosPathW(DirectoryName->Buffer)){
        // Remove L"\\??\\C:\\"
        RemainPart.Length -= 7 * sizeof(WCHAR);
        RemainPart.Buffer += 7;
        RemainPart.MaximumLength = RemainPart.Length;
        Path.Length = 16;
    }
    else {
        // Not device path, not global dos path, not support
        return STATUS_INVALID_PARAMETER;
    }

    RtlCopyMemory(&Path, DirectoryName, sizeof(UNICODE_STRING));
    while (0 != RemainPart.Length) {

        NkSplitUnicodeString(RemainPart, L'\\', &FirstPart, &RemainPart);
        Path.Length = DirectoryName->Length - RemainPart.Length;
        if (L'\\' == Path.Buffer[Path.Length / 2 - 1]) {
            Path.Length -= 2;
        }
        if (0 == Path.Length) {
            return STATUS_INVALID_PARAMETER;
        }

        Status = NkCreateDirectory(&Path);
        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

    return Status;
}