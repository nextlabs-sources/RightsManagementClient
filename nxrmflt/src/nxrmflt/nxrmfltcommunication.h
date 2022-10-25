#pragma once

NTSTATUS nxrmfltPrepareServerPort(VOID);
NTSTATUS nxrmfltCloseServerPort(VOID);

NTSTATUS
	nxrmfltConnectNotifyCallback (
	_In_ PFLT_PORT ClientPort,
	_In_ PVOID ServerPortCookie,
	_In_reads_bytes_(SizeOfContext) PVOID ConnectionContext,
	_In_ ULONG SizeOfContext,
	_Outptr_result_maybenull_ PVOID *ConnectionCookie
	);

VOID
	nxrmfltDisconnectNotifyCallback(
	_In_opt_ PVOID ConnectionCookie
	);

NTSTATUS
	nxrmfltMessageNotifyCallback (
	_In_ PVOID ConnectionCookie,
	_In_reads_bytes_opt_(InputBufferSize) PVOID InputBuffer,
	_In_ ULONG InputBufferSize,
	_Out_writes_bytes_to_opt_(OutputBufferSize,*ReturnOutputBufferLength) PVOID OutputBuffer,
	_In_ ULONG OutputBufferSize,
	_Out_ PULONG ReturnOutputBufferLength
	);

