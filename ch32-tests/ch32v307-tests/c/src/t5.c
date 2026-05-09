#include "ch32v30x_conf.h"

extern unsigned int _sstack, _estack;

static void delay(unsigned int cycles)
{

	while (cycles--) {
		__asm__ volatile ("nop");
	}
}

static void blink_all_leds(unsigned int cycles)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_SET);
	GPIO_WriteBit(GPIOB, GPIO_Pin_4, Bit_SET);
	delay(cycles);
	GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_RESET);
	GPIO_WriteBit(GPIOB, GPIO_Pin_4, Bit_RESET);
	delay(cycles);
}

static void blink_red_led(unsigned int cycles)
{
	GPIO_WriteBit(GPIOB, GPIO_Pin_4, Bit_RESET);

	GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_SET);
	delay(cycles);
	GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_RESET);
	delay(cycles);
}

static void blink_blue_led(unsigned int cycles)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_RESET);

	GPIO_WriteBit(GPIOB, GPIO_Pin_4, Bit_SET);
	delay(cycles);
	GPIO_WriteBit(GPIOB, GPIO_Pin_4, Bit_RESET);
	delay(cycles);
}

static void stack_overflow(void);

static void w(void)
{
	/* trick for '-Werror=infinite-recursion' */
	stack_overflow();
}

static void stack_overflow(void)
{
	char t[10] = {0};
	unsigned int n;

	for (n = 0; n < sizeof(t) / sizeof(t[0]); n++) {
		t[n] = 'a' + n;
	}

	w();
}

void invalid_access(void)
{
	/* access protected area */
	*((volatile unsigned int *)0x1fffffe0) = 0xdeadbeef;
}

void early_init(void)
{
	/*
	 * Setup PMP protection to catch stack overflows
	 * Protected memory area: 256 bytes from reserved address space below RAM
	 * - 0x1fff_ff00 <= addr < 0x2000_0000
	 * - size: 256 bytes
	 * PMP: TOR configuration
	 * - pmpaddr0 = 0x1fffff00 >> 2 = 0x7ffffc0
	 * - pmpaddr1 = 0x20000000 >> 2 = 0x8000000
	 * - pmpcfg1: 0x8800 = 0x88 << 8 = 0b10001000 << 8, where 0b10001000:
	 *   - 1  (Locked)
	 *   - 0  (Reserved)
	 *   - 0  (Reserved)
	 *   - 01 (TOR)
	 *   - 0  (no X)
	 *   - 0  (no W)
	 *   - 0  (no R)
	 */

	__asm__ __volatile__(
		"li	t0, 0x7ffffc0\n\t"
		"csrw	pmpaddr0, t0\n\t"
		"li	t0, 0x8000000\n\t"
		"csrw	pmpaddr1, t0\n\t"
		"li	t0, 0x8800\n\t"
		"csrw	pmpcfg0, t0\n\t"
		: : : );
}

/* keep this handler naked: it can be called on stack overflow */
void HardFault_Handler(void) __attribute__((naked));
void HardFault_Handler(void)
{
	register unsigned int sp asm("sp");

	if (sp <= (unsigned int)&_sstack) {
		/* restore sp and fp to startup values
		 *
		 * NOTE: it is not system recovery !!!
		 * system is in failed state with overflowed stack,
		 * so sp and fp are restored for the sole purpose
		 * of proper recovery
		 * - example: indicate stack overflow by blinking red LED
		 * - proper recovery should do something useful, e.g
		 *   to store error logs and to reset the system
		 */
		__asm__ __volatile__(
			"mv	sp, %0\n\t"
			"add 	s0, sp, zero\n\t"
			: : "r"(&_estack) : );

		while (1) {
			blink_red_led(100000);
		}
	}

	/*
	 * Hard fault reason: anything else but stack overflow
	 * - example: indicate 'any other' fault by blinking blue LED
	 * - proper recovery should do something useful, e.g
	 *   to store error logs and to reset the system
	 */
	while (1) {
		blink_blue_led(100000);
	}
}

void main(void)
{
	unsigned int n = 0;

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

	/* show that firmware is alive: slow blink two LEDs */
	for (n = 0; n < 5; n++) {
		blink_all_leds(800000);
	}

#if 0
	invalid_access();
#else
	stack_overflow();
#endif

	/* unexpected success: fast blink two LEDs */
	while (1) {
		blink_all_leds(200000);
	}
}
