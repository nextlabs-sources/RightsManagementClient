

#ifndef __NXRM_VHD_H__
#define __NXRM_VHD_H__

#include <nudf\shared\vhddef.h>

#define VHD_MAX_THREADS         4
#define VHD_MAX_NAME_LENGTH     128


#define try_return(S)   {S; goto try_exit;}
#define try_return2(S)   {S; goto try_exit2;}



#pragma pack(push,8)

typedef struct _VHDMGR {

	PDEVICE_OBJECT	VhdMgrDevice;

	LIST_ENTRY		IoQueue;
	KSPIN_LOCK		IoQueueLock;
	KSEMAPHORE		IoQueueSem;	
	KEVENT			IoQueueStopEvent;
	PETHREAD 		IoThread[VHD_MAX_THREADS];
	CLIENT_ID		IoThreadId[VHD_MAX_THREADS];

	ULONG			ActiveProcessors;

    // Virtual Disks
	ERESOURCE       VhdDiskLock;
    ULONG           VhdDiskCount;
    LIST_ENTRY      VhdDiskList;

} VHDMGR, *PVHDMGR;


typedef struct _VHDDISKEXT {

	ULONG			StopPending;

    LIST_ENTRY      Link;

	IO_REMOVE_LOCK	IoRemoveLock;
	volatile ULONG  OutstandingIoCount;
	KEVENT			NoOutstandingIoEvent;

	ULONG			DiskId;	
	WCHAR			DosDriveLetter;
    UNICODE_STRING  DosDeviceName;
	WCHAR			DosDeviceNameBuf[VHD_MAX_NAME_LENGTH];
    UNICODE_STRING  NtDeviceName;
	WCHAR			NtDeviceNameBuf[VHD_MAX_NAME_LENGTH];

	LARGE_INTEGER	DiskLength;			    // The length of the disk referred to by this device
	LARGE_INTEGER	NumberOfCylinders;		// Partition info	
	ULONG			TracksPerCylinder;		// Partition info	
	ULONG			SectorsPerTrack;		// Partition info
	ULONG			BytesPerSector;			// Partition info	
	ULONG			PartitionType;			// Partition info	
	ULONG			HostBytesPerSector;		// For the host partition

	ULONG			Visible;
	ULONG			Removable;
	HANDLE			VhdFileHandle;
	PDEVICE_OBJECT	VolumeDevice;

} VHDDISKEXT, *PVHDDISKEXT;


typedef enum {
	ValidateInput = 0x100,
	ValidateOutput,
	ValidateInputOutput
} ValidateIoBufferSizeType;

#pragma pack(pop)

extern VHDMGR _VhdMgr;

__forceinline
VOID
IncrementOutstandingIoCount(
                            _Inout_ PVHDDISKEXT pExt
                            )
{
	InterlockedIncrement((volatile LONG*)&pExt->OutstandingIoCount);
}

__forceinline
VOID
DecrementOutstandingIoCount(
                            _Inout_ PVHDDISKEXT pExt
                            )
{
	InterlockedDecrement((volatile LONG*)&pExt->OutstandingIoCount);
}


#endif