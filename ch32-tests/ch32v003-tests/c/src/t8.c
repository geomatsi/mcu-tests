#include "ch32v00x_conf.h"
#include "xprintf.h"

void early_init(void)
{
	/* setup HSE 24MHz system clock */
	SystemInit();
}

void delay(unsigned int cycles)
{

	while (cycles--) {
		__asm__ volatile ("nop");
	}
}

void uart_init(uint32_t baudrate)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx;

	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);
}

void putchar(int ch)
{
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
	USART_SendData(USART1, ch);
}

void main(void)
{
	int count = 0;

	uart_init(115200);
	xdev_out(putchar);

	/* tests from common/src/xprintf.c */

	xprintf("%d\r\n", 1234);            // "1234"
	xprintf("%6d,%3d%%\r\n", -200, 5);  // "  -200,  5%"
	xprintf("%-6u\r\n", 100);           // "100   "
	xprintf("%ld\r\n", 12345678);       // "12345678"
	xprintf("%04x\r\n", 0xA3);          // "00a3"
	xprintf("%08lX\r\n", 0x123ABC);     // "00123ABC"
	xprintf("%016b\r\n", 0x550F);       // "0101010100001111"
	xprintf("%*d\r\n", 6, 100);         // "   100"
	xprintf("%s\r\n", "String");        // "String"
	xprintf("%5s\r\n", "abc");          // "  abc"
	xprintf("%-5s\r\n", "abc");         // "abc  "
	xprintf("%-5s\r\n", "abcdefg");     // "abcdefg"
	xprintf("%-5.5s\r\n", "abcdefg");   // "abcde"
	xprintf("%-.5s\r\n", "abcdefg");    // "abcde"
	xprintf("%-5.5s\r\n", "abc");       // "abc  "
	xprintf("%c\r\n", 'a');             // "a"

	while (1) {
		xprintf("count: %d\r\n", count);
		delay(10000000);
		count += 1;
	}
}
