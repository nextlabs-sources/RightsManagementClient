
/**
 * \file <nkdf/basic/gtable.h>
 * \brief Header file for safe generic table
 *
 * This header file defines Safe Generic Table object and provides related routines
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */

#ifndef __NKDF_BASIC_SAFE_GENERIC_TABLE_H__
#define __NKDF_BASIC_SAFE_GENERIC_TABLE_H__


#include <ntifs.h>


/**
 * \addtogroup nkdf-basic
 * @{
 */


/**
 * \defgroup nkdf-basic-gtable Safe Generic Table
 * @{
 */


/**
 * \defgroup nkdf-basic-gtable-callback Callbacks
 * @{
 */


/** \typedef NKGT_TRAVERSE_ROUTINE
 *  \brief NK generic Table traverse callback routine.
 *  \param Table Pointer to RTL_GENERIC_TABLE.
 *  \param Element Pointer to an element in the table.
 *  \param Continue Pointer to a BOOLEAN variable. If the routine set it to FALSE, the traverse will stop.
 *  \param TraverseContext Context defined by caller.
 *  \return N/A.
 */
typedef
_IRQL_requires_same_
_Function_class_(NKGT_TRAVERSE_ROUTINE)
VOID
NTAPI
NKGT_TRAVERSE_ROUTINE(
                      _In_ struct _RTL_GENERIC_TABLE *Table,
                      _In_ PVOID Element,
                      _Out_ PBOOLEAN Continue,
                      _Inout_opt_ PVOID TraverseContext
                      );
typedef NKGT_TRAVERSE_ROUTINE* PNKGT_TRAVERSE_ROUTINE; /**< Function pointer */

/** \typedef NKGT_OPERATE_ROUTINE
 *  \brief NK generic Table operate callback routine
 *  \param Table Pointer to RTL_GENERIC_TABLE.
 *  \param Element Pointer to an element in the table.
 *  \param OperateContext Context defined by caller.
 *  \return N/A.
 */
typedef
_IRQL_requires_same_
_Function_class_(NKGT_OPERATE_ROUTINE)
VOID
NTAPI
NKGT_OPERATE_ROUTINE(
                     _In_ struct _RTL_GENERIC_TABLE *Table,
                     _In_ PVOID Element,
                     _Inout_opt_ PVOID OperateContext
                     );
typedef NKGT_OPERATE_ROUTINE* PNKGT_OPERATE_ROUTINE; /**< Function pointer */



/**@}*/ // Group End: nkdf-basic-dev-callback



/**
 * \defgroup nkdf-basic-gtable-api Routines
 * @{
 */

/**
 * \brief Function to create a safe generic table.
 * \param TableHandle Pointer to a HANDLE object which will receive the handle to the newly created safe generic table.
 * \param CompareRoutine Compare routine used by the generic table.
 * \param ElementSize The lsize of table element.
 * \param PoolType Type of the memory allocated by this table.
 * \param PoolTag Tag of the memory allocated by this table.
 * \return NTSTATUS
 */
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCreateSafeGenericTable(
                         _Out_ PHANDLE TableHandle,
                         _In_ PRTL_GENERIC_COMPARE_ROUTINE CompareRoutine,
                         _In_ ULONG ElementSize,
                         _In_ POOL_TYPE PoolType,
                         _In_ ULONG PoolTag
                         );

/**
 * \brief Destroy an existing safe generic table.
 * \param TableHandle HANDLE of the safe generic table.
 * \return N/A
 */
_IRQL_requires_(PASSIVE_LEVEL)
VOID
NkDestroySafeGenericTable(
                          _In_ HANDLE TableHandle
                          );

/**
 * \brief Check if the table is empty.
 * \param TableHandle HANDLE of the safe generic table.
 * \return TRUE if the table is empty. Otherwise it returns FALSE.
 */
_Check_return_
_IRQL_requires_max_(APC_LEVEL)
BOOLEAN
NkIsGenericTableEmpty(
                      _In_ HANDLE TableHandle
                      );

/**
 * \brief Get the number of the elements this table has.
 * \param TableHandle HANDLE of the safe generic table.
 * \return Return how many elements are in the table.
 */
_Check_return_
_IRQL_requires_max_(APC_LEVEL)
ULONG
NkNumberGenericTableElements(
                             _In_ HANDLE TableHandle
                             );

/**
 * \brief Insert an element to the table.
 * \param TableHandle HANDLE of the safe generic table.
 * \param Buffer The element buffer.
 * \param BufferSize Size of the element buffer.
 * \return NTSTATUS. Following are possible return values:
 *         STTAUS_SUCCESS: Element has been inserted.
 *         STTAUS_INSUFFICIENT_RESOURCES: Fail to insert element.
 *         STATUS_OBJECTID_EXISTS: Object already exists, element is not inserted.
 */
_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkInsertElementGenericTable(
                            _In_ HANDLE TableHandle,
                            _In_ PVOID Buffer,
                            _In_ CLONG BufferSize
                            );

/**
 * \brief Delete an element to the table.
 * \param TableHandle HANDLE of the safe generic table.
 * \param Buffer The element buffer.
 * \param OperateRoutine Operate callback routine which will be called before the element is deleted.
 * \param OperateContext Context of the operate callback routine which is defined by caller.
 * \return NTSTATUS. Following are possible return values:
 *         STTAUS_SUCCESS: Element has been deleted.
 *         STATUS_NOT_FOUND: Element not exists.
 */
_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkDeleteElementGenericTable(
                            _In_ HANDLE TableHandle,
                            _In_ PVOID Buffer,
                            _In_opt_ PNKGT_OPERATE_ROUTINE OperateRoutine,
                            _Inout_opt_ PVOID OperateContext
                            );

/**
 * \brief Get an element in the table.
 * \param TableHandle HANDLE of the safe generic table.
 * \param Index The index of the element to get.
 * \param Exclusive Is the access exclusive.
 * \param OperateRoutine Callback routine which will be called if the element is found.
 * \param OperateContext Context of the operate callback routine which is defined by caller.
 * \return TRUE if the function found the element. Otherwise it returns FALSE.
 */
_Check_return_
_IRQL_requires_max_(APC_LEVEL)
BOOLEAN
NkGetElementGenericTable(
                         _In_ HANDLE TableHandle,
                         _In_ ULONG Index,
                         _In_ BOOLEAN Exclusive,
                         _In_ PNKGT_OPERATE_ROUTINE OperateRoutine,
                         _Inout_opt_ PVOID OperateContext
                         );

/**
 * \brief Function searches a generic table for an element that matches the specified data, and execute the callback routine on the element found.
 * \param TableHandle HANDLE of the safe generic table.
 * \param Buffer A buffer of search data to pass to the CompareRoutine that was registered when NkCreateSafeGenericTable initialized the table.
 * \param Exclusive Is the access exclusive.
 * \param OperateRoutine Callback routine which will be called if the element is found.
 * \param OperateContext Context of the operate callback routine which is defined by caller.
 * \return TRUE if the function found the element. Otherwise it returns FALSE.
 */
_Check_return_
_IRQL_requires_max_(APC_LEVEL)
BOOLEAN
NkLookupElementGenericTable(
                            _In_ HANDLE TableHandle,
                            _In_ PVOID Buffer,
                            _In_ BOOLEAN Exclusive,
                            _In_opt_ PNKGT_OPERATE_ROUTINE OperateRoutine,
                            _Inout_opt_ PVOID OperateContext
                            );

/**
 * \brief Enum all the elements in the table.
 * \param TableHandle HANDLE of the safe generic table.
 * \param Exclusive Is the access exclusive.
 * \param TraverseRoutine Callback routine which will be called for each element.
 * \param TraverseContext Context of the traverse callback routine which is defined by caller.
 * \return N/A.
 */
_IRQL_requires_max_(APC_LEVEL)
VOID
NkEnumerateGenericTable(
                        _In_ HANDLE TableHandle,
                        _In_ BOOLEAN Exclusive,
                        _In_ PNKGT_TRAVERSE_ROUTINE TraverseRoutine,
                        _Inout_opt_ PVOID TraverseContext
                        );


/**@}*/ // Group End: nkdf-basic-gtable-api


/**@}*/ // Group End: nkdf-basic-gtable


/**@}*/ // Group End: nkdf-basic


#endif // __NKDF_BASIC_SAFE_GENERIC_TABLE_H__