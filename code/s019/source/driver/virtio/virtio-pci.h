#ifndef VIRTIO_PCI_H_
#define VIRTIO_PCI_H_

#include "types.h"
#include "virtio-ring.h"

/* Common configuration */
#define VIRTIO_PCI_CAP_COMMON_CFG 1
/* Notifications */
#define VIRTIO_PCI_CAP_NOTIFY_CFG 2
/* ISR Status */
#define VIRTIO_PCI_CAP_ISR_CFG 3
/* Device specific configuration */
#define VIRTIO_PCI_CAP_DEVICE_CFG 4
/* PCI configuration access */
#define VIRTIO_PCI_CAP_PCI_CFG 5
/* Shared memory region */
#define VIRTIO_PCI_CAP_SHARED_MEMORY_CFG 8
/* Vendor-specific data */
#define VIRTIO_PCI_CAP_VENDOR_CFG 9

#define le64 u64
#define le32 u32
#define le16 u16
#define off_t u32
#pragma pack(1)
struct virtio_pci_cap {
    u8 cap_vndr; /* Generic PCI field: PCI_CAP_ID_VNDR */
    u8 cap_next; /* Generic PCI field: next ptr. */
    u8 cap_len; /* Generic PCI field: capability length */
    u8 cfg_type; /* Identifies the structure. */
    u8 bar; /* Where to find it. */
    u8 id; /* Multiple capabilities of the same type */
    u8 padding[2]; /* Pad to full dword. */
    le32 offset; /* Offset within bar. */
    le32 length; /* Length of the structure, in bytes. */
};
#pragma pack()
#undef le64
#undef le32
#undef le16
#undef off_t

struct virtio_pci_hw {
    u8      bar;
    u8	    use_msix;
	//u8      modern;
    u32     notify_off_multiplier;
    void    *common_cfg;
    void    *isr_cfg;
    void    *device_cfg;
    void    *notify_cfg;
    //struct pci_msix msix;
};

int virtio_pci_read_caps(u32 pci_base);

#endif /* VIRTIO_PCI_H_ */
