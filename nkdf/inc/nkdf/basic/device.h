
/**
 * \file <nkdf/basic/device.h>
 * \brief Header file for device control
 *
 * This header file contains routines for device control
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */

#ifndef __NKDF_BASIC_DEVICE_H__
#define __NKDF_BASIC_DEVICE_H__


/**
 * \addtogroup nkdf-basic
 * @{
 */


/**
 * \defgroup nkdf-basic-dev Device Control
 * @{
 */


/**
 * \defgroup nkdf-basic-dev-api Routines
 * @{
 */

/**
 * \brief Function to send an I/O Control request to specific device.
 * \param Device The device who should receive this control code.
 * \param InputBuffer Input buffer (optional).
 * \param InputBufferLength The length of input buffer, it should be zero if InputBuffer is NULL.
 * \param OutputBuffer Output buffer (optional).
 * \param OutputBufferLength The length of output buffer, it should be zero if InputBuffer is NULL.
 * \param InternalDeviceIoControl TRUE if this is a internal device I/O control. Otherwise it hsould be FALSE.
 * \param Iosb Pointer to a IO_STATUS_BLOCK struct to receive result.
 * \return NTSTATUS
 */
_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkDeviceIoControl(
                  _In_ ULONG IoControlCode,
                  _In_ PDEVICE_OBJECT Device,
                  _In_opt_ PVOID InputBuffer,
                  _In_ ULONG InputBufferLength,
                  _Out_opt_ PVOID OutputBuffer,
                  _In_ ULONG OutputBufferLength,
                  _In_ BOOLEAN InternalDeviceIoControl,
                  _In_ BOOLEAN OverrideVerify,
                  _Out_opt_ PIO_STATUS_BLOCK Iosb
                  );



/**@}*/ // Group End: nkdf-basic-dev-api


/**@}*/ // Group End: nkdf-basic-dev


/**@}*/ // Group End: nkdf-basic


#endif  // #ifndef __NKDF_BASIC_DEVICE_H__