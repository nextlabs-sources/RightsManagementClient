



#include <ntifs.h>
#include <fltkernel.h>

#include <nkdf/basic/defines.h>
#include <nkdf/fs/disk.h>
#include <nkdf/fs/vol.h>





#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NkFltGetVolumeProperties)
#pragma alloc_text(PAGE, NkFltGetVolumeDiskInfo)
#endif




_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltGetVolumeProperties(
                         _In_ PFLT_VOLUME Volume,
                         _Out_ PFLT_VOLUME_PROPERTIES* PropPtr
                         )
{
    NTSTATUS                Status = STATUS_UNSUCCESSFUL;
    ULONG                   LengthReturned = 0;
    PFLT_VOLUME_PROPERTIES  Prop = NULL;

    PAGED_CODE();

    *PropPtr = NULL;

    try {

        Status = FltGetVolumeProperties(Volume, NULL, 0, &LengthReturned);
        if (STATUS_BUFFER_TOO_SMALL != Status) {
            try_return(Status);
        }

        Prop = ExAllocatePoolWithTag(NonPagedPool, LengthReturned + 32, TAG_VOL);
        if (NULL == Prop) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory(Prop, LengthReturned + 32);
        Status = FltGetVolumeProperties(Volume, Prop, LengthReturned + 32, &LengthReturned);
        if (!NT_SUCCESS(Status) && STATUS_BUFFER_OVERFLOW != Status) {
            try_return(Status);
        }

        *PropPtr = Prop;
        Prop = NULL; // transfer the ownership

try_exit: NOTHING;
    }
    finally {

        if (NULL != Prop) {
            ExFreePool(Prop);
            Prop = NULL;
        }
    }

    return Status;
}


_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltGetVolumePropertiesEx(
                           _In_ PFLT_FILTER Filter,
                           _In_ PCUNICODE_STRING VolumeName,
                           _Out_ PFLT_VOLUME_PROPERTIES* PropPtr
                           )
{
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    PFLT_VOLUME Volume = NULL;

    PAGED_CODE();

    *PropPtr = NULL;

    try {

        Status = FltGetVolumeFromName(Filter, VolumeName, &Volume);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = NkFltGetVolumeProperties(Volume, PropPtr);

try_exit: NOTHING;
    }
    finally {

        if (NULL != Volume) {
            FltObjectDereference(Volume);
            Volume = NULL;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
NkFltGetVolumeDiskInfo(
                       _In_ PFLT_VOLUME Volume,
                       _Out_ PWCHAR DriveLetter,
                       _Out_writes_bytes_(sizeof(DISK_GEOMETRY)) PDISK_GEOMETRY DiskGeometry
                       )
{
    NTSTATUS        Status = STATUS_UNSUCCESSFUL;
    PDEVICE_OBJECT  DiskDeviceObject = NULL;


    PAGED_CODE();


    try {

        Status = FltGetDiskDeviceObject(Volume, &DiskDeviceObject);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        ASSERT(NULL != DiskDeviceObject);

        Status = NkGetDriveLetter(DiskDeviceObject, DriveLetter);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = NkGetDiskGeometry(DiskDeviceObject, DiskGeometry);


try_exit: NOTHING;
    }
    finally {

        if (NULL != DiskDeviceObject) {
            ObDereferenceObject(DiskDeviceObject);
            DiskDeviceObject = NULL;
        }
    }

    return Status;
}


_Check_return_
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
NkFltGetVolumeDiskInfoEx(
                         _In_ PFLT_FILTER Filter,
                         _In_ PCUNICODE_STRING VolumeName,
                         _Out_ PWCHAR DriveLetter,
                         _Out_writes_bytes_(sizeof(DISK_GEOMETRY)) PDISK_GEOMETRY DiskGeometry
                         )
{
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    PFLT_VOLUME Volume = NULL;

    PAGED_CODE();

    try {

        Status = FltGetVolumeFromName(Filter, VolumeName, &Volume);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = NkFltGetVolumeDiskInfo(Volume, DriveLetter, DiskGeometry);

try_exit: NOTHING;
    }
    finally {

        if (NULL != Volume) {
            FltObjectDereference(Volume);
            Volume = NULL;
        }
    }

    return Status;
}

_Check_return_
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
NkFltGetVolumeDriveLetter(
                          _In_ PFLT_VOLUME Volume,
                          _Out_ PWCHAR DriveLetter
                          )
{
    NTSTATUS        Status = STATUS_UNSUCCESSFUL;
    PDEVICE_OBJECT  DiskDeviceObject = NULL;


    PAGED_CODE();


    try {

        Status = FltGetDiskDeviceObject(Volume, &DiskDeviceObject);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        ASSERT(NULL != DiskDeviceObject);

        Status = NkGetDriveLetter(DiskDeviceObject, DriveLetter);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

try_exit: NOTHING;
    }
    finally {

        if (NULL != DiskDeviceObject) {
            ObDereferenceObject(DiskDeviceObject);
            DiskDeviceObject = NULL;
        }
    }

    return Status;
}


_Check_return_
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
NkFltVolumeNameToDriveLetter(
                             _In_ PFLT_FILTER Filter,
                             _In_ PCUNICODE_STRING VolumeName,
                             _Out_ PWCHAR DriveLetter
                             )
{
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    PFLT_VOLUME Volume = NULL;

    PAGED_CODE();

    try {

        Status = FltGetVolumeFromName(Filter, VolumeName, &Volume);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = NkFltGetVolumeDriveLetter(Volume, DriveLetter);

try_exit: NOTHING;
    }
    finally {

        if (NULL != Volume) {
            FltObjectDereference(Volume);
            Volume = NULL;
        }
    }

    return Status;
}