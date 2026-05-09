#include <stdint.h>
#include <msp430g2231.h>

#define	BIT(n)	(0x1 << (n))

#define	LED_R	(0x1 << 0x0)
#define	LED_G	(0x1 << 0x6)

static void sleep(unsigned int c)
{
	volatile unsigned int i;

	for(i = 0; i < c; i++);
}

static void led_toggle(void)
{
	P1OUT ^= (LED_R | LED_G);
}

static int get_pin(uint8_t n)
{
	return ((P1IN & BIT(n)) == BIT(n));
}

int main(void)

{

	volatile int is_pressed;

	// Stop watchdog timer
	WDTCTL = WDTPW + WDTHOLD;

	// Set P1.0 and P1.6 to output direction
	P1DIR |= (LED_R | LED_G);

	// Set P1.3 to input direction
	P1DIR &= ~BIT(3);

	// Set RED on, GREEN off
	P1OUT |= LED_R;
	P1OUT &= ~LED_G;

	for (;;)
	{
		// keep toggling LEDs when button is pressed
		is_pressed = !get_pin(3);
		if (is_pressed)
		{
			led_toggle();
		}

		sleep(10000);
	}
}
