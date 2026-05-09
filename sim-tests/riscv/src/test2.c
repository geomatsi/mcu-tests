#include <stdint.h>

#include "xprintf.h"
#include "htif.h"

#define ILL_INSN (0x00000000)

void m_trap_handler(void) __attribute__((interrupt("machine")));
void m_trap_handler(void)
{
	unsigned long mcause, mtval, mepc;
	int ret = 0;

	__asm__ volatile ("csrr %0, mcause\n\t"
		: "=r" (mcause) : : );
	__asm__ volatile ("csrr %0, mtval\n\t"
		: "=r" (mtval) : : );
	__asm__ volatile ("csrr %0, mepc\n\t"
		: "=r" (mepc) : : );

	xprintf("%s: mcause(0x%lx) mtval(0x%lx) mepc(0x%lx)\n", 
			__func__, mcause, mtval, mepc);

	switch (mcause) {
		case 2: /* ILLEGAL_INSTRUCTION */
			xprintf("%s: illegal instruction exception\n",
				__func__);
			break;
		default:
			xprintf("%s: unexpected exception: 0x%lx...\n",
				__func__, mcause);
			ret = 3;
			break;
	}

	htif_exit(ret);
}

int main(void)
{
	xdev_out(htif_putc);

	xprintf("%s: Expelliarmus !\n", __func__);
	__asm__ volatile (".word %0" : : "i"(ILL_INSN) : );

	htif_exit(0);
}
