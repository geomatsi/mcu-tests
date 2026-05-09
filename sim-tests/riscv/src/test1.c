#include <stdint.h>

#include "xprintf.h"
#include "htif.h"

void m_trap_handler(void) __attribute__((interrupt("machine")));
void m_trap_handler(void)
{
	while (1) {
		__asm__ volatile ("wfi");
	}
}

int main(void)
{
	xdev_out(htif_putc);

	for (int i = 0; i < 10; i++) {
		xprintf("%s: count: %x\n", __func__, i);
	}

	htif_exit(10);
}
