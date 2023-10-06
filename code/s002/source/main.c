#include "riscv.h"
#include "riscv-virt.h"
#include "printf/printf.h"

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

	return 0;
}
