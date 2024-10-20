#include "riscv.h"
#include "riscv-virt.h"
#include "printf/printf.h"
#include "clint.h"
#include "cfiflash.h"
#include "virtio-rng.h"
#include "virtio-blk.h"
#include "virtio-net.h"
#include "rtc-goldfish.h"
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOS_UDP_IP.h"
#include "FreeRTOS_Sockets.h"
#include "core_sntp_config.h"


void virtio_mmio_rng_test(void);
void virtio_mmio_blk_test(void);
void virtio_mmio_net_test(void);
void cli_init(void);
void fat_init(void);
u32 fat_read_counter(void);
void net_init_all(void);
void print_task_list( void );

extern void freertos_risc_v_trap_handler( void );
extern void freertos_vector_table( void );
void initializeSystemClock( void );

u32 version1 = BUILD_DATE1;
u32 version2 = BUILD_DATE2;
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
	printf("  version1 is: %d\n", version1);
	printf("  version2 is: %06x\n", version2);
	printf("  pointer is: %p\n", &version1);
	printf("  hartid is: %lu\n", cpuid());
	printf("  mstatus is: %lu\n", read_csr(mstatus));
	printf("  rdtime is: %lu\n", rdtime());
	printf("  rdcycle is: %lu\n", rdcycle());
	printf("  rdinstret is: %lu\n", rdinstret());
	printf("  %s\n", hello);
	printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");

	//clint_add_mtimecmp(D_CLOCK_RATE, 0);
	//clint_set_msip(1, 0);

	//virtio_mmio_rng_test();
	//virtio_mmio_blk_test();
	//virtio_mmio_net_test();


#if 0 //( mainVECTOR_MODE_DIRECT == 1 )
	{
		__asm__ volatile( "csrw mtvec, %0" :: "r"( freertos_risc_v_trap_handler ) );
	}
#else
	{
		__asm__ volatile( "csrw mtvec, %0" :: "r"( ( uintptr_t )freertos_vector_table | 0x1 ) );
	}
#endif

	/* Create the SNTP client task that is responsible for synchronizing system time with the time servers
     * periodically. This is created as a high priority task to keep the SNTP client operation unhindered. */
    xTaskCreate( sntpTask,                 /* Function that implements the task. */
                 "SntpClientTask",         /* Text name for the task - only used for debugging. */
                 512,                      /* Size of stack (in words, not bytes) to allocate for the task. */
                 NULL,                     /* Task parameter - not used in this case. */
                 configMAX_PRIORITIES - 1, /* Task priority, must be between 0 and configMAX_PRIORITIES - 1. */
                 NULL );

    /* Create the task that represents an application needing wall-clock time. */
    xTaskCreate( sampleAppTask,            /* Function that implements the task. */
                 "SampleAppTask",          /* Text name for the task - only used for debugging. */
                 512,                      /* Size of stack (in words, not bytes) to allocate for the task. */
                 NULL,                     /* Task parameter - not used in this case. */
                 tskIDLE_PRIORITY,         /* Task priority, must be between 0 and configMAX_PRIORITIES - 1. */
                 NULL );                   /* Used to pass out a handle to the created task - not used in this case. */


	cli_init();
	virtio_blk_init(VIRTIO1_ADDR);
	net_init_all();
	initializeSystemClock();
    vTaskStartScheduler();

	while(1)
		;

	return 0;
}
