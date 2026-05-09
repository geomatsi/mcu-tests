#include "tim2.h"

__IO uint32_t timer_tim2_count = 0;

void timer_tim2_irq(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		if (timer_tim2_count > 0x00)
		{
			timer_tim2_count--;
		}
		else
		{
			TIM_Cmd(TIM2, DISABLE);
		}
	}
}

void timer_tim2_delay_block(uint32_t nTime, uint8_t unit)
{
	timer_tim2_count = nTime;
	timer_tim2_settime(unit);
	while (timer_tim2_count != 0);
	TIM_Cmd(TIM2, DISABLE);
}

void timer_tim2_settime(uint8_t unit)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	TIM_Cmd(TIM2, DISABLE);
	TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);


	if (unit == TIM_USEC_DELAY)
	{
		TIM_TimeBaseStructure.TIM_Period = 14;//11;
	}
	else if (unit == TIM_MSEC_DELAY)
	{
		TIM_TimeBaseStructure.TIM_Period = 14000;//11999;
	}
	TIM_TimeBaseStructure.TIM_Prescaler = 5;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

	TIM_ARRPreloadConfig(TIM2, ENABLE);

	/* TIM IT enable */
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	/* TIM2 enable counter */
	TIM_Cmd(TIM2, ENABLE);
}

void timer_tim2_init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x00);

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
}
