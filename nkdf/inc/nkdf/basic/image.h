
/**
 * \file <nkdf/basic/image.h>
 * \brief Header file for PE image
 *
 * This header file contains routines to manipulate PE image
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */

#ifndef __NKDF_BASIC_IMAGE_H__
#define __NKDF_BASIC_IMAGE_H__



/**
 * \addtogroup nkdf-basic
 * @{
 */


/**
 * \defgroup nkdf-basic-img PE Image Util
 * @{
 */


/**
 * \defgroup nkdf-basic-img-api Routines
 * @{
 */



/**
 * \brief This routine converts virtual address to raw address.
 * \param ImgNtHdr Pointer to struct PIMAGE_NT_HEADERS.
 * \param VirtualAddress Virtual address.
 * \return Raw address.
 */
_Check_return_
ULONG
NkVirtualAddressToRaw(
                      _In_ PIMAGE_NT_HEADERS ImgNtHdr,
                      _In_ ULONG VirtualAddress
                      );



/**
 * \brief This routine retrieves the address of an exported function or variable from the specified module.
 * \param ModuleHandle Handle of the module.
 * \param ProcName The function or variable name.
 * \param Virtual If TRUE it will return virtual address. Otherwise raw address will be returned.
 * \return Function or variable address.
 */
_Check_return_
PVOID
NkGetProcAddress(
                 _In_ HANDLE ModuleHandle,
                 _In_ PCSTR ProcName,
                 _In_ BOOLEAN Virtual
                 );

/**
 * \brief This routine get entry point address of the given module.
 * \param ModuleHandle Handle of the module.
 * \return Entry-point address.
 */
_Check_return_
PVOID
NkGetAddressOfEntryPoint(
                         _In_ HANDLE ModuleHandle
                         );


/**
 * \brief Map a DLL to memory.
 * \param DllPath Path of the DLL file.
 * \param FileHandle File handle of the DLL file.
 * \param SectionHandle Section handle of the DLL file.
 * \param ModuleHandle Module handle of the DLL file.
 * \return NTSTATUS.
 */
_Check_return_
NTSTATUS
NkMapDllFile(
             _In_ PCUNICODE_STRING DllPath,
             _Out_ PHANDLE FileHandle,
             _Out_ PHANDLE SectionHandle,
             _Out_ PHANDLE ModuleHandle
             );

/**
 * \brief Unmap a DLL to memory.
 * \param FileHandle File handle of the DLL file.
 * \param SectionHandle Section handle of the DLL file.
 * \param ModuleHandle Module handle of the DLL file.
 * \return N/A.
 */
VOID
NkUnmapDllFile(
               _In_ HANDLE FileHandle,
               _In_ HANDLE SectionHandle,
               _In_ HANDLE ModuleHandle
               );



/**@}*/ // Group End: nkdf-basic-img-api


/**@}*/ // Group End: nkdf-basic-img


/**@}*/ // Group End: nkdf-basic


#endif  // __NKDF_BASIC_IMAGE_H__