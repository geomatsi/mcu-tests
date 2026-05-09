#include "ch32v20x_conf.h"

void delay(unsigned int cycles)
{

	while (cycles--) {
		__asm__ volatile ("nop");
	}
}

void main(void)
{
	GPIO_InitTypeDef GPIO_InitStructure = {0};

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	while (1) {
		GPIO_WriteBit(GPIOB, GPIO_Pin_8, Bit_SET);
		delay(400000);
		GPIO_WriteBit(GPIOB, GPIO_Pin_8, Bit_RESET);
		delay(800000);
	}
}
