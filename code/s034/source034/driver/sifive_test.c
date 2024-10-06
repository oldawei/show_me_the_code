#include "types.h"

#define SIFIVE_TEST_BASE			(0x100000)
#define FINISHER_FAIL				0x3333
#define FINISHER_PASS				0x5555
#define FINISHER_RESET				0x7777

#define RESET_TYPE_SHUTDOWN			0x0
#define RESET_TYPE_COLD_REBOOT		0x1
#define RESET_TYPE_WARM_REBOOT		0x2

#define RESET_REASON_NONE			0x0
#define RESET_REASON_SYSFAIL		0x1

static inline void __raw_writew(u16 val, volatile void *addr)
{
	__asm volatile("sh %0, 0(%1)" : : "r"(val), "r"(addr));
}

#define __io_br()	do {} while (0)
#define __io_ar()	__asm__ __volatile__ ("fence i,r" : : : "memory");
#define __io_bw()	__asm__ __volatile__ ("fence w,o" : : : "memory");
#define __io_aw()	do {} while (0)
#define writew(v,c)	({ __io_bw(); __raw_writew((u16)(v),(volatile void *)(c)); __io_aw(); })

void sifive_test_system_reset(u32 type, u32 reason)
{
	/*
	 * Tell the "finisher" that the simulation
	 * was successful so that QEMU exits
	 */
	switch (type) {
	case RESET_TYPE_SHUTDOWN:
		if (reason == RESET_REASON_NONE)
			writew(FINISHER_PASS, SIFIVE_TEST_BASE);
		else
			writew(FINISHER_FAIL, SIFIVE_TEST_BASE);
		break;
	case RESET_TYPE_COLD_REBOOT:
	case RESET_TYPE_WARM_REBOOT:
		writew(FINISHER_RESET, SIFIVE_TEST_BASE);
		break;
	}
}

void exit_qemu(void)
{
	sifive_test_system_reset(RESET_TYPE_SHUTDOWN, FINISHER_FAIL);
}

void reboot_qemu(void)
{
	sifive_test_system_reset(RESET_TYPE_COLD_REBOOT, FINISHER_PASS);
}
