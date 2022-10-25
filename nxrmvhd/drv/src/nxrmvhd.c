

#include <ntifs.h>
#include <Ntdddisk.h>
#include <Ntddvol.h>
#include <Ntstrsafe.h>

#include <Mountmgr.h>
#include <Mountdev.h>

#include <nudf\shared\vhddef.h>

#include "nxrmvhd.h"
#include "vhdfile.h"
#include "vhddev.h"
#include "vhdutil.h"



//
//  Global Data
//
VHDMGR      _VhdMgr;


//
//  Declare Local Routines
//

_Function_class_(DRIVER_INITIALIZE)
NTSTATUS
DriverEntry(
            _In_ PDRIVER_OBJECT DriverObject,
            _In_ PUNICODE_STRING RegistryPath
            );

_IRQL_requires_(PASSIVE_LEVEL)
VOID
VhdUnload(
	      _In_ PDRIVER_OBJECT DriverObject
	      );

_IRQL_requires_(PASSIVE_LEVEL)
VOID
IoThreadProc(
             _In_ PVOID Arg
             );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
ProcessDiskIo(
              _In_ PIRP Irp
              );

//
//  Assign text sections for each routine.
//
#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, VhdUnload)
#endif


_Function_class_(DRIVER_INITIALIZE)
NTSTATUS
DriverEntry(
            _In_ PDRIVER_OBJECT DriverObject,
            _In_ PUNICODE_STRING RegistryPath
            )
{
	UNICODE_STRING		NtDeviceName  = {0, 0, NULL};
	UNICODE_STRING		DosDeviceName = {0, 0, NULL};

	PDEVICE_OBJECT		DeviceObject = NULL;

	BOOLEAN				ReleaseShutdownNotification = FALSE;
	BOOLEAN				ReleaseSymbolicLink = FALSE;
	BOOLEAN				ReleaseDiskListLock = FALSE;

	OBJECT_ATTRIBUTES	IoThreadAttributes = {0};
	HANDLE				IoThreadHandle = NULL;

    NTSTATUS            Status = STATUS_SUCCESS;
	ULONG               i = 0;


    PAGED_CODE();


    UNREFERENCED_PARAMETER(RegistryPath);
      
    RtlZeroMemory(&_VhdMgr, sizeof(VHDMGR));
    
    __try {

	    //
	    // Creating Control Device
	    //
        RtlInitUnicodeString(&NtDeviceName, NXRMVHD_DEVICE_NAME_W);
        Status = IoCreateDevice(DriverObject,
							    0,								// DeviceExtensionSize
							    &NtDeviceName,				    // DeviceName
							    FILE_DEVICE_NXRMVHD,			// DeviceType
							    FILE_DEVICE_SECURE_OPEN,		// DeviceCharacteristics
							    TRUE,							// Exclusive
							    &DeviceObject);				    // [OUT]


	    if(!NT_SUCCESS(Status)) {
		    goto try_exit;
	    }

	    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
		    DriverObject->MajorFunction[i] = VhdDispatchDeviceControl;
	    }
#ifdef _DEBUG
	    DriverObject->DriverUnload = VhdUnload;
#else
	    DriverObject->DriverUnload = NULL;
#endif

        Status = IoRegisterShutdownNotification(DeviceObject);
        if(!NT_SUCCESS(Status)) {
		    goto try_exit;
	    }
        ReleaseShutdownNotification = TRUE;

        RtlInitUnicodeString(&DosDeviceName, NXRMVHD_DOS_DEVICE_NAME_W);
        Status = IoCreateSymbolicLink(&DosDeviceName, &NtDeviceName);
        if(!NT_SUCCESS(Status)) {
		    goto try_exit;
	    }
	    ReleaseSymbolicLink = TRUE;
        

	    //
	    // Initialize VhdMgr
	    //
	    _VhdMgr.VhdMgrDevice		= DeviceObject;
	    _VhdMgr.ActiveProcessors	= GetCPUCount();

	    InitializeListHead(&_VhdMgr.IoQueue);
	    KeInitializeSemaphore(&_VhdMgr.IoQueueSem, 0, MAXLONG);
	    KeInitializeSpinLock(&_VhdMgr.IoQueueLock);
	    KeInitializeEvent(&_VhdMgr.IoQueueStopEvent, NotificationEvent, FALSE);
        InitializeListHead(&_VhdMgr.VhdDiskList);
	    Status = ExInitializeResourceLite(&_VhdMgr.VhdDiskLock);
        if(!NT_SUCCESS(Status)) {
		    goto try_exit;
	    }
        ReleaseDiskListLock = TRUE;


	    //
	    // Create IO thread
	    //
	    // IO thread has to be created at the end of DriverEntry
	    //
	    InitializeObjectAttributes(&IoThreadAttributes,
							       NULL,
							       OBJ_KERNEL_HANDLE,
							       NULL,
							       NULL);

	    for(i = 0; i < min(_VhdMgr.ActiveProcessors*2, VHD_MAX_THREADS); i++) {

		    Status = PsCreateSystemThread(&IoThreadHandle,
									      THREAD_ALL_ACCESS,
									      &IoThreadAttributes,
									      NULL,
									      &_VhdMgr.IoThreadId[i],
									      IoThreadProc,
									      NULL);
		    if(!NT_SUCCESS(Status)) {
			    goto try_exit;
		    }

		    ObReferenceObjectByHandle(IoThreadHandle,
								      THREAD_ALL_ACCESS,
								      *PsThreadType,
								      KernelMode,
								      (PVOID*)&_VhdMgr.IoThread[i],
								      NULL);

		    ZwClose(IoThreadHandle);
		    IoThreadHandle = NULL;
	    }

        // Transfer Ownership
        DeviceObject = NULL;

try_exit: NOTHING;
    }
    __finally {

        if(!NT_SUCCESS(Status)) {
              
            if(ReleaseShutdownNotification && NULL!=DeviceObject) {
                IoUnregisterShutdownNotification(DeviceObject);
            }

            if(ReleaseSymbolicLink) {
                IoDeleteSymbolicLink(&NtDeviceName);
            }

            if(ReleaseDiskListLock) {
                ExDeleteResourceLite(&_VhdMgr.VhdDiskLock);
            }

            if(DeviceObject) {
                IoDeleteDevice(DeviceObject);
            }
        }
    }

    return Status;
}


_IRQL_requires_(PASSIVE_LEVEL)
VOID
VhdUnload(
	      _In_ PDRIVER_OBJECT DriverObject
	      )
{
	UNICODE_STRING DosDeviceName = {0};
	PVOID WaitThreads[VHD_MAX_THREADS] = {0};	
	ULONG WaitThreadsCount = 0;
    KWAIT_BLOCK WaitBlocks[VHD_MAX_THREADS] = { 0 };
	ULONG i = 0;

    PAGED_CODE();
    UNREFERENCED_PARAMETER(DriverObject);

    RtlZeroMemory(WaitThreads, sizeof(WaitThreads));
    RtlZeroMemory(WaitBlocks, sizeof(WaitBlocks));
    
	//
    // Free any resources
    //

    // Remove All Active Disks
    ExEnterCriticalRegionAndAcquireResourceExclusive(&_VhdMgr.VhdDiskLock);
    while(!IsListEmpty(&_VhdMgr.VhdDiskList)) {
        PLIST_ENTRY pEntry = NULL;
        PVHDDISKEXT pExt = NULL;
        pEntry = RemoveHeadList(&_VhdMgr.VhdDiskList);
        pExt = CONTAINING_RECORD(pEntry, VHDDISKEXT, Link);
        VhdCloseDeviceObject(pExt->VolumeDevice);
        pExt = NULL;
        --_VhdMgr.VhdDiskCount;
    }
    ExReleaseResourceAndLeaveCriticalRegion(&_VhdMgr.VhdDiskLock);
	
    // Stop all working i/o threads
	for(i=0; i< VHD_MAX_THREADS; i++) {
		if(_VhdMgr.IoThread[i]) {
			WaitThreads[WaitThreadsCount] = _VhdMgr.IoThread[i];
			WaitThreadsCount++;
		}
	}
	KeSetEvent(&_VhdMgr.IoQueueStopEvent, IO_NO_INCREMENT, TRUE);
	KeWaitForMultipleObjects(WaitThreadsCount,
							 WaitThreads,
							 WaitAll,
							 Executive,
							 KernelMode,
							 FALSE,
							 NULL,
							 WaitBlocks);
    //
    // Delete the symbolic link
    //
    RtlInitUnicodeString(&DosDeviceName, NXRMVHD_DOS_DEVICE_NAME_W);
    IoDeleteSymbolicLink(&DosDeviceName);
    ExDeleteResourceLite(&_VhdMgr.VhdDiskLock);

    //
    // Delete the device object
    //
    IoDeleteDevice(DriverObject->DeviceObject);
}


_IRQL_requires_(PASSIVE_LEVEL)
VOID
IoThreadProc(
             _In_ PVOID Arg
             )
{
	NTSTATUS    Status = STATUS_SUCCESS;
    PLIST_ENTRY ListEntry = NULL;
	PIRP	    Irp = NULL;
	PVOID	    WaitObjects[2] = {&_VhdMgr.IoQueueSem, &_VhdMgr.IoQueueStopEvent};

    PAGED_CODE();

    UNREFERENCED_PARAMETER(Arg);

	KeSetPriorityThread (KeGetCurrentThread(), LOW_REALTIME_PRIORITY);

	do {

		Status = KeWaitForMultipleObjects(sizeof(WaitObjects)/sizeof(PVOID),
										  WaitObjects,
										  WaitAny,
										  Executive,
										  KernelMode,
										  FALSE,
										  NULL,
										  NULL);

		if(Status == STATUS_WAIT_0) {

			ListEntry = ExInterlockedRemoveHeadList(&_VhdMgr.IoQueue, &_VhdMgr.IoQueueLock);
			Irp = CONTAINING_RECORD(ListEntry, IRP, Tail.Overlay.ListEntry);
			Status = ProcessDiskIo(Irp);
		}
		else if(Status == STATUS_WAIT_1) {
			break;
		}
		else {
			continue;
		}

	} while (TRUE);

	PsTerminateSystemThread(Status);
}


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
ProcessDiskIo(
              _In_ PIRP Irp
              )
{
	NTSTATUS Status = STATUS_SUCCESS;

	PIO_STACK_LOCATION	IrpSp = NULL;
	PVOID				IoBuffer = NULL;
	ULONG				InputBufferLength = 0, OutputBufferLength = 0;
	ULONG				IoControlCode = 0;
	PVHDDISKEXT	        pExt = NULL;
    ULONG               OutLength = 0;


    PAGED_CODE();

	IrpSp = IoGetCurrentIrpStackLocation(Irp);
	pExt = (PVHDDISKEXT)(IrpSp->DeviceObject->DeviceExtension);

	//
	// Get the pointer to the input/output buffer and it's length
	//
	if(Irp->MdlAddress)
		IoBuffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, HighPagePriority);
	else
		IoBuffer = Irp->AssociatedIrp.SystemBuffer;

	InputBufferLength	= IrpSp->Parameters.DeviceIoControl.InputBufferLength;
	OutputBufferLength	= IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
	IoControlCode		= IrpSp->Parameters.DeviceIoControl.IoControlCode;

	//
	// Set return length default to 0
	//
	Irp->IoStatus.Information = 0;

	__try {

		switch(IrpSp->MajorFunction)
		{
		case IRP_MJ_READ:

			if(pExt->VhdFileHandle != NULL) {

                Status = VhdRead(pExt->VhdFileHandle,
                                 IrpSp->Parameters.Read.ByteOffset.QuadPart,
                                 IrpSp->Parameters.Read.Length,
                                 (PUCHAR)IoBuffer,
                                 &OutLength
                                 );
                Irp->IoStatus.Status = Status;
                Irp->IoStatus.Information = OutLength;
				ASSERT(NT_SUCCESS(Status));
			}
			else {
				Status = STATUS_INVALID_HANDLE;
				Irp->IoStatus.Status = Status;
			}
			break;

		case IRP_MJ_WRITE:

			if(pExt->VhdFileHandle != NULL) {

                Status = VhdWrite(pExt->VhdFileHandle,
                                  IrpSp->Parameters.Write.ByteOffset.QuadPart,
                                  IrpSp->Parameters.Write.Length,
                                  (PUCHAR)IoBuffer,
                                  &OutLength
                                  );
                Irp->IoStatus.Status = Status;
                Irp->IoStatus.Information = OutLength;
				ASSERT(NT_SUCCESS(Status));
			}
			else {
				Status = STATUS_INVALID_HANDLE;
				Irp->IoStatus.Status = Status;
			}
			break;

		case IRP_MJ_DEVICE_CONTROL:
			
			switch(IrpSp->Parameters.DeviceIoControl.IoControlCode)
			{
			case IOCTL_MOUNTDEV_QUERY_DEVICE_NAME:

				do  {

					if(!ValidateIoBufferSize (Irp, sizeof(MOUNTDEV_NAME), ValidateOutput)) {
						Irp->IoStatus.Information	= sizeof(MOUNTDEV_NAME);
						Irp->IoStatus.Status		= STATUS_BUFFER_OVERFLOW;
						break;
					}
					else {

						ULONG			OutLength = 0;
						PMOUNTDEV_NAME	OutputBuffer = (PMOUNTDEV_NAME) Irp->AssociatedIrp.SystemBuffer;

						OutputBuffer->NameLength = pExt->NtDeviceName.Length;
						OutLength = pExt->NtDeviceName.Length + sizeof(USHORT);

						if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength < OutLength) {
							Irp->IoStatus.Information	= sizeof (MOUNTDEV_NAME);
							Irp->IoStatus.Status		= STATUS_BUFFER_OVERFLOW;
							break;
						}

						RtlCopyMemory((PCHAR)OutputBuffer->Name, pExt->NtDeviceName.Buffer, pExt->NtDeviceName.Length);
						Irp->IoStatus.Status = STATUS_SUCCESS;
						Irp->IoStatus.Information = OutLength;
					}
				
				} while (FALSE);
				break;

			case IOCTL_MOUNTDEV_QUERY_UNIQUE_ID:
				
				do  {

					if(!ValidateIoBufferSize(Irp, sizeof(MOUNTDEV_UNIQUE_ID), ValidateOutput)) {
						Irp->IoStatus.Information	= sizeof(MOUNTDEV_UNIQUE_ID);
						Irp->IoStatus.Status		= STATUS_BUFFER_OVERFLOW;						
						break;
					}
					else {

						ULONG				OutLength = 0;
						UCHAR				VolId[128] = {0};
						PMOUNTDEV_UNIQUE_ID OutputBuffer = (PMOUNTDEV_UNIQUE_ID)Irp->AssociatedIrp.SystemBuffer;

                        RtlZeroMemory(VolId, 128);
                        RtlStringCbPrintfA((CHAR*)VolId, 128, "%s%d", NXRMVHD_UNIQUE_ID_PREFIX_A, pExt->DiskId);

						OutputBuffer->UniqueIdLength = (USHORT)(strlen((CHAR*)VolId));   // Prefix + DriveLetter(1)
						OutLength = (ULONG)(OutputBuffer->UniqueIdLength + sizeof(USHORT));

						if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength < OutLength) {
							Irp->IoStatus.Information	= sizeof(MOUNTDEV_UNIQUE_ID);
							Irp->IoStatus.Status		= STATUS_BUFFER_OVERFLOW;							
							break;
						}

						RtlCopyMemory((PCHAR)OutputBuffer->UniqueId, VolId, OutputBuffer->UniqueIdLength);
						Irp->IoStatus.Status = STATUS_SUCCESS;
						Irp->IoStatus.Information = OutLength;
					}

				} while (FALSE);
				break;

			case IOCTL_MOUNTDEV_QUERY_SUGGESTED_LINK_NAME:

				do 
				{
					ULONG							OutLength = 0;
					PMOUNTDEV_SUGGESTED_LINK_NAME	OutputBuffer = (PMOUNTDEV_SUGGESTED_LINK_NAME)Irp->AssociatedIrp.SystemBuffer;

					if(!ValidateIoBufferSize(Irp, sizeof(MOUNTDEV_SUGGESTED_LINK_NAME), ValidateOutput)) {
						Irp->IoStatus.Status		= STATUS_INVALID_PARAMETER;
						Irp->IoStatus.Information	= 0;						
						break; 
					}

					OutLength = FIELD_OFFSET(MOUNTDEV_SUGGESTED_LINK_NAME, Name) + pExt->DosDeviceName.Length;
					OutputBuffer->UseOnlyIfThereAreNoOtherLinks = FALSE;
					OutputBuffer->NameLength = pExt->DosDeviceName.Length;

					if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength < OutLength) {
						Irp->IoStatus.Information	= sizeof (MOUNTDEV_SUGGESTED_LINK_NAME);
						Irp->IoStatus.Status		= STATUS_BUFFER_OVERFLOW;
						break;
					}

					RtlCopyMemory((PCHAR)OutputBuffer->Name, pExt->DosDeviceName.Buffer, pExt->DosDeviceName.Length);
					Irp->IoStatus.Status		= STATUS_SUCCESS;
					Irp->IoStatus.Information	= OutLength;
				} while (FALSE);
				break;

			case IOCTL_DISK_GET_MEDIA_TYPES:
			case IOCTL_DISK_GET_DRIVE_GEOMETRY:

				do {

					if(!ValidateIoBufferSize(Irp, sizeof(DISK_GEOMETRY), ValidateOutput)) {
						Irp->IoStatus.Information	= sizeof(DISK_GEOMETRY);
						Irp->IoStatus.Status		= STATUS_BUFFER_OVERFLOW;
						break;
					}
					else {

						PDISK_GEOMETRY OutputBuffer = (PDISK_GEOMETRY) Irp->AssociatedIrp.SystemBuffer;

						OutputBuffer->MediaType				= pExt->Removable ? RemovableMedia : FixedMedia;
						OutputBuffer->Cylinders.QuadPart	= pExt->NumberOfCylinders.QuadPart;
						OutputBuffer->TracksPerCylinder		= pExt->TracksPerCylinder;
						OutputBuffer->SectorsPerTrack		= pExt->SectorsPerTrack;
						OutputBuffer->BytesPerSector		= pExt->BytesPerSector;
						
						Irp->IoStatus.Status		= STATUS_SUCCESS;
						Irp->IoStatus.Information	= sizeof (DISK_GEOMETRY);
					}

				} while (FALSE);
				break;

			case IOCTL_DISK_GET_PARTITION_INFO:

				do {

					if(!ValidateIoBufferSize(Irp, sizeof(PARTITION_INFORMATION), ValidateOutput)) {
						Irp->IoStatus.Information	= sizeof(PARTITION_INFORMATION);
						Irp->IoStatus.Status		= STATUS_BUFFER_OVERFLOW;
						break;
					}
					else {

						PPARTITION_INFORMATION OutputBuffer = (PPARTITION_INFORMATION)Irp->AssociatedIrp.SystemBuffer;

						OutputBuffer->PartitionType				= (UCHAR)pExt->PartitionType;
						OutputBuffer->BootIndicator				= FALSE;
						OutputBuffer->RecognizedPartition		= TRUE;
						OutputBuffer->RewritePartition			= FALSE;
						OutputBuffer->StartingOffset.QuadPart	= pExt->BytesPerSector;
						OutputBuffer->PartitionLength.QuadPart	= pExt->DiskLength.QuadPart;
						OutputBuffer->HiddenSectors				= 0;

						Irp->IoStatus.Status		= STATUS_SUCCESS;
						Irp->IoStatus.Information	= sizeof(PARTITION_INFORMATION);
					}

				} while (FALSE);
				break;

			case IOCTL_DISK_GET_PARTITION_INFO_EX:

				do  {

					if(!ValidateIoBufferSize(Irp, sizeof(PARTITION_INFORMATION_EX), ValidateOutput)) {
						Irp->IoStatus.Information	= sizeof(PARTITION_INFORMATION_EX);
						Irp->IoStatus.Status		= STATUS_BUFFER_OVERFLOW;
						break;
					}
					else {

						PPARTITION_INFORMATION_EX OutputBuffer = (PPARTITION_INFORMATION_EX)Irp->AssociatedIrp.SystemBuffer;

						OutputBuffer->PartitionStyle			= PARTITION_STYLE_MBR;
						OutputBuffer->RewritePartition			= FALSE;
						OutputBuffer->StartingOffset.QuadPart	= pExt->BytesPerSector;
						OutputBuffer->PartitionLength.QuadPart	= pExt->DiskLength.QuadPart;
						OutputBuffer->Mbr.PartitionType			= (UCHAR)pExt->PartitionType;
						OutputBuffer->Mbr.BootIndicator			= FALSE;
						OutputBuffer->Mbr.RecognizedPartition	= TRUE;
						OutputBuffer->Mbr.HiddenSectors			= 0;
						
						Irp->IoStatus.Status		= STATUS_SUCCESS;
						Irp->IoStatus.Information	= sizeof(PARTITION_INFORMATION_EX);
					}

				} while (FALSE);
				break;

			case IOCTL_DISK_GET_DRIVE_LAYOUT:
				
				do {

					if(!ValidateIoBufferSize(Irp, sizeof(DRIVE_LAYOUT_INFORMATION), ValidateOutput)) {
						Irp->IoStatus.Information	= sizeof(DRIVE_LAYOUT_INFORMATION);
						Irp->IoStatus.Status		= STATUS_BUFFER_OVERFLOW;
						break;
					}
					else {

						PDRIVE_LAYOUT_INFORMATION OutputBuffer = (PDRIVE_LAYOUT_INFORMATION)Irp->AssociatedIrp.SystemBuffer;

						OutputBuffer->PartitionCount	= 1;
						OutputBuffer->Signature			= 0;

						OutputBuffer->PartitionEntry->PartitionType				= (UCHAR)pExt->PartitionType;
						OutputBuffer->PartitionEntry->BootIndicator				= FALSE;
						OutputBuffer->PartitionEntry->RecognizedPartition		= TRUE;
						OutputBuffer->PartitionEntry->RewritePartition			= FALSE;
						OutputBuffer->PartitionEntry->StartingOffset.QuadPart	= pExt->BytesPerSector;
						OutputBuffer->PartitionEntry->PartitionLength.QuadPart	= pExt->DiskLength.QuadPart;
						OutputBuffer->PartitionEntry->HiddenSectors				= 0;

						Irp->IoStatus.Status		= STATUS_SUCCESS;
						Irp->IoStatus.Information	= sizeof(PARTITION_INFORMATION);
					}

				} while (FALSE);
				break;

			case IOCTL_DISK_GET_LENGTH_INFO:

				do  {

					if(!ValidateIoBufferSize(Irp, sizeof(GET_LENGTH_INFORMATION), ValidateOutput)) {
						Irp->IoStatus.Status		= STATUS_BUFFER_OVERFLOW;
						Irp->IoStatus.Information	= sizeof(GET_LENGTH_INFORMATION);
						break;
					}
					else {

						PGET_LENGTH_INFORMATION OutputBuffer = (PGET_LENGTH_INFORMATION)Irp->AssociatedIrp.SystemBuffer;
						OutputBuffer->Length.QuadPart = pExt->DiskLength.QuadPart;						
						Irp->IoStatus.Status		= STATUS_SUCCESS;
						Irp->IoStatus.Information	= sizeof(GET_LENGTH_INFORMATION);
					}

				} while (FALSE);
				break;

			case IOCTL_DISK_VERIFY:				
				Irp->IoStatus.Status		= STATUS_SUCCESS;
				Irp->IoStatus.Information	= 0;
				break;

			case IOCTL_DISK_CHECK_VERIFY:
			case IOCTL_STORAGE_CHECK_VERIFY:				
				Irp->IoStatus.Status		= STATUS_SUCCESS;
				Irp->IoStatus.Information	= 0;
				if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength >= sizeof(ULONG)) {
					*((ULONG *)Irp->AssociatedIrp.SystemBuffer) = 0;
					Irp->IoStatus.Information = sizeof (ULONG);
				}
				break;

			case IOCTL_VOLUME_GET_GPT_ATTRIBUTES:

				do  {

					if(!ValidateIoBufferSize(Irp, sizeof(VOLUME_GET_GPT_ATTRIBUTES_INFORMATION), ValidateOutput)) {
						Irp->IoStatus.Status		= STATUS_BUFFER_OVERFLOW;
						Irp->IoStatus.Information	= sizeof(VOLUME_GET_GPT_ATTRIBUTES_INFORMATION);
						break;
					}
					else {

						VOLUME_GET_GPT_ATTRIBUTES_INFORMATION *pGPTInfo = (VOLUME_GET_GPT_ATTRIBUTES_INFORMATION*)Irp->AssociatedIrp.SystemBuffer;

						pGPTInfo->GptAttributes	= GPT_BASIC_DATA_ATTRIBUTE_NO_DRIVE_LETTER;
						Irp->IoStatus.Status		= STATUS_SUCCESS;
						Irp->IoStatus.Information	= sizeof(VOLUME_GET_GPT_ATTRIBUTES_INFORMATION);
					}

				} while (FALSE);
				break;

			case IOCTL_DISK_IS_WRITABLE:
				Irp->IoStatus.Status		= STATUS_SUCCESS;
				Irp->IoStatus.Information	= 0;
				break;

			case IOCTL_VOLUME_ONLINE:
				Irp->IoStatus.Status		= STATUS_SUCCESS;
				Irp->IoStatus.Information	= 0;
				break;

			case IOCTL_MOUNTDEV_LINK_DELETED: 
			case IOCTL_MOUNTDEV_LINK_CREATED:
				Irp->IoStatus.Status		= STATUS_SUCCESS;
				Irp->IoStatus.Information	= 0;
				break;

			case IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS:
				Irp->IoStatus.Status		= STATUS_INVALID_DEVICE_REQUEST;
				Irp->IoStatus.Information	= 0;
				break;

			case IOCTL_STORAGE_GET_HOTPLUG_INFO:

				do {

					if(!ValidateIoBufferSize(Irp, sizeof(STORAGE_HOTPLUG_INFO), ValidateOutput)) {
						Irp->IoStatus.Status		= STATUS_BUFFER_OVERFLOW;
						Irp->IoStatus.Information	= sizeof(STORAGE_HOTPLUG_INFO);
						break;
					}
					else {

						PSTORAGE_HOTPLUG_INFO pHotPlugInfo = (PSTORAGE_HOTPLUG_INFO)Irp->AssociatedIrp.SystemBuffer;

						pHotPlugInfo->Size						= sizeof(STORAGE_HOTPLUG_INFO);
						pHotPlugInfo->MediaRemovable			= FALSE;
						pHotPlugInfo->MediaHotplug				= FALSE;
						pHotPlugInfo->DeviceHotplug				= pExt->Removable?TRUE:FALSE;
						pHotPlugInfo->WriteCacheEnableOverride	= FALSE;

						Irp->IoStatus.Status		= STATUS_SUCCESS;
						Irp->IoStatus.Information	= sizeof(STORAGE_HOTPLUG_INFO);
					}

				} while (FALSE);
				break;

			default:
				Irp->IoStatus.Status		= STATUS_INVALID_DEVICE_REQUEST;
				Irp->IoStatus.Information	= 0;
				break;
			}
			break;
		
		default:
			Irp->IoStatus.Status		= STATUS_INVALID_DEVICE_REQUEST;
			Irp->IoStatus.Information	= 0;
			break;
		}

	}
    __finally {
        ; // NOTHING
    }

	Status = Irp->IoStatus.Status;
	IoReleaseRemoveLock(&pExt->IoRemoveLock, Irp);
	DecrementOutstandingIoCount(pExt);
	IoCompleteRequest(Irp,NT_SUCCESS(Status)?IO_DISK_INCREMENT:IO_NO_INCREMENT);

	return Status;
}