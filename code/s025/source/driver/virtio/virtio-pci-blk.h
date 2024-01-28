#ifndef VIRTIO_PCI_BLK_H_
#define VIRTIO_PCI_BLK_H_

#include "types.h"

int virtio_pci_blk_init(void);
void virtio_pci_blk_cfg(void);
void virtio_pci_blk_rw(struct blk_buf *b);
int virtio_pci_blk_cfg_isr(int irq);
int virtio_pci_blk_intr(int irq);
int virtio_pci_blk_close(void);

#endif /* VIRTIO_PCI_BLK_H_ */
