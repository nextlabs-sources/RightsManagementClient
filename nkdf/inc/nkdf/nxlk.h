


#ifndef __NEXTLABS_FILE_FORMAT_H__
#define __NEXTLABS_FILE_FORMAT_H__
#include <ntifs.h>
#include <fltkernel.h>
#include <nkdf/rmp/nxlfmt.h>



_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NXLCheck(
         _In_ PFLT_INSTANCE Instance,
         _In_ PFILE_OBJECT FileObject,
         _Out_ PBOOLEAN Result
         );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NXLCheckEx(
           _In_ PFLT_FILTER Filter,
           _In_ PFLT_INSTANCE Instance,
           _In_ PCUNICODE_STRING FileName,
           _Out_ PBOOLEAN Result
           );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NXLCheck2(
          _In_ PFLT_INSTANCE Instance,
          _In_ PFILE_OBJECT FileObject,
          _In_opt_ PCNXL_KEKEY_BLOB PrimaryKey,
          _Out_ PBOOLEAN Result
          );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NXLCheckEx2(
            _In_ PFLT_FILTER Filter,
            _In_ PFLT_INSTANCE Instance,
            _In_ PCUNICODE_STRING FileName,
            _In_opt_ PCNXL_KEKEY_BLOB PrimaryKey,
            _Out_ PBOOLEAN Result
            );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NXLValidate(
            _In_ PFLT_INSTANCE Instance,
            _In_ PFILE_OBJECT FileObject,
            _Out_ LONG* Result
            );

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONG
NXLValidateHeader(
                  _In_ PCNXL_HEADER Header
                  );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NXLValidateEx(
              _In_ PFLT_INSTANCE Instance,
              _In_ PFILE_OBJECT FileObject,
              _In_opt_ PCNXL_KEKEY_BLOB PrimaryKey,
              _Out_ LONG* Result
              );

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONG
NXLValidateHeaderEx(
                    _In_ PCNXL_HEADER Header,
                    _In_opt_ PCNXL_KEKEY_BLOB PrimaryKey
                    );

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
NXL_SECTION*
NXLFindSection(
               _In_ PNXL_HEADER Header,
               _In_ const CHAR* Name,
               _Out_ PULONG Offset
               );

VOID
NXLGetOrignalFileExtension(
                           _In_ PCUNICODE_STRING FileName,
                           _In_ PUNICODE_STRING FileExtension
                           );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NXLCreateFile(
              _In_ PFLT_FILTER Filter,
              _In_ PFLT_INSTANCE Instance,
              _In_ PCUNICODE_STRING FileName,
              _In_ PCNXL_KEKEY_BLOB PrimaryKey,
              _In_opt_ PCNXL_KEKEY_BLOB RecoveryKey,
              _In_ BOOLEAN Overwrite
              );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NXLCreateEmptyFile(
                   _In_ PFLT_INSTANCE Instance,
                   _In_ PFILE_OBJECT FileObject,
                   _In_ PCNXL_KEKEY_BLOB PrimaryKey,
                   _In_opt_ PCNXL_KEKEY_BLOB RecoveryKey,
                   _In_opt_ PCUNICODE_STRING Extension,
                   _Out_writes_opt_(32) PUCHAR ContentKey
                   );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NXLReadHeader(
              _In_ PFLT_INSTANCE Instance,
              _In_ PFILE_OBJECT FileObject,
              _Out_ PNXL_HEADER Header
              );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NXLWriteHeader(
               _In_ PFLT_INSTANCE Instance,
               _In_ PFILE_OBJECT FileObject,
               _In_ PCNXL_HEADER Header
               );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NXLGetContentKey(
                 _In_ PCNXL_HEADER Header,
                 _In_reads_(32) PUCHAR PrimaryKey,
                 _Out_writes_(32) PUCHAR ContentKey
                 );


_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NXLReadData(
            _In_ PFLT_INSTANCE Instance,
            _In_ PFILE_OBJECT FileObject,
            _In_ const ULONG HeaderSize,
            _In_ const UCHAR* ContentKey,
            _In_ const LARGE_INTEGER ContentLength,
            _In_ const LARGE_INTEGER AllocationLength,
            _In_ const LARGE_INTEGER Offset,
            _Out_writes_(BytesToRead) PVOID Data,
            _In_ const ULONG BytesToRead,
            _Out_ PULONG BytesRead
            );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NXLWriteData(
             _In_ PFLT_INSTANCE Instance,
             _In_ PFILE_OBJECT FileObject,
             _In_ const ULONG HeaderSize,
             _In_ const UCHAR* ContentKey,
             _In_ const LARGE_INTEGER ContentLength,
             _In_ const LARGE_INTEGER AllocationLength,
             _In_ const LARGE_INTEGER Offset,
             _Inout_updates_(BytesToWrite) PVOID Data,
             _In_ const ULONG BytesToWrite,
             _Out_ PULONG BytesWritten
             );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NXLGetFileSize(
               _In_ PFLT_INSTANCE Instance,
               _In_ PFILE_OBJECT FileObject,
               _Out_ PLARGE_INTEGER FileSize
               );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NXLSetFileSize(
               _In_ PFLT_INSTANCE Instance,
               _In_ PFILE_OBJECT FileObject,
               _In_ const UCHAR* ContentKey,
               _In_ LARGE_INTEGER FileSize
               );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NXLReadSectionData(
                   _In_ PFLT_INSTANCE Instance,
                   _In_ PFILE_OBJECT FileObject,
                   _In_ const CHAR* Name,
                   _Out_writes_opt_(*Size) PVOID Data,
                   _Inout_ PULONG Size
                   );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NXLWriteSectionData(
                    _In_ PFLT_INSTANCE Instance,
                    _In_ PFILE_OBJECT FileObject,
                    _In_ const UCHAR* ContentKey,
                    _In_ const CHAR* Name,
                    _In_reads_(Size) const VOID* Data,
                    _In_ ULONG Size
                    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NXLGetSectionBlock(
                   _In_ PFLT_INSTANCE Instance,
                   _In_ PFILE_OBJECT FileObject,
                   _Outptr_result_maybenull_ PNXL_SECTION_TABLE* Buffer,
                   _Out_ PULONG Size
                   );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NXLSetSectionBlock(
                   _In_ PFLT_INSTANCE Instance,
                   _In_ PFILE_OBJECT FileObject,
                   _In_ const UCHAR* ContentKey,
                   _In_reads_(Size) PCNXL_SECTION_TABLE Buffer,
                   _In_ const ULONG Size
                   );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NXLSetSectionBlockEx(
                     _In_ PFLT_INSTANCE Instance,
                     _In_ PFILE_OBJECT FileObject,
                     _In_ const UCHAR* ContentKey,
                     _In_reads_(Size) PCNXL_SECTION_TABLE Buffer,
                     _In_ const ULONG Size,
                     _In_opt_ PCUNICODE_STRING NewExtension
                     );



#endif      // __NEXTLABS_FILE_FORMAT_H__