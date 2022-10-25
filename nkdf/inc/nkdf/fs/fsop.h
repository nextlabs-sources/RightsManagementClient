
/**
 * \file <nkdf/fs/fsop.h>
 * \brief Header file for file operation support routines
 *
 * This header file declare file operation support routines
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */


#ifndef __NKDF_FS_OPERATION_H__
#define __NKDF_FS_OPERATION_H__


/**
 * \addtogroup nkdf-fs
 * @{
 */


/**
 * \defgroup nkdf-fs-fsop File Operation Support
 * @{
 */


/**
 * \defgroup nkdf-fs-fsop-api Routines
 * @{
 */


_Check_return_
_IRQL_requires_max_(APC_LEVEL)
BOOLEAN
NkIsReservedFile(
                 _In_ PCUNICODE_STRING FileName
                 );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCreateFile(
             _Out_ PHANDLE FileHandle,
             _In_ ACCESS_MASK DesiredAccess,
             _In_ PCUNICODE_STRING FileName,
             _Out_opt_ PULONG Result,
             _In_ ULONG FileAttributes,
             _In_ ULONG ShareAccess,
             _In_ ULONG CreateDisposition,
             _In_ ULONG CreateOptions
             );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkOpenFile(
           _Out_ PHANDLE FileHandle,
           _In_ ACCESS_MASK DesiredAccess,
           _In_ PCUNICODE_STRING FileName,
           _Out_opt_ PULONG Result,
           _In_ ULONG ShareAccess,
           _In_ ULONG OpenOptions
           );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkOpenFileById(
               _Out_ PHANDLE FileHandle,
               _In_ ACCESS_MASK DesiredAccess,
               _In_ PUNICODE_STRING VolumeName,
               _In_ LARGE_INTEGER FileId,
               _Out_opt_ PULONG Result,
               _In_ ULONG ShareAccess,
               _In_ ULONG OpenOptions
               );

_IRQL_requires_(PASSIVE_LEVEL)
VOID
NkCloseFile(
            _In_ HANDLE FileHandle
            );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkDeleteFile(
             _In_ PCUNICODE_STRING FileName
             );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRenameFile(
             _In_ HANDLE FileHandle,
             _In_ PCUNICODE_STRING NewFileName,
             _In_ BOOLEAN ReplaceIfExists
             );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkMoveFile(
           _In_ HANDLE FileHandle,
           _In_ PCUNICODE_STRING NewFilePath,
           _In_ BOOLEAN ReplaceIfExists
           );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkIsFileDirectory(
                  _In_ HANDLE FileHandle,
                  _Out_ PBOOLEAN Directory
                  );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
BOOLEAN
NkIsFileExisting(
                 _In_ PCUNICODE_STRING FileName
                 );


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkGetFileAttributes(
                    _In_ HANDLE FileHandle,
                    _Out_ PULONG FileAttributes
                    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkGetFileTime(
              _In_ HANDLE FileHandle,
              _Out_opt_ PLARGE_INTEGER CreationTime,
              _Out_opt_ PLARGE_INTEGER LastAccessTime,
              _Out_opt_ PLARGE_INTEGER LastWriteTime,
              _Out_opt_ PLARGE_INTEGER ChangeTime
              );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkGetFileSize(
              _In_ HANDLE FileHandle,
              _Out_ PLARGE_INTEGER FileSize
              );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkGetFilePointer(
                 _In_ HANDLE FileHandle,
                 _Out_ PLARGE_INTEGER CurrentByteOffset
                 );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkSetFileAttributes(
                    _In_ HANDLE FileHandle,
                    _In_ ULONG FileAttributes
                    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkSetFileTime(
              _In_ HANDLE FileHandle,
              _In_ LONGLONG CreationTime,
              _In_ LONGLONG LastAccessTime,
              _In_ LONGLONG LastWriteTime,
              _In_ LONGLONG ChangeTime
              );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkSetFileSize(
              _In_ HANDLE FileHandle,
              _In_ LARGE_INTEGER FileSize
              );


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkSetFilePointer(
                 _In_ HANDLE FileHandle,
                 _In_ LARGE_INTEGER CurrentByteOffset
                 );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkZeroFileContent(
                  _In_ HANDLE FileHandle,
                  _In_ PLARGE_INTEGER ByteOffset,
                  _In_ ULONG Length
                  );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCreateDirectory(
                  _In_ PCUNICODE_STRING DirectoryName
                  );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCreateDirectoryEx(
                    _In_ PCUNICODE_STRING VolumeName,
                    _In_ PCUNICODE_STRING DirectoryName
                    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkCreateDirectorySafe(
                      _In_ PCUNICODE_STRING DirectoryName
                      );

/**@}*/ // Group End: nkdf-fs-fsop-api


/**@}*/ // Group End: nkdf-fs-fsop


/**@}*/ // Group End: nkdf-fs



#endif  // #ifndef __NKDF_FS_OPERATION_H__