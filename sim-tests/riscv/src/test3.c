#include <stdint.h>

#include "riscv_encoding.h"
#include "xprintf.h"
#include "aclint.h"
#include "htif.h"

void m_trap_handler(void) __attribute__((interrupt("machine")));
void m_trap_handler(void)
{
	unsigned long mcause, mtval, mepc;

	__asm__ volatile ("csrr %0, mcause\n\t"
		: "=r" (mcause) : : );
	__asm__ volatile ("csrr %0, mtval\n\t"
		: "=r" (mtval) : : );
	__asm__ volatile ("csrr %0, mepc\n\t"
		: "=r" (mepc) : : );

	switch (mcause) {
		case (MCAUSE_IRQ_MASK | IRQ_M_SOFT):
			xprintf("%s: machine software interrupt: 0x%lx\n",
				__func__, mcause);
			//__asm__ volatile ("csrsi mstatus, %0" : : "K"(MSTATUS_MIE) : "memory");
			*REG_MSWI0 = 0x1;
			while (1);
			break;
		case (MCAUSE_IRQ_MASK | IRQ_M_TIMER):
			xprintf("%s: machine timer interrupt: 0x%lx\n",
				__func__, mcause);
			//*REG_MTIMECMP = (-1UL);
			*REG_MSWI0 = 0x1;
			__asm__ volatile ("csrsi mstatus, %0" : : "K"(MSTATUS_MIE) : "memory");
			while (1);
			break;
		default:
			xprintf("%s: unexpected exception: 0x%lx\n",
				__func__, mcause);
			htif_exit(2);
			break;
	}

	return;
}

int main(void)
{
	xdev_out(htif_putc);

	*REG_MTIMECMP = (-1UL);

	__asm__ volatile ("csrs mie, %0" : : "rK"(MIE_MTIE | MIE_MSIE) : "memory");
	__asm__ volatile ("csrsi mstatus, %0" : : "K"(MSTATUS_MIE) : "memory");

	*REG_MTIMECMP = ((*REG_MTIME) + 5000000);

	while(1) {
		xprintf("%s: mtimecmp[%lu] mtime[%lu]\n",
			__func__, *REG_MTIMECMP, *REG_MTIME);
	}

	htif_exit(2);
}
