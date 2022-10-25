#include <windows.h>
#include "x86.H"

ULONG x86GetCurrentInstructionLength(PBYTE ins);
ULONG x86BackupInstructions(PBYTE psrc, ULONG length, PBYTE pdst);

ULONG x86GetCurrentInstructionLength(PBYTE psrc)
{
	BYTE pdst[DISASM_X86_INSTRUCTION_MAX_LENGTH];

	REFCOPYENTRY pEntry = NULL;
	X86DISASM_INFO	Info;
	PBYTE pbTarget = NULL;
	LONG  lExtra = 0;

	BYTE	*p = NULL;
	
	memset(pdst,0, sizeof(pdst));

	memset(&Info,0,sizeof(Info));

	Info.ppbTarget	= &pbTarget;
	Info.plExtra	= &lExtra;

	pEntry = &instruction_copytable[psrc[0]];

	p = pEntry->pfCopy(&Info,pEntry,pdst,psrc);
	
	if(!p)
	{
		return 0;
	}
	else
	{
		return (ULONG)(p - psrc);
	}
}

ULONG x86BackupInstructions(PBYTE psrc, ULONG length, PBYTE pdst)
{
	REFCOPYENTRY pEntry = NULL;
	X86DISASM_INFO	Info;
	PBYTE pbTarget = NULL;
	LONG  lExtra = 0;

	BYTE	*p = NULL;

	ULONG offset = 0;

	memset(&Info,0,sizeof(Info));

	Info.ppbTarget	= &pbTarget;
	Info.plExtra	= &lExtra;

	pEntry = &instruction_copytable[psrc[0]];

	p = pEntry->pfCopy(&Info,pEntry,pdst+offset,psrc+offset);

	do 
	{
		if(!p)
		{
			break;
		}

		offset = (ULONG)(p - psrc);

		if(offset >= length)
		{
			break;
		}

		pEntry = &instruction_copytable[psrc[offset]];

		p = pEntry->pfCopy(&Info,pEntry,pdst+offset,psrc+offset);

	} while (TRUE);

	if(!p)
	{
		return 0;
	}
	else
	{
		return (ULONG)(p - psrc);
	}
}

static PBYTE x86CopyBytes(PX86DISASM_INFO pInfo, REFCOPYENTRY pEntry, PBYTE pbDst, PBYTE pbSrc)
{
	LONG nBytesFixed = (pEntry->nFlagBits & ADDRESS)
						? (pInfo->bAddressOverride ? pEntry->nFixedSize16 : pEntry->nFixedSize)
						: (pInfo->bOperandOverride ? pEntry->nFixedSize16 : pEntry->nFixedSize);

	LONG nBytes = nBytesFixed;
	LONG nRelOffset = pEntry->nRelOffset;
	LONG cbTarget = nBytes - nRelOffset;

	if (pEntry->nModOffset > 0) 
	{
		BYTE bModRm = pbSrc[pEntry->nModOffset];
		BYTE bFlags = rbmodrm[bModRm];

		nBytes += bFlags & NOTSIB;

		if (bFlags & SIB) 
		{
			BYTE bSib = pbSrc[pEntry->nModOffset + 1];

			if ((bSib & 0x07) == 0x05)
			{
				if ((bModRm & 0xc0) == 0x00) 
				{
					nBytes += 4;
				}
				else if ((bModRm & 0xc0) == 0x40) 
				{
					nBytes += 1;
				}
				else if ((bModRm & 0xc0) == 0x80)
				{
					nBytes += 4;
				}
			}
			cbTarget = nBytes - nRelOffset;
		}
		else if (bFlags & RIP) 
		{
			//
			//	x64 instruction
			//
		}
	}

	memcpy(pbDst, pbSrc, nBytes);

	if (nRelOffset) 
	{
		if(pInfo->ppbTarget)
			*(pInfo->ppbTarget) = x86AdjustTarget(pInfo, pbDst, pbSrc, nBytesFixed, nRelOffset, cbTarget);

	}

	if (pEntry->nFlagBits & DYNAMIC)
	{
		if(pInfo->ppbTarget)
			*(pInfo->ppbTarget) = (PBYTE)DETOUR_INSTRUCTION_TARGET_DYNAMIC;
	}
	return pbSrc + nBytes;
}

static PBYTE x86CopyBytesPrefix(PX86DISASM_INFO	pInfo, REFCOPYENTRY pEntry, PBYTE pbDst, PBYTE pbSrc)
{
	x86CopyBytes(pInfo, pEntry, pbDst, pbSrc);

	pEntry = &instruction_copytable[pbSrc[1]];
	return pEntry->pfCopy(pInfo, pEntry, pbDst + 1, pbSrc + 1);
}

static PBYTE x86CopyBytesRax(PX86DISASM_INFO pInfo, REFCOPYENTRY pEntry, PBYTE pbDst, PBYTE pbSrc)
{
	x86CopyBytes(pInfo, pEntry, pbDst, pbSrc);

	if (*pbSrc & 0x8) {
		pInfo->bRaxOverride = TRUE;
	}

	pEntry = &instruction_copytable[pbSrc[1]];
	return pEntry->pfCopy(pInfo, pEntry, pbDst + 1, pbSrc + 1);
}

static PBYTE x86AdjustTarget(
	PX86DISASM_INFO pInfo,
	PBYTE			pbDst, 
	PBYTE			pbSrc, 
	LONG			cbOp,
	LONG			cbTargetOffset,
	LONG			cbTargetSize)
{
	PBYTE pbTarget = NULL;
	PVOID pvTargetAddr = &pbDst[cbTargetOffset];
	LONG_PTR nOldOffset = 0;
	LONG_PTR nNewOffset = 0;

	switch (cbTargetSize) {
	  case 1:
		  nOldOffset = (LONG_PTR)(*(UCHAR*)pvTargetAddr);
		  break;
	  case 2:
		  nOldOffset = (LONG_PTR)(*(SHORT*)pvTargetAddr);
		  break;
	  case 4:
		  nOldOffset = (LONG_PTR)(*(LONG*)pvTargetAddr);
		  break;
	  case 8:
		  nOldOffset = (LONG_PTR)(*(LONG_PTR*)pvTargetAddr);
		  break;
	  default:
		  break;
	}

	pbTarget = pbSrc + cbOp + nOldOffset;

	nNewOffset = nOldOffset - (pbDst - pbSrc);

	switch (cbTargetSize) {
	  case 1:
		  *(UCHAR*)pvTargetAddr = (UCHAR)nNewOffset;
		  
		  if (nNewOffset < SCHAR_MIN || nNewOffset > SCHAR_MAX) 
		  {
			  if(pInfo->plExtra)
				*(pInfo->plExtra) = sizeof(ULONG_PTR) - 1;
		  }

		  break;
	  case 2:
		  *(SHORT*)pvTargetAddr = (SHORT)nNewOffset;
		  
		  if (nNewOffset < SHRT_MIN || nNewOffset > SHRT_MAX) 
		  {
			  if(pInfo->plExtra)
				*(pInfo->plExtra) = sizeof(ULONG_PTR) - 2;
		  }

		  break;
	  case 4:
		  *(LONG*)pvTargetAddr = (LONG)nNewOffset;

		  if (nNewOffset < LONG_MIN || nNewOffset > LONG_MAX) 
		  {
			  if(pInfo->plExtra)
				*(pInfo->plExtra) = sizeof(ULONG_PTR) - 4;
		  }

		  break;
	  case 8:
		  *(LONG_PTR*)pvTargetAddr = (LONG_PTR)nNewOffset;
		  break;
	}
	
	return pbTarget;
}


static PBYTE x86Invalid(PX86DISASM_INFO pInfo, REFCOPYENTRY pEntry, PBYTE pbDst, PBYTE pbSrc)
{
	UNREFERENCED_PARAMETER(pbDst);
	UNREFERENCED_PARAMETER(pEntry);

	return pbSrc + 1;
}


static PBYTE x86Copy0F(PX86DISASM_INFO pInfo, REFCOPYENTRY pEntry, PBYTE pbDst, PBYTE pbSrc)
{
	x86CopyBytes(pInfo, pEntry, pbDst, pbSrc);

	pEntry = &instruction_copytable0f[pbSrc[1]];
	return pEntry->pfCopy(pInfo, pEntry, pbDst + 1, pbSrc + 1);
}

static PBYTE x86Copy66(PX86DISASM_INFO pInfo, REFCOPYENTRY pEntry, PBYTE pbDst, PBYTE pbSrc)
{ 
	pInfo->bOperandOverride = TRUE;

	return x86CopyBytesPrefix(pInfo, pEntry, pbDst, pbSrc);
}

static PBYTE x86Copy67(PX86DISASM_INFO pInfo, REFCOPYENTRY pEntry, PBYTE pbDst, PBYTE pbSrc)
{   
	pInfo->bAddressOverride = TRUE;

	return x86CopyBytesPrefix(pInfo, pEntry, pbDst, pbSrc);
}

static PBYTE x86CopyF6(PX86DISASM_INFO pInfo, REFCOPYENTRY pEntry, PBYTE pbDst, PBYTE pbSrc)
{
	const COPYENTRY ce = { 0xf6, ENTRY_CopyBytes2Mod };

	UNREFERENCED_PARAMETER(pEntry);

	if (0x00 == (0x38 & pbSrc[1]))
	{
		const COPYENTRY ce_mod1 = { 0xf6, ENTRY_CopyBytes2Mod1 };
		return ce_mod1.pfCopy(pInfo, &ce_mod1, pbDst, pbSrc);
	}

	return ce.pfCopy(pInfo, &ce, pbDst, pbSrc);
}

static PBYTE x86CopyF7(PX86DISASM_INFO pInfo, REFCOPYENTRY pEntry, PBYTE pbDst, PBYTE pbSrc)
{
	const COPYENTRY ce = { 0xf7, ENTRY_CopyBytes2Mod };

	UNREFERENCED_PARAMETER(pEntry);

	if (0x00 == (0x38 & pbSrc[1])) 
	{
		const COPYENTRY ce_modoperand = { 0xf7, ENTRY_CopyBytes2ModOperand };
		return ce_modoperand.pfCopy(pInfo, &ce_modoperand, pbDst, pbSrc);
	}

	return ce.pfCopy(pInfo, &ce, pbDst, pbSrc);
}

static PBYTE x86CopyFF(PX86DISASM_INFO pInfo, REFCOPYENTRY pEntry, PBYTE pbDst, PBYTE pbSrc)
{   
	const COPYENTRY ce = { 0xff, ENTRY_CopyBytes2Mod };

	UNREFERENCED_PARAMETER(pEntry);

	if (0x15 == pbSrc[1] || 0x25 == pbSrc[1]) 
	{

		PBYTE *ppbTarget = *(PBYTE**)&pbSrc[2];
		
		if(pInfo->ppbTarget)
			*(pInfo->ppbTarget) = *ppbTarget;

	}
	else if (0x10 == (0x38 & pbSrc[1]) ||
			 0x18 == (0x38 & pbSrc[1]) ||
			 0x20 == (0x38 & pbSrc[1]) ||
			 0x28 == (0x38 & pbSrc[1])) 
	{
		if(pInfo->ppbTarget)
			*(pInfo->ppbTarget) = (PBYTE)DETOUR_INSTRUCTION_TARGET_DYNAMIC;
	}

	return ce.pfCopy(pInfo, &ce, pbDst, pbSrc);
}