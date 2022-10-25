
/**
 * \file <nkdf/fs/disk.h>
 * \brief Header file for disk support routines
 *
 * This header file declare disk support routines
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */


#ifndef __NKDF_FS_DISK_H__
#define __NKDF_FS_DISK_H__

#include <ntifs.h>
#include <Ntdddisk.h>


/**
 * \addtogroup nkdf-fs
 * @{
 */


/**
 * \defgroup nkdf-fs-disk Disk Support
 * @{
 */


/**
 * \defgroup nkdf-fs-disk-api Routines
 * @{
 */


/**
 * \brief Function to get disk geometry.
 * \IRQL Less than or equal to APC_LEVEL
 * \param DiskDeviceObject Pointer to disk device object.
 * \param DiskGeometry Pointer to DISK_GEOMETRY struct to receive result.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkGetDiskGeometry (
                   _In_ PDEVICE_OBJECT DiskDeviceObject,
                   _Out_writes_bytes_(sizeof(DISK_GEOMETRY)) PDISK_GEOMETRY DiskGeometry
                   );


/**
 * \brief Function to get volume's drive letter.
 * \IRQL Less than or equal to APC_LEVEL
 * \param DiskDeviceObject Pointer to disk device object.
 * \param DiskGeometry Pointer to WCHAR to receive drive letter.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkGetDriveLetter(
                 _In_ PDEVICE_OBJECT DiskDeviceObject,
                 _Out_writes_bytes_(sizeof(WCHAR)) PWCHAR DriveLetter
                 );


/**@}*/ // Group End: nkdf-fs-disk-api


/**@}*/ // Group End: nkdf-fs-disk


/**@}*/ // Group End: nkdf-fs


#endif  // #ifndef __NKDF_FS_DISK_H__