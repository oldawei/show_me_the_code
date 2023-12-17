#ifndef VIRTIO_PCI_NET_H_
#define VIRTIO_PCI_NET_H_

#include "types.h"

int virtio_pci_net_init(u32 base);
int virtio_pci_net_tx(u8 *buf, int buf_len);
int virtio_pci_net_rx(u8 *buf);
int virtio_pci_net_intr(void);
int virtio_pci_net_close(void);

#endif /* VIRTIO_PCI_NET_H_ */
