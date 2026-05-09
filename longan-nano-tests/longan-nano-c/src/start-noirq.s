.section .init, "ax"
.global _start
_start:
	/* disable interrupts */
	csrw mie, 0
	csrw mip, 0

	/* jump to logical address first to ensure correct operation of RAM region  */
	la a0, _start
	li a1, 1
	slli a1, a1, 29
	bleu a1, a0, _start0800
	srli a1, a1, 2
	bleu a1, a0, _start0800
	la a0, _start0800
	add a0, a0, a1
	jr a0

_start0800:
	/* setup sp and fp */
	la sp, __stack_top
	add s0, sp, zero

	/* load data section */
	la a0, _data_lma
	la a1, _sdata
	la a2, _edata
	bgeu a1, a2, 2f
1:
	lw t0, (a0)
	sw t0, (a1)
	addi a0, a0, 4
	addi a1, a1, 4
	bltu a1, a2, 1b
2:
	/* clear bss section */
	la a0, _sbss
	la a1, _ebss
	bgeu a0, a1, 2f
1:
	sw zero, (a0)
	addi a0, a0, 4
	bltu a0, a1, 1b
2:
	call main
1:
	wfi
	j 1b
