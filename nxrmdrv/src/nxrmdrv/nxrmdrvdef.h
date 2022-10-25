#pragma once

#include <nudf\shared\rbtree.h>

#define MAX_PATH_LEN				268

#define SystemProcessInformation	5
#define SystemHandleInformation		16

#define PROCESS_QUERY_INFORMATION          (0x0400)

#ifndef PKNORMAL_ROUTINE
#define PKNORMAL_ROUTINE	PVOID
#endif

#ifndef PKKERNEL_ROUTINE
#define PKKERNEL_ROUTINE	PVOID
#endif

#ifndef PKRUNDOWN_ROUTINE
#define PKRUNDOWN_ROUTINE	PVOID
#endif

typedef NTSTATUS (__stdcall *ZWPROTECTVIRTUALMEMORY)(
	IN		HANDLE	ProcessHandle,
	IN OUT	PVOID	*BaseAddress,
	IN OUT	PSIZE_T	NumberOfBytesToProtect,
	IN		ULONG	NewAccessProtection,
	OUT		PULONG	OldAccessProtection
	);

typedef NTSTATUS (__stdcall *ZWFLUSHINSTRUCTIONCACHE)(
	IN		HANDLE	ProcessHandle,  
	IN		PVOID	BaseAddress,  
	IN		ULONG	NumberOfBytesToFlush 
	);

typedef NTSTATUS (__stdcall *ZWFREEVIRTUALMEMORY)(
	IN		HANDLE	ProcessHandle,
	IN OUT	PVOID	*BaseAddress,
	IN OUT	PSIZE_T RegionSize,
	IN		ULONG	FreeType
	);

typedef NTSTATUS (__stdcall *ZWCREATETHREAD)(
	OUT		HANDLE				*ThreadHandle,
	IN		ACCESS_MASK			DesiredAccess,
	IN		OBJECT_ATTRIBUTES	*ObjectAttributes,
	IN		HANDLE				ProcessHandle,
	OUT		CLIENT_ID			*ClientId,
	IN		CONTEXT				*ThreadContext,
	IN		PVOID				UserStack,
	IN		BOOLEAN				CreateSuspended
	);

typedef NTSTATUS(__stdcall *ZWCREATETHREADEX)(
	OUT		HANDLE					*ThreadHandle,
	IN		ACCESS_MASK				DesiredAccess,
	IN		OBJECT_ATTRIBUTES		*ObjectAttributes,
	IN		HANDLE					ProcessHandle,
	IN		ULONG_PTR				lpStartAddress,
	IN		PVOID					lpParameter,
	IN		ULONG_PTR				CreateSuspended,
	IN		ULONG_PTR				StackZeroBits,
	IN		ULONG_PTR				SizeOfStackCommit,
	IN		ULONG_PTR				SizeOfStackReserve,
	OUT		PVOID					lpBuffer
	);

typedef NTSTATUS (__stdcall *OBREGISTERCALLBACKS)(
	__in		POB_CALLBACK_REGISTRATION	CallbackRegistration,
	__deref_out PVOID						*RegistrationHandle
	);

typedef NTSTATUS (__stdcall *RTLQUERYPACKAGEIDENTITY)(
	IN			PACCESS_TOKEN	pToken,
	OUT			WCHAR			*wszPackageFullName,
	IN OUT		size_t			*ulPackageFullNameLength,
	OUT			WCHAR			*wszPackageID,
	IN OUT		size_t			*ulPackageIDLength,
	OUT			WCHAR			*wszPackageIDExt);

#ifndef PROCESS_TERMINATE
#define PROCESS_TERMINATE                  (0x0001)
#endif

#ifndef PROCESS_VM_WRITE
#define PROCESS_VM_WRITE                   (0x0020)
#endif

typedef enum _NXRM_INSTANCE_TYPE{
	
	NXRM_INSTANCE_UNKNOWN	= 0x0000,
	NXRM_INSTANCE_RM		= 0x1000

}NXRM_INSTANCE_TYPE;

//
// A structure representing the instance information associated with
// a particular device
//
#pragma pack(push,8)

typedef struct _NXRM_OPENINSTANCE
{
	PMDL				u_section_mdl;
	
	PVOID				u_section;
	
	BOOLEAN				is_server;
	
	BOOLEAN				is_badguy;
	
	BOOLEAN				open_in_progress;
	
	LIST_ENTRY			open_link;
	
	KSPIN_LOCK			open_lock;
	
	HANDLE				hsem;
	
	HANDLE				hnotifyevent;
	
	NXRM_INSTANCE_TYPE	type;

}NXRM_OPENINSTANCE, *PNXRM_OPENINSTANCE;

typedef struct _DEVICE_EXTENSION
{
	PVOID				rm_k_section;

	ULONG				rm_section_size;

	LONG				rm_instance_count;

	NXRM_OPENINSTANCE	*rm_server_instance;

	PVOID				rm_semaphore_object;

	HANDLE				rm_semaphore_kernel_handle;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

typedef struct _NXRM_PROCESS
{
	HANDLE				process_id;
	
	HANDLE				parent_id;
	
	WCHAR				process_path[MAX_PATH_LEN];
	
	BOOLEAN				is_Service;
	
	BOOLEAN				is_CLR;
	
	BOOLEAN				is_AppContainer;
	
	BOOLEAN				initialized;

	ULONG_PTR			process_base;
	
	ULONG_PTR			ntdll_base;
	
	ULONG_PTR			kernel32_base;
	
	ULONG_PTR			trampoline_address;
	
	ULONG_PTR			win32start_address;
	
	ULONG_PTR			fn_LoadLibraryW;
	
	ULONG_PTR			fn_Memcpy;
	
	ULONG_PTR			fn_VirtualProtect;
	
	ULONG_PTR			fn_CorExeMain;
	
	BOOLEAN				hooked;
	
	BOOLEAN				isbypassapp;
	
	ULONG				session_id;
	
	ULONG				platform;
	
	rb_node				process_cache_node;
	
	ULONG				thread_count;

}NXRM_PROCESS,*PNXRM_PROCESS;

typedef struct _NXRM_PROCESS_NOTIFY
{
	LIST_ENTRY						link;
	
	PCREATE_PROCESS_NOTIFY_ROUTINE	notify_fn;

	EX_RUNDOWN_REF					RundownRef;

}NXRM_PROCESS_NOTIFY,*PNXRM_PROCESS_NOTIFY;


typedef struct _NXRM_GLOBAL_DATA
{

	DECLSPEC_CACHEALIGN rb_root					rb_process_cache;

	DECLSPEC_CACHEALIGN KSPIN_LOCK				process_cache_lock;

	DECLSPEC_CACHEALIGN LONG					process_count;

						NPAGED_LOOKASIDE_LIST	process_cache_pool;

						NPAGED_LOOKASIDE_LIST	instance_cache_pool;

						NPAGED_LOOKASIDE_LIST	apc_pool;

	DECLSPEC_CACHEALIGN	PVOID					notify_event;

						HANDLE					notify_event_kernel_handle;

	DECLSPEC_CACHEALIGN NXRM_PROCESS_ENTRY		last_entry;

	DECLSPEC_CACHEALIGN	ULONG					last_entry_count;

	DECLSPEC_CACHEALIGN	ULONG					total_query_count;

	DECLSPEC_CACHEALIGN	HANDLE					SystemPID;

						NXRM_PROCESS_ENTRY		System_entry;

						PDEVICE_OBJECT			devobj;
	
						ULONG					os_major;

						ULONG					os_minor;

						ULONG					os_build;

						PVOID					ksection;
	
	DECLSPEC_CACHEALIGN	PVOID					pKernelBase;

	DECLSPEC_CACHEALIGN ZWPROTECTVIRTUALMEMORY	fn_ZwProtectVirtualMemory;

	DECLSPEC_CACHEALIGN	ZWFLUSHINSTRUCTIONCACHE	fn_ZwFlushInstructionCache;

	DECLSPEC_CACHEALIGN	ZWFREEVIRTUALMEMORY		fn_ZwFreeVirtualMemory;

	DECLSPEC_CACHEALIGN	ZWCREATETHREAD			fn_ZwCreateThread;

	DECLSPEC_CACHEALIGN ZWCREATETHREADEX		fn_ZwCreateThreadEx;
	
	DECLSPEC_CACHEALIGN OBREGISTERCALLBACKS		fn_ObRegisterCallbacks;

						PVOID					pObMgr;

						HANDLE					hProtectedProcess;
						
						UNICODE_STRING			Altitude;

	DECLSPEC_CACHEALIGN LIST_ENTRY				process_notify_list;

	DECLSPEC_CACHEALIGN	ERESOURCE				process_notify_list_rw_lock;

	DECLSPEC_CACHEALIGN	RTLQUERYPACKAGEIDENTITY	fn_RtlQueryPackageIdentity;

}NXRM_GLOBAL_DATA,*PNXRM_GLOBAL_DATA;

#pragma pack(pop)

#pragma pack(push,1)

typedef struct _INITIAL_TEB
{
	PVOID	StackCommit;
	
	PVOID	StackCommitMax;
	
	PVOID	StackBase;
	
	PVOID	StackLimit;
	
	PVOID	StackReserved;

}INITIAL_TEB, *PINITIAL_TEB;

#pragma pack(pop)

typedef struct _SYSTEM_THREAD_INFORMATION {
	
	LARGE_INTEGER   KernelTime;             // time spent in kernel mode
	
	LARGE_INTEGER   UserTime;               // time spent in user mode
	
	LARGE_INTEGER   CreateTime;             // thread creation time
	
	ULONG           WaitTime;               // wait time
	
	PVOID           StartAddress;           // start address
	
	CLIENT_ID       ClientId;               // thread and process IDs
	
	KPRIORITY       Priority;               // dynamic priority
	
	KPRIORITY       BasePriority;           // base priority
	
	ULONG           ContextSwitchCount;     // number of context switches
	
	LONG            State;                  // current state
	
	LONG            WaitReason;             // wait reason

} SYSTEM_THREAD_INFORMATION, *PSYSTEM_THREAD_INFORMATION;

typedef struct _SYSTEM_PROCESS_INFORMATION {
	
	ULONG						NextEntryDelta;         // offset to the next entry
	
	ULONG						ThreadCount;            // number of threads
	
	ULONG						Reserved1[6];           // reserved
	
	LARGE_INTEGER				CreateTime;             // process creation time
	
	LARGE_INTEGER				UserTime;               // time spent in user mode
	
	LARGE_INTEGER				KernelTime;             // time spent in kernel mode
	
	UNICODE_STRING				ProcessName;            // process name
	
	KPRIORITY					BasePriority;           // base process priority
	
	HANDLE						ProcessId;              // process identifier
	
	HANDLE						InheritedFromProcessId; // parent process identifier
	
	ULONG						HandleCount;            // number of handles
	
	ULONG						SessionId;
	
	PVOID						PageDirectoryBase;
	
	VM_COUNTERS					VmCounters;             // virtual memory counters
	
	SIZE_T						PrivatePageCount;
	
	IO_COUNTERS					IoCounters;             // i/o counters
	
	SYSTEM_THREAD_INFORMATION	Threads[1];				// threads

}SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

typedef struct _PEB_LDR_DATA {
	
	UCHAR      Reserved1[8];
	
	PVOID      Reserved2[3];
	
	LIST_ENTRY InMemoryOrderModuleList;

} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _LDR_DATA_TABLE_ENTRY {

	UCHAR			Reserved1[2];

	LIST_ENTRY		InMemoryOrderLinks;

	PVOID			Reserved2[2];

	PVOID			DllBase;

	PVOID			EntryPoint;

	SIZE_T			SizeOfImage;

	UNICODE_STRING	FullDllName;

	UCHAR			Reserved4[8];

	PVOID			Reserved5[3];

	union {

		ULONG CheckSum;

		PVOID Reserved6;
	};

	ULONG TimeDateStamp;

} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

typedef struct _RTL_USER_PROCESS_PARAMETERS {  

	UCHAR			Reserved1[56];  

	UNICODE_STRING	ImagePathName;  

	UNICODE_STRING	CommandLine;  

	UCHAR			Reserved2[92];

} RTL_USER_PROCESS_PARAMETERS,  *PRTL_USER_PROCESS_PARAMETERS;

#ifdef _X86_

typedef struct _PEB 
{  
	UCHAR							Reserved1[2];  

	UCHAR							BeingDebugged;  

	UCHAR							Reserved2[9];  

	PPEB_LDR_DATA					LoaderData; 

	PRTL_USER_PROCESS_PARAMETERS	ProcessParameters;  

	UCHAR							Reserved3[312];  

	ULONG_PTR						PostProcessInitRoutine;  

	UCHAR							Reserved4[132];  

	ULONG							SessionId;

} PEB,  *PPEB;

typedef struct _THREADINFO
{
	ULONG		Size;

	ULONG		Type1;
	
	ULONG		Length1;
	
	PCLIENT_ID	pClientId;
	
	ULONG		Reserved1;
	
	ULONG		Type2;
	
	ULONG		Length2;
	
	PULONG_PTR	ppTeb;				// point to a pointer to TEB
	
	ULONG		Reserved2;

} THREADINFO, *PTHREADINFO;

#else

typedef struct _PEB {

	UCHAR							Reserved1[2];

	UCHAR							BeingDebugged;

	UCHAR							Reserved2[21];

	PPEB_LDR_DATA					LoaderData;

	PRTL_USER_PROCESS_PARAMETERS	ProcessParameters;

	UCHAR							Reserved3[520];

	ULONG_PTR						PostProcessInitRoutine;

	UCHAR							Reserved4[136];

	ULONG							SessionId;

} PEB;

#pragma pack(push,1)

typedef struct _PEB32{

	UCHAR							Reserved1[2];  

	UCHAR							BeingDebugged;  

	UCHAR							Reserved2[9];  

	ULONG							LoaderData; 

	ULONG							ProcessParameters;  

	UCHAR							Reserved3[312];  

	ULONG							PostProcessInitRoutine;  

	UCHAR							Reserved4[132];  

	ULONG							SessionId;

}PEB32,*PPEB32;

typedef struct _PEB_LDR_DATA32 {

	UCHAR			Reserved1[8];

	ULONG			Reserved2[3];

	LIST_ENTRY32	InMemoryOrderModuleList;

} PEB_LDR_DATA32, *PPEB_LDR_DATA32;


typedef struct _LDR_DATA_TABLE_ENTRY32 {

	UCHAR				Reserved1[2];

	LIST_ENTRY32		InMemoryOrderLinks;

	ULONG				Reserved2[2];

	ULONG				DllBase;

	ULONG				EntryPoint;

	ULONG				SizeOfImage;

	UNICODE_STRING32	FullDllName;

	UCHAR				Reserved4[8];

	ULONG				Reserved5[3];

	union {

		ULONG CheckSum;

		ULONG Reserved6;
	};

	ULONG TimeDateStamp;

} LDR_DATA_TABLE_ENTRY32, *PLDR_DATA_TABLE_ENTRY32;

#pragma pack(pop)

#pragma pack(push, 8)

typedef struct _THREADINFO
{
	size_t		Size;

	size_t		Type1;

	size_t		Length1;

	PCLIENT_ID	pClientId;

	size_t		Reserved1;

	size_t		Type2;

	size_t		Length2;

	PULONG_PTR	ppTeb;				// point to a pointer to TEB

	size_t		Reserved2;

} THREADINFO, *PTHREADINFO;

typedef struct _THREADINFO32
{
	size_t		Size;

	size_t		Type1;

	size_t		Length1;

	PCLIENT_ID	pClientId;

	size_t		Reserved1;

} THREADINFO32, *PTHREADINFO32;

#pragma pack(pop)

#endif

typedef enum WINBUILDNUM {
	// Workstations
	WBN_WINNT31 = 528,
	WBN_WINNT35 = 807,
	WBN_WINNT351 = 1057,
	WBN_WINNT40 = 1381,
	WBN_WIN2000 = 2195,
	WBN_WINXP32BIT = 2600,
	WBN_WINXP32BIT_SP2 = 3790,
	WBN_WINXP64BIT = WBN_WINXP32BIT_SP2,
	WBN_WINVISTA = 6000,
	WBN_WINVISTA_SP1 = WBN_WINVISTA + 1,
	WBN_WINVISTA_SP2 = WBN_WINVISTA + 2,
	WBN_WIN7 = 7600,
	WBN_WIN7_SP1 = WBN_WIN7 + 1,
	WBN_WIN8 = 9200,
	WBN_WIN81 = 9600,
	WBN_WIN10 = 10240,
	WBN_WIN10_NOVEMBERUPDATE = 10586,
	WBN_WIN10_ANNIVERSARYUPDATE = 14393,
	WBN_WIN10_CREATORSUPDATE = 15063,
	WBN_WIN10_FALLCREATORSUPDATE = 16299,
	WBN_WIN10_APRIL2018UPDATE = 17134,
	WBN_WIN10_OCTOBER2018UPDATE = 17763,
	WBN_WIN10_MAY2019UPDATE = 18362,
	WBN_WIN10_NOVEMBER2019UPDATE = 18363,

	// Servers
	WBN_WINNT31ADVANCEDSERVER = WBN_WINNT31,
	WBN_WINNT35SERVER = WBN_WINNT35,
	WBN_WINNT351SERVER = WBN_WINNT351,
	WBN_WINNT40SERVER = WBN_WINNT40,
	WBN_WIN2000SERVER = WBN_WIN2000,
	WBN_WINSERVER2003 = WBN_WINXP32BIT_SP2,
	WBN_WINSERVER2008 = WBN_WINVISTA_SP1,
	WBN_WINSERVER2008_SP2 = WBN_WINVISTA_SP2 + 1,
	WBN_WINSERVER2008R2 = WBN_WIN7,
	WBN_WINSERVER2008R2_SP1 = WBN_WIN7_SP1,
	WBN_WINSERVER2012 = WBN_WIN8,
	WBN_WINSERVER2012R2 = WBN_WIN81,
	WBN_WINSERVER2016 = WBN_WIN10_ANNIVERSARYUPDATE,
	WBN_WINSERVER2019 = WBN_WIN10_OCTOBER2018UPDATE
} WINBUILDNUM;