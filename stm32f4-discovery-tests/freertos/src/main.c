#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f4xx_conf.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "usbd_hid_core.h"
#include "usbd_desc.h"
#include "usbd_usr.h"
#include "usb_conf.h"

__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;

GPIO_InitTypeDef gpio_init;
NVIC_InitTypeDef nvic_init;
EXTI_InitTypeDef exti_init;

xQueueHandle xQueue;

static void hw_init(void)
{
	/* GPIOD Periph clock enable */

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	/* Configure PD12, PD13, PD14 and PD15 in output pushpull mode */

	gpio_init.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	gpio_init.GPIO_Mode = GPIO_Mode_OUT;
	gpio_init.GPIO_OType = GPIO_OType_PP;
	gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
	gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(GPIOD, &gpio_init);

	/* Enable GPIOA clock */

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	/* Enable SYSCFG clock */

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* Configure PA0 pin as input floating */

	gpio_init.GPIO_Pin = GPIO_Pin_0;
	gpio_init.GPIO_Mode = GPIO_Mode_IN;
	gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(GPIOA, &gpio_init);

	/* Connect EXTI Line0 to PA0 pin */

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

	/* Configure EXTI Line0 */

	exti_init.EXTI_Line = EXTI_Line0;
	exti_init.EXTI_Mode = EXTI_Mode_Interrupt;
	exti_init.EXTI_Trigger = EXTI_Trigger_Rising;
	exti_init.EXTI_LineCmd = ENABLE;

	EXTI_Init(&exti_init);

	/* Enable and set EXTI Line0 Interrupt to the lowest priority */

	nvic_init.NVIC_IRQChannel = EXTI0_IRQn;
	nvic_init.NVIC_IRQChannelPreemptionPriority = 0x01;
	nvic_init.NVIC_IRQChannelSubPriority = 0x01;
	nvic_init.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_init);

	/* Enable USB device */

	USBD_Init(&USB_OTG_dev, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_HID_cb, &USR_cb);

	/*
		http://www.freertos.org/RTOS-Cortex-M3-M4.html
		Preempt priority and subpriority:

			If you are using an STM32 with the STM32 driver library then ensure all the
			priority bits are assigned to be preempt priority bits by calling
			NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); before the RTOS is started.
	*/

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/* DON'T PUT ANYTHING HERE AFTER NVIC PRIO GROUP ARE PROPERLY CONFIGURED FOR FREERTOS */
}

static void LedFlash(void *Parameters)
{
	uint16_t pin = (uint16_t) Parameters;
	uint32_t num = pin >> 12;

	portTickType LastWake;

	GPIO_SetBits(GPIOD, pin);

	LastWake = xTaskGetTickCount();

	while(1) {
		GPIO_ToggleBits(GPIOD, pin);
		vTaskDelayUntil(&LastWake, 100*num);
	}
}

static void USBHidTask(void *Parameters)
{
	uint8_t hid_buffer[4] = {0};
	portTickType LastWake;

	GPIO_SetBits(GPIOD, GPIO_Pin_14);

	LastWake = xTaskGetTickCount();

	while(1) {
		hid_buffer[0] += 1;
		hid_buffer[1] += 1;

		USBD_HID_SendReport(&USB_OTG_dev, hid_buffer, 4);
		GPIO_ToggleBits(GPIOD, GPIO_Pin_14);
		vTaskDelayUntil(&LastWake, 3000);
	}
}

static void BlueLedControl(void *Parameters)
{
	portTickType LastWake;
	unsigned char c;

	GPIO_SetBits(GPIOD, GPIO_Pin_15);

	LastWake = xTaskGetTickCount();

	while(1) {
		xQueueReceive(xQueue, &c, 10000);
		GPIO_ToggleBits(GPIOD, GPIO_Pin_15);
	}
}

int main()
{
	hw_init();
	blink(5, GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);

	xTaskCreate(LedFlash, (signed char *) "led1", configMINIMAL_STACK_SIZE, (void *) GPIO_Pin_12, tskIDLE_PRIORITY + 2, NULL);
	blink(3, GPIOD, GPIO_Pin_12);

	xTaskCreate(LedFlash, (signed char *) "led2", configMINIMAL_STACK_SIZE, (void *) GPIO_Pin_13, tskIDLE_PRIORITY + 3, NULL);
	blink(3, GPIOD, GPIO_Pin_13);

	xQueue = xQueueCreate( 10, sizeof( unsigned char ) );
	xTaskCreate(BlueLedControl, (signed char *) "button", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL);
	blink(3, GPIOD, GPIO_Pin_15);

	xTaskCreate(USBHidTask, (signed char *) "usb", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
	blink(3, GPIOD, GPIO_Pin_14);

	vTaskStartScheduler();

	/* Just sit and flash all the LED quickly if we fail */

	critical_error_blink();
}
