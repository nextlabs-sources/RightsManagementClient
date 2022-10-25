
/**
 * \file <nkdf/process/utility.h>
 * \brief Header file for process monitor
 *
 * This header file declares process monitor routines
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */


#ifndef __NKDF_PROCESS_MONITOR_H__
#define __NKDF_PROCESS_MONITOR_H__

#include <ntifs.h>




/**
 * \addtogroup nkdf-proc
 * @{
 */


/**
 * \defgroup nkdf-proc-mon Monitor
 * @{
 */


#ifndef MAX_USER_NAME
#define MAX_USER_NAME   128     /**< maximum user name (64) plus maximum domain name (64) */
#endif

#ifndef MAX_SID_LENGTH
#define MAX_SID_LENGTH  64      /**< Maximum SID size (Sid string is normally 48 characters) */
#endif


/**
 * \defgroup nkdf-proc-mon-def Definitions
 * @{
 */


/**
 * \struct _PROCESS_INFOW
 */
struct _PROCESS_INFOW {
    ULONG   ProcessId;          /**< Process id */
    ULONG   SessionId;          /**< Session id */
    ULONG   Flags;              /**< Process flags */
    WCHAR   ImageFileName[280]; /**< Image file name (full path) */
    WCHAR   UserName[128];      /**< User name */
    WCHAR   UserSid[64];        /**< User sid */
};


typedef struct _PROCESS_INFOW           PROCESS_INFOW;      /**< Struct */
typedef struct _PROCESS_INFOW*          PPROCESS_INFOW;     /**< Struct pointer */
typedef const struct _PROCESS_INFOW*    PCPROCESS_INFOW;    /**< Const struct pointer */


/**
 * \struct _NKPROCESS_CONTEXT
 */
struct _NKPROCESS_CONTEXT {
    ULONG       Size;   /**< Context size */
    PVOID       Data;   /**< Context data */
};

typedef struct _NKPROCESS_CONTEXT   NKPROCESS_CONTEXT;      /**< Struct */
typedef struct _NKPROCESS_CONTEXT*  PNKPROCESS_CONTEXT;     /**< Struct pointer */


/**
 * \defgroup nkdf-proc-mon-def-flag Monitor Flags
 * @{
 */
#define PROCESS_MONITOR_USER_PROCESS        0x00000001      /**< Monitor user process */
#define PROCESS_MONITOR_SYSTEM_PROCESS      0x00000002      /**< Monitor system process */
/**@}*/ // Group End: nkdf-proc-mon-def-flag


/**@}*/ // Group End: nkdf-proc-mon-def



/**
 * \defgroup nkdf-proc-mon-callback Callback Routines
 * @{
 */


typedef
_IRQL_requires_same_
_Function_class_(PM_ON_CREATE_FUNC)
_IRQL_requires_(PASSIVE_LEVEL)
VOID
(*PROCESS_CREATE_CALLBACK) (
                            _In_ HANDLE ProcessId,
                            _In_ PCPROCESS_INFOW Info,
                            _In_opt_ PVOID Context,
                            _In_ ULONG ContextSize,
                            _Out_ PBOOLEAN Deny
                            );

typedef
_IRQL_requires_same_
_Function_class_(PM_ON_DESTROY_FUNC)
_IRQL_requires_(PASSIVE_LEVEL)
VOID
(*PROCESS_DESTROY_CALLBACK) (
                             _In_ HANDLE ProcessId,
                             _In_ PCPROCESS_INFOW Info,
                             _In_opt_ PVOID Context,
                             _In_ ULONG ContextSize
                             );

/**@}*/ // Group End: nkdf-proc-mon-callback


/**
* \defgroup nkdf-proc-mon-api Routines
* @{
*/

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkProcessMonitorStart(
                      _In_ ULONG Flags,
                      _In_ ULONG ContextSize,
                      _In_ ULONG ContextTag,
                      _In_opt_ PROCESS_CREATE_CALLBACK CreateCallback,
                      _In_opt_ PROCESS_DESTROY_CALLBACK DestroyCallback
                      );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkProcessMonitorStop(
                     );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFindProcessInformation(
                         _In_ HANDLE ProcessId,
                         _Out_writes_bytes_opt_(sizeof(PROCESS_INFOW)) PPROCESS_INFOW Info
                         );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkGetProcessFlags(
                  _In_ HANDLE ProcessId,
                  _Out_ PULONG Flags
                  );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkSetProcessFlags(
                  _In_ HANDLE ProcessId,
                  _In_ ULONG NewFlags
                  );


_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkRemoveProcessFlags(
                     _In_ HANDLE ProcessId,
                     _In_ ULONG Flags
                     );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkAcquireProcessContext(
                        _In_ HANDLE ProcessId,
                        _In_ BOOLEAN Exclusive,
                        _Out_ PNKPROCESS_CONTEXT* PtrContext
                        );

_IRQL_requires_max_(DISPATCH_LEVEL)
VOID
NkReleaseProcessContext(
                        _In_ PNKPROCESS_CONTEXT ProcContext
                        );



/**@}*/ // Group End: nkdf-proc-mon-api


/**@}*/ // Group End: nkdf-proc-mon


/**@}*/ // Group End: nkdf-proc




#endif  // __NKDF_PROCESS_MONITOR_H__