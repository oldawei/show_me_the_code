#include "riscv.h"
#include "riscv-virt.h"
#include "printf/printf.h"
#include "clint.h"
#include "rtc-goldfish.h"

void goldfish_rtc_test(void);

int version = 20230917;
char *hello = "Hello, qemu and risc-v!";
int main( void )
{
	plt_virt_init();

	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf("  version is: %d\n", version);
	printf("  version is: 0x%08x\n", version);
	printf("  pointer is: %p\n", &version);
	printf("  hartid is: %lu\n", cpuid());
	printf("  mstatus is: %lu\n", read_csr(mstatus));
	printf("  rdtime is: %lu\n", rdtime());
	printf("  rdcycle is: %lu\n", rdcycle());
	printf("  rdinstret is: %lu\n", rdinstret());
	printf("  %s\n", hello);
	printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");

	clint_add_mtimecmp(D_CLOCK_RATE, 0);
	clint_set_msip(1, 0);
	goldfish_rtc_test();

	while(1)
		;

	return 0;
}

void goldfish_rtc_test(void)
{
	struct rtc_time tm;

	printf("\n");
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

	goldfish_rtc_init(RTC0_ADDR);

	u64 sec = goldfish_rtc_read_time_sec();
	rtc_time64_to_tm(sec, &tm);
	printf("  %04d-%02d-%02d %02d:%02d:%02d\n",
		YEAR(tm.tm_year), MONTH(tm.tm_mon), tm.tm_mday,
		HOUR(tm.tm_hour), tm.tm_min, tm.tm_sec
		);

	// wait one second
	while (goldfish_rtc_read_time_sec() == sec)
		;

	goldfish_rtc_read_time(&tm);
	printf("  %04d-%02d-%02d %02d:%02d:%02d\n",
		YEAR(tm.tm_year), MONTH(tm.tm_mon), tm.tm_mday,
		HOUR(tm.tm_hour), tm.tm_min, tm.tm_sec
		);

	printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
}