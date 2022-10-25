

#ifndef __NXRM_VHD_UTIL_H__
#define __NXRM_VHD_UTIL_H__

#include <nudf\shared\vhddef.h>
#include "nxrmvhd.h"


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
ULONG
GetCPUCount(
            );

VOID
GetDosNameFromDriveLetter(
                          _Out_writes_(128) LPWSTR DosName,
                          _In_ WCHAR DriveLetter
                          );

VOID
GetNtNameFromNumber(
                    _Out_writes_(128) LPWSTR NtName,
                    _In_ LONG VolumeId
                    );

_Check_return_
BOOLEAN
ValidateIoBufferSize(
                     _In_ PIRP Irp,
                     _In_ size_t RequiredBufferSize,
                     _In_ ValidateIoBufferSizeType Type
                     );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
ULONG
GetAvailableDiskId(
                   );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
ULONG
GetActiveDiskCount(
                   );

_Check_return_
BOOLEAN
IsDriveLetterAvailable(
                       _In_ WCHAR DriveLetter
                       );

_Check_return_
WCHAR
FindAvailableDriveLetter(
                         _In_ WCHAR PreferredDriveLetter
                         );

_Check_return_
NTSTATUS
VhdDeviceIoControl(
                   _In_ PWSTR DeviceName,
                   _In_ ULONG IoControlCode,
                   _In_opt_ PVOID InputBuffer,
                   _In_ ULONG InputBufferSize,
                   _Out_opt_ PVOID OutputBuffer,
                   _In_ ULONG OutputBufferSize
                   );

_Check_return_
NTSTATUS
MountManagerMount(
                  _In_ PDEVICE_OBJECT DeviceObject
                  );

_Check_return_
NTSTATUS
MountManagerUnmount(
                    _In_ PUNICODE_STRING DosName
                    );

_Check_return_
NTSTATUS
VhdCloseDeviceObject(
                     _In_ PDEVICE_OBJECT DeviceObject
                     );

_Check_return_
NTSTATUS
VhdCreateDeviceObject(
                      _In_ PDRIVER_OBJECT DriverObject,
                      _Out_ PDEVICE_OBJECT* ppDeviceObject,
                      _In_ PNXRMVHDMOUNTDRIVE Mount
                      );

_Check_return_
NTSTATUS
VhdOpenVolume(
              _In_ PVHDDISKEXT pExt,
              _In_ PNXRMVHDMOUNTDRIVE Mount
              );

_Check_return_
NTSTATUS
VhdRemoveDriveLink(
                   _In_ PCUNICODE_STRING SymbolicLink
                   );

_Check_return_
NTSTATUS
VhdCreateDriveLink(
                   _In_ PCUNICODE_STRING DeviceName,
                   _In_ PCUNICODE_STRING SymbolicLink
                   );

_Check_return_
NTSTATUS
VhdOpenFsVolume(
                _In_ PVHDDISKEXT pExt, 
                _Out_ PFILE_OBJECT* FileObject
                );

VOID
CloseFsVolume(
              _In_ PFILE_OBJECT FileObject
              );

_Check_return_
NTSTATUS
VhdFsctlCall(
             _In_ PFILE_OBJECT FileObject,
             _In_ LONG IoControlCode,
             _In_opt_ PVOID InputBuffer,
             _In_ ULONG InputBufferSize,
             _Out_opt_ PVOID OutputBuffer,
             _In_ ULONG OutputBufferSize
             );

VOID
VhdSleep(
         _In_ ULONG MilliSeconds
         );


#endif