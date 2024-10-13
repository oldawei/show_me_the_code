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
#include "FreeRTOS_TCP_server.h"
#include "FreeRTOS_UDP_IP.h"
#include "FreeRTOS_Sockets.h"


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
void vStartNTPTask( uint16_t usTaskStackSize, UBaseType_t uxTaskPriority );

u32 version1 = BUILD_DATE1;
u32 version2 = BUILD_DATE2;
char *hello = "Hello, qemu and risc-v!";

void delay(int sec)
{
	u64 tic = goldfish_rtc_read_time_sec();
	u64 toc = tic + sec;
	while (goldfish_rtc_read_time_sec() < toc);
}

void test1_task(void *arg)
{
    (void)arg;

	printf("test1_task\n");
	print_task_list();
	clint_set_msip(1, 0);
    while (1) {
		//printf("test1_task\n");
		vTaskDelay(10);
	}
}

void test2_task(void *arg)
{
    (void)arg;

	printf("test2_task\n");
	clint_set_msip(1, 0);

	fat_init();
	fat_read_counter();

	vTaskDelay(1000/portTICK_RATE_MS);

	struct xSERVER_CONFIG cfg[] = {
		{
		.eType = eSERVER_FTP,
		.xPortNumber = 2121,
		.xBackLog = 8,
		.pcRootDir = "/oldawei"
		},
		{
		.eType = eSERVER_HTTP,
		.xPortNumber = 8080,
		.xBackLog = 8,
		.pcRootDir = "/oldawei/web_root"
		},
	};
	TCPServer_t *svr = FreeRTOS_CreateTCPServer(cfg, sizeof(cfg)/sizeof(cfg[0]));

    while (1) {
		FreeRTOS_TCPServerWork(svr, portMAX_DELAY);
    }
}

void ntp_task(void *arg)
{
    (void)arg;

	vTaskDelay(1000/portTICK_RATE_MS);

	vStartNTPTask(512, 20); // create NTP task

    while (1) {
		vTaskDelay(1000/portTICK_RATE_MS);
		vStartNTPTask(0, 0); // poll NTP task
    }
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

	xTaskCreate(test1_task, (portCHAR *)"test1_task", 512, NULL, 20, NULL);
    // xTaskCreate(test2_task, (portCHAR *)"test2_task", 768, NULL, 20, NULL);
	xTaskCreate(ntp_task, (portCHAR *)"NTP", 512, NULL, 20, NULL);

	cli_init();
	virtio_blk_init(VIRTIO1_ADDR);
	net_init_all();
    vTaskStartScheduler();

	while(1)
		;

	return 0;
}
