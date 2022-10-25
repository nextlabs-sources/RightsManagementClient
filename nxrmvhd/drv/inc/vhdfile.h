

#ifndef __NXRM_VHD_FILE_H__
#define __NXRM_VHD_FILE_H__



_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdOpen(
        _In_ PCUNICODE_STRING DiskFile,
        _Out_ PHANDLE Vhd
        );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdClose(
         _In_ HANDLE Vhd
         );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdRead(
        _In_ HANDLE Vhd,
        _In_ LONGLONG Start,
        _In_ ULONG Length,
        _Out_writes_(Length) PUCHAR Buffer,
        _Out_ PULONG BytesReturned
        );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdWrite(
         _In_ HANDLE Vhd,
         _In_ LONGLONG Start,
         _In_ ULONG Length,
         _In_reads_(Length) const UCHAR* Buffer,
         _Out_ PULONG BytesReturned
         );

_Check_return_
LONGLONG
VhdGetVolumeSize(
                 _In_ HANDLE Vhd
                 );



#endif