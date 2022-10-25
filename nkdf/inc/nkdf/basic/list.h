

/**
 * \file <nkdf/basic/list.h>
 * \brief Header file for safe list
 *
 * This header file defines Safe List object and provides related routines
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */


#ifndef __NKDF_BASIC_LIST_H__
#define __NKDF_BASIC_LIST_H__



/**
 * \addtogroup nkdf-basic
 * @{
 */


/**
 * \defgroup nkdf-basic-safelist Safe List
 * @{
 */


/**
 * \defgroup nkdf-basic-safelist-callback Callbacks
 * @{
 */


/** \typedef NKLIST_COMPARE_ROUTINE
 *  \brief Entry1 Entry of the first element.
 *  \brief Entry2 Entry of the second element.
 *  \return TRUE if these two elements are equal.
 */
typedef
BOOLEAN
(*NKLIST_COMPARE_ROUTINE)(
                          _In_ const LIST_ENTRY* Entry1,
                          _In_ const LIST_ENTRY* Entry2
                          );

/** \typedef NKLIST_FREE_ROUTINE
 *  \brief Entry Entry of the element to free.
 *  \return N/A.
 */
typedef
VOID
(*NKLIST_FREE_ROUTINE)(
                       _In_ LIST_ENTRY* Entry
                       );

/** \typedef NKLIST_TRAVERSE_ROUTINE
 *  \brief Entry Entry of the element to free.
 *  \brief Continue Pointer of a BOOLEAN variable. If this routine set it to FALSE, the traverse will stop.
 *  \return N/A.
 */
typedef
NTSTATUS
(*NKLIST_TRAVERSE_ROUTINE)(
                           _In_ PLIST_ENTRY Entry,
                           _Out_ PBOOLEAN Continue
                           );


/**@}*/ // Group End: nkdf-basic-safelist-callback



/**
 * \defgroup nkdf-basic-safelist-api Routines
 * @{
 */


/**
 * \brief This routine creates a Safe List.
 * \param CompareRoutine Routine to compare two elements.
 * \param FreeRoutine Routine to free a element.
 * \param ListHandle Pointer to a HANDLE which receives the handle of the newly created Safe List.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
NkCreateList(
             _In_ NKLIST_COMPARE_ROUTINE CompareRoutine,
             _In_ NKLIST_FREE_ROUTINE FreeRoutine,
             _Out_ PHANDLE ListHandle
             );

/**
 * \brief This routine destroy an existing Safe List.
 * \param ListHandle Handle of a Safe List.
 * \return N/A.
 */
_IRQL_requires_max_(APC_LEVEL)
VOID
NkDestoryList(
              _In_ HANDLE ListHandle
              );

/**
 * \brief This routine traverse a Safe List and perform user defined operation.
 * \param ListHandle Handle of a Safe List.
 * \param Exclusive Is the access exclusive.
 * \param TraverseRoutine Operation to performed on each element.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkTraverseList(
               _In_ HANDLE ListHandle,
               _In_ BOOLEAN Exclusive,
               _In_ NKLIST_TRAVERSE_ROUTINE TraverseRoutine
               );

/**
 * \brief This routine check if a Safe List is empty.
 * \param ListHandle Handle of a Safe List.
 * \return TRUE if the list is empty.
 */
_Check_return_
_IRQL_requires_max_(APC_LEVEL)
BOOLEAN
NkIsListEmpty(
              _In_ HANDLE ListHandle
              );

/**
 * \brief This routine removes all the element from a Safe List.
 * \param ListHandle Handle of a Safe List.
 * \return N/A.
 */
_IRQL_requires_max_(APC_LEVEL)
VOID
NkClearList(
            _In_ HANDLE ListHandle
            );

/**
 * \brief This routine insert an element to the head of Safe List.
 * \param ListHandle Handle of a Safe List.
 * \param Entry Entry of the element.
 * \param Unique If TRUE, the element will be inserted only if it doesn't exist in the list.
 * \return N/A.
 */
_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkInsertHeadList(
                 _In_ HANDLE ListHandle,
                 _In_ PLIST_ENTRY Entry,
                 _In_ BOOLEAN Unique
                 );

/**
 * \brief This routine insert an element to the tail of Safe List.
 * \param ListHandle Handle of a Safe List.
 * \param Entry Entry of the element.
 * \param Unique If TRUE, the element will be inserted only if it doesn't exist in the list.
 * \return N/A.
 */
_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkInsertTailList(
                 _In_ HANDLE ListHandle,
                 _In_ PLIST_ENTRY Entry,
                 _In_ BOOLEAN Unique
                 );

/**
 * \brief This routine removes an element from head of Safe List.
 * \param ListHandle Handle of a Safe List.
 * \return Pointer to LIST_ENTRY. If the list is empty, it returns NULL.
 */
_Check_return_
_IRQL_requires_max_(APC_LEVEL)
PLIST_ENTRY
NkRemoveHeadList(
                 _In_ HANDLE ListHandle
                 );

/**
 * \brief This routine removes an element from tail of Safe List.
 * \param ListHandle Handle of a Safe List.
 * \return Pointer to LIST_ENTRY. If the list is empty, it returns NULL.
 */
_Check_return_
_IRQL_requires_max_(APC_LEVEL)
PLIST_ENTRY
NkRemoveTailList(
                 _In_ HANDLE ListHandle
                 );

/**
 * \brief This routine removes an entry from a Safe List of LIST_ENTRY structures.
 * \param ListHandle Handle of a Safe List.
 * \param Entry Pointer to the entry of the element.
 * \return This routine returns TRUE if, after removal of the designated entry, the list is empty.
 *         Otherwise, the routine returns FALSE to indicate that the resulting list still contains one or more entries.
 */
_Check_return_
_IRQL_requires_max_(APC_LEVEL)
BOOLEAN
NkRemoveEntryList(
                  _In_ HANDLE ListHandle,
                  _In_ PLIST_ENTRY Entry
                  );


/**@}*/ // Group End: nkdf-basic-safelist-api


/**@}*/ // Group End: nkdf-basic-safelist


/**@}*/ // Group End: nkdf-basic


#endif // __NKDF_BASIC_LIST_H__