#include "precomp.h"
#include "nxrmdrv.h"
#include "nxrmdrvdef.h"
#include "nxrmdrvhlp.h"

extern NXRM_GLOBAL_DATA	Global;

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
/*  lea         rcx, qword ptr [rsp-20h]                                */
/*  call        rax                                                     */
/*  xor         eax, eax                                                */
/*  mov         ecx, eax                                                */
/*  mov         rax, RtlExitUserThread                                  */
/*  call        rax                                                     */
/*  ret                                                                 */
/*                                                                      */
/*                                                                      */
/************************************************************************/

static CONST UCHAR g_threadproc64[] = {\
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
	0x33,0xc0,\
	0x8b,0xc8,\
	0x48,0xb8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
	0xff,0xd0,\
	0xcc};

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
/*  xor         eax,eax                                                 */
/*  mov         eax, RtlExitUserThread                                  */
/*  call        eax                                                     */
/*  ret                                                                 */
/*                                                                      */
/*                                                                      */
/************************************************************************/

static CONST UCHAR g_threadproc[] = {\
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
	0x33,0xc0,\
	0xb8,0x00,0x00,0x00,0x00,\
	0xff,0xd0,\
	0xcc};

/************************************************************************/
/*																		*/
/*  xor         eax, eax                                                */
/*  mov         ecx, eax                                                */
/*  mov         rax, RtlExitUserProcess                                 */
/*  call        rax                                                     */
/*  ret                                                                 */
/*                                                                      */
/************************************************************************/

static CONST UCHAR g_killdwmThreadProc64[] = { \
	0x33,0xc0, \
	0x8b,0xc8, \
	0x48,0xb8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0xff,0xd0, \
	0xc3 };

/************************************************************************/
/*                                                                      */
/*  xor         eax,eax                                                 */
/*  push        eax                                                     */
/*  mov         eax, RtlExitUserProcess                                 */
/*  call        eax                                                     */
/*  ret                                                                 */
/*                                                                      */
/************************************************************************/

static CONST UCHAR g_killdwmThreadProc[] = { \
	0x33,0xc0, \
	0x50, \
	0xb8,0x00,0x00,0x00,0x00, \
	0xff,0xd0, \
	0xc3 };

static PVOID g_fn_LoadLibraryW = NULL;
static PVOID g_fn_RtlExitUserThread = NULL;
static PVOID g_fn_RtlExitUserProcess = NULL;

#ifdef _AMD64_

BOOLEAN is_Wow64Process(void);

static BOOLEAN find_wow64_usermode_xxx(PVOID *fn_LoadLibraryW,PVOID *fn_RtlExitUserThread);
static BOOLEAN init_context_wow64(CONTEXT *context, ULONG_PTR StartAddr, ULONG_PTR Parameter, ULONG_PTR StackBase);

#endif

static BOOLEAN init_stack(INITIAL_TEB *init_teb);
static BOOLEAN init_context(CONTEXT *context, ULONG_PTR StartAddr, ULONG_PTR Parameter, ULONG_PTR StackBase);

static DECLSPEC_CACHEALIGN WCHAR g_wsz_kernel32_full_path[NXRM_MAX_PATH_LEN];
static DECLSPEC_CACHEALIGN WCHAR g_wsz_ntdll_full_path[NXRM_MAX_PATH_LEN];
static DECLSPEC_CACHEALIGN WCHAR g_wsz_wow64_kernel32_full_path[NXRM_MAX_PATH_LEN];
static DECLSPEC_CACHEALIGN WCHAR g_wsz_wow64_ntdll_full_path[NXRM_MAX_PATH_LEN];

//////////////////////////////////////////////////////////////////////////
//
// external functions
//
//////////////////////////////////////////////////////////////////////////

extern NTSTATUS ZwQuerySystemInformation(
	ULONG	SystemInformationClass,
	PVOID	SystemInformation,
	ULONG	SystemInformationLength,
	PULONG	ReturnLength
	);

extern NTSTATUS ZwQueryInformationProcess(
	HANDLE				ProcessHandle,
	PROCESSINFOCLASS	ProcessInformationClass,
	PVOID				ProcessInformation,
	ULONG				ProcessInformationLength,
	PULONG				ReturnLength
	);

extern NTSTATUS KeUserModeCallback(
	IN ULONG		ApiNumber,
	IN PVOID		InputBuffer,
	IN ULONG		InputLength,
	OUT PVOID		*OutputBuffer,
	IN PULONG		OutputLength
	);

extern PVOID PsGetProcessSectionBaseAddress(PEPROCESS Process);

extern ULONG PsGetCurrentProcessSessionId(void);

extern LPSTR PsGetProcessImageFileName(PEPROCESS  Process);   

#ifdef _AMD64_

extern PVOID PsGetProcessWow64Process(PEPROCESS Process);

#endif

extern NXRM_PROCESS* find_process_in_cache(rb_root *proc_cache, HANDLE process_id);

extern BOOLEAN	add_process_into_cache(rb_root *proc_cache, NXRM_PROCESS *proc);

extern void hash2wstr(UCHAR *hash, WCHAR *wstr);

extern BOOLEAN is_process_a_service(PEPROCESS Process);

extern BOOLEAN is_trustapp(void);

extern BOOLEAN is_xxx_dll(PUNICODE_STRING FullImageName, WCHAR *DllName);

//////////////////////////////////////////////////////////////////////////
//
// local static functions
//
//////////////////////////////////////////////////////////////////////////

static BOOLEAN find_usermode_xxx(SYSTEM_PROCESS_INFORMATION *pProcessInfo,PVOID *fn_LoadLibraryW, PVOID *fn_RtlExitUserThread, PVOID *pfn_RtlExitUserProcess);
static BOOLEAN is_kernel32_full_path(PUNICODE_STRING FullImagePath);
static BOOLEAN is_ntdll_full_path(PUNICODE_STRING FullImagePath);
static BOOLEAN is_wow64_kernel32_full_path(PUNICODE_STRING32 FullImageName);
static BOOLEAN is_wow64_ntdll_full_path(PUNICODE_STRING32 FullImagName);
static BOOLEAN is_dwm(void);

//////////////////////////////////////////////////////////////////////////
//
// local functions
//
//////////////////////////////////////////////////////////////////////////

void apc_kernel_callback(
	KAPC				*Apc,
	PKNORMAL_ROUTINE	*norm_routine,
	PVOID				*context,
	PVOID				*SysArg1,
	PVOID				*SysArg2
	);

void apc_normal_callback(
	PVOID				context,
	PVOID				SysArg1,
	PVOID				SysArg2
	);

void win10_apc_normal_callback(
	PVOID				context,
	PVOID				SysArg1,
	PVOID				SysArg2
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


NTSTATUS NxrmLaunchInitialize(void)
{
	NTSTATUS status = STATUS_SUCCESS;

	ULONG				length = 0;
	ULONG				lengthret = 0;

	SYSTEM_PROCESS_INFORMATION		*pProcessInfo = NULL;
	SYSTEM_PROCESS_INFORMATION		*pProcessItem = NULL;
	SYSTEM_THREAD_INFORMATION		*pThreadInfo = NULL;

	PETHREAD	threadobj = NULL;	
	KAPC		*apc = NULL;

	do 
	{
		lengthret = 256 * 1024;	// 256k

		do 
		{
			length = (lengthret + 4095) & (~4095);

			if(pProcessInfo)
			{
				ExFreePoolWithTag(pProcessInfo, NXRMTAG);
				pProcessInfo = NULL;
			}

			pProcessInfo = ExAllocatePoolWithTag(PagedPool,
												 length,
												 NXRMTAG);

			if(pProcessInfo == NULL)
			{
				status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

			RtlZeroMemory(pProcessInfo,length);

			status = ZwQuerySystemInformation(SystemProcessInformation,
											  pProcessInfo,
											  length,
											  &lengthret);

		} while(status == STATUS_INFO_LENGTH_MISMATCH && length <=4*1024*1024);

		if(status != STATUS_SUCCESS)
		{
			break;
		}

		//
		// initialize g_wsz_kernel32_full_path
		//
		memset(g_wsz_kernel32_full_path,0,sizeof(g_wsz_kernel32_full_path));
		memset(g_wsz_wow64_kernel32_full_path,0,sizeof(g_wsz_wow64_kernel32_full_path));
		memset(g_wsz_ntdll_full_path,0,sizeof(g_wsz_ntdll_full_path));
		memset(g_wsz_wow64_ntdll_full_path,0,sizeof(g_wsz_wow64_ntdll_full_path));

#ifdef _AMD64_

		RtlStringCbCatW(g_wsz_wow64_ntdll_full_path,sizeof(g_wsz_wow64_ntdll_full_path),L"\\ntdll.dll");

		RtlStringCbCatW(g_wsz_wow64_kernel32_full_path, sizeof(g_wsz_wow64_kernel32_full_path), L"\\kernel32.dll");

#endif

		RtlStringCbCatW(g_wsz_kernel32_full_path, sizeof(g_wsz_kernel32_full_path), (WCHAR*)(0x7ffe0030));
		RtlStringCbCatW(g_wsz_ntdll_full_path, sizeof(g_wsz_ntdll_full_path), (WCHAR*)(0x7ffe0030));

		RtlStringCbCatW(g_wsz_kernel32_full_path,sizeof(g_wsz_kernel32_full_path),L"\\System32\\kernel32.dll");
		RtlStringCbCatW(g_wsz_ntdll_full_path,sizeof(g_wsz_ntdll_full_path),L"\\System32\\ntdll.dll");

		//
		// try to find kernel32!LoadLibraryW
		//

		find_usermode_xxx(pProcessInfo, &g_fn_LoadLibraryW, &g_fn_RtlExitUserThread, &g_fn_RtlExitUserProcess);

		if(g_fn_LoadLibraryW == NULL ||
		   g_fn_RtlExitUserThread == NULL ||
		   g_fn_RtlExitUserProcess == NULL ||
		   Global.fn_ZwCreateThread == NULL)
		{
			break;
		}

		//
		// this means we are running for the first time after install
		//
		pProcessItem = pProcessInfo;

		while(pProcessItem->NextEntryDelta)
		{
			if(pProcessItem->ProcessId == NULL || pProcessItem->ProcessId == (HANDLE)4)
			{
				pProcessItem = (SYSTEM_PROCESS_INFORMATION*)((UCHAR*)pProcessItem + pProcessItem->NextEntryDelta);
				continue;
			}

			pThreadInfo = pProcessItem->Threads;

			status = PsLookupThreadByThreadId(pThreadInfo->ClientId.UniqueThread,&threadobj);

			if(NT_SUCCESS(status))
			{
				apc = (KAPC*)ExAllocateFromNPagedLookasideList(&Global.apc_pool);

				if(apc)
				{
					if (Global.os_major == 10 && Global.os_build < WBN_WIN10_MAY2019UPDATE)
					{
						KeInitializeApc(apc,
							threadobj,
							0,
							apc_kernel_callback,
							NULL,
							win10_apc_normal_callback,
							KernelMode,
							NULL);
					}
					else
					{
						KeInitializeApc(apc,
							threadobj,
							0,
							apc_kernel_callback,
							NULL,
							apc_normal_callback,
							KernelMode,
							NULL);
					}

					KeInsertQueueApc(apc,(PVOID)g_fn_LoadLibraryW,NULL,0);
				}
				else
				{
					dprintf("[nxrmdrv] OS is running out of memory![Checkpoint 6]!\n");
				}

				ObDereferenceObject(threadobj);
				threadobj = NULL;
			}
			else
			{
				dprintf("[nxrmdrv] Failed PsLookupThreadByThreadId\n");
			}

			pProcessItem = (SYSTEM_PROCESS_INFORMATION*)((UCHAR*)pProcessItem + pProcessItem->NextEntryDelta);
		}

	} while (FALSE);

	if(pProcessInfo)
	{
		ExFreePoolWithTag(pProcessInfo, NXRMTAG);
		pProcessInfo = NULL;
	}

	return status;
}

NTSTATUS NxrmLaunchCleanup(void)
{
	return STATUS_SUCCESS;
}

void apc_kernel_callback(
	KAPC				*Apc,
	PKNORMAL_ROUTINE	*norm_routine,
	PVOID				*context,
	PVOID				*SysArg1,
	PVOID				*SysArg2
	)
{
	NTSTATUS status = STATUS_SUCCESS;

	// variable for step 1
	BOOLEAN	bIsService = FALSE;
	BOOLEAN bIsBypassApp = FALSE;
	BOOLEAN	bIsDwm = FALSE;

#ifdef _AMD64_

	BOOLEAN bIsWow64Process = FALSE;
	PVOID	fn_wow64LoadLibraryW = NULL;
	PVOID	fn_wow64RtlExitUserThread = NULL;

#endif
	// variable for step 2
	NXRM_PROCESS	*process = NULL;
	BOOLEAN			bRet = TRUE;
	PUNICODE_STRING	pproc_name = NULL;
	ULONG			lengthret = 0;
	UCHAR			buf[MAX_PATH_LEN*sizeof(WCHAR)];


	// variable for step 3
	PVOID	u_trampoline_function = NULL;
	SIZE_T	size = 256;
	ULONG	oldprotect = PAGE_EXECUTE_READWRITE;

	//
	// variable for step 4
	KLOCK_QUEUE_HANDLE lockhandle = {0};

	do 
	{
		if(PsIsThreadTerminating(PsGetCurrentThread()))
		{
			*norm_routine = NULL;
			break;
		}

		//
		// Step 1:
		//
		//	We are going to find out is current process a service in this step
		//

		bIsService = is_process_a_service(PsGetCurrentProcess());

#ifdef _AMD64_

		bIsWow64Process = is_Wow64Process();
#endif
		bIsDwm = is_dwm();

		//
		// Step 2:
		//
		//	We are going to create a NXRM_PROCESS structure in this step
		//	and fill two fields in the NXRM_PROCESS structure we allocate
		//

		process = (NXRM_PROCESS*)ExAllocateFromNPagedLookasideList(&Global.process_cache_pool);

		if(process)
		{
			process->process_id				= PsGetCurrentProcessId();
			process->parent_id				= 0;
			process->hooked					= FALSE;
			process->kernel32_base			= 0;
			process->ntdll_base				= 0;
			process->fn_LoadLibraryW		= 0;
			process->fn_Memcpy				= 0;
			process->fn_VirtualProtect		= 0;
			process->fn_CorExeMain			= 0;
			process->process_base			= (ULONG_PTR)PsGetProcessSectionBaseAddress(PsGetCurrentProcess());
			process->trampoline_address		= 0;
			process->win32start_address		= 0;
			process->is_Service				= bIsService?TRUE:FALSE;
			process->isbypassapp			= FALSE;
			process->initialized			= TRUE;	// set to true because this record has all information		
			process->session_id				= PsGetCurrentProcessSessionId();	// this is correct because we are under the process context
#ifdef _AMD64_
			process->platform				= bIsWow64Process?IMAGE_FILE_MACHINE_I386:IMAGE_FILE_MACHINE_AMD64;
#else
			process->platform				= IMAGE_FILE_MACHINE_I386;
#endif
			process->thread_count			= 0;

			memset(process->process_path,0,sizeof(process->process_path));

			status = ZwQueryInformationProcess(NtCurrentProcess(),
											   ProcessImageFileName,
											   buf,
											   sizeof(buf),
											   &lengthret);

			if(NT_SUCCESS(status))
			{
				pproc_name = (UNICODE_STRING*)buf;

				RtlCopyMemory(process->process_path,
							  pproc_name->Buffer,
							  min(pproc_name->Length,sizeof(process->process_path)-sizeof(WCHAR)));
			}

			_wcslwr(process->process_path);

			if (is_trustapp())
			{
				process->isbypassapp = TRUE;
				bIsBypassApp = TRUE;
			}
		}

		//
		// Step 3:
		//
		//	We are going to allocate memory for our loader function
		//

		status =  ZwAllocateVirtualMemory(NtCurrentProcess(),
										  &u_trampoline_function,
										  0,
										  &size,
										  MEM_COMMIT,
										  PAGE_EXECUTE_READWRITE);

		if(NT_SUCCESS(status))
		{
			memset(u_trampoline_function,0xC3,size);

#ifdef _AMD64_
			if(bIsWow64Process)
			{
				RtlCopyMemory(u_trampoline_function,
							  g_threadproc,
							  sizeof(g_threadproc));

				find_wow64_usermode_xxx(&fn_wow64LoadLibraryW,&fn_wow64RtlExitUserThread);

				*(ULONG*)((UCHAR*)u_trampoline_function + 0x85) = (ULONG)(ULONG_PTR)fn_wow64LoadLibraryW;
				*(ULONG*)((UCHAR*)u_trampoline_function + 0x91) = (ULONG)(ULONG_PTR)fn_wow64RtlExitUserThread;
			}
			else
			{
				if (!bIsDwm)
				{
					RtlCopyMemory(u_trampoline_function,
								  g_threadproc64,
								  sizeof(g_threadproc64));

					*(ULONG_PTR*)((UCHAR*)u_trampoline_function + 0x164) = (ULONG_PTR)g_fn_LoadLibraryW;
					*(ULONG_PTR*)((UCHAR*)u_trampoline_function + 0x179) = (ULONG_PTR)g_fn_RtlExitUserThread;
				}
				else
				{
					RtlCopyMemory(u_trampoline_function,
								  g_killdwmThreadProc64,
								  sizeof(g_killdwmThreadProc64));

					*(ULONG_PTR*)((UCHAR*)u_trampoline_function + 0x6) = (ULONG_PTR)g_fn_RtlExitUserProcess;
				}
			}
#else
			if (!bIsDwm)
			{
				RtlCopyMemory(u_trampoline_function,
							  g_threadproc,
							  sizeof(g_threadproc));

				*(ULONG*)((UCHAR*)u_trampoline_function + 0x85) = (ULONG)g_fn_LoadLibraryW;
				*(ULONG*)((UCHAR*)u_trampoline_function + 0x91) = (ULONG)g_fn_RtlExitUserThread;
			}
			else
			{
				RtlCopyMemory(u_trampoline_function,
							  g_killdwmThreadProc,
							  sizeof(g_killdwmThreadProc));

				*(ULONG_PTR*)((UCHAR*)u_trampoline_function + 0x4) = (ULONG_PTR)g_fn_RtlExitUserProcess;
			}

#endif
			if(Global.fn_ZwProtectVirtualMemory)
			{
				Global.fn_ZwProtectVirtualMemory(NtCurrentProcess(),
												 &u_trampoline_function,
												 &size,
												 PAGE_EXECUTE_READ,
												 &oldprotect);
			}
		}
		else
		{
			//
			// just making sure
			//
			u_trampoline_function = NULL;
		}

		//
		// Step 4:
		//
		//	We are going to add process into our process cache
		//
		if(process)
		{
			KeAcquireInStackQueuedSpinLock(&Global.process_cache_lock, &lockhandle);

			bRet = add_process_into_cache(&Global.rb_process_cache, process);

			if(bRet)
				InterlockedIncrement(&Global.process_count);

			KeReleaseInStackQueuedSpinLock(&lockhandle);

			//
			// skip this one if the record is in our cache
			//
			if(bRet == FALSE)
			{
				ExFreeToNPagedLookasideList(&Global.process_cache_pool, (PVOID)process);
				*norm_routine = NULL;
				process = NULL;
				break;
			}
		}

		//
		// Step 5:
		//
		//	Set user APC normal routine
		//
		if(bIsService || bIsBypassApp)
		{
			*norm_routine = NULL;
		}
		else
		{
			*SysArg1 = u_trampoline_function;

			*context = (PVOID)process;
		}

	} while (FALSE);

	ExFreeToNPagedLookasideList(&Global.apc_pool, Apc);

	return;
}

void apc_normal_callback(
	PVOID				context,
	PVOID				SysArg1,
	PVOID				SysArg2
	)
{
	NXRM_PROCESS *process = (NXRM_PROCESS*)context;


	NTSTATUS			status = STATUS_SUCCESS;
	CONTEXT				ThreadContext;
	INITIAL_TEB			InitialTeb;
	HANDLE				ThreadHandle = NULL;
	CLIENT_ID			ThreadClientId;

	do 
	{
		memset(&InitialTeb,0,sizeof(InitialTeb));
		memset(&ThreadClientId,0,sizeof(ThreadClientId));

		if(!init_stack(&InitialTeb))
		{
			break;
		}

#ifdef _AMD64_

		if(process->platform == IMAGE_FILE_MACHINE_I386)
		{
			if(!init_context_wow64(&ThreadContext,(ULONG_PTR)SysArg1,0,(ULONG_PTR)InitialTeb.StackBase))
			{
				break;
			}
		}
		else
		{
			if(!init_context(&ThreadContext,(ULONG_PTR)SysArg1,0,(ULONG_PTR)InitialTeb.StackBase))
			{
				break;
			}
		}
#else

		if(!init_context(&ThreadContext,(ULONG_PTR)SysArg1,0,(ULONG_PTR)InitialTeb.StackBase))
		{
			break;
		}

#endif

		status = Global.fn_ZwCreateThread(&ThreadHandle,
										  THREAD_ALL_ACCESS,
										  NULL,
										  NtCurrentProcess(),
										  &ThreadClientId,
										  &ThreadContext,
										  &InitialTeb,
										  FALSE);

		if(NT_SUCCESS(status))
		{
			process->hooked = TRUE;
		}

	} while (FALSE);

	if (ThreadHandle)
	{
		ZwClose(ThreadHandle);
		ThreadHandle = NULL;
	}

	return;
}

static BOOLEAN find_usermode_xxx(SYSTEM_PROCESS_INFORMATION *pProcessInfo,PVOID *pfn_LoadLibraryW, PVOID *pfn_RtlExitUserThread, PVOID *pfn_RtlExitUserProcess)
{
	BOOLEAN bRet = TRUE;

	PVOID fn_LoadLibraryW = NULL;
	PVOID fn_RtlExitUserThread = NULL;
	PVOID fn_RtlExitUserProcess = NULL;

	SYSTEM_PROCESS_INFORMATION	*pProcessItem = NULL;
	PROCESS_BASIC_INFORMATION	ProcessBasicInfo;

	PEPROCESS	process_obj = NULL;
	KAPC_STATE	apc_state;
	ULONG		length = 0;

	NTSTATUS	status = STATUS_SUCCESS;

	UNICODE_STRING strwininit;
	UNICODE_STRING strwinlogon;

	LIST_ENTRY				*ite = NULL;
	LDR_DATA_TABLE_ENTRY	*ldr_item = NULL;

	RtlInitUnicodeString(&strwininit,L"wininit.exe");
	RtlInitUnicodeString(&strwinlogon,L"winlogon.exe");

	pProcessItem = pProcessInfo;

	while(pProcessItem->NextEntryDelta)
	{
		if(RtlCompareUnicodeString(&pProcessItem->ProcessName,&strwinlogon,TRUE) == 0 ||
		   RtlCompareUnicodeString(&pProcessItem->ProcessName,&strwininit,TRUE) == 0)
		{

			status = PsLookupProcessByProcessId(pProcessItem->ProcessId,&process_obj);

			if(!NT_SUCCESS(status))
			{
				bRet = FALSE;
				break;
			}

			KeStackAttachProcess(process_obj,&apc_state);

			status = ZwQueryInformationProcess(NtCurrentProcess(),
											   ProcessBasicInformation,
											   &ProcessBasicInfo,
											   sizeof(ProcessBasicInfo),
											   &length);

			if(!NT_SUCCESS(status))
			{
				KeUnstackDetachProcess(&apc_state);
				bRet = FALSE;
				break;
			}

			FOR_EACH_LIST(ite,&ProcessBasicInfo.PebBaseAddress->LoaderData->InMemoryOrderModuleList)
			{
				ldr_item = CONTAINING_RECORD(ite,LDR_DATA_TABLE_ENTRY,InMemoryOrderLinks);

				if(is_ntdll_full_path(&ldr_item->FullDllName))
				{
					fn_RtlExitUserThread = nxrmFindRtlExitUserThreadAddress(ldr_item->DllBase);

					fn_RtlExitUserProcess = nxrmFindRtlExitUserProcessAddress(ldr_item->DllBase);

					if(fn_RtlExitUserThread && fn_LoadLibraryW && fn_RtlExitUserProcess)
					{
						break;
					}
					else
					{
						continue;
					}
				}

				if(is_kernel32_full_path(&ldr_item->FullDllName))
				{
					fn_LoadLibraryW = nxrmFindLoadLibraryWAddress(ldr_item->DllBase);

					if(fn_RtlExitUserThread && fn_LoadLibraryW && fn_RtlExitUserProcess)
					{
						break;
					}
					else
					{
						continue;
					}
				}
			}

			KeUnstackDetachProcess(&apc_state);
			break;
		}

		pProcessItem = (SYSTEM_PROCESS_INFORMATION*)((UCHAR*)pProcessItem + pProcessItem->NextEntryDelta);
	}

	if(process_obj)
	{
		ObDereferenceObject(process_obj);
		process_obj = NULL;
	}

	*pfn_LoadLibraryW		= fn_LoadLibraryW;
	*pfn_RtlExitUserThread	= fn_RtlExitUserThread;
	*pfn_RtlExitUserProcess = fn_RtlExitUserProcess;

	return bRet;
}


#ifdef _AMD64_

static BOOLEAN find_wow64_usermode_xxx(PVOID *pfn_LoadLibraryW,PVOID *pfn_RtlExitUserThread)
{
	BOOLEAN bRet = TRUE;

	PVOID fn_LoadLibraryW = NULL;
	PVOID fn_RtlExitUserThread = NULL;

	LIST_ENTRY32			*ite = NULL;

	LDR_DATA_TABLE_ENTRY32	*Wow64ldr_item = NULL;
	PEB_LDR_DATA32			*Wow64ldr_data = NULL;

	ULONG		length = 0;

	PEB32 *pWow64Peb = NULL;

	do 
	{
		if(Global.os_major == 6 || Global.os_major == 10)
			pWow64Peb = (PEB32*)(PsGetProcessWow64Process(PsGetCurrentProcess()));
		else
			pWow64Peb = *(PEB32**)(PsGetProcessWow64Process(PsGetCurrentProcess()));

		if(!pWow64Peb)
		{
			break;
		}

		Wow64ldr_data = (PEB_LDR_DATA32*)pWow64Peb->LoaderData;

		for(ite = (LIST_ENTRY32 *)Wow64ldr_data->InMemoryOrderModuleList.Flink;ite != &(Wow64ldr_data->InMemoryOrderModuleList);ite = (LIST_ENTRY32*)ite->Flink)
		{
			Wow64ldr_item = CONTAINING_RECORD(ite,LDR_DATA_TABLE_ENTRY32,InMemoryOrderLinks);

			if(is_wow64_kernel32_full_path(&Wow64ldr_item->FullDllName))
			{
				fn_LoadLibraryW = nxrmFindWow64LoadLibraryWAddress((PVOID)Wow64ldr_item->DllBase);

				if(fn_LoadLibraryW && fn_RtlExitUserThread)
				{
					break;
				}
				else
				{
					continue;
				}
			}

			if(is_wow64_ntdll_full_path(&Wow64ldr_item->FullDllName))
			{
				fn_RtlExitUserThread = nxrmFindWow64RtlExitUserThreadAddress((PVOID)Wow64ldr_item->DllBase);

				if(fn_LoadLibraryW && fn_RtlExitUserThread)
				{
					break;
				}
				else
				{
					continue;
				}
			}
		}

	} while (FALSE);

	*pfn_LoadLibraryW		= fn_LoadLibraryW;
	*pfn_RtlExitUserThread	= fn_RtlExitUserThread;

	if(fn_LoadLibraryW && fn_RtlExitUserThread)
	{
		bRet = TRUE;
	}
	else
	{
		bRet = FALSE;
	}

	return bRet;
}

#endif

static BOOLEAN is_kernel32_full_path(PUNICODE_STRING FullImagePath)
{
	BOOLEAN bRet = TRUE;

	UNICODE_STRING strkernel32;

	RtlInitUnicodeString(&strkernel32,g_wsz_kernel32_full_path);

	if(RtlCompareUnicodeString(FullImagePath,&strkernel32,TRUE) != 0)
	{
		bRet = FALSE;
	}

	return bRet;
}

static BOOLEAN is_ntdll_full_path(PUNICODE_STRING FullImagePath)
{
	BOOLEAN bRet = TRUE;

	UNICODE_STRING strntdll;

	RtlInitUnicodeString(&strntdll,g_wsz_ntdll_full_path);

	if(RtlCompareUnicodeString(FullImagePath,&strntdll,TRUE) != 0)
	{
		bRet = FALSE;
	}

	return bRet;
}

#ifdef _AMD64_

BOOLEAN is_Wow64Process(void)
{
	BOOLEAN bRet = FALSE;

	ULONG_PTR Wow64Process = 0;

	ULONG uBytesRet = 0;

	NTSTATUS status = STATUS_SUCCESS;

	do 
	{
		status = ZwQueryInformationProcess(NtCurrentProcess(),
										   ProcessWow64Information,
										   &Wow64Process,
										   sizeof(Wow64Process),
										   &uBytesRet);

		if(!NT_SUCCESS(status))
		{
			break;
		}

		if(Wow64Process != 0)
		{
			bRet = TRUE;
		}

	} while (FALSE);

	return bRet;
}

static BOOLEAN is_wow64_kernel32_full_path(PUNICODE_STRING32 FullImagePath)
{
	UNICODE_STRING FullImagePath64;

	FullImagePath64.Length			= FullImagePath->Length;
	FullImagePath64.MaximumLength	= FullImagePath->MaximumLength;
	FullImagePath64.Buffer			= (PWCHAR)FullImagePath->Buffer;

	return is_xxx_dll(&FullImagePath64, g_wsz_wow64_kernel32_full_path);
}

static BOOLEAN is_wow64_ntdll_full_path(PUNICODE_STRING32 FullImagePath)
{
	UNICODE_STRING FullImagePath64;

	FullImagePath64.Length			= FullImagePath->Length;
	FullImagePath64.MaximumLength	= FullImagePath->MaximumLength;
	FullImagePath64.Buffer			= (PWCHAR)FullImagePath->Buffer;

	return is_xxx_dll(&FullImagePath64, g_wsz_wow64_ntdll_full_path);
}

static BOOLEAN init_context(CONTEXT *context, ULONG_PTR StartAddr, ULONG_PTR Parameter, ULONG_PTR StackBase)
{
	BOOLEAN bRet = TRUE;

	do 
	{
		if(StackBase & 0xf)
		{
			bRet = FALSE;
			break;
		}

		memset(context,0,sizeof(*context));

		context->Rax					= 0;
		context->Rbp					= 0;
		context->EFlags					= 0x200;
		context->Rbx					= 1;
		context->Rsp					= StackBase - 0x28;
		context->Rsi					= 4;
		context->Rdi					= 5;
		context->R8						= 8;
		context->R10					= 0xa;
		context->R11					= 0xb;
		context->R12					= 0xc;
		context->R13					= 0xd;
		context->R14					= 0xe;
		context->R15					= 0xf;

		memset(&context->FltSave,0,sizeof(context->FltSave));

		context->FltSave.ControlWord	= 0x027f;
		context->Rip					= StartAddr;
		context->Rcx					= Parameter;
		context->R9						= 0x0f0e0d0c0a090807;
		context->MxCsr					= 0x1f80;
		context->FltSave.MxCsr			= 0x1f80;
		context->ContextFlags			= 0x10000b;

	} while (FALSE);

	return bRet;
}

static BOOLEAN init_context_wow64(CONTEXT *context, ULONG_PTR StartAddr, ULONG_PTR Parameter, ULONG_PTR StackBase)
{
	BOOLEAN bRet = TRUE;

	do 
	{
		memset(context,0,sizeof(*context));

		context->Rsp					= StackBase - 0x28;
		context->Rip					= StartAddr;
		context->Rcx					= Parameter;
		context->ContextFlags			= 0x100003;

	} while (FALSE);

	return bRet;
}

#else

static BOOLEAN init_context(CONTEXT *context, ULONG_PTR StartAddr, ULONG_PTR Parameter, ULONG_PTR StackBase)
{
	do 
	{
		memset(context, 0, sizeof(*context));

		context->SegGs	= 0;
		context->Eip	= StartAddr;
		context->Eax	= Parameter;
		context->SegFs	= 0x3B;
		context->SegEs	= 0x23;
		context->SegDs	= 0x23;
		context->SegSs	= 0x23;
		context->SegCs	= 0x1B;
		context->Esp	= StackBase - 0x2cc;

		context->ContextFlags	= 0x10007;

	} while (FALSE);

	return TRUE;
}

#endif

static BOOLEAN init_stack(INITIAL_TEB *init_teb)
{
	BOOLEAN bRet = TRUE;

	NTSTATUS Status = STATUS_SUCCESS;
	UCHAR *Stack = NULL;

	SIZE_T MaximumStackSize	= 1024*1024;
	SIZE_T CommittedStackSize = PAGE_SIZE*9;
	SIZE_T PageSize = PAGE_SIZE;

	ULONG OldProtection = 0;

	memset(init_teb,0,sizeof(*init_teb));

	do 
	{
		//
		// reserve the stack memory
		//
		Status = ZwAllocateVirtualMemory(NtCurrentProcess(),
										 (PVOID *)&Stack,
										 0,
										 &MaximumStackSize,
										 MEM_RESERVE,
										 PAGE_READWRITE);

		if(!NT_SUCCESS(Status))
		{
			bRet = FALSE;
			break;
		}

		init_teb->StackReserved	= Stack;
		init_teb->StackBase		= Stack + MaximumStackSize;

		Stack += (MaximumStackSize - CommittedStackSize);

		//
		// commit 9 pages of stack by default, 8 pages stack and 1 guard page
		//
		Status = ZwAllocateVirtualMemory(NtCurrentProcess(),
										 (PVOID *)&Stack,
										 0,
										 &CommittedStackSize,
										 MEM_COMMIT,
										 PAGE_READWRITE);

		if(!NT_SUCCESS(Status))
		{
			bRet = FALSE;
			break;
		}

		memset(Stack,0,CommittedStackSize);

		init_teb->StackLimit = Stack;

		Status = Global.fn_ZwProtectVirtualMemory(NtCurrentProcess(),
												  (PVOID *)&Stack,
												  &PageSize,
												  PAGE_GUARD|PAGE_READWRITE,
												  &OldProtection);

		if(!NT_SUCCESS(Status))
		{
			break;
		}

		init_teb->StackLimit = (PVOID)(Stack + PageSize);

	} while (FALSE);


	if(!bRet)
	{
		memset(init_teb,0,sizeof(*init_teb));
	}

	return bRet;
}

void win10_apc_normal_callback(
	PVOID				context,
	PVOID				SysArg1,
	PVOID				SysArg2
	)
{
	NXRM_PROCESS *process = (NXRM_PROCESS*)context;


	NTSTATUS			status = STATUS_SUCCESS;
	HANDLE				ThreadHandle = NULL;

	CLIENT_ID Cid = { 0 };
	ULONG_PTR pTeb = 0;
	
	THREADINFO Info = { 0 };

	do
	{

#ifdef _AMD64_

		if (process->platform == IMAGE_FILE_MACHINE_I386)
		{
			THREADINFO32 Wow64Info = { 0 };

			Wow64Info.Size		= sizeof(Wow64Info);
			Wow64Info.Type1		= 0x10003;
			Wow64Info.Length1	= sizeof(Cid);
			Wow64Info.pClientId = &Cid;
			Wow64Info.Reserved1 = 0;

			status = Global.fn_ZwCreateThreadEx(&ThreadHandle,
												THREAD_ALL_ACCESS,
												NULL,
												NtCurrentProcess(),
												(ULONG_PTR)SysArg1,
												NULL,
												FALSE,
												0,
												0,
												0,
												&Wow64Info);

			if (NT_SUCCESS(status))
			{
				process->hooked = TRUE;
			}

		}
		else
#endif
		{
			Info.Size		= sizeof(Info);
			Info.Type1		= 0x10003;
			Info.Length1	= sizeof(Cid);
			Info.pClientId	= &Cid;
			Info.Reserved1	= 0;
			Info.Type2		= 0x10004;
			Info.Length2	= sizeof(pTeb);
			Info.ppTeb		= &pTeb;
			Info.Reserved2	= 0;

			status = Global.fn_ZwCreateThreadEx(&ThreadHandle,
												THREAD_ALL_ACCESS,
												NULL,
												NtCurrentProcess(),
												(ULONG_PTR)SysArg1,
												NULL,
												FALSE,
												0,
												0,
												0,
												&Info);

			if (NT_SUCCESS(status))
			{
				process->hooked = TRUE;
			}
		}

	} while (FALSE);

	if (ThreadHandle)
	{
		ZwClose(ThreadHandle);
		ThreadHandle = NULL;
	}

	return;
}

BOOLEAN is_dwm(void)
{
	BOOLEAN bRet = FALSE;

	CHAR *p = NULL;

	do
	{
		p = PsGetProcessImageFileName(PsGetCurrentProcess());

		if (p)
		{
			if (_stricmp(p, "dwm.exe") == 0)
			{
				bRet = TRUE;
			}
		}

	} while (FALSE);

	return bRet;
}
