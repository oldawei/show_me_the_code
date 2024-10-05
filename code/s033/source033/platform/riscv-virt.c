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

#include "rtc-goldfish.h"
u32 calc_mtimer_clk(void)
{
	u64 cycle0 = clint_get_mtime();
	u64 tic = goldfish_rtc_read_time_usec();
	u64 toc = tic + 1000000; // 1s
	while(goldfish_rtc_read_time_usec() < toc);
	u64 cycle1 = clint_get_mtime();

	u32 mhz = (u32)(cycle1 - cycle0)/1000/1000;
	return mhz;
}

u32 calc_cpu_clk(void)
{
	u64 cycle0 = rdcycle();
	u64 tic = goldfish_rtc_read_time_usec();
	u64 toc = tic + 1000000; // 1s
	while(goldfish_rtc_read_time_usec() < toc);
	u64 cycle1 = rdcycle();

	u32 mhz = (u32)(cycle1 - cycle0)/1000/1000;
	return mhz;
}
