
/**
 * \file <nkdf/fs/vol.h>
 * \brief Header file for volume support routines
 *
 * This header file declare volume support routines
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */

#ifndef __NKDF_FS_VOL_H__
#define __NKDF_FS_VOL_H__


#include <fltkernel.h>
#include <Ntdddisk.h>


/**
 * \addtogroup nkdf-fs
 * @{
 */


/**
 * \defgroup nkdf-fs-vol Volume Support
 * @{
 */


/**
 * \defgroup nkdf-fs-vol-api Routines
 * @{
 */



/**
 * \brief Function to get volume properties.
 * \IRQL Less than or equal to APC_LEVEL
 * \param Volume Pointer to FLT_VOLUME object.
 * \param PropPtr Pointer to PFLT_VOLUME_PROPERTIES which allocated by this function.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltGetVolumeProperties(
                         _In_ PFLT_VOLUME Volume,
                         _Out_ PFLT_VOLUME_PROPERTIES* PropPtr
                         );

/**
 * \brief Function to get volume properties by volume name.
 * \IRQL Less than or equal to APC_LEVEL
 * \param Volume Pointer to FLT_VOLUME object.
 * \param VolumeName Volume name.
 * \param PropPtr Pointer to PFLT_VOLUME_PROPERTIES which allocated by this function.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltGetVolumePropertiesEx(
                           _In_ PFLT_FILTER Filter,
                           _In_ PCUNICODE_STRING VolumeName,
                           _Out_ PFLT_VOLUME_PROPERTIES* PropPtr
                           );

/**
 * \brief Function to get volume's disk information.
 * \IRQL PASSIVE_LEVEL
 * \param Volume Pointer to FLT_VOLUME object.
 * \param DriveLetter Pointer to WCHAR to receive drive letter.
 * \param DiskGeometry Pointer to DISK_GEOMETRY to receive disk geometry.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
NkFltGetVolumeDiskInfo(
                       _In_ PFLT_VOLUME Volume,
                       _Out_ PWCHAR DriveLetter,
                       _Out_writes_bytes_(sizeof(DISK_GEOMETRY)) PDISK_GEOMETRY DiskGeometry
                       );


/**
 * \brief Function to get volume's disk information by volume name.
 * \IRQL PASSIVE_LEVEL
 * \param Filter Mini-filter object.
 * \param VolumeName Volume name.
 * \param DriveLetter Pointer to WCHAR to receive drive letter.
 * \param DiskGeometry Pointer to DISK_GEOMETRY to receive disk geometry.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
NkFltGetVolumeDiskInfoEx(
                         _In_ PFLT_FILTER Filter,
                         _In_ PCUNICODE_STRING VolumeName,
                         _Out_ PWCHAR DriveLetter,
                         _Out_writes_bytes_(sizeof(DISK_GEOMETRY)) PDISK_GEOMETRY DiskGeometry
                         );


/**
 * \brief Function to get volume's drive letter.
 * \IRQL PASSIVE_LEVEL
 * \param Volume Pointer to FLT_VOLUME object.
 * \param DriveLetter Pointer to WCHAR to receive drive letter.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
NkFltGetVolumeDriveLetter(
                          _In_ PFLT_VOLUME Volume,
                          _Out_ PWCHAR DriveLetter
                          );


/**
 * \brief Function to get volume's drive letter by volume's name.
 * \IRQL PASSIVE_LEVEL
 * \param Filter Mini-filter object.
 * \param VolumeName Volume name.
 * \param DriveLetter Pointer to WCHAR to receive drive letter.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
NkFltVolumeNameToDriveLetter(
                             _In_ PFLT_FILTER Filter,
                             _In_ PCUNICODE_STRING VolumeName,
                             _Out_ PWCHAR DriveLetter
                             );

/**@}*/ // Group End: nkdf-fs-vol-api


/**@}*/ // Group End: nkdf-fs-vol


/**@}*/ // Group End: nkdf-fs



#endif  // #ifndef __NKDF_FS_VOL_H__