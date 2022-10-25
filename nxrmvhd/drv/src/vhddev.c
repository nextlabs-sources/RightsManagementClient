
#include <ntifs.h>


#include "nxrmvhd.h"
#include "vhdutil.h"
#include "vhddev.h"




_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdDispatchMountDisk(
	                 _In_ PDEVICE_OBJECT DeviceObject,
	                 _In_ PIRP Irp
	                 );

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdDispatchUnmountDisk(
	                   _In_ PDEVICE_OBJECT DeviceObject,
	                   _In_ PIRP Irp
	                   );

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdDispatchQueryDisk(
	                 _In_ PDEVICE_OBJECT DeviceObject,
	                 _In_ PIRP Irp
	                 );

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdDiskDispatchDeviceControl(
	                         _In_ PDEVICE_OBJECT DeviceObject,
	                         _In_ PIRP Irp
	                         );


//
//  Assign text sections for each routine.
//
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, VhdDispatchDeviceControl)
#pragma alloc_text(PAGE, VhdDispatchMountDisk)
#pragma alloc_text(PAGE, VhdDispatchUnmountDisk)
#pragma alloc_text(PAGE, VhdDispatchQueryDisk)
#pragma alloc_text(PAGE, VhdDiskDispatchDeviceControl)
#endif


__forceinline
NTSTATUS
InterCompleteIrp(
	             _In_ PIRP Irp,
	             _In_ NTSTATUS Status,
	             _In_ ULONG_PTR Information
                 )
{
	Irp->IoStatus.Status = Status;
	Irp->IoStatus.Information = Information;	
	IoCompleteRequest(Irp,IO_NO_INCREMENT);	
	return Status;
}

__forceinline
NTSTATUS
InterCompleteDiskIoIrp(
	                   _In_ PIRP Irp,
	                   _In_ NTSTATUS Status,
	                   _In_ ULONG_PTR Information
                       )
{
	Irp->IoStatus.Status		= Status;
	Irp->IoStatus.Information	= Information;
	IoCompleteRequest(Irp, NT_SUCCESS(Status) ? IO_DISK_INCREMENT : IO_NO_INCREMENT);
	return Status;
}

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdDispatchDeviceControl(
	                     _In_ PDEVICE_OBJECT DeviceObject,
	                     _In_ PIRP Irp
	                     )
{
	NTSTATUS			Status = STATUS_UNSUCCESSFUL;
    PIO_STACK_LOCATION	IrpSp = NULL;
    PVOID				IoBuffer = NULL;
    ULONG				InputBufferLength = 0;
	ULONG				OutputBufferLength = 0;
	ULONG				IoControlCode = 0;

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    Irp->IoStatus.Information = 0;

    //
    // Get the pointer to the input/output buffer and it's length
    //
    IoBuffer			= Irp->AssociatedIrp.SystemBuffer;
    InputBufferLength	= IrpSp->Parameters.DeviceIoControl.InputBufferLength;
    OutputBufferLength	= IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
    IoControlCode		= IrpSp->Parameters.DeviceIoControl.IoControlCode;

	//
	// Set return length default to 0
	//
	Irp->IoStatus.Information = 0;

    NT_ASSERT(DeviceObject);

    if(DeviceObject == _VhdMgr.VhdMgrDevice) {

        switch(IrpSp->MajorFunction)
        {
        case IRP_MJ_CREATE:
            Status = STATUS_SUCCESS;
            Irp->IoStatus.Status = Status;
            Irp->IoStatus.Information = FILE_OPENED;
            break;

        case IRP_MJ_CLEANUP:
            Status = STATUS_SUCCESS;
            Irp->IoStatus.Status = Status;
            break;

        case IRP_MJ_CLOSE:
            Status = STATUS_SUCCESS;
            Irp->IoStatus.Status = Status;
            break;

        case IRP_MJ_DEVICE_CONTROL:

            switch(IoControlCode)
            {
            case IOCTL_NXRMVHD_MOUNT_DISK:
                Status = VhdDispatchMountDisk(DeviceObject, Irp);
                Irp->IoStatus.Status = Status;
                break;

            case IOCTL_NXRMVHD_UNMOUNT_DISK:
                Status = VhdDispatchUnmountDisk(DeviceObject, Irp);
                Irp->IoStatus.Status = Status;
                break;

            case IOCTL_NXRMVHD_QUERY_DISK:
                Status = VhdDispatchQueryDisk(DeviceObject, Irp);
                Irp->IoStatus.Status = Status;
                break;

            default:
                Status = STATUS_SUCCESS;
                Irp->IoStatus.Status = Status;
                break;
            }
            break;

        case IRP_MJ_SHUTDOWN:
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
            Status = STATUS_SUCCESS;
            Irp->IoStatus.Status = Status;

        case IRP_MJ_PNP:
            Status = STATUS_SUCCESS;
            Irp->IoStatus.Status = Status;
            break;

        default:
            break;
        }

        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }
    else {
        //
        // DISK device complete its own IRP
        //
        Status = VhdDiskDispatchDeviceControl(DeviceObject,Irp);
    }

    return Status;
}

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdDispatchMountDisk(
	                 _In_ PDEVICE_OBJECT DeviceObject,
	                 _In_ PIRP Irp
	                 )
{
	NTSTATUS Status = STATUS_SUCCESS;

	PIO_STACK_LOCATION	IrpSp = NULL;
	PVOID				IoBuffer = NULL;
	ULONG				InputBufferLength = 0, OutputBufferLength = 0;
	ULONG				IoControlCode = 0;
	PNXRMVHDMOUNTDRIVE  MountReq = NULL;
	PDEVICE_OBJECT		DiskDeviceObject = NULL;
    PVHDDISKEXT         pExt = NULL;

	IrpSp = IoGetCurrentIrpStackLocation(Irp);

	//
	// Get the pointer to the input/output buffer and it's length
	//
	IoBuffer			= Irp->AssociatedIrp.SystemBuffer;
	InputBufferLength	= IrpSp->Parameters.DeviceIoControl.InputBufferLength;
	OutputBufferLength	= IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
	IoControlCode		= IrpSp->Parameters.DeviceIoControl.IoControlCode;

	//
	// Set return length default to 0
	//
	Irp->IoStatus.Information = 0;

	__try {
        
        MountReq = (PNXRMVHDMOUNTDRIVE)IoBuffer;

        // Check Drive Letter
        if(MountReq->Visible) {

            MountReq->PreferredDriveLetter = FindAvailableDriveLetter(MountReq->PreferredDriveLetter);
            if(0 == MountReq->PreferredDriveLetter) {
                try_return(Status = STATUS_FILE_NOT_AVAILABLE);
            }
        }

		Status = VhdCreateDeviceObject(DeviceObject->DriverObject, &DiskDeviceObject, MountReq);
		if(!NT_SUCCESS(Status)) {
			goto try_exit;
		}

        pExt = (PVHDDISKEXT)DiskDeviceObject->DeviceExtension;

		//
		// Notify mount manager but be careful about its return status
		//
		Status = MountManagerMount(DiskDeviceObject);

try_exit: NOTHING;
	}
    __finally {

        if(!NT_SUCCESS(Status)) {
            VhdCloseDeviceObject(DiskDeviceObject);
            DiskDeviceObject = NULL;
        }
        else {
            PNXRMVHDINFO VhdInf = IoBuffer;
            // Insert to DiskList, and increase DiskCount
            ExEnterCriticalRegionAndAcquireResourceExclusive(&_VhdMgr.VhdDiskLock);
            InsertTailList(&_VhdMgr.VhdDiskList, &((PVHDDISKEXT)DiskDeviceObject->DeviceExtension)->Link);
            ++_VhdMgr.VhdDiskCount;
            ExReleaseResourceAndLeaveCriticalRegion(&_VhdMgr.VhdDiskLock);

            // Set output
            VhdInf->DiskId = pExt->DiskId;
            VhdInf->DriveLetter = pExt->DosDriveLetter;
            VhdInf->Removable = pExt->Removable;
            VhdInf->Visible = pExt->Visible;
            RtlCopyMemory(VhdInf->VolumeName, pExt->NtDeviceNameBuf, VHD_MAX_NAME_LENGTH);
            Irp->IoStatus.Information = sizeof(NXRMVHDINFO);
        }
    }

	return Status;
}

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdDispatchUnmountDisk(
	                   _In_ PDEVICE_OBJECT DeviceObject,
	                   _In_ PIRP Irp
	                   )
{
    NTSTATUS    Status = STATUS_SUCCESS;

    PIO_STACK_LOCATION	IrpSp = NULL;
    PVOID				IoBuffer = NULL;
    ULONG				InputBufferLength = 0, OutputBufferLength = 0;
    ULONG				IoControlCode = 0;


    PAGED_CODE();
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);

    //
    // Get the pointer to the input/output buffer and it's length
    //
    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    IoBuffer = Irp->AssociatedIrp.SystemBuffer;
    InputBufferLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
    OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
    IoControlCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;

    //
    // Set return length default to 0
    //
    Irp->IoStatus.Information = 0;

    try {

        ULONG DiskId = 0;

        if (InputBufferLength < sizeof(ULONG)) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        DiskId = *(ULONG*)IoBuffer;

        // Insert to DiskList, and increase DiskCount
        PVHDDISKEXT pExt = NULL;
        ExEnterCriticalRegionAndAcquireResourceExclusive(&_VhdMgr.VhdDiskLock);
        for (PLIST_ENTRY pEntry = _VhdMgr.VhdDiskList.Flink; pEntry != &_VhdMgr.VhdDiskList; pEntry = pEntry->Flink) {
            pExt = CONTAINING_RECORD(pEntry, VHDDISKEXT, Link);
            if (pExt->DiskId == DiskId) {
                RemoveEntryList(pEntry);
                --_VhdMgr.VhdDiskCount;
                break;
            }
            pExt = NULL;
        }
        ExReleaseResourceAndLeaveCriticalRegion(&_VhdMgr.VhdDiskLock);

        if (pExt != NULL) {
            VhdCloseDeviceObject(pExt->VolumeDevice);
            pExt = NULL;
        }

try_exit: NOTHING;
    }
    finally {
        Irp->IoStatus.Status = Status;
    }

    return Status;
}

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdDispatchQueryDisk(
	                 _In_ PDEVICE_OBJECT DeviceObject,
	                 _In_ PIRP Irp
	                 )
{
    NTSTATUS    Status = STATUS_SUCCESS;

	PIO_STACK_LOCATION	IrpSp = NULL;
	PVOID				IoBuffer = NULL;
	ULONG				InputBufferLength = 0, OutputBufferLength = 0;
	ULONG				IoControlCode = 0;
    ULONG               DiskIdToQuery = 0xFFFFFFFF;

    PAGED_CODE();
    UNREFERENCED_PARAMETER(DeviceObject);

	IrpSp = IoGetCurrentIrpStackLocation(Irp);

	//
	// Get the pointer to the input/output buffer and it's length
	//
	IoBuffer			= Irp->AssociatedIrp.SystemBuffer;
	InputBufferLength	= IrpSp->Parameters.DeviceIoControl.InputBufferLength;
	OutputBufferLength	= IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
	IoControlCode		= IrpSp->Parameters.DeviceIoControl.IoControlCode;

	//
	// Set return length default to 0
	//
	Irp->IoStatus.Information = 0;

    if(InputBufferLength != sizeof(ULONG)) {
        return STATUS_INVALID_PARAMETER;
    }

    if (OutputBufferLength < sizeof(ULONG)) {
        return STATUS_BUFFER_TOO_SMALL;
    }



    DiskIdToQuery = *((PULONG)IoBuffer);

    ASSERT(IoControlCode == IOCTL_NXRMVHD_QUERY_DISK);

    __try {

        RtlZeroMemory(IoBuffer, OutputBufferLength);

        if (0xFFFFFFFF != DiskIdToQuery) {

            BOOLEAN Found = FALSE;

            // Query specified disk
            if (OutputBufferLength < sizeof(NXRMVHDINFO)) {
                *((PULONG)IoBuffer) = sizeof(NXRMVHDINFO);
                Irp->IoStatus.Information = sizeof(ULONG);
                try_return(Status = (OutputBufferLength == sizeof(ULONG)) ? STATUS_SUCCESS : STATUS_BUFFER_TOO_SMALL);
            }

            // Insert to DiskList, and increase DiskCount
            ExEnterCriticalRegionAndAcquireResourceExclusive(&_VhdMgr.VhdDiskLock);
            for (PLIST_ENTRY pEntry = _VhdMgr.VhdDiskList.Flink; pEntry != &_VhdMgr.VhdDiskList; pEntry = pEntry->Flink) {
                PVHDDISKEXT pExt = NULL;
                pExt = CONTAINING_RECORD(pEntry, VHDDISKEXT, Link);
                if (pExt->DiskId == DiskIdToQuery) {
                    PNXRMVHDINFO VhdInfo = (PNXRMVHDINFO)IoBuffer;
                    Found = TRUE;
                    VhdInfo->DiskId = pExt->DiskId;
                    VhdInfo->DriveLetter = pExt->DosDriveLetter;
                    VhdInfo->Removable = pExt->Removable;
                    VhdInfo->Visible = pExt->Visible;
                    RtlCopyMemory(VhdInfo->VolumeName, pExt->NtDeviceNameBuf, sizeof(WCHAR) * VHD_MAX_NAME_LENGTH);
                    Irp->IoStatus.Information = sizeof(NXRMVHDINFO);
                    break;
                }
            }
            ExReleaseResourceAndLeaveCriticalRegion(&_VhdMgr.VhdDiskLock);

            if (!Found) {
                try_return(Status = STATUS_NOT_FOUND);
            }
        }
        else {

            const ULONG ActiveDiskCount = GetActiveDiskCount();
            const ULONG BufferCount = (OutputBufferLength < sizeof(NXRMVHDINFOS)) ? 0 : ((ULONG)((OutputBufferLength - sizeof(NXRMVHDINFOS)) / sizeof(NXRMVHDINFO)) + 1);
            PNXRMVHDINFOS Infos = NULL;


            if (0 == ActiveDiskCount) {
                *((PULONG)IoBuffer) = 0;
                Irp->IoStatus.Information = sizeof(ULONG);
                try_return(Status = STATUS_SUCCESS);
            }

            if (BufferCount < ActiveDiskCount) {
                *((PULONG)IoBuffer) = ActiveDiskCount;
                Irp->IoStatus.Information = sizeof(ULONG);
                try_return(Status = (OutputBufferLength == sizeof(ULONG)) ? STATUS_SUCCESS : STATUS_BUFFER_TOO_SMALL);
            }

            Infos = (PNXRMVHDINFOS)IoBuffer;

            ExEnterCriticalRegionAndAcquireResourceExclusive(&_VhdMgr.VhdDiskLock);
            for (PLIST_ENTRY pEntry = _VhdMgr.VhdDiskList.Flink; pEntry != &_VhdMgr.VhdDiskList; pEntry = pEntry->Flink) {

                PVHDDISKEXT pExt = NULL;
                ULONG       nIndex = 0;

                pExt = CONTAINING_RECORD(pEntry, VHDDISKEXT, Link);
                nIndex = Infos->Count;

                Infos->Infs[nIndex].DiskId = pExt->DiskId;
                Infos->Infs[nIndex].DriveLetter = pExt->DosDriveLetter;
                Infos->Infs[nIndex].Removable = pExt->Removable;
                Infos->Infs[nIndex].Visible = pExt->Visible;
                RtlCopyMemory(Infos->Infs[nIndex].VolumeName, pExt->NtDeviceNameBuf, sizeof(WCHAR) * VHD_MAX_NAME_LENGTH);
                Infos->Count++;

                if (Infos->Count >= BufferCount) {
                    break;
                }
            }
            ExReleaseResourceAndLeaveCriticalRegion(&_VhdMgr.VhdDiskLock);

            Irp->IoStatus.Information = sizeof(NXRMVHDINFOS) + ((Infos->Count - 1) * sizeof(NXRMVHDINFO));
        }

try_exit: NOTHING;
    }
    __finally {
    }

    return Status;
}

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdDiskDispatchDeviceControl(
	                         _In_ PDEVICE_OBJECT DeviceObject,
	                         _In_ PIRP Irp
	                         )
{
	NTSTATUS			Status = STATUS_INVALID_PARAMETER;
	PIO_STACK_LOCATION	IrpSp;
	PVOID				IoBuffer;
	ULONG				InputBufferLength=0, OutputBufferLength=0;
	ULONG				IoControlCode;
	PVHDDISKEXT         pExt = NULL;

	IrpSp = IoGetCurrentIrpStackLocation(Irp);
	Irp->IoStatus.Information = 0;

	//
	// Get the pointer to the input/output buffer and it's length
	//
	IoBuffer			= Irp->AssociatedIrp.SystemBuffer;
	InputBufferLength	= IrpSp->Parameters.DeviceIoControl.InputBufferLength;
	OutputBufferLength	= IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
	IoControlCode		= IrpSp->Parameters.DeviceIoControl.IoControlCode;
	pExt				= (PVHDDISKEXT)DeviceObject->DeviceExtension;

	//
	// Set return length default to 0
	//
	Irp->IoStatus.Information = 0;

	do 
	{
		switch(IrpSp->MajorFunction)
		{
		case IRP_MJ_CLOSE:
		case IRP_MJ_CREATE:
		case IRP_MJ_CLEANUP:			
			Status = InterCompleteIrp(Irp, STATUS_SUCCESS, 0);
			break;

		case IRP_MJ_SHUTDOWN:	
			Status = InterCompleteIrp(Irp, STATUS_SUCCESS, 0);
			break;

		case IRP_MJ_READ:
		case IRP_MJ_WRITE:
		case IRP_MJ_DEVICE_CONTROL:		
			do 
			{
				IncrementOutstandingIoCount(pExt);

				if(pExt->StopPending) {
					Status = STATUS_DEVICE_NOT_READY;
					DecrementOutstandingIoCount(pExt);
					break;
				}

				Status = IoAcquireRemoveLock(&pExt->IoRemoveLock, Irp);
				if(!NT_SUCCESS(Status)) {
					DecrementOutstandingIoCount(pExt);
					break;
				}
				
				IoMarkIrpPending(Irp);

				ExInterlockedInsertTailList(&_VhdMgr.IoQueue, &Irp->Tail.Overlay.ListEntry, &_VhdMgr.IoQueueLock);
				KeReleaseSemaphore(&_VhdMgr.IoQueueSem, IO_DISK_INCREMENT, 1, FALSE);
				Status = STATUS_PENDING;

			} while (FALSE);
			break;

		case IRP_MJ_FLUSH_BUFFERS:
			Status = InterCompleteDiskIoIrp(Irp, STATUS_SUCCESS, 0);
			break;

		case IRP_MJ_PNP:

			if(IrpSp->MinorFunction == IRP_MN_DEVICE_USAGE_NOTIFICATION &&
			   IrpSp->Parameters.UsageNotification.Type == DeviceUsageTypePaging &&
			   IrpSp->Parameters.UsageNotification.InPath) {

				Status = InterCompleteIrp(Irp, STATUS_UNSUCCESSFUL, 0);
			}
			else {

				Status = InterCompleteIrp(Irp, STATUS_UNSUCCESSFUL, 0);
			}
			break;

		default:
			Status = InterCompleteIrp(Irp, STATUS_INVALID_DEVICE_REQUEST, 0);
			break;
		}

	} while (FALSE);

	return Status;
}