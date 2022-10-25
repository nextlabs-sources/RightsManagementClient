

#include <Ntifs.h>
#include <ntddk.h>

#include <nkdf/basic/defines.h>
#include <nkdf/basic/string.h>
#include <nkdf/basic/image.h>
#include <nkdf/basic/ntapi.h>


// -----------------------------------------------------
//  DECLARE NT APIs
// -----------------------------------------------------

typedef
NTSTATUS
(NTAPI* ZWQUERYSYSTEMINFORMATION)(
                                  _In_ SYSTEM_INFORMATION_CLASS SystemInformationClass,
                                  _Out_opt_ PVOID SystemInformation,
                                  _In_ ULONG SystemInformationLength,
                                  _Out_opt_ PULONG ReturnLength
                                  );

typedef
NTSTATUS
(NTAPI* ZWQUERYINFORMATIONPROCESS)(
                                   _In_ HANDLE ProcessHandle,
                                   _In_ PROCESSINFOCLASS ProcessInformationClass,
                                   _Out_opt_ PVOID ProcessInformation,
                                   _In_ ULONG ProcessInformationLength,
                                   _Out_opt_ PULONG ReturnLength
                                   );

typedef
NTSTATUS
(NTAPI* ZWQUERYINFORMATIONTHREAD)(
                                  _In_ HANDLE ThreadHandle,
                                  _In_ THREADINFOCLASS ThreadInformationClass,
                                  _Inout_ PVOID ThreadInformation,
                                  _In_ ULONG ThreadInformationLength,
                                  _Out_opt_ PULONG ReturnLength
                                  );

typedef
PEPROCESS
(NTAPI* PSGETTHREADPROCESS)(
                            _In_ PETHREAD Thread
                            );

typedef
HANDLE
(NTAPI* PSGETTHREADID)(
                       _In_ PETHREAD Thread
                       );

typedef
HANDLE
(NTAPI* PSGETTHREADPROCESSID)(
                              _In_ PETHREAD Thread
                              );

typedef
NTSTATUS
(NTAPI* ZWREADVIRTUALMEMORY)(
                             _In_ HANDLE ProcessHandle,
                             _In_ PVOID BaseAddress,
                             _Out_ PVOID Buffer,
                             _In_ ULONG NumberOfBytesToRead,
                             _Out_ PULONG NumberOfBytesRead
                             );

typedef
NTSTATUS
(NTAPI* ZWWRITEVIRTUALMEMORY)(
                              _In_ HANDLE ProcessHandle,
                              _In_ PVOID BaseAddress,
                              _In_ PVOID Buffer,
                              _In_ ULONG NumberOfBytesToWrite,
                              _Out_ PULONG NumberOfBytesWritten
                              );

typedef
NTSTATUS
(NTAPI* ZWPROTECTVIRTUALMEMORY)(
                                _In_ HANDLE ProcessHandle,
                                _In_ PVOID *BaseAddress,
                                _In_ PSIZE_T NumberOfBytesToProtect,
                                _In_ ULONG NewAccessProtection,
                                _Out_ PULONG OldAccessProtection
                                );

typedef
NTSTATUS
(NTAPI* ZWQUERYVIRTUALMEMORY)(
                              _In_ HANDLE ProcessHandle,
                              _In_ PVOID BaseAddress,
                              _In_ MEMORY_INFORMATION_CLASS MemoryInformationClass,
                              _Out_ PVOID MemoryInformation,
                              _In_ ULONG MemoryInformationLength,
                              _Out_ PULONG ReturnLength
                              );




// -----------------------------------------------------
//  DECLARE LOCAL ROUTINES
// -----------------------------------------------------

_IRQL_requires_(PASSIVE_LEVEL)
PVOID
NkLoadSysRoutine(
                 _In_ PCWSTR Name
                 );

_Check_return_
ULONG
NtdllApiToServiceNo(
                    _In_ PVOID NtdllApi
                    );

_Check_return_
NTSTATUS
NkGetServiceInfo(
                 _In_ PCSTR ServiceName,
                 _Out_ PULONG ServiceNo,
                 _Out_opt_ PVOID* ServiceAddress
                 );

_Check_return_
ULONG
NkSysGetServiceNumberByName(
                            _In_ HANDLE NtdllModule, 
                            _In_ BOOLEAN Virtual,
                            _In_ PCSTR ServiceName
                            );

#ifdef _WIN64

ULONG
NkGetMagicInstructOffset(
                         _In_ PUCHAR ServiceAddress,
                         _In_ ULONG ServiceNo
                         );

PUCHAR
NkGetMagicInstructAddress(
                          _In_ PUCHAR ServiceAddress,
                          _In_ ULONG ServiceNo
                          );

_Check_return_
ULONG
NkGetServiceLength(
                   _In_ PUCHAR ServiceAddress,
                   _In_ ULONG ServiceNo,
                   _In_ BOOLEAN Virtual
                   );

_Check_return_
PVOID
NkSysGetServiceAddressByNumber(
                               _In_ HANDLE NtdllModule,
                               _In_ BOOLEAN Virtual,
                               _In_ ULONG ServiceNo
                               );

#else   // #ifdef _WIN64

NTSTATUS
ZwXxxVirtualMemoryAsm(
                      ULONG ServiceId,
                      HANDLE ProcessHandle,
                      PVOID Param1,
                      PVOID Param2,
                      ULONG Param3,
                      PULONG Param4
                      );

NTSTATUS
ZwQueryVirtualMemoryAsm(
                        ULONG ServiceId,
                        HANDLE ProcessHandle,
                        PVOID BaseAddress,
                        ULONG MemoryInformationClass,
                        PVOID MemoryInformation,
                        ULONG MemoryInformationLength,
                        PULONG ReturnLength
                        );

#endif  // #ifdef _WIN64


// -----------------------------------------------------
//  DEFINE EXPORT ROUTINES
// -----------------------------------------------------



_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
HANDLE
NkGetSysModuleHandle(
                     _In_ PCSTR ModuleName
                     )
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE Handle = NULL;
    ULONG Size = 0;
    PULONG_PTR InfoBuf = NULL;
    ULONG i = 0;



    try {

        PSYSTEM_MODULE_INFORMATION Modules = NULL;

        Status = NkZwQuerySystemInformation(SystemModuleInformation, &i, sizeof(ULONG), &Size);
        if (0 == Size) {
            try_return(Status);
        }

        InfoBuf = ExAllocatePoolWithTag(PagedPool, Size * 2, TAG_TEMP);
        if (NULL == InfoBuf) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }
        RtlZeroMemory(InfoBuf, Size * 2);

        Status = NkZwQuerySystemInformation(SystemModuleInformation, InfoBuf, Size * 2, NULL);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Modules = (PVOID)(InfoBuf + 1);
        for (i = 0; i < *((ULONG*)InfoBuf); i++) {

            // now we loop through all modules to find this system module
            if (0 == NkCompareStringA(ModuleName, &Modules[i].ImageName[Modules[i].ModuleNameOffset], TRUE))
            {
                // found it!
                Handle = Modules[i].ImageBase;
                break;
            }
        }

try_exit: NOTHING;
    }
    finally {
        if (NULL != InfoBuf) {
            ExFreePool(InfoBuf);
        }
    }

    return Handle;
}


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NTAPI
NkZwQuerySystemInformation(
                           _In_ SYSTEM_INFORMATION_CLASS SystemInformationClass,
                           _Out_opt_ PVOID SystemInformation,
                           _In_ ULONG SystemInformationLength,
                           _Out_opt_ PULONG ReturnLength
                           )
{
    static ZWQUERYSYSTEMINFORMATION ZwQuerySystemInformation = (ZWQUERYSYSTEMINFORMATION)-1;

    if ((ZWQUERYSYSTEMINFORMATION)-1 == ZwQuerySystemInformation) {
        ZwQuerySystemInformation = (ZWQUERYSYSTEMINFORMATION)NkLoadSysRoutine(L"ZwQuerySystemInformation");
    }

    if (NULL == ZwQuerySystemInformation) {
        return STATUS_PROCEDURE_NOT_FOUND;
    }

    return ZwQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NTAPI
NkZwQueryInformationProcess(
                            _In_ HANDLE ProcessHandle,
                            _In_ PROCESSINFOCLASS ProcessInformationClass,
                            _Out_opt_ PVOID ProcessInformation,
                            _In_ ULONG ProcessInformationLength,
                            _Out_opt_ PULONG ReturnLength
                            )
{
    static ZWQUERYINFORMATIONPROCESS ZwQueryInformationProcess = (ZWQUERYINFORMATIONPROCESS)-1;

    if ((ZWQUERYINFORMATIONPROCESS)-1 == ZwQueryInformationProcess) {
        ZwQueryInformationProcess = (ZWQUERYINFORMATIONPROCESS)NkLoadSysRoutine(L"ZwQueryInformationProcess");
    }

    if (NULL == ZwQueryInformationProcess) {
        return STATUS_PROCEDURE_NOT_FOUND;
    }

    return ZwQueryInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NTAPI
NkZwQueryInformationThread(
                           _In_ HANDLE ThreadHandle,
                           _In_ THREADINFOCLASS ThreadInformationClass,
                           _Inout_ PVOID ThreadInformation,
                           _In_ ULONG ThreadInformationLength,
                           _Out_opt_ PULONG ReturnLength
                           )
{
    static ZWQUERYINFORMATIONTHREAD ZwQueryInformationThread = (ZWQUERYINFORMATIONTHREAD)-1;

    if ((ZWQUERYINFORMATIONTHREAD)-1 == ZwQueryInformationThread) {
        ZwQueryInformationThread = (ZWQUERYINFORMATIONTHREAD)NkLoadSysRoutine(L"ZwQueryInformationThread");
    }

    if (NULL == ZwQueryInformationThread) {
        return STATUS_PROCEDURE_NOT_FOUND;
    }

    return ZwQueryInformationThread(ThreadHandle, ThreadInformationClass, ThreadInformation, ThreadInformationLength, ReturnLength);
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
PEPROCESS
NTAPI
NkPsGetThreadProcess(
                     _In_ PETHREAD Thread
                     )
{
    static PSGETTHREADPROCESS PsGetThreadProcess = (PSGETTHREADPROCESS)-1;

    if ((PSGETTHREADPROCESS)-1 == PsGetThreadProcess) {
        PsGetThreadProcess = (PSGETTHREADPROCESS)NkLoadSysRoutine(L"PsGetThreadProcess");
    }

    if (NULL == PsGetThreadProcess) {
        return NULL;
    }

    return PsGetThreadProcess(Thread);
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
HANDLE
NTAPI
NkPsGetThreadId(
                _In_ PETHREAD Thread
                )
{
    static PSGETTHREADID PsGetThreadId = (PSGETTHREADID)-1;

    if ((PSGETTHREADID)-1 == PsGetThreadId) {
        PsGetThreadId = (PSGETTHREADID)NkLoadSysRoutine(L"PsGetThreadId");
    }

    if (NULL == PsGetThreadId) {
        return NULL;
    }

    return PsGetThreadId(Thread);
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
HANDLE
NTAPI
NkPsGetThreadProcessId(
                       _In_ PETHREAD Thread
                       )
{
    static PSGETTHREADPROCESSID PsGetThreadProcessId = (PSGETTHREADPROCESSID)-1;

    if ((PSGETTHREADPROCESSID)-1 == PsGetThreadProcessId) {
        PsGetThreadProcessId = (PSGETTHREADPROCESSID)NkLoadSysRoutine(L"PsGetThreadProcessId");
    }

    if (NULL == PsGetThreadProcessId) {
        return NULL;
    }

    return PsGetThreadProcessId(Thread);
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NTAPI
NkZwReadVirtualMemory(
                      _In_ HANDLE ProcessHandle,
                      _In_ PVOID BaseAddress,
                      _Out_ PVOID Buffer,
                      _In_ ULONG NumberOfBytesToRead,
                      _Out_ PULONG NumberOfBytesRead
                      )
{
    static ULONG ZwReadVmNo = (ULONG)-2;
    static ZWREADVIRTUALMEMORY   ZwReadVirtualMemory = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    if ((ULONG)-2 == ZwReadVmNo) {
        Status = NkGetServiceInfo("ZwReadVirtualMemory", &ZwReadVmNo, (PVOID*)&ZwReadVirtualMemory);
    }

    if ((ULONG)-1 == ZwReadVmNo) {
        return STATUS_PROCEDURE_NOT_FOUND;
    }

#ifdef _WIN64

    if (NULL == ZwReadVirtualMemory) {
        return STATUS_PROCEDURE_NOT_FOUND;
    }
    return ZwReadVirtualMemory(ProcessHandle, BaseAddress, Buffer, NumberOfBytesToRead, NumberOfBytesRead);

#else

    return ZwXxxVirtualMemoryAsm(ZwReadVmNo, ProcessHandle, BaseAddress, Buffer, NumberOfBytesToRead, NumberOfBytesRead);

#endif
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NTAPI
NkZwWriteVirtualMemory(
                       _In_ HANDLE ProcessHandle,
                       _In_ PVOID BaseAddress,
                       _In_ PVOID Buffer,
                       _In_ ULONG NumberOfBytesToWrite,
                       _Out_ PULONG NumberOfBytesWritten
                       )
{
    static ULONG ZwWriteVmNo = (ULONG)-2;
    static ZWWRITEVIRTUALMEMORY   ZwWriteVirtualMemory = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    if ((ULONG)-2 == ZwWriteVmNo) {
        Status = NkGetServiceInfo("ZwWriteVirtualMemory", &ZwWriteVmNo, (PVOID*)&ZwWriteVirtualMemory);
    }

    if ((ULONG)-1 == ZwWriteVmNo) {
        return STATUS_PROCEDURE_NOT_FOUND;
    }

#ifdef _WIN64

    if (NULL == ZwWriteVirtualMemory) {
        return STATUS_PROCEDURE_NOT_FOUND;
    }
    return ZwWriteVirtualMemory(ProcessHandle, BaseAddress, Buffer, NumberOfBytesToWrite, NumberOfBytesWritten);

#else

    return ZwXxxVirtualMemoryAsm(ZwWriteVmNo, ProcessHandle, BaseAddress, Buffer, NumberOfBytesToWrite, NumberOfBytesWritten);

#endif
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NTAPI
NkZwProtectVirtualMemory(
                         _In_ HANDLE ProcessHandle,
                         _In_ PVOID *BaseAddress,
                         _In_ PSIZE_T NumberOfBytesToProtect,
                         _In_ ULONG NewAccessProtection,
                         _Out_ PULONG OldAccessProtection
                         )
{
    static ULONG ZwProtectVmNo = (ULONG)-2;
    static ZWPROTECTVIRTUALMEMORY   ZwProtectVirtualMemory = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    if ((ULONG)-2 == ZwProtectVmNo) {
        Status = NkGetServiceInfo("ZwProtectVirtualMemory", &ZwProtectVmNo, (PVOID*)&ZwProtectVirtualMemory);
    }

    if ((ULONG)-1 == ZwProtectVmNo) {
        return STATUS_PROCEDURE_NOT_FOUND;
    }

#ifdef _WIN64

    if (NULL == ZwProtectVirtualMemory) {
        return STATUS_PROCEDURE_NOT_FOUND;
    }
    return ZwProtectVirtualMemory(ProcessHandle, BaseAddress, NumberOfBytesToProtect, NewAccessProtection, OldAccessProtection);

#else
    
    return ZwXxxVirtualMemoryAsm(ZwProtectVmNo, ProcessHandle, (PVOID)BaseAddress, (PVOID)NumberOfBytesToProtect, NewAccessProtection, OldAccessProtection);

#endif
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NTAPI
NkZwQueryVirtualMemory(
                       _In_ HANDLE ProcessHandle,
                       _In_ PVOID BaseAddress,
                       _In_ MEMORY_INFORMATION_CLASS MemoryInformationClass,
                       _Out_ PVOID MemoryInformation,
                       _In_ ULONG MemoryInformationLength,
                       _Out_ PULONG ReturnLength
                       )
{
    static ULONG ZwQueryVmNo = (ULONG)-2;
    static ZWQUERYVIRTUALMEMORY   ZwQueryVirtualMemory = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    if ((ULONG)-2 == ZwQueryVmNo) {
        Status = NkGetServiceInfo("ZwQueryVirtualMemory", &ZwQueryVmNo, (PVOID*)&ZwQueryVirtualMemory);
    }

    if ((ULONG)-1 == ZwQueryVmNo) {
        return STATUS_PROCEDURE_NOT_FOUND;
    }

#ifdef _WIN64

    if (NULL == ZwQueryVirtualMemory) {
        return STATUS_PROCEDURE_NOT_FOUND;
    }
    return ZwQueryVirtualMemory(ProcessHandle, BaseAddress, MemoryInformationClass, MemoryInformation, MemoryInformationLength, ReturnLength);

#else

    return ZwQueryVirtualMemoryAsm(ZwQueryVmNo, ProcessHandle, BaseAddress, MemoryInformationClass, MemoryInformation, MemoryInformationLength, ReturnLength);

#endif
}



// -----------------------------------------------------
//  DEFINE LOCAL ROUTINES
// -----------------------------------------------------

_IRQL_requires_(PASSIVE_LEVEL)
PVOID
NkLoadSysRoutine(
                 _In_ PCWSTR Name
                 )
{
    UNICODE_STRING NameW = { 0, 0, NULL };
    RtlInitUnicodeString(&NameW, Name);
    return MmGetSystemRoutineAddress(&NameW);
}

_Check_return_
ULONG
NtdllApiToServiceNo(
                    _In_ PVOID NtdllApi
                    )
{

    if (NULL == NtdllApi) {
        return (ULONG)-1;
    }

#ifdef _WIN64

    // most native APIs in the 64bit "ntdll.dll" begin with the code:
    //
    // asm
    //   mov r10, rcx   (binary: 4c 8b d1)
    //   mov eax, dw    (binary: b8 dw)

    if (((ULONG*)NtdllApi)[0] == 0xb8d18b4c) {
        return ((ULONG*)NtdllApi)[1];
    }

#else

    // XPSP2 and higher:
    //   mov eax, dw           (binary: b8 dw)
    //   mov edx, dw           (binary: ba dw)
    //   call dword ptr [edx]  (binary: ff 12)
    //   retn w                (binary: c2 w)
    if ((*(UCHAR *)((ULONG_PTR)NtdllApi + 0) == 0xb8) &&
        (*(UCHAR *)((ULONG_PTR)NtdllApi + 5) == 0xba) &&
        (*(USHORT*)((ULONG_PTR)NtdllApi + 10) == 0x12ff) &&
        (*(UCHAR *)((ULONG_PTR)NtdllApi + 12) == 0xc2)) {
        return  *(ULONG*)((ULONG_PTR)NtdllApi + 1);
    }


    // Windows 8:
    //   mov eax, dw           (binary: b8 dw)
    //   call dw               (binary: e8 dw)
    //   retn w                (binary: c2 w)
    if ((*(UCHAR *)((ULONG_PTR)NtdllApi + 0) == 0xb8) &&
        (*(UCHAR *)((ULONG_PTR)NtdllApi + 5) == 0xe8) &&
        (*(UCHAR *)((ULONG_PTR)NtdllApi + 10) == 0xc2)) {
        return *(ULONG*)((ULONG_PTR)NtdllApi + 1);
    }


    // XPSP0 + XPSP1:
    //   mov eax, dw  (binary: b8 dw)
    //   mov edx, dw  (binary: ba dw)
    //   call edx     (binary: ff d2)
    //   retn w       (binary: c2 w)

    if ((*(UCHAR *)((ULONG_PTR)NtdllApi + 0) == 0xb8) &&
        (*(UCHAR *)((ULONG_PTR)NtdllApi + 5) == 0xba) &&
        (*(USHORT*)((ULONG_PTR)NtdllApi + 10) == 0xd2ff) &&
        (*(UCHAR *)((ULONG_PTR)NtdllApi + 12) == 0xc2)) {
        return *(ULONG*)((ULONG_PTR)NtdllApi + 1);
    }


    // NT4 + 2000:
    //   mov eax, dw           (binary: b8 dw)
    //   lea edx, [esp+4]      (binary: 8d 54 24 04)
    //   int 0x2e              (binary: cd 2e)
    //   retn w                (binary: c2 w)
    if ((*(UCHAR *)((ULONG_PTR)NtdllApi + 0) == 0xb8) &&
        (*(ULONG *)((ULONG_PTR)NtdllApi + 5) == 0x0424548d) &&
        (*(USHORT*)((ULONG_PTR)NtdllApi + 9) == 0x2ecd) &&
        (*(UCHAR *)((ULONG_PTR)NtdllApi + 11) == 0xc2)) {
        return *(ULONG*)((ULONG_PTR)NtdllApi + 1);
    }
#endif
    
    return (ULONG)-1;
}



_Check_return_
NTSTATUS
NkGetServiceInfo(
                 _In_ PCSTR ServiceName,
                 _Out_ PULONG ServiceNo,
                 _Out_opt_ PVOID* ServiceAddress
                 )
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE NtdllFile = NULL;
    HANDLE NtdllSection = NULL;
    HANDLE NtdllModule = NULL;
    BOOLEAN Virtual = TRUE;


    *ServiceNo = (ULONG)-1;
    if (NULL != ServiceAddress) {
        *ServiceAddress = NULL;
    }

    try {

        // Make sure Ntdll.dll is mapped
        NtdllModule = NkGetSysModuleHandle("ntdll.dll");
        if (NULL == NtdllModule) {
            UNICODE_STRING NtdllPath;
            Virtual = FALSE;
            RtlInitUnicodeString(&NtdllPath, L"\\SystemRoot\\System32\\ntdll.dll");
            Status = NkMapDllFile(&NtdllPath, &NtdllFile, &NtdllSection, &NtdllModule);
            if (!NT_SUCCESS(Status)) {
                try_return(Status);
            }
        }

        // Get Service Number
        *ServiceNo = NkSysGetServiceNumberByName(NtdllModule, Virtual, ServiceName);
        if ((ULONG)-1 == *ServiceNo) {
            try_return(Status = STATUS_PROCEDURE_NOT_FOUND);
        }

#ifdef _WIN64
        // Get Service Address
        if (NULL != ServiceAddress) {
            *ServiceAddress = NkSysGetServiceAddressByNumber(NtdllModule, Virtual, *ServiceNo);
        }
#endif

try_exit: NOTHING;
    }
    finally {

        if(!Virtual) {
            NkUnmapDllFile(NtdllFile, NtdllSection, NtdllModule);
        }
    }

    return Status;
}

_Check_return_
ULONG
NkSysGetServiceNumberByName(
                            _In_ HANDLE NtdllModule, 
                            _In_ BOOLEAN Virtual,
                            _In_ PCSTR ServiceName
                            )
{
    PVOID NtdllApi = NkGetProcAddress(NtdllModule, ServiceName, Virtual);
    return NtdllApiToServiceNo(NtdllApi);
}

#ifdef _WIN64

ULONG
NkGetMagicInstructOffset(
                         _In_ PUCHAR ServiceAddress,
                         _In_ ULONG ServiceNo
                         )
{
    const ULONG   SearchRange = 101;    // The body must be less than 100 bytes
    PUCHAR Address = ServiceAddress;
    ULONG i = 0;

    if ((ULONG)-1 == ServiceNo) {
        return (ULONG)-1;
    }

    for (i = 0; i < SearchRange; i++) {
        if (0xB8 == Address[i] && ServiceNo == (*((ULONG*)(&Address[i + 1])))) {
            return i;
        }
    }

    return (ULONG)-1;
}

PUCHAR
NkGetMagicInstructAddress(
                          _In_ PUCHAR ServiceAddress,
                          _In_ ULONG ServiceNo
                          )
{
    const ULONG   SearchRange = 101;    // The body must be less than 100 bytes
    PUCHAR Address = ServiceAddress;

    if ((ULONG)-1 == ServiceNo) {
        return NULL;
    }
    
    while ((ULONG)(Address - ServiceAddress) <= SearchRange) {
        if (0xB8 == (*Address) && ServiceNo == (*((ULONG*)(Address + 1)))) {
            return Address;
        }
    }

    return NULL;
}

_Check_return_
ULONG
NkGetServiceLength(
                   _In_ PUCHAR ServiceAddress,
                   _In_ ULONG ServiceNo,
                   _In_ BOOLEAN Virtual
                   )
{
    PUCHAR StartAddress = NULL;
    PUCHAR EndAddress = NULL;

    StartAddress = NkGetMagicInstructAddress(ServiceAddress, ServiceNo);
    if (NULL == StartAddress) {
        return (ULONG)-1;
    }
    EndAddress = NkGetMagicInstructAddress(StartAddress+5, ServiceNo+1);
    if (NULL == EndAddress) {
        return (ULONG)-1;
    }

    return (ULONG)(EndAddress - StartAddress);
}

__forceinline
_Check_return_
BOOLEAN
VerifyServiceAddress(
                     _In_ PUCHAR Address,
                     _In_ ULONG Number,
                     _In_ ULONG InstructOffset
                     )
{
    return ((0xB8 == Address[InstructOffset]) && (Number == *((ULONG*)(&Address[InstructOffset+1]))));
}

_Check_return_
PVOID
NkSysGetServiceAddressByNumber(
                               _In_ HANDLE NtdllModule,
                               _In_ BOOLEAN Virtual,
                               _In_ ULONG ServiceNo
                               )
{
    static PUCHAR MagicAddress = NULL;
    static ULONG MagicServiceLength = (ULONG)-2;
    static ULONG MagicServiceNo = (ULONG)-2;
    static ULONG MagicInstructOffset = (ULONG)-2;

    PUCHAR TargetAddress = NULL;


    if ((ULONG)-1 == ServiceNo) {
        return NULL;
    }

    if(NULL == MagicAddress) {
#pragma warning(push)
#pragma warning(disable: 4054)
        // We need to convert function pointer to data address
        MagicAddress = (PVOID)ZwAllocateVirtualMemory;
#pragma warning(pop)
    }

    // Get Service Number of "ZwAllocateVirtualMemory" -- this is our Magic Service Number
    if ((ULONG)-2 == MagicServiceNo) {
        PVOID  MagicProcAddress = NkGetProcAddress(NtdllModule, "ZwAllocateVirtualMemory", Virtual);
        MagicServiceNo = NtdllApiToServiceNo(MagicProcAddress);
    }
    if ((ULONG)-1 == MagicServiceNo) {
        return NULL;
    }

    // Get Offset to instruct "MOV EAX, SERVICENO" in magic service
    // ** NOTE **:
    //     We assume this offset is fixed
    if ((ULONG)-2 == MagicInstructOffset) {
        MagicInstructOffset = NkGetMagicInstructOffset(MagicAddress, MagicServiceNo);
    }
    if ((ULONG)-1 == MagicInstructOffset) {
        return NULL;
    }

    // Get size of magic service body
    // ** NOTE **:
    //     We assume this size is fixed
    if ((ULONG)-2 == MagicServiceLength) {
        MagicServiceLength = NkGetServiceLength(MagicAddress, MagicServiceNo, Virtual);
    }
    if ((ULONG)-1 == MagicServiceLength) {
        return NULL;
    }

    //
    //  Calculate Address of target service:
    //    ADDRESS = MAGICADDRESS + MAGICSERVICE_LENGTH * (NEWSERVICE_NUMBER - MAGIC_SERVICE_NUMBER)
    //
    TargetAddress = MagicAddress + MagicServiceLength * (ServiceNo - MagicServiceNo);
    ASSERT(VerifyServiceAddress(TargetAddress, ServiceNo, MagicInstructOffset));
    if (!VerifyServiceAddress(TargetAddress, ServiceNo, MagicInstructOffset)) {
        TargetAddress = NULL;   // Doesn't match
    }

    return TargetAddress;
}

#else   // #ifdef _WIN64

#pragma optimize( "", off )

// ZwProtect/Read/WriteVirtualMemory
NTSTATUS
ZwXxxVirtualMemoryAsm(
                      ULONG ServiceId,
                      HANDLE ProcessHandle,
                      PVOID Param1,
                      PVOID Param2,
                      ULONG Param3,
                      PULONG Param4
                      )
{
    _asm
    {
        // this is the same that ntdll.dll does in user land
        // when ZwProtect/Read/WriteVirtualMemory are called
        mov eax, ServiceId
        lea edx, ServiceId
        add edx, 4
        int 0x2e
    }
}

NTSTATUS
ZwQueryVirtualMemoryAsm(
                        ULONG ServiceId,
                        HANDLE ProcessHandle,
                        PVOID BaseAddress,
                        ULONG MemoryInformationClass,
                        PVOID MemoryInformation,
                        ULONG MemoryInformationLength,
                        PULONG ReturnLength
                        )
{
    _asm
    {
        mov eax, ServiceId
        lea edx, ServiceId
        add edx, 4
        int 0x2e
    }
}

#pragma optimize( "", on )
#endif  // #ifdef _WIN64