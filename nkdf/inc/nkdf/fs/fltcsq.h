
/**
 * \file <nkdf/fs/fltcsq.h>
 * \brief Header file for mini-filter csq support
 *
 * This header file declares mini-filter csq support routines
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */

#ifndef __NKDF_FS_FLT_CSQ_H__
#define __NKDF_FS_FLT_CSQ_H__

#include <fltkernel.h>



/**
 * \addtogroup nkdf-fs
 * @{
 */


/**
 * \defgroup nkdf-fs-fltcsq Mini-filter CSQ
 * @{
 */


/**
 * \defgroup nkdf-fs-fltcsq-typedef Typedefs and Structs
 * @{
 */

typedef struct _NKFLTCSQ*    PNKFLTCSQ; /**< Pointer of _NKFLTCSQ */


/**
 * \struct _NKFLTCSQ_CONTEXT
 */
struct _NKFLTCSQ_CONTEXT {
    FLT_CALLBACK_DATA_QUEUE_IO_CONTEXT  CbdqIoContext;  /**< FLt-callback Io context */
    BOOLEAN                             IsPreCb;        /**< Is pre-callback */
    FLT_PREOP_CALLBACK_STATUS           PreCbStatus;    /**< Pre-callback status */
    ULONG                               UserContextSize;/**< User context size */
    PVOID                               UserContext;    /**< User context */
};

typedef struct _NKFLTCSQ_CONTEXT NKFLTCSQ_CONTEXT;      /**< NKFLTCSQ_CONTEXT */
typedef struct _NKFLTCSQ_CONTEXT* PNKFLTCSQ_CONTEXT;    /**< PNKFLTCSQ_CONTEXT */

/**@}*/ // Group End: nkdf-fs-fltcsq-typedef


/**
 * \defgroup nkdf-fs-fltcsq-callback Callback Routines
 * @{
 */


/** \typedef NKFLTCSQ_PRE_CALLBACK
 *  \brief Csq pre-callback routine.
 *  \IRQL PASSIVE_LEVEL.
 *  \param Data Pointer to FLT_CALLBACK_DATA.
 *  \param Csq Pointer to NKFLTCSQ.
 *  \param UserContext Pointer to user context buffer.
 *  \param UserContextSize Size of user context
 *  \param CbContext Pointer to PVOID to receive callback context. CbContext is only valid when CbStatus is FLT_PREOP_SUCCESS_WITH_CALLBACK
 *  \return FLT_PREOP_CALLBACK_STATUS (It never return FLT_PREOP_PENDING).
 */
typedef
FLT_PREOP_CALLBACK_STATUS
NTAPI
NKFLTCSQ_PRE_CALLBACK (
                       _Inout_ PFLT_CALLBACK_DATA Data,
                       _In_ PNKFLTCSQ Csq,
                       _In_opt_ PVOID UserContext,
                       _In_ ULONG UserContextSize,
                       _Out_ PVOID* CbContext
                       );
typedef NKFLTCSQ_PRE_CALLBACK*  PNKFLTCSQ_PRE_CALLBACK;     /**< Function pointer */

/** \typedef NKFLTCSQ_POST_CALLBACK
 *  \brief Csq post-callback routine.
 *  \IRQL PASSIVE_LEVEL.
 *  \param Data Pointer to FLT_CALLBACK_DATA.
 *  \param Csq Pointer to NKFLTCSQ.
 *  \param UserContext Pointer to user context buffer.
 *  \param UserContextSize Size of user context
 *  \return N/A
 */
typedef
VOID
NTAPI
NKFLTCSQ_POST_CALLBACK (
                        _Inout_ PFLT_CALLBACK_DATA Data,
                        _In_ PNKFLTCSQ Csq,
                        _In_opt_ PVOID UserContext,
                        _In_ ULONG UserContextSize
                        );
typedef NKFLTCSQ_POST_CALLBACK*  PNKFLTCSQ_POST_CALLBACK;   /**< Function pointer */

/**@}*/ // Group End: nkdf-fs-fltcsq-callback


/**
 * \defgroup nkdf-fs-fltcsq-api Routines
 * @{
 */


/**
 * \brief Function to create a NKFLTCSQ object.
 * \IRQL Less than or equal to APC_LEVEL
 * \param Filter Pointer to filter object.
 * \param Instance Filter instance.
 * \param CsqPreCallback Pre-callback routine.
 * \param CsqPostCallback Post-callback routine.
 * \param Csq Pointer to PNKFLTCSQ to receive newly created NKFLTCSQ object.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltCsqCreate(
               _In_ PFLT_FILTER Filter,
               _In_ PFLT_INSTANCE Instance,
               _In_ PNKFLTCSQ_PRE_CALLBACK CsqPreCallback,
               _In_ PNKFLTCSQ_POST_CALLBACK CsqPostCallback,
               _Out_ PNKFLTCSQ* Csq
               );

/**
 * \brief Function to create a NKFLTCSQ object.
 * \IRQL Less than or equal to APC_LEVEL
 * \param Csq Pointer to NKFLTCSQ object.
 * \return N/A.
 */
_IRQL_requires_max_(APC_LEVEL)
VOID
NkFltCsqDestroy(
                _In_ PNKFLTCSQ Csq
                );

/**
 * \brief Function to create a NKFLTCSQ object.
 * \IRQL Less than or equal to DISPATCH_LEVEL
 * \param Data Pointer to FLT_CALLBACK_DATA.
 * \param Csq Pointer to NKFLTCSQ object.
 * \param IsPreCb Is this a pre-callback operation.
 * \param UserContext Pointer to user context buffer.
 * \param UserContextSize Size of user context buffer.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkFltCsqInsert(
               _Inout_ PFLT_CALLBACK_DATA Data,
               _In_ PNKFLTCSQ Csq,
               _In_ BOOLEAN IsPreCb,
               _In_opt_ PVOID UserContext,
               _In_ ULONG UserContextSize
               );


/**@}*/ // Group End: nkdf-fs-fltcsq-api


/**@}*/ // Group End: nkdf-fs-fltcsq


/**@}*/ // Group End: nkdf-fs


#endif  // #ifndef __NKDF_FS_FLT_CSQ_H__