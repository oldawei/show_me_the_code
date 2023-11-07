#ifndef PLIC_H_
#define PLIC_H_

#define VIRTIO0_IRQ 1 // rng
#define VIRTIO1_IRQ 2 // blk
#define VIRTIO2_IRQ 3 // net
#define UART0_IRQ   10

extern void plic_init(void);
void plic_enable(int irq);
extern int plic_claim(void);
extern void plic_complete(int irq);

#endif /* PLIC_H_ */
