#ifndef PLIC_H_
#define PLIC_H_

#define VIRTIO0_IRQ     0x01 // rng
#define VIRTIO1_IRQ     0x02 // blk
#define VIRTIO2_IRQ     0x03 // net
#define UART0_IRQ       0x0a // uart

void plic_init(void);
void plic_enable(int irq);
void plic_disable(int irq);
int plic_claim(void);
void plic_complete(int irq);

#endif /* PLIC_H_ */
