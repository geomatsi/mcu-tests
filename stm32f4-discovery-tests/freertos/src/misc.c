#include <stdint.h>

#include "stm32f4xx_conf.h"

void delay(volatile uint32_t count)
{
	while(count) {
		count--;
	}
}

void blink(volatile int count, GPIO_TypeDef* GPIOx, uint16_t pin)
{
	while (count--) {
		GPIO_SetBits(GPIOx, pin);
		delay(0x3FFFFF);
		GPIO_ResetBits(GPIOx, pin);
		delay(0x3FFFFF);
	}
}

void critical_error_blink(void)
{
	while( 1 ) {
		GPIO_ResetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
		delay(0x1FFFFF);
		GPIO_SetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
		delay(0x1FFFFF);
	}
}
