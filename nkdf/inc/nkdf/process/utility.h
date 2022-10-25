
/**
 * \file <nkdf/process/utility.h>
 * \brief Header file for process utility routines
 *
 * This header file declares process utility routines
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */


#ifndef __NKDF_PROCESS_UTILITY_H__
#define __NKDF_PROCESS_UTILITY_H__

#include <ntifs.h>



/**
 * \addtogroup nkdf-proc
 * @{
 */


/**
 * \defgroup nkdf-proc-util Utilities
 * @{
 */

#ifndef MAX_IMAGE_NAME
#define MAX_IMAGE_NAME  280     /**< MAX_PATH (260) plus volume name (20) */
#endif

#ifndef MAX_USER_NAME
#define MAX_USER_NAME   128     /**< maximum user name (64) plus maximum domain name (64) */
#endif

#ifndef MAX_SID_NAME
#define MAX_SID_NAME    64      /**< Maximum SID size (Sid string is normally 48 characters) */
#endif



/**
 * \defgroup nkdf-proc-util-api Routines
 * @{
 */

_Check_return_
PEPROCESS
NkGetThreadProcess(
                   _In_ PETHREAD Thread
                   );

_Check_return_
HANDLE
NkGetThreadId(
              _In_ PETHREAD Thread
              );

_Check_return_
HANDLE
NkGetThreadProcessId(
                     _In_ PETHREAD Thread
                     );

_Check_return_
NTSTATUS
NkProcessHandleToId(
                    _In_ HANDLE ProcessHandle,
                    _Out_ PHANDLE ProcessId
                    );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkGetProcessImageName(
                      _In_ PEPROCESS Process,
                      _Out_writes_z_(MAX_IMAGE_NAME) PWCH ImageName
                      );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkGetProcessImageNameById(
                          _In_ HANDLE ProcessId,
                          _Out_writes_z_(MAX_IMAGE_NAME) PWCH ImageName
                          );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkGetProcessImageNameByHandle(
                              _In_ HANDLE Handle,
                              _Out_writes_z_(MAX_IMAGE_NAME) PWCH ImageName
                              );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkGetProcessInfo(
                 _In_ HANDLE ProcessId,
                 _Out_ PULONG SessionId,
                 _Out_writes_z_(MAX_USER_NAME) PWCH UserName,
                 _Out_writes_z_(MAX_SID_NAME) PWCH UserSid,
                 _Out_writes_z_(MAX_IMAGE_NAME) PWCH ImageName
                 );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkOpenProcess(
              _In_ HANDLE ProcessId,
              _In_ ACCESS_MASK DesiredAccess,
              _Out_ PHANDLE ProcessHandle
              );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkTerminateProcess(
                   _In_ HANDLE ProcessId,
                   _In_ NTSTATUS ExitStatus
                   );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkIsNativeProcess(
                  _In_ HANDLE ProcessHandle,
                  _Out_ PBOOLEAN Result
                  );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
BOOLEAN
NkIs64bitProcess(
                 _In_ HANDLE ProcessHandle
                 );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkIsSystemProcess(
                  _In_ HANDLE ProcessHandle,
                  _Out_ PBOOLEAN Result
                  );


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkGetProcessEntryPoint(
                       _In_ HANDLE ProcessHandle,
                       _Out_ PVOID* EntryPoint
                       );


/**@}*/ // Group End: nkdf-proc-util-api


/**@}*/ // Group End: nkdf-proc-util


/**@}*/ // Group End: nkdf-proc


#endif  // __NKDF_PROCESS_UTILITY_H__