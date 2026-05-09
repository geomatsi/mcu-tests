/*
 * Startup code in C for ch32v307 SoC with QingKeV4 RV32 core
 * Based on startup files of the libopencm3 project.
 *
 * Copyright (C) 2010 Piotr Esden-Tempski <piotr@esden.net>,
 * Copyright (C) 2012 chrysn <chrysn@fsfe.org>
 * Copyright (C) 2023 Sergey Matyukevich <geomatsi@gmail.com>
 */

/* interrupt handler types */

typedef void (*vector_table_entry_t)(void);

typedef struct {
	vector_table_entry_t reset;
	vector_table_entry_t reserved_0x0001;
	vector_table_entry_t NMI;
	vector_table_entry_t HardFault;
	vector_table_entry_t Reserved_0x0004;
	vector_table_entry_t Ecall_M;
	vector_table_entry_t Reserved_0x0006;
	vector_table_entry_t Reserved_0x0007;
	vector_table_entry_t Ecall_U;
	vector_table_entry_t BreakPoint;
	vector_table_entry_t Reserved_0x000a;
	vector_table_entry_t Reserved_0x000b;
	vector_table_entry_t SysTick;
	vector_table_entry_t reserved_0x000d;
	vector_table_entry_t SW;
	vector_table_entry_t reserved_0x000f;
	vector_table_entry_t WWDG;
	vector_table_entry_t PVD;
	vector_table_entry_t TAMPER;
	vector_table_entry_t RTC;
	vector_table_entry_t FLASH;
	vector_table_entry_t RCC;
	vector_table_entry_t EXTI0;
	vector_table_entry_t EXTI1;
	vector_table_entry_t EXTI2;
	vector_table_entry_t EXTI3;
	vector_table_entry_t EXTI4;
	vector_table_entry_t DMA1_Channel1;
	vector_table_entry_t DMA1_Channel2;
	vector_table_entry_t DMA1_Channel3;
	vector_table_entry_t DMA1_Channel4;
	vector_table_entry_t DMA1_Channel5;
	vector_table_entry_t DMA1_Channel6;
	vector_table_entry_t DMA1_Channel7;
	vector_table_entry_t ADC1_2;
	vector_table_entry_t USB_HP_CAN1_TX;
	vector_table_entry_t USB_LP_CAN1_RX0;
	vector_table_entry_t CAN1_RX1;
	vector_table_entry_t CAN1_SCE;
	vector_table_entry_t EXTI9_5;
	vector_table_entry_t TIM1_BRK;
	vector_table_entry_t TIM1_UP;
	vector_table_entry_t TIM1_TRG_COM;
	vector_table_entry_t TIM1_CC;
	vector_table_entry_t TIM2;
	vector_table_entry_t TIM3;
	vector_table_entry_t TIM4;
	vector_table_entry_t I2C1_EV;
	vector_table_entry_t I2C1_ER;
	vector_table_entry_t I2C2_EV;
	vector_table_entry_t I2C2_ER;
	vector_table_entry_t SPI1;
	vector_table_entry_t SPI2;
	vector_table_entry_t USART1;
	vector_table_entry_t USART2;
	vector_table_entry_t USART3;
	vector_table_entry_t EXTI15_10;
	vector_table_entry_t RTCAlarm;
	vector_table_entry_t USBWakeUp;
	vector_table_entry_t TIM8_BRK;
	vector_table_entry_t TIM8_UP;
	vector_table_entry_t TIM8_TRG_COM;
	vector_table_entry_t TIM8_CC;
	vector_table_entry_t RNG;
	vector_table_entry_t FSMC;
	vector_table_entry_t SDIO;
	vector_table_entry_t TIM5;
	vector_table_entry_t SPI3;
	vector_table_entry_t UART4;
	vector_table_entry_t UART5;
	vector_table_entry_t TIM6;
	vector_table_entry_t TIM7;
	vector_table_entry_t DMA2_Channel1;
	vector_table_entry_t DMA2_Channel2;
	vector_table_entry_t DMA2_Channel3;
	vector_table_entry_t DMA2_Channel4;
	vector_table_entry_t DMA2_Channel5;
	vector_table_entry_t ETH;
	vector_table_entry_t ETH_WKUP;
	vector_table_entry_t CAN2_TX;
	vector_table_entry_t CAN2_RX0;
	vector_table_entry_t CAN2_RX1;
	vector_table_entry_t CAN2_SCE;
	vector_table_entry_t OTG_FS;
	vector_table_entry_t USBHSWakeup;
	vector_table_entry_t USBHS;
	vector_table_entry_t DVP;
	vector_table_entry_t UART6;
	vector_table_entry_t UART7;
	vector_table_entry_t UART8;
	vector_table_entry_t TIM9_BRK;
	vector_table_entry_t TIM9_UP;
	vector_table_entry_t TIM9_TRG_COM;
	vector_table_entry_t TIM9_CC;
	vector_table_entry_t TIM10_BRK;
	vector_table_entry_t TIM10_UP;
	vector_table_entry_t TIM10_TRG_COM;
	vector_table_entry_t TIM10_CC;
	vector_table_entry_t DMA2_Channel6;
	vector_table_entry_t DMA2_Channel7;
	vector_table_entry_t DMA2_Channel8;
	vector_table_entry_t DMA2_Channel9;
	vector_table_entry_t DMA2_Channel10;
	vector_table_entry_t DMA2_Channel11;
} vector_table_t;

/* variables from linker script */

extern unsigned _data_lma, _data_vma, _edata;
extern unsigned _text_lma, _text_vma, _etext;
extern unsigned _sstack, _estack;
extern unsigned _sbss, _ebss;

/* default interrupt handlers */

void NMI_Handler(void) __attribute__((weak, alias("blocking_handler")));
void HardFault_Handler(void) __attribute__((weak, alias("blocking_handler")));
void Ecall_M_Mode_Handler(void) __attribute__((weak, alias("blocking_handler")));
void Ecall_U_Mode_Handler(void) __attribute__((weak, alias("blocking_handler")));
void Break_Point_Handler(void) __attribute__((weak, alias("blocking_handler")));
void SysTick_Handler(void) __attribute__((weak, alias("null_handler")));
void SW_Handler(void) __attribute__((weak, alias("blocking_handler")));
void WWDG_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void PVD_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void TAMPER_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void RTC_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void FLASH_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void RCC_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void EXTI0_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void EXTI1_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void EXTI2_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void EXTI3_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void EXTI4_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void DMA1_Channel1_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void DMA1_Channel2_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void DMA1_Channel3_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void DMA1_Channel4_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void DMA1_Channel5_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void DMA1_Channel6_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void DMA1_Channel7_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void ADC1_2_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void USB_HP_CAN1_TX_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void USB_LP_CAN1_RX0_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void CAN1_RX1_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void CAN1_SCE_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void EXTI9_5_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void TIM1_BRK_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void TIM1_UP_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void TIM1_TRG_COM_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void TIM1_CC_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void TIM2_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void TIM3_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void TIM4_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void I2C1_EV_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void I2C1_ER_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void I2C2_EV_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void I2C2_ER_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void SPI1_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void SPI2_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void USART1_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void USART2_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void USART3_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void EXTI15_10_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void RTCAlarm_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void USBWakeUp_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void TIM8_BRK_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void TIM8_UP_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void TIM8_TRG_COM_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void TIM8_CC_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void RNG_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void FSMC_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void SDIO_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void TIM5_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void SPI3_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void UART4_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void UART5_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void TIM6_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void TIM7_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void DMA2_Channel1_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void DMA2_Channel2_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void DMA2_Channel3_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void DMA2_Channel4_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void DMA2_Channel5_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void ETH_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void ETH_WKUP_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void CAN2_TX_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void CAN2_RX0_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void CAN2_RX1_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void CAN2_SCE_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void OTG_FS_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void USBHSWakeup_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void USBHS_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void DVP_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void UART6_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void UART7_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void UART8_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void TIM9_BRK_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void TIM9_UP_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void TIM9_TRG_COM_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void TIM9_CC_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void TIM10_BRK_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void TIM10_UP_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void TIM10_TRG_COM_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void TIM10_CC_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void DMA2_Channel6_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void DMA2_Channel7_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void DMA2_Channel8_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void DMA2_Channel9_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void DMA2_Channel10_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));
void DMA2_Channel11_IRQHandler(void) __attribute__((weak, alias("blocking_handler")));

void blocking_handler(void) __attribute((naked));
void blocking_handler(void)
{
	while (1);
}

void null_handler(void)
{
	/* nop */
}

/* misc functions declarations */

void early_init(void) __attribute__((weak, alias("null_handler")));
void _start(void) __attribute__((naked, section(".init")));
void reset_handler(void) __attribute__((naked));
int main(void);

/* vector table */

__attribute__ ((section(".vector")))
vector_table_t vector_table = {
	.reset			= _start,
	.NMI			= NMI_Handler,
	.HardFault		= HardFault_Handler,
	.Ecall_M		= Ecall_M_Mode_Handler,
	.Ecall_U		= Ecall_U_Mode_Handler,
	.BreakPoint		= Break_Point_Handler,
	.SysTick		= SysTick_Handler,
	.SW			= SW_Handler,
	.WWDG			= WWDG_IRQHandler,
	.PVD			= PVD_IRQHandler,
	.TAMPER			= TAMPER_IRQHandler,
	.RTC			= RTC_IRQHandler,
	.FLASH			= FLASH_IRQHandler,
	.RCC			= RCC_IRQHandler,
	.EXTI0			= EXTI0_IRQHandler,
	.EXTI1			= EXTI1_IRQHandler,
	.EXTI2			= EXTI2_IRQHandler,
	.EXTI3			= EXTI3_IRQHandler,
	.EXTI4			= EXTI4_IRQHandler,
	.DMA1_Channel1		= DMA1_Channel1_IRQHandler,
	.DMA1_Channel2		= DMA1_Channel2_IRQHandler,
	.DMA1_Channel3		= DMA1_Channel3_IRQHandler,
	.DMA1_Channel4		= DMA1_Channel4_IRQHandler,
	.DMA1_Channel5		= DMA1_Channel5_IRQHandler,
	.DMA1_Channel6		= DMA1_Channel6_IRQHandler,
	.DMA1_Channel7		= DMA1_Channel7_IRQHandler,
	.ADC1_2			= ADC1_2_IRQHandler,
	.USB_HP_CAN1_TX		= USB_HP_CAN1_TX_IRQHandler,
	.USB_LP_CAN1_RX0	= USB_LP_CAN1_RX0_IRQHandler,
	.CAN1_RX1		= CAN1_RX1_IRQHandler,
	.CAN1_SCE		= CAN1_SCE_IRQHandler,
	.EXTI9_5		= EXTI9_5_IRQHandler,
	.TIM1_BRK		= TIM1_BRK_IRQHandler,
	.TIM1_UP		= TIM1_UP_IRQHandler,
	.TIM1_TRG_COM		= TIM1_TRG_COM_IRQHandler,
	.TIM1_CC		= TIM1_CC_IRQHandler,
	.TIM2			= TIM2_IRQHandler,
	.TIM3			= TIM3_IRQHandler,
	.TIM4			= TIM4_IRQHandler,
	.I2C1_EV		= I2C1_EV_IRQHandler,
	.I2C1_ER		= I2C1_ER_IRQHandler,
	.I2C2_EV		= I2C2_EV_IRQHandler,
	.I2C2_ER		= I2C2_ER_IRQHandler,
	.SPI1			= SPI1_IRQHandler,
	.SPI2			= SPI2_IRQHandler,
	.USART1			= USART1_IRQHandler,
	.USART2			= USART2_IRQHandler,
	.USART3			= USART3_IRQHandler,
	.EXTI15_10		= EXTI15_10_IRQHandler,
	.RTCAlarm		= RTCAlarm_IRQHandler,
	.USBWakeUp		= USBWakeUp_IRQHandler,
	.TIM8_BRK		= TIM8_BRK_IRQHandler,
	.TIM8_UP		= TIM8_UP_IRQHandler,
	.TIM8_TRG_COM		= TIM8_TRG_COM_IRQHandler,
	.TIM8_CC		= TIM8_CC_IRQHandler,
	.RNG			= RNG_IRQHandler,
	.FSMC			= FSMC_IRQHandler,
	.SDIO		 	= SDIO_IRQHandler,
	.TIM5		 	= TIM5_IRQHandler,
	.SPI3		 	= SPI3_IRQHandler,
	.UART4		 	= UART4_IRQHandler,
	.UART5		 	= UART5_IRQHandler,
	.TIM6		 	= TIM6_IRQHandler,
	.TIM7		 	= TIM7_IRQHandler,
	.DMA2_Channel1		= DMA2_Channel1_IRQHandler,
	.DMA2_Channel2		= DMA2_Channel2_IRQHandler,
	.DMA2_Channel3		= DMA2_Channel3_IRQHandler,
	.DMA2_Channel4		= DMA2_Channel4_IRQHandler,
	.DMA2_Channel5		= DMA2_Channel5_IRQHandler,
	.ETH			= ETH_IRQHandler,
	.ETH_WKUP		= ETH_WKUP_IRQHandler,
	.CAN2_TX		= CAN2_TX_IRQHandler,
	.CAN2_RX0		= CAN2_RX0_IRQHandler,
	.CAN2_RX1		= CAN2_RX1_IRQHandler,
	.CAN2_SCE		= CAN2_SCE_IRQHandler,
	.OTG_FS			= OTG_FS_IRQHandler,
	.USBHSWakeup		= USBHSWakeup_IRQHandler,
	.USBHS			= USBHS_IRQHandler,
	.DVP			= DVP_IRQHandler,
	.UART6			= UART6_IRQHandler,
	.UART7			= UART7_IRQHandler,
	.UART8			= UART8_IRQHandler,
	.TIM9_BRK		= TIM9_BRK_IRQHandler,
	.TIM9_UP		= TIM9_UP_IRQHandler,
	.TIM9_TRG_COM		= TIM9_TRG_COM_IRQHandler,
	.TIM9_CC		= TIM9_CC_IRQHandler,
	.TIM10_BRK		= TIM10_BRK_IRQHandler,
	.TIM10_UP		= TIM10_UP_IRQHandler,
	.TIM10_TRG_COM		= TIM10_TRG_COM_IRQHandler,
	.TIM10_CC		= TIM10_CC_IRQHandler,
	.DMA2_Channel6		= DMA2_Channel6_IRQHandler,
	.DMA2_Channel7		= DMA2_Channel7_IRQHandler,
	.DMA2_Channel8		= DMA2_Channel8_IRQHandler,
	.DMA2_Channel9		= DMA2_Channel9_IRQHandler,
	.DMA2_Channel10		= DMA2_Channel10_IRQHandler,
	.DMA2_Channel11		= DMA2_Channel11_IRQHandler
};

/* startup sequence */

void _start(void)
{
	__asm__ __volatile__(
		"jr	%0\n\t"
		: : "r"(reset_handler) : );
}

void reset_handler(void)
{
	volatile unsigned *src, *dest;

	/* setup sp and fp */

	__asm__ __volatile__(
		"mv	sp, %0\n\t"
		"add 	s0, sp, zero\n\t"
		: : "r"(&_estack) : );

	/* load r/w data section from ROM to RAM */

	for (src = &_data_lma, dest = &_data_vma; dest < &_edata; src++, dest++) {
		*dest = *src;
	}

	/* load in-memory functions section from ROM to RAM */

	for (src = &_text_lma, dest = &_text_vma; dest < &_etext; src++, dest++) {
		*dest = *src;
	}

	/* clear bss section */

	for (dest = &_sbss; dest < &_ebss; dest++) {
		*dest = 0x0;
	}

	/* fill stack region with 0xaa canaries */

	for (dest = &_sstack; dest < &_estack; dest++) {
		*dest = 0xaaaaaaaa;
	}

	/* setup corecfgr (TBD: undocumented in QingKe v4 manual */

	__asm__ __volatile__(
		"li	t0, 0x1f\n\t"
		"csrw	0xbc0, t0\n\t"
		: : : );

	/* setup MPIE: enable FPU and enable interrupts after 'mret' to main */

	__asm__ __volatile__(
		"li	t0, 0x6080\n\t"
		"csrw	mstatus, t0\n\t"
		: : : );

	/* setup vector table with 'irq handler absolute addr' entries */

	__asm__ __volatile__(
		"mv	t0, %0\n\t"
		"ori	t0, t0, 3\n\t"
		"csrw	mtvec, t0\n\t"
		: : "r"(&vector_table) : );

	/* early init */

	early_init();

	/* setup jump to main */

	__asm__ __volatile__(
		"mv     t0, %0\n\t"
		"csrw	mepc, t0\n\t"
		"mret\n\t"
		: : "r"(main) : );

	/* unreachable */

	while (1) {
		__asm__ volatile ("wfi");
	}
}
