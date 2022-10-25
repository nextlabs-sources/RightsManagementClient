#pragma once

extern ULONG x64GetCurrentInstructionLength(PBYTE ins);
extern ULONG x86GetCurrentInstructionLength(PBYTE ins);
extern ULONG x86BackupInstructions(PBYTE psrc, ULONG length, PBYTE pdst);