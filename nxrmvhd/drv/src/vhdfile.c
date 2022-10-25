
#include <ntifs.h>
#include <Ntdddisk.h>

#include <nudf\shared\vhddef.h>

#include "nxrmvhd.h"
#include "vhdfile.h"
#include "vhdutil.h"




#pragma pack(push,8)


typedef struct _VHDINSTANCE {
    HANDLE          FileHandle;
    PFILE_OBJECT    FileObject;
    ERESOURCE       DiskFileLock;
    UCHAR           UniqueId[16];
    LONGLONG        DiskSpace;
    DISK_GEOMETRY   DiskGeometry;
    UCHAR           Key[32];
    PVHDBAT         Bat;
} VHDINSTANCE, *PVHDINSTANCE;

#pragma pack(pop)



_Check_return_
PVHDINSTANCE
VhdInterAllocInstance(
                      );


VOID
VhdInterFreeInstance(
                     _Inout_ PVHDINSTANCE Instance
                     );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdLoad(
        _In_ PVHDINSTANCE Instance
        );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdReadBlock(
            _In_ PVHDINSTANCE Instance,
            _In_ ULONG BlockId,
            _In_ ULONG Offset,
            _In_ ULONG BytesToRead,
            _Out_writes_(BytesToRead) PUCHAR Buffer,
            _Out_opt_ PULONG BytesRead
            );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdWriteBlock(
              _In_ PVHDINSTANCE Instance,
              _In_ ULONG BlockId,
              _In_ ULONG Offset,
              _In_ ULONG BytesToWrite,
              _In_reads_(BytesToWrite) const UCHAR* Buffer,
              _Out_opt_ PULONG BytesWritten
              );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdAllocateblock(
                 _In_ PVHDINSTANCE Instance,
                 _In_ ULONG BlockId
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdFlushBlockInfomation(
                        _In_ PVHDINSTANCE Instance,
                        _In_ ULONG BlockId
                        );



//
//  Assign text sections for each routine.
//
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, VhdOpen)
#pragma alloc_text(PAGE, VhdClose)
#endif



_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdOpen(
        _In_ PCUNICODE_STRING DiskFile,
        _Out_ PHANDLE Vhd
        )
{
    NTSTATUS     Status = STATUS_SUCCESS;
    PVHDINSTANCE Instance = NULL;

    PAGED_CODE();
    UNREFERENCED_PARAMETER(DiskFile);


    __try {

        IO_STATUS_BLOCK Iosb = {0};
        OBJECT_ATTRIBUTES ObjectAttributes = {0};

        Instance = VhdInterAllocInstance();
        if(NULL == Instance) {
            try_return(Status = STATUS_INSUFFICIENT_NVRAM_RESOURCES);
        }

        
		InitializeObjectAttributes(&ObjectAttributes,
								   (PUNICODE_STRING)DiskFile,
								   OBJ_CASE_INSENSITIVE | OBJ_FORCE_ACCESS_CHECK | OBJ_KERNEL_HANDLE,
								   NULL,
								   NULL);

        // Open VHD file
        Status = ZwCreateFile(&Instance->FileHandle,
                              GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                              &ObjectAttributes,
                              &Iosb,
                              NULL,
                              0,
                              FILE_SHARE_READ,
                              FILE_OPEN,
                              FILE_NON_DIRECTORY_FILE | FILE_WRITE_THROUGH | FILE_SYNCHRONOUS_IO_NONALERT,
                              NULL,
                              0);
        if(!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = ObReferenceObjectByHandle(Instance->FileHandle,
										   FILE_READ_DATA,
										   NULL,
										   KernelMode,
										   (PVOID*)&(Instance->FileObject),
										   NULL);
        if(!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        //
        //  Load 
        //
        Status = VhdLoad(Instance);
        if(!NT_SUCCESS(Status)) {
            try_return(Status);
        }


try_exit: NOTHING;
    }
    __finally {
        
        if(!NT_SUCCESS(Status)) {
            if(NULL != Instance) {
                VhdInterFreeInstance(Instance);
                Instance = NULL;
            }
        }

        *Vhd = Instance;
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdClose(
         _In_ HANDLE Vhd
         )
{
    NTSTATUS     Status = STATUS_SUCCESS;
    PVHDINSTANCE Instance = (PVHDINSTANCE)Vhd;

    PAGED_CODE();

    if (NULL == Vhd) {
        return STATUS_INVALID_HANDLE;
    }


    try {

        VhdFlushBlockInfomation(Instance, 0xFFFFFFFF);
        VhdInterFreeInstance(Instance);
    }
    finally {
        ; // Nothing
    }

    return Status;
}

VOID
VhdInterFreeInstance(
                     _Inout_ PVHDINSTANCE Instance
                     )
{
    if(NULL == Instance) {
        return;
    }

    if(NULL != Instance->FileObject) {
        ObDereferenceObject(Instance->FileObject);
        Instance->FileObject = NULL;
    }

    if(NULL != Instance->FileHandle) {
        ZwClose(Instance->FileHandle);
        Instance->FileHandle = NULL;
    }

    if(NULL != Instance->Bat) {
        ExFreePool(Instance->Bat);
        Instance->Bat = NULL;
    }

    ExDeleteResourceLite(&Instance->DiskFileLock);

    RtlSecureZeroMemory(Instance, sizeof(VHDINSTANCE));
    ExFreePool(Instance);
    Instance = NULL;
}

_Check_return_
PVHDINSTANCE
VhdInterAllocInstance(
                      )
{
    NTSTATUS     Status = STATUS_SUCCESS;
    PVHDINSTANCE Instance = NULL;

    __try {

        Instance = (PVHDINSTANCE)ExAllocatePoolWithTag(NonPagedPool, sizeof(VHDINSTANCE), NXRMVHITAG);
        if(NULL == Instance) {
            try_return(Status = STATUS_INSUFFICIENT_NVRAM_RESOURCES);
        }
        RtlSecureZeroMemory(Instance, sizeof(VHDINSTANCE));

        Status = ExInitializeResourceLite(&Instance->DiskFileLock);
        if(!NT_SUCCESS(Status)) {
            ExFreePool(Instance);
            Instance = NULL;
            try_return(Status);
        }

        Instance->Bat = (PVHDBAT)ExAllocatePoolWithTag(PagedPool, sizeof(VHDBAT), NXRMVHITAG);
        if(NULL == Instance->Bat) {
            try_return(Status = STATUS_INSUFFICIENT_NVRAM_RESOURCES);
        }
        RtlSecureZeroMemory(Instance->Bat, sizeof(VHDBAT));

try_exit: NOTHING;
    }
    __finally {
        if(!NT_SUCCESS(Status)) {
            VhdInterFreeInstance(Instance);
            Instance = NULL;
        }
    }

    return Instance;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdLoad(
        _In_ PVHDINSTANCE Instance
        )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    VHDFILEHEADER_BLOB* Blob = NULL;

    PAGED_CODE();

    if(NULL == Instance) {
        return STATUS_INVALID_PARAMETER;
    }
    if(NULL == Instance->Bat) {
        return STATUS_INVALID_HANDLE;
    }
    if(NULL == Instance->FileHandle) {
        return STATUS_INVALID_PARAMETER;
    }
    if(NULL == Instance->FileObject) {
        return STATUS_INVALID_HANDLE;
    }

    __try {
        
        IO_STATUS_BLOCK Iosb = {0};
        LARGE_INTEGER   ByteOffset = {0, 0};

        Blob = (VHDFILEHEADER_BLOB*)ExAllocatePoolWithTag(PagedPool, sizeof(VHDFILEHEADER_BLOB), NXRMTMPTAG);
        if(NULL == Blob) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }

        //
        //  Load 
        //

        ByteOffset.QuadPart = 0;
        Status = ZwReadFile(Instance->FileHandle, NULL, NULL, NULL, &Iosb, (PVOID)Blob, sizeof(VHDFILEHEADER_BLOB), &ByteOffset, NULL);
        if(!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        if(Blob->Header.Magic != VHDFMT_FILE_MAGIC) {
            try_return(Status = STATUS_BAD_FILE_TYPE);
        }
        if(Blob->Header.Version != VHDFMT_FILE_VERSION) {
            try_return(Status = STATUS_BAD_FILE_TYPE);
        }

        RtlCopyMemory(Instance->UniqueId, Blob->Header.UniqueId, 16);
        Instance->DiskSpace                       = Blob->Header.DiskSpace;
        Instance->DiskGeometry.Cylinders.QuadPart = Blob->Header.DiskSpace / VHDFMT_SECTOR_SIZE;
        Instance->DiskGeometry.MediaType          = RemovableMedia; // FixedMedia;    // RemovableMedia
        Instance->DiskGeometry.TracksPerCylinder  = 1;
        Instance->DiskGeometry.SectorsPerTrack    = 1;
        Instance->DiskGeometry.BytesPerSector     = VHDFMT_SECTOR_SIZE;

        ByteOffset.QuadPart = VHDFMT_BLOCKTABLE_START;
        Status = ZwReadFile(Instance->FileHandle, NULL, NULL, NULL, &Iosb, (PVOID)Instance->Bat, sizeof(VHDBAT), &ByteOffset, NULL);
        if(!NT_SUCCESS(Status)) {
            try_return(Status);
        }


try_exit: NOTHING;
    }
    __finally {
        
        if(NULL != Blob) {
            ExFreePool(Blob);
            Blob = NULL;
        }
    }

    return Status;
}



_Check_return_
LONGLONG
VhdGetVolumeSize(
                 _In_ HANDLE Vhd
                 )
{
    PVHDINSTANCE Instance = (PVHDINSTANCE)Vhd;
    return (NULL == Instance) ? 0LL : Instance->DiskSpace;
}


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdRead(
        _In_ HANDLE Vhd,
        _In_ LONGLONG Start,
        _In_ ULONG Length,
        _Out_writes_(Length) PUCHAR Buffer,
        _Out_ PULONG BytesReturned
        )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    PVHDINSTANCE Instance = (PVHDINSTANCE)Vhd;
#ifdef _DEBUG
    const ULONG BlockCount = (ULONG)(Instance->DiskSpace / VHDFMT_BLOCK_SIZE);
#endif

    PAGED_CODE();

    *BytesReturned = 0;

    if(NULL == Instance->FileHandle) {
        return STATUS_FILE_CLOSED;
    }

    // Check Alignment
    if(0 != (Start % VHDFMT_SECTOR_SIZE)) {
        return STATUS_INVALID_PARAMETER;
    }
    if(Start >= Instance->DiskSpace) {
        return STATUS_INVALID_PARAMETER;
    }
    if((Start+Length) > Instance->DiskSpace) {
        Length = (ULONG)(Instance->DiskSpace - Start);
    }
    if(0 != (Length % VHDFMT_SECTOR_SIZE)) {
        return STATUS_INVALID_PARAMETER;
    }

    while(Length != 0) {
        
        ULONG BlockId = 0;
        ULONG OffsetInBlock = 0;
        ULONG BytesToRead = 0;
        ULONG BytesRead = 0;

        PositionToBlockInfo(Start, &BlockId, &OffsetInBlock);
#ifdef _DEBUG
        ASSERT(BlockId < BlockCount);
#endif
        ASSERT(OffsetInBlock < VHDFMT_BLOCK_SIZE);
        ASSERT(0 == (OffsetInBlock % VHDFMT_SECTOR_SIZE));

        BytesToRead = VHDFMT_BLOCK_SIZE - OffsetInBlock;
        if(BytesToRead > Length) {
            BytesToRead = Length;
        }
        ASSERT(0 == (BytesToRead % VHDFMT_SECTOR_SIZE));
        Status = VhdReadBlock(Instance, BlockId, OffsetInBlock, BytesToRead, Buffer, &BytesRead);
        if(!NT_SUCCESS(Status)) {
            return Status;
        }
        if(BytesToRead != BytesRead) {
            return STATUS_FILE_CORRUPT_ERROR;
        }

        Start  += BytesRead;
        Buffer += BytesRead;
        Length -= BytesRead;
        *BytesReturned += BytesRead;
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdWrite(
         _In_ HANDLE Vhd,
         _In_ LONGLONG Start,
         _In_ ULONG Length,
         _In_reads_(Length) const UCHAR* Buffer,
         _Out_ PULONG BytesReturned
         )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    PVHDINSTANCE Instance = (PVHDINSTANCE)Vhd;
#ifdef _DEBUG
    const ULONG BlockCount = (ULONG)(Instance->DiskSpace / VHDFMT_BLOCK_SIZE);
#endif

    PAGED_CODE();

    *BytesReturned = 0;

    if(NULL == Instance->FileHandle) {
        return STATUS_FILE_CLOSED;
    }

    // Check Alignment
    if(0 != (Start % VHDFMT_SECTOR_SIZE)) {
        return STATUS_INVALID_PARAMETER;
    }
    if(Start >= Instance->DiskSpace) {
        return STATUS_INVALID_PARAMETER;
    }
    if((Start+Length) > Instance->DiskSpace) {
        Length = (ULONG)(Instance->DiskSpace - Start);
    }
    if(0 != (Length % VHDFMT_SECTOR_SIZE)) {
        return STATUS_INVALID_PARAMETER;
    }

    while(Length != 0) {
        
        ULONG BlockId = 0;
        ULONG OffsetInBlock = 0;
        ULONG BytesToWrite = 0;
        ULONG BytesWritten = 0;

        PositionToBlockInfo(Start, &BlockId, &OffsetInBlock);
#ifdef _DEBUG
        ASSERT(BlockId < BlockCount);
#endif
        ASSERT(OffsetInBlock < VHDFMT_BLOCK_SIZE);
        ASSERT(0 == (OffsetInBlock % VHDFMT_SECTOR_SIZE));

        BytesToWrite = VHDFMT_BLOCK_SIZE - OffsetInBlock;
        if(BytesToWrite > Length) {
            BytesToWrite = Length;
        }
        ASSERT(0 == (BytesToWrite % VHDFMT_SECTOR_SIZE));
        Status = VhdWriteBlock(Instance, BlockId, OffsetInBlock, BytesToWrite, Buffer, &BytesWritten);
        if(!NT_SUCCESS(Status)) {
            return Status;
        }
        if(BytesToWrite != BytesWritten) {
            return STATUS_FILE_CORRUPT_ERROR;
        }

        Start  += BytesWritten;
        Buffer += BytesWritten;
        Length -= BytesWritten;
        *BytesReturned += BytesWritten;
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdReadBlock(
            _In_ PVHDINSTANCE Instance,
            _In_ ULONG BlockId,
            _In_ ULONG Offset,
            _In_ ULONG BytesToRead,
            _Out_writes_(BytesToRead) PUCHAR Buffer,
            _Out_opt_ PULONG BytesRead
            )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    IO_STATUS_BLOCK Iosb = {0};
    LARGE_INTEGER   ByteOffset = {0, 0};

    PAGED_CODE();

    // Check Alignment
    if(BlockId >= VHDFMT_MAX_BLOCK_COUNT) {
        return STATUS_INVALID_PARAMETER;
    }
    if(0 != (Offset % VHDFMT_SECTOR_SIZE)) {
        return STATUS_INVALID_PARAMETER;
    }
    if(0 != (BytesToRead % VHDFMT_SECTOR_SIZE)) {
        return STATUS_INVALID_PARAMETER;
    }
    if((Offset + BytesToRead) > VHDFMT_BLOCK_SIZE) {
        return STATUS_INVALID_PARAMETER;
    }

    __try {
        
        ULONG BlockState = VhdGetBlockState(Instance->Bat, BlockId);
        
        
        ExEnterCriticalRegionAndAcquireResourceShared(&Instance->DiskFileLock);
        BlockState = VhdGetBlockState(Instance->Bat, BlockId);
        ExReleaseResourceAndLeaveCriticalRegion(&Instance->DiskFileLock);

        if(BooleanFlagOn(BlockState, BLOCK_STATE_ALLOCED) && BooleanFlagOn(BlockState, BLOCK_STATE_INITED)) {

            // Read Data
            ByteOffset.QuadPart = SeqIdToBlockOffset(VhdGetBlockSeqId(Instance->Bat, BlockId)) + Offset;
            Status = ZwReadFile(Instance->FileHandle, NULL, NULL, NULL, &Iosb, Buffer, BytesToRead, &ByteOffset, NULL);
            if(!NT_SUCCESS(Status)) {
                try_return(Status);
            }

            // Decrypt Data
        }
        else {

            // Not allocated or not initialized
            // Set zero
            RtlZeroMemory(Buffer, BytesToRead);
            Iosb.Information = BytesToRead;
        }


try_exit: NOTHING;
    }
    __finally {

        if(NULL != BytesRead) {
            *BytesRead = NT_SUCCESS(Status) ? (ULONG)Iosb.Information : 0;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdWriteBlock(
              _In_ PVHDINSTANCE Instance,
              _In_ ULONG BlockId,
              _In_ ULONG Offset,
              _In_ ULONG BytesToWrite,
              _In_reads_(BytesToWrite) const UCHAR* Buffer,
              _Out_opt_ PULONG BytesWritten
              )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    IO_STATUS_BLOCK Iosb = {0};
    LARGE_INTEGER   ByteOffset = {0, 0};
    PUCHAR          Cipher = NULL;

    PAGED_CODE();

    // Check Alignment
    if(BlockId >= VHDFMT_MAX_BLOCK_COUNT) {
        return STATUS_INVALID_PARAMETER;
    }
    if(0 != (Offset % VHDFMT_SECTOR_SIZE)) {
        return STATUS_INVALID_PARAMETER;
    }
    if(0 != (BytesToWrite % VHDFMT_SECTOR_SIZE)) {
        return STATUS_INVALID_PARAMETER;
    }
    if((Offset + BytesToWrite) > VHDFMT_BLOCK_SIZE) {
        return STATUS_INVALID_PARAMETER;
    }

    __try {
        

        ULONG BlockState = 0;

        ExEnterCriticalRegionAndAcquireResourceShared(&Instance->DiskFileLock);
        BlockState = VhdGetBlockState(Instance->Bat, BlockId);
        ExReleaseResourceAndLeaveCriticalRegion(&Instance->DiskFileLock);

        if(!BooleanFlagOn(BlockState, BLOCK_STATE_ALLOCED)) {

            // Allocate new block
            Status = VhdAllocateblock(Instance, BlockId);
            if(!NT_SUCCESS(Status)) {
                try_return(Status);
            }
#ifndef _DEBUG
            BlockState |= BLOCK_STATE_ALLOCED;
#else
            ExEnterCriticalRegionAndAcquireResourceShared(&Instance->DiskFileLock);
            BlockState = VhdGetBlockState(Instance->Bat, BlockId);
            ExReleaseResourceAndLeaveCriticalRegion(&Instance->DiskFileLock);
#endif
        }

        ASSERT(BooleanFlagOn(BlockState, BLOCK_STATE_ALLOCED));
        
        if(!BooleanFlagOn(BlockState, BLOCK_STATE_INITED)) {

            Cipher = (PUCHAR)ExAllocatePoolWithTag(PagedPool, VHDFMT_BLOCK_SIZE, NXRMTMPTAG);
            if(NULL == Cipher) {
                try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
            }

            RtlZeroMemory(Cipher, VHDFMT_BLOCK_SIZE);
            RtlCopyMemory(Cipher+Offset, Buffer, BytesToWrite);

            // Encrypt Data

            // Write
            ByteOffset.QuadPart = SeqIdToBlockOffset(VhdGetBlockSeqId(Instance->Bat, BlockId));
            Status = ZwWriteFile(Instance->FileHandle, NULL, NULL, NULL, &Iosb, (PVOID)Cipher, VHDFMT_BLOCK_SIZE, &ByteOffset, NULL);
            if(!NT_SUCCESS(Status)) {
                try_return(Status);
            }

            ASSERT(Iosb.Information == VHDFMT_BLOCK_SIZE);

            Iosb.Information = (Iosb.Information > Offset) ? (Iosb.Information - Offset) : 0;
            if (Iosb.Information > BytesToWrite) {
                Iosb.Information = BytesToWrite;
            }
            
            VhdSetBlockState(Instance->Bat, BlockId, BLOCK_STATE_INITED);
            VhdFlushBlockInfomation(Instance, BlockId);
        }
        else {

            // Encrypt Data

            // Write
            ByteOffset.QuadPart = SeqIdToBlockOffset(VhdGetBlockSeqId(Instance->Bat, BlockId)) + Offset;
            Status = ZwWriteFile(Instance->FileHandle, NULL, NULL, NULL, &Iosb, (PVOID)Buffer, BytesToWrite, &ByteOffset, NULL);
            if(!NT_SUCCESS(Status)) {
                try_return(Status);
            }
        }

try_exit: NOTHING;
    }
    __finally {

        if(NULL != BytesWritten) {
            *BytesWritten = NT_SUCCESS(Status) ? (ULONG)Iosb.Information : 0;
        }

        if(NULL != Cipher) {
            ExFreePool(Cipher);
            Cipher = NULL;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdAllocateblock(
                 _In_ PVHDINSTANCE Instance,
                 _In_ ULONG BlockId
                 )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    IO_STATUS_BLOCK Iosb = {0};
    ULONG BlockState = 0;
    ULONG RetryCount = 0;
    BOOLEAN SelfAllocating = FALSE;

    static const ULONG MaxRetryCount = 10;

    PAGED_CODE();

    // Check Alignment
    if(BlockId >= VHDFMT_MAX_BLOCK_COUNT) {
        return STATUS_INVALID_PARAMETER;
    }

    // Get Status
    ExEnterCriticalRegionAndAcquireResourceExclusive(&Instance->DiskFileLock);
    BlockState = VhdGetBlockState(Instance->Bat, BlockId);
    if(0 == BlockState) {
        BlockState = VhdSetBlockState(Instance->Bat, BlockId, BLOCK_STATE_ALLOCING);
        SelfAllocating = TRUE;
    }
    else {
        // Someone else IS allocating this block, or this block has been allocated
        ASSERT(BooleanFlagOn(BlockState, BLOCK_STATE_ALLOCING) || BooleanFlagOn(BlockState, BLOCK_STATE_ALLOCED));
    }
    ExReleaseResourceAndLeaveCriticalRegion(&Instance->DiskFileLock);

    if(BooleanFlagOn(BlockState, BLOCK_STATE_ALLOCED)) {
        // If the block has been allocated
        // Return Success
        return STATUS_SUCCESS;
    }

    ASSERT(BooleanFlagOn(BlockState, BLOCK_STATE_ALLOCING));

    // If there is another request is allocating this block
    // Wait and re-try
    if(!SelfAllocating) {

        do {

            // Delay 200 milliseconds, and try again
            VhdSleep(200);
            // Check Status Again
            ExEnterCriticalRegionAndAcquireResourceExclusive(&Instance->DiskFileLock);
            BlockState = VhdGetBlockState(Instance->Bat, BlockId);
            ExReleaseResourceAndLeaveCriticalRegion(&Instance->DiskFileLock);
            if(0 == BlockState) {
                // Someone has tried to allocate this disk but failed
                Status = STATUS_DISK_FULL;
                break;
            }
            else if(BooleanFlagOn(BlockState, BLOCK_STATE_ALLOCED)) {
                // If the block has been allocated by someone else
                // Return Success
                return STATUS_SUCCESS;
            }
            else if(BooleanFlagOn(BlockState, BLOCK_STATE_ALLOCING)) {
                // Still allocating?
                // Wait if try-count less than 10
                // Otherwise return error
                if(RetryCount > MaxRetryCount) {
                    Status = STATUS_DISK_FULL;
                    break;
                }
                continue;
            }
            else {
                // SHould never reach here
                ASSERT(FALSE);
            }

        } while(MaxRetryCount >= RetryCount++);

        return Status;
    }

    __try {
        
        FILE_STANDARD_INFORMATION fsi = {0};
        FILE_END_OF_FILE_INFORMATION feof = {0};
        ULONG SeqId = 0;

                
        RtlZeroMemory(&fsi, sizeof(fsi));
        Status = ZwQueryInformationFile(Instance->FileHandle, &Iosb, &fsi, sizeof(fsi), FileStandardInformation);
        if(!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        if (0 != ((fsi.EndOfFile.QuadPart - VHDFMT_BLOCK_START) % VHDFMT_BLOCK_SIZE)) {
            ASSERT(0 == ((fsi.EndOfFile.QuadPart - VHDFMT_BLOCK_START) % VHDFMT_BLOCK_SIZE));
        }
        SeqId = (ULONG)((fsi.EndOfFile.QuadPart - VHDFMT_BLOCK_START) / VHDFMT_BLOCK_SIZE);

        feof.EndOfFile.QuadPart = fsi.EndOfFile.QuadPart + VHDFMT_BLOCK_SIZE;
        Status = ZwSetInformationFile(Instance->FileHandle, &Iosb, &feof, sizeof(feof), FileEndOfFileInformation);
        if(!NT_SUCCESS(Status)) {
            try_return(Status);
        }
        
        // Succeeded
        Status = STATUS_SUCCESS;
        ExEnterCriticalRegionAndAcquireResourceExclusive(&Instance->DiskFileLock);
        BlockState = BLOCK_STATE_ALLOCED;
        Instance->Bat->BlockInfo[BlockId] = BLOCK_STATE_ALLOCED;
        Instance->Bat->BlockInfo[BlockId] <<= 16;
        Instance->Bat->BlockInfo[BlockId] |= (SeqId & 0x0000FFFF);
        ExReleaseResourceAndLeaveCriticalRegion(&Instance->DiskFileLock);
        VhdFlushBlockInfomation(Instance, BlockId);

try_exit: NOTHING;
    }
    __finally {
        if(!NT_SUCCESS(Status)) {
            ExEnterCriticalRegionAndAcquireResourceExclusive(&Instance->DiskFileLock);
            Instance->Bat->BlockInfo[BlockId] = 0;
            ExReleaseResourceAndLeaveCriticalRegion(&Instance->DiskFileLock);
        }
    }

    return Status;
}


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdFlushBlockInfomation(
                        _In_ PVHDINSTANCE Instance,
                        _In_ ULONG BlockId
                        )
{
    NTSTATUS        Status = STATUS_SUCCESS;

    PAGED_CODE();

    // Check Alignment
    if (BlockId >= VHDFMT_MAX_BLOCK_COUNT) {
        if (0xFFFFFFFF != BlockId) {
            return STATUS_INVALID_PARAMETER;
        }
    }
    
    __try {

        IO_STATUS_BLOCK Iosb = { 0 };
        LARGE_INTEGER   ByteOffset = { 0, 0 };
        ULONG           BytesToWrite = 0;
        PVOID           Buffer = NULL;

        if (0xFFFFFFFF != BlockId) {
            Buffer = (PVOID)(Instance->Bat->BlockInfo + BlockId);
            ByteOffset.QuadPart = VHDFMT_BLOCKTABLE_START + sizeof(ULONG) * BlockId;
            BytesToWrite = sizeof(ULONG);
        }
        else {
            Buffer = (PVOID)Instance->Bat;
            ByteOffset.QuadPart = VHDFMT_BLOCKTABLE_START;
            BytesToWrite = sizeof(VHDBAT);
        }

        Status = ZwWriteFile(Instance->FileHandle, NULL, NULL, NULL, &Iosb, (PVOID)Instance->Bat, sizeof(VHDBAT), &ByteOffset, NULL);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

try_exit: NOTHING;
    }
    __finally {
        ; // NOTHING
    }

    return Status;
}