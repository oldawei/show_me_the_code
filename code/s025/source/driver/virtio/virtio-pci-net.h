#ifndef VIRTIO_PCI_NET_H_
#define VIRTIO_PCI_NET_H_

#include "types.h"

int virtio_pci_net_init(void);
void virtio_pci_net_cfg(void);
int virtio_pci_net_tx(u8 *buf, int buf_len);
int virtio_pci_net_rx(u8 *buf);
int virtio_pci_net_cfg_isr(int irq);
int virtio_pci_net_intr(int irq);
int virtio_pci_net_tx_isr(int irq);
int virtio_pci_net_close(void);

#endif /* VIRTIO_PCI_NET_H_ */
