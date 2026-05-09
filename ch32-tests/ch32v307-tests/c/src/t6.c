#include "ch32v30x_conf.h"

void delay(unsigned int cycles)
{

	while (cycles--) {
		__asm__ volatile ("nop");
	}
}

void main(void)
{

	float d = 400000.0f;

	GPIO_InitTypeDef GPIO_InitStructure = {0};

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	/* setup LED1 (A15) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* setup LED2 (B4) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	while (1) {
		GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_SET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_4, Bit_RESET);
		delay(d / 2.1f);
		GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_RESET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_4, Bit_SET);
		delay(d * 2.3f);
	}
}
