#include <stdint.h>

#include "riscv_encoding.h"
#include "riscv_csr.h"
#include "xprintf.h"
#include "opcodes.h"
#include "aclint.h"
#include "htif.h"

void illegal(void)
{
#define ILL_INSN (0x0U)
	__asm__ volatile (".half %0" : : "i"(ILL_INSN) : );
}

void ebreak(void)
{
	__asm__ volatile ("ebreak" : : : );
}

void ecall(void)
{
	__asm__ volatile ("ecall" : : : );
}

void m_trap_handler(void) __attribute__((interrupt("machine")));
void m_trap_handler(void)
{
	unsigned long mcause, mtval, mepc;
	unsigned long t1 = 0;
	int step_over = 0;

	mcause = csr_read(mcause);
	mtval = csr_read(mtval);
	mepc = csr_read(mepc);

	xprintf("%s: mcause(0x%lx) mtval(0x%lx) mepc(0x%lx)\n", 
			__func__, mcause, mtval, mepc);

	switch (mcause) {
		case CAUSE_ILLEGAL_INSTRUCTION:
			step_over = insn_len(mepc);
			xprintf("%s: illegal instruction: mcause(0x%lx) len(%d)\n",
					__func__, mcause, step_over);
			break;
		case CAUSE_BREAKPOINT: 
			step_over = insn_len(mepc);
			xprintf("%s: breakpoint exception: mcause(0x%lx) len(%d)\n",
				__func__, mcause, step_over);
			break;
		case CAUSE_SUPERVISOR_ECALL: 
			step_over = insn_len(mepc);
			xprintf("%s: supervisor ecall exception: mcause(0x%lx) len(%d)\n",
				__func__, mcause, step_over);
			break;
		default:
			xprintf("%s: unexpected exception: 0x%lx\n",
					__func__, mcause);
			htif_exit(2);
			break;
	}

	if (step_over) {
		__asm__ volatile (
			"csrr %0, mepc\n\t"
			"add %0, %0, %1\n\t"
			"csrw	mepc, %0\n\t"
			: "+r"(t1), "+r"(step_over) : : );
	}

	return;
}

void s_trap_handler(void) __attribute__((interrupt("supervisor")));
void s_trap_handler(void)
{
	unsigned long scause, stval, sepc;
	unsigned long t1 = 0;
	int step_over = 0;
	unsigned long t2;

	scause = csr_read(scause);
	stval = csr_read(stval);
	sepc = csr_read(sepc);

	xprintf("%s: scause(0x%lx) stval(0x%lx) sepc(0x%lx)\n", 
			__func__, scause, stval, sepc);

	switch (scause) {
		case CAUSE_ILLEGAL_INSTRUCTION:
			step_over = insn_len(sepc);
			xprintf("%s: illegal instruction: scause(0x%lx) len(%d)\n",
					__func__, scause, step_over);

			/*
			 * problem:
			 * - both illinsn and ebreak are delegated to S-mode
			 * - so ebreak in s-handler will trap into the same (this) s-handler
			 * - so ebreak will rewrite sepc breaking return from the orig trap
			 * - so need save/restore sepc to fix return from the orig trap
			 */
			xprintf("%s: ebreak in s-mode trap handler, expect s-trap\n",
				__func__);
			t2 = sepc;
			ebreak();
			csr_write(sepc, t2);

			break;
		case CAUSE_BREAKPOINT: 
			step_over = insn_len(sepc);
			xprintf("%s: breakpoint exception: scause(0x%lx) len(%d)\n",
				__func__, scause, step_over);
			/* no problems:
			 * - ecall will trap into m-handler
			 * - m-handler uses mepc, so sepc will not be rewritten
			 * - no need to save/restore sepc for return from orig trap
			 */
			xprintf("%s: ecall in s-mode trap handler, expect m-trap\n",
				__func__);
			ecall();
			break;
		default:
			xprintf("%s: unexpected exception: 0x%lx\n",
					__func__, scause);
			htif_exit(3);
			break;
	}

	if (step_over) {
		__asm__ volatile (
			"csrr %0, sepc\n\t"
			"add %0, %0, %1\n\t"
			"csrw	sepc, %0\n\t"
			: "+r"(t1), "+r"(step_over) : : );
	}

	return;
}

void main_s_mode(void)
{
	xprintf("%s: continue in S-mode...\n", __func__);

	xprintf("%s: ecall -> expect m-trap...\n", __func__);
	ecall();

	xprintf("%s: ebreak -> expect s-trap...\n", __func__);
	ebreak();

	xprintf("%s: illegal insn -> expect s-trap...\n", __func__);
	illegal();

	xprintf("%s: all good...\n", __func__);
	htif_exit(0);
}

int main(void)
{
	unsigned long reg;

	xdev_out(htif_putc);

	xprintf("%s: start in M-mode...\n", __func__);

	/* disable paging */
	csr_write(satp, 0x0);

	/* disable interrupts delegations */
	csr_write(mideleg, 0x0);

	/* delegate ebreak and illegal insn handling to S-mode */
	csr_write(medeleg, BIT(CAUSE_BREAKPOINT) | BIT(CAUSE_ILLEGAL_INSTRUCTION));

	/* disable interrupts in M-mode */
	csr_write(mie, 0x0);
	csr_write(mip, 0x0);

	/* disable interrupts in S-mode */
	csr_write(sie, 0x0);
	csr_write(sip, 0x0);

	/* set MPP=M/S, MPIE=0, MIE=0, SIE=0, SPP=U */
	reg = csr_read(mstatus);
	reg &= ~(MSTATUS_MPP | MSTATUS_SPP | MSTATUS_MPIE | MSTATUS_MIE | MSTATUS_SIE);
	csr_write(mstatus, reg | (1 << 11));

	/* jump to S-mode */
	csr_write(mepc, main_s_mode);
	asm volatile("mret" ::: "memory");

	/* should not be here */
	xprintf("%s: failed to jump to S-mode...\n", __func__);
	htif_exit(1);
}
