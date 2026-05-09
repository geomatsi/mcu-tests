#include "ch32v00x_conf.h"

void delay(unsigned int cycles)
{

	while (cycles--) {
		__asm__ volatile ("nop");
	}
}

void early_init(void)
{
	/* setup HSE 24MHz system clock */
	SystemInit();
}

void main(void)
{
	GPIO_InitTypeDef GPIO_InitStructure = {0};

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	while (1) {
		GPIO_WriteBit(GPIOD, GPIO_Pin_6, Bit_SET);
		delay(200000);
		GPIO_WriteBit(GPIOD, GPIO_Pin_6, Bit_RESET);
		delay(200000);
	}
}
