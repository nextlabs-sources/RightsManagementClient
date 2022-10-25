

#include <ntifs.h>
#include <Ntstrsafe.h>


#include <nkdf/basic.h>
#include <nkdf/fs.h>
#include <nkdf/log/logdef.h>
#include <nkdf/log/filelog.h>





typedef struct _LOG_ITEM {
    LIST_ENTRY      Link;
    LOGLEVEL        Level;
    LARGE_INTEGER   Timestamp;
    ULONG           ProcessId;
    ULONG           ThreadId;
    CHAR            Info[MAX_LOG_MSG_LENGTH];
} LOG_ITEM, *PLOG_ITEM;



typedef struct _LOGCB {
    BOOLEAN         Active;
    HANDLE          FileHandle;
    HANDLE          ThreadHandle;
    HANDLE          ThreadId;
    KEVENT          LogEvents[2];   // 0: NewItem Event
                                    // 1: Closing Event
    LOGLEVEL        AcceptedLevel;
    ULONG           RotateCount;
    ULONG           QueueSizeLimit; // The maximum queue size
    ULONG           FileSizeLimit;  // in MB
    WCHAR           Directory[260];
    WCHAR           Name[64];
    LIST_ENTRY      MsgList;
    ULONG           MsgListCount;
    FAST_MUTEX      MsgListLock;
} LOGCB;


typedef struct _NXLOGTHREADPARAM {
    KEVENT       Event;
    NTSTATUS     Status;
} NXLOGTHREADPARAM, *PNXLOGTHREADPARAM;



static LOGCB    lcb = { FALSE };


static const WCHAR LogIndex[10] = { L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', L'8', L'9' };


_IRQL_requires_(PASSIVE_LEVEL)
VOID
WriteLineEnd(
             );

_IRQL_requires_(PASSIVE_LEVEL)
VOID
WriteLogItem(
             _In_ const LOG_ITEM* Item
             );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkOpenLogFile(
              );

_IRQL_requires_(PASSIVE_LEVEL)
VOID
NkCloseLogFile(
               );


KSTART_ROUTINE  NkLogThread;
_IRQL_requires_same_
_Function_class_(KSTART_ROUTINE)
VOID
NkLogThread (
             _In_ PVOID Context
             );


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkLogCreateThread(
                  );

_IRQL_requires_max_(APC_LEVEL)
VOID
NkPushItem(
           _In_ PLOG_ITEM Item
           );

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
PLOG_ITEM
NkPopItem(
          );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkLogRotate(
            );


//  Assign text sections for each routine.
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NkLogInitialize)
#pragma alloc_text(PAGE, NkLogCleanup)
#pragma alloc_text(PAGE, NkLogGetLevel)
#pragma alloc_text(PAGE, NkLogSetLevel)
#pragma alloc_text(PAGE, NkLogGetRotate)
#pragma alloc_text(PAGE, NkLogSetRotate)
#pragma alloc_text(PAGE, NkLogGetQueueSizeLimit)
#pragma alloc_text(PAGE, NkLogSetQueueSizeLimit)
#pragma alloc_text(PAGE, NkLogGetFileSizeLimit)
#pragma alloc_text(PAGE, NkLogSetFileSizeLimit)
#pragma alloc_text(PAGE, WriteLineEnd)
#pragma alloc_text(PAGE, WriteLogItem)
#pragma alloc_text(PAGE, NkOpenLogFile)
#pragma alloc_text(PAGE, NkCloseLogFile)
#pragma alloc_text(PAGE, NkLogThread)
#pragma alloc_text(PAGE, NkLogCreateThread)
#endif






_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkLogInitialize(
                _In_ LOGLEVEL AcceptedLevel,
                _In_ ULONG RotateCount,
                _In_ ULONG QueueLimit,
                _In_ ULONG FileSizeLimit,
                _In_ PUNICODE_STRING Directory,
                _In_ PUNICODE_STRING Name
                )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    ULONG       DirectoryLength = 0;


    PAGED_CODE();

    if (lcb.Active) {
        return STATUS_ALREADY_REGISTERED;
    }

    if (Directory->Length < sizeof(WCHAR) || Directory->Length >= 260 * sizeof(WCHAR)) {
        return STATUS_INVALID_PARAMETER;
    }

    if (Name->Length < sizeof(WCHAR) || Name->Length >= 64 * sizeof(WCHAR)) {
        return STATUS_INVALID_PARAMETER;
    }

    DirectoryLength = Directory->Length;
    if (L'\\' == Directory->Buffer[(DirectoryLength / sizeof(WCHAR)) - 1]) {
        DirectoryLength -= sizeof(WCHAR);   // Remove the last L'\\'
    }

    RtlZeroMemory(&lcb, sizeof(lcb));
    lcb.AcceptedLevel   = AcceptedLevel;
    lcb.RotateCount     = (RotateCount < MAX_ROTATE_COUNT) ? RotateCount : MAX_ROTATE_COUNT;
    lcb.QueueSizeLimit  = (QueueLimit < MAX_LOG_QUEUE_SIZE) ? ((QueueLimit < MIN_LOG_QUEUE_SIZE) ? MIN_LOG_QUEUE_SIZE : QueueLimit) : MAX_LOG_QUEUE_SIZE; // The maximum queue size
    lcb.FileSizeLimit   = (FileSizeLimit < MAX_LOG_FILE_SIZE) ? ((FileSizeLimit < MIN_LOG_FILE_SIZE) ? MIN_LOG_FILE_SIZE : FileSizeLimit) : MAX_LOG_FILE_SIZE;  // in MB
    lcb.FileHandle      = NULL;
    lcb.MsgListCount    = 0;
    RtlCopyMemory(lcb.Directory, Directory->Buffer, Directory->Length);
    RtlCopyMemory(lcb.Name, Name->Buffer, Name->Length);
    InitializeListHead(&lcb.MsgList);
    ExInitializeFastMutex(&lcb.MsgListLock);
    KeInitializeEvent(&lcb.LogEvents[0], NotificationEvent, FALSE);
    KeInitializeEvent(&lcb.LogEvents[1], NotificationEvent, FALSE);

    Status = NkOpenLogFile();
    if (!NT_SUCCESS(Status)) {
        RtlZeroMemory(&lcb, sizeof(lcb));
        lcb.Active = FALSE;
        return Status;
    }

    Status = NkLogCreateThread();
    if (!NT_SUCCESS(Status)) {
        NkCloseLogFile();
        RtlZeroMemory(&lcb, sizeof(lcb));
        lcb.Active = FALSE;
        return Status;
    }

    lcb.Active = TRUE;
    return Status;
}

_IRQL_requires_(PASSIVE_LEVEL)
VOID
NkLogCleanup(
             )
{
    NTSTATUS        Status = STATUS_SUCCESS;



    PAGED_CODE();

    if (!lcb.Active) {
        return;
    }

    try {

        // Stop thread
        if (NULL != lcb.ThreadHandle) {

            PKTHREAD    KThread = NULL;

            Status = ObReferenceObjectByHandle(lcb.ThreadHandle, GENERIC_ALL, NULL, KernelMode, &KThread, NULL);
            if (NT_SUCCESS(Status)) {

                ASSERT(NULL != KThread);

#pragma prefast(suppress:28160, "Error annotation: Caution: 'Wait' argument does not provide any synchronization guarantees, only a hint to the system that the thread will immediately issue a wait operation.")
                KeSetEvent(&lcb.LogEvents[1], 0, TRUE);
                (VOID)KeWaitForSingleObject(KThread, Executive, KernelMode, FALSE, NULL);
                ObDereferenceObject((PVOID)KThread);
                KThread = NULL;
            }

            ZwClose(lcb.ThreadHandle);
            lcb.ThreadHandle = NULL;
            lcb.ThreadId = NULL;
        }

        NkCloseLogFile();

        // Check message queue
        ASSERT(0 == lcb.MsgListCount);

    }
    finally {
        
        RtlZeroMemory(&lcb, sizeof(lcb));
        lcb.Active = FALSE;
    }
}

_Check_return_
BOOLEAN
NkLogIsLogThread(
                 )
{
    return (lcb.Active && lcb.ThreadId == PsGetCurrentProcessId()) ? TRUE : FALSE;
}

LOGLEVEL
NkLogGetLevel(
              )
{
    return lcb.AcceptedLevel;
}

VOID
NkLogSetLevel(
              _In_ LOGLEVEL Level
              )
{
    lcb.AcceptedLevel = (Level < LogMax) ? Level : LogMax;
}

ULONG
NkLogGetRotate(
               )
{
    return lcb.RotateCount;
}

VOID
NkLogSetRotate(
               _In_ ULONG Count
               )
{
    lcb.RotateCount = (Count > 10) ? 10 : Count;
}

ULONG
NkLogGetQueueSizeLimit(
                       )
{
    return lcb.QueueSizeLimit;
}

VOID
NkLogSetQueueSizeLimit(
                       _In_ ULONG Limit
                       )
{
    if (Limit > MAX_LOG_QUEUE_SIZE) Limit = MAX_LOG_QUEUE_SIZE;
    if (Limit < MIN_LOG_QUEUE_SIZE) Limit = MIN_LOG_QUEUE_SIZE;
    lcb.QueueSizeLimit = Limit;
}

ULONG
NkLogGetFileSizeLimit(
                      )
{
    return lcb.FileSizeLimit;
}

VOID
NkLogSetFileSizeLimit(
                      _In_ ULONG Limit
                      )
{
    if (Limit > MAX_LOG_FILE_SIZE) Limit = MAX_LOG_FILE_SIZE;
    if (Limit < MIN_LOG_FILE_SIZE) Limit = MIN_LOG_FILE_SIZE;
    lcb.FileSizeLimit = Limit;
}

_IRQL_requires_max_(DISPATCH_LEVEL)
VOID
NkLog(
      _In_ LOGLEVEL Level,
      _In_ const CHAR* MsgFmt,
      ...
      )
{
    va_list         va;
    PLOG_ITEM       Item = NULL;


    if (!lcb.Active) {
        return;
    }

    // Since we only need to get this count, not must be very accurate
    // don't use lock here to enhance performance
    if (lcb.MsgListCount >= lcb.QueueSizeLimit) {
        // Exceeds queue size
        return;
    }

    // Allocate Message
#pragma prefast(suppress:6014, "This is not a memory leak, the item will be free in logging thread.")
    Item = ExAllocatePoolWithTag(PagedPool, sizeof(LOG_ITEM), TAG_LOG);
    if (NULL == Item) {
        return;
    }

    RtlZeroMemory(Item, sizeof(LOG_ITEM));
    Item->Level = Level;
    Item->ProcessId = (ULONG)(ULONG_PTR)PsGetCurrentProcessId();
    Item->ThreadId = (ULONG)(ULONG_PTR)PsGetCurrentThreadId();
    KeQuerySystemTime(&Item->Timestamp);
    InitializeListHead(&Item->Link);
    va_start(va, MsgFmt);
    (VOID)RtlStringCbVPrintfA(Item->Info, MAX_LOG_MSG_LENGTH - 1, MsgFmt, va);
    va_end(va);

    NkPushItem(Item);
}

_IRQL_requires_(PASSIVE_LEVEL)
VOID
WriteLineEnd(
             )
{
    static const CHAR EndFlag[2] = { '\r', '\n' };
    IO_STATUS_BLOCK Iosb;
    LARGE_INTEGER Offset = { FILE_WRITE_TO_END_OF_FILE, -1 };

    PAGED_CODE();

    if (NULL == lcb.FileHandle) {
        return;
    }

    (VOID)ZwWriteFile(lcb.FileHandle, NULL, NULL, NULL, &Iosb, (PVOID)EndFlag, 2, &Offset, NULL);
}

_IRQL_requires_(PASSIVE_LEVEL)
VOID
WriteLogItem(
             _In_ const LOG_ITEM* Item
             )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    IO_STATUS_BLOCK Iosb;
    CHAR Header[LOG_HEADER_LENGTH + 1] = { 0 };
    LARGE_INTEGER   LocalTime;
    TIME_FIELDS     TimeFields;
    ULONG           MsgLength = 0;
    LARGE_INTEGER Offset = { FILE_WRITE_TO_END_OF_FILE, -1 };


    PAGED_CODE();

    if (NULL == lcb.FileHandle) {
        return;
    }

    ExSystemTimeToLocalTime((PLARGE_INTEGER)&Item->Timestamp, &LocalTime);
    RtlTimeToTimeFields(&LocalTime, &TimeFields);
    Status = RtlStringCbPrintfA(Header,
                                LOG_HEADER_LENGTH + 1,
                                LOG_HEADER_FORMAT,
                                TimeFields.Year,
                                TimeFields.Month,
                                TimeFields.Day,
                                TimeFields.Hour,
                                TimeFields.Minute,
                                TimeFields.Second,
                                TimeFields.Milliseconds,
                                Item->ProcessId,
                                Item->ThreadId,
                                NkLogGetLevelName(Item->Level)
                                );
    Status = ZwWriteFile(lcb.FileHandle, NULL, NULL, NULL, &Iosb, Header, 49, &Offset, NULL);
    while (L'\0' != Item->Info[MsgLength]) {
        ++MsgLength;
    }
    if (0 != MsgLength) {
        Status = ZwWriteFile(lcb.FileHandle, NULL, NULL, NULL, &Iosb, (PVOID)Item->Info, MsgLength, &Offset, NULL);
    }
    WriteLineEnd();
}


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkOpenLogFile(
              )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    UNICODE_STRING  FileName = { 0, 0, NULL };
    UNICODE_STRING  Directory = { 0, 0, NULL };
    UNICODE_STRING  Name = { 0, 0, NULL };
    UNICODE_STRING  Extension = { 0, 0, NULL };
    UNICODE_STRING  Slash = { 0, 0, NULL };
    ULONG           Result = 0;


    PAGED_CODE();

    if (lcb.FileHandle != NULL) {
        ZwClose(lcb.FileHandle);
        lcb.FileHandle = NULL;
    }

    RtlInitUnicodeString(&Directory, lcb.Directory);
    RtlInitUnicodeString(&Name, lcb.Name);
    RtlInitUnicodeString(&Extension, L".log");
    RtlInitUnicodeString(&Slash, L"\\");
    if (Directory.Length < sizeof(WCHAR) || Name.Length < sizeof(WCHAR)) {
        return STATUS_INVALID_PARAMETER;
    }

    Status = NkCreateDirectorySafe(&Directory);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    FileName.MaximumLength = Directory.Length + Slash.Length + Name.Length + Extension.Length + sizeof(WCHAR);
    FileName.Length = 0;
    FileName.Buffer = ExAllocatePoolWithTag(PagedPool, FileName.MaximumLength, TAG_TEMP);
    if (NULL == FileName.Buffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(FileName.Buffer, FileName.MaximumLength);

    try {

        LARGE_INTEGER   FileSize = { 0, 0 };

        RtlAppendUnicodeStringToString(&FileName, &Directory);
        RtlAppendUnicodeStringToString(&FileName, &Slash);
        RtlAppendUnicodeStringToString(&FileName, &Name);
        RtlAppendUnicodeStringToString(&FileName, &Extension);

        Status = NkCreateFile(&lcb.FileHandle,
                              GENERIC_READ | GENERIC_WRITE | DELETE | SYNCHRONIZE,
                              &FileName,
                              &Result,
                              FILE_ATTRIBUTE_NORMAL,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              FILE_OPEN_IF,
                              FILE_NON_DIRECTORY_FILE | FILE_WRITE_THROUGH | FILE_SYNCHRONOUS_IO_NONALERT);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = NkGetFileSize(lcb.FileHandle, &FileSize);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = NkSetFilePointer(lcb.FileHandle, FileSize);

try_exit: NOTHING;
    }
    finally {

        if (NULL != FileName.Buffer) {
            ExFreePool(FileName.Buffer);
            FileName.Buffer = NULL;
            FileName.Length = 0;
            FileName.MaximumLength = 0;
        }

        if (!NT_SUCCESS(Status)) {
            if (NULL != lcb.FileHandle) {
                ZwClose(lcb.FileHandle);
                lcb.FileHandle = NULL;
            }
        }
    }

    return Status;
}

_IRQL_requires_(PASSIVE_LEVEL)
VOID
NkCloseLogFile(
               )
{
    PAGED_CODE();
    if (NULL != lcb.FileHandle) {
        ZwClose(lcb.FileHandle);
        lcb.FileHandle = NULL;
    }
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkLogCreateThread(
                  )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    HANDLE      ThreadHandle = NULL;
    NXLOGTHREADPARAM Param;


    PAGED_CODE();

    try {

        KeInitializeEvent(&Param.Event, SynchronizationEvent, FALSE);
        Param.Status = STATUS_SUCCESS;
        Status = PsCreateSystemThread(&ThreadHandle,
                                      GENERIC_ALL,
                                      NULL,
                                      NULL,
                                      NULL,
                                      (PKSTART_ROUTINE)NkLogThread,
                                      &Param);

        if (NT_SUCCESS(Status)) {

            ASSERT(NULL != ThreadHandle);

            (VOID)KeWaitForSingleObject(&Param.Event, Executive, KernelMode, FALSE, NULL);

            Status = Param.Status;
            if (!NT_SUCCESS(Status)) {
                ZwClose(ThreadHandle);
                ThreadHandle = NULL;
            }
        }

    }
    finally {

        if (!NT_SUCCESS(Status)) {

            ASSERT(NULL == ThreadHandle);
        }
    }

    lcb.ThreadHandle = ThreadHandle;
    return Status;
}


_IRQL_requires_same_
_Function_class_(KSTART_ROUTINE)
VOID
NkLogThread (
             _In_ PVOID Context
             )
{
    PNXLOGTHREADPARAM   Param = (PNXLOGTHREADPARAM)Context;
    BOOLEAN             Running = FALSE;
    PVOID               Objects[2];


    PAGED_CODE();


    ASSERT(NULL != Param);

    lcb.ThreadId = PsGetCurrentThreadId();
    Param->Status = STATUS_SUCCESS;
    KeSetEvent(&Param->Event, 0, FALSE);
    Param = NULL;

    Running = TRUE;
    Objects[0] = &lcb.LogEvents[0];
    Objects[1] = &lcb.LogEvents[1];

    while (Running)
    {
        NTSTATUS  Status = STATUS_SUCCESS;
        PLOG_ITEM Item = NULL;

        Status = KeWaitForMultipleObjects(2,
                                          Objects,
                                          WaitAny,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          NULL,
                                          NULL
                                          );
        if (STATUS_WAIT_1 == Status) {

            // Stop Event
            Running = FALSE;
        }

        while (NULL != (Item = NkPopItem())) {

            WriteLogItem(Item);
            ExFreePool(Item);
            Item = NULL;

            Status = NkLogRotate();
            if (!NT_SUCCESS(Status)) {
                NkCloseLogFile();
                Running = FALSE;
                lcb.Active = FALSE;
                break;
            }
        }

        // Clear new log message event
        KeClearEvent(&lcb.LogEvents[0]);
    }

    PsTerminateSystemThread(STATUS_SUCCESS);
}


_IRQL_requires_max_(APC_LEVEL)
VOID
NkPushItem(
           _In_ PLOG_ITEM Item
           )
{
    PAGED_CODE();

    if (!lcb.Active) {
        return;
    }

    ExAcquireFastMutex(&lcb.MsgListLock);
    InsertTailList(&lcb.MsgList, &Item->Link);
    lcb.MsgListCount++;
    ExReleaseFastMutex(&lcb.MsgListLock);

    KeSetEvent(&lcb.LogEvents[0], 0, FALSE);
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
PLOG_ITEM
NkPopItem(
          )
{
    PLOG_ITEM Item = NULL;

    PAGED_CODE();

    if (!lcb.Active) {
        return NULL;
    }

    ExAcquireFastMutex(&lcb.MsgListLock);
    if (!IsListEmpty(&lcb.MsgList)) {
        PLIST_ENTRY Entry = NULL;
        ASSERT(0 != lcb.MsgListCount);
        Entry = RemoveHeadList(&lcb.MsgList);
        Item = CONTAINING_RECORD(Entry, LOG_ITEM, Link);
        lcb.MsgListCount--;
    }
    ExReleaseFastMutex(&lcb.MsgListLock);

    return Item;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkLogRotate(
            )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    UNICODE_STRING  SourceName = { 0, 0, NULL };
    UNICODE_STRING  Directory = { 0, 0, NULL };
    UNICODE_STRING  Name = { 0, 0, NULL };
    UNICODE_STRING  Extension = { 0, 0, NULL };
    UNICODE_STRING  Slash = { 0, 0, NULL };
    LARGE_INTEGER   FileSize = { 0, 0 };
    UNICODE_STRING  NewFileName = { 0, 0, NULL };

    PAGED_CODE();


    RtlInitUnicodeString(&Directory, lcb.Directory);
    RtlInitUnicodeString(&Name, lcb.Name);
    RtlInitUnicodeString(&Extension, L"9.log");
    RtlInitUnicodeString(&Slash, L"\\");
    if (Directory.Length < sizeof(WCHAR) || Name.Length < sizeof(WCHAR)) {
        return STATUS_INVALID_PARAMETER;
    }

    try {

        SourceName.MaximumLength = Directory.Length + Slash.Length + Name.Length + Extension.Length + sizeof(WCHAR);
        SourceName.Length = 0;
        SourceName.Buffer = ExAllocatePoolWithTag(PagedPool, SourceName.MaximumLength, TAG_TEMP);
        if (NULL == SourceName.Buffer) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }
        RtlZeroMemory(SourceName.Buffer, SourceName.MaximumLength);
        RtlAppendUnicodeStringToString(&SourceName, &Directory);
        RtlAppendUnicodeStringToString(&SourceName, &Slash);
        RtlAppendUnicodeStringToString(&SourceName, &Name);
        RtlAppendUnicodeStringToString(&SourceName, &Extension);

        NewFileName.MaximumLength = Name.Length + Extension.Length + sizeof(WCHAR);
        NewFileName.Length = 0;
        NewFileName.Buffer = ExAllocatePoolWithTag(PagedPool, NewFileName.MaximumLength, TAG_TEMP);
        if (NULL == NewFileName.Buffer) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }
        RtlZeroMemory(NewFileName.Buffer, NewFileName.MaximumLength);
        RtlCopyUnicodeString(&NewFileName, &Name);
        RtlAppendUnicodeStringToString(&NewFileName, &Extension);

        if (0 == lcb.RotateCount) {
            FileSize.QuadPart = 0;
            (VOID)NkSetFileSize(lcb.FileHandle, FileSize);
            Status = STATUS_SUCCESS;
        }
        else if (1 == lcb.RotateCount) {

            NewFileName.Buffer[(NewFileName.Length / sizeof(WCHAR)) - 5] = LogIndex[0];
            (VOID)NkRenameFile(lcb.FileHandle, &NewFileName, TRUE);
            ZwClose(lcb.FileHandle);
            lcb.FileHandle = NULL;
            Status = NkOpenLogFile();
        }
        else {

            ULONG i = 0;
            ASSERT(lcb.RotateCount <= 10);
            for (i = lcb.RotateCount; i > 0; i--) {

                NewFileName.Buffer[(NewFileName.Length / sizeof(WCHAR)) - 5] = LogIndex[i - 1];

                if (i == 1) {
                    (VOID)NkRenameFile(lcb.FileHandle, &NewFileName, TRUE);
                    ZwClose(lcb.FileHandle);
                    lcb.FileHandle = NULL;
                    Status = NkOpenLogFile();
                    break;
                }
                else {
                    NTSTATUS LocalStatus = STATUS_SUCCESS;
                    HANDLE   SourceHandle = NULL;
                    ULONG    LocalResult = 0;
                    SourceName.Buffer[(SourceName.Length / sizeof(WCHAR)) - 5] = LogIndex[i - 2];
                    LocalStatus = NkOpenFile(&SourceHandle, GENERIC_READ | GENERIC_WRITE | DELETE, &SourceName, &LocalResult, 0, FILE_NON_DIRECTORY_FILE);
                    if (NT_SUCCESS(LocalStatus)) {
                        (VOID)NkRenameFile(SourceHandle, &NewFileName, TRUE);
                        ZwClose(SourceHandle);
                        SourceHandle = NULL;
                    }
                }
            }
        }

try_exit: NOTHING;
    }
    finally {

        if (NULL != SourceName.Buffer) {
            ExFreePool(SourceName.Buffer);
            SourceName.Buffer = NULL;
        }
        if (NULL != NewFileName.Buffer) {
            ExFreePool(NewFileName.Buffer);
            NewFileName.Buffer = NULL;
        }
    }

    return Status;
}