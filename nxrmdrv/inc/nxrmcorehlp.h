#pragma once

#ifdef __cplusplus
	extern "C"{
#endif

typedef PVOID NXCONTEXT;

#define COREAPI __stdcall

#define NX_MAX_TIMEOUT		(30*1000)

#define NX_CHANNEL_MASK_OVERLAY_BITMAP_READY	(0x00000001)

//
// client side API
//
extern PVOID COREAPI init_transporter_client(void);

extern BOOL COREAPI close_transporter_client(PVOID section);

extern NXCONTEXT COREAPI submit_request(PVOID section, ULONG type, PVOID req, ULONG length);

extern BOOL COREAPI submit_notify(PVOID section, ULONG type, PVOID req, ULONG length);

extern BOOL COREAPI wait_for_response(NXCONTEXT context, PVOID section, PVOID resp, ULONG length, ULONG *bytesret);

extern BOOL COREAPI is_transporter_enabled(PVOID section);

extern BOOL	COREAPI is_xxx_channel_on(PVOID section, const LONG channel);

extern BOOL COREAPI is_protected_process(PVOID section, DWORD pid);

extern LONG COREAPI get_rm_policy_sn(PVOID section);

extern BOOL COREAPI is_good_version(PVOID section);

extern BOOL COREAPI get_overlay_windows(PVOID section, ULONG *hWnds, ULONG *cbSize);

extern void COREAPI set_dwm_active_session(PVOID section, ULONG sessionid);

extern BOOL COREAPI is_overlay_bitmap_ready(PVOID section);
//
// server side API
//
extern PVOID COREAPI init_transporter_server(void);

extern void COREAPI enable_transporter(PVOID section);

extern void COREAPI disable_transporter(PVOID section);

extern BOOL COREAPI close_transporter_server(PVOID section);

extern NXRMRECORD* COREAPI find_pending_request(PVOID section);

extern void COREAPI recycle_cancel_request(PVOID section);

extern BOOL COREAPI post_response(PVOID section, NXRMRECORD *record, PVOID resp, ULONG length);

extern HANDLE COREAPI get_transporter_semaphore(PVOID section);

extern void COREAPI set_channel_mask(PVOID section, LONG channelmask);

extern DWORD COREAPI get_loader_snapshot(PVOID section, ULONG length, UCHAR* buf, ULONG *outlength);

extern HRESULT COREAPI query_process_information(PVOID section, HANDLE PID, ULONG length, UCHAR* buf, ULONG *outlength);

extern HANDLE COREAPI get_notify_event(PVOID section);

extern void COREAPI increase_rm_policy_sn(PVOID section);

extern ULONG COREAPI set_overlay_windows(PVOID section, ULONG *hWnd, ULONG cbSize);

extern ULONG COREAPI get_dwm_active_session(PVOID section);

#ifdef __cplusplus
}
#endif
