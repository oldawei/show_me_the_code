#include <string.h>
#include "riscv.h"
#include "riscv-virt.h"
#include "ns16550.h"
#include "aclint_mswi.h"
#include "aplic.h"
#include "imsic.h"

void plt_virt_init(void)
{
	UartInit(NS16550_ADDR);
	aclint_irq_init();
	imsic_init();
	aplic_init(APLIC_DM_MSI); // msix mode
	aplic_enable_irq(APLIC_MACHINE, APLIC_DM_MSI, APLIC_UART0_IRQ, 1);
	aplic_enable_irq(APLIC_SUPERVISOR, APLIC_DM_MSI, APLIC_IPI_IRQ, 1);
}

void println( const char *s )
{
	u32 irq = save_irq();
	for (int i = 0; i < strlen(s); i++) {
		UartPutc( s[i] );
	}
	UartPutc( '\n' );
	restore_irq(irq);
}

void putchar_(char c)
{
	UartPutc( c );
}
