#pragma once

extern PVOID nxrmFindKernelModuleBaseAddress(
	IN STRING *ModuleName
	);

extern NTSTATUS nxrmLookupEntryPoint(
	IN PVOID DllBase,
	IN PCHAR Name,
	OUT PVOID *EntryPoint
	);

extern NTSTATUS nxrmLookupWow64EntryPoint(
	IN PVOID DllBase,
	IN PCHAR Name,
	OUT PVOID *EntryPoint
	);

extern PVOID nxrmFindLoadLibraryWAddress(
	IN PVOID Kernel32DllBase
	);

extern PVOID nxrmFindVirtualProtectAddress(
	IN PVOID Kernel32DllBase
	);

extern PVOID nxrmFindMemcpyAddress(
	IN PVOID NtDllBase
	);

extern PVOID nxrmFindRtlExitUserThreadAddress(
	IN PVOID NtDllBase
	);

extern PVOID nxrmFindRtlExitUserProcessAddress(
	IN PVOID NtdllBase
	);

extern PVOID nxrmFindCorExeMainAddress(IN PVOID MscoreeBase);

#ifdef _AMD64_
extern PVOID nxrmFindWow64LoadLibraryWAddress(
	IN PVOID Kernel32DllBase
	);

extern PVOID nxrmFindWow64RtlExitUserThreadAddress(
	IN PVOID Kernel32DllBase
	);

extern PVOID nxrmFindWow64VirtualProtectAddress(
	IN PVOID Kernel32DllBase
	);

extern PVOID nxrmFindWow64MemcpyAddress(
	IN PVOID NtDllBase
	);

extern PVOID nxrmFindWow64CorExeMainAddress(
	IN PVOID MscoreeBase
	);

#endif