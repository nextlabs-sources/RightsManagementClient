/*++

Copyright (c) NextLabs, Inc.

Module Name:

	nxrmdrv.h

Author:

	Fei Qi

Environment:

	User or kernel mode.

Revision History:

	12/31/2014	v1.0	Fei Qi
--*/

#pragma once

#include <nudf\shared\rbtree.h>

//
// Define the various device type values.  Note that values used by Microsoft
// Corporation are in the range 0-0x7FFF(32767), and 0x8000(32768)-0xFFFF(65535)
// are reserved for use by customers.
//

#define FILE_DEVICE_NXRMDRV	0x9898

//
// Macro definition for defining IOCTL and FSCTL function control codes. Note
// that function codes 0-0x7FF(2047) are reserved for Microsoft Corporation,
// and 0x800(2048)-0xFFF(4095) are reserved for customers.
//

#define NXRMDRV_IOCTL_BASE	0x818

//
// The device driver IOCTLs
//

#define CTL_CODE_NXRMDRV(i)		CTL_CODE(FILE_DEVICE_NXRMDRV, NXRMDRV_IOCTL_BASE+i, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_NXRMDRV_MAP_RM_SERVER_SECTION	CTL_CODE_NXRMDRV(8)
#define IOCTL_NXRMDRV_MAP_RM_CLIENT_SECTION	CTL_CODE_NXRMDRV(18)
#define IOCTL_NXRMDRV_QUERY_POOL_INFO		CTL_CODE_NXRMDRV(28)
#define IOCTL_NXRMDRV_QUERY_PROCESS_INFO	CTL_CODE_NXRMDRV(38)
#define IOCTL_NXRMDRV_SET_PROTECTED_PID		CTL_CODE_NXRMDRV(48)
#define IOCTL_NXRMDRV_GET_LOADER_SNAPSHOT	CTL_CODE_NXRMDRV(58)

#define CACHE_LINE	64
#define CACHE_ALIGN	__declspec(align(CACHE_LINE))

#define NXRM_RM_SECTION_DEFAULT_SIZE			512*1024

#define NXRM_SECTION_MAX_DATA_SIZE			3948
#define NXRM_MAX_PATH_LEN					268
#define NXRM_MAX_DWM_WND					256

#define NXRM_SECTION_TYPE_RM				0x88888888

#define NXRM_DRV_VERSION					0x08010064

typedef struct _NXRMSECTION{
	PVOID	section;
	HANDLE	hsem;
	HANDLE	hnotify;
}NXRMSECTION,*PNXRMSECTION;

typedef union _NXRMSECTIONHDR{
	struct{
		CACHE_ALIGN	volatile			LONG	ready;
										ULONG	version;
										ULONG	length;
										ULONG	type;
		CACHE_ALIGN	volatile			LONG	policy_sn;
		CACHE_ALIGN volatile			LONG	channel_mask;
		CACHE_ALIGN	volatile			ULONG	protected_process_id;
		CACHE_ALIGN volatile			ULONG	dwm_active_session_id;
		CACHE_ALIGN volatile			LONG	dwm_wnd_spin_lock;
		CACHE_ALIGN volatile			ULONG	dwm_wnd[NXRM_MAX_DWM_WND];
	}hdr;
	UCHAR	raw_hdr[4096];
}NXRMSECTIONHDR,*PNXRMSECTIONHDR;

typedef struct _NXRMRECORDHDR{
	CACHE_ALIGN volatile	LONG	record_taken;
				volatile	LONG	record_state;
				volatile	LONG	record_cancelled;
							ULONG	record_type;
							ULONG	record_owner;
							ULONG	record_flags;
}NXRMRECORDHDR,*PNXRMRECORDHDR;

typedef struct _NXRMRECORD{
	NXRMRECORDHDR	hdr;
	UCHAR			reserved[32];
	UCHAR			checksum[16];
	ULONG			length;
	UCHAR			data[NXRM_SECTION_MAX_DATA_SIZE];
}NXRMRECORD;

#pragma pack(push,8)

typedef struct _NXRM_POOL_INFO{
	ULONG		pool_status;
	ULONG		pool_size;
	ULONG_PTR	pool_address;
	ULONG		pool_instances;
}NXRM_POOL_INFO,*PNXRM_POOL_INFO;

typedef struct _NXRM_PROCESS_ENTRY
{
	HANDLE		process_id;
	HANDLE		parent_id;
	WCHAR		process_path[NXRM_MAX_PATH_LEN];
	ULONG		is_service;
	ULONG		hooked;
	ULONG		session_id;
	ULONG		platform;
	rb_node		node;
	LIST_ENTRY	link;
}NXRM_PROCESS_ENTRY,*PNXRM_PROCESS_ENTRY;

typedef struct _NXRM_PROCESS_INFO{
	ULONG				NumberOfEntry;
	ULONG				NumberOfQuery;
	ULONG				NumberOfCacheHit;
	NXRM_PROCESS_ENTRY	Entry[1];
}NXRM_PROCESS_INFO,*PNXRM_PROCESS_INFO;

#pragma pack(pop)

#define NXRM_RECORD_TYPE_INVALID			0xcccc

#define NXRM_RECORD_STATE_READY_FOR_ENGINE	0x00001000
#define	NXRM_RECORD_STATE_READY_FOR_CLIENT	0x00002000
#define NXRM_RECORD_STATE_IN_PROCESSING		0x00003000
#define NXRM_RECORD_STATE_FREE				0x00004000
#define NXRM_RECORD_STATE_BAD_RECORD		0x00005000

//
// Name that Win32 front end will use to open the nxrmdrv device
//

#define NXRMDRV_WIN32_DEVICE_NAME_A			"\\\\.\\nxrmdrv"
#define NXRMDRV_WIN32_DEVICE_NAME_W			L"\\\\.\\nxrmdrv"
#define NXRMDRV_DEVICE_NAME_A				"\\Device\\nxrmdrv"
#define NXRMDRV_DEVICE_NAME_W				L"\\Device\\nxrmdrv"
#define NXRMDRV_DOS_DEVICE_NAME_A			"\\DosDevices\\nxrmdrv"
#define NXRMDRV_DOS_DEVICE_NAME_W			L"\\DosDevices\\nxrmdrv"
#define NXRMDRV_PARAMETERS_REG_KEY_A		"\\nxrmdrv\\Parameters"
#define NXRMDRV_PARAMETERS_REG_KEY_W		L"\\nxrmdrv\\Parameters"

#ifdef _UNICODE
#define NXRMDRV_WIN32_DEVICE_NAME			NXRMDRV_WIN32_DEVICE_NAME_W
#define NXRMDRV_DEVICE_NAME					NXRMDRV_DEVICE_NAME_W
#define NXRMDRV_DOS_DEVICE_NAME				NXRMDRV_DOS_DEVICE_NAME_W
#define NXRMDRV_PARAMETERS_REG_KEY			NXRMDRV_PARAMETERS_REG_KEY_W
#else
#define NXRMDRV_WIN32_DEVICE_NAME			NXRMDRV_WIN32_DEVICE_NAME_A
#define NXRMDRV_DEVICE_NAME					NXRMDRV_DEVICE_NAME_A
#define NXRMDRV_DOS_DEVICE_NAME				NXRMDRV_DOS_DEVICE_NAME_A
#define NXRMDRV_PARAMETERS_REG_KEY			NXRMDRV_PARAMETERS_REG_KEY_A
#endif