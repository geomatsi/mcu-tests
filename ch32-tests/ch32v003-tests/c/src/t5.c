#include "ch32v00x_conf.h"

BitAction action = Bit_SET;

void early_init(void)
{
	/* setup HSE 24MHz system clock */
	SystemInit();
}

void SysTick_Handler(void) __attribute__((interrupt("machine")));
void SysTick_Handler(void)
{
	action = (action == Bit_SET) ? Bit_RESET : Bit_SET;
	GPIO_WriteBit(GPIOD, GPIO_Pin_6, action);
	SysTick->SR = 0;
}

void main(void)
{
	GPIO_InitTypeDef GPIO_InitStructure = {0};

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_WriteBit(GPIOD, GPIO_Pin_6, Bit_RESET);

	SysTick->CMP = SystemCoreClock - 1;
	SysTick->CNT = 0;
	SysTick->SR = 0;
	SysTick->CTLR = 0xF;

	NVIC_EnableIRQ(SysTicK_IRQn);

	while (1) {
		__asm__ volatile ("wfi");
	}
}
