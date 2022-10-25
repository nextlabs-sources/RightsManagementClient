


#include <ntifs.h>
#include <Ntstrsafe.h>

#include <Mountmgr.h>
#include <Mountdev.h>

#include "vhdfile.h"
#include "vhdutil.h"


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
ULONG
GetCPUCount(
            )
{
	KAFFINITY	ActiveCPUMap        = KeQueryActiveProcessors();
	ULONG_PTR	ActiveCPUMapSize    = sizeof(ActiveCPUMap) * 8;
	ULONG		CPUCount            = 0;

	while(ActiveCPUMapSize--) {
		if(ActiveCPUMap & 1) {
			++CPUCount;
        }
		ActiveCPUMap >>= 1;
	}

	if(CPUCount == 0) {
		return 1;
    }

	return CPUCount;
}

VOID
GetDosNameFromDriveLetter(
                          _Out_writes_(128) LPWSTR DosName,
                          _In_ WCHAR DriveLetter
                          )
{
    RtlCopyMemory(DosName, DOS_MOUNT_PREFIX, sizeof(DOS_MOUNT_PREFIX));
    DosName[sizeof(DOS_MOUNT_PREFIX)/sizeof(WCHAR) - 1] = DriveLetter;
    DosName[sizeof(DOS_MOUNT_PREFIX)/sizeof(WCHAR)] =L':';
    DosName[sizeof(DOS_MOUNT_PREFIX)/sizeof(WCHAR) + 1] =L'\0';
}

VOID
GetNtNameFromNumber(
                    _Out_writes_(128) LPWSTR NtName,
                    _In_ LONG VolumeId
                    )
{
    ASSERT(VolumeId >= 0 && VolumeId < MAX_ACTIVEDISK);
    RtlStringCbPrintfW(NtName, 128, L"%s%d", NT_MOUNT_PREFIX, VolumeId);
}

_Check_return_
BOOLEAN
ValidateIoBufferSize(
                     _In_ PIRP Irp,
                     _In_ size_t RequiredBufferSize,
                     _In_ ValidateIoBufferSizeType Type)
{
	PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);

	BOOLEAN Input  = (Type == ValidateInput || Type == ValidateInputOutput);
	BOOLEAN Output = (Type == ValidateOutput || Type == ValidateInputOutput);

	if((Input && IrpSp->Parameters.DeviceIoControl.InputBufferLength < RequiredBufferSize) || 
	   (Output && IrpSp->Parameters.DeviceIoControl.OutputBufferLength < RequiredBufferSize))
	{
		Irp->IoStatus.Status		= STATUS_BUFFER_TOO_SMALL;
		Irp->IoStatus.Information	= 0;		
		return FALSE;
	}

	if(!Input && Output) {
		memset(Irp->AssociatedIrp.SystemBuffer, 0, IrpSp->Parameters.DeviceIoControl.OutputBufferLength);
    }

	return TRUE;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
ULONG
GetAvailableDiskId(
                   )
{
    static ULONG Id = 0;
    ULONG ValidId = 0xFFFFFFFF;

    PAGED_CODE();

    ExEnterCriticalRegionAndAcquireResourceExclusive(&_VhdMgr.VhdDiskLock);
    if (Id > 9999) {
        Id = 0;
    }
    ValidId = Id++;
    ExReleaseResourceAndLeaveCriticalRegion(&_VhdMgr.VhdDiskLock);

    return ValidId;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
ULONG
GetActiveDiskCount(
                   )
{
    ULONG Count = 0;

    PAGED_CODE();

    ExEnterCriticalRegionAndAcquireResourceShared(&_VhdMgr.VhdDiskLock);
    __try {
        Count = _VhdMgr.VhdDiskCount;
    }
    __finally {
        ExReleaseResourceAndLeaveCriticalRegion(&_VhdMgr.VhdDiskLock);
    }

    return Count;
}

_Check_return_
BOOLEAN
IsDriveLetterAvailable(
                       _In_ WCHAR DriveLetter
                       )
{
	OBJECT_ATTRIBUTES	ObjectAttributes = {0};
	UNICODE_STRING		ObjectName = {0};
	WCHAR				LinkName[128] = {0};
	HANDLE				LinkHandle = NULL;
    
    ASSERT(DriveLetter >= L'A' && DriveLetter <= L'Z');

	GetDosNameFromDriveLetter(LinkName, DriveLetter);
	RtlInitUnicodeString(&ObjectName, LinkName);	
	InitializeObjectAttributes(&ObjectAttributes,
							   &ObjectName,
							   OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
							   NULL,
							   NULL);

	if(NT_SUCCESS(ZwOpenSymbolicLinkObject(&LinkHandle, GENERIC_READ, &ObjectAttributes))) {
		ZwClose (LinkHandle);
        LinkHandle = NULL;
		return FALSE;
	}

	return TRUE;
}

_Check_return_
WCHAR
FindAvailableDriveLetter(
                         _In_ WCHAR PreferredDriveLetter
                         )
{
    WCHAR i = 0;

    if(PreferredDriveLetter>=L'a' && PreferredDriveLetter<=L'z') {
        PreferredDriveLetter -= 0x20;
    }
    if(PreferredDriveLetter>L'C' || PreferredDriveLetter<=L'Z') {
        if(IsDriveLetterAvailable(PreferredDriveLetter)) {
            return PreferredDriveLetter;
        }
    }
    for(i=L'D'; i<=L'Z'; i++) {
        if(i == PreferredDriveLetter) {
            continue;
        }
        if(IsDriveLetterAvailable(i)) {
            return i;
        }
    }
    return 0;
}

_Check_return_
NTSTATUS
VhdDeviceIoControl(
                   _In_ PWSTR DeviceName,
                   _In_ ULONG IoControlCode,
                   _In_opt_ PVOID InputBuffer,
                   _In_ ULONG InputBufferSize,
                   _Out_opt_ PVOID OutputBuffer,
                   _In_ ULONG OutputBufferSize
                   )
{
	NTSTATUS        Status = STATUS_SUCCESS;
	IO_STATUS_BLOCK IoStatusBlock ={0};
	PIRP            Irp = NULL;
	PFILE_OBJECT    FileObject = NULL;
	PDEVICE_OBJECT  DeviceObject = NULL;
	KEVENT          WaitEvent = {0};
	UNICODE_STRING  Name = {0};

	__try {

		RtlInitUnicodeString(&Name, DeviceName);

		Status = IoGetDeviceObjectPointer(&Name, FILE_READ_ATTRIBUTES, &FileObject, &DeviceObject);
		if(!NT_SUCCESS(Status)) {
			goto try_exit;
        }

		KeInitializeEvent(&WaitEvent, NotificationEvent, FALSE);
		Irp = IoBuildDeviceIoControlRequest(IoControlCode,
											DeviceObject,
											InputBuffer,
											InputBufferSize,
											OutputBuffer,
											OutputBufferSize,
											FALSE,
											&WaitEvent,
											&IoStatusBlock);
		if(Irp == NULL) {
			Status = STATUS_INSUFFICIENT_RESOURCES;
			goto try_exit;
		}

		IoGetNextIrpStackLocation(Irp)->FileObject = FileObject;
		Status = IoCallDriver(DeviceObject, Irp);
		if(Status == STATUS_PENDING) {
			KeWaitForSingleObject(&WaitEvent, Executive, KernelMode, FALSE, NULL);
			Status = IoStatusBlock.Status;
		}

try_exit: NOTHING;
	}
    __finally {
        if(FileObject) {
            ObDereferenceObject(FileObject);
            FileObject = NULL;
        }
    }

	return Status;
}

_Check_return_
NTSTATUS
MountManagerMount(
                  _In_ PDEVICE_OBJECT DeviceObject
                  )
{
	NTSTATUS    Status = STATUS_SUCCESS; 
	PUCHAR      buf  = NULL;
    PVHDDISKEXT pExt = (PVHDDISKEXT)DeviceObject->DeviceExtension;

	PMOUNTMGR_TARGET_NAME        Target = NULL;
	PMOUNTMGR_CREATE_POINT_INPUT Point  = NULL;

    buf = (PUCHAR)ExAllocatePoolWithTag(NonPagedPool, 1024, NXRMTMPTAG);
    if(NULL == buf) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    Target = (PMOUNTMGR_TARGET_NAME)buf;
    Point  = (PMOUNTMGR_CREATE_POINT_INPUT)buf;

    __try {

        RtlZeroMemory(buf, 1024);
        Target->DeviceNameLength = pExt->NtDeviceName.Length;
        RtlCopyMemory(Target->DeviceName, pExt->NtDeviceName.Buffer, pExt->NtDeviceName.Length);
        Status = VhdDeviceIoControl(MOUNTMGR_DEVICE_NAME,
                                    IOCTL_MOUNTMGR_VOLUME_ARRIVAL_NOTIFICATION,
                                    Target, 
                                    (ULONG)(sizeof(Target->DeviceNameLength) + Target->DeviceNameLength),
                                    NULL,
                                    0);
        if(!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        if(pExt->Visible && 0 != pExt->DosDriveLetter) {

            RtlZeroMemory(buf, 1024);

            Point->SymbolicLinkNameOffset = sizeof(MOUNTMGR_CREATE_POINT_INPUT);
            Point->SymbolicLinkNameLength = pExt->DosDeviceName.Length;
            RtlCopyMemory(&Point[1], pExt->DosDeviceName.Buffer, pExt->DosDeviceName.Length);

            Point->DeviceNameOffset = Point->SymbolicLinkNameOffset + Point->SymbolicLinkNameLength;
            Point->DeviceNameLength = pExt->NtDeviceName.Length;
            RtlCopyMemory((buf + Point->DeviceNameOffset), pExt->NtDeviceName.Buffer, pExt->NtDeviceName.Length);

            Status = VhdDeviceIoControl(MOUNTMGR_DEVICE_NAME,
                                        IOCTL_MOUNTMGR_CREATE_POINT,
                                        Point,
                                        Point->DeviceNameOffset + Point->DeviceNameLength,
                                        0,
                                        0);
        }

try_exit: NOTHING;
    }
    __finally {
        ExFreePool(buf);
        buf = NULL;
    }

	return Status;
}

_Check_return_
NTSTATUS
MountManagerUnmount(
                    _In_ PUNICODE_STRING DosName
                    )
{
	NTSTATUS Status = STATUS_SUCCESS; 
	CHAR     buf[300] = {0};
	CHAR     out[300] = {0};
	PMOUNTMGR_MOUNT_POINT Target = (PMOUNTMGR_MOUNT_POINT)buf;

    if(DosName->Length == 0 || NULL == DosName->Buffer) {
        return STATUS_INVALID_PARAMETER;
    }

    RtlZeroMemory(buf, sizeof(buf));
    RtlZeroMemory(out, sizeof(out));
    
    // Only symbolic link can be deleted with IOCTL_MOUNTMGR_DELETE_POINTS.
    // If any other entry is specified, the mount manager will ignore
    // subsequent IOCTL_MOUNTMGR_VOLUME_ARRIVAL_NOTIFICATION for the same volume ID.
    Target->SymbolicLinkNameOffset = sizeof(MOUNTMGR_MOUNT_POINT);
    Target->SymbolicLinkNameLength = DosName->Length;
    RtlCopyMemory((PWSTR)(&Target[1]), DosName->Buffer, DosName->Length);

    Status = VhdDeviceIoControl(MOUNTMGR_DEVICE_NAME,
                                IOCTL_MOUNTMGR_DELETE_POINTS,
                                Target,
                                sizeof(MOUNTMGR_MOUNT_POINT) + Target->SymbolicLinkNameLength,
                                out,
                                sizeof(out));

	return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdCloseDeviceObject(
                     _In_ PDEVICE_OBJECT DeviceObject
                     )
{
	NTSTATUS        Status = STATUS_SUCCESS;
	PVHDDISKEXT     pExt = NULL;
	PFILE_OBJECT	VolumeFileObject = NULL;
	ULONG           i = 0;
    ULONG           DiskId = 0xFFFFFFFF;

    PAGED_CODE();

	__try {

		if(NULL == DeviceObject) {
            goto try_exit;
        }

		pExt = (PVHDDISKEXT)DeviceObject->DeviceExtension;
        DiskId = pExt->DiskId;

	
		//
		// unload file system from our virtual volume
        //
        do {

            Status = VhdOpenFsVolume(pExt, &VolumeFileObject);
            if(!NT_SUCCESS(Status)) {
                break;
            }

            //
            // lock volume
            //
            Status = VhdFsctlCall(VolumeFileObject, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0);
            if(!NT_SUCCESS(Status)) {
                break;
            }

            for(i =0 ; i< 100; i++) {

                Status = VhdFsctlCall(VolumeFileObject, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0);
                if(Status == STATUS_VOLUME_DISMOUNTED) {
                    Status = STATUS_SUCCESS;
                }
                if(Status == STATUS_SUCCESS) {
                    break;
                }
                VhdSleep(100);
            }

        } while(FALSE);

		if(VolumeFileObject) {
			CloseFsVolume(VolumeFileObject);
			VolumeFileObject = NULL;
		}

        if(pExt->Visible) {
            MountManagerUnmount(&pExt->DosDeviceName);
            VhdRemoveDriveLink(&pExt->DosDeviceName);
        }
		pExt->StopPending = 1;

		IoAcquireRemoveLock(&pExt->IoRemoveLock,NULL);
		//
		// Call Remove lock and wait to ensure all outstanding operations
		// have completed
		//
		IoReleaseRemoveLockAndWait(&pExt->IoRemoveLock, NULL);

		if(pExt->VhdFileHandle != NULL) {
            VhdClose(pExt->VhdFileHandle);
			pExt->VhdFileHandle = NULL;
		}

		pExt->VolumeDevice = NULL;
        pExt = NULL;
		IoDeleteDevice(DeviceObject);

try_exit: NOTHING;
	}
    __finally {
        ; // NOTHING
    }

	return Status;
}

_Check_return_
NTSTATUS
VhdCreateDeviceObject(
                      _In_ PDRIVER_OBJECT DriverObject,
                      _Out_ PDEVICE_OBJECT* ppDeviceObject,
                      _In_ PNXRMVHDMOUNTDRIVE Mount
                      )
{
	NTSTATUS Status = STATUS_SUCCESS;

	WCHAR	DosName[VHD_MAX_NAME_LENGTH] = {0};
	WCHAR	NtName[VHD_MAX_NAME_LENGTH] = {0};
	ULONG	DevChars = 0;

	UNICODE_STRING	Win32NameString = {0};
	UNICODE_STRING	NtUnicodeString = {0};

    PDEVICE_OBJECT  DeviceObject = NULL;
	PVHDDISKEXT     pExt = NULL;
    BOOLEAN         LinkCreated = FALSE;

    const ULONG DiskId = GetAvailableDiskId();

    *ppDeviceObject = NULL;

    // Make sure we can get unique Volume Id
    if(DiskId == 0xFFFFFFFF) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    ASSERT(DiskId < MAX_ACTIVEDISK);

	__try  {
                
        RtlZeroMemory(DosName, sizeof(DosName));
        RtlZeroMemory(NtName, sizeof(NtName));
        if(Mount->Visible && 0 != Mount->PreferredDriveLetter) {
            GetDosNameFromDriveLetter(DosName, Mount->PreferredDriveLetter);
        }
		GetNtNameFromNumber(NtName, DiskId);
		
		RtlInitUnicodeString(&NtUnicodeString, NtName);		
		RtlInitUnicodeString(&Win32NameString, DosName);

		DevChars = FILE_DEVICE_SECURE_OPEN;
		DevChars |= Mount->Removable ? FILE_REMOVABLE_MEDIA : 0;

		Status = IoCreateDevice(DriverObject,           // Our Driver Object
								sizeof(VHDDISKEXT),	    // Size of state information
								&NtUnicodeString,       // Device name "\Device\Name"
								FILE_DEVICE_DISK,       // Device type
								DevChars,               // Device characteristics
								FALSE,                  // Exclusive device
								&DeviceObject);         // Returned ptr to Device Object

		if(!NT_SUCCESS(Status)) {
			goto try_exit;
		}

		pExt = (PVHDDISKEXT)(DeviceObject->DeviceExtension);
        RtlZeroMemory(pExt, sizeof(VHDDISKEXT));
		pExt->StopPending					= 0;		
		pExt->OutstandingIoCount			= 0;
		pExt->VhdFileHandle  				= NULL;		
		pExt->VolumeDevice					= DeviceObject;
		pExt->BytesPerSector				= VHDFMT_SECTOR_SIZE;
		pExt->PartitionType					= 0;		
		pExt->SectorsPerTrack				= 1;
		pExt->TracksPerCylinder				= 1;
		pExt->Visible     				    = Mount->Visible ? 1 : 0;
		pExt->Removable     				= Mount->Removable ? 1 : 0;
        pExt->DosDriveLetter				= Mount->PreferredDriveLetter;
		pExt->DiskId				        = DiskId;

		IoInitializeRemoveLock(&pExt->IoRemoveLock, NXRMVHDTAG, 0, 0);		
		KeInitializeEvent(&pExt->NoOutstandingIoEvent, SynchronizationEvent, FALSE);

        if(pExt->Visible) {
            RtlCopyMemory(pExt->DosDeviceNameBuf, DosName, sizeof(DosName));
            RtlInitUnicodeString(&pExt->DosDeviceName, pExt->DosDeviceNameBuf);
        }
        RtlCopyMemory(pExt->NtDeviceNameBuf, NtName, sizeof(NtName));
        RtlInitUnicodeString(&pExt->NtDeviceName, pExt->NtDeviceNameBuf);        
                
        // Try to create SymbolicLink
        if(pExt->Visible && 0 != Mount->PreferredDriveLetter) {
            Status = VhdCreateDriveLink(&pExt->NtDeviceName, &pExt->DosDeviceName);
            if(!NT_SUCCESS(Status)) {
                RtlZeroMemory(&pExt->DosDeviceName, sizeof(UNICODE_STRING));
                RtlZeroMemory(pExt->NtDeviceNameBuf, 128);
                try_return (Status);
            }
            LinkCreated = TRUE;
        }

		Status = VhdOpenVolume(pExt, Mount);
		if(!NT_SUCCESS(Status)) {
            goto try_exit;
		}

		DeviceObject->Flags |= DO_DIRECT_IO;
		DeviceObject->StackSize += 6;

		//
		// Device is ready
		//
		DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
        // Transfer the ownership
        *ppDeviceObject = DeviceObject;
        DeviceObject = NULL;

try_exit: NOTHING;
	}
    __finally {
        if(!NT_SUCCESS(Status)) {
            if(NULL != DeviceObject) {
                if(LinkCreated) {
                    VhdRemoveDriveLink(&pExt->DosDeviceName);
                }
                IoDeleteDevice(DeviceObject);
                DeviceObject = NULL;
            }
        }
    }

	return Status;
}

_Check_return_
NTSTATUS
VhdOpenVolume(
              _In_ PVHDDISKEXT pExt,
              _In_ PNXRMVHDMOUNTDRIVE Mount
              )
{
	NTSTATUS                    Status = STATUS_SUCCESS;
	UNICODE_STRING              HostFileName = {0};
	
	__try {

		RtlInitUnicodeString(&HostFileName, Mount->HostFileName);

		if(Mount->BytesPerSector != VHDFMT_SECTOR_SIZE) {
            try_return(Status = STATUS_INVALID_PARAMETER);
		}
		pExt->HostBytesPerSector = Mount->BytesPerSector;

        Status = VhdOpen(&HostFileName, &pExt->VhdFileHandle);
        if(!NT_SUCCESS(Status)) {
            try_return(Status);
        }

		pExt->DiskLength.QuadPart = VhdGetVolumeSize(pExt->VhdFileHandle);
		pExt->NumberOfCylinders.QuadPart= (pExt->DiskLength.QuadPart/pExt->BytesPerSector); 

try_exit: NOTHING;		
	}
    __finally {

        if(!NT_SUCCESS(Status)) {
            if(pExt->VhdFileHandle != NULL) {
                VhdClose(pExt->VhdFileHandle);
                pExt->VhdFileHandle = NULL;
            }
        }
    }

	return Status;
}

_Check_return_
NTSTATUS
VhdRemoveDriveLink(
                   _In_ PCUNICODE_STRING SymbolicLink
                   )
{
	NTSTATUS Status = STATUS_SUCCESS;

	Status = IoDeleteSymbolicLink((PUNICODE_STRING)SymbolicLink);

	return Status;
}

_Check_return_
NTSTATUS
VhdCreateDriveLink(
                   _In_ PCUNICODE_STRING DeviceName,
                   _In_ PCUNICODE_STRING SymbolicLink
                   )
{
	NTSTATUS Status =STATUS_SUCCESS;

	Status = IoCreateSymbolicLink((PUNICODE_STRING)SymbolicLink, (PUNICODE_STRING)DeviceName);

	return Status;
}

_Check_return_
NTSTATUS
VhdOpenFsVolume(
                _In_ PVHDDISKEXT pExt, 
                _Out_ PFILE_OBJECT* FileObject
                )
{
	NTSTATUS			Status = STATUS_SUCCESS;
	OBJECT_ATTRIBUTES	ObjectAttributes ={0};
	IO_STATUS_BLOCK		IoStatus = {0};
	HANDLE				VolumeHandle = NULL;

	__try {

		InitializeObjectAttributes(&ObjectAttributes,
								   &pExt->NtDeviceName,
								   OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
								   NULL,
								   NULL);

		Status = ZwCreateFile(&VolumeHandle,
							  SYNCHRONIZE | GENERIC_READ,
							  &ObjectAttributes,
							  &IoStatus,
							  NULL,
							  FILE_ATTRIBUTE_NORMAL,
							  FILE_SHARE_READ | FILE_SHARE_WRITE,
							  FILE_OPEN,
							  FILE_SYNCHRONOUS_IO_NONALERT,
							  NULL,
							  0);

		if(!NT_SUCCESS(Status)) {
			goto try_exit;
        }

		Status = ObReferenceObjectByHandle(VolumeHandle,
										   FILE_READ_DATA,
										   NULL,
										   KernelMode,
										   (PVOID*)FileObject,
										   NULL);

try_exit: NOTHING;
	}
    __finally {

        if(VolumeHandle != NULL) {
            ZwClose(VolumeHandle);
            VolumeHandle = NULL;
        }
    }

	return Status;
}

VOID
CloseFsVolume(
              _In_ PFILE_OBJECT FileObject
              )
{
	ObDereferenceObject(FileObject);
}

_Check_return_
NTSTATUS
VhdFsctlCall(
             _In_ PFILE_OBJECT FileObject,
             _In_ LONG IoControlCode,
             _In_opt_ PVOID InputBuffer,
             _In_ ULONG InputBufferSize,
             _Out_opt_ PVOID OutputBuffer,
             _In_ ULONG OutputBufferSize
             )
{
	NTSTATUS            Status = STATUS_SUCCESS;
	IO_STATUS_BLOCK     IoStatus = {0};
	PIRP                Irp = NULL;
	KEVENT              WaitEvent = {0};
	PIO_STACK_LOCATION  IrpSp = NULL;

	PDEVICE_OBJECT DeviceObject = IoGetRelatedDeviceObject(FileObject);

	__try {

		KeInitializeEvent(&WaitEvent,NotificationEvent,FALSE);

		Irp = IoBuildDeviceIoControlRequest(IoControlCode,
											DeviceObject,
											InputBuffer,
											InputBufferSize,
											OutputBuffer,
											OutputBufferSize,
											FALSE,
											&WaitEvent,
											&IoStatus);

		if(Irp == NULL) {
			Status = STATUS_INSUFFICIENT_RESOURCES;
			goto try_exit;
		}

		IrpSp = IoGetNextIrpStackLocation(Irp);

		IrpSp->MajorFunction    = IRP_MJ_FILE_SYSTEM_CONTROL;
		IrpSp->MinorFunction    = IRP_MN_USER_FS_REQUEST;
		IrpSp->FileObject       = FileObject;

		Status = IoCallDriver(DeviceObject,Irp);
		if(Status == STATUS_PENDING) {
			KeWaitForSingleObject(&WaitEvent, Executive, KernelMode, FALSE, NULL);
			Status = IoStatus.Status;
		}

try_exit: NOTHING;
	}
    __finally {
        ; // NOTHING;
    }

	return Status;
}

VOID
VhdSleep(
         _In_ ULONG MilliSeconds
         )
{
	LARGE_INTEGER duetime = {0};
	duetime.QuadPart = (__int64) (MilliSeconds * -10000);	
	KeDelayExecutionThread(KernelMode, FALSE, &duetime);
	return;
}