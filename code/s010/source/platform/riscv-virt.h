#ifndef RISCV_VIRT_H_
#define RISCV_VIRT_H_

#define NS16550_ADDR		0x10000000UL
#define CLINT0_ADDR		    0x02000000UL
#define PLIC0_ADDR		    0x0c000000UL
#define PFLASH0_ADDR		0x20000000UL
#define PFLASH1_ADDR		0x22000000UL
#define RTC0_ADDR		    0x00101000UL
#define VIRTIO0_ADDR        0x10001000UL
#define VIRTIO1_ADDR        (VIRTIO0_ADDR + 0x1000UL)

void plt_virt_init(void);
void println( const char *s );

#endif /* RISCV_VIRT_H_ */
