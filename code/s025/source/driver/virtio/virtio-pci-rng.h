#ifndef VIRTIO_PCI_RNG_H_
#define VIRTIO_PCI_RNG_H_

#include "types.h"

int virtio_pci_rng_init(void);
int virtio_pci_rng_read(u8 *buf, int len);
int virtio_pci_rng_cfg_isr(int irq);
int virtio_pci_rng_intr(int irq);
int virtio_pci_rng_close(void);

#endif /* VIRTIO_PCI_RNG_H_ */
