#include "riscv.h"
#include "clint.h"
#include "riscv32.h"
#include "printf/printf.h"

u32 g_sys_tick = 0;

u32 handle_trap(u32 mcause, u32 mepc)
{
	int is_interrupt = mcause & 0x80000000;

	//printf("mcause: 0x%08x\n", mcause);
	//printf("mepc: 0x%08x\n", mepc);
	//printf("is_interrupt: 0x%08x\n", is_interrupt);

	mcause = mcause & 0xFF;
	if (is_interrupt) {
		switch (mcause) {
        case IRQ_M_SOFT: // soft
			clint_set_msip(0, 0); // clear soft isr
            printf("soft isr: mcause %d\n", mcause);
			break;

		case IRQ_M_TIMER: // mtime
			clint_add_mtimecmp(D_CLOCK_RATE, 0);
            ++g_sys_tick;
			// test only
			if ((g_sys_tick & 3) == 0)
				clint_set_msip(1, 0);
			// debug only
            printf("mtime: %d\n", g_sys_tick);
			break;

		default:
			printf("unknow isr");
			break;
		}
	} else {
        printf("exception:\n");
        printf("mcause: 0x%08x\n", mcause);
        printf("mepc: 0x%08x\n", mepc);
    }

	//printf("mret");
	return is_interrupt ? mepc : (mepc + 4);
}