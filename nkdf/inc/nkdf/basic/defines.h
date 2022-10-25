
/**
 * \file <nkdf/basic/defines.h>
 * \brief Header file for NKDF definitions
 *
 * This header file contains all the NKDF definitions
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */


#ifndef __NKDF_DEFINES_H__
#define __NKDF_DEFINES_H__


/**
 * \addtogroup nkdf-basic
 * @{
 */


/**
 * \defgroup nkdf-basic-def Basic NKDF Definitions
 * @{
 */

/**
 * \defgroup nkdf-basic-def-memtag Memory tags
 * @{
 */

/** \def TAG_TEMP
 *  Tag of the memory allocated temporarily - user should not see this in PoolMon.
 */
#define TAG_TEMP        '**kN'

/** \def TAG_ALG
 *  Tag of the memory allocated for algorithm.
 */
#define TAG_ALG         'lAkN'

/** \def TAG_LOG
 *  Tag of the memory allocated by logging library.
 */
#define TAG_LOG         'gLkN'

/** \def TAG_VOL
 *  Tag of the memory allocated for volume information.
 */
#define TAG_VOL         'oVkN'

/** \def TAG_IOBUF
 *  Tag of the memory allocated for I/O buffer.
 */
#define TAG_IOBUF       'oIkN'

/** \def TAG_PROC
 *  Tag of the memory allocated for process information.
 */
#define TAG_PROC        'rPkN'

/** \def TAG_CSQ
 *  Tag of the memory allocated for safe cancel queue.
 */
#define TAG_CSQ         'qCkN'

/** \def TAG_FILENAME
 *  Tag of the memory allocated for file name.
 */
#define TAG_FILENAME    'NFkN'

/** \def TAG_ERESOURCE
 *  Tag of the memory allocated for ERESOURCE.
 */
#define TAG_ERESOURCE   'REkN'

/** \def TAG_FLTCONN
 *  Tag of the memory allocated for mini-filter conn object.
 */
#define TAG_FLTCONN     'cFkN'

/** \def TAG_HANDLE
 *  Tag of the memory allocated for handles.
 */
#define TAG_HANDLE      'dHkN'

/** \def TAG_DETOUR
 *  Tag of the memory allocated for detour object.
 */
#define TAG_DETOUR      'tDkN'

/** \def TAG_LIST
 *  Tag of the memory allocated for safe list.
 */
#define TAG_LIST        'sLkN'

/** \def TAG_TABLE
 *  Tag of the memory allocated for safe generic table.
 */
#define TAG_TABLE       'bTkN'

/**@}*/ // Group End: nkdf-basic-def-memtag



#ifndef WINAPI
#define WINAPI  __stdcall   /**< Define WINAPI */
#endif

#ifndef NTAPI
#define NTAPI   __stdcall   /**< Define NTAPI */
#endif

#ifndef BYTE
typedef UCHAR  BYTE;        /**< Define BYTE */
#endif



/**
 * \defgroup nkdf-basic-def-const CONST values
 * @{
 */

/**
 * \defgroup nkdf-basic-def-predefsize Predefined size
 * @{
 */

/** \def ONE_KB
 *  One Kilobytes
 */
#define ONE_KB                          0x00000400UL

/** \def ONE_MB
 *  One Megabytes
 */
#define ONE_MB                          0x00100000UL

/** \def ONE_GB
 *  One Gigabytes
 */
#define ONE_GB                          0x40000000UL

/**@}*/ // Group End: nkdf-basic-def-predefsize


/**
 * \defgroup nkdf-basic-def-maxval Maximum values
 * @{
 */

/** \def MAX_LONG
 *  Maximum LONG value
 */
#define MAX_LONG                        0x7FFFFFFF

/** \def MAX_LONGLONG
 *  Maximum LONGLONG value
 */
#define MAX_LONGLONG                    0x7FFFFFFFFFFFFFFF

/** \def MAX_ULONG
 *  Maximum ULONG value
 */
#define MAX_ULONG                       0xFFFFFFFF

/** \def MAX_ULONGLONG
 *  Maximum ULONGLONG value
 */
#define MAX_ULONGLONG                   0xFFFFFFFFFFFFFFFF

/**@}*/ // Group End: nkdf-basic-def-maxval


/**
 * \defgroup nkdf-basic-def-delaytime Delay time values
 * @{
 */

/** \def DELAY_ONE_MICROSECOND
 *  Delay one microsecond.
 */
#define DELAY_ONE_MICROSECOND           (-10)

/** \def DELAY_ONE_MILLISECOND
 *  Delay one millisecond.
 */
#define DELAY_ONE_MILLISECOND           (DELAY_ONE_MICROSECOND*1000)

/** \def DELAY_ONE_SECOND
 *  Delay one second.
 */
#define DELAY_ONE_SECOND                (DELAY_ONE_MILLISECOND*1000)

/**@}*/ // Group End: nkdf-basic-def-delaytime

/**@}*/ // Group End: nkdf-basic-def-const



/**
 * \defgroup nkdf-basic-def-util Macro Utilities
 * @{
 */


/**
 * \defgroup nkdf-basic-def-util-tryreturn Handy macros for try-finally
 * @{
 */
#ifndef TRY_RETURN
#define TRY_RETURN  /**< TRY_RETURN Swicth */
/** \def try_return(S)
*  execute statement (S) and then goto label 'try_exit'.
*/
#define try_return(S)   {S; goto try_exit;}

/** \def try_return2(S)
*  execute statement (S) and then goto label 'try_exit2'.
*/
#define try_return2(S)  {S; goto try_exit2;}
#endif

/**@}*/ // nkdf-basic-def-util-tryreturn


/**
 * \defgroup nkdf-basic-def-util-pointer Handy macros for doing pointer arithmetic
 * @{
 */
/** \def NKADD2PTR(P,I)
 *  Add offset (I) to pointer (P).
 */
#define NKADD2PTR(P,I)      ((PVOID)((PUCHAR)(P) + (I)))

/** \def NKPTROFFSET(B,O)
 *  Calculate the offset between pointer O and B.
 */
#define NKPTROFFSET(B,O)    ((ULONG)((ULONG_PTR)(O) - (ULONG_PTR)(B)))

/**@}*/ // nkdf-basic-def-util-pointer



/**
 * \defgroup nkdf-basic-def-util-alignment Handy macros for size alignment
 * @{
 */

/** \def NKROUND_TO_SIZE(Type, Length, Alignment)
 *  This macro takes a length & rounds it up to a multiple of the alignment
 *  Alignment is given as a power of 2.
 */
#define NKROUND_TO_SIZE(Type, Length, Alignment)        ((Type)(((ULONGLONG)Length + ((ULONGLONG)Alignment - 1)) & ~((ULONGLONG)Alignment - 1)))

/** \def NKROUNDDOWN_TO_SIZE(Type, Length, Alignment)
 *  This macro takes a length & rounds it down to a multiple of the alignment
 *  Alignment is given as a power of 2.
 */
#define NKROUNDDOWN_TO_SIZE(Type, Length, Alignment)    ((Type)(((ULONGLONG)Length) & ~((ULONGLONG)Alignment - 1)))

/** \def NKIS_ALIGNED(Pointer, Alignment)
 *  Checks if 1st argument is aligned on given power of 2 boundary specified
 *  by 2nd argument.
 */
#define NKIS_ALIGNED(Pointer, Alignment)                ((((ULONG_PTR) (Pointer)) & ((Alignment) - 1)) == 0)

/**@}*/ // nkdf-basic-def-util-alignment



/**@}*/ // Group End: nkdf-basic-def-util Macro Utilities


/**@}*/ // Group End: nkdf-basic-def NKDF Definitions


/**@}*/ // Group End: nkdf-basic


#endif  // #ifndef __NKDF_DEFINES_H__