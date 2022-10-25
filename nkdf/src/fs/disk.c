


#include <ntifs.h>

#include <nkdf/basic.h>
#include <nkdf/fs/disk.h>





#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NkGetDiskGeometry)
#pragma alloc_text(PAGE, NkGetDriveLetter)
#endif



_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkGetDiskGeometry(
                  _In_ PDEVICE_OBJECT DiskDeviceObject,
                  _Out_writes_bytes_(sizeof(DISK_GEOMETRY)) PDISK_GEOMETRY DiskGeometry
                  )
{
    PAGED_CODE();

    return NkDeviceIoControl(IOCTL_DISK_GET_DRIVE_GEOMETRY,
                             DiskDeviceObject,
                             NULL,
                             0,
                             DiskGeometry,
                             sizeof(DISK_GEOMETRY),
                             FALSE,
                             FALSE,
                             NULL);
}




_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkGetDriveLetter(
                 _In_ PDEVICE_OBJECT DiskDeviceObject,
                 _Out_writes_bytes_(sizeof(WCHAR)) PWCHAR DriveLetter
                 )
{
    NTSTATUS        Status = STATUS_UNSUCCESSFUL;
    UNICODE_STRING  DosName = { 0, 0, NULL };
    WCHAR           Drive;


    PAGED_CODE();


    try {

        if (KeAreAllApcsDisabled()) {
            try_return(Status = STATUS_UNSUCCESSFUL);
        }

        Status = IoVolumeDeviceToDosName(DiskDeviceObject, &DosName);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        if (0 == DosName.Length || NULL == DosName.Buffer) {
            try_return(Status = STATUS_UNSUCCESSFUL);
        }

        if (DosName.Length < (2 * sizeof(WCHAR))) {
            try_return(Status = STATUS_NOT_FOUND);
        }

        Drive = RtlUpcaseUnicodeChar(DosName.Buffer[0]);
        if (':' != DosName.Buffer[1] || Drive<L'A' || Drive>L'Z') {
            // Not normal drive letter "C:" to "Z:"?
            try_return(Status = STATUS_NOT_FOUND);
        }

        //
        //  Get valid drive letter
        //
        *DriveLetter = Drive;
        Status = STATUS_SUCCESS;

try_exit: NOTHING;
    }
    finally {

        if (NULL != DosName.Buffer) {
            ExFreePool(DosName.Buffer);
        }
    }

    return Status;
}