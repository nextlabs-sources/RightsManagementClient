#include "precomp.h"
#include "nxrmdrv.h"
#include "nxrmdrvdef.h"
#include "nxrmdrvhlp.h"
#include "nxrmlaunch.h"
#include "nxrmldr.h"

NXRM_GLOBAL_DATA	Global = { 0 };

//
// Device driver routine declarations.
//
DRIVER_INITIALIZE DriverEntry;

_Dispatch_type_(IRP_MJ_CREATE) 
DRIVER_DISPATCH NxrmdrvDispatchCreate;

_Dispatch_type_(IRP_MJ_CLOSE) 
DRIVER_DISPATCH NxrmdrvDispatchClose;

_Dispatch_type_(IRP_MJ_DEVICE_CONTROL) 
DRIVER_DISPATCH NxrmdrvDispatchDeviceControl;

_Dispatch_type_(IRP_MJ_CLEANUP) 
DRIVER_DISPATCH NxrmdrvDispatchCleanup;

NTSTATUS
	NxrmdrvCreateRMServerSection(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	);

NTSTATUS
	NxrmdrvCreateRMClientSection(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	);

NTSTATUS
	NxrmdrvQueryPoolInformation(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	);

NTSTATUS
	NxrmdrvQueryProcessCacheInformation(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	);

NTSTATUS
	NxrmdrvGetLoaderSnapshot(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	);

NTSTATUS
	NxrmdrvSetProtectedProcess(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	);

NTSTATUS
	NxrmdrvSetDeviceDacl(
	IN PDEVICE_OBJECT		DeviceObject
	);

void recycle_cancel_request(
	IN PVOID				section
	);

void cleanup_client_request(
	IN PVOID				section,
	IN HANDLE				pid
	);

void cleanup_server_request(
	IN PVOID				section
	);

BOOLEAN is_nxrmcore(PUNICODE_STRING FullImageName);

OB_PREOP_CALLBACK_STATUS NxrmdrvObjectMgrPreCallback(
	__in	PVOID							RegistrationContext,
	__inout POB_PRE_OPERATION_INFORMATION	OperationInformation
	);

extern PVOID nxrmFindZwXXX(
	IN OUT ZWPROTECTVIRTUALMEMORY	*pfn_ZwProtectVirtualMemory,
	IN OUT ZWFLUSHINSTRUCTIONCACHE	*pfn_ZwFlushInstructionCache,
	IN OUT ZWFREEVIRTUALMEMORY		*pfn_ZwFreeVirtualMemory,
	IN OUT ZWCREATETHREAD			*pfn_ZwCreateThread,
	IN OUT ZWCREATETHREADEX			*pfn_ZwCreateThreadEx);

extern
	NTSTATUS ObCreateObject(
	IN	KPROCESSOR_MODE		ObjectAttributesAccessMode OPTIONAL,
	IN	POBJECT_TYPE		ObjectType,
	IN	POBJECT_ATTRIBUTES	ObjectAttributes OPTIONAL,
	IN	KPROCESSOR_MODE		AccessMode,
	IN	PVOID				Reserved,
	IN	ULONG				ObjectSizeToAllocate,
	IN	ULONG				PagedPoolCharge OPTIONAL,
	IN	ULONG				NonPagedPoolCharge OPTIONAL,
	OUT	PVOID				*Object
	);

extern 
	NTSTATUS ZwQueryInformationProcess(
	HANDLE				ProcessHandle,
	PROCESSINFOCLASS	ProcessInformationClass,
	PVOID				ProcessInformation,
	ULONG				ProcessInformationLength,
	PULONG				ReturnLength
	);

typedef struct _NXRMSECTION32{
	ULONG	section;
	ULONG	hsem;
	ULONG	hnotify;
}NXRMSECTION32,*PNXRMSECTION32;

extern BOOLEAN is_process_a_service(PEPROCESS Process);

extern NXRM_PROCESS* find_process_in_cache(rb_root *proc_cache, HANDLE process_id);

static NTSTATUS QueryRegistryValue(PWSTR KeyName, PUNICODE_STRING KeyValue);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#endif // ALLOC_PRAGMA

NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT		DriverObject,
	IN PUNICODE_STRING		RegistryPath
	)
{
	NTSTATUS			status = STATUS_SUCCESS;    
	UNICODE_STRING		ntDeviceName = {0};
	UNICODE_STRING		dosDeviceName = {0};
	PDEVICE_EXTENSION	deviceExtension = NULL;
	PDEVICE_OBJECT		deviceObject = NULL;
	PNXRMSECTIONHDR		sec_hdr = NULL;
	BOOLEAN				fSymbolicLink = FALSE;
	BOOLEAN				fProcessLock = FALSE;

	UNICODE_STRING		ObRegisterCallbacksName = {0};
	UNICODE_STRING		RtlQueryPackageIdentityName ={0};

	UNICODE_STRING		KeyValue = {0};

	RTL_OSVERSIONINFOW	Ver = {0};

	ExInitializeDriverRuntime(DrvRtPoolNxOptIn);

	RtlInitUnicodeString(&ntDeviceName, NXRMDRV_DEVICE_NAME_W);

	RtlInitUnicodeString(&ObRegisterCallbacksName,L"ObRegisterCallbacks");

	RtlInitUnicodeString(&Global.Altitude,L"261680");

	RtlInitUnicodeString(&RtlQueryPackageIdentityName,L"RtlQueryPackageIdentity");

	//
	// Create an NON-EXCLUSIVE device, i.e. more than one thread at a time can send
	// i/o requests.
	//

	status = IoCreateDevice(DriverObject,
							sizeof(DEVICE_EXTENSION),	// DeviceExtensionSize
							&ntDeviceName,				// DeviceName
							FILE_DEVICE_NXRMDRV,		// DeviceType
							0,							// DeviceCharacteristics
							FALSE,						// Exclusive
							&deviceObject				// [OUT]
							);

	if (!NT_SUCCESS(status))
	{
		dprintf("[nxrmdrv] IoCreateDevice = 0x%x\n", status);
		goto __failed;
	}

	Global.devobj				= deviceObject;
	Global.hProtectedProcess	= NULL;

	status = RtlGetVersion(&Ver);

	if (!NT_SUCCESS(status))
	{
		dprintf("[nxrmdrv] RtlGetVersion = 0x%x\n", status);
		goto __failed;
	}

	Global.os_major = Ver.dwMajorVersion;
	Global.os_minor	= Ver.dwMinorVersion;
	Global.os_build	= Ver.dwBuildNumber;

	deviceExtension = (PDEVICE_EXTENSION)deviceObject->DeviceExtension;

	if(MmQuerySystemSize() == MmLargeSystem)
	{
		deviceExtension->rm_section_size = NXRM_RM_SECTION_DEFAULT_SIZE*2;
	}
	else
	{
		deviceExtension->rm_section_size = NXRM_RM_SECTION_DEFAULT_SIZE;
	}

	while(deviceExtension->rm_section_size >= 8192)
	{
		deviceExtension->rm_k_section	= ExAllocatePoolWithTag(NonPagedPool,
																deviceExtension->rm_section_size,
																NXRMTAG);

		if(deviceExtension->rm_k_section)
		{
			break;
		}

		deviceExtension->rm_section_size >>=1;
	}

	if(!deviceExtension->rm_k_section)
	{
		dprintf("[nxrmdrv] Out of memory!\n");
		goto __failed;
	}

	RtlZeroMemory(deviceExtension->rm_k_section, deviceExtension->rm_section_size);

	sec_hdr = (PNXRMSECTIONHDR)deviceExtension->rm_k_section;

	sec_hdr->hdr.ready					= 0;
	sec_hdr->hdr.length					= deviceExtension->rm_section_size;
	sec_hdr->hdr.version				= NXRM_DRV_VERSION;

	deviceExtension->rm_server_instance	= NULL;
	deviceExtension->rm_instance_count	= 0;
	deviceExtension->rm_semaphore_object= NULL;

	KeyValue.Buffer			= NULL;
	KeyValue.Length			= 0;
	KeyValue.MaximumLength	= 0;

	status = QueryRegistryValue(L"P1",&KeyValue);

	if(NT_SUCCESS(status))
	{
		dprintf("[nxrmdrv] P1 key value: %wZ\n", &KeyValue);

		RtlFreeUnicodeString(&KeyValue);
	}

	Global.fn_ObRegisterCallbacks = (OBREGISTERCALLBACKS)MmGetSystemRoutineAddress(&ObRegisterCallbacksName);

	if(Global.fn_ObRegisterCallbacks  && Global.pObMgr == NULL)
	{
		OB_CALLBACK_REGISTRATION	ObMgrReg;
		OB_OPERATION_REGISTRATION	ObMgrOperation[2];

		memset(&ObMgrReg, 0, sizeof(ObMgrReg));
		memset(ObMgrOperation, 0, sizeof(ObMgrOperation));

		ObMgrOperation[0].ObjectType		= PsProcessType;
		ObMgrOperation[0].Operations		= OB_OPERATION_HANDLE_CREATE;
		ObMgrOperation[0].PreOperation		= NxrmdrvObjectMgrPreCallback;
		ObMgrOperation[0].PostOperation		= NULL;

		ObMgrOperation[1].ObjectType		= PsThreadType;
		ObMgrOperation[1].Operations		= OB_OPERATION_HANDLE_CREATE;
		ObMgrOperation[1].PreOperation		= NxrmdrvObjectMgrPreCallback;
		ObMgrOperation[1].PostOperation		= NULL;

		ObMgrReg.Version					= OB_FLT_REGISTRATION_VERSION;
		ObMgrReg.OperationRegistrationCount	= 2;
		ObMgrReg.Altitude					= Global.Altitude;
		ObMgrReg.RegistrationContext		= NULL;
		ObMgrReg.OperationRegistration		= ObMgrOperation;

		Global.fn_ObRegisterCallbacks(&ObMgrReg, &Global.pObMgr);
	}

	InitializeListHead(&Global.process_notify_list);

	ExInitializeResourceLite(&Global.process_notify_list_rw_lock);

	fProcessLock = TRUE;

	status = ObCreateObject(KernelMode,
							*ExSemaphoreObjectType,
							NULL,
							KernelMode,
							NULL,
							sizeof(KSEMAPHORE),
							0,
							0,
							&deviceExtension->rm_semaphore_object);

	if(!NT_SUCCESS(status))
	{
		dprintf("[nxrmdrv] ObCreateObject return 0x%x at checkpoint 1\n", status);
		goto __failed;
	}

	KeInitializeSemaphore((PRKSEMAPHORE)deviceExtension->rm_semaphore_object,
						  0,
						  deviceExtension->rm_section_size/sizeof(NXRMRECORD) - 1);

	if((Global.os_major >= 6 && Global.os_minor > 2) || (Global.os_major >= 10))
	{
		status = ObInsertObject(deviceExtension->rm_semaphore_object,
								NULL,
							    SYNCHRONIZE|SEMAPHORE_MODIFY_STATE,
								0,
								NULL,
								&deviceExtension->rm_semaphore_kernel_handle);

		if(!NT_SUCCESS(status))
		{
			dprintf("[nxrmdrv] ObInsertObject return 0x%x at checkpoint 1\n", status);
			goto __failed;
		}
	}

	status = ObCreateObject(KernelMode,
							*ExEventObjectType,
							NULL,
							KernelMode,
							NULL,
							sizeof(KEVENT),
							0,
							0,
							&Global.notify_event);

	if(!NT_SUCCESS(status))
	{
		dprintf("[nxrmdrv] ObCreateObject return 0x%x at checkpoint 2\n", status);
		goto __failed;
	}

	KeInitializeEvent((PRKEVENT)Global.notify_event,
					  SynchronizationEvent,
					  FALSE);

	if((Global.os_major >= 6 && Global.os_minor > 2) || (Global.os_major >= 10))
	{
		status = ObInsertObject(Global.notify_event,
								NULL,
								SYNCHRONIZE,
								0,
								NULL,
								&Global.notify_event_kernel_handle);

		if(!NT_SUCCESS(status))
		{
			dprintf("[nxrmdrv] ObInsertObject return 0x%x at checkpoint 2\n", status);
			goto __failed;
		}
	}

	//
	// init look aside list
	//

	ExInitializeNPagedLookasideList(&Global.process_cache_pool,
									NULL,
									NULL,
									0,
									sizeof(NXRM_PROCESS),
									NXRMTAG,
									0);

	ExInitializeNPagedLookasideList(&Global.instance_cache_pool,
									NULL,
									NULL,
									0,
									sizeof(NXRM_OPENINSTANCE),
									NXRMTAG,
									0);

	ExInitializeNPagedLookasideList(&Global.apc_pool,
									NULL,
									NULL,
									0,
									sizeof(KAPC),
									NXRMTAG,
									0);

	RtlZeroMemory(&Global.last_entry, sizeof(Global.last_entry));
	RtlZeroMemory(&Global.System_entry, sizeof(Global.System_entry));

	Global.last_entry_count		= 0;
	Global.total_query_count	= 0;
	Global.SystemPID			= PsGetCurrentProcessId();

	Global.System_entry.hooked		= FALSE;
	Global.System_entry.is_service	= TRUE;
	Global.System_entry.process_id	= Global.SystemPID;

	memcpy(Global.System_entry.process_path,
		   L"System",
		   min(sizeof(Global.System_entry.process_path), sizeof(L"System")));

	RtlInitUnicodeString(&dosDeviceName, NXRMDRV_DOS_DEVICE_NAME_W);

	status = IoCreateSymbolicLink(&dosDeviceName, &ntDeviceName);

	if (!NT_SUCCESS(status))
	{
		dprintf("[nxrmdrv] IoCreateSymbolicLink = 0x%x\n", status);
		goto __failed;
	}

	fSymbolicLink = TRUE;

	//
	// Create dispatch points for device control, create, close.
	//

	DriverObject->MajorFunction[IRP_MJ_CREATE]			= NxrmdrvDispatchCreate;
	DriverObject->MajorFunction[IRP_MJ_CLOSE]			= NxrmdrvDispatchClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]	= NxrmdrvDispatchDeviceControl;
	DriverObject->MajorFunction[IRP_MJ_CLEANUP]			= NxrmdrvDispatchCleanup;
	DriverObject->DriverUnload							= NULL;

	Global.pKernelBase = nxrmFindZwXXX(&Global.fn_ZwProtectVirtualMemory,
									   &Global.fn_ZwFlushInstructionCache,
									   &Global.fn_ZwFreeVirtualMemory,
									   &Global.fn_ZwCreateThread,
									   &Global.fn_ZwCreateThreadEx);

	if((Global.os_major >= 6 && Global.os_minor >=2) || (Global.os_major >= 10))
	{
		Global.fn_RtlQueryPackageIdentity = (RTLQUERYPACKAGEIDENTITY)MmGetSystemRoutineAddress(&RtlQueryPackageIdentityName);
	}

	Global.rb_process_cache.rb_node = NULL;

	Global.process_count = 0;

	KeInitializeSpinLock(&Global.process_cache_lock);

	if ((Global.os_major >= 6 && Global.os_minor >= 2) || (Global.os_major >= 10))
	{
		NTSTATUS Daclstatus = NxrmdrvSetDeviceDacl(deviceObject);

		if (!NT_SUCCESS(status))
		{
			dprintf("[nxrmdrv] NxrmdrvSetDeviceDacl = 0x%x\n", Daclstatus);
		}
	}

	status = NxrmLdrInitialize();

	if (!NT_SUCCESS(status))
	{
		dprintf("[nxrmdrv] NxrmLdrInitialize = 0x%x\n", status);
		goto __failed;
	}

	status = NxrmLaunchInitialize();

	if (!NT_SUCCESS(status))
	{
		dprintf("[nxrmdrv] NxrmLaunchInitialize = 0x%x\n", status);
		goto __failed;
	}

	if (NT_SUCCESS(status))
		return status;

__failed:

	if (deviceExtension)
	{
		if (Global.notify_event_kernel_handle)
		{
			ZwClose(Global.notify_event_kernel_handle);
			Global.notify_event_kernel_handle = NULL;
		}

		if (Global.notify_event)
		{
			ObDereferenceObject(Global.notify_event);
			Global.notify_event = NULL;
		}
		
		if (deviceExtension->rm_semaphore_kernel_handle)
		{
			ZwClose(deviceExtension->rm_semaphore_kernel_handle);
			deviceExtension->rm_semaphore_kernel_handle = NULL;
		}

		if (deviceExtension->rm_semaphore_object)
		{
			ObDereferenceObject(deviceExtension->rm_semaphore_object);
			deviceExtension->rm_semaphore_object = NULL;
		}

		if (deviceExtension->rm_k_section)
		{
			ExFreePoolWithTag(deviceExtension->rm_k_section, NXRMTAG);
			deviceExtension->rm_k_section = NULL;
		}

		if (fProcessLock)
		{
			ExDeleteResourceLite(&Global.process_notify_list_rw_lock);
		}
	}

	if (fSymbolicLink)
		IoDeleteSymbolicLink(&dosDeviceName);

	if (deviceObject)
		IoDeleteDevice(deviceObject);

	return status;
}

NTSTATUS
NxrmdrvDispatchCreate(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	)
{
	NTSTATUS status = STATUS_SUCCESS;

	PDEVICE_EXTENSION	deviceExtension;
	PIO_STACK_LOCATION  IrpSp = NULL;
	PNXRM_OPENINSTANCE	open = NULL;

	Irp->IoStatus.Information = 0;

	dprintf("[nxrmdrv] IRP_MJ_CREATE\n");

	do 
	{
		IrpSp = IoGetCurrentIrpStackLocation(Irp);

		deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

		open = (PNXRM_OPENINSTANCE)ExAllocateFromNPagedLookasideList(&Global.instance_cache_pool);

		if(open == NULL)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			IrpSp->FileObject->FsContext = NULL;
			break;
		}

		open->u_section_mdl		= NULL;
		open->u_section			= NULL;
		open->hsem				= NULL;
		open->hnotifyevent		= NULL;
		open->is_server			= FALSE;
		open->is_badguy			= FALSE;
		open->open_in_progress	= FALSE;

		KeInitializeSpinLock(&open->open_lock);

		IrpSp->FileObject->FsContext = open;

	} while(FALSE);

	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;
}

NTSTATUS
NxrmdrvDispatchCleanup(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	)
{
	NTSTATUS status = STATUS_SUCCESS;

	PIO_STACK_LOCATION  IrpSp = NULL;
	PDEVICE_EXTENSION	deviceExtension = NULL;

	PNXRM_OPENINSTANCE	open = NULL;
	PNXRM_OPENINSTANCE	p = NULL;
	PNXRMSECTIONHDR		sec_hdr = NULL;

	Irp->IoStatus.Information = 0;

	dprintf("[nxrmdrv] IRP_MJ_CLEANUP\n");

	IrpSp = IoGetCurrentIrpStackLocation(Irp);

	open = (PNXRM_OPENINSTANCE)IrpSp->FileObject->FsContext;

	deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

	do 
	{
		if(open == NULL)
		{
			break;
		}

		if(open->is_server)
		{
			sec_hdr	 = (PNXRMSECTIONHDR)open->u_section;

			InterlockedExchange(&sec_hdr->hdr.ready,0);

			InterlockedExchange(&sec_hdr->hdr.channel_mask,0);

			cleanup_server_request(deviceExtension->rm_k_section);

			recycle_cancel_request(deviceExtension->rm_k_section);

			_InterlockedExchange((LONG*)&deviceExtension->rm_server_instance, 0);

			Global.hProtectedProcess = NULL;

			sec_hdr->hdr.protected_process_id = 0;
		}
		else
		{
			cleanup_client_request(deviceExtension->rm_k_section,PsGetCurrentProcessId());

			if(open->u_section)
				InterlockedDecrement(&deviceExtension->rm_instance_count);
		}

		if(open->u_section && open->u_section_mdl)
		{
			MmUnmapLockedPages(open->u_section,open->u_section_mdl);
			open->u_section = NULL;
		}

		if(open->u_section_mdl)
		{
			IoFreeMdl(open->u_section_mdl);
			open->u_section_mdl = NULL;
		}

		ExFreeToNPagedLookasideList(&Global.instance_cache_pool,(PVOID)open);

		open = NULL;

	} while(FALSE);

	IrpSp->FileObject->FsContext = NULL;

	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;

}

NTSTATUS
NxrmdrvDispatchClose(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	)
{
	NTSTATUS status = STATUS_SUCCESS;

	Irp->IoStatus.Information = 0;

	dprintf("[nxrmdrv] IRP_MJ_CLOSE\n");

	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;
}

NTSTATUS
NxrmdrvDispatchDeviceControl(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	)
{
	NTSTATUS			status = STATUS_SUCCESS;
	PIO_STACK_LOCATION	IrpSp;

	ULONG				ioControlCode;

	IrpSp = IoGetCurrentIrpStackLocation(Irp);

	//
	// Get the pointer to the input/output buffer and it's length
	//

	ioControlCode	= IrpSp->Parameters.DeviceIoControl.IoControlCode;

	Irp->IoStatus.Information = 0;

	switch (ioControlCode)
	{
	case IOCTL_NXRMDRV_MAP_RM_SERVER_SECTION:
		{
			status = NxrmdrvCreateRMServerSection(DeviceObject,Irp);
			break;
		}
	case IOCTL_NXRMDRV_MAP_RM_CLIENT_SECTION:
		{
			status = NxrmdrvCreateRMClientSection(DeviceObject,Irp);
			break;
		}
	case IOCTL_NXRMDRV_QUERY_POOL_INFO:
		{
			status = NxrmdrvQueryPoolInformation(DeviceObject,Irp);
			break;
		}
	case IOCTL_NXRMDRV_QUERY_PROCESS_INFO:
		{
			status = NxrmdrvQueryProcessCacheInformation(DeviceObject,Irp);
			break;
		}
	case IOCTL_NXRMDRV_GET_LOADER_SNAPSHOT:
		{
			status = NxrmdrvGetLoaderSnapshot(DeviceObject,Irp);
			break; 
		}
	case IOCTL_NXRMDRV_SET_PROTECTED_PID:
		{
			status = NxrmdrvSetProtectedProcess(DeviceObject,Irp);
			break;
		}
	default:
		status = STATUS_INVALID_PARAMETER;

		dprintf("[nxrmdrv] unknown IRP_MJ_DEVICE_CONTROL = 0x%x (%04x,%04x)\n",
				ioControlCode, 
				DEVICE_TYPE_FROM_CTL_CODE(ioControlCode),
				IoGetFunctionCodeFromCtlCode(ioControlCode));

		break;
	}

	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;
}

NTSTATUS NxrmdrvCreateRMServerSection(
	IN PDEVICE_OBJECT	DeviceObject, 
	IN PIRP				Irp
	)
{
	NTSTATUS status = STATUS_SUCCESS;

	PIO_STACK_LOCATION	IrpSp;
	PDEVICE_EXTENSION	deviceExtension;
	PVOID				ioBuf;
	ULONG				outBufLength;

	PNXRM_OPENINSTANCE	open	= NULL;
	PNXRMSECTIONHDR		sec_hdr	= NULL;

	NXRMSECTION			*psection = NULL;

	IrpSp			= IoGetCurrentIrpStackLocation(Irp);
	deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

	Irp->IoStatus.Information = 0;

	//
	// Get the pointer to the input/output buffer and it's length
	//

	ioBuf			= Irp->AssociatedIrp.SystemBuffer;
	outBufLength	= IrpSp->Parameters.DeviceIoControl.OutputBufferLength;


	do 
	{
		if(outBufLength < sizeof(NXRMSECTION))
		{
			status = STATUS_INVALID_PARAMETER;
			break;
		}

#ifndef _DEBUG
		if(!is_process_a_service(PsGetCurrentProcess()))
		{
			status = STATUS_PRIVILEGE_NOT_HELD;
			break;
		}
#endif
		open = (PNXRM_OPENINSTANCE)IrpSp->FileObject->FsContext;

		if(open == NULL)
		{
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		if(open->is_badguy)
		{
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		if(_InterlockedCompareExchange((LONG*)&deviceExtension->rm_server_instance,*(LONG*)&open,0))
		{
			dprintf("[nxrmdrv] Another thread own server section already\n!");
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		open->open_in_progress = TRUE;

		open->u_section_mdl = IoAllocateMdl(deviceExtension->rm_k_section,
											deviceExtension->rm_section_size,
											FALSE,
											FALSE,
											NULL);

		if(open->u_section_mdl == NULL)
		{
			_InterlockedExchange((LONG*)&deviceExtension->rm_server_instance, 0);

			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		MmBuildMdlForNonPagedPool(open->u_section_mdl);

		__try
		{
			open->u_section = MmMapLockedPagesSpecifyCache(open->u_section_mdl,
														   UserMode,
														   MmCached,
														   NULL,
														   FALSE,
														   HighPagePriority);

		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			dprintf("[nxrmdrv] Fail to map section page![Checkpoint 1]\n");

			_InterlockedExchange((LONG*)&deviceExtension->rm_server_instance, 0);

			status = STATUS_UNSUCCESSFUL;
			break;
		}

		if(!open->u_section)
		{
			dprintf("[nxrmdrv] Fail to map section page![Checkpoint 2]\n");

			_InterlockedExchange((LONG*)&deviceExtension->rm_server_instance, 0);

			status = STATUS_UNSUCCESSFUL;
			break;
		}

		psection = (NXRMSECTION*)ioBuf;

		psection->section	= open->u_section;

		status = ObOpenObjectByPointer(deviceExtension->rm_semaphore_object,
									   0,
									  NULL,
									  SYNCHRONIZE|SEMAPHORE_MODIFY_STATE,
									  *ExSemaphoreObjectType,
									  UserMode,
									  &open->hsem);

		if(!NT_SUCCESS(status))
		{
			dprintf("[nxrmdrv] fail to create semaphore\n");

			_InterlockedExchange((LONG*)&deviceExtension->rm_server_instance, 0);

			status = STATUS_UNSUCCESSFUL;
			break;
		}

		status = ObOpenObjectByPointer(Global.notify_event,
									   0,
									   NULL,
									   SYNCHRONIZE,
									   *ExEventObjectType,
									   UserMode,
									   &open->hnotifyevent);

		if(!NT_SUCCESS(status))
		{
			dprintf("[nxrmdrv] fail to create event\n");

			_InterlockedExchange((LONG*)&deviceExtension->rm_server_instance, 0);

			status = STATUS_UNSUCCESSFUL;
			break;
		}

		psection->hsem		= open->hsem;
		psection->hnotify	= open->hnotifyevent;

		sec_hdr =(NXRMSECTIONHDR*)open->u_section;


		// not required at all but make sure that flag is correct
		InterlockedExchange(&sec_hdr->hdr.ready, 0);

		Irp->IoStatus.Information = sizeof(NXRMSECTION);

		open->open_in_progress	= FALSE;
		open->is_server			= TRUE;

#ifndef _DEBUG
		Global.hProtectedProcess = PsGetCurrentProcessId();

		sec_hdr->hdr.protected_process_id = (ULONG)(ULONG_PTR)Global.hProtectedProcess;
#endif
	} while(FALSE);

	if(!NT_SUCCESS(status))
	{
		if(open)
		{
			if(open->hsem)
			{
				ZwClose(open->hsem);
				open->hsem = NULL;
			}

			if(open->hnotifyevent)
			{
				ZwClose(open->hnotifyevent);
				open->hnotifyevent = NULL;
			}

			if(open->u_section && open->u_section_mdl)
			{
				MmUnmapLockedPages(open->u_section,open->u_section_mdl);
				open->u_section = NULL;
			}

			if(open->u_section_mdl)
			{
				IoFreeMdl(open->u_section_mdl);
				open->u_section_mdl = NULL;
			}

			open->open_in_progress	= FALSE;
			open->is_server			= FALSE;
		}
	}

	return status;
}

NTSTATUS NxrmdrvCreateRMClientSection(
	IN PDEVICE_OBJECT	DeviceObject, 
	IN PIRP				Irp
	)
{
	NTSTATUS status = STATUS_SUCCESS;

	PIO_STACK_LOCATION	IrpSp;
	PDEVICE_EXTENSION	deviceExtension;
	PVOID				ioBuf;
	ULONG				outBufLength;
	PNXRM_OPENINSTANCE	open	= NULL;
	KLOCK_QUEUE_HANDLE	lockhandle;
	NXRMSECTION			*psection = NULL;
	NXRMSECTION32		*psection32 = NULL;
	BOOLEAN				bIsWow64 = FALSE;

	IrpSp			= IoGetCurrentIrpStackLocation(Irp);
	deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

	Irp->IoStatus.Information = 0;

	//
	// Get the pointer to the input/output buffer and it's length
	//

	ioBuf			= Irp->AssociatedIrp.SystemBuffer;
	outBufLength	= IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

#ifdef _AMD64_

	bIsWow64 = IoIs32bitProcess(Irp);

#endif

	do 
	{

		if(bIsWow64)
		{
			if(outBufLength < sizeof(NXRMSECTION32))
			{
				status = STATUS_INVALID_PARAMETER;
				break;
			}
		}
		else
		{
			if(outBufLength < sizeof(NXRMSECTION))
			{
				status = STATUS_INVALID_PARAMETER;
				break;
			}
		}

		open = (PNXRM_OPENINSTANCE)IrpSp->FileObject->FsContext;

		if(open == NULL)
		{
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		if(open->is_badguy)
		{
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		KeAcquireInStackQueuedSpinLock(&open->open_lock,&lockhandle);

		if(open->u_section || open->u_section_mdl || open->open_in_progress)
		{
			KeReleaseInStackQueuedSpinLock(&lockhandle);
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		open->open_in_progress = TRUE;

		KeReleaseInStackQueuedSpinLock(&lockhandle);

		open->u_section_mdl = IoAllocateMdl(deviceExtension->rm_k_section,
											deviceExtension->rm_section_size,
											FALSE,
											FALSE,
											NULL);

		if(open->u_section_mdl == NULL)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		MmBuildMdlForNonPagedPool(open->u_section_mdl);

		__try
		{
			open->u_section = MmMapLockedPagesSpecifyCache(open->u_section_mdl,
														   UserMode,
														   MmCached,
														   NULL,
														   FALSE,
														   HighPagePriority);

		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			dprintf("[nxrmdrv] Fail to map section page!\n");
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		if(!open->u_section)
		{
			dprintf("[nxrmdrv] Fail to map section page![B]\n");
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		if(bIsWow64)
		{
			psection32 = (NXRMSECTION32*)ioBuf;
		}
		else
		{
			psection = (NXRMSECTION*)ioBuf;
		}

		status = ObOpenObjectByPointer(deviceExtension->rm_semaphore_object,
									   0,
									   NULL,
									   SYNCHRONIZE|SEMAPHORE_MODIFY_STATE,
									   *ExSemaphoreObjectType,
									   UserMode,
									   &open->hsem);

		if(!NT_SUCCESS(status))
		{
			dprintf("[nxrmdrv] fail to create semaphore\n");
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		if(bIsWow64)
		{
			psection32->hsem		= (ULONG)(ULONG_PTR)open->hsem;
			psection32->hnotify		= 0;

			psection32->section		= (ULONG)(ULONG_PTR)open->u_section;

			Irp->IoStatus.Information = sizeof(NXRMSECTION32);
		}
		else
		{
			psection->hsem		= open->hsem;
			psection->hnotify	= NULL;

			psection->section = open->u_section;

			Irp->IoStatus.Information = sizeof(NXRMSECTION);
		}


		open->open_in_progress	= FALSE;
		open->is_server			= FALSE;

		InterlockedIncrement(&deviceExtension->rm_instance_count);

	} while(FALSE);

	if(!NT_SUCCESS(status))
	{
		if(open)
		{
			if(open->hsem)
			{
				ZwClose(open->hsem);
				open->hsem = NULL;
			}

			if(open->u_section && open->u_section_mdl)
			{
				MmUnmapLockedPages(open->u_section,open->u_section_mdl);
				open->u_section = NULL;
			}

			if(open->u_section_mdl)
			{
				IoFreeMdl(open->u_section_mdl);
				open->u_section_mdl = NULL;
			}

			open->open_in_progress	= FALSE;
			open->is_server			= FALSE;
		}
	}

	return status;
}

NTSTATUS
NxrmdrvQueryPoolInformation(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	)
{
	NTSTATUS status = STATUS_SUCCESS;

	PIO_STACK_LOCATION	IrpSp;
	PDEVICE_EXTENSION	deviceExtension;
	PVOID				ioBuf;
	ULONG				inBufLength, outBufLength;
	PNXRMSECTIONHDR		sec_hdr = NULL;
	PNXRM_POOL_INFO		pPoolInfo = NULL;
	ULONG				instances = 0;
	LIST_ENTRY			*ite = NULL;

	IrpSp			= IoGetCurrentIrpStackLocation(Irp);
	deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

	Irp->IoStatus.Information = 0;

	//
	// Get the pointer to the input/output buffer and it's length
	//

	ioBuf			= Irp->AssociatedIrp.SystemBuffer;
	inBufLength		= IrpSp->Parameters.DeviceIoControl.InputBufferLength;
	outBufLength	= IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
	sec_hdr			= (NXRMSECTIONHDR*)(deviceExtension->rm_k_section);

	do 
	{

#ifndef _DEBUG
		if(!is_process_a_service(PsGetCurrentProcess()))
		{
			status = STATUS_PRIVILEGE_NOT_HELD;
			break;
		}
#endif
		if(outBufLength < sizeof(NXRM_POOL_INFO))
		{
			if(Irp->UserBuffer)
				Irp->IoStatus.Information = 0;
			else
				Irp->IoStatus.Information = sizeof(NXRM_POOL_INFO);

			status = STATUS_BUFFER_OVERFLOW;
			break;
		}

		pPoolInfo = (NXRM_POOL_INFO*)ioBuf;

		pPoolInfo->pool_size	= deviceExtension->rm_section_size;
		pPoolInfo->pool_address	= (ULONG_PTR)deviceExtension->rm_k_section;
		pPoolInfo->pool_status	= sec_hdr->hdr.ready;

		pPoolInfo->pool_instances = deviceExtension->rm_instance_count;

		Irp->IoStatus.Information = sizeof(NXRM_POOL_INFO);

	} while(FALSE);

	return status;
}

NTSTATUS
NxrmdrvQueryProcessCacheInformation(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	)
{
	NTSTATUS status = STATUS_SUCCESS;

	PIO_STACK_LOCATION	IrpSp = NULL;
	PVOID				ioBuf = NULL;
	NXRM_PROCESS_ENTRY	*pEntry = NULL;
	NXRM_PROCESS		*process = NULL;
	ULONG				inBufLength = 0;
	ULONG				outBufLength = 0;
	HANDLE				PID = NULL;
	KLOCK_QUEUE_HANDLE	lockhandle = {0};

	IrpSp			= IoGetCurrentIrpStackLocation(Irp);

	Irp->IoStatus.Information = 0;

	//
	// Get the pointer to the input/output buffer and it's length
	//

	ioBuf			= Irp->AssociatedIrp.SystemBuffer;
	inBufLength		= IrpSp->Parameters.DeviceIoControl.InputBufferLength;
	outBufLength	= IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

	do 
	{
		if(inBufLength != sizeof(HANDLE) || outBufLength != sizeof(NXRM_PROCESS_ENTRY))
		{
			status = STATUS_INVALID_PARAMETER;
			break;
		}

		PID = *(HANDLE*)ioBuf;

		pEntry = (NXRM_PROCESS_ENTRY*)ioBuf;

		KeAcquireInStackQueuedSpinLock(&Global.process_cache_lock, &lockhandle);

		do 
		{
			if(PID == Global.SystemPID)
			{
				memcpy(pEntry, &Global.System_entry, sizeof(*pEntry));
				break;
			}

			if(PID == Global.last_entry.process_id &&
			   PID != 0)
			{
				memcpy(pEntry, &Global.last_entry, sizeof(*pEntry));
				break;
			}

			process = find_process_in_cache(&Global.rb_process_cache, PID);

			if(process)
			{
				if (!process->initialized)
				{
					dprintf("[nxrmdrv] Process %p is in the cache but it's not finished initialization yet!\n", PID);
					status = STATUS_UNSUCCESSFUL;
					break;
				}

				memset(pEntry, 0, sizeof(*pEntry));

				pEntry->hooked				= (ULONG)process->hooked;
				pEntry->is_service			= (ULONG)process->is_Service;
				pEntry->process_id			= process->process_id;
				pEntry->parent_id			= process->parent_id;
				pEntry->session_id			= process->session_id;
				pEntry->platform			= process->platform;

				memcpy(pEntry->process_path,
					   process->process_path,
					min(sizeof(pEntry->process_path) - sizeof(WCHAR),sizeof(process->process_path)));

				memcpy(&Global.last_entry,
					   pEntry,
					   sizeof(Global.last_entry));

			}
			else
			{
				dprintf("[nxrmdrv] Can not find process %p in cache!\n", PID);
				status = STATUS_UNSUCCESSFUL;
			}


		} while (FALSE);

		KeReleaseInStackQueuedSpinLock(&lockhandle);

	} while (FALSE);

	if(status == STATUS_SUCCESS)
	{
		Irp->IoStatus.Information = sizeof(NXRM_PROCESS_ENTRY);
	}

	return status;
}


void cleanup_client_request(
	IN PVOID				section,
	IN HANDLE				pid
	)
{
	NXRMRECORD *record = NULL;

	NXRMSECTIONHDR *phdr = NULL;

	phdr = (NXRMSECTIONHDR*)section;

	record = (NXRMRECORD*)(phdr+1);

	if (phdr->hdr.dwm_wnd_spin_lock == *((LONG*)&pid))
	{
		InterlockedExchange(&phdr->hdr.dwm_wnd_spin_lock, 0);
	}

	do 
	{
		if(record->hdr.record_taken == *((LONG*)&pid))
		{
			InterlockedExchange(&record->hdr.record_cancelled,1);
		}

		record +=1;

	} while((UCHAR*)record + sizeof(NXRMRECORD) <= (UCHAR*)phdr + phdr->hdr.length);

	return;
}

void recycle_cancel_request(
	IN PVOID				section
	)
{
	NXRMRECORD *record = NULL;

	NXRMSECTIONHDR *phdr = NULL;

	phdr = (NXRMSECTIONHDR*)section;

	record = (NXRMRECORD*)(phdr+1);

	do 
	{
		if((record->hdr.record_state == NXRM_RECORD_STATE_READY_FOR_CLIENT || record->hdr.record_state == NXRM_RECORD_STATE_BAD_RECORD) &&
			record->hdr.record_cancelled)
		{

			//
			// found one record need to recycle
			//
			record->hdr.record_cancelled	= 0;
			record->hdr.record_flags		= 0;
			record->hdr.record_state		= NXRM_RECORD_STATE_FREE;
			record->hdr.record_owner		= 4;
			record->hdr.record_type			= NXRM_RECORD_TYPE_INVALID;

			memset(&record->data, 0, NXRM_SECTION_MAX_DATA_SIZE);
			memset(&record->checksum, 0, sizeof(record->checksum));

			record->length = 0;

			InterlockedExchange(&record->hdr.record_taken,0);

		}

		record +=1;

	} while((UCHAR*)record + sizeof(NXRMRECORD) <= (UCHAR*)phdr + phdr->hdr.length);

	return;
}


void cleanup_server_request(
	IN PVOID				section
	)
{
	NXRMRECORD *record = NULL;

	NXRMSECTIONHDR *phdr = NULL;

	HANDLE pid = PsGetCurrentProcessId();

	phdr = (NXRMSECTIONHDR*)section;

	record = (NXRMRECORD*)(phdr+1);

	if (phdr->hdr.dwm_wnd_spin_lock == *((LONG*)&pid))
	{
		InterlockedExchange(&phdr->hdr.dwm_wnd_spin_lock, 0);
	}

	do 
	{
		if(record->hdr.record_state == NXRM_RECORD_STATE_READY_FOR_ENGINE || 
		   record->hdr.record_state == NXRM_RECORD_STATE_IN_PROCESSING)
		{
			record->hdr.record_state = NXRM_RECORD_STATE_BAD_RECORD;
		}

		record +=1;

	} while((UCHAR*)record + sizeof(NXRMRECORD) <= (UCHAR*)phdr + phdr->hdr.length);

	return;
}

BOOLEAN is_nxrmcore(PUNICODE_STRING FullImageName)
{
	BOOLEAN bRet = TRUE;

	WCHAR	wsz_nxrmcore[NXRM_MAX_PATH_LEN];

	UNICODE_STRING strnxrmcore;

	memset(wsz_nxrmcore, 0, sizeof(wsz_nxrmcore));

	RtlStringCbCatW(wsz_nxrmcore, sizeof(wsz_nxrmcore), (WCHAR*)0x7ffe0030);

	RtlStringCbCatW(wsz_nxrmcore, sizeof(wsz_nxrmcore), L"\\System32\\nxrmcore.dll");

	RtlInitUnicodeString(&strnxrmcore, wsz_nxrmcore);

	if(RtlCompareUnicodeString(FullImageName,&strnxrmcore,TRUE) != 0)
	{
		bRet = FALSE;
	}

	return bRet;
}

static NTSTATUS QueryRegistryValue(
	PWSTR			KeyName, 
	PUNICODE_STRING KeyValue)
{
	NTSTATUS status = STATUS_SUCCESS;
	RTL_QUERY_REGISTRY_TABLE  paramTable[2];

	RtlZeroMemory(paramTable, sizeof(paramTable));

	paramTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
	paramTable[0].Name          = KeyName;
	paramTable[0].EntryContext	= KeyValue;

	status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES, NXRMDRV_PARAMETERS_REG_KEY, paramTable, NULL, NULL);

	return status;
}

OB_PREOP_CALLBACK_STATUS NxrmdrvObjectMgrPreCallback(
	__in	PVOID							RegistrationContext,
	__inout POB_PRE_OPERATION_INFORMATION	OperationInformation
	)
{
	do 
	{
		if(!Global.hProtectedProcess)
		{
			break;
		}

		if(OperationInformation->ObjectType == *PsProcessType && (OperationInformation->KernelHandle != 1))
		{
			HANDLE hProcessID = NULL;

			ACCESS_MASK mask; 

			hProcessID = PsGetProcessId((PEPROCESS)OperationInformation->Object);

			if(hProcessID != Global.hProtectedProcess)
			{
				break;
			}

			mask = OperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess;

			if(FlagOn(mask,PROCESS_TERMINATE))
			{
				ClearFlag(OperationInformation->Parameters->CreateHandleInformation.DesiredAccess,PROCESS_TERMINATE);
			}

			if(FlagOn(mask,PROCESS_VM_WRITE))
			{
				ClearFlag(OperationInformation->Parameters->CreateHandleInformation.DesiredAccess,PROCESS_VM_WRITE);
			}
		}
		else if(OperationInformation->ObjectType == *PsThreadType && (OperationInformation->KernelHandle != 1))
		{
			HANDLE hProcessID = NULL;

			ACCESS_MASK mask; 

			hProcessID = PsGetThreadProcessId((PETHREAD)OperationInformation->Object);

			if(hProcessID != Global.hProtectedProcess)
			{
				break;
			}

			//
			// ignore thread function call from protected process
			// this means we allow our protected process to kill our its own thread
			//
			if(hProcessID == PsGetCurrentProcessId())
			{
				break;
			}

			mask = OperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess;

			if(FlagOn(mask,THREAD_TERMINATE))
			{
				ClearFlag(OperationInformation->Parameters->CreateHandleInformation.DesiredAccess,THREAD_TERMINATE);
			}

			if(FlagOn(mask,THREAD_SET_CONTEXT))
			{
				ClearFlag(OperationInformation->Parameters->CreateHandleInformation.DesiredAccess,THREAD_SET_CONTEXT);
			}
		}
		else
		{

		}

	} while (FALSE);

	return OB_PREOP_SUCCESS;
}

NTSTATUS
	NxrmdrvSetDeviceDacl(
	IN PDEVICE_OBJECT		DeviceObject
	)
{
	NTSTATUS status = STATUS_SUCCESS;

	SECURITY_DESCRIPTOR sd = { 0 };
	ULONG aclSize = 0;
	PACL pAcl = NULL;
	HANDLE hDeviceObj = NULL;

	do 
	{
		status = ObOpenObjectByPointer(DeviceObject,
									   OBJ_KERNEL_HANDLE,
									   NULL,
									   WRITE_DAC,
									   0,
									   KernelMode,
									   &hDeviceObj);

		if(!NT_SUCCESS(status)) 
		{
			break;
		} 

		aclSize = sizeof(ACL);
		aclSize += RtlLengthSid(SeExports->SeLocalSystemSid);
		aclSize += RtlLengthSid(SeExports->SeAliasAdminsSid);
		aclSize += RtlLengthSid(SeExports->SeWorldSid);
		aclSize += RtlLengthSid(SeExports->SeRestrictedSid);
		aclSize += RtlLengthSid(SeExports->SeAllAppPackagesSid);

		aclSize += 5 * FIELD_OFFSET(ACCESS_ALLOWED_ACE, SidStart);

		pAcl = (PACL)ExAllocatePoolWithTag(PagedPool, aclSize, NXRMTAG);

		if (pAcl == NULL) 
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		status = RtlCreateAcl(pAcl, aclSize, ACL_REVISION);

		if(!NT_SUCCESS(status)) 
		{
			break;
		}

		status = RtlAddAccessAllowedAce(pAcl,
										ACL_REVISION,
										GENERIC_ALL,
										SeExports->SeLocalSystemSid);
		if(!NT_SUCCESS(status)) 
		{
			break;
		}

		status = RtlAddAccessAllowedAce(pAcl,
										ACL_REVISION,
										GENERIC_ALL,
										SeExports->SeAliasAdminsSid );
		if(!NT_SUCCESS(status)) 
		{
			break;
		}

		status = RtlAddAccessAllowedAce(pAcl,
										ACL_REVISION,
										GENERIC_READ,
										SeExports->SeWorldSid);
		if(!NT_SUCCESS(status)) 
		{
			break;
		}

		status = RtlAddAccessAllowedAce(pAcl,
										ACL_REVISION,
										GENERIC_READ,
										SeExports->SeRestrictedSid);
		if(!NT_SUCCESS(status)) 
		{
			break;
		}

		status = RtlAddAccessAllowedAce(pAcl,
										ACL_REVISION,
										GENERIC_READ,
										SeExports->SeAllAppPackagesSid);
		if(!NT_SUCCESS(status)) 
		{
			break;
		}

		//
		// Create a security descriptor
		//
		status = RtlCreateSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);

		if(!NT_SUCCESS(status)) 
		{
			break;
		}

		//
		// Associate the above pAcl with the security descriptor
		//
		status = RtlSetDaclSecurityDescriptor(&sd,TRUE,pAcl,FALSE);

		if(!NT_SUCCESS(status)) 
		{
			break;
		}

		//
		// Set security on the object
		//
		status = ZwSetSecurityObject(hDeviceObj, DACL_SECURITY_INFORMATION, &sd);

		if(!NT_SUCCESS(status)) 
		{
			break;
		}

	} while (FALSE);

	if(hDeviceObj)
	{
		ZwClose(hDeviceObj);
		hDeviceObj = NULL;
	}

	if(pAcl != NULL) 
	{
		ExFreePoolWithTag(pAcl, NXRMTAG);
		pAcl = NULL;
	}

	return status;

}

NTSTATUS
	NxrmdrvGetLoaderSnapshot(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	)
{
	NTSTATUS status = STATUS_SUCCESS;

	PIO_STACK_LOCATION	IrpSp;
	PDEVICE_EXTENSION	deviceExtension;
	PVOID				ioBuf;
	ULONG				inBufLength, outBufLength;
	PNXRM_PROCESS_INFO	pInfo = NULL;
	PNXRM_PROCESS_ENTRY	pEntry = NULL;
	NXRM_PROCESS		*process = NULL;
	rb_node				*node = NULL;
	ULONG				total = 0;
	KLOCK_QUEUE_HANDLE	Lockhandle = {0};

	IrpSp			= IoGetCurrentIrpStackLocation(Irp);
	deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

	Irp->IoStatus.Information = 0;

	//
	// Get the pointer to the input/output buffer and it's length
	//

	ioBuf			= Irp->AssociatedIrp.SystemBuffer;
	inBufLength		= IrpSp->Parameters.DeviceIoControl.InputBufferLength;
	outBufLength	= IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

	do 
	{
		KeAcquireInStackQueuedSpinLock(&Global.process_cache_lock, &Lockhandle);

		if(outBufLength < sizeof(NXRM_PROCESS_INFO) + (Global.process_count - 1) *sizeof(NXRM_PROCESS_ENTRY))
		{
			if(Irp->UserBuffer)
				Irp->IoStatus.Information = 0;
			else
				Irp->IoStatus.Information = sizeof(NXRM_PROCESS_INFO) + (Global.process_count - 1) *sizeof(NXRM_PROCESS_ENTRY);

			status = STATUS_BUFFER_OVERFLOW;

			KeReleaseInStackQueuedSpinLock(&Lockhandle);

			break;
		}

		pInfo = (NXRM_PROCESS_INFO*)ioBuf;

		pInfo->NumberOfEntry	= 0;
		pInfo->NumberOfQuery	= Global.total_query_count;
		pInfo->NumberOfCacheHit	= Global.last_entry_count;

		pEntry = pInfo->Entry;

		total = sizeof(NXRM_PROCESS_INFO) - sizeof(NXRM_PROCESS_ENTRY);

		RB_EACH_NODE(node, &Global.rb_process_cache)
		{
			process = rb_entry(node, NXRM_PROCESS, process_cache_node);

			if (!process->initialized)
			{
				continue;
			}

			memset(pEntry,0,sizeof(*pEntry));

			pEntry->hooked				= (ULONG)process->hooked;
			pEntry->is_service			= (ULONG)process->is_Service;
			pEntry->process_id			= process->process_id;
			pEntry->parent_id			= process->parent_id;
			pEntry->session_id			= process->session_id;
			pEntry->platform			= process->platform;

			memcpy(pEntry->process_path,
				   process->process_path,
				   min(sizeof(pEntry->process_path) - sizeof(WCHAR),sizeof(process->process_path)));

			total += sizeof(NXRM_PROCESS_ENTRY);

			pInfo->NumberOfEntry++;

			pEntry++;
		}

		KeReleaseInStackQueuedSpinLock(&Lockhandle);

		Irp->IoStatus.Information = total;

	} while(FALSE);

	return status;

}

NTSTATUS
	NxrmdrvSetProtectedProcess(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	)
{
	NTSTATUS status = STATUS_SUCCESS;

	PIO_STACK_LOCATION	IrpSp;
	PDEVICE_EXTENSION	deviceExtension;
	PVOID				ioBuf;
	ULONG				inBufLength;
	ULONG				pid;
	PNXRMSECTIONHDR		sec_hdr	= NULL;

	IrpSp			= IoGetCurrentIrpStackLocation(Irp);
	deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

	Irp->IoStatus.Information = 0;

	//
	// Get the pointer to the input/output buffer and it's length
	//

	ioBuf			= Irp->AssociatedIrp.SystemBuffer;
	inBufLength		= IrpSp->Parameters.DeviceIoControl.InputBufferLength;

	do 
	{
		if(inBufLength != sizeof(ULONG))
		{
			status = STATUS_INVALID_PARAMETER;
			break;
		}

#ifndef _DEBUG
		if(!is_process_a_service(PsGetCurrentProcess()))
		{
			status = STATUS_PRIVILEGE_NOT_HELD;
			break;
		}
#endif

		pid = *(ULONG*)ioBuf;

		sec_hdr = (PNXRMSECTIONHDR)deviceExtension->rm_k_section;

		sec_hdr->hdr.protected_process_id = pid;

		Global.hProtectedProcess = (HANDLE)pid;

	} while(FALSE);

	return status;
}
