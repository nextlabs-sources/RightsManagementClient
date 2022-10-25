
/**
 * \file <nkdf/basic/csq.h>
 * \brief Header file for CSQ in libkbasic library
 *
 * This is the header file defining CSQ in libkbasic library.
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */


#ifndef __NKDF_BASIC_CSQ_H__
#define __NKDF_BASIC_CSQ_H__


/**
 * \addtogroup nkdf-basic
 * @{
 */


/**
 * \defgroup nkdf-basic-csq Cancel Safe Queue
 * @{
 */


/**
 * \defgroup nkdf-basic-csq-struct Structures
 * @{
 */

/**
 * \typedef typedef struct _NKCSQ* PNKCSQ;
 * \brief NKCSQ Struct Pointer
 * This is a opaque struct defined/used by NKDF. 
 */
typedef struct _NKCSQ*      PNKCSQ;

/**@}*/ // Group End: nkdf-basic-csq-struct



/**
 * \defgroup nkdf-basic-csq-api Routines
 * @{
 */

/**
 * \IRQL Less or equal to APC_LEVEL
 * \brief Function to create NkCsq object.
 * \param DeviceObject The device object who owns this Cancel-Safe-Queue
 * \param IrpDispatch The IRP dispatch routine
 * \param Csq The address of a NKCSQ object pointer to receive newly created NKCSQ object.
 * \return NTSTATUS 
 */
_IRQL_requires_max_(APC_LEVEL)
_Check_return_
NTSTATUS
NkCsqCreate(
            _In_ PDEVICE_OBJECT DeviceObject,
            _In_ PDRIVER_DISPATCH IrpDispatch,
            _Out_ PNKCSQ* Csq
            );

/**
 * \IRQL Less or equal to APC_LEVEL
 * \brief Function to create NkCsq object.
 * \param Csq Pointer to CSQ object.
 * \return N/A
 */
_IRQL_requires_max_(APC_LEVEL)
VOID
NkCsqDestroy(
             _In_ PNKCSQ Csq
             );

/**
 * \IRQL Less or equal to APC_LEVEL
 * \brief Insert an IRP to CSQ.
 * \param Csq Pointer to CSQ object.
 * \param Irp The IRP to insert.
 * \return NTSTATUS
 */
_IRQL_requires_max_(DISPATCH_LEVEL)
_Check_return_
NTSTATUS
NkCsqInsert(
             _In_ PNKCSQ Csq,
             _Inout_ PIRP Irp
             );

/**@}*/ // Group End: nkdf-basic-csq-api


/**@}*/ // Group End: nkdf-basic-csq


/**@}*/ // Group End: nkdf-basic


#endif  // #ifndef __NKDF_BASIC_CSQ_H__