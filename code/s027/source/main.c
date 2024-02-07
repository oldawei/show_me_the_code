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

void virtio_mmio_rng_test(void);
void virtio_mmio_blk_test(void);
void virtio_mmio_net_test(void);
void cli_init(void);

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
    while (1) {
    	//printf("test2_task\n");
    	vTaskDelay(10);
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

	xTaskCreate(test1_task, (portCHAR *)"test1_task", 512, NULL, 29, NULL);
    xTaskCreate(test2_task, (portCHAR *)"test2_task", 512, NULL, 30, NULL);
	cli_init();
    vTaskStartScheduler();

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

#define BLK_1K		(2*SECTOR_SZIE)
#define DATA_LEN	(64*BLK_1K) // 64KB
u32 wdata[DATA_LEN/4] = { 0 };
u32 rdata[DATA_LEN/4] = { 0 };
#define TEST_CNT (64*1024*1024/DATA_LEN) // 64MB
void virtio_mmio_blk_test(void)
{
	int r = virtio_blk_init(VIRTIO1_ADDR);
	printf("r: %d\n", r);

	int dlen = DATA_LEN;
	struct blk_buf req[1] = { 0 };

	for (int i = 0; i < dlen/4; ++i) {
		wdata[i] = 0xffffffff;
	}

	printf("blk test...");
	for (int n = 0; n < TEST_CNT; ++n) {
		//printf("==== n: %d ====\n", n);
		// blk write
		req[0].addr = n * dlen;
		req[0].data = wdata;
		req[0].data_len = dlen;
		req[0].is_write = 1;
		virtio_blk_rw(&req[0]);

		// reset rdata
		for (int j = 0; j < dlen/4; ++j) {
			rdata[j] = 0;
		}

		// blk read
		req[0].addr = n * dlen;
		req[0].data = rdata;
		req[0].data_len = dlen;
		req[0].is_write = 0;
		virtio_blk_rw(&req[0]);

		// check read data
		for (int j = 0; j < dlen/4; ++j) {
			if (rdata[j] != 0xffffffff) {
				printf("blk write or read failed\n");
				goto L1;
			}
		}
		//delay(1);
	}
	printf("passed!\n");
	return;

L1:
	printf("failed!\n");
}

void virtio_mmio_net_test(void)
{
	int r = virtio_net_init(VIRTIO2_ADDR);
	printf("r: %d\n", r);

	u8 buf[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0x08, 0x06, 0x00, 0x01,
		0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xa8, 0x01, 0x64 };
	printf("buf: %p\n", buf);
	for (int i = 0; i < 200; ++i) {
		r = virtio_net_tx(buf, sizeof(buf));
		//printf("r: %d\n", r);
	}
	printf("done\n");

	// u8 buf2[VIRTIO_NET_PKT_LEN] = { 0 };
	// for (int i = 0; i < 16; ++i) {
	// 	virtio_net_rx(buf2);
	// }
}
