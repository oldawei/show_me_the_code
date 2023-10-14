#include "riscv.h"
#include "riscv-virt.h"
#include "printf/printf.h"
#include "clint.h"
#include "cfiflash.h"

void cfi_flash_test(void);

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
	cfi_flash_test();

	while(1)
		;

	return 0;
}

void cfi_flash_test(void)
{
	u32 pdrv = 0;
	int r = CfiFlashInit(pdrv, PFLASH1_ADDR);
	printf("r = %d\n", r);
	r = CfiFlashQuery(pdrv);
	printf("r = %d\n", r);

	u32 data[] = {
		0x11111111,
		0x22222222,
		0x33333333,
		0x44444444,
		0x55555555,
		0x66666666,
		0x77777777,
		0x88888888,
		0x99999999
	};
	u32 buf[9] = { 0x0 };
	u32 cfi_addr = 0;

	r = CfiFlashRead(pdrv, buf, cfi_addr, sizeof(buf));
	printf("r = %d\n", r);
	printf("read after boot:\n");
	for (int i = 0; i < 9; ++i) {
		printf("0x%08x\n", buf[i]);
	}

	r = CfiFlashErase(pdrv, cfi_addr);
	printf("r = %d\n", r);

	r = CfiFlashRead(pdrv, buf, cfi_addr, sizeof(buf));
	printf("r = %d\n", r);
	printf("read after erase:\n");
	for (int i = 0; i < 9; ++i) {
		printf("0x%08x\n", buf[i]);
	}

	r = CfiFlashWrite(pdrv, data, cfi_addr, sizeof(data));
	printf("r = %d\n", r);

	r = CfiFlashRead(pdrv, buf, cfi_addr, sizeof(buf));
	printf("r = %d\n", r);
	printf("read after write:\n");
	for (int i = 0; i < 9; ++i) {
		printf("0x%08x\n", buf[i]);
	}
}