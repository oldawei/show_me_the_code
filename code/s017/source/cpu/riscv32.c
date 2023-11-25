#include "riscv.h"
#include "riscv32.h"
#include "ns16550.h"
#include "printf/printf.h"
#include "aplic.h"
#include "imsic.h"
#include "aclint_mtimer.h"
#include "virtio-blk.h"
#include "virtio-net.h"

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
		case IRQ_S_SOFT: // s-mode soft
			//aclint_clr_ssip(); // clear soft isr
            printf("ssoft isr: mcause %d\n", mcause);
			break;

        case IRQ_M_SOFT: // soft
			//aclint_set_msip(0, 0); // clear soft isr
            printf("msoft isr: mcause %d\n", mcause);
			break;

		case IRQ_M_TIMER: // mtime
			aclint_add_mtimecmp(D_CLOCK_RATE, 0);
            ++g_sys_tick;
			// test only
			if ((g_sys_tick & 3) == 0)
				imsic_ipi_send(APLIC_SUPERVISOR, 0);
			// debug only
            printf("mtime: %d\n", g_sys_tick);
			break;

		case IRQ_M_EXT: // external
			handle_external_trap(APLIC_MACHINE);
			break;

		case IRQ_S_EXT: // external
			handle_external_trap(APLIC_SUPERVISOR);
			break;

		default:
			printf("unknow isr: %d\n", mcause);
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

void handle_external_trap(int mode)
{
	int irq = imsic_get_irq(mode);
	//printf("mode %d, irq: %d\n", mode, irq);
	if (irq == APLIC_UART0_IRQ) {
		UartIsr();
	} else if (irq == APLIC_VIRTIO1_IRQ) {
		virtio_blk_intr();
	} else if (irq == APLIC_VIRTIO2_IRQ) {
		virtio_net_intr();
	} else if (irq == APLIC_IPI_IRQ) {
		 printf("ipi: %d\n", irq);
	} else {
		printf("unknow external isr: %d\n", irq);
	}
}
