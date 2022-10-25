#include "precomp.h"
#include "nxrmdrv.h"
#include "nxrmdrvdef.h"
#include "nxrmdrvhlp.h"
#include "nxrmldr.h"

extern NXRM_GLOBAL_DATA	Global;

static DECLSPEC_CACHEALIGN WCHAR		g_wsz_kernel32[NXRM_MAX_PATH_LEN];
static DECLSPEC_CACHEALIGN WCHAR		g_wsz_ntdll[NXRM_MAX_PATH_LEN];
static DECLSPEC_CACHEALIGN WCHAR		g_wsz_mscoree[NXRM_MAX_PATH_LEN];

#ifdef _AMD64_

static DECLSPEC_CACHEALIGN WCHAR		g_wsz_wow64_kernel32[NXRM_MAX_PATH_LEN];
static DECLSPEC_CACHEALIGN WCHAR		g_wsz_wow64_ntdll[NXRM_MAX_PATH_LEN];
static DECLSPEC_CACHEALIGN WCHAR		g_wsz_wow64_mscoree[NXRM_MAX_PATH_LEN];

#endif

extern LPSTR PsGetProcessImageFileName(PEPROCESS  Process);   
extern HANDLE PsGetProcessInheritedFromUniqueProcessId(PEPROCESS Process);
extern PVOID RtlImageDirectoryEntryToData(PVOID BaseAddress,BOOLEAN MappedAsImage,USHORT Directory,PULONG Size);
extern PIMAGE_NT_HEADERS RtlImageNtHeader(IN PVOID BaseAddress);
extern ULONG PsGetCurrentProcessSessionId(void);
extern ULONG NTAPI PsGetProcessSessionId(PEPROCESS Process);

#ifdef _AMD64_

extern BOOLEAN is_Wow64Process(void);
extern PVOID Wow64RtlImageDirectoryEntryToData(PVOID BaseAddress,BOOLEAN MappedAsImage,USHORT Directory,PULONG Size);

#endif

/************************************************************************/
/*                                                                      */
/*                                                                      */
/*  push        ebp                                                     */  
/*  mov         ebp,esp                                                 */ 
/*  sub         esp,80h                                                 */ 
/*  mov         dword ptr [ebp-80h],3A0043h                             */  
/*  mov         dword ptr [ebp-7Ch],57005Ch                             */  
/*  mov         dword ptr [ebp-78h],6E0069h                             */
/*  mov         dword ptr [ebp-74h],6F0064h                             */
/*  mov         dword ptr [ebp-70h],730077h                             */  
/*  mov         dword ptr [ebp-6Ch],53005Ch                             */  
/*  mov         dword ptr [ebp-68h],730079h                             */  
/*  mov         dword ptr [ebp-64h],650074h                             */  
/*  mov         dword ptr [ebp-60h],33006Dh                             */  
/*  mov         dword ptr [ebp-5Ch],5C0032h                             */  
/*  mov         dword ptr [ebp-58h],78006Eh                             */  
/*  mov         dword ptr [ebp-54h],6D0072h                             */  
/*  mov         dword ptr [ebp-50h],6F0063h                             */  
/*  mov         dword ptr [ebp-4Ch],650072h                             */  
/*  mov         dword ptr [ebp-48h],64002Eh                             */  
/*  mov         dword ptr [ebp-44h],6C006Ch                             */
/*  mov         dword ptr [ebp-40h],0h                                  */  
/*  lea         eax,[ebp-80h]                                           */ 
/*  push        eax                                                     */
/*  mov         eax,LoadLibraryW                                        */
/*  call        eax                                                     */ 
/*  mov         esp,ebp                                                 */ 
/*  pop         ebp                                                     */
/*  push        ebp                                                     */  
/*  mov         ebp,esp                                                 */ 
/*  sub         esp,8                                                   */ 
/*  mov         dword ptr [ebp-8],0                                     */ 
/*  mov         dword ptr [ebp-4],2                                     */ 
/*  mov         dword ptr [ebp-8],win32startaddr                        */ 
/*  lea         eax,[ebp-4]                                             */ 
/*  push        eax                                                     */  
/*  push        40h                                                     */  
/*  push        5h                                                      */ 
/*  mov         ecx,dword ptr [ebp-8]                                   */ 
/*  push        ecx                                                     */
/*  mov         eax,VirtualProtect                                      */
/*  call        eax                                                     */ 
/*  push        5                                                       */  
/*  push        src_addr                                                */  
/*  push        dst_addr                                                */ 
/*  mov         eax,Memcpy                                              */ 
/*  call        eax                                                     */
/*  lea         edx,[ebp-4]                                             */ 
/*  push        edx                                                     */
/*  mov         eax, dword ptr [ebp-4]                                  */
/*  push        eax                                                     */
/*  push        5h                                                      */ 
/*  mov         ecx,dword ptr [ebp-8]                                   */ 
/*  push        ecx                                                     */
/*  mov         eax,VirtualProtect                                      */
/*  call        eax                                                     */ 
/*  mov         esp,ebp                                                 */ 
/*  pop         ebp                                                     */  
/*  jmp         win32_start_address+5                                   */
/*                                                                      */
/*                                                                      */
/************************************************************************/

static CONST UCHAR	g_k_loader_function[] ={\
	0x55,\
	0x8B,0xEC,\
	0x81,0xEC,0x80,0x00,0x00,0x00,\
	0xC7,0x45,0x80,0x43,0x00,0x3A,0x00,\
	0xC7,0x45,0x84,0x5C,0x00,0x57,0x00,\
	0xC7,0x45,0x88,0x69,0x00,0x6E,0x00,\
	0xC7,0x45,0x8C,0x64,0x00,0x6F,0x00,\
	0xC7,0x45,0x90,0x77,0x00,0x73,0x00,\
	0xC7,0x45,0x94,0x5C,0x00,0x53,0x00,\
	0xC7,0x45,0x98,0x79,0x00,0x73,0x00,\
	0xC7,0x45,0x9C,0x74,0x00,0x65,0x00,\
	0xC7,0x45,0xA0,0x6D,0x00,0x33,0x00,\
	0xC7,0x45,0xA4,0x32,0x00,0x5C,0x00,\
	0xC7,0x45,0xA8,0x6E,0x00,0x78,0x00,\
	0xC7,0x45,0xAC,0x72,0x00,0x6D,0x00,\
	0xC7,0x45,0xB0,0x63,0x00,0x6F,0x00,\
	0xC7,0x45,0xB4,0x72,0x00,0x65,0x00,\
	0xC7,0x45,0xB8,0x2E,0x00,0x64,0x00,\
	0xC7,0x45,0xBC,0x6C,0x00,0x6C,0x00,\
	0xC7,0x45,0xC0,0x00,0x00,0x00,0x00,\
	0x8D,0x45,0x80,\
	0x50,\
	0xB8,0x00,0x00,0x00,0x00,\
	0xFF,0xD0,\
	0x8B,0xE5,\
	0x5D,\
	0x55,\
	0x8B,0xEC,\
	0x83,0xEC,0x08,\
	0xC7,0x45,0xF8,0x00,0x00,0x00,0x00,\
	0xC7,0x45,0xFC,0x02,0x00,0x00,0x00,\
	0xC7,0x45,0xF8,0x00,0x00,0x00,0x00,\
	0x8D,0x45,0xFC,\
	0x50,\
	0x6A,0x40,\
	0x68,0x05,0x00,0x00,0x00,\
	0x8B,0x4D,0xF8,\
	0x51,\
	0xB8,0x00,0x00,0x00,0x00,\
	0xFF,0xD0,\
	0x68,0x05,0x00,0x00,0x00,\
	0x68,0x00,0x00,0x00,0x00,\
	0x68,0x00,0x00,0x00,0x00,\
	0xB8,0x00,0x00,0x00,0x00,\
	0xFF,0xD0,\
	0x8D,0x55,0xFC,\
	0x52,\
	0x8B,0x45,0xFC,\
	0x50,\
	0x68,0x05,0x00,0x00,0x00,\
	0x8B,0x4D,0xF8,\
	0x51,\
	0xB8,0x00,0x00,0x00,0x00,\
	0xFF,0xD0,\
	0x8B,0xE5,\
	0x5D,\
	0xE9,0x00,0x00,0x00,0x00\
};

#ifdef _AMD64_

/************************************************************************/
/*                                                                      */
/*                                                                      */
/*  sub         rsp,0A8h                                                */
/*  mov         eax,43h                                                 */
/*  mov         word ptr [rsp+20h],ax                                   */
/*  mov         eax,3Ah                                                 */
/*  mov         word ptr [rsp+22h],ax                                   */
/*  mov         eax,5Ch                                                 */
/*  mov         word ptr [rsp+24h],ax                                   */
/*  mov         eax,57h                                                 */
/*  mov         word ptr [rsp+26h],ax                                   */
/*  mov         eax,69h                                                 */
/*  mov         word ptr [rsp+28h],ax                                   */
/*  mov         eax,6Eh                                                 */
/*  mov         word ptr [rsp+2Ah],ax                                   */
/*  mov         eax,64h                                                 */
/*  mov         word ptr [rsp+2Ch],ax                                   */
/*  mov         eax,6Fh                                                 */
/*  mov         word ptr [rsp+2Eh],ax                                   */
/*  mov         eax,77h                                                 */
/*  mov         word ptr [rsp+30h],ax                                   */
/*  mov         eax,73h                                                 */
/*  mov         word ptr [rsp+32h],ax                                   */
/*  mov         eax,5Ch                                                 */
/*  mov         word ptr [rsp+34h],ax                                   */
/*  mov         eax,53h                                                 */
/*  mov         word ptr [rsp+36h],ax                                   */
/*  mov         eax,79h                                                 */
/*  mov         word ptr [rsp+38h],ax                                   */
/*  mov         eax,73h                                                 */
/*  mov         word ptr [rsp+3Ah],ax                                   */
/*  mov         eax,74h                                                 */
/*  mov         word ptr [rsp+3Ch],ax                                   */
/*  mov         eax,65h                                                 */
/*  mov         word ptr [rsp+3Eh],ax                                   */
/*  mov         eax,6Dh                                                 */
/*  mov         word ptr [rsp+40h],ax                                   */
/*  mov         eax,33h                                                 */
/*  mov         word ptr [rsp+42h],ax                                   */
/*  mov         eax,32h                                                 */
/*  mov         word ptr [rsp+44h],ax                                   */
/*  mov         eax,5Ch                                                 */
/*  mov         word ptr [rsp+46h],ax                                   */
/*  mov         eax,6Eh                                                 */
/*  mov         word ptr [rsp+48h],ax                                   */
/*  mov         eax,78h                                                 */
/*  mov         word ptr [rsp+4Ah],ax                                   */
/*  mov         eax,72h                                                 */
/*  mov         word ptr [rsp+4Ch],ax                                   */
/*  mov         eax,6Dh                                                 */
/*  mov         word ptr [rsp+4Eh],ax                                   */
/*  mov         eax,63h                                                 */
/*  mov         word ptr [rsp+50h],ax                                   */
/*  mov         eax,6Fh                                                 */
/*  mov         word ptr [rsp+52h],ax                                   */
/*  mov         eax,72h                                                 */
/*  mov         word ptr [rsp+54h],ax                                   */
/*  mov         eax,65h                                                 */
/*  mov         word ptr [rsp+56h],ax                                   */
/*  mov         eax,36h                                                 */
/*  mov         word ptr [rsp+58h],ax                                   */
/*  mov         eax,34h                                                 */
/*  mov         word ptr [rsp+5Ah],ax                                   */
/*  mov         eax,2Eh                                                 */
/*  mov         word ptr [rsp+5Ch],ax                                   */
/*  mov         eax,64h                                                 */
/*  mov         word ptr [rsp+5Eh],ax                                   */
/*  mov         eax,6Ch                                                 */
/*  mov         word ptr [rsp+60h],ax                                   */
/*  mov         eax,6Ch                                                 */
/*  mov         word ptr [rsp+62h],ax                                   */
/*  xor         eax,eax                                                 */
/*  mov         word ptr [rsp+64h],ax                                   */
/*  mov         rax, LoadLibraryW                                       */ 
/*  lea         rcx, qword ptr [rsp+20h]                                */
/*  call        rax                                                     */
/*  mov         r11, VirtualProtect                                     */
/*  mov         rcx, win32startaddr                                     */
/*	mov			edx, 12													*/
/*  mov         r8d, 40h                                                */
/*  lea         r9, qword ptr original_page_rights                      */
/*  call        r11                                                     */  
/*  mov         r11, RtlCopyMemory                                      */
/*  mov         rcx, dst_addr                                           */ 
/*  mov         rdx, src_addr                                           */	
/*  mov         r8d, 12                                                 */
/*  call        r11                                                     */
/*  mov         r11, VirtualProtect                                     */
/*  mov         r8d, dword PTR original_page_rights                     */
/*  mov         rcx, win32startaddr                                     */
/*  lea         r9, qword PTR original_page_rights                      */
/*  mov         edx, 12                                                 */
/*  call        r11                                                     */
/*  add         rsp, 72                                                 */
/*  mov         rax, offset win32startaddr                              */
/*  jmp         rax                                                     */
/*                                                                      */
/*                                                                      */
/************************************************************************/


static CONST UCHAR	g_k_loader_function64[] = {\
	0x48,0x81,0xEC,0xA8,0x00,0x00,0x00,\
	0xB8,0x43,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x20,\
	0xB8,0x3A,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x22,\
	0xB8,0x5C,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x24,\
	0xB8,0x57,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x26,\
	0xB8,0x69,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x28,\
	0xB8,0x6E,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x2A,\
	0xB8,0x64,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x2C,\
	0xB8,0x6F,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x2E,\
	0xB8,0x77,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x30,\
	0xB8,0x73,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x32,\
	0xB8,0x5C,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x34,\
	0xB8,0x53,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x36,\
	0xB8,0x79,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x38,\
	0xB8,0x73,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x3A,\
	0xB8,0x74,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x3C,\
	0xB8,0x65,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x3E,\
	0xB8,0x6D,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x40,\
	0xB8,0x33,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x42,\
	0xB8,0x32,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x44,\
	0xB8,0x5C,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x46,\
	0xB8,0x6E,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x48,\
	0xB8,0x78,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x4A,\
	0xB8,0x72,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x4C,\
	0xB8,0x6D,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x4E,\
	0xB8,0x63,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x50,\
	0xB8,0x6F,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x52,\
	0xB8,0x72,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x54,\
	0xB8,0x65,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x56,\
	0xB8,0x36,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x58,\
	0xB8,0x34,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x5A,\
	0xB8,0x2E,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x5C,\
	0xB8,0x64,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x5E,\
	0xB8,0x6C,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x60,\
	0xB8,0x6C,0x00,0x00,0x00,\
	0x66,0x89,0x44,0x24,0x62,\
	0x33,0xC0,\
	0x66,0x89,0x44,0x24,0x64,\
	0x48,0xb8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
	0x48,0x8D,0x4C,0x24,0x20,\
	0xff,0xd0,\
	0x49,0xbb,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
	0x48,0xb9,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
	0xba,0x0c,0x00,0x00,0x00,\
	0x41,0xb8,0x40,0x00,0x00,0x00,\
	0x4c,0x8d,0x4c,0x24,0x50,\
	0x41,0xff,0xd3,\
	0x49,0xbb,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
	0x48,0xb9,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
	0x48,0xba,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
	0x41,0xb8,0x0c,0x00,0x00,0x00,\
	0x41,0xff,0xd3,\
	0x49,0xbb,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
	0x44,0x8b,0x44,0x24,0x50,\
	0x48,0xb9,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
	0x4c,0x8d,0x4c,0x24,0x50,\
	0xba,0x0c,0x00,0x00,0x00,\
	0x41,0xff,0xd3,\
	0x48,0x81,0xc4,0xA8,0x00,0x00,0x00,\
	0x48,0xb8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
	0xff,0xe0\
};

BOOLEAN Install_nxrmcore64(
	IN NXRM_PROCESS *process
	);

#endif // _AMD64_

BOOLEAN Install_nxrmcore(
	IN NXRM_PROCESS *process
	);

NXRM_PROCESS* find_process_in_cache(rb_root *proc_cache, HANDLE process_id);

BOOLEAN	add_process_into_cache(rb_root *proc_cache, NXRM_PROCESS *proc);

BOOLEAN is_win32process(IMAGE_FILE_HEADER *pFileHdr);

BOOLEAN is_kernel32(PUNICODE_STRING FullImageName);

BOOLEAN is_ntdll(PUNICODE_STRING FullImageName);

BOOLEAN is_mscoree(PUNICODE_STRING FullImageName);

#ifdef _AMD64_

BOOLEAN is_wow64_ntdll(PUNICODE_STRING FullImageName);

BOOLEAN is_wow64_kernel32(PUNICODE_STRING FullImageName);

BOOLEAN is_wow64_mscoree(PUNICODE_STRING FullImageName);

#endif

BOOLEAN is_process_a_service(PEPROCESS Process);

void hash2wstr(UCHAR *hash, WCHAR *wstr);

BOOLEAN is_trustapp(void);

BOOLEAN is_xxx_dll(PUNICODE_STRING FullImageName, WCHAR *DllName);

void kernel_apc_proc(
	KAPC				*Apc,
	PKNORMAL_ROUTINE	*norm_routine,
	PVOID				*context,
	PVOID				*SysArg1,
	PVOID				*SysArg2
	);

void normal_apc_proc(
	PVOID				*context,
	PVOID				*SysArg1,
	PVOID				*SysArg2
	);

void KeInitializeApc(
	KAPC				*Apc, 
	PKTHREAD			thread,
	unsigned char		state_index,
	PKKERNEL_ROUTINE	ker_routine,
	PKRUNDOWN_ROUTINE	rd_routine,
	PKNORMAL_ROUTINE	nor_routine,
	unsigned char		mode,
	void				*context
	);

void KeInsertQueueApc(
	KAPC			*APC,
	void			*SysArg1,
	void			*SysArg2,
	unsigned char	arg4
	);

VOID NxrmdrvLdrCreateProcessNotifyEx(
	_Inout_   PEPROCESS					Process,
	_In_      HANDLE					ProcessId,
	_In_opt_  PPS_CREATE_NOTIFY_INFO	CreateInfo
	);

VOID
	NxrmdrvLdrLoadImageNotify(
	IN PUNICODE_STRING  FullImageName,
	IN HANDLE			ProcessId,
	IN PIMAGE_INFO		ImageInfo
	);

NTSTATUS NxrmLdrInitialize(void)
{
	NTSTATUS status = STATUS_SUCCESS;

	ULONG length = 0;

	do 
	{
		status = PsSetCreateProcessNotifyRoutineEx(NxrmdrvLdrCreateProcessNotifyEx,FALSE);

		if(!NT_SUCCESS(status))
		{
			dprintf("[nxrmdrv] fail to call PsSetCreateProcessNotifyRoutine()\n");
			break;
		}

		status = PsSetLoadImageNotifyRoutine(NxrmdrvLdrLoadImageNotify);

		if(!NT_SUCCESS(status))
		{
			dprintf("[nxrmdrv] fail to call PsSetLoadImageNotifyRoutine()\n");
			break;
		}

		memset(g_wsz_kernel32,0,sizeof(g_wsz_kernel32));

		RtlStringCbCatW(g_wsz_kernel32,sizeof(g_wsz_kernel32),(WCHAR*)(0x7ffe0034));
		RtlStringCbCatW(g_wsz_kernel32,sizeof(g_wsz_kernel32),L"\\System32\\kernel32.dll");

		memset(g_wsz_ntdll,0,sizeof(g_wsz_ntdll));

		RtlStringCbCatW(g_wsz_ntdll,sizeof(g_wsz_ntdll),L"\\System32\\ntdll.dll");

		memset(g_wsz_mscoree,0,sizeof(g_wsz_mscoree));

		RtlStringCbCatW(g_wsz_mscoree,sizeof(g_wsz_mscoree),(WCHAR*)(0x7ffe0034));
		RtlStringCbCatW(g_wsz_mscoree,sizeof(g_wsz_mscoree),L"\\System32\\mscoree.dll");

#ifdef _AMD64_

		memset(g_wsz_wow64_kernel32,0,sizeof(g_wsz_wow64_kernel32));

		RtlStringCbCatW(g_wsz_wow64_kernel32,sizeof(g_wsz_wow64_kernel32),L"\\SysWOW64\\kernel32.dll");

		memset(g_wsz_wow64_ntdll,0,sizeof(g_wsz_wow64_ntdll));

		RtlStringCbCatW(g_wsz_wow64_ntdll,sizeof(g_wsz_wow64_ntdll),L"\\SysWOW64\\ntdll.dll");

		memset(g_wsz_wow64_mscoree,0,sizeof(g_wsz_wow64_mscoree));

		RtlStringCbCatW(g_wsz_wow64_mscoree,sizeof(g_wsz_wow64_mscoree),L"\\SysWOW64\\mscoree.dll");
#endif

	} while(FALSE);

	return status;
}

NTSTATUS NxrmLdrCleanup(void)
{
	NTSTATUS status = STATUS_SUCCESS;

	rb_node		*node = NULL;
	NXRM_PROCESS	*process = NULL;

	do 
	{

		PsRemoveLoadImageNotifyRoutine(NxrmdrvLdrLoadImageNotify);

		PsSetCreateProcessNotifyRoutineEx(NxrmdrvLdrCreateProcessNotifyEx,TRUE);

		RB_EACH_NODE(node, &Global.rb_process_cache)
		{
			process = rb_entry(node, NXRM_PROCESS, process_cache_node);

			InterlockedDecrement(&Global.process_count);

			rb_erase(node, &Global.rb_process_cache);

			ExFreeToNPagedLookasideList(&Global.process_cache_pool, process);

			process = NULL;
		}

	} while(FALSE);

	return status;
}

VOID NxrmdrvLdrCreateProcessNotifyEx(
	_Inout_   PEPROCESS					Process,
	_In_      HANDLE					ProcessId,
	_In_opt_  PPS_CREATE_NOTIFY_INFO	CreateInfo
	)
{
	NTSTATUS status = STATUS_SUCCESS;

	NXRM_PROCESS	*process = NULL;

	BOOLEAN bRet = TRUE;

	KLOCK_QUEUE_HANDLE lockhandle = {0};

	if(!CreateInfo)
	{
		KeAcquireInStackQueuedSpinLock(&Global.process_cache_lock, &lockhandle);

		process = find_process_in_cache(&Global.rb_process_cache, ProcessId);

		if(process)
		{
			rb_erase(&process->process_cache_node, &Global.rb_process_cache);

			InterlockedDecrement(&Global.process_count);
		}

		if(ProcessId == Global.last_entry.process_id)
		{
			memset(&Global.last_entry, 0, sizeof(Global.last_entry));
		}

		KeReleaseInStackQueuedSpinLock(&lockhandle);

		if(process)
		{
			ExFreeToNPagedLookasideList(&Global.process_cache_pool, (PVOID)process);

			process = NULL;
		}

		if(Global.notify_event)
		{
			KeSetEvent((PRKEVENT)Global.notify_event, IO_NO_INCREMENT,FALSE);
		}
	}
	else
	{
		do 
		{
			process = (NXRM_PROCESS*)ExAllocateFromNPagedLookasideList(&Global.process_cache_pool);

			if(process)
			{
				process->process_id				= ProcessId;
				process->parent_id				= CreateInfo->ParentProcessId;
				process->hooked					= FALSE;
				process->kernel32_base			= 0;
				process->ntdll_base				= 0;
				process->fn_LoadLibraryW		= 0;
				process->fn_Memcpy				= 0;
				process->fn_VirtualProtect		= 0;
				process->fn_CorExeMain			= 0;
				process->process_base			= 0;
				process->trampoline_address		= 0;
				process->win32start_address		= 0;
				process->is_Service				= FALSE;
				process->is_CLR					= FALSE;
				process->is_AppContainer		= FALSE;
				process->isbypassapp			= FALSE;
				process->initialized			= FALSE;
				process->session_id				= PsGetProcessSessionId(Process);
				process->platform				= IMAGE_FILE_MACHINE_UNKNOWN;
				process->thread_count			= 0;

				memset(process->process_path,0,sizeof(process->process_path));

				memcpy(process->process_path, 
					   CreateInfo->ImageFileName->Buffer,
					   min(sizeof(process->process_path) - sizeof(WCHAR), CreateInfo->ImageFileName->Length));

				KeAcquireInStackQueuedSpinLock(&Global.process_cache_lock, &lockhandle);

				bRet = add_process_into_cache(&Global.rb_process_cache, process);

				if(bRet)
					InterlockedIncrement(&Global.process_count);

				if(ProcessId == Global.last_entry.process_id)
				{
					memset(&Global.last_entry, 0, sizeof(Global.last_entry));
				}

				KeReleaseInStackQueuedSpinLock(&lockhandle);

				if(bRet == FALSE)
				{
					ExFreeToNPagedLookasideList(&Global.process_cache_pool, (PVOID)process);
					process = NULL;
				}
			}

		} while(FALSE);
	}

	return;
}

VOID
NxrmdrvLdrLoadImageNotify(
	IN PUNICODE_STRING  FullImageName,
	IN HANDLE			ProcessId,
	IN PIMAGE_INFO		ImageInfo
	)
{
	NXRM_PROCESS *process = NULL;

	IMAGE_NT_HEADERS	*pNtHdr = NULL;

	KLOCK_QUEUE_HANDLE lockhandle = {0};

	__try
	{
		do 
		{
			pNtHdr = RtlImageNtHeader(ImageInfo->ImageBase);

			if(ProcessId == 0 || pNtHdr == NULL)
			{
				break;
			}

			KeAcquireInStackQueuedSpinLock(&Global.process_cache_lock, &lockhandle);

			process = find_process_in_cache(&Global.rb_process_cache, ProcessId);

			KeReleaseInStackQueuedSpinLock(&lockhandle);

			if(!process)
			{
				break;
			}

			if (is_trustapp())
			{
				process->isbypassapp = TRUE;
			}

			if (process->hooked || process->is_Service || process->isbypassapp)
			{
				break;
			}

			if(is_win32process(&pNtHdr->FileHeader))
			{
#ifdef _AMD64_
				IMAGE_OPTIONAL_HEADER32	*pOptionalHdr32 = NULL;
				IMAGE_TLS_DIRECTORY32	*pTlsDir32 = NULL;

				BOOLEAN	bIsWow64Process = FALSE;
#endif
				IMAGE_OPTIONAL_HEADER	*pOptionalHdr = NULL;
				IMAGE_TLS_DIRECTORY		*pTlsDir = NULL;
				IMAGE_COR20_HEADER		*pCLRHdr = NULL;

				ULONG_PTR	start_address	= 0;

				PEPROCESS p = NULL;

				ULONG size = 0;

#ifdef _AMD64_
				bIsWow64Process = is_Wow64Process();

				if(bIsWow64Process)
				{
					pOptionalHdr32 = (IMAGE_OPTIONAL_HEADER32 *)&pNtHdr->OptionalHeader;

					pTlsDir32 = Wow64RtlImageDirectoryEntryToData(ImageInfo->ImageBase,TRUE,IMAGE_DIRECTORY_ENTRY_TLS,&size);

					if(pTlsDir32)
					{
						if(pTlsDir32->AddressOfCallBacks)
						{
							if(*(ULONG*)pTlsDir32->AddressOfCallBacks)
							{
								start_address =(ULONG_PTR)(*(ULONG*)pTlsDir32->AddressOfCallBacks);
							}
							else
							{
								start_address = (ULONG_PTR)(pOptionalHdr32->AddressOfEntryPoint + (ULONG_PTR)ImageInfo->ImageBase);
							}
						}
						else
						{
							start_address = (ULONG_PTR)(pOptionalHdr32->AddressOfEntryPoint + (ULONG_PTR)ImageInfo->ImageBase);
						}
					}
					else
					{
						start_address = (ULONG_PTR)(pOptionalHdr32->AddressOfEntryPoint + (ULONG_PTR)ImageInfo->ImageBase);
					}

					size = 0;

					pCLRHdr = Wow64RtlImageDirectoryEntryToData((PVOID)ImageInfo->ImageBase,TRUE,IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR,&size);

					if(pCLRHdr)
					{
						process->is_CLR = TRUE;
					}
				}
				else
#endif
				{
					pOptionalHdr = &pNtHdr->OptionalHeader;

					pTlsDir = RtlImageDirectoryEntryToData(ImageInfo->ImageBase,TRUE,IMAGE_DIRECTORY_ENTRY_TLS,&size);

					if(pTlsDir)
					{
						if(pTlsDir->AddressOfCallBacks)
						{
							if(*(ULONG_PTR*)pTlsDir->AddressOfCallBacks)
							{
								start_address = *(ULONG_PTR*)pTlsDir->AddressOfCallBacks;
							}
							else
							{
								start_address = (ULONG_PTR)(pOptionalHdr->AddressOfEntryPoint + (ULONG_PTR)ImageInfo->ImageBase);
							}
						}
						else
						{
							start_address = (ULONG_PTR)(pOptionalHdr->AddressOfEntryPoint + (ULONG_PTR)ImageInfo->ImageBase);
						}
					}
					else
					{
						start_address = (ULONG_PTR)(pOptionalHdr->AddressOfEntryPoint + (ULONG_PTR)ImageInfo->ImageBase);
					}

					size = 0;

					pCLRHdr = RtlImageDirectoryEntryToData((PVOID)ImageInfo->ImageBase,TRUE,IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR,&size);

					if(pCLRHdr)
					{
						process->is_CLR = TRUE;
					}
				}

				if(STATUS_SUCCESS == PsLookupProcessByProcessId(process->process_id,&p))
				{
					if(is_process_a_service(p))
					{
						process->is_Service = TRUE;
					}

					ObDereferenceObject(p);
					p = NULL;
				}

				process->process_base		= (ULONG_PTR)ImageInfo->ImageBase;
				process->win32start_address	= start_address;

#ifdef _AMD64_
				process->platform = bIsWow64Process?IMAGE_FILE_MACHINE_I386:IMAGE_FILE_MACHINE_AMD64;
#else
				process->platform = IMAGE_FILE_MACHINE_I386;
#endif

				if (process->process_path[0] == L'\0')
				{
					memcpy(process->process_path,
						   FullImageName->Buffer,
						   min(FullImageName->MaximumLength,sizeof(process->process_path)));
				}

				_wcslwr(process->process_path);

				process->session_id = PsGetCurrentProcessSessionId();

				process->initialized = TRUE;	// make this process record available for querying
			}

#ifdef _AMD64_
			else
			{
				if(process->platform == IMAGE_FILE_MACHINE_AMD64)
				{
					if(is_ntdll(FullImageName))
					{
						process->ntdll_base = (ULONG_PTR)ImageInfo->ImageBase;
						process->fn_Memcpy	= (ULONG_PTR)nxrmFindMemcpyAddress(ImageInfo->ImageBase);
					}
					else if(is_mscoree(FullImageName))
					{
						if(process->is_CLR)
						{
							process->win32start_address = (ULONG_PTR)nxrmFindCorExeMainAddress(ImageInfo->ImageBase);
							process->fn_CorExeMain		= process->win32start_address;
						}
					}
					else if(is_kernel32(FullImageName))
					{
						KAPC		*apc = NULL;

						process->kernel32_base		= (ULONG_PTR)ImageInfo->ImageBase;
						process->fn_LoadLibraryW	= (ULONG_PTR)nxrmFindLoadLibraryWAddress(ImageInfo->ImageBase);
						process->fn_VirtualProtect	= (ULONG_PTR)nxrmFindVirtualProtectAddress(ImageInfo->ImageBase);

						if(process->fn_LoadLibraryW &&
						   process->fn_VirtualProtect &&
						   process->fn_Memcpy)
						{

							apc = (KAPC*)ExAllocateFromNPagedLookasideList(&Global.apc_pool);

							if(!apc)
							{
								dprintf("[nxrmdrv] OS is running out of memory! [Checkpoint 2]\n");
								process->hooked = FALSE;
								break;
							}

							KeInitializeApc(apc,
											PsGetCurrentThread(),
											0,
											kernel_apc_proc,
											NULL,
											normal_apc_proc,
											KernelMode,
											NULL);

							KeInsertQueueApc(apc,
											 (PVOID)process,
											 NULL,
											 0);

							process->hooked	 = TRUE;

							if(Global.notify_event)
							{
								KeSetEvent(Global.notify_event, IO_NO_INCREMENT,FALSE);
							}
						}
						else
						{
							dprintf("[nxrmdrv] Fail to deal with %s\n",process->process_path);
						}
					}
					else
					{

					}
				}
				else if(process->platform == IMAGE_FILE_MACHINE_I386)
				{
					if(is_wow64_ntdll(FullImageName))
					{
						KAPC		*apc = NULL;

						process->ntdll_base = (ULONG_PTR)ImageInfo->ImageBase;
						process->fn_Memcpy	= (ULONG_PTR)nxrmFindWow64MemcpyAddress(ImageInfo->ImageBase);


						if(process->fn_LoadLibraryW &&
						   process->fn_VirtualProtect &&
						   process->fn_Memcpy &&
						   (process->is_CLR?process->fn_CorExeMain:TRUE))
						{

							apc = (KAPC*)ExAllocateFromNPagedLookasideList(&Global.apc_pool);

							if(!apc)
							{
								dprintf("[nxrmdrv] OS is running out of memory! [Checkpoint 3]\n");
								process->hooked = FALSE;
								break;
							}

							KeInitializeApc(apc,
											PsGetCurrentThread(),
											0,
											kernel_apc_proc,
											NULL,
											normal_apc_proc,
											KernelMode,
											NULL);

							KeInsertQueueApc(apc,
											 (PVOID)process,
											 NULL,
											 0);

							process->hooked	 = TRUE;

							if(Global.notify_event)
							{
								KeSetEvent(Global.notify_event, IO_NO_INCREMENT,FALSE);
							}
						}
					}
					else if(is_wow64_mscoree(FullImageName))
					{
						if(process->is_CLR)
						{
							process->win32start_address = (ULONG_PTR)nxrmFindWow64CorExeMainAddress(ImageInfo->ImageBase);
							process->fn_CorExeMain		= process->win32start_address;
						}
					}
					else if(is_wow64_kernel32(FullImageName))
					{
						KAPC		*apc = NULL;

						process->kernel32_base		= (ULONG_PTR)ImageInfo->ImageBase;
						process->fn_LoadLibraryW	= (ULONG_PTR)nxrmFindWow64LoadLibraryWAddress(ImageInfo->ImageBase);
						process->fn_VirtualProtect	= (ULONG_PTR)nxrmFindWow64VirtualProtectAddress(ImageInfo->ImageBase);

						if(process->fn_LoadLibraryW &&
						   process->fn_VirtualProtect &&
						   process->fn_Memcpy &&
						   (process->is_CLR?process->fn_CorExeMain:TRUE))
						{

							apc = (KAPC*)ExAllocateFromNPagedLookasideList(&Global.apc_pool);

							if(!apc)
							{
								dprintf("[nxrmdrv] OS is running out of memory! [Checkpoint 4]\n");
								process->hooked = FALSE;
								break;
							}

							KeInitializeApc(apc,
										    PsGetCurrentThread(),
											0,
											kernel_apc_proc,
											NULL,
											normal_apc_proc,
											KernelMode,
											NULL);

							KeInsertQueueApc(apc,
											 (PVOID)process,
											 NULL,
											 0);

							process->hooked	 = TRUE;

							if(Global.notify_event)
							{
								KeSetEvent(Global.notify_event, IO_NO_INCREMENT,FALSE);
							}
						}
					}
					else
					{
						if(process->fn_LoadLibraryW &&
						   process->fn_VirtualProtect &&
						   process->fn_Memcpy &&
						   (process->is_CLR?process->fn_CorExeMain:TRUE))
						{

							KAPC		*apc = NULL;

							apc = (KAPC*)ExAllocateFromNPagedLookasideList(&Global.apc_pool);

							if(!apc)
							{
								dprintf("[nxrmdrv] OS is running out of memory! [Checkpoint 5]\n");
								process->hooked = FALSE;
								break;
							}

							KeInitializeApc(apc,
										    PsGetCurrentThread(),
											0,
											kernel_apc_proc,
											NULL,
											normal_apc_proc,
											KernelMode,
											NULL);

							KeInsertQueueApc(apc,
											 (PVOID)process,
											 NULL,
											 0);

							process->hooked	 = TRUE;

							if(Global.notify_event)
							{
								KeSetEvent((PRKEVENT)Global.notify_event, IO_NO_INCREMENT,FALSE);
							}
						}
					}
				}
				else
				{
					dprintf("[nxrmdrv] Unknown Image type %x!!!!!!\n",process->platform);
				}
			}	
#else
			else if(is_ntdll(FullImageName))
			{
				process->ntdll_base = (ULONG_PTR)ImageInfo->ImageBase;
				process->fn_Memcpy	= (ULONG_PTR)nxrmFindMemcpyAddress(ImageInfo->ImageBase);

			}
			else if(is_mscoree(FullImageName))
			{
				if(process->is_CLR)
				{
					process->win32start_address = (ULONG_PTR)nxrmFindCorExeMainAddress(ImageInfo->ImageBase);
					process->fn_CorExeMain		= process->win32start_address;
				}
			}
			else if(is_kernel32(FullImageName))
			{
				KAPC		*apc = NULL;

				process->kernel32_base		= (ULONG_PTR)ImageInfo->ImageBase;
				process->fn_LoadLibraryW	= (ULONG_PTR)nxrmFindLoadLibraryWAddress(ImageInfo->ImageBase);
				process->fn_VirtualProtect	= (ULONG_PTR)nxrmFindVirtualProtectAddress(ImageInfo->ImageBase);

				if(process->fn_LoadLibraryW &&
				   process->fn_VirtualProtect &&
				   process->fn_Memcpy)
				{

					apc = (KAPC*)ExAllocateFromNPagedLookasideList(&Global.apc_pool);

					if(!apc)
					{
						dprintf("[nxrmdrv] OS is running out of memory! [Checkpoint 6]\n");
						process->hooked = FALSE;
						break;
					}

					KeInitializeApc(apc,
									PsGetCurrentThread(),
									0,
									kernel_apc_proc,
									NULL,
									normal_apc_proc,
									KernelMode,
									NULL);

					KeInsertQueueApc(apc,
									 (PVOID)process,
									 NULL,
									 0);

					process->hooked	 = TRUE;

					if(Global.notify_event)
					{
						KeSetEvent((PRKEVENT)Global.notify_event, IO_NO_INCREMENT,FALSE);
					}
				}
				else
				{
					dprintf("[nxrmdrv] Fail to deal with %S\n",process->process_path);
				}
			}
			else
			{

			}
#endif

		} while(FALSE);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
			dprintf("[nxrmdrv] Exception at checkpoint 1. Process with PID %d is terminating\n",(ULONG)(ULONG_PTR)ProcessId);
	}

	return;
}

BOOLEAN is_win32process(IMAGE_FILE_HEADER *pFileHdr)
{
	BOOLEAN bRet = TRUE;

	if(pFileHdr->Characteristics & IMAGE_FILE_DLL ||
	   pFileHdr->Characteristics & IMAGE_FILE_SYSTEM)
	{
		bRet = FALSE;
	}

	return bRet;
}


BOOLEAN is_ntdll(PUNICODE_STRING FullImageName)
{
	return is_xxx_dll(FullImageName, g_wsz_ntdll);
}

BOOLEAN is_kernel32(PUNICODE_STRING FullImageName)
{
	return is_xxx_dll(FullImageName, g_wsz_kernel32);
}

BOOLEAN is_mscoree(PUNICODE_STRING FullImageName)
{
	return is_xxx_dll(FullImageName, g_wsz_mscoree);
}

NXRM_PROCESS* find_process_in_cache(rb_root *proc_cache, HANDLE process_id)
{
	NXRM_PROCESS *process = NULL;

	rb_node *node = NULL;

	node = proc_cache->rb_node;

	while (node) 
	{ 
		process = rb_entry(node, NXRM_PROCESS, process_cache_node);

		if (process_id < process->process_id) 
			node = node->rb_left; 
		else if (process_id > process->process_id) 
			node = node->rb_right; 
		else return process; 
	}

	return NULL; 
}

//
// internal function, should always search first before call this insert function
//
BOOLEAN	add_process_into_cache(rb_root *proc_cache, NXRM_PROCESS *proc)
{
	NXRM_PROCESS *process = NULL;

	rb_node **ite;
	rb_node *parent = NULL;

	ite = &(proc_cache->rb_node);
	parent = NULL;


	while (*ite) 
	{ 
		process = rb_entry(*ite, NXRM_PROCESS, process_cache_node);

		parent = *ite;

		if (proc->process_id < process->process_id) 
			ite = &((*ite)->rb_left); 
		else if (proc->process_id > process->process_id) 
			ite = &((*ite)->rb_right); 
		else 
			return FALSE; 
	}

	rb_link_node(&proc->process_cache_node, parent, ite);
	rb_insert_color(&proc->process_cache_node, proc_cache); 

	return TRUE;
}

BOOLEAN Install_nxrmcore(IN NXRM_PROCESS *process)
{
	BOOLEAN bRet = TRUE;

	UCHAR	trampoline_jmp[5] = {0xe9,0x00,0x00,0x00,0x00};

	UCHAR	loader_jmp[5] = {0xe9,0x00,0x00,0x00,0x00};

	ULONG_PTR	dst_offset = 0;
	ULONG_PTR	src_offset = 0;

	do
	{
		//
		// get kernel address
		//

		//
		// build loader function
		//

		memcpy((UCHAR*)process->trampoline_address,
			   g_k_loader_function,
			   sizeof(g_k_loader_function));

		//
		// set LoadLibraryW function pointer
		//
		*(ULONG*)((UCHAR*)process->trampoline_address + 0x85) = (ULONG)process->fn_LoadLibraryW;

		//
		// set Win32StartAddress field in loader function
		//
		*(ULONG*)((UCHAR*)process->trampoline_address + 0xA5) = (ULONG)process->win32start_address;

		//
		// set VirtualProtect function pointer
		//
		*(ULONG*)((UCHAR*)process->trampoline_address + 0xB9) = (ULONG)process->fn_VirtualProtect;

		//
		// set src_addr field in loader function
		//
		*(ULONG*)((UCHAR*)process->trampoline_address + 0xC5) = (ULONG)(ULONG_PTR)((UCHAR*)process->trampoline_address + sizeof(g_k_loader_function));

		//
		// set dst_addr field in loader function
		//
		*(ULONG*)((UCHAR*)process->trampoline_address + 0xCA) = (ULONG)process->win32start_address;

		//
		// set memcpy field in loader function
		//
		*(ULONG*)((UCHAR*)process->trampoline_address + 0xCF) = (ULONG)process->fn_Memcpy;

		//
		// set VirtualProtect function pointer
		//
		*(ULONG*)((UCHAR*)process->trampoline_address + 0xE7) = (ULONG)process->fn_VirtualProtect;

		//
		// set trampoline jmp field in loader function
		//

		src_offset = (ULONG_PTR)((UCHAR*)process->trampoline_address + sizeof(g_k_loader_function) - sizeof(trampoline_jmp));
		dst_offset = (ULONG_PTR)process->win32start_address;

		*(ULONG*)&trampoline_jmp[1] = (ULONG)(dst_offset - (src_offset + 5));

		memcpy((UCHAR*)process->trampoline_address + sizeof(g_k_loader_function) - sizeof(trampoline_jmp),
			   trampoline_jmp,
			   sizeof(trampoline_jmp));


		//
		// backup bytes start from k_win32start_addr after loader function
		//
		memcpy((UCHAR*)process->trampoline_address + sizeof(g_k_loader_function),
			   (UCHAR*)process->win32start_address,
			   5);

		//
		// set loader jmp in patch function
		//
		src_offset = process->win32start_address;
		dst_offset = process->trampoline_address;

		*(ULONG*)&loader_jmp[1] = (ULONG)(dst_offset - (src_offset + 5));

		memcpy((UCHAR*)process->win32start_address,
			   loader_jmp,
			   sizeof(loader_jmp));

		if(Global.fn_ZwFlushInstructionCache)
		{
			Global.fn_ZwFlushInstructionCache(NtCurrentProcess(),
											  (PVOID)process->win32start_address,
											  sizeof(loader_jmp));
		}
		else
		{
			bRet = FALSE;
		}

	}while(FALSE);

	return bRet;
}

#ifdef _AMD64_

BOOLEAN Install_nxrmcore64(
	IN NXRM_PROCESS *process
	)
{
	BOOLEAN bRet = TRUE;

	UCHAR	loader_jmp[] = {0x48,0xb8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
		0xff,0xe0};

	ULONG_PTR	dst_offset = 0;
	ULONG_PTR	src_offset = 0;

	do
	{
		//
		// get kernel address
		//

		//
		// build loader function
		//

		memcpy((UCHAR*)process->trampoline_address,
			   g_k_loader_function64,
			   sizeof(g_k_loader_function64));

		//
		// set LoadLibraryW function pointer
		//
		//*(ULONG_PTR*)((UCHAR*)process->trampoline_address + 0x5e) = process->fn_LoadLibraryW;
		*(ULONG_PTR*)((UCHAR*)process->trampoline_address + 0x164) = process->fn_LoadLibraryW;

		//
		//	set VirtualProtect function pointer
		//
		//*(ULONG_PTR*)((UCHAR*)process->trampoline_address + 0x6f) = process->fn_VirtualProtect;
		*(ULONG_PTR*)((UCHAR*)process->trampoline_address + 0x175) = process->fn_VirtualProtect;

		//
		// set Win32StartAddress field in loader function
		//
		//*(ULONG_PTR*)((UCHAR*)process->trampoline_address + 0x79) = process->win32start_address;
		*(ULONG_PTR*)((UCHAR*)process->trampoline_address + 0x17F) = process->win32start_address;

		//
		// set memcpy field in loader function
		//
		//*(ULONG_PTR*)((UCHAR*)process->trampoline_address + 0x94) = process->fn_Memcpy;
		*(ULONG_PTR*)((UCHAR*)process->trampoline_address + 0x19C) = process->fn_Memcpy;


		//
		// set dst_addr field in loader function
		//
		//*(ULONG_PTR*)((UCHAR*)process->trampoline_address + 0x9e) = process->win32start_address;
		*(ULONG_PTR*)((UCHAR*)process->trampoline_address + 0x1A6) = process->win32start_address;

		//
		// set src_addr field in loader function
		//
		//*(ULONG_PTR*)((UCHAR*)process->trampoline_address + 0xa8) = (ULONG_PTR)((UCHAR*)process->trampoline_address + sizeof(g_k_loader_function64));
		*(ULONG_PTR*)((UCHAR*)process->trampoline_address + 0x1B0) = (ULONG_PTR)((UCHAR*)process->trampoline_address + sizeof(g_k_loader_function64));

		//
		// set VirtualProtect function pointer
		//
		//*(ULONG_PTR*)((UCHAR*)process->trampoline_address + 0xbb) = process->fn_VirtualProtect;
		*(ULONG_PTR*)((UCHAR*)process->trampoline_address + 0x1C3) = process->fn_VirtualProtect;

		//
		// set Win32StartAddress field in loader function
		//
		//*(ULONG_PTR*)((UCHAR*)process->trampoline_address + 0xca) = process->win32start_address;
		*(ULONG_PTR*)((UCHAR*)process->trampoline_address + 0x1D2) = process->win32start_address;

		//
		// set offset jmp field in loader function
		//

		//*(ULONG_PTR*)((UCHAR*)process->trampoline_address + 0xe5) = process->win32start_address;
		*(ULONG_PTR*)((UCHAR*)process->trampoline_address + 0x1F0) = process->win32start_address;


		//
		// backup bytes start from k_win32start_addr after loader function
		//
		memcpy((UCHAR*)process->trampoline_address + sizeof(g_k_loader_function64),
			   (UCHAR*)process->win32start_address,
			   sizeof(loader_jmp));

		//
		// set loader jmp in patch function
		//
		*(ULONG_PTR*)&loader_jmp[2] = process->trampoline_address;

		memcpy((UCHAR*)process->win32start_address,
			   loader_jmp,
			   sizeof(loader_jmp));

		if(Global.fn_ZwFlushInstructionCache)
		{
			Global.fn_ZwFlushInstructionCache(NtCurrentProcess(),
											  (PVOID)process->win32start_address,
											  sizeof(loader_jmp));
		}
		else
		{
			bRet = FALSE;
		}

	} while(FALSE);

	return bRet;
}

#endif // _AMD64_

void kernel_apc_proc(
	KAPC				*Apc,
	PKNORMAL_ROUTINE	*norm_routine,
	PVOID				*context,
	PVOID				*SysArg1,
	PVOID				*SysArg2
	)
{

	do 
	{
		if(PsIsThreadTerminating(PsGetCurrentThread()))
		{
			*norm_routine = NULL;
			break;
		}

	} while(FALSE);

	ExFreeToNPagedLookasideList(&Global.apc_pool, Apc);

	Apc = NULL;

	return;
}

BOOLEAN is_process_a_service(PEPROCESS  Process)
{
	BOOLEAN bRet = FALSE;

	TOKEN_USER	user;

	PACCESS_TOKEN	pPrimaryToken = NULL;

	NTSTATUS status = STATUS_SUCCESS;

	PISID	pUserSid = NULL;

	do 
	{
		memset(&user,0,sizeof(user));

		pPrimaryToken = PsReferencePrimaryToken(Process);

		if(!pPrimaryToken)
		{
			break;
		}

		status = SeQueryInformationToken(pPrimaryToken,TokenUser,(PVOID)&user);

		if(!NT_SUCCESS(status))
		{
			break;
		}

		pUserSid = *(PISID*)user.User.Sid;

		switch (pUserSid->SubAuthority[0])
		{
		case SECURITY_LOCAL_SYSTEM_RID:
		case SECURITY_LOCAL_SERVICE_RID:
		case SECURITY_NETWORK_SERVICE_RID:
			bRet = TRUE;
			break;
		default:
			bRet = FALSE;
			break;
		}

	} while (FALSE);

	if(pPrimaryToken)
	{
		PsDereferencePrimaryToken(pPrimaryToken);
		pPrimaryToken = NULL;
	}

	return bRet;
}


void hash2wstr(UCHAR *hash, WCHAR *wstr)
{
	static const WCHAR table[16] ={L'0',L'1',L'2',L'3',L'4',L'5',L'6',L'7',L'8',L'9',L'A',L'B',L'C',L'D',L'E',L'F'};

	ULONG i = 0;

	for(i = 0; i < 16; i++)
	{
		wstr[i*2]	= table[(hash[i] & 0xf0) >> 4];
		wstr[i*2+1] = table[hash[i] & 0x0f];
	}

	return;
}

void normal_apc_proc(
	PVOID				*context,
	PVOID				*SysArg1,
	PVOID				*SysArg2
	)
{
	NTSTATUS status = STATUS_SUCCESS;

	ULONG			oldprotect = PAGE_READONLY;
	PVOID			pBase= NULL;
	PVOID			pWin32StartAddr = NULL;
	NXRM_PROCESS	*process = NULL;

#ifdef _AMD64_
	SIZE_T			size = (sizeof(g_k_loader_function64) + 63) & (~63);
	SIZE_T			bytes_to_protect = 12;
#else
	SIZE_T			size = (sizeof(g_k_loader_function) + 63) & (~63);
	SIZE_T			bytes_to_protect = 5;
#endif	//_AMD64_

	__try
	{

		do 
		{
			process = (NXRM_PROCESS*)SysArg1;

			if(!process)
			{
				break;
			}

			status = ZwAllocateVirtualMemory(NtCurrentProcess(),
											 &pBase,
											 0,
											 &size,
											 MEM_COMMIT,
											 PAGE_READWRITE);

			if(!NT_SUCCESS(status))
			{
				break;
			}

			if (Global.fn_ZwProtectVirtualMemory)
			{
				ULONG oldbaseprotect = PAGE_READWRITE;

				status = Global.fn_ZwProtectVirtualMemory(NtCurrentProcess(),
														  &pBase,
														  &size,
														  PAGE_EXECUTE_READWRITE,
														  &oldbaseprotect);

				if (!NT_SUCCESS(status))
				{
					break;
				}
			}

			process->trampoline_address = (ULONG_PTR)pBase;

			if(Global.fn_ZwProtectVirtualMemory)
			{
				pWin32StartAddr = (PVOID)process->win32start_address;

				status = Global.fn_ZwProtectVirtualMemory(NtCurrentProcess(),
														  &pWin32StartAddr,
														  &bytes_to_protect,
														  PAGE_EXECUTE_READWRITE,
														  &oldprotect);

				if(!NT_SUCCESS(status))
				{
					size = 0;

					if(Global.fn_ZwFreeVirtualMemory)
					{
						Global.fn_ZwFreeVirtualMemory(NtCurrentProcess(),
													  &pBase,
													  &size,
													  MEM_RELEASE);
					}
					else
					{
						dprintf("[nxrmdrv] Can not free memory because ZwFreeVirtualMemory does not exists!!\n");
					}

					process->trampoline_address = 0;

					break;
				}
#ifndef _AMD64_
				Install_nxrmcore(process);
#else
				if(process->platform == IMAGE_FILE_MACHINE_AMD64)
				{
					Install_nxrmcore64(process);
				}
				else
				{
					Install_nxrmcore(process);
				}
#endif //_AMD64_

				status = Global.fn_ZwProtectVirtualMemory(NtCurrentProcess(),
														  &pWin32StartAddr,
														  &bytes_to_protect,
														  oldprotect,
														  &oldprotect);
				if(!NT_SUCCESS(status))
				{
					dprintf("[nxrmdrv] fail to change loader back\n");
					//
					// we still can continue here so I don't break
					//
				}

				status = Global.fn_ZwProtectVirtualMemory(NtCurrentProcess(),
														  &pBase,
														  &size,
														  PAGE_EXECUTE_READ,
														  &oldprotect);
				if(!NT_SUCCESS(status))
				{
					dprintf("[nxrmdrv] fail to change trampoline memory to PAGE_EXECUTE_READ\n");
					//
					// we still can continue here so I don't break
					//
				}
			}
			else
			{
				dprintf("[nxrmdrv] free allocated memory because ZwProtectVirtualMemory does not exits!!!\n");

				size = 0;

				if(Global.fn_ZwFreeVirtualMemory)
				{
					Global.fn_ZwFreeVirtualMemory(NtCurrentProcess(),
												  &pBase,
												  &size,
												  MEM_RELEASE);
				}
				else
				{
					dprintf("[nxrmdrv] Can not free memory because ZwFreeVirtualMemory does not exists!!\n");
				}

				process->trampoline_address = 0;

				break;
			}

		} while(FALSE);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		dprintf("[nxrmdrv] Exception at checkpoint 2. Process with PID %d is terminating\n", (ULONG)(ULONG_PTR)PsGetCurrentProcessId());
	}

	return;
}

#ifdef _AMD64_

BOOLEAN is_wow64_ntdll(PUNICODE_STRING FullImageName)
{
	return is_xxx_dll(FullImageName, g_wsz_wow64_ntdll);
}

BOOLEAN is_wow64_kernel32(PUNICODE_STRING FullImageName)
{
	return is_xxx_dll(FullImageName, g_wsz_wow64_kernel32);
}

BOOLEAN is_wow64_mscoree(PUNICODE_STRING FullImageName)
{
	return is_xxx_dll(FullImageName, g_wsz_wow64_mscoree);
}

#endif

BOOLEAN is_trustapp(void)
{
	BOOLEAN bRet = FALSE;

	CHAR *p = NULL;

	do
	{
		p = PsGetProcessImageFileName(PsGetCurrentProcess());

		if (p)
		{
			if (_stricmp(p, "ANSYSViewer.ex") == 0)
			{
				bRet = TRUE;
			}
		}

	} while (FALSE);

	return bRet;
}


BOOLEAN is_xxx_dll(PUNICODE_STRING FullImageName, WCHAR *DllName)
{
	BOOLEAN bRet = TRUE;

	USHORT i = 0;

	WCHAR *p = NULL;
	WCHAR *q = NULL;

	WCHAR c1 = L'\0';
	WCHAR c2 = L'\0';

	USHORT length = 0;

	length = (USHORT)wcslen(DllName);

	p = (WCHAR*)((UCHAR*)FullImageName->Buffer + FullImageName->Length - sizeof(WCHAR));

	q = DllName + length - 1;

	for (i = min(FullImageName->Length >> 1, length); i > 0; i--, p--, q--)
	{
		c1 = *p;
		c2 = *q;

		if (c1 == c2)
		{
			continue;
		}
		else if ((c1 - c2) == (L'A' - L'a'))
		{
			continue;
		}
		else if ((c2 - c1) == (L'A' - L'a'))
		{
			continue;
		}
		else
		{
			bRet = FALSE;
			break;
		}
	}

	return bRet;
}