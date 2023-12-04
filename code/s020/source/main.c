#include "riscv.h"
#include "riscv-virt.h"
#include "printf/printf.h"
#include "aplic.h"
#include "imsic.h"
#include "aclint_mtimer.h"
#include "cfiflash.h"
#include "virtio-rng.h"
#include "virtio-blk.h"
#include "virtio-net.h"
#include "rtc-goldfish.h"
#include "pci.h"
#include "virtio-pci-net.h"

void virtio_pci_test(void);

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

	aclint_add_mtimecmp(D_CLOCK_RATE, 0);
	imsic_ipi_send(APLIC_SUPERVISOR, 0);

	virtio_pci_test();

	while(1)
		;

	return 0;
}

void virtio_pci_test(void)
{
	virtio_pci_net_init(PCIE0_MMIO);
}
