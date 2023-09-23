#include "riscv-virt.h"

int main( void )
{
	plt_virt_init();
	println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
	println("  Hello, qemu and risc-v!");
	println("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");

	return 0;
}
