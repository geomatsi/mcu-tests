/*
 * Startup code for ch32v003 SoC with QingKeV2 RV32 core
 *
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Copyright (c) 2023 geomatsi@gmail.com
 */

	.section .init, "ax", @progbits
	.global _start
	.align 2

_start:
	.option	norvc;
	j handle_reset
	.word	0
	.word   NMI_Handler
	.word   Exception_Handler
	.word   0
	.word   0
	.word   0
	.word   0
	.word   0
	.word   0
	.word   0
	.word   0
	.word   SysTick_Handler
	.word   0
	.word   SWI_Handler
	.word   0
	/* External Interrupts */
	.word   WWDG_IRQHandler
	.word   PVD_IRQHandler
	.word   FLASH_IRQHandler
	.word   RCC_IRQHandler
	.word   EXTI7_0_IRQHandler
	.word   AWU_IRQHandler
	.word   DMA1_Ch1_IRQHandler
	.word   DMA1_Ch2_IRQHandler
	.word   DMA1_Ch3_IRQHandler
	.word   DMA1_Ch4_IRQHandler
	.word   DMA1_Ch5_IRQHandler
	.word   DMA1_Ch6_IRQHandler
	.word   DMA1_Ch7_IRQHandler
	.word   ADC1_IRQHandler
	.word   I2C1_EV_IRQHandler
	.word   I2C1_ER_IRQHandler
	.word   USART1_IRQHandler
	.word   SPI1_IRQHandler
	.word   TIM1_BRK_IRQHandler
	.word   TIM1_UP_IRQHandler
	.word   TIM1_TRG_COM_IRQHandler
	.word   TIM1_CC_IRQHandler
	.word   TIM2_IRQHandler

	.option rvc;
	.section  .text.vector_handler, "ax", @progbits

	.weak   NMI_Handler
	.weak   Exception_Handler
	.weak   SysTick_Handler
	.weak   SWI_Handler
	.weak   WWDG_IRQHandler
	.weak   PVD_IRQHandler
	.weak   FLASH_IRQHandler
	.weak   RCC_IRQHandler
	.weak   EXTI7_0_IRQHandler
	.weak   AWU_IRQHandler
	.weak   DMA1_Ch1_IRQHandler
	.weak   DMA1_Ch2_IRQHandler
	.weak   DMA1_Ch3_IRQHandler
	.weak   DMA1_Ch4_IRQHandler
	.weak   DMA1_Ch5_IRQHandler
	.weak   DMA1_Ch6_IRQHandler
	.weak   DMA1_Ch7_IRQHandler
	.weak   ADC1_IRQHandler
	.weak   I2C1_EV_IRQHandler
	.weak   I2C1_ER_IRQHandler
	.weak   USART1_IRQHandler
	.weak   SPI1_IRQHandler
	.weak   TIM1_BRK_IRQHandler
	.weak   TIM1_UP_IRQHandler
	.weak   TIM1_TRG_COM_IRQHandler
	.weak   TIM1_CC_IRQHandler
	.weak   TIM2_IRQHandler

NMI_Handler:			1: j 1b
Exception_Handler:		1: j 1b
SysTick_Handler:		1: j 1b
SWI_Handler:			1: j 1b
WWDG_IRQHandler:		1: j 1b
PVD_IRQHandler:			1: j 1b
FLASH_IRQHandler:		1: j 1b
RCC_IRQHandler:			1: j 1b
EXTI7_0_IRQHandler:		1: j 1b
AWU_IRQHandler:			1: j 1b
DMA1_Ch1_IRQHandler:		1: j 1b
DMA1_Ch2_IRQHandler:		1: j 1b
DMA1_Ch3_IRQHandler:		1: j 1b
DMA1_Ch4_IRQHandler:		1: j 1b
DMA1_Ch5_IRQHandler:		1: j 1b
DMA1_Ch6_IRQHandler:		1: j 1b
DMA1_Ch7_IRQHandler:		1: j 1b
ADC1_IRQHandler:		1: j 1b
I2C1_EV_IRQHandler:		1: j 1b
I2C1_ER_IRQHandler:		1: j 1b
USART1_IRQHandler:		1: j 1b
SPI1_IRQHandler:		1: j 1b
TIM1_BRK_IRQHandler:		1: j 1b
TIM1_UP_IRQHandler:		1: j 1b
TIM1_TRG_COM_IRQHandler:	1: j 1b
TIM1_CC_IRQHandler:		1: j 1b
TIM2_IRQHandler:		1: j 1b

	.section .text.handle_reset, "ax", @progbits
	.align 2

	.weak early_init

early_init:
	ret

handle_reset:
	/* setup gp */
	/*
	.option push
	.option norelax
	la gp, __global_pointer$
	.option pop
	*/

	/* setup sp and fp */
	la sp, _estack
	add s0, sp, zero

	/* load data section from flash to RAM */
	la a0, _data_lma
	la a1, _data_vma
	la a2, _edata
	bgeu a1, a2, 2f
1:
	lw t0, (a0)
	sw t0, (a1)
	addi a0, a0, 4
	addi a1, a1, 4
	bltu a1, a2, 1b

2:
	/* load sram functions section from flash to RAM */
	la a0, _text_lma
	la a1, _text_vma
	la a2, _etext
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
	/* fill stack region with 0xaa canaries */
	la a0, _sstack
	la a1, _estack
	bgeu a0, a1, 2f
	li t0, 0xaaaaaaaa
1:
	sw t0, (a0)
	addi a0, a0, 4
	bltu a0, a1, 1b

2:
	/* set MPIE: enable interrupts after 'mret' to 'main' */
	li t0, 0x80
	csrw mstatus, t0

	/* setup vector table with 'irq handler absolute addr' entires */
	la t0, _start
	ori t0, t0, 3
	csrw mtvec, t0

	/* early init */
	jal early_init

	/* setup jump to main */
	la t0, main
	csrw mepc, t0
	mret

	/* unreachable */
1:
	wfi
	j 1b
