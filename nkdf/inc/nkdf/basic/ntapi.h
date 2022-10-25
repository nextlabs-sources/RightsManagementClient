
/**
 * \file <nkdf/basic/ntapi.h>
 * \brief Header file for NT Undocumented APIs
 *
 * This header file declare NT undocumented APIs, Enums and Sturcts
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */



#ifndef __NKDF_BASIC_NTAPI_H__
#define __NKDF_BASIC_NTAPI_H__



/**
 * \addtogroup nkdf-basic
 * @{
 */


/**
 * \defgroup nkdf-basic-ntapi NT Undocumented APIs
 * @{
 */


/**
 * \defgroup nkdf-basic-ntapi-enum Enums
 * @{
 */


/**
 *  \enum _SYSTEM_INFORMATION_CLASS
 *  Enum system information class
 */
enum _SYSTEM_INFORMATION_CLASS {

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

};
typedef enum _SYSTEM_INFORMATION_CLASS SYSTEM_INFORMATION_CLASS;        /**< SYSTEM_INFORMATION_CLASS */
typedef enum _SYSTEM_INFORMATION_CLASS* PSYSTEM_INFORMATION_CLASS;      /**< PSYSTEM_INFORMATION_CLASS */

/**
 * \enum _MEMORY_INFORMATION_CLASS
 */
//enum _MEMORY_INFORMATION_CLASS {
//    MemoryBasicInformation,     /**< Basic information.         (0) */
//    MemoryWorkingSetList,       /**< Working set information.   (1) */
//    MemorySectionName,          /**< Section name.              (2) */
//    MemoryBasicVlmInformation,  /**< Basic vlm information.     (3) */
//    MemoryWorkingSetExList      /**< WrokingSetEx information.  (4) */
//};
//typedef enum _MEMORY_INFORMATION_CLASS MEMORY_INFORMATION_CLASS;        /**< MEMORY_INFORMATION_CLASS */


/**@}*/ // Group End: nkdf-basic-ntapi-enum


/**
 * \defgroup nkdf-basic-ntapi-struct Structs
 * @{
 */

/**
 * \struct _SYSTEM_MODULE_INFORMATION
 */
struct _SYSTEM_MODULE_INFORMATION {
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
};
typedef struct _SYSTEM_MODULE_INFORMATION SYSTEM_MODULE_INFORMATION;    /**< SYSTEM_MODULE_INFORMATION */
typedef struct _SYSTEM_MODULE_INFORMATION *PSYSTEM_MODULE_INFORMATION;  /**< PSYSTEM_MODULE_INFORMATION */

/**
 * \struct _SYSTEM_PROCESS_INFORMATION
 */
struct _SYSTEM_PROCESS_INFORMATION {
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
};
typedef struct _SYSTEM_PROCESS_INFORMATION SYSTEM_PROCESS_INFORMATION;  /**< SYSTEM_PROCESS_INFORMATION */

/**
 * \struct _MEMORY_BASIC_INFORMATION
 */
//struct _MEMORY_BASIC_INFORMATION {
//    PVOID  BaseAddress;         /**< Base address */
//    PVOID  AllocationBase;      /**< Base of the allocation */
//    ULONG  AllocationProtect;   /**< Allocation protect */
//    SIZE_T RegionSize;          /**< Region size */
//    ULONG  State;               /**< State */
//    ULONG  Protect;             /**< Protect */
//    ULONG  Type;                /**< Type */
//};
//typedef struct _MEMORY_BASIC_INFORMATION MEMORY_BASIC_INFORMATION;      /**< MEMORY_BASIC_INFORMATION */
//typedef struct _MEMORY_BASIC_INFORMATION* PMEMORY_BASIC_INFORMATION;    /**< PMEMORY_BASIC_INFORMATION */

/**@}*/ // Group End: nkdf-basic-ntapi-struct



/**
 * \defgroup nkdf-basic-ntapi-api Routines
 * @{
 */


/**
 * \brief Function to get system module handle.
 * \IRQL PASSIVE_LEVEL
 * \param ModuleName Module's name.
 * \return Handle of the module.
 */
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
HANDLE
NkGetSysModuleHandle(
                     _In_ PCSTR ModuleName
                     );

/**
 * \brief Function to query system information.
 * \IRQL PASSIVE_LEVEL
 * \param SystemInformationClass System information class.
 * \param SystemInformation Output buffer to receive system information.
 * \param SystemInformationLength Output buffer size.
 * \param ReturnLength Pointer to a ULONG variable to receive returned length.
 * \return Handle of the module.
 */
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

/**
 * \brief Function to query process information.
 * \IRQL PASSIVE_LEVEL
 * \param ProcessHandle Process handle.
 * \param ProcessInformationClass Process information class.
 * \param ProcessInformation Output buffer to receive process information.
 * \param ProcessInformationLength Output buffer size.
 * \param ReturnLength Pointer to a ULONG variable to receive returned length.
 * \return NTSTATUS.
 */
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

/**
 * \brief Function to query thread information.
 * \IRQL PASSIVE_LEVEL
 * \param ThreadHandle Thread handle.
 * \param ThreadInformationClass Thread information class.
 * \param ThreadInformation Output buffer to receive thread information.
 * \param ThreadInformationLength Output buffer size.
 * \param ReturnLength Pointer to a ULONG variable to receive returned length.
 * \return NTSTATUS.
 */
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

/**
 * \brief Get pointer to the EPROCESS structure of the process which specified thread belongs to.
 * \IRQL PASSIVE_LEVEL
 * \param Thread Pointer to the ETHREAD structure of thread.
 * \return Pointer to the EPROCESS structure.
 */
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
PEPROCESS
NTAPI
NkPsGetThreadProcess(
                     _In_ PETHREAD Thread
                     );

/**
 * \brief Get thread id from ETHREAD.
 * \IRQL PASSIVE_LEVEL
 * \param Thread Pointer to the ETHREAD structure of thread.
 * \return Thread id.
 */
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
HANDLE
NTAPI
NkPsGetThreadId(
                _In_ PETHREAD Thread
                );

/**
 * \brief Get id of the process which specified thread belongs to.
 * \IRQL PASSIVE_LEVEL
 * \param Thread Pointer to the ETHREAD structure of thread.
 * \return Thread id.
 */
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
HANDLE
NTAPI
NkPsGetThreadProcessId(
                       _In_ PETHREAD Thread
                       );

/**
 * \brief Read virtual memory.
 * \IRQL PASSIVE_LEVEL
 * \param ProcessHandle Process handle.
 * \param BaseAddress Base address where the read begins from.
 * \param Buffer Output buffer receiving read data.
 * \param NumberOfBytesToRead Number of bytes to read.
 * \param NumberOfBytesRead Number of bytes actual read.
 * \return Thread id.
 */
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

/**
 * \brief Write virtual memory.
 * \IRQL PASSIVE_LEVEL
 * \param ProcessHandle Process handle.
 * \param BaseAddress Base address where the read begins from.
 * \param Buffer Input buffer containg data to write.
 * \param NumberOfBytesToWrite Number of bytes to write.
 * \param NumberOfBytesWritten Number of bytes actual written.
 * \return Thread id.
 */
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

/**
 * \brief Protect virtual memory.
 * \IRQL PASSIVE_LEVEL
 * \param ProcessHandle Process handle.
 * \param BaseAddress Pointer to a variable which contains base address of the virtual memory.
 * \param NumberOfBytesToProtect Number of bytes to protect.
 * \param NewAccessProtection ULONG variable which contains access protection.
 * \param OldAccessProtection Pointer to a ULONG variable to receive old access protection.
 * \return Thread id.
 */
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

/**
 * \brief Query virtual memory information.
 * \IRQL PASSIVE_LEVEL
 * \param ProcessHandle Process handle.
 * \param BaseAddress Pointer to a variable which contains base address of the virtual memory.
 * \param MemoryInformationClass Memory information class.
 * \param MemoryInformation Output buffer to receive memory information.
 * \param MemoryInformationLength Output buffer size.
 * \param ReturnLength Pointer to a ULONG variable to receive returned length.
 * \return NTSTATUS.
 */
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


/**@}*/ // Group End: nkdf-basic-ntapi-api


/**@}*/ // Group End: nkdf-basic-ntapi


/**@}*/ // Group End: nkdf-basic



#endif  // __NKDF_BASIC_NTAPI_H__