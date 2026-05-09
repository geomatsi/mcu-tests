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

/* symbols from linker script */

extern char _sstack[];
extern char _estack[];
extern char _sbss[];
extern char _ebss[];

/* allocated in data section */

int test1 = 1;

/* allocated in bss */

int test2;

void stack_dump(void)
{
	char *p;
	int i;

	for (p = _sstack, i = 1; p < _estack; p++, i++) {
		xprintf("%02x ", *p);
		if (i % 64 == 0)
			xputs("\r\n");
	}

	xputs("\r\n");
}

unsigned int stack_depth(void)
{
	char *p;

	for (p = _sstack; p < _estack; p++) {
		if (*p != 0xaa)
			break;
	}

	return (unsigned int)(_estack - p);
}

unsigned long recursion(unsigned long v)
{
	if (v == 0 || v == 1)
		return 1;

	return v + recursion(v - 1);
}

void main(void)
{
	unsigned int count;

	uart_init(115200);
	xdev_out(putchar);

	xprintf("sbss: 0x%x\r\n", _sbss);
	xprintf("ebss: 0x%x\r\n", _ebss);

	xprintf("sstack: 0x%x\r\n", _sstack);
	xprintf("estack: 0x%x\r\n", _estack);

	xprintf("test1: 0x%x -> %d\r\n", &test1, test1);
	xprintf("test2: 0x%x -> %d\r\n", &test2, test2);

	stack_dump();

	for (count = 0; 1; count++)  {
		xprintf("f(%lu) = %lu\r\n", count, recursion(count));
		xprintf("stack depth : %d\r\n", stack_depth());
		delay(500000);
	}
}
