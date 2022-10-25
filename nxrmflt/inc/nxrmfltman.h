#pragma once

#include "nxrmflt.h"
#include <nudf\shared\logdef.h>
#include <nudf\shared\keydef.h>
#include <nudf\shared\rightsdef.h>

#ifdef _X86_
#ifdef NXRMFLTMAN_EXPORTS
#define NXRMFLTMAN_API __declspec(dllexport)
#else
#define NXRMFLTMAN_API __declspec(dllimport)
#endif
#else
#define NXRMFLTMAN_API
#endif

#ifndef NXRMFLT_MAX_PATH
#define NXRMFLT_MAX_PATH	(260)
#endif

typedef PVOID	NXRMFLT_HANDLE;

#ifdef __cplusplus
extern "C" {
#endif

	/************************************************************************/
	/*                                                                      */
	/*	call back functions													*/
	/*                                                                      */
	/************************************************************************/

	typedef ULONG (__stdcall *NXRMFLT_CALLBACK_NOTIFY)(
		ULONG				type,
		PVOID				msg,
		ULONG				Length,
		PVOID				msgctx,
		PVOID				userctx);

	NXRMFLT_HANDLE NXRMFLTMAN_API __stdcall nxrmfltCreateManager(
		NXRMFLT_CALLBACK_NOTIFY		NotifyCallback, 
		LOGAPI_LOG					DebugDumpCallback,
		LOGAPI_ACCEPT				DebugDumpCheckLevelCallback,
		NXRM_KEY_BLOB				*KeyChain,
		ULONG						KeyChainSizeInByte,
		PVOID						UserContext); 

	HRESULT	NXRMFLTMAN_API __stdcall nxrmfltReplyMessage(
		NXRMFLT_HANDLE				hMgr, 
		PVOID						msgctx, 
		NXRMFLT_CHECK_RIGHTS_REPLY	*reply);

	HRESULT NXRMFLTMAN_API __stdcall nxrmfltSetSaveAsForecast(
		NXRMFLT_HANDLE				hMgr,
		ULONG						ProcessId,
		CONST WCHAR					*SrcFileName,
		CONST WCHAR					*SaveAsFileName);

	HRESULT NXRMFLTMAN_API __stdcall nxrmfltSetPolicyChanged(NXRMFLT_HANDLE	hMgr);
	
	HRESULT NXRMFLTMAN_API __stdcall nxrmfltUpdateKeyChain(
		NXRMFLT_HANDLE				hMgr,
		NXRM_KEY_BLOB				*KeyChain,
		ULONG						KeyChainSizeInByte);

	ULONG	NXRMFLTMAN_API __stdcall nxrmfltStartFiltering(NXRMFLT_HANDLE hMgr);

	ULONG	NXRMFLTMAN_API __stdcall nxrmfltStopFiltering(NXRMFLT_HANDLE hMgr);

	ULONG	NXRMFLTMAN_API __stdcall nxrmfltCloseManager(NXRMFLT_HANDLE hMgr);

#ifdef __cplusplus
}
#endif