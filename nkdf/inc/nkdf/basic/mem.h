

/**
 * \file <nkdf/basic/mem.h>
 * \brief Header file for memory support routines
 *
 * This header file declare memory support routines
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */


#ifndef __NKDF_BASIC_MEMORY_H__
#define __NKDF_BASIC_MEMORY_H__



/**
 * \addtogroup nkdf-basic
 * @{
 */


/**
 * \defgroup nkdf-basic-mem Memory Support
 * @{
 */


/**
 * \defgroup nkdf-basic-mem-api Routines
 * @{
 */


/**
 * \brief Function to lock user buffer.
 * \IRQL Less or equal to APC_LEVEL.
 * \param Irp I/O request.
 * \param Operation Lock operation.
 * \param BufferLength Buffer size.
 * \return N/A.
 */
_IRQL_requires_max_(APC_LEVEL)
VOID
NkLockUserBuffer(
                 _Inout_ PIRP Irp,
                 _In_ LOCK_OPERATION Operation,
                 _In_ ULONG BufferLength
                 );

/**
 * \brief Function to map user buffer.
 * \IRQL Less or equal to APC_LEVEL.
 * \param Irp I/O request.
 * \return Pointer to the mapped buffer.
 */
_IRQL_requires_max_(APC_LEVEL)
PVOID
NkMapUserBuffer(
                _Inout_ PIRP Irp
                );

/**
 * \brief Function to buffer a user buffer.
 * \IRQL Less or equal to APC_LEVEL.
 * \param Irp I/O request.
 * \param BufferLength Buffer size.
 * \return Pointer to the mapped buffer.
 */
_IRQL_requires_max_(APC_LEVEL)
PVOID
NkBufferUserBuffer (
                    _Inout_ PIRP Irp,
                    _In_ ULONG BufferLength
                    );


/**@}*/ // Group End: nkdf-basic-mem-api





/**
 * \defgroup nkdf-basic-mem-inline Inline Routines
 * @{
 */


/**
 * \brief Function to compare memory.
 * \param Mem1 Pointer to first memory.
 * \param Mem2 Pointer to second memory.
 * \param Size Size to compare.
 * \return The bytes of the memory are equal.
 */
__forceinline
_Check_return_
SIZE_T
NkMemoryCompare(
                _In_ const VOID* Mem1,
                _In_ const VOID* Mem2,
                _In_ SIZE_T Size
                )
{
    SIZE_T i = 0;
    const UCHAR* p1 = (const UCHAR*)Mem1;
    const UCHAR* p2 = (const UCHAR*)Mem2;

    for (i = 0; i < Size; i++) {
        if (p1[i] != p2[i]) {
            break;
        }
    }

    return i;
}

/**
 * \brief Check if specified memory is filled by zero.
 * \param Mem Pointer to memory.
 * \param Size Size of the memory to check.
 * \return BOOLEAN.
 * \retval TRUE The memory is filled by zero.
 * \retval FALSE The memory is NOT filled by zero.
 */
__forceinline
_Check_return_
BOOLEAN
NkIsMemoryZero(
               _In_ const VOID* Mem,
               _In_ SIZE_T Size
               )
{
    SIZE_T i = 0;
    for (i = 0; i < Size; i++) {
        if (0 != ((const UCHAR*)Mem)[i]) {
            return FALSE;
        }
    }
    return TRUE;
}



/**@}*/ // Group End: nkdf-basic-mem-inline


/**@}*/ // Group End: nkdf-basic-mem


/**@}*/ // Group End: nkdf-basic


#endif  // #ifndef __NKDF_BASIC_MEMORY_H__