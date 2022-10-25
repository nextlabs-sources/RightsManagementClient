#include <windows.h>
#include "detour.h"
#include "disasm.h"

#define TRAMPOLINE_PROC_SIZE	80

typedef struct _TRAMPOLINE_PROC{
	BYTE nop[TRAMPOLINE_PROC_SIZE];
}TRAMPOLINE_PROC,*PTRAMPOLINE_PROC;

#pragma pack(push,1)

typedef struct _DETOUR_CALL_INFO{
	PVOID	original_call_addr;
	PVOID	staging_jmp;
	DWORD	bytes_copied;
	BYTE	original_instructions[1];
}DETOUR_CALL_INFO,*PDETOUR_CALL_INFO;

#pragma pack(pop)

#define DETOUR_CALL_INFO_OFFSET	0x28		// half of trampoline call

#ifndef _AMD64_
static BOOL inspect_original_call_x86(PVOID original_call);
static TRAMPOLINE_PROC* init_trampoline_call_x86(PVOID original_call);
static BOOL build_trampoline_call_from_original_x86(PVOID original_call, PVOID trampoline_call,DWORD bytes_to_save);
static void cleanup_trampoline_call_x86(TRAMPOLINE_PROC* trampoline_call);
#else
static BOOL inspect_original_call_x64(PVOID original_call);
static TRAMPOLINE_PROC* init_trampoline_call_x64(PVOID original_call);
static BOOL build_trampoline_call_from_original_x64(PVOID original_call, PVOID trampoline_call,DWORD bytes_to_save,PVOID staging_jmp);
static void cleanup_trampoline_call_x64(TRAMPOLINE_PROC* trampoline_call);
static BOOL inspect_original_call_ex_x64(PVOID original_call);
static BOOL build_trampoline_call_from_original_ex_x64(PVOID original_call, PVOID trampoline_call,DWORD bytes_to_save,PVOID staging_jmp);
static PVOID find_free_memory_region_x64(PVOID pbase);
static PVOID search_free_memory_region_forward_x64(PVOID pbase);
static PVOID search_free_memory_region_backward_x64(PVOID pbase);
static PVOID build_staging_jmp_x64(PVOID original_call, PVOID new_call);
static PVOID reparse_origial_call_x64(PVOID original_call);

#endif

static PIMAGE_NT_HEADERS RtlImageNtHeader(
	IN PVOID BaseAddress
	);

static PVOID RtlImageDirectoryEntryToData(
	PVOID	BaseAddress,
	BOOLEAN MappedAsImage,
	USHORT	Directory,
	PULONG	Size
	);

static PIMAGE_SECTION_HEADER RtlImageRvaToSection(
	PIMAGE_NT_HEADERS       NtHeader,
	PVOID                   BaseAddress,
	ULONG                   Rva
	);

static PVOID RtlImageRvaToVa (
	PIMAGE_NT_HEADERS       NtHeader,
	PVOID                   BaseAddress,
	ULONG                   Rva,
	PIMAGE_SECTION_HEADER   *SectionHeader
	);

#ifdef _X86_

BOOL install_hook_x86(
	PVOID original_call, 
	PVOID *trampoline_call,
	PVOID new_call
	)
{
	BOOL bRet = TRUE;

	//
	// 20 bytes should be enough
	//
	BYTE	detour_jmp[20]		= {0xe9,0x00,0x00,0x00,0x00,0x90,0x90,0x90,0x90,0x90,\
								   0x90,0x90,0x90,0x90,0x90,0x90,0x90};

	BYTE	trampoline_jmp[5]	= {0xe9,0x00,0x00,0x00,0x00};
	
	PTRAMPOLINE_PROC	pfntrampoline = NULL;

	DWORD	detour_step = 0;
	DWORD	original_page_rights;
	DWORD	bytes_to_save = 0;

	BYTE	first_instruction;

	do 
	{
		if(!inspect_original_call_x86(original_call))
		{
			bRet = FALSE;
			break;
		}

		pfntrampoline = init_trampoline_call_x86(original_call);

		if(!pfntrampoline)
		{
			bRet = FALSE;
			break;
		}

		detour_step++;
		
		bytes_to_save = 0;
		
		__try
		{
			first_instruction = *(BYTE*)original_call;

			if(first_instruction == 0xeb)
			{
				original_call = (PVOID)((BYTE*)(original_call) + 2);
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			
		}

		do 
		{
			bytes_to_save += x86GetCurrentInstructionLength((PBYTE)original_call + bytes_to_save);

		} while(bytes_to_save < sizeof(trampoline_jmp));

		if(!build_trampoline_call_from_original_x86(original_call,pfntrampoline,bytes_to_save))
		{
			bRet = FALSE;
			break;
		}

		//
		// copy detour jmp
		//
		__try
		{
			
			//
			// build detour jmp
			//
			*(DWORD*)(detour_jmp+1) = (DWORD)((BYTE*)new_call - ((BYTE*)original_call + 5));

			if(!VirtualProtect(original_call,
							   bytes_to_save,
							   PAGE_EXECUTE_READWRITE,
							   &original_page_rights))
			{
				bRet = FALSE;
				break;
			}

			memcpy(original_call,
				   detour_jmp,
				   max(5,bytes_to_save));

			if(!VirtualProtect(original_call,
							   bytes_to_save,
							   original_page_rights,
							   &original_page_rights))
			{
				bRet = FALSE;
				break;
			}

			detour_step++;

			*trampoline_call = pfntrampoline;

			FlushInstructionCache(GetCurrentProcess(),original_call,bytes_to_save);

		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			bRet = FALSE;
		}
	
	} while(FALSE);
	
	if(!bRet)
	{
		if(pfntrampoline)
		{
			cleanup_trampoline_call_x86(pfntrampoline);
			pfntrampoline = NULL;
		}
	}

	return bRet;
}

BOOL remove_hook_x86(PVOID trampoline_call)
{
	BOOL bRet = TRUE;

	PDETOUR_CALL_INFO pdetour_info = NULL;

	DWORD	original_page_rights;

	do 
	{
		pdetour_info = (PDETOUR_CALL_INFO)((PBYTE)trampoline_call + DETOUR_CALL_INFO_OFFSET);

		__try
		{
			if(!VirtualProtect(pdetour_info->original_call_addr,
							   pdetour_info->bytes_copied,
							   PAGE_EXECUTE_READWRITE,
							   &original_page_rights))
			{
				bRet = FALSE;
				break;
			}

			memcpy(pdetour_info->original_call_addr,
				   pdetour_info->original_instructions,
				   pdetour_info->bytes_copied);

			if(!VirtualProtect(pdetour_info->original_call_addr,
							   pdetour_info->bytes_copied,
							   original_page_rights,
							   &original_page_rights))
			{
				bRet = FALSE;
				break;
			}

		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			bRet = FALSE;
		}
		
		cleanup_trampoline_call_x86((TRAMPOLINE_PROC*)trampoline_call);

	} while(FALSE);

	return bRet;
}

static TRAMPOLINE_PROC* init_trampoline_call_x86(PVOID original_call)
{
	TRAMPOLINE_PROC *pfntrampoline_call = NULL;

	do 
	{
		pfntrampoline_call = (TRAMPOLINE_PROC*)VirtualAlloc(NULL,
															sizeof(TRAMPOLINE_PROC),
															MEM_COMMIT,
															PAGE_EXECUTE_READWRITE);
		if(!pfntrampoline_call)
		{
			break;
		}

		memset(pfntrampoline_call,0x90,sizeof(TRAMPOLINE_PROC));

	} while(FALSE);

	return pfntrampoline_call;
}

static void cleanup_trampoline_call_x86(TRAMPOLINE_PROC* trampoline_call)
{
	VirtualFree(trampoline_call,0,MEM_RELEASE);
}

static BOOL build_trampoline_call_from_original_x86(
	PVOID original_call, 
	PVOID trampoline_call,
	DWORD bytes_to_save
	)
{
	BOOL bRet = TRUE;

	BYTE	trampoline_jmp[5]		= {0xe9,0x00,0x00,0x00,0x00};

	PDETOUR_CALL_INFO	pdetour_info = NULL;
	DWORD				original_page_rights;


	do 
	{

		pdetour_info = (PDETOUR_CALL_INFO)((PBYTE)trampoline_call + DETOUR_CALL_INFO_OFFSET);

		__try
		{
			
				
			//
			// setup trampoline jmp
			//
			*(DWORD*)(trampoline_jmp+1) = (DWORD)((BYTE*)original_call - ((BYTE*)trampoline_call + 5));

			if(bytes_to_save != x86BackupInstructions((PBYTE)original_call,bytes_to_save,(PBYTE)trampoline_call))
			{
				bRet = FALSE;
				break;
			}

			//
			// copy trampoline jmp
			//
			memcpy((BYTE*)(trampoline_call) + bytes_to_save,
					trampoline_jmp,
					sizeof(trampoline_jmp));
	
			//
			// fill out detour info
			//
			pdetour_info->original_call_addr = original_call;

			
			memcpy(pdetour_info->original_instructions,
				   (BYTE*)original_call,
				   bytes_to_save);

			pdetour_info->bytes_copied	= bytes_to_save;

			if(!VirtualProtect(trampoline_call,
							   TRAMPOLINE_PROC_SIZE,
							   PAGE_EXECUTE_READ,
							   &original_page_rights))
			{
				bRet = FALSE;
				break;
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			bRet = FALSE;
			break;
		}

	} while(FALSE);

	return bRet;
}

static BOOL inspect_original_call_x86(PVOID original_call)
{
	//
	// does not check much
	// at this moment, just making sure that the API starts with what we want to
	//

	BOOL bRet = TRUE;

	const BYTE	expected_instruction[5] = {0x00,0x00,0x00,0x00,0x00};

	do 
	{

	} while(FALSE);

	return bRet;
}

BOOL install_com_hook_x86(
	PVOID	*call_in_vtbl_addr,
	PVOID	new_call)
{
	BOOL bRet = FALSE;

	DWORD	original_page_rights = 0;

	if(VirtualProtect(call_in_vtbl_addr,
					  sizeof(ULONG_PTR),
					  PAGE_EXECUTE_READWRITE,
					  &original_page_rights))
	{
		*((ULONG_PTR*)call_in_vtbl_addr) = (ULONG_PTR)new_call;

		if(VirtualProtect(call_in_vtbl_addr,
						  sizeof(ULONG_PTR),
						  original_page_rights,
						  &original_page_rights))
		{
			bRet = TRUE;
		}
	}

	return bRet;
}

BOOL remove_com_hook_x86(
	 PVOID	*call_in_vtbl_addr,
	 PVOID	old_call)
{
	BOOL bRet = FALSE;

	DWORD	original_page_rights = 0;

	if(VirtualProtect(call_in_vtbl_addr,
					  sizeof(ULONG_PTR),
					  PAGE_EXECUTE_READWRITE,
					  &original_page_rights))
	{
		*((ULONG_PTR*)call_in_vtbl_addr) = (ULONG_PTR)old_call;

		if(VirtualProtect(call_in_vtbl_addr,
						  sizeof(ULONG_PTR),
						  original_page_rights,
						  &original_page_rights))
		{
			bRet = TRUE;
		}
	}

	return bRet;
}


BOOL install_export_hook_x86(
	PVOID module_base,
	PVOID original_call, 
	PVOID new_call)
{
	BOOL bRet = FALSE;

	IMAGE_NT_HEADERS	*pNtHdr = NULL;

	IMAGE_OPTIONAL_HEADER	*pOptionalHdr = NULL;

	IMAGE_THUNK_DATA		*pImportTbl = NULL;

	ULONG size = 0;
	ULONG count = 0;

	DWORD	original_page_rights = 0;

	do 
	{
		pNtHdr = RtlImageNtHeader(module_base);

		if(pNtHdr == NULL)
		{
			bRet = FALSE;
			break;
		}

		pOptionalHdr = &pNtHdr->OptionalHeader;
		
		pImportTbl = (IMAGE_THUNK_DATA*)RtlImageDirectoryEntryToData(module_base,TRUE,IMAGE_DIRECTORY_ENTRY_IAT,&size);
		
		if(pImportTbl == NULL)
		{
			bRet = FALSE;
			break;
		}

		do 
		{
			if((ULONG_PTR)pImportTbl[count].u1.Function == (ULONG_PTR)original_call)
			{
				if(VirtualProtect(&pImportTbl[count].u1.Function,
								  sizeof(ULONG_PTR),
								  PAGE_EXECUTE_READWRITE,
								  &original_page_rights))
				{
					*((ULONG_PTR*)&pImportTbl[count].u1.Function) = (ULONG_PTR)new_call;

					if(VirtualProtect(&pImportTbl[count].u1.Function,
									  sizeof(ULONG_PTR),
									  original_page_rights,
									  &original_page_rights))
					{
						bRet = TRUE;
					}
				}

				break;
			}

			count++;

		} while (sizeof(IMAGE_THUNK_DATA)*count < size);
			
	} while (FALSE);

	return bRet;
}

BOOL remove_export_hook_x86(
	PVOID module_base,
	PVOID new_call, 
	PVOID original_call)
{
	return install_export_hook_x86(module_base,new_call,original_call);
}

#endif

#ifdef _AMD64_

BOOL __fastcall install_hook_x64(
	PVOID original_call, 
	PVOID *trampoline_call, 
	PVOID new_call)
{
	BOOL bRet = TRUE;

	//
	// 32 bytes should be enough
	//
	BYTE	detour_jmp[32]		= {0xe9,0x00,0x00,0x00,0x00,\
								   0x58,\
								   0x90,0x90,0x90,0x90,\
								   0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,\
								   0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,\
								   0x90,0x90};

	PTRAMPOLINE_PROC	pfntrampoline = NULL;
	PVOID				staging_jmp = NULL;

	DWORD	detour_step = 0;
	DWORD	original_page_rights;
	DWORD	bytes_to_save = 0;

	do 
	{
		if(!inspect_original_call_x64(original_call))
		{
			bRet = FALSE;
			break;
		}

		pfntrampoline = init_trampoline_call_x64(original_call);

		if(!pfntrampoline)
		{
			bRet = FALSE;
			break;
		}

		detour_step++;

		staging_jmp = build_staging_jmp_x64(original_call,new_call);

		if(!staging_jmp)
		{
			bRet = FALSE;
			break;
		}

		detour_step++;

		bytes_to_save = 0;

		do 
		{
			bytes_to_save += x64GetCurrentInstructionLength((PBYTE)original_call + bytes_to_save);

		} while(bytes_to_save < 6);

		if(!build_trampoline_call_from_original_x64(original_call,pfntrampoline,bytes_to_save,staging_jmp))
		{
			bRet = FALSE;
			break;
		}

		
		//
		// copy detour jmp
		//
		__try
		{

			//
			// build detour jmp
			//
			*(DWORD*)(detour_jmp+1) = (DWORD)((BYTE*)staging_jmp - ((BYTE*)original_call + 5));

			if(!VirtualProtect(original_call,
							   bytes_to_save,
							   PAGE_EXECUTE_READWRITE,
							   &original_page_rights))
			{
				bRet = FALSE;
				break;
			}

			memcpy(original_call,
				   detour_jmp,
				   max(6,bytes_to_save));

			if(!VirtualProtect(original_call,
							   bytes_to_save,
							   original_page_rights,
							   &original_page_rights))
			{
				bRet = FALSE;
				break;
			}

			detour_step++;

			*trampoline_call = pfntrampoline;

			FlushInstructionCache(GetCurrentProcess(),original_call,bytes_to_save);

		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			bRet = FALSE;
		}

	} while(FALSE);

	if(!bRet)
	{
		if(pfntrampoline)
		{
			cleanup_trampoline_call_x64(pfntrampoline);
			pfntrampoline = NULL;
		}
	}

	return bRet;

}

static TRAMPOLINE_PROC* init_trampoline_call_x64(PVOID original_call)
{
	TRAMPOLINE_PROC *pfntrampoline_call = NULL;

	do 
	{
		pfntrampoline_call = (TRAMPOLINE_PROC*)VirtualAlloc(NULL,
															sizeof(TRAMPOLINE_PROC),
															MEM_COMMIT,
															PAGE_EXECUTE_READWRITE);
		if(!pfntrampoline_call)
		{
			break;
		}

		memset(pfntrampoline_call,0x90,sizeof(TRAMPOLINE_PROC));

	} while(FALSE);

	return pfntrampoline_call;
}

BOOL remove_hook_x64(PVOID trampoline_call)
{
	BOOL bRet = TRUE;

	PDETOUR_CALL_INFO pdetour_info = NULL;

	DWORD	original_page_rights;

	do 
	{
		pdetour_info = (PDETOUR_CALL_INFO)((PBYTE)trampoline_call + DETOUR_CALL_INFO_OFFSET);

		__try
		{
			if(!VirtualProtect(pdetour_info->original_call_addr,
							   pdetour_info->bytes_copied,
							   PAGE_EXECUTE_READWRITE,
							   &original_page_rights))
			{
				bRet = FALSE;
				break;
			}

			memcpy(pdetour_info->original_call_addr,
				   pdetour_info->original_instructions,
				   pdetour_info->bytes_copied);

			if(!VirtualProtect(pdetour_info->original_call_addr,
							   pdetour_info->bytes_copied,
							   original_page_rights,
							   &original_page_rights))
			{
				bRet = FALSE;
				break;
			}

		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			bRet = FALSE;
		}

		cleanup_trampoline_call_x64((TRAMPOLINE_PROC*)trampoline_call);

	} while(FALSE);

	return bRet;
}

static void cleanup_trampoline_call_x64(TRAMPOLINE_PROC* trampoline_call)
{
	PDETOUR_CALL_INFO pdetour_info = NULL;

	pdetour_info = (PDETOUR_CALL_INFO)((PBYTE)trampoline_call + DETOUR_CALL_INFO_OFFSET);
	
	//if(pdetour_info->staging_jmp)
	//{
	//	VirtualFree(pdetour_info->staging_jmp,0,MEM_RELEASE);
	//}

	VirtualFree(trampoline_call,0,MEM_RELEASE);
}

static BOOL build_trampoline_call_from_original_x64(
	PVOID original_call, 
	PVOID trampoline_call,
	DWORD bytes_to_save,
	PVOID staging_jmp)
{
	BOOL bRet = TRUE;

	BYTE	trampoline_jmp[13]	= {0x50,\
								   0x48,0xb8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
								   0xff,0xe0};


	PDETOUR_CALL_INFO	pdetour_info = NULL;
	DWORD				original_page_rights;

	BYTE	first_instruction;
	BYTE	modrm;

	do 
	{

		pdetour_info = (PDETOUR_CALL_INFO)((PBYTE)trampoline_call + DETOUR_CALL_INFO_OFFSET);

		__try
		{

			first_instruction = *(BYTE*)original_call;

			if(first_instruction == 0xe9)
			{
				//
				// I'm going to add code later
				//
				bRet = FALSE;
				break;
			}
			else if(first_instruction == 0xeb)
			{
				//
				// I'm going to add code later
				//
				bRet = FALSE;
				break;
			}
			else if(first_instruction == 0xff)
			{
				modrm = *((BYTE*)original_call + 1);

				if(modrm == 0x25)
				{
					bRet = FALSE;
					break;
				}

				//
				// setup trampoline jmp
				//
				*(ULONG_PTR*)(trampoline_jmp+3) = (ULONG_PTR)((UCHAR*)original_call + 5);

				//
				// setup trampoline call
				//
				memcpy(trampoline_call,
					   original_call,
					   bytes_to_save);
				//
				// copy trampoline jmp
				//
				memcpy((BYTE*)(trampoline_call) + bytes_to_save,
					   trampoline_jmp,
					   sizeof(trampoline_jmp));

			}
			//else if(first_instruction == 0xea)
			//{
			//	//
			//	// I'm going to add code later
			//	//
			//	DebugBreak();
			//}
			else 
			{
				// not jump instruction

				//
				// setup trampoline jmp
				//
				*(ULONG_PTR*)(trampoline_jmp+3) = (ULONG_PTR)((UCHAR*)original_call + 5);

				//
				// setup trampoline call
				//
				memcpy(trampoline_call,
					   original_call,
					   bytes_to_save);
				//
				// copy trampoline jmp
				//
				memcpy((BYTE*)(trampoline_call) + bytes_to_save,
					   trampoline_jmp,
					   sizeof(trampoline_jmp));

			}

			//
			// fill out detour info
			//
			pdetour_info->original_call_addr = original_call;


			memcpy(pdetour_info->original_instructions,
				   (BYTE*)original_call,
				   bytes_to_save);

			pdetour_info->bytes_copied	= bytes_to_save;
			pdetour_info->staging_jmp	= staging_jmp;

			if(!VirtualProtect(trampoline_call,
							   TRAMPOLINE_PROC_SIZE,
							   PAGE_EXECUTE_READ,
							   &original_page_rights))
			{
				bRet = FALSE;
				break;
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			bRet = FALSE;
			break;
		}

	} while(FALSE);

	return bRet;
}

static BOOL inspect_original_call_x64(PVOID original_call)
{
	//
	// does not check much
	// at this moment, just making sure that the API starts with what we want to
	//

	BOOL bRet = TRUE;

	const BYTE	expected_instruction[12] = {0x90,0x90,0x90,0x90,0x90,0x90,\
											0x90,0x90,0x90,0x90,0x90,0x90};

	do 
	{

	} while(FALSE);

	return bRet;
}

BOOL __fastcall install_com_hook_x64(
	PVOID	*call_in_vtbl_addr,
	PVOID	new_call)
{
	BOOL bRet = FALSE;

	DWORD	original_page_rights = 0;

	if(VirtualProtect(call_in_vtbl_addr,
					  sizeof(ULONG_PTR),
					  PAGE_EXECUTE_READWRITE,
					  &original_page_rights))
	{
		*((ULONG_PTR*)call_in_vtbl_addr) = (ULONG_PTR)new_call;

		if(VirtualProtect(call_in_vtbl_addr,
						  sizeof(ULONG_PTR),
						  original_page_rights,
						  &original_page_rights))
		{
			bRet = TRUE;
		}
	}

	return bRet;
}

BOOL __fastcall remove_com_hook_x64(
	PVOID	*call_in_vtbl_addr,
	PVOID	old_call)
{
	BOOL bRet = FALSE;

	DWORD	original_page_rights = 0;

	if(VirtualProtect(call_in_vtbl_addr,
					  sizeof(ULONG_PTR),
					  PAGE_EXECUTE_READWRITE,
					  &original_page_rights))
	{
		*((ULONG_PTR*)call_in_vtbl_addr) = (ULONG_PTR)old_call;

		if(VirtualProtect(call_in_vtbl_addr,
						  sizeof(ULONG_PTR),
						  original_page_rights,
						  &original_page_rights))
		{
			bRet = TRUE;
		}
	}

	return bRet;
}


BOOL __fastcall install_export_hook_x64(
	PVOID module_base,
	PVOID original_call, 
	PVOID new_call)
{
	BOOL bRet = FALSE;

	IMAGE_NT_HEADERS	*pNtHdr = NULL;

	IMAGE_OPTIONAL_HEADER	*pOptionalHdr = NULL;

	IMAGE_THUNK_DATA		*pImportTbl = NULL;

	ULONG size = 0;
	ULONG count = 0;

	DWORD	original_page_rights = 0;

	do 
	{
		pNtHdr = RtlImageNtHeader(module_base);

		if(pNtHdr == NULL)
		{
			bRet = FALSE;
			break;
		}

		pOptionalHdr = &pNtHdr->OptionalHeader;

		pImportTbl = (IMAGE_THUNK_DATA*)RtlImageDirectoryEntryToData(module_base,TRUE,IMAGE_DIRECTORY_ENTRY_IAT,&size);

		if(pImportTbl == NULL)
		{
			bRet = FALSE;
			break;
		}

		do 
		{
			if((ULONG_PTR)pImportTbl[count].u1.Function == (ULONG_PTR)original_call)
			{
				if(VirtualProtect(&pImportTbl[count].u1.Function,
								  sizeof(ULONG_PTR),
								  PAGE_EXECUTE_READWRITE,
								  &original_page_rights))
				{
					*((ULONG_PTR*)&pImportTbl[count].u1.Function) = (ULONG_PTR)new_call;

					if(VirtualProtect(&pImportTbl[count].u1.Function,
									  sizeof(ULONG_PTR),
									  original_page_rights,
									  &original_page_rights))
					{
						bRet = TRUE;
					}
				}

				break;
			}

			count++;

		} while (sizeof(IMAGE_THUNK_DATA)*count < size);

	} while (FALSE);

	return bRet;
}

BOOL __fastcall remove_export_hook_x64(
	PVOID module_base,
	PVOID new_call, 
	PVOID original_call)
{
	return install_export_hook_x64(module_base,new_call,original_call);
}

static BOOL build_trampoline_call_from_original_ex_x64(
	PVOID original_call, 
	PVOID trampoline_call,
	DWORD bytes_to_save,
	PVOID staging_jmp
	)
{
	BOOL bRet = TRUE;

	BYTE	trampoline_jmp[13]	= {0x50,\
								   0x48,0xb8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
								   0xff,0xe0};


	PDETOUR_CALL_INFO	pdetour_info = NULL;
	DWORD				original_page_rights;

	BYTE	first_instruction;
	BYTE	modrm;

	do 
	{

		pdetour_info = (PDETOUR_CALL_INFO)((PBYTE)trampoline_call + DETOUR_CALL_INFO_OFFSET);

		__try
		{

			first_instruction = *(BYTE*)original_call;

			if(first_instruction == 0xe9)
			{
				//
				// I'm going to add code later
				//
				bRet = FALSE;
				break;
			}
			else if(first_instruction == 0xeb)
			{
				//
				// I'm going to add code later
				//
				bRet = FALSE;
				break;
			}
			else if(first_instruction == 0xff)
			{
				modrm = *((BYTE*)original_call + 1);

				if(modrm == 0x25)
				{
					bRet = FALSE;
					break;
				}

				//
				// setup trampoline jmp
				//
				*(ULONG_PTR*)(trampoline_jmp+3) = (ULONG_PTR)((UCHAR*)original_call + 2);

				//
				// setup trampoline call
				//
				memcpy(trampoline_call,
					   original_call,
					   bytes_to_save);
				//
				// copy trampoline jmp
				//
				memcpy((BYTE*)(trampoline_call) + bytes_to_save,
					   trampoline_jmp,
					   sizeof(trampoline_jmp));

			}
			//else if(first_instruction == 0xea)
			//{
			//	//
			//	// I'm going to add code later
			//	//
			//	DebugBreak();
			//}
			else 
			{
				// not jump instruction

				//
				// setup trampoline jmp
				//
				*(ULONG_PTR*)(trampoline_jmp+3) = (ULONG_PTR)((UCHAR*)original_call + 2);

				//
				// setup trampoline call
				//
				memcpy(trampoline_call,
					   original_call,
					   bytes_to_save);
				//
				// copy trampoline jmp
				//
				memcpy((BYTE*)(trampoline_call) + bytes_to_save,
					   trampoline_jmp,
					   sizeof(trampoline_jmp));

			}

			//
			// fill out detour info
			//
			pdetour_info->original_call_addr = original_call;


			memcpy(pdetour_info->original_instructions,
				   (BYTE*)original_call,
				   bytes_to_save);

			pdetour_info->bytes_copied	= bytes_to_save;
			pdetour_info->staging_jmp	= staging_jmp;

			if(!VirtualProtect(trampoline_call,
							   TRAMPOLINE_PROC_SIZE,
							   PAGE_EXECUTE_READ,
							   &original_page_rights))
			{
				bRet = FALSE;
				break;
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			bRet = FALSE;
			break;
		}

	} while(FALSE);

	return bRet;
}

static BOOL inspect_original_call_ex_x64(PVOID original_call)
{
	//
	// does not check much
	// at this moment, just making sure that the API starts with what we want to
	//

	BOOL bRet = TRUE;

	const BYTE	expected_instruction[5] = {0x90,0x90,0x90,0x90,0x90};

	do 
	{
		if(memcmp((BYTE*)original_call - sizeof(expected_instruction),
				  expected_instruction,
				  sizeof(expected_instruction)) != 0)
		{
			bRet = FALSE;
			break;
		}

	} while(FALSE);

	return bRet;
}

BOOL install_hook_ex_x64(
						 PVOID original_call,
						 PVOID *trampoline_call,
						 PVOID new_call)
{
	BOOL bRet = TRUE;


	BYTE	detour_jmp[] = {0xe9,0x00,0x00,0x00,0x00};

	//
	// 32 bytes should be enough
	//
	const BYTE	short_jmp[32] = {0xeb,0xf9,0x58,0x90,0x90,0x90,0x90,0x90,\
								 0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,\
								 0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,\
								 0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90};

	PTRAMPOLINE_PROC	pfntrampoline = NULL;
	PVOID				staging_jmp = NULL;

	DWORD	detour_step = 0;
	DWORD	original_page_rights;
	DWORD	bytes_to_save = 0;

	do 
	{
		original_call = reparse_origial_call_x64(original_call);

		if(!inspect_original_call_ex_x64(original_call))
		{
			bRet = FALSE;
			break;
		}

		pfntrampoline = init_trampoline_call_x64(original_call);

		if(!pfntrampoline)
		{
			bRet = FALSE;
			break;
		}

		detour_step++;

		staging_jmp = build_staging_jmp_x64(original_call,new_call);

		if(!staging_jmp)
		{
			bRet = FALSE;
			break;
		}

		bytes_to_save = 0;

		do 
		{
			bytes_to_save += x64GetCurrentInstructionLength((PBYTE)original_call + bytes_to_save);

		} while(bytes_to_save < 3);

		if(!build_trampoline_call_from_original_ex_x64(original_call,pfntrampoline,bytes_to_save,staging_jmp))
		{
			bRet = FALSE;
			break;
		}

		//
		// copy detour jmp
		//
		__try
		{

			//
			// build detour jmp
			//
			*(DWORD*)(detour_jmp+1) = (DWORD)((BYTE*)staging_jmp - ((BYTE*)original_call - sizeof(detour_jmp) + 5));

			if(!VirtualProtect((BYTE*)original_call - sizeof(detour_jmp),
							   sizeof(detour_jmp),
							   PAGE_EXECUTE_READWRITE,
							   &original_page_rights))
			{
				bRet = FALSE;
				break;
			}

			memcpy((BYTE*)original_call - sizeof(detour_jmp),
				   detour_jmp,
				   sizeof(detour_jmp));

			if(!VirtualProtect((BYTE*)original_call - sizeof(detour_jmp),
							   sizeof(detour_jmp),
							   original_page_rights,
							   &original_page_rights))
			{
				bRet = FALSE;
				break;
			}

			//
			// setup short jmp
			//
			detour_step++;

			if(!VirtualProtect(original_call,
							   bytes_to_save,
							   PAGE_EXECUTE_READWRITE,
							   &original_page_rights))
			{
				bRet = FALSE;
				break;
			}

			memcpy(original_call,
				   short_jmp,
				   max(3,bytes_to_save));

			if(!VirtualProtect(original_call,
							   bytes_to_save,
							   original_page_rights,
							   &original_page_rights))
			{
				bRet = FALSE;
				break;
			}

			detour_step++;

			*trampoline_call = pfntrampoline;

			FlushInstructionCache(GetCurrentProcess(),
								  (BYTE*)original_call - sizeof(detour_jmp),
								  sizeof(detour_jmp) + bytes_to_save);

		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			bRet = FALSE;
		}

	} while(FALSE);

	if(!bRet)
	{
		if(pfntrampoline)
		{
			cleanup_trampoline_call_x64(pfntrampoline);
			pfntrampoline = NULL;
		}
	}

	return bRet;
}

BOOL remove_hook_ex_x64(PVOID trampoline_call)
{
	BOOL bRet = TRUE;

	PDETOUR_CALL_INFO pdetour_info = NULL;

	DWORD	original_page_rights;

	const BYTE	expected_instruction[5] = {0x90,0x90,0x90,0x90,0x90};

	do 
	{
		pdetour_info = (PDETOUR_CALL_INFO)((PBYTE)trampoline_call + DETOUR_CALL_INFO_OFFSET);

		__try
		{
			if(!VirtualProtect(pdetour_info->original_call_addr,
							   pdetour_info->bytes_copied,
							   PAGE_EXECUTE_READWRITE,
							   &original_page_rights))
			{
				bRet = FALSE;
				break;
			}

			memcpy(pdetour_info->original_call_addr,
				   pdetour_info->original_instructions,
				   pdetour_info->bytes_copied);

			if(!VirtualProtect(pdetour_info->original_call_addr,
							   pdetour_info->bytes_copied,
							   original_page_rights,
							   &original_page_rights))
			{
				bRet = FALSE;
				break;
			}

			if(!VirtualProtect((BYTE*)pdetour_info->original_call_addr - sizeof(expected_instruction),
							   sizeof(expected_instruction),
							   PAGE_EXECUTE_READWRITE,
							   &original_page_rights))
			{
				bRet = FALSE;
				break;
			}

			memcpy((BYTE*)pdetour_info->original_call_addr - sizeof(expected_instruction),
				   expected_instruction,
				   sizeof(expected_instruction));

			if(!VirtualProtect((BYTE*)pdetour_info->original_call_addr - sizeof(expected_instruction),
							   sizeof(expected_instruction),
							   original_page_rights,
							   &original_page_rights))
			{
				bRet = FALSE;
				break;
			}

		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			bRet = FALSE;
		}

		cleanup_trampoline_call_x64((TRAMPOLINE_PROC*)trampoline_call);

	} while(FALSE);

	return bRet;
}

static PVOID find_free_memory_region(PVOID pbase)
{
	PVOID p = NULL;
	
	do 
	{
		pbase = (PVOID)(((ULONG_PTR)pbase + 4095) & (~4095));

		p = search_free_memory_region_backward_x64(pbase);

		if(p)
		{
			break;
		}

		p = search_free_memory_region_forward_x64(pbase);

	} while (FALSE);
	
	return p;
}

static PVOID search_free_memory_region_forward_x64(PVOID pbase)
{
	PVOID p = NULL;

	p = VirtualAlloc(NULL,4096,MEM_RESERVE,PAGE_NOACCESS);
	
	if((ULONG_PTR)p > (ULONG_PTR)pbase)
	{
		if(((ULONG_PTR)p - (ULONG_PTR)pbase) > 0x7fffffff)
		{
			p = NULL;
		}
	}
	else
	{
		if((ULONG_PTR)pbase - (ULONG_PTR)p > 0x7fffffff)
		{
			p = NULL;
		}
	}

	return p;
}

static PVOID search_free_memory_region_backward_x64(PVOID pbase)
{
	PVOID p = NULL;
	PVOID v = NULL;

	MEMORY_BASIC_INFORMATION mi;

	SIZE_T offset = 0;

	p = pbase;

	memset(&mi,0,sizeof(mi));

	do 
	{
		if(!VirtualQuery(p,&mi,sizeof(mi)))
		{
			p = NULL;
			break;
		}

		if(mi.State == MEM_FREE)
		{
			p = mi.BaseAddress;

			if(((ULONG_PTR)pbase - (ULONG_PTR)p) > 0x7fffffff)
			{
				p = NULL;
				break;
			}

			if((ULONG_PTR)p  >= 0x70000000 &&
			   (ULONG_PTR)p  <= 0x80000000)
			{
				p = (PVOID)((UCHAR*)mi.AllocationBase - 1);
				continue;
			}

			v = VirtualAlloc(p,4096,MEM_RESERVE,PAGE_NOACCESS);

			if(v)
				break;
			else
			{
				p = (PVOID)((UCHAR*)mi.BaseAddress - 1);
				continue;
			}
		}

		if(mi.AllocationBase == 0)
		{
			p = NULL;
			break;
		}

		p = (PVOID)((UCHAR*)mi.AllocationBase - 1);

	} while (TRUE);

	return p;
}

static PVOID build_staging_jmp_x64(PVOID original_call, PVOID new_call)
{
	PVOID p = NULL;
	DWORD oldprotection = PAGE_EXECUTE_READWRITE;

	BYTE staging_jmp[12] = {0x48,0xb8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
							0xff,0xe0};
	do 
	{
		p = find_free_memory_region(original_call);

		if(p)
		{
			p = VirtualAlloc(p,
							 sizeof(staging_jmp),
							 MEM_COMMIT,
							 PAGE_EXECUTE_READWRITE);

			if(!p)
			{
				break;
			}

			*(ULONG_PTR*)(staging_jmp + 2) = (ULONG_PTR)new_call;

			memcpy(p,staging_jmp,sizeof(staging_jmp));

			VirtualProtect(p,
						   sizeof(staging_jmp),
						   PAGE_EXECUTE_READ,
						   &oldprotection);

		}
		else
		{
			break;
		}

	} while (FALSE);

	return p;
}

static PVOID reparse_origial_call_x64(PVOID original_call)
{
	PVOID	fn_original_call = original_call;

	BYTE	first_instruction;

	do 
	{

		__try
		{

			first_instruction = *(BYTE*)original_call;

			if(first_instruction == 0xe9)
			{
				fn_original_call = (PVOID)((BYTE*)original_call + ((*(DWORD*)((BYTE*)original_call + 1)) + 5));
				fn_original_call = (PVOID)(((ULONG_PTR)original_call & 0xffffffff00000000) + ((ULONG_PTR)fn_original_call & 0x00000000ffffffff));
				break;
			}
			else if(first_instruction == 0xeb)
			{
				//
				// I'm going to add code later
				//
				fn_original_call = original_call;
				break;
			}
			//else if(first_instruction == 0xff)
			//{
			//	//
			//	// I'm going to add code later
			//	//
			//	DebugBreak();
			//}
			//else if(first_instruction == 0xea)
			//{
			//	//
			//	// I'm going to add code later
			//	//
			//	DebugBreak();
			//}
			else 
			{
				// not jump instruction
				fn_original_call = original_call;
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			fn_original_call = original_call;
			break;
		}

	} while(FALSE);

	return fn_original_call;
}

#endif	//_AMD64_


static PIMAGE_NT_HEADERS RtlImageNtHeader (IN PVOID BaseAddress)
{
	PIMAGE_NT_HEADERS NtHeader;
	PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)BaseAddress;

	if(DosHeader && DosHeader->e_magic == IMAGE_DOS_SIGNATURE)
	{
		NtHeader = (PIMAGE_NT_HEADERS)((ULONG_PTR)BaseAddress + DosHeader->e_lfanew);

		if(NtHeader->Signature == IMAGE_NT_SIGNATURE)
			return NtHeader;
	}

	return NULL;
}

static PVOID RtlImageDirectoryEntryToData(
	PVOID	BaseAddress,
	BOOLEAN MappedAsImage,
	USHORT	Directory,
	PULONG	Size)
{
	PIMAGE_NT_HEADERS NtHeader;
	ULONG Va;

	if ((ULONG_PTR)BaseAddress & 1)
	{
		BaseAddress = (PVOID)((ULONG_PTR)BaseAddress & ~1);
		MappedAsImage = FALSE;
	}


	NtHeader = RtlImageNtHeader (BaseAddress);
	if (NtHeader == NULL)
		return NULL;

	if (Directory >= NtHeader->OptionalHeader.NumberOfRvaAndSizes)
		return NULL;

	Va = NtHeader->OptionalHeader.DataDirectory[Directory].VirtualAddress;
	if (Va == 0)
		return NULL;

	*Size = NtHeader->OptionalHeader.DataDirectory[Directory].Size;

	if (MappedAsImage || Va < NtHeader->OptionalHeader.SizeOfHeaders)
		return (PVOID)((ULONG_PTR)BaseAddress + Va);

	return RtlImageRvaToVa (NtHeader, BaseAddress, Va, NULL);
}

static PIMAGE_SECTION_HEADER RtlImageRvaToSection (
	PIMAGE_NT_HEADERS       NtHeader,
	PVOID                   BaseAddress,
	ULONG                   Rva
	)
{
	PIMAGE_SECTION_HEADER Section;
	ULONG Va;
	ULONG Count;

	Count = NtHeader->FileHeader.NumberOfSections;
	Section = (PIMAGE_SECTION_HEADER)((ULONG_PTR)&NtHeader->OptionalHeader + NtHeader->FileHeader.SizeOfOptionalHeader);

	while (Count)
	{
		Va = Section->VirtualAddress;
		if ((Va <= Rva) &&
			(Rva < Va + Section->SizeOfRawData))
			return Section;
		Section++;
	}
	return NULL;
}


static PVOID RtlImageRvaToVa (
	PIMAGE_NT_HEADERS       NtHeader,
	PVOID                   BaseAddress,
	ULONG                   Rva,
	PIMAGE_SECTION_HEADER   *SectionHeader
	)
{
	PIMAGE_SECTION_HEADER Section = NULL;

	if (SectionHeader)
		Section = *SectionHeader;

	if (Section == NULL ||
		Rva < Section->VirtualAddress ||
		Rva >= Section->VirtualAddress + Section->SizeOfRawData)
	{
		Section = RtlImageRvaToSection (NtHeader, BaseAddress, Rva);

		if (Section == NULL)
			return 0;

		if (SectionHeader)
			*SectionHeader = Section;
	}

	return (PVOID)((ULONG_PTR)BaseAddress +	Rva + Section->PointerToRawData - (ULONG_PTR)Section->VirtualAddress);
}


