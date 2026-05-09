#ifndef __TIM2__H__
#define __TIM2__H__

#include "stm32f4xx_conf.h"
#include "stm32f4xx.h"

#define TIM_MSEC_DELAY	0x01
#define TIM_USEC_DELAY	0x02

extern __IO uint32_t timer_tim2_count;

void timer_tim2_settime(uint8_t Unit);
void timer_tim2_delay_block(uint32_t nTime, uint8_t Unit);
void timer_tim2_delay_async(uint32_t nTime, uint8_t Unit);
void timer_tim2_init(void);
void timer_tim2_irq(void);

#endif /*__TIM2__H__ */
