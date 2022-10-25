
/**
 * \file <nkdf/fs/fltconn.h>
 * \brief Header file for mini-filter communication support
 *
 * This header file declares routines for mini-filter communication support
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */


#ifndef __NKDF_FS_FLT_CONN_H__
#define __NKDF_FS_FLT_CONN_H__



/**
 * \addtogroup nkdf-fs
 * @{
 */


/**
 * \defgroup nkdf-fs-fltconn Mini-filter Communication Support
 * @{
 */


/**
 * \defgroup nkdf-fs-fltconn-callback Callback Routines
 * @{
 */

#define NKFLT_MAX_CONN      32  /**< Maximum Mini-filter connections */


/** \typedef NKFLT_CONN_MSGDISPATCH_ROUTINE
 *  \brief Mini-filter message dispatch callback routine.
 *  \param InputBuffer Pointer to input buffer (optional).
 *  \param InputBufferLength Input buffer size.
 *  \param OutputBuffer Pointer to output buffer (optional).
 *  \param OutputBufferLength Output buffer size.
 *  \param ReturnOutputBufferLength Size of data written to output buffer.
 *  \return N/A.
 */
typedef NTSTATUS
(*NKFLT_CONN_MSGDISPATCH_ROUTINE)(
                                  _In_opt_ PVOID InputBuffer,
                                  _In_ ULONG InputBufferLength,
                                  _Out_opt_ PVOID OutputBuffer,
                                  _In_ ULONG OutputBufferLength,
                                  _Out_ PULONG ReturnOutputBufferLength
                                  );


/**@}*/ // Group End: nkdf-fs-fltconn-callback


/**
 * \defgroup nkdf-fs-fltconn-api Routines
 * @{
 */


/**
 * \brief Function to start mini-filter communication.
 * \IRQL PASSIVE_LEVEL
 * \param Filter Pointer to filter object.
 * \param Name Port name.
 * \param ServerConn Count of server mode connection. Driver works as a server when use this port: it handles application's request.
 * \param ClientConn Count of client mode connection. Driver works as a client when use this port: it sends request to application and gets reply.
 * \param MsgDispatchRoutine Message dispatch callback routine.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltConnStart(
               _In_ PFLT_FILTER Filter,
               _In_ PCUNICODE_STRING Name,
               _In_ ULONG ServerConn,
               _In_ ULONG ClientConn,
               _In_opt_ NKFLT_CONN_MSGDISPATCH_ROUTINE MsgDispatchRoutine
               );


/**
 * \brief Function to shutdown mini-filter communication.
 * \IRQL PASSIVE_LEVEL
 * \return N/A.
 */
_IRQL_requires_(PASSIVE_LEVEL)
VOID
NkFltConnShutdown(
                  );


/**
 * \brief Function to send a query to user mode service.
 * \IRQL PASSIVE_LEVEL
 * \param SenderBuffer Pointer to sender's buffer.
 * \param SenderBufferLength Size of sender's buffer.
 * \param ReplyBuffer Pointer to reply buffer.
 * \param ReplyLength Pointer to ULONG which contains size of reply buffer. If the call succeeds, it contains the bytes of data replied by user mode service.
 * \param Timeout Timeout value.
 * \return NTSTATUS.
 */
_Check_return_
NTSTATUS
NkFltConnQuery(
               _In_ PVOID SenderBuffer,
               _In_ ULONG SenderBufferLength,
               _Out_opt_ PVOID ReplyBuffer,
               _Inout_ PULONG ReplyLength,
               _In_opt_ PLARGE_INTEGER Timeout
               );



/**@}*/ // Group End: nkdf-fs-fltconn-api


/**@}*/ // Group End: nkdf-fs-fltconn


/**@}*/ // Group End: nkdf-fs


#endif  // #ifndef __NKDF_FS_FLT_CONN_H__