#include "stm8s.h"

#define	LED	(0x1 << 0x5)

extern void sleep(unsigned int c);

int main()
{
	// PB.5 output
	PB_DDR |= LED;

	// PB.5 push-pull
	PB_CR1 |= LED;

	// PB.5 set to 1
	PB_ODR |= LED;

	for (;;)
	{
		PB_ODR ^= LED;
		sleep(10000);
	}
}
