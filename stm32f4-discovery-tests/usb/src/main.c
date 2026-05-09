#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f4_discovery_audio_codec.h"
#include "stm32f4xx_conf.h"

#include "usbd_cdc_core.h"
#include "usbd_desc.h"
#include "usbd_usr.h"
#include "usb_conf.h"

#include "tim2.h"

USB_OTG_CORE_HANDLE  USB_OTG_dev;
uint8_t ch;

GPIO_InitTypeDef  gpio_init;
RCC_ClocksTypeDef RCC_Clocks;


void delay(volatile uint32_t count)
{
	while(count) {
		count--;
	}
}

void OTG_FS_WKUP_IRQHandler(void)
{
	if(USB_OTG_dev.cfg.low_power)
	{
		 *(uint32_t *)(0xE000ED10) &= 0xFFFFFFF9 ;
		SystemInit();
		USB_OTG_UngateClock(&USB_OTG_dev);
	}

	EXTI_ClearITPendingBit(EXTI_Line18);
}

void OTG_FS_IRQHandler(void)
{
	USBD_OTG_ISR_Handler(&USB_OTG_dev);
}

void TIM2_IRQHandler(void)
{
	timer_tim2_irq();
}

//void SysTick_Handler(void)
//{
//
//}

void blink(volatile int count, GPIO_TypeDef* GPIOx, uint16_t pin)
{
	while (count--) {
		GPIO_SetBits(GPIOx, pin);
		delay(0x3FFFFF);
		GPIO_ResetBits(GPIOx, pin);
		delay(0x3FFFFF);
	}
}

void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size)
{
	blink(10, GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
}

uint32_t Codec_TIMEOUT_UserCallback(void)
{
	blink(10, GPIOD, GPIO_Pin_12 | GPIO_Pin_14);
}

uint16_t EVAL_AUDIO_GetSampleCallBack(void)
{
	blink(10, GPIOD, GPIO_Pin_13 | GPIO_Pin_15);
}

#define AUDIO_SIZE 446636
#define AUDIO_ADDR 0x08020000

int main(void)
{
	int ret;

	//if (SysTick_Config(SystemCoreClock / 1000)) {
	//	while (1){};
	//}

	/* GPIOD Periph clock enable */

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	/* Configure PD12, PD13, PD14 and PD15 in output pushpull mode */

	gpio_init.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	gpio_init.GPIO_Mode = GPIO_Mode_OUT;
	gpio_init.GPIO_OType = GPIO_OType_PP;
	gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
	gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(GPIOD, &gpio_init);

	timer_tim2_init();

	/* test audio */

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1 | RCC_APB1Periph_SPI3, ENABLE);
	RCC_PLLI2SCmd(ENABLE);

#ifdef AUDIO_TEST
	blink(3, GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);

	EVAL_AUDIO_SetAudioInterface(AUDIO_INTERFACE_I2S);

	ret = EVAL_AUDIO_Init(OUTPUT_DEVICE_HEADPHONE, 200, I2S_AudioFreq_11k);

	if (ret != 0) {
		blink(3, GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
	} else {
		blink(6, GPIOD, GPIO_Pin_13 | GPIO_Pin_15);
	}

	ret = EVAL_AUDIO_Play((uint16_t *) AUDIO_ADDR, AUDIO_SIZE);

	if (ret != 0) {
		blink(3, GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
	} else {
		blink(6, GPIOD, GPIO_Pin_12 | GPIO_Pin_14);
	}
#endif

#ifdef USB_TEST

	blink(3, GPIOD, GPIO_Pin_12 | GPIO_Pin_14);

	USBD_Init(&USB_OTG_dev,
			USB_OTG_FS_CORE_ID,
			&USR_desc,
			&USBD_CDC_cb,
			&USR_cb);

	blink(3, GPIOD, GPIO_Pin_13 | GPIO_Pin_15);

	while (1) {
		VCP_send_str("0123456789\0");
		delay(0x3FFFFF);
	}

#endif

	/* blink */

	while (1)
	{
		/* PD12 to be toggled */
		GPIO_SetBits(GPIOD, GPIO_Pin_12);

		/* Insert delay */
		delay(0x3FFFFF);

		/* PD13 to be toggled */
		GPIO_SetBits(GPIOD, GPIO_Pin_13);

		/* Insert delay */
		delay(0x3FFFFF);

		/* PD14 to be toggled */
		GPIO_SetBits(GPIOD, GPIO_Pin_14);

		/* Insert delay */
		delay(0x3FFFFF);

		/* PD15 to be toggled */
		GPIO_SetBits(GPIOD, GPIO_Pin_15);

		/* Insert delay */
		delay(0x7FFFFF);

		GPIO_ResetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);

		/* Insert delay */
		delay(0xFFFFFF);
	}
}
