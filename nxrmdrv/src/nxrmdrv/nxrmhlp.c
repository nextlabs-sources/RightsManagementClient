#include "precomp.h"
#include "nxrmdrv.h"
#include "nxrmdrvdef.h"

#define krnl_name		"NTOSKRNL.EXE"
#define krnlpae_name	"NTKRNLPA.EXE"
#define krnlmp_name		"NTKRNLMP.EXE"	
#define krnlmppae_name	"NTKRPAMP.EXE"

#define SystemModuleInformation	11

extern NXRM_GLOBAL_DATA Global;

typedef struct _NXRM_MODULE_INFORMATION
{
	ULONG_PTR	Reserved[2];
	
	PVOID		Base;
	
	ULONG		Size;
	
	ULONG		Flags;
	
	USHORT		Index;
	
	USHORT		Unknown;
	
	USHORT		LoadCount;
	
	USHORT		ModuleNameOffset;
	
	CHAR		ImageName[256];

} NXRM_MODULE_INFORMATION, *PNXRM_MODULE_INFORMATION;

typedef struct _NXRM_MODULE_TABLE 
{
	SIZE_T						NumberOfModules;
	NXRM_MODULE_INFORMATION		Module[1];

} NXRM_MODULE_TABLE, *PNXRM_MODULE_TABLE;

extern NTSTATUS ZwQuerySystemInformation(
	ULONG	SystemInformationClass,
	PVOID	SystemInformation,
	ULONG	SystemInformationLength,
	PULONG	ReturnLength
	);

static USHORT NameToOrdinal(
	IN PCHAR Name,
	IN PVOID DllBase,
	IN ULONG NumberOfNames,
	IN PULONG NameTable,
	IN PUSHORT OrdinalTable
	);

static PIMAGE_NT_HEADERS RtlImageNtHeader(
	IN PVOID BaseAddress
	);

static PVOID RtlImageDirectoryEntryToData(
	PVOID BaseAddress,
	BOOLEAN MappedAsImage,
	USHORT Directory,
	PULONG Size
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

#ifdef _AMD64_

static PIMAGE_NT_HEADERS32 Wow64RtlImageNtHeader(
	IN PVOID BaseAddress
	);

static PIMAGE_NT_HEADERS32 Wow64RtlImageNtHeader(
	IN PVOID BaseAddress
	);

PVOID Wow64RtlImageDirectoryEntryToData(
	PVOID BaseAddress,
	BOOLEAN MappedAsImage,
	USHORT Directory,
	PULONG Size
	);

static PIMAGE_SECTION_HEADER Wow64RtlImageRvaToSection(
	PIMAGE_NT_HEADERS32     NtHeader,
	PVOID                   BaseAddress,
	ULONG                   Rva
	);

static PVOID Wow64RtlImageRvaToVa (
	PIMAGE_NT_HEADERS32     NtHeader,
	PVOID                   BaseAddress,
	ULONG                   Rva,
	PIMAGE_SECTION_HEADER   *SectionHeader
	);

#endif

NTSTATUS nxrmLookupEntryPoint(
	IN PVOID DllBase,
	IN PCHAR Name,
	OUT PVOID *EntryPoint
	);

NTSTATUS nxrmLookupWow64EntryPoint(
	IN PVOID DllBase,
	IN PCHAR Name,
	OUT PVOID *EntryPoint
	);

PVOID nxrmFindLoadLibraryWAddress(PVOID Kernel32DllBase)
{
	NTSTATUS Status = STATUS_SUCCESS;

	PVOID LoadLibraryWAddr = NULL;	

	do 
	{
		Status = nxrmLookupEntryPoint(Kernel32DllBase,"LoadLibraryW",&LoadLibraryWAddr);

		if(!NT_SUCCESS(Status))
		{
			LoadLibraryWAddr = NULL;
			break;
		}

	} while(FALSE);

	return LoadLibraryWAddr;
}

PVOID nxrmFindVirtualProtectAddress(PVOID Kernel32DllBase)
{
	NTSTATUS Status = STATUS_SUCCESS;

	PVOID VirtualProtectAddr = NULL;

	do 
	{
		Status = nxrmLookupEntryPoint(Kernel32DllBase,"VirtualProtect",&VirtualProtectAddr);

		if(!NT_SUCCESS(Status))
		{
			VirtualProtectAddr = NULL;
			break;
		}

	} while(FALSE);

	return VirtualProtectAddr;
}

PVOID nxrmFindRtlExitUserProcessAddress(PVOID NtdllBase)
{
	NTSTATUS Status = STATUS_SUCCESS;

	PVOID RtlExitUserProcessAddr = NULL;

	do 
	{
		Status = nxrmLookupEntryPoint(NtdllBase, "RtlExitUserProcess", &RtlExitUserProcessAddr);

		if (!NT_SUCCESS(Status))
		{
			RtlExitUserProcessAddr = NULL;
			break;
		}

	} while (FALSE);

	return RtlExitUserProcessAddr;
}

#ifdef _AMD64_

PVOID nxrmFindWow64LoadLibraryWAddress(PVOID Kernel32DllBase)
{
	NTSTATUS Status = STATUS_SUCCESS;

	PVOID LoadLibraryWAddr = NULL;	

	do 
	{
		Status = nxrmLookupWow64EntryPoint(Kernel32DllBase,"LoadLibraryW",&LoadLibraryWAddr);

		if(!NT_SUCCESS(Status))
		{
			LoadLibraryWAddr = NULL;
			break;
		}

	} while(FALSE);

	return LoadLibraryWAddr;
}

PVOID nxrmFindWow64VirtualProtectAddress(PVOID Kernel32DllBase)
{
	NTSTATUS Status = STATUS_SUCCESS;

	PVOID VirtualProtectAddr = NULL;

	do 
	{
		Status = nxrmLookupWow64EntryPoint(Kernel32DllBase,"VirtualProtect",&VirtualProtectAddr);

		if(!NT_SUCCESS(Status))
		{
			VirtualProtectAddr = NULL;
			break;
		}

	} while(FALSE);

	return VirtualProtectAddr;
}

PVOID nxrmFindWow64MemcpyAddress(IN PVOID NtDllBase)
{
	NTSTATUS Status = STATUS_SUCCESS;

	PVOID MemcpyAddr = NULL;

	do 
	{
		Status = nxrmLookupWow64EntryPoint(NtDllBase,"memcpy",&MemcpyAddr);

		if(!NT_SUCCESS(Status))
		{
			MemcpyAddr = NULL;
			break;
		}

	} while(FALSE);

	return MemcpyAddr;
}

PVOID nxrmFindWow64RtlExitUserThreadAddress(IN PVOID NtDllBase)
{
	NTSTATUS Status = STATUS_SUCCESS;

	PVOID RtlExitUserThreadAddr = NULL;

	do 
	{
		Status = nxrmLookupWow64EntryPoint(NtDllBase,"RtlExitUserThread",&RtlExitUserThreadAddr);

		if(!NT_SUCCESS(Status))
		{
			RtlExitUserThreadAddr = NULL;
			break;
		}

	} while(FALSE);

	return RtlExitUserThreadAddr;
}

PVOID nxrmFindWow64CorExeMainAddress(IN PVOID MscoreeBase)
{
	NTSTATUS Status = STATUS_SUCCESS;

	PVOID _CorExeMain = NULL;

	do 
	{
		Status = nxrmLookupWow64EntryPoint(MscoreeBase,"_CorExeMain",&_CorExeMain);

		if(!NT_SUCCESS(Status))
		{
			_CorExeMain = NULL;
			break;
		}

	} while(FALSE);

	return _CorExeMain;
}

#endif

PVOID nxrmFindKernelModuleBaseAddress(
	IN STRING *ModuleName
	)
{
	PVOID BaseAddr = NULL;

	NTSTATUS	status = STATUS_SUCCESS;

	ULONG lengthret = 0;
	ULONG length	= 0;

	NXRM_MODULE_TABLE		*pModuleTable = NULL;
	NXRM_MODULE_INFORMATION	*pModule = NULL;

	STRING	strModuleName;

	ULONG i = 0;

	CHAR	*p = NULL;

	do 
	{
		lengthret = 256 * 1024;	// 256k

		do 
		{
			length = (lengthret + 4095) & (~4095);

			if(pModuleTable)
			{
				ExFreePoolWithTag(pModuleTable,NXRMTAG);
				pModuleTable = NULL;
			}

			pModuleTable = ExAllocatePoolWithTag(PagedPool,
												 length,
												 NXRMTAG);

			if(pModuleTable == NULL)
			{
				status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

			RtlZeroMemory(pModuleTable,length);

			status = ZwQuerySystemInformation(SystemModuleInformation,
											  pModuleTable,
											  length,
											  &lengthret);

		} while(status == STATUS_INFO_LENGTH_MISMATCH && length <=40*1024*1024);

		if(status != STATUS_SUCCESS)
		{
			break;
		}

		for(i = 0; i< pModuleTable->NumberOfModules; i++)
		{
			pModule = &pModuleTable->Module[i];

			p = NULL;

			p = strrchr(pModule->ImageName,'\\');

			if(p)
			{
				p++;

				RtlInitString(&strModuleName,p);

				if(0 == RtlCompareString(&strModuleName,ModuleName,TRUE))
				{
					break;
				}
				else
				{
					pModule = NULL;
				}
			}
		}

		if(pModule)
		{
			BaseAddr = pModule->Base;
		}

	} while(FALSE);

	if(pModuleTable)
	{
		ExFreePoolWithTag(pModuleTable, NXRMTAG);
		pModuleTable = NULL;
	}

	return BaseAddr;
}

PVOID nxrmFindMemcpyAddress(IN PVOID NtDllBase)
{
	NTSTATUS Status = STATUS_SUCCESS;

	PVOID MemcpyAddr = NULL;

	do 
	{
#ifdef _AMD64_		
		Status = nxrmLookupEntryPoint(NtDllBase,"RtlCopyMemory",&MemcpyAddr);
#else
		Status = nxrmLookupEntryPoint(NtDllBase,"memcpy",&MemcpyAddr);
#endif
		if(!NT_SUCCESS(Status))
		{
			MemcpyAddr = NULL;
			break;
		}

	} while(FALSE);

	return MemcpyAddr;
}

PVOID nxrmFindRtlExitUserThreadAddress(IN PVOID NtDllBase)
{
	NTSTATUS Status = STATUS_SUCCESS;

	PVOID RtlExitUserThreadAddr = NULL;

	do 
	{
		Status = nxrmLookupEntryPoint(NtDllBase,"RtlExitUserThread",&RtlExitUserThreadAddr);

		if(!NT_SUCCESS(Status))
		{
			RtlExitUserThreadAddr = NULL;
			break;
		}

	} while(FALSE);

	return RtlExitUserThreadAddr;

}

PVOID nxrmFindCorExeMainAddress(IN PVOID MscoreeBase)
{
	NTSTATUS Status = STATUS_SUCCESS;

	PVOID _CorExeMain = NULL;

	do 
	{
		Status = nxrmLookupEntryPoint(MscoreeBase,"_CorExeMain",&_CorExeMain);

		if(!NT_SUCCESS(Status))
		{
			_CorExeMain = NULL;
			break;
		}

	} while(FALSE);

	return _CorExeMain;
}

PVOID nxrmFindZwXXX(
	IN OUT PVOID *pfn_ZwProtectVirtualMemory,
	IN OUT PVOID *pfn_ZwFlushInstructionCache,
	IN OUT PVOID *pfn_ZwFreeVirtualMemory,
	IN OUT PVOID *pfn_ZwCreateThread,
	IN OUT PVOID *pfn_ZwCreateThreadEx
	)
{
	PVOID ntaddr = NULL;

#ifdef _X86_
	PVOID fnZwPulseEvent = NULL;
#endif

#ifdef _AMD64_
	PVOID fnZwQuerySection = NULL;
#endif

	PVOID fnZwCreateSymbolicLinkObject = NULL;

	NTSTATUS status = STATUS_SUCCESS;

	STRING	nt;

	UCHAR	*pcode = NULL;

	ULONG_PTR offset = 0;
	ULONG	i = 0;

	do 
	{
		//
		// find NT kernel base address
		//
		do 
		{
			RtlInitString(&nt, krnl_name);

			ntaddr = nxrmFindKernelModuleBaseAddress(&nt);

			if(ntaddr)
			{
				break;
			}

			RtlInitString(&nt,krnlpae_name);

			ntaddr = nxrmFindKernelModuleBaseAddress(&nt);

			if(ntaddr)
			{
				break;
			}

			RtlInitString(&nt,krnlmppae_name);

			ntaddr = nxrmFindKernelModuleBaseAddress(&nt);

			if(ntaddr)
			{
				break;
			}

			RtlInitString(&nt,krnlmp_name);

			ntaddr = nxrmFindKernelModuleBaseAddress(&nt);

			if(ntaddr)
			{
				break;
			}

		} while(FALSE);

		if(!ntaddr)
		{
			status = STATUS_UNSUCCESSFUL;
			break;
		}

#ifdef _X86_

		status = nxrmLookupEntryPoint(ntaddr,"ZwPulseEvent",&fnZwPulseEvent);

		if(!NT_SUCCESS(status))
		{
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		for(i = 0; i < 64; i++)
		{
			pcode = (((UCHAR*)fnZwPulseEvent) + i);

			if(*pcode == 0xc2)
			{
				if(*(pcode -1) != 0xb8)
				{
					break;
				}
				else
				{
					pcode = NULL;
				}
			}
			else
			{
				pcode = NULL;
			}
		}

		if(!pcode)
		{
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		pcode+= (sizeof(SHORT) + sizeof(UCHAR));

		offset = (ULONG)(pcode - (UCHAR*)fnZwPulseEvent);

		pcode = (((UCHAR*)fnZwPulseEvent) - 1);

		if(*pcode == 0x90)
		{
			*pfn_ZwProtectVirtualMemory = (PVOID)((UCHAR*)fnZwPulseEvent - offset - 5);
		}
		else if (*pcode == 0xcc && Global.os_major == 10)
		{
			*pfn_ZwProtectVirtualMemory = (PVOID)((UCHAR*)fnZwPulseEvent + offset + 12);
		}
		else
		{
			if((Global.os_major == 6 && Global.os_minor >= 2) || Global.os_major == 10)
			{
				*pfn_ZwProtectVirtualMemory = (PVOID)((UCHAR*)fnZwPulseEvent + offset);
			}
			else
			{
				*pfn_ZwProtectVirtualMemory = (PVOID)((UCHAR*)fnZwPulseEvent - offset);
			}
		}

		status = nxrmLookupEntryPoint(ntaddr,"ZwCreateSymbolicLinkObject",&fnZwCreateSymbolicLinkObject);

		if(!NT_SUCCESS(status))
		{
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		pcode = NULL;

		for(i = 0; i < 64; i++)
		{
			pcode = (((UCHAR*)fnZwCreateSymbolicLinkObject) + i);

			if(*pcode == 0xc2 && *(pcode + 1) == 0x10)
			{
				if(*(pcode -1) != 0xb8)
				{
					break;
				}
				else
				{
					pcode = NULL;
				}
			}
			else
			{
				pcode = NULL;
			}
		}

		if(!pcode)
		{
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		pcode+= (sizeof(SHORT) + sizeof(UCHAR));

		offset = (ULONG)(pcode - (UCHAR*)fnZwCreateSymbolicLinkObject);

		pcode = (((UCHAR*)fnZwCreateSymbolicLinkObject) - 1);

		if (Global.os_major == 10)
		{
			if (*pcode == 0xcc)
				*pfn_ZwCreateThread = (PVOID)((UCHAR*)fnZwCreateSymbolicLinkObject - offset - 12);
			else
				*pfn_ZwCreateThread = (PVOID)((UCHAR*)fnZwCreateSymbolicLinkObject - offset);

			*pfn_ZwCreateThreadEx = (PVOID)((UCHAR*)fnZwCreateSymbolicLinkObject - offset - offset);
		}
		else
		{
			*pfn_ZwCreateThread = (PVOID)((UCHAR*)fnZwCreateSymbolicLinkObject + offset);
			*pfn_ZwCreateThreadEx = NULL;
		}

#endif

#ifdef _AMD64_

		status = nxrmLookupEntryPoint(ntaddr,"ZwQuerySection",&fnZwQuerySection);

		if(!NT_SUCCESS(status))
		{
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		for(i = 0; i < 64; i++)
		{
			pcode = (((UCHAR*)fnZwQuerySection) + i);

			if (Global.os_major != 10)
			{
				if (*pcode == 0x66 && *(pcode + 1) == 0x90)
				{
					break;
				}
				else
				{
					pcode = NULL;
				}
			}
			else
			{
				if (*pcode == 0xc3 && *(pcode + 1) == 0x90)
				{
					break;
				}
				else
				{
					pcode = NULL;
				}
			}
		}

		if(!pcode)
		{
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		pcode+= 2;

		offset = (ULONG_PTR)(pcode - (UCHAR*)fnZwQuerySection);

		*pfn_ZwProtectVirtualMemory = (PVOID)((UCHAR*)fnZwQuerySection - offset);

		*pfn_ZwCreateThread = (PVOID)((UCHAR*)fnZwQuerySection - 3 * offset);

		if (Global.os_major == 10)
		{
			status = nxrmLookupEntryPoint(ntaddr, "ZwCreateSymbolicLinkObject", &fnZwCreateSymbolicLinkObject);

			if (!NT_SUCCESS(status))
			{
				status = STATUS_UNSUCCESSFUL;
				break;
			}

			*pfn_ZwCreateThreadEx = (PVOID)((UCHAR*)fnZwCreateSymbolicLinkObject + offset);
		}
		else
		{
			*pfn_ZwCreateThreadEx = NULL;
		}
#endif

		nxrmLookupEntryPoint(ntaddr,"ZwFlushInstructionCache",pfn_ZwFlushInstructionCache);

		nxrmLookupEntryPoint(ntaddr,"ZwFreeVirtualMemory",pfn_ZwFreeVirtualMemory);

	} while(FALSE);

	return ntaddr;
}


static USHORT NameToOrdinal(
	IN PCHAR Name,
	IN PVOID DllBase,
	IN ULONG NumberOfNames,
	IN PULONG NameTable,
	IN PUSHORT OrdinalTable
	)
{
	ULONG	Mid;
	LONG	Ret;

	if(!NumberOfNames) 
		return -1;

	Mid = NumberOfNames >> 1;
	Ret = strcmp(Name, (PCHAR)((ULONG_PTR)DllBase + NameTable[Mid]));

	if(!Ret) 
		return OrdinalTable[Mid];

	if(NumberOfNames == 1) 
		return -1;

	if (Ret < 0)
	{
		NumberOfNames = Mid;
	}
	else
	{
		NameTable		= &NameTable[Mid + 1];
		OrdinalTable	= &OrdinalTable[Mid + 1];
		NumberOfNames	-= (Mid - 1);
	}

	return NameToOrdinal(Name, DllBase, NumberOfNames, NameTable, OrdinalTable);
}

NTSTATUS nxrmLookupEntryPoint(
	IN PVOID DllBase,
	IN PCHAR Name,
	OUT PVOID *EntryPoint
	)
{
	PULONG NameTable;
	PUSHORT OrdinalTable;
	PIMAGE_EXPORT_DIRECTORY ExportDirectory;
	ULONG ExportSize;
	CHAR Buffer[64];
	USHORT Ordinal;
	PULONG ExportTable;

	ExportDirectory = RtlImageDirectoryEntryToData(DllBase,
												   TRUE,
												   IMAGE_DIRECTORY_ENTRY_EXPORT,
												   &ExportSize);

	if(strlen(Name) > sizeof(Buffer) - 2) 
		return STATUS_INVALID_PARAMETER;

	RtlZeroMemory(Buffer,64);

	memcpy(Buffer, Name, min(strlen(Name),64));

	NameTable = (PULONG)((ULONG_PTR)DllBase + ExportDirectory->AddressOfNames);
	OrdinalTable = (PUSHORT)((ULONG_PTR)DllBase + ExportDirectory->AddressOfNameOrdinals);

	Ordinal = NameToOrdinal(Buffer,
							DllBase,
							ExportDirectory->NumberOfNames,
							NameTable,
							OrdinalTable);

	if(Ordinal >= ExportDirectory->NumberOfFunctions)
	{
		return STATUS_PROCEDURE_NOT_FOUND;
	}

	ExportTable = (PULONG)((ULONG_PTR)DllBase + ExportDirectory->AddressOfFunctions);
	*EntryPoint = (PVOID)((ULONG_PTR)DllBase + ExportTable[Ordinal]);

	return STATUS_SUCCESS;
}

static PIMAGE_NT_HEADERS RtlImageNtHeader (IN PVOID BaseAddress)
{
	PIMAGE_NT_HEADERS NtHeader;
	PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)BaseAddress;

	if(DosHeader && DosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		dprintf("DosHeader->e_magic %x\n", DosHeader->e_magic);
		dprintf("NtHeader 0x%lx\n", ((ULONG_PTR)BaseAddress + DosHeader->e_lfanew));
	}

	if(DosHeader && DosHeader->e_magic == IMAGE_DOS_SIGNATURE)
	{
		NtHeader = (PIMAGE_NT_HEADERS)((ULONG_PTR)BaseAddress + DosHeader->e_lfanew);

		if(NtHeader->Signature == IMAGE_NT_SIGNATURE)
			return NtHeader;
	}

	return NULL;
}


static PVOID RtlImageDirectoryEntryToData(
	PVOID BaseAddress,
	BOOLEAN MappedAsImage,
	USHORT Directory,
	PULONG Size
	)
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

#ifdef _AMD64_

NTSTATUS nxrmLookupWow64EntryPoint(
	IN PVOID DllBase,
	IN PCHAR Name,
	OUT PVOID *EntryPoint
	)
{
	PULONG NameTable;
	PUSHORT OrdinalTable;
	PIMAGE_EXPORT_DIRECTORY ExportDirectory;
	ULONG ExportSize;
	CHAR Buffer[64];
	USHORT Ordinal;
	PULONG ExportTable;

	ExportDirectory = Wow64RtlImageDirectoryEntryToData(DllBase,
														TRUE,
														IMAGE_DIRECTORY_ENTRY_EXPORT,
														&ExportSize);

	if(strlen(Name) > sizeof(Buffer) - 2) 
		return STATUS_INVALID_PARAMETER;

	RtlZeroMemory(Buffer,64);

	strncpy(Buffer, Name, 64);

	NameTable = (PULONG)((ULONG_PTR)DllBase + ExportDirectory->AddressOfNames);
	OrdinalTable = (PUSHORT)((ULONG_PTR)DllBase + ExportDirectory->AddressOfNameOrdinals);

	Ordinal = NameToOrdinal(Buffer,
							DllBase,
							ExportDirectory->NumberOfNames,
							NameTable,
							OrdinalTable);

	if(Ordinal >= ExportDirectory->NumberOfFunctions)
	{
		return STATUS_PROCEDURE_NOT_FOUND;
	}

	ExportTable = (PULONG)((ULONG_PTR)DllBase + ExportDirectory->AddressOfFunctions);
	*EntryPoint = (PVOID)((ULONG_PTR)DllBase + ExportTable[Ordinal]);

	return STATUS_SUCCESS;
}

static PIMAGE_NT_HEADERS32 Wow64RtlImageNtHeader(
	IN PVOID BaseAddress
	)
{
	PIMAGE_NT_HEADERS32 NtHeader;
	PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)BaseAddress;

	if(DosHeader && DosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		dprintf("DosHeader->e_magic %x\n", DosHeader->e_magic);
		dprintf("NtHeader 0x%lx\n", ((ULONG_PTR)BaseAddress + DosHeader->e_lfanew));
	}

	if(DosHeader && DosHeader->e_magic == IMAGE_DOS_SIGNATURE)
	{
		NtHeader = (PIMAGE_NT_HEADERS32)((ULONG_PTR)BaseAddress + DosHeader->e_lfanew);

		if(NtHeader->Signature == IMAGE_NT_SIGNATURE)
			return NtHeader;
	}

	return NULL;
}

PVOID Wow64RtlImageDirectoryEntryToData(
	PVOID BaseAddress,
	BOOLEAN MappedAsImage,
	USHORT Directory,
	PULONG Size
	)
{
	PIMAGE_NT_HEADERS32 NtHeader;
	ULONG Va;

	if ((ULONG_PTR)BaseAddress & 1)
	{
		BaseAddress = (PVOID)((ULONG_PTR)BaseAddress & ~1);
		MappedAsImage = FALSE;
	}


	NtHeader = Wow64RtlImageNtHeader (BaseAddress);
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

	return Wow64RtlImageRvaToVa (NtHeader, BaseAddress, Va, NULL);
}


static PIMAGE_SECTION_HEADER Wow64RtlImageRvaToSection (
	PIMAGE_NT_HEADERS32     NtHeader,
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


static PVOID Wow64RtlImageRvaToVa (
	PIMAGE_NT_HEADERS32       NtHeader,
	PVOID					  BaseAddress,
	ULONG					  Rva,
	PIMAGE_SECTION_HEADER	  *SectionHeader
	)
{
	PIMAGE_SECTION_HEADER Section = NULL;

	if (SectionHeader)
		Section = *SectionHeader;

	if (Section == NULL ||
		Rva < Section->VirtualAddress ||
		Rva >= Section->VirtualAddress + Section->SizeOfRawData)
	{
		Section = Wow64RtlImageRvaToSection(NtHeader, BaseAddress, Rva);

		if (Section == NULL)
			return 0;

		if (SectionHeader)
			*SectionHeader = Section;
	}

	return (PVOID)((ULONG_PTR)BaseAddress +	Rva + Section->PointerToRawData - (ULONG_PTR)Section->VirtualAddress);
}

#endif