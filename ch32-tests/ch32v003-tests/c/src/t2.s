/*
 * Simple tests for ch32v003 SoC with QingKeV2 RV32 core.
 * Example: setup different unified exception handlers.
 * Copyright (c) 2023 geomatsi@gmail.com
 */

	.section .init, "ax", @progbits
	.global main

main:
	li t0, 0x0
	la t2, handler1
	csrrw t1, mtvec, t2
	ecall
	bnez t0, 2f
1:
	/* error */
	wfi
	j 1b
2:
	li t0, 0x0
	la t2, handler2
	csrw mtvec, t2
	ecall
	bnez t0, 4f
3:
	/* error */
	wfi
	j 3b
4:
	csrw mtvec, t1
	ecall

	/* unreachable */
1:
	wfi
	j 1b

	/* QingKeV2 2.2: it should be noted that the vector table base address
         * needs to be 1KB aligned in the QingKe V2 microprocessor.
	 */
	.balign 0x400
handler1:
	/* return to insn after ecall */
	csrr t0, mepc
	addi t0, t0, 4
	csrw mepc, t0

	li t0, 0x1
	mret

	/* QingKeV2 2.2: it should be noted that the vector table base address
         * needs to be 1KB aligned in the QingKe V2 microprocessor.
	 */
	.balign 0x400
handler2:
	/* return to insn after ecall */
	csrr t0, mepc
	addi t0, t0, 4
	csrw mepc, t0

	li t0, 0x1
	mret
