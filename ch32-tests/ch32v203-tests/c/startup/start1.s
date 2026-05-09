/*
 * Startup code for ch32v203 SoC with QingKeV4 RV32 core
 *
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Copyright (c) 2023 geomatsi@gmail.com
 */

	.section .init, "ax", @progbits
	.global _start
	.align 1

_start:
	j handle_reset
	/* TBD
	 * - undocumented 0x13/0x73 entries
         * - not sure if any reason other than 64-byte alignment of .vector section
         */
	.word 0x00000013
	.word 0x00000013
	.word 0x00000013
	.word 0x00000013
	.word 0x00000013
	.word 0x00000013
	.word 0x00000013
	.word 0x00000013
	.word 0x00000013
	.word 0x00000013
	.word 0x00000013
	.word 0x00000013
	.word 0x00100073

	.section .vector, "ax", @progbits
	.option norvc
	.align  1

_vector_base:
	.word   _start
	.word   0
	.word   NMI_Handler
	.word   HardFault_Handler
	.word   0
	.word   Ecall_M_Mode_Handler
	.word   0
	.word   0
	.word   Ecall_U_Mode_Handler
	.word   Break_Point_Handler
	.word   0
	.word   0
	.word   SysTick_Handler
	.word   0
	.word   SW_Handler
	.word   0
	/* External Interrupts */
	.word   WWDG_IRQHandler
	.word   PVD_IRQHandler
	.word   TAMPER_IRQHandler
	.word   RTC_IRQHandler
	.word   FLASH_IRQHandler
	.word   RCC_IRQHandler
	.word   EXTI0_IRQHandler
	.word   EXTI1_IRQHandler
	.word   EXTI2_IRQHandler
	.word   EXTI3_IRQHandler
	.word   EXTI4_IRQHandler
	.word   DMA1_Channel1_IRQHandler
	.word   DMA1_Channel2_IRQHandler
	.word   DMA1_Channel3_IRQHandler
	.word   DMA1_Channel4_IRQHandler
	.word   DMA1_Channel5_IRQHandler
	.word   DMA1_Channel6_IRQHandler
	.word   DMA1_Channel7_IRQHandler
	.word   ADC1_2_IRQHandler
	.word   USB_HP_CAN1_TX_IRQHandler
	.word   USB_LP_CAN1_RX0_IRQHandler
	.word   CAN1_RX1_IRQHandler
	.word   CAN1_SCE_IRQHandler
	.word   EXTI9_5_IRQHandler
	.word   TIM1_BRK_IRQHandler
	.word   TIM1_UP_IRQHandler
	.word   TIM1_TRG_COM_IRQHandler
	.word   TIM1_CC_IRQHandler
	.word   TIM2_IRQHandler
	.word   TIM3_IRQHandler
	.word   TIM4_IRQHandler
	.word   I2C1_EV_IRQHandler
	.word   I2C1_ER_IRQHandler
	.word   I2C2_EV_IRQHandler
	.word   I2C2_ER_IRQHandler
	.word   SPI1_IRQHandler
	.word   SPI2_IRQHandler
	.word   USART1_IRQHandler
	.word   USART2_IRQHandler
	.word   USART3_IRQHandler
	.word   EXTI15_10_IRQHandler
	.word   RTCAlarm_IRQHandler
	.word   USBWakeUp_IRQHandler
	.word   USBHD_IRQHandler
	.word   USBHDWakeUp_IRQHandler
	.word   UART4_IRQHandler
	.word   DMA1_Channel8_IRQHandler

	.section  .text.vector_handler, "ax", @progbits
	.option rvc

	.weak   NMI_Handler
	.weak   HardFault_Handler
	.weak   Ecall_M_Mode_Handler
	.weak   Ecall_U_Mode_Handler
	.weak   Break_Point_Handler
	.weak   SysTick_Handler
	.weak   SW_Handler
	.weak   WWDG_IRQHandler
	.weak   PVD_IRQHandler
	.weak   TAMPER_IRQHandler
	.weak   RTC_IRQHandler
	.weak   FLASH_IRQHandler
	.weak   RCC_IRQHandler
	.weak   EXTI0_IRQHandler
	.weak   EXTI1_IRQHandler
	.weak   EXTI2_IRQHandler
	.weak   EXTI3_IRQHandler
	.weak   EXTI4_IRQHandler
	.weak   DMA1_Channel1_IRQHandler
	.weak   DMA1_Channel2_IRQHandler
	.weak   DMA1_Channel3_IRQHandler
	.weak   DMA1_Channel4_IRQHandler
	.weak   DMA1_Channel5_IRQHandler
	.weak   DMA1_Channel6_IRQHandler
	.weak   DMA1_Channel7_IRQHandler
	.weak   ADC1_2_IRQHandler
	.weak   USB_HP_CAN1_TX_IRQHandler
	.weak   USB_LP_CAN1_RX0_IRQHandler
	.weak   CAN1_RX1_IRQHandler
	.weak   CAN1_SCE_IRQHandler
	.weak   EXTI9_5_IRQHandler
	.weak   TIM1_BRK_IRQHandler
	.weak   TIM1_UP_IRQHandler
	.weak   TIM1_TRG_COM_IRQHandler
	.weak   TIM1_CC_IRQHandler
	.weak   TIM2_IRQHandler
	.weak   TIM3_IRQHandler
	.weak   TIM4_IRQHandler
	.weak   I2C1_EV_IRQHandler
	.weak   I2C1_ER_IRQHandler
	.weak   I2C2_EV_IRQHandler
	.weak   I2C2_ER_IRQHandler
	.weak   SPI1_IRQHandler
	.weak   SPI2_IRQHandler
	.weak   USART1_IRQHandler
	.weak   USART2_IRQHandler
	.weak   USART3_IRQHandler
	.weak   EXTI15_10_IRQHandler
	.weak   RTCAlarm_IRQHandler
	.weak   USBWakeUp_IRQHandler
	.weak   USBHD_IRQHandler
	.weak   USBHDWakeUp_IRQHandler
	.weak   UART4_IRQHandler
	.weak   DMA1_Channel8_IRQHandler

NMI_Handler:			1:  j 1b
HardFault_Handler:		1:  j 1b
Ecall_M_Mode_Handler:		1:  j 1b
Ecall_U_Mode_Handler:		1:  j 1b
Break_Point_Handler:		1:  j 1b
SysTick_Handler:		1:  j 1b
SW_Handler:			1:  j 1b
WWDG_IRQHandler:		1:  j 1b
PVD_IRQHandler:			1:  j 1b
TAMPER_IRQHandler:		1:  j 1b
RTC_IRQHandler:			1:  j 1b
FLASH_IRQHandler:		1:  j 1b
RCC_IRQHandler:			1:  j 1b
EXTI0_IRQHandler:		1:  j 1b
EXTI1_IRQHandler:		1:  j 1b
EXTI2_IRQHandler:		1:  j 1b
EXTI3_IRQHandler:		1:  j 1b
EXTI4_IRQHandler:		1:  j 1b
DMA1_Channel1_IRQHandler:	1:  j 1b
DMA1_Channel2_IRQHandler:	1:  j 1b
DMA1_Channel3_IRQHandler:	1:  j 1b
DMA1_Channel4_IRQHandler:	1:  j 1b
DMA1_Channel5_IRQHandler:	1:  j 1b
DMA1_Channel6_IRQHandler:	1:  j 1b
DMA1_Channel7_IRQHandler:	1:  j 1b
ADC1_2_IRQHandler:		1:  j 1b
USB_HP_CAN1_TX_IRQHandler:	1:  j 1b
USB_LP_CAN1_RX0_IRQHandler:	1:  j 1b
CAN1_RX1_IRQHandler:		1:  j 1b
CAN1_SCE_IRQHandler:		1:  j 1b
EXTI9_5_IRQHandler:		1:  j 1b
TIM1_BRK_IRQHandler:		1:  j 1b
TIM1_UP_IRQHandler:		1:  j 1b
TIM1_TRG_COM_IRQHandler:	1:  j 1b
TIM1_CC_IRQHandler:		1:  j 1b
TIM2_IRQHandler:		1:  j 1b
TIM3_IRQHandler:		1:  j 1b
TIM4_IRQHandler:		1:  j 1b
I2C1_EV_IRQHandler:		1:  j 1b
I2C1_ER_IRQHandler:		1:  j 1b
I2C2_EV_IRQHandler:		1:  j 1b
I2C2_ER_IRQHandler:		1:  j 1b
SPI1_IRQHandler:		1:  j 1b
SPI2_IRQHandler:		1:  j 1b
USART1_IRQHandler:		1:  j 1b
USART2_IRQHandler:		1:  j 1b
USART3_IRQHandler:		1:  j 1b
EXTI15_10_IRQHandler:		1:  j 1b
RTCAlarm_IRQHandler:		1:  j 1b
USBWakeUp_IRQHandler:		1:  j 1b
USBHD_IRQHandler:		1:  j 1b
USBHDWakeUp_IRQHandler:		1:  j 1b
UART4_IRQHandler:		1:  j 1b
DMA1_Channel8_IRQHandler:	1:  j 1b

	.section .text.handle_reset, "ax", @progbits
	.weak early_init
	.align 1

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
	/* setup CORECFGR (TBD: undocumented) */
	li t0, 0x1f
	csrw 0xbc0, t0

	/* set MPIE: enable interrupts after 'mret' to 'main' */
	li t0, 0x80
	csrw mstatus, t0

	/* setup vector table with 'irq handler absolute addr' entires */
	la t0, _vector_base
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
