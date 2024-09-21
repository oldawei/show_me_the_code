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

	xSocket_t xSocket;
	struct freertos_sockaddr xEchoServerAddress;
	char cTxString[ 64 ];
	volatile uint32_t ulTxCount = 0UL;

	xEchoServerAddress.sin_port = FreeRTOS_htons( 9999 );
	// 在家庭或办公室局域网内测试UDP组播功能，可以使用的组播地址范围是239.0.0.0~239.255.255.255
	xEchoServerAddress.sin_addr = FreeRTOS_inet_addr_quick(239, 10, 20, 30);

	/* Create a socket. */
	xSocket = FreeRTOS_socket( FREERTOS_AF_INET, FREERTOS_SOCK_DGRAM, FREERTOS_IPPROTO_UDP );
	configASSERT( xSocket != FREERTOS_INVALID_SOCKET );

	for( ;; ) {
		/* Create the string that is sent to the echo server. */
		sprintf( cTxString, "Message number %u\r\n", ( unsigned int ) ulTxCount );
		printf("%s\n", cTxString);

		/* Send the string to the socket.  ulFlags is set to 0, so the zero
		copy interface is not used.  That means the data from cTxString is
		copied into a network buffer inside FreeRTOS_sendto(), and cTxString
		can be reused as soon as FreeRTOS_sendto() has returned.  1 is added
		to ensure the NULL string terminator is sent as part of the message. */
		FreeRTOS_sendto( xSocket,				/* The socket being sent to. */
						( void * ) cTxString,	/* The data being sent. */
						strlen( cTxString ) + 1,/* The length of the data being sent. */
						0,						/* ulFlags with the FREERTOS_ZERO_COPY bit clear. */
						&xEchoServerAddress,	/* The destination address. */
						sizeof( xEchoServerAddress ) );

		ulTxCount++;
		vTaskDelay(1000/portTICK_RATE_MS);
	}
	/* Close this socket before looping back to create another. */
	FreeRTOS_closesocket( xSocket );
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
    xTaskCreate(test2_task, (portCHAR *)"test2_task", 512*2, NULL, 20, NULL);
	cli_init();
	virtio_blk_init(VIRTIO1_ADDR);
	net_init_all();
    vTaskStartScheduler();

	while(1)
		;

	return 0;
}
