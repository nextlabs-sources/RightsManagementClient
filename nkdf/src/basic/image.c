
#include <ntddk.h>
#include <ntimage.h>

#include <nkdf/basic/defines.h>
#include <nkdf/basic/string.h>
#include <nkdf/basic/image.h>


_Check_return_
ULONG
NkVirtualAddressToRaw(
                      _In_ PIMAGE_NT_HEADERS ImgNtHdr,
                      _In_ ULONG Address
                      )
{
    PIMAGE_SECTION_HEADER Sections = NULL;
    USHORT i = 0;

    if (ImgNtHdr->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        Sections = (PIMAGE_SECTION_HEADER)(((ULONG_PTR)&ImgNtHdr->OptionalHeader) + sizeof(IMAGE_OPTIONAL_HEADER64));
    }
    else {
        Sections = (PIMAGE_SECTION_HEADER)(((ULONG_PTR)&ImgNtHdr->OptionalHeader) + sizeof(IMAGE_OPTIONAL_HEADER32));
    }

    for (i = 0; i < ImgNtHdr->FileHeader.NumberOfSections; i++) {

        if ((Address >= Sections[i].VirtualAddress) &&
            ((i == ImgNtHdr->FileHeader.NumberOfSections - 1) || (Address < Sections[i + 1].VirtualAddress))) {

            // found the image section header which this virtual address is in
            Address = Address - Sections[i].VirtualAddress + Sections[i].PointerToRawData;
            break;
        }
    }

    return Address;
}

_Check_return_
PVOID
NkGetProcAddress(
                 _In_ HANDLE ModuleHandle,
                 _In_ PCSTR ProcName,
                 _In_ BOOLEAN Virtual
                 )
{
    PIMAGE_NT_HEADERS ImgNtHdr = NULL;

    // Image Export Directory Information
    PIMAGE_EXPORT_DIRECTORY ImgDDExports = NULL;
    ULONG   ImgDDSize = 0;
    ULONG   ImgDDAddr = 0;
    PULONG  ImgDDFunctions = NULL;
    PUSHORT ImgDDOrdinals = NULL;
    PULONG  ImgDDNames = NULL;
    ULONG   i = 0;
    BOOLEAN FirstFunc = TRUE;
    ULONG   NameOffset = 0;


    if (NULL == ModuleHandle || IMAGE_DOS_SIGNATURE == ((PIMAGE_DOS_HEADER)ModuleHandle)->e_magic) {
        return NULL;    // Not a valid PE Module
    }

    // Get PIMAGE_NT_HEADERS
    ImgNtHdr = (PVOID)((ULONG_PTR)ModuleHandle + ((PIMAGE_DOS_HEADER)ModuleHandle)->e_lfanew);
    if (IMAGE_NT_SIGNATURE != ImgNtHdr->Signature) {
        return NULL;    // Not a valid NT PE Image
    }

    ImgDDSize = ((PIMAGE_DATA_DIRECTORY)(ImgNtHdr->OptionalHeader.DataDirectory + IMAGE_DIRECTORY_ENTRY_EXPORT))->Size;
    ImgDDAddr = ((PIMAGE_DATA_DIRECTORY)(ImgNtHdr->OptionalHeader.DataDirectory + IMAGE_DIRECTORY_ENTRY_EXPORT))->VirtualAddress;
    ImgDDExports = (PVOID)((ULONG_PTR)ModuleHandle + ((Virtual) ? (ImgDDAddr) : NkVirtualAddressToRaw(ImgNtHdr, ImgDDAddr)));
    ImgDDFunctions = (PULONG)((ULONG_PTR)ModuleHandle + ((Virtual) ? (ImgDDExports->AddressOfFunctions) : NkVirtualAddressToRaw(ImgNtHdr, ImgDDExports->AddressOfFunctions)));
    ImgDDOrdinals = (PUSHORT)((ULONG_PTR)ModuleHandle + ((Virtual) ? (ImgDDExports->AddressOfNameOrdinals) : NkVirtualAddressToRaw(ImgNtHdr, ImgDDExports->AddressOfNameOrdinals)));
    ImgDDNames = (PULONG)((ULONG_PTR)ModuleHandle + ((Virtual) ? (ImgDDExports->AddressOfNames) : NkVirtualAddressToRaw(ImgNtHdr, ImgDDExports->AddressOfNames)));


    for (i = 0; i < ImgDDExports->NumberOfNames; i++) {

        // let's loop through all APIs until we find the right one
        if ((ImgDDFunctions[ImgDDOrdinals[i]] < ImgDDAddr) || (ImgDDFunctions[ImgDDOrdinals[i]] >= ImgDDAddr + ImgDDSize)) {

            if (FirstFunc) {
                if (!Virtual) {
                    // Get name offset
                    NameOffset = (ULONG_PTR)NkVirtualAddressToRaw(ImgNtHdr, ImgDDNames[i]) - (ULONG_PTR)ImgDDNames[i];
                }
                FirstFunc = FALSE;
            }

            // Compare Names            
            if (0 == NkCompareStringA(ProcName, (PSTR)((ULONG_PTR)ModuleHandle + ImgDDNames[i] + NameOffset), FALSE)) {
                // found it!
                return (PVOID)((ULONG_PTR)ModuleHandle + ((Virtual) ? (ImgDDFunctions[ImgDDOrdinals[i]]) : NkVirtualAddressToRaw(ImgNtHdr, ImgDDFunctions[ImgDDOrdinals[i]])));
            }
        }
    }

    return NULL;
}


_Check_return_
PVOID
NkGetAddressOfEntryPoint(
                         _In_ HANDLE ModuleHandle
                         )
{
    if ((ModuleHandle) && (((PIMAGE_DOS_HEADER)ModuleHandle)->e_magic == IMAGE_DOS_SIGNATURE)) {

        // might be a valid image
        PIMAGE_NT_HEADERS ImgNtHdr = (PVOID)((ULONG_PTR)ModuleHandle + ((PIMAGE_DOS_HEADER)ModuleHandle)->e_lfanew);
        if (ImgNtHdr->Signature == IMAGE_NT_SIGNATURE) {
            // It's really a valid image
            return &(ImgNtHdr->OptionalHeader.AddressOfEntryPoint);
        }
    }

    return NULL;
}

//_Check_return_
//PVOID
//NkGetRemoteAddressOfEntryPoint(
//                               _In_ HANDLE Process,
//                               _In_ HANDLE ModuleHandle
//                               )
//{
//    NTSTATUS Status = STATUS_SUCCESS;
//    IMAGE_DOS_HEADER DosHeader = { 0 };
//    IMAGE_NT_HEADERS NtHeader = { 0 };
//    ULONG   BytesRead = 0;
//
//    Status = ZwReadVirtualMemory(Process, ModuleHandle, &DosHeader, sizeof(DosHeader), &BytesRead);
//    if (!NT_SUCCESS(Status) || sizeof(DosHeader) != BytesRead) {
//        return NULL;
//    }
//
//    BytesRead = 0;
//    Status = ZwReadVirtualMemory(Process, ModuleHandle, (PVOID)((ULONG_PTR)ModuleHandle + DosHeader.e_lfanew), sizeof(NtHeader), &BytesRead);
//    if (!NT_SUCCESS(Status) || sizeof(NtHeader) != BytesRead || NtHeader.Signature != IMAGE_NT_SIGNATURE) {
//        return NULL;
//    }
//    
//    return (PVOID)((ULONG_PTR)ModuleHandle + DosHeader.e_lfanew + (ULONG_PTR)&NtHeader.OptionalHeader.AddressOfEntryPoint - (ULONG_PTR)&NtHeader);
//}

_Check_return_
NTSTATUS
NkMapDllFile(
             _In_ PCUNICODE_STRING DllPath,
             _Out_ PHANDLE FileHandle,
             _Out_ PHANDLE SectionHandle,
             _Out_ PHANDLE ModuleHandle
             )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES   ObjAttr;
    IO_STATUS_BLOCK     Iosb;



    *FileHandle = NULL;
    *SectionHandle = NULL;
    *ModuleHandle = NULL;

    try {

        SIZE_T  ViewSize = 0;
        PIMAGE_NT_HEADERS NtHeader = NULL;


        InitializeObjectAttributes(&ObjAttr, (PUNICODE_STRING)DllPath, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);
        Status = ZwCreateFile(FileHandle, FILE_READ_DATA, &ObjAttr, &Iosb, NULL, 0, FILE_SHARE_READ, FILE_OPEN, 0, NULL, 0);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        InitializeObjectAttributes(&ObjAttr, NULL, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);
        Status = ZwCreateSection(SectionHandle,
                                 STANDARD_RIGHTS_REQUIRED | SECTION_MAP_READ | SECTION_QUERY,
                                 &ObjAttr,
                                 NULL,
                                 PAGE_READONLY,
                                 SEC_COMMIT,
                                 *FileHandle);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = ZwMapViewOfSection(*SectionHandle, (HANDLE)-1, ModuleHandle, 0, 0, NULL, &ViewSize, 1, 0, PAGE_READONLY);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        if (((PIMAGE_DOS_HEADER)(*ModuleHandle))->e_magic != IMAGE_DOS_SIGNATURE) {
            try_return(Status = STATUS_SECTION_NOT_IMAGE);
        }

        NtHeader = (PVOID)((ULONG_PTR)(*ModuleHandle) + ((PIMAGE_DOS_HEADER)(*ModuleHandle))->e_lfanew);
        if (NtHeader->Signature == IMAGE_NT_SIGNATURE) {
            try_return(Status = STATUS_SECTION_NOT_IMAGE);
        }

        // Succeed
        Status = STATUS_SUCCESS;


try_exit: NOTHING;
    }
    finally {

        if (!NT_SUCCESS(Status)) {

            if (NULL != *ModuleHandle) {
                ZwUnmapViewOfSection((HANDLE)-1, *ModuleHandle);
                *ModuleHandle = NULL;
            }
            if (NULL != *SectionHandle) {
                ZwClose(*SectionHandle);
                *SectionHandle = NULL;
            }
            if (NULL != *FileHandle) {
                ZwClose(*FileHandle);
                *FileHandle = NULL;
            }
        }
    }

    return Status;
}

VOID
NkUnmapDllFile(
               _In_ HANDLE FileHandle,
               _In_ HANDLE SectionHandle,
               _In_ HANDLE ModuleHandle
               )
{
    if (NULL != ModuleHandle) {
        ZwUnmapViewOfSection((HANDLE)-1, ModuleHandle);
    }
    if (NULL != SectionHandle) {
        ZwClose(SectionHandle);
    }
    if (NULL != FileHandle) {
        ZwClose(FileHandle);
    }
}