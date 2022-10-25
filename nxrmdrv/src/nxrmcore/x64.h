#pragma once

#define F_MODRM         0x00000001
#define F_SIB           0x00000002
#define F_IMM8          0x00000004
#define F_IMM16         0x00000008
#define F_IMM32         0x00000010
#define F_IMM64         0x00000020
#define F_DISP8         0x00000040
#define F_DISP16        0x00000080
#define F_DISP32        0x00000100
#define F_RELATIVE      0x00000200
#define F_ERROR         0x00001000
#define F_ERROR_OPCODE  0x00002000
#define F_ERROR_LENGTH  0x00004000
#define F_ERROR_LOCK    0x00008000
#define F_ERROR_OPERAND 0x00010000
#define F_PREFIX_REPNZ  0x01000000
#define F_PREFIX_REPX   0x02000000
#define F_PREFIX_REP    0x03000000
#define F_PREFIX_66     0x04000000
#define F_PREFIX_67     0x08000000
#define F_PREFIX_LOCK   0x10000000
#define F_PREFIX_SEG    0x20000000
#define F_PREFIX_REX    0x40000000
#define F_PREFIX_ANY    0x7f000000

#define PREFIX_SEGMENT_CS   0x2e
#define PREFIX_SEGMENT_SS   0x36
#define PREFIX_SEGMENT_DS   0x3e
#define PREFIX_SEGMENT_ES   0x26
#define PREFIX_SEGMENT_FS   0x64
#define PREFIX_SEGMENT_GS   0x65
#define PREFIX_LOCK         0xf0
#define PREFIX_REPNZ        0xf2
#define PREFIX_REPX         0xf3
#define PREFIX_OPERAND_SIZE 0x66
#define PREFIX_ADDRESS_SIZE 0x67

#define C_NONE    0x00
#define C_MODRM   0x01
#define C_IMM8    0x02
#define C_IMM16   0x04
#define C_IMM_P66 0x10
#define C_REL8    0x20
#define C_REL32   0x40
#define C_GROUP   0x80
#define C_ERROR   0xff

#define PRE_ANY  0x00
#define PRE_NONE 0x01
#define PRE_F2   0x02
#define PRE_F3   0x04
#define PRE_66   0x08
#define PRE_67   0x10
#define PRE_LOCK 0x20
#define PRE_SEG  0x40
#define PRE_ALL  0xff

#define DELTA_OPCODES      0x4a
#define DELTA_FPU_REG      0xfd
#define DELTA_FPU_MODRM    0x104
#define DELTA_PREFIXES     0x13c
#define DELTA_OP_LOCK_OK   0x1ae
#define DELTA_OP2_LOCK_OK  0x1c6
#define DELTA_OP_ONLY_MEM  0x1d8
#define DELTA_OP2_ONLY_MEM 0x1e7

#pragma pack(push,1)

typedef struct {
	UCHAR					len;
	UCHAR					p_rep;
	UCHAR					p_lock;
	UCHAR					p_seg;
	UCHAR					p_66;
	UCHAR					p_67;
	UCHAR					rex;
	UCHAR					rex_w;
	UCHAR					rex_r;
	UCHAR					rex_x;
	UCHAR					rex_b;
	UCHAR					opcode;
	UCHAR					opcode2;
	UCHAR					modrm;
	UCHAR					modrm_mod;
	UCHAR					modrm_reg;
	UCHAR					modrm_rm;
	UCHAR					sib;
	UCHAR					sib_scale;
	UCHAR					sib_index;
	UCHAR					sib_base;
	union {
		UCHAR				imm8;
		USHORT				imm16;
		ULONG				imm32;
		unsigned long long	imm64;
	} imm;
	union {
		UCHAR				disp8;
		USHORT				disp16;
		ULONG				disp32;
	} disp;

	ULONG					flags;

} x64s;

#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif

	unsigned int __stdcall x64_disasm(const void *code, x64s *instruinfo);

#ifdef __cplusplus
}
#endif
