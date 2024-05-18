#include <string.h>
#include "riscv.h"
#include "riscv-virt.h"
#include "ns16550.h"
#include "clint.h"
#include "plic.h"

void plt_virt_init(void)
{
	UartInit(NS16550_ADDR);
	clint_irq_init();
	plic_init();
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

void putchar(char c)
{
	UartPutc( c );
}