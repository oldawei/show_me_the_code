#include "riscv.h"
#include "riscv-virt.h"
#include "printf/printf.h"
#include "clint.h"
#include "cfiflash.h"
#include "virtio-rng.h"
#include "rtc-goldfish.h"

void virtio_mmio_rng_test(void);

int version = 20230917;
char *hello = "Hello, qemu and risc-v!";

void delay(int sec)
{
	u64 tic = goldfish_rtc_read_time_sec();
	u64 toc = tic + sec;
	while (goldfish_rtc_read_time_sec() < toc);
}

int main( void )
{
	plt_virt_init();
	goldfish_rtc_init(RTC0_ADDR);

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

	virtio_mmio_rng_test();

	while(1)
		;

	return 0;
}

void virtio_mmio_rng_test(void)
{
	int r = virtio_rng_init(VIRTIO0_ADDR);
	printf("r: %d\n", r);

	u32 buf[4] = { 0 };

	for (int n = 0; n < 256; ++n){
		printf("==== %d ====\n", n);
		int rlen = virtio_rng_read((u8 *)buf, sizeof(buf));
		(void)rlen;
		//printf("rlen: %d\n", rlen);

		for (int i = 0; i < sizeof(buf)/sizeof(buf[0]); i += 4) {
			printf("0x%08x 0x%08x 0x%08x 0x%08x\n", buf[i], buf[i+1], buf[i+2], buf[i+3]);
		}

		for (int i = 0; i < sizeof(buf)/sizeof(buf[0]); ++i) {
			buf[i] = 0;
		}
	}

	printf("virtio-rng test passed!\n");
}