#include <stdint.h>
#include <msp430g2231.h>

#define	LED_R	(0x1 << 0x0)
#define	LED_G	(0x1 << 0x6)

extern void sleep(unsigned int c);

static void led_toggle(void)
{
	P1OUT ^= (LED_R | LED_G);
}

int main(void)

{
	// Stop watchdog timer
	WDTCTL = WDTPW + WDTHOLD;

	// Set P1.0 and P1.6 to output direction
	P1DIR |= (LED_R | LED_G);

	// Set RED on, GREEN off
	P1OUT |= LED_R;
	P1OUT &= ~LED_G;

	for (;;)
	{
		led_toggle();
		sleep(20000);
	}
}
