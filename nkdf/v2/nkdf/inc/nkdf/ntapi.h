

#pragma once
#ifndef __NKDF_NTAPI_H__
#define __NKDF_NTAPI_H__



typedef enum SYSTEM_INFORMATION_CLASS {

    SystemBasicInformation,                 /**< Basic information.                 (0) */
    SystemProcessorInformation,             /**< Processor information.             (1) */
    SystemPerformanceInformation,           /**< Performance information.           (2) */
    SystemTimeOfDayInformation,             /**< Time of day information.           (3) */
    SystemPathInformation,                  /**< Path information.                  (4) */
    SystemProcessInformation,               /**< Process information.               (5) */
    SystemCallCountInformation,             /**< Call count information.            (6) */
    SystemDeviceInformation,                /**< Device information.                (7) */
    SystemProcessorPerformanceInformation,  /**< Processor performance information. (8) */
    SystemFlagsInformation,                 /**< Flags information.                 (9) */
    SystemCallTimeInformation,              /**< Call time information.             (10) */
    SystemModuleInformation,                /**< Module information.                (11) */
    SystemLocksInformation,                 /**< Locks information.                 (12) */
    SystemStackTraceInformation,            /**< Stack trace information.           (13) */
    SystemPagedPoolInformation,             /**< Paged pool information.            (14) */
    SystemNonPagedPoolInformation,          /**< Non-paged pool information.        (15) */
    SystemHandleInformation,                /**< Handle information.                (16) */
    SystemObjectInformation,                /**< Object information.                (17) */
    SystemPageFileInformation,              /**< Page file information.             (18) */
    SystemVdmInstemulInformation,           /**< Vdm instemul information.          (19) */
    SystemVdmBopInformation,                /**< Vdm bop information.               (20) */
    SystemFileCacheInformation,             /**< File cache information.            (21) */
    SystemPoolTagInformation,               /**< Pool tag information.              (22) */
    SystemInterruptInformation,             /**< Interrupt information.             (23) */
    SystemDpcBehaviorInformation,           /**< Dpc behavior information.          (24) */
    SystemFullMemoryInformation,            /**< Full memory information.           (25) */
    SystemLoadGdiDriverInformation,         /**< Load GDI driver information.       (26) */
    SystemUnloadGdiDriverInformation,       /**< Unload GDI driver information.     (27) */
    SystemTimeAdjustmentInformation,        /**< Time adjustment information.       (28) */
    SystemSummaryMemoryInformation,         /**< Summary memory information.        (29) */
    SystemNextEventIdInformation,           /**< Next event id information.         (30) */
    SystemEventIdsInformation,              /**< Event IDs information.             (31) */
    SystemCrashDumpInformation,             /**< Crash dump information.            (32) */
    SystemExceptionInformation,             /**< Exception information.             (33) */
    SystemCrashDumpStateInformation,        /**< Crash dump state information.      (34) */
    SystemKernelDebuggerInformation,        /**< Kernel debugger information.       (35) */
    SystemContextSwitchInformation,         /**< Context switch information.        (36) */
    SystemRegistryQuotaInformation,         /**< Registry quota information.        (37) */
    SystemExtendServiceTableInformation,    /**< Extend service table information.  (38) */
    SystemPrioritySeperation,               /**< Priority seperation.               (39) */
    SystemPlugPlayBusInformation,           /**< Pnp bus information.               (40) */
    SystemDockInformation,                  /**< Dock information.                  (41) */
    SystemPowerInformation_0,               /**< Power information-0.               (42) */
    SystemProcessorSpeedInformation,        /**< Processor speed information.       (43) */
    SystemCurrentTimeZoneInformation,       /**< Current time-zone information.     (44) */
    SystemLookasideInformation              /**< Lookaside information.             (45) */

} SYSTEM_INFORMATION_CLASS; 

//typedef enum MEMORY_INFORMATION_CLASS {
//    MemoryBasicInformation,     /**< Basic information.         (0) */
//    MemoryWorkingSetList,       /**< Working set information.   (1) */
//    MemorySectionName,          /**< Section name.              (2) */
//    MemoryBasicVlmInformation,  /**< Basic vlm information.     (3) */
//    MemoryWorkingSetExList      /**< WrokingSetEx information.  (4) */
//} MEMORY_INFORMATION_CLASS;

typedef struct _SYSTEM_MODULE_INFORMATION {
    PVOID  Reserved;        /**< Reserved */
    PVOID  MappedBase;      /**< Mapped base */
    PVOID  ImageBase;       /**< Image base */
    ULONG  Size;            /**< Size */
    ULONG  Flags;           /**< Flags */
    USHORT Index;           /**< Index */
    USHORT Unknown;         /**< Unknown */
    USHORT LoadCount;       /**< Load count */
    USHORT ModuleNameOffset;/**< Module name offset */
    CHAR   ImageName[256];  /**< Image name */
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

typedef struct _SYSTEM_PROCESS_INFORMATION {
    ULONG   NextEntryOffset;    /**< Offset to next entry */
    ULONG   NumberOfThreads;    /**< Number of threads */
    UCHAR   Reserved1[48];      /**< Reserved 1 */
    PVOID   Reserved2[3];       /**< Reserved 2 */
    HANDLE  UniqueProcessId;    /**< Process Id */
    PVOID   Reserved3;          /**< Reserved 3 */
    ULONG   HandleCount;        /**< Count of handles */
    UCHAR   Reserved4[4];       /**< Reserved 4 */
    PVOID   Reserved5[11];      /**< Reserved 5 */
    SIZE_T  PeakPagefileUsage;  /**< Peak page file usage */
    SIZE_T  PrivatePageCount;   /**< Private page count */
    LARGE_INTEGER Reserved6[6]; /**< Reserved 6 */
} SYSTEM_PROCESS_INFORMATION;  /**< SYSTEM_PROCESS_INFORMATION */

//typedef struct _MEMORY_BASIC_INFORMATION {
//    PVOID  BaseAddress;         /**< Base address */
//    PVOID  AllocationBase;      /**< Base of the allocation */
//    ULONG  AllocationProtect;   /**< Allocation protect */
//    SIZE_T RegionSize;          /**< Region size */
//    ULONG  State;               /**< State */
//    ULONG  Protect;             /**< Protect */
//    ULONG  Type;                /**< Type */
//} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
HANDLE
NkGetSysModuleHandle(
    _In_ PCSTR ModuleName
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NTAPI
NkZwQuerySystemInformation(
    _In_ SYSTEM_INFORMATION_CLASS SystemInformationClass,
    _Out_opt_ PVOID SystemInformation,
    _In_ ULONG SystemInformationLength,
    _Out_opt_ PULONG ReturnLength
    );

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
    );

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
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
PEPROCESS
NTAPI
NkPsGetThreadProcess(
    _In_ PETHREAD Thread
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
HANDLE
NTAPI
NkPsGetThreadId(
    _In_ PETHREAD Thread
    );

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
HANDLE
NTAPI
NkPsGetThreadProcessId(
    _In_ PETHREAD Thread
    );

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
    );

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
    );

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
    );

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
    );


_Check_return_
ULONG
NkVirtualAddressToRaw(
    _In_ PIMAGE_NT_HEADERS ImgNtHdr,
    _In_ ULONG VirtualAddress
    );

_Check_return_
PVOID
NkGetProcAddress(
    _In_ HANDLE ModuleHandle,
    _In_ PCSTR ProcName,
    _In_ BOOLEAN Virtual
    );

_Check_return_
PVOID
NkGetAddressOfEntryPoint(
    _In_ HANDLE ModuleHandle
    );


_Check_return_
NTSTATUS
NkMapDllFile(
    _In_ PCUNICODE_STRING DllPath,
    _Out_ PHANDLE FileHandle,
    _Out_ PHANDLE SectionHandle,
    _Out_ PHANDLE ModuleHandle
    );

VOID
NkUnmapDllFile(
    _In_ HANDLE FileHandle,
    _In_ HANDLE SectionHandle,
    _In_ HANDLE ModuleHandle
    );


#endif