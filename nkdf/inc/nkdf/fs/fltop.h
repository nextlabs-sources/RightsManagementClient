
/**
 * \file <nkdf/fs/fltop.h>
 * \brief Header file for mini-filter file operation support routines
 *
 * This header file declare mini-filter file operation support routines
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */


#ifndef __NKDF_FS_FLT_OPERATION_H__
#define __NKDF_FS_FLT_OPERATION_H__



#include <fltkernel.h>


/**
 * \addtogroup nkdf-fs
 * @{
 */


/**
 * \defgroup nkdf-fs-fltop Mini-filter File Operation Support
 * @{
 */


/**
 * \defgroup nkdf-fs-fltop-api Routines
 * @{
 */



_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltCreateFile(
                _In_ PFLT_FILTER Filter,
                _In_opt_ PFLT_INSTANCE Instance,
                _Out_ PHANDLE FileHandle,
                _In_ ACCESS_MASK DesiredAccess,
                _In_ PCUNICODE_STRING FileName,
                _Out_opt_ PULONG Result,
                _In_ ULONG FileAttributes,
                _In_ ULONG ShareAccess,
                _In_ ULONG CreateDisposition,
                _In_ ULONG CreateOptions,
                _In_ ULONG Flags
                );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltOpenFile(
              _In_ PFLT_FILTER Filter,
              _In_opt_ PFLT_INSTANCE Instance,
              _Out_ PHANDLE FileHandle,
              _In_ ACCESS_MASK DesiredAccess,
              _In_ PCUNICODE_STRING FileName,
              _Out_opt_ PULONG Result,
              _In_ ULONG ShareAccess,
              _In_ ULONG OpenOptions,
              _In_ ULONG Flags
              );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltOpenFileById(
                  _In_ PFLT_FILTER Filter,
                  _In_opt_ PFLT_INSTANCE Instance,
                  _Out_ PHANDLE FileHandle,
                  _In_ ACCESS_MASK DesiredAccess,
                  _In_ PUNICODE_STRING VolumeName,
                  _In_ LARGE_INTEGER FileId,
                  _Out_opt_ PULONG Result,
                  _In_ ULONG ShareAccess,
                  _In_ ULONG OpenOptions,
                  _In_ ULONG Flags
                  );

_IRQL_requires_(PASSIVE_LEVEL)
VOID
NkFltCloseFile(
               _In_ HANDLE FileHandle
               );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltDeleteFile(
                _In_ PFLT_FILTER Filter,
                _In_opt_ PFLT_INSTANCE Instance,
                _In_ PCUNICODE_STRING FileName
                );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltRenameFile(
                _In_ PFLT_INSTANCE Instance,
                _In_ PFILE_OBJECT FileObject,
                _In_ PCUNICODE_STRING NewFileName,
                _In_ BOOLEAN ReplaceIfExists
                );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltMoveFile(
              _In_ PFLT_INSTANCE Instance,
              _In_ PFILE_OBJECT FileObject,
              _In_ PCUNICODE_STRING NewFilePath,
              _In_ BOOLEAN ReplaceIfExists
              );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltIsFileDirectory(
                     _In_ PFLT_INSTANCE Instance,
                     _In_ PFILE_OBJECT FileObject,
                     _Out_ PBOOLEAN Directory
                     );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
BOOLEAN
NkFltIsFileExisting(
                    _In_ PFLT_FILTER Filter,
                    _In_opt_ PFLT_INSTANCE Instance,
                    _In_ PCUNICODE_STRING FileName
                    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltGetFileAttributes(
                       _In_ PFLT_INSTANCE Instance,
                       _In_ PFILE_OBJECT FileObject,
                       _Out_ PULONG FileAttributes
                       );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltGetFileTime(
                 _In_ PFLT_INSTANCE Instance,
                 _In_ PFILE_OBJECT FileObject,
                 _Out_opt_ PLARGE_INTEGER CreationTime,
                 _Out_opt_ PLARGE_INTEGER LastAccessTime,
                 _Out_opt_ PLARGE_INTEGER LastWriteTime,
                 _Out_opt_ PLARGE_INTEGER ChangeTime
                 );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltGetFileSize(
                 _In_ PFLT_INSTANCE Instance,
                 _In_ PFILE_OBJECT FileObject,
                 _Out_ PLARGE_INTEGER FileSize
                 );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltGetFilePointer(
                    _In_ PFLT_INSTANCE Instance,
                    _In_ PFILE_OBJECT FileObject,
                    _Out_ PLARGE_INTEGER CurrentByteOffset
                    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltSetFileAttributes(
                       _In_ PFLT_INSTANCE Instance,
                       _In_ PFILE_OBJECT FileObject,
                       _In_ ULONG FileAttributes
                       );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltSetFileTime(
                 _In_ PFLT_INSTANCE Instance,
                 _In_ PFILE_OBJECT FileObject,
                 _In_ LONGLONG CreationTime,
                 _In_ LONGLONG LastAccessTime,
                 _In_ LONGLONG LastWriteTime,
                 _In_ LONGLONG ChangeTime
                 );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltSetFileSize(
                 _In_ PFLT_INSTANCE Instance,
                 _In_ PFILE_OBJECT FileObject,
                 _In_ LARGE_INTEGER FileSize
                 );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltSetFilePointer(
                    _In_ PFLT_INSTANCE Instance,
                    _In_ PFILE_OBJECT FileObject,
                    _In_ LARGE_INTEGER CurrentByteOffset
                    );


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltZeroFileContent(
                     _In_ PFLT_INSTANCE Instance,
                     _In_ PFILE_OBJECT FileObject,
                     _In_ PLARGE_INTEGER ByteOffset,
                     _In_ ULONG Length
                     );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncReadFile(
                  _In_ PFLT_INSTANCE Instance,
                  _In_ PFILE_OBJECT FileObject,
                  _In_opt_ PLARGE_INTEGER ByteOffset,
                  _In_ ULONG Length,
                  _Out_ PVOID Buffer,
                  _In_ FLT_IO_OPERATION_FLAGS Flags,
                  _Out_opt_ PULONG BytesRead
                  );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncWriteFile(
                   _In_ PFLT_INSTANCE Instance,
                   _In_ PFILE_OBJECT FileObject,
                   _In_opt_ PLARGE_INTEGER ByteOffset,
                   _In_ ULONG Length,
                   _In_ PVOID Buffer,
                   _In_ FLT_IO_OPERATION_FLAGS Flags,
                   _Out_opt_ PULONG BytesWritten
                  );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncQueryInformationFile(
                              _In_ PFLT_INSTANCE Instance,
                              _In_ PFILE_OBJECT FileObject,
                              _Out_ PVOID FileInformation,
                              _In_ ULONG Length,
                              _In_ FILE_INFORMATION_CLASS FileInformationClass,
                              _Out_opt_ PULONG LengthReturned
                              );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncSetInformationFile(
                            _In_ PFLT_INSTANCE Instance,
                            _In_ PFILE_OBJECT FileObject,
                            _In_ PVOID FileInformation,
                            _In_ ULONG Length,
                            _In_ FILE_INFORMATION_CLASS FileInformationClass
                            );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncRenameFile(
                    _In_ PFLT_INSTANCE Instance,
                    _In_ PFILE_OBJECT FileObject,
                    _In_ PCUNICODE_STRING NewFileName,
                    _In_ BOOLEAN ReplaceIfExists
                    );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncMoveFile(
                  _In_ PFLT_INSTANCE Instance,
                  _In_ PFILE_OBJECT FileObject,
                  _In_ PCUNICODE_STRING NewFilePath,
                  _In_ BOOLEAN ReplaceIfExists
                  );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncGetFileAttributes(
                           _In_ PFLT_INSTANCE Instance,
                           _In_ PFILE_OBJECT FileObject,
                           _Out_ PULONG FileAttributes
                           );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncGetFileTime(
                     _In_ PFLT_INSTANCE Instance,
                     _In_ PFILE_OBJECT FileObject,
                     _Out_opt_ PLARGE_INTEGER CreationTime,
                     _Out_opt_ PLARGE_INTEGER LastAccessTime,
                     _Out_opt_ PLARGE_INTEGER LastWriteTime,
                     _Out_opt_ PLARGE_INTEGER ChangeTime
                     );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncGetFileSize(
                     _In_ PFLT_INSTANCE Instance,
                     _In_ PFILE_OBJECT FileObject,
                     _Out_ PLARGE_INTEGER FileSize
                     );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncGetFilePointer(
                        _In_ PFLT_INSTANCE Instance,
                        _In_ PFILE_OBJECT FileObject,
                        _Out_ PLARGE_INTEGER CurrentByteOffset
                        );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncSetFileAttributes(
                           _In_ PFLT_INSTANCE Instance,
                           _In_ PFILE_OBJECT FileObject,
                           _In_ ULONG FileAttributes
                           );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncSetFileTime(
                     _In_ PFLT_INSTANCE Instance,
                     _In_ PFILE_OBJECT FileObject,
                     _In_ LONGLONG CreationTime,
                     _In_ LONGLONG LastAccessTime,
                     _In_ LONGLONG LastWriteTime,
                     _In_ LONGLONG ChangeTime
                     );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncSetFileSize(
                     _In_ PFLT_INSTANCE Instance,
                     _In_ PFILE_OBJECT FileObject,
                     _In_ LARGE_INTEGER FileSize
                     );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncSetFilePointer(
                        _In_ PFLT_INSTANCE Instance,
                        _In_ PFILE_OBJECT FileObject,
                        _In_ LARGE_INTEGER CurrentByteOffset
                        );

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncZeroFileContent(
                         _In_ PFLT_INSTANCE Instance,
                         _In_ PFILE_OBJECT FileObject,
                         _In_ PLARGE_INTEGER ByteOffset,
                         _In_ ULONG Length
                         );


_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkFltSyncQueryDirectoryFile(
                            _In_ PFLT_INSTANCE Instance,
                            _In_ PFILE_OBJECT FileObject,
                            _Out_ PVOID FileInformation,
                            _In_ ULONG Length,
                            _In_ FILE_INFORMATION_CLASS FileInformationClass,
                            _In_ BOOLEAN ReturnSingleEntry,
                            _In_opt_ PUNICODE_STRING FileName,
                            _In_ BOOLEAN RestartScan,
                            _Out_opt_ PULONG LengthReturned
                            );


/**@}*/ // Group End: nkdf-fs-fltop-api


/**@}*/ // Group End: nkdf-fs-fltop


/**@}*/ // Group End: nkdf-fs

#endif  // #ifndef __NKDF_FS_FLT_OPERATION_H__