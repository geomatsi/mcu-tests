#include "opcodes.h"

/* See Section 1.5.1 in RISC-V Unprivileged ISA 20240411 */
int insn_len(unsigned long addr)
{
	uint8_t opcode = *(uint8_t *)addr;

	/* 16-bit opcodes */
	if ((opcode & 0b11) != 0b11)
		return 2;

	/* 32-bit opcodes */
	if ((opcode & 0b11111) != 0b11111)
		return 4;

	/* 48-bit opcodes (not considered frozen) */
	if ((opcode & 0b111111) != 0b111111)
		return 6;

	/* 64-bit opcodes (not considered frozen) */
	if ((opcode & 0b1111111) != 0b1111111)
		return 8;

	/* unknown insn length */
	return 0;
}
