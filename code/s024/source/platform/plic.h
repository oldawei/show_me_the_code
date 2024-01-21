#ifndef PLIC_H_
#define PLIC_H_

#include "types.h"

#define VIRTIO0_IRQ     0x01 // rng
#define VIRTIO1_IRQ     0x02 // blk
#define VIRTIO2_IRQ     0x03 // net
#define PCIE_IRQ        0x21 // pci-net
#define UART0_IRQ       0x0a

void plic_init(void);
void plic_enable(int irq);
int plic_claim(void);
void plic_complete(int irq);
u64 plic_get_pending(void);

#endif /* PLIC_H_ */
