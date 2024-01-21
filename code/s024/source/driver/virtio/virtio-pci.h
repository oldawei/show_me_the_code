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

struct virtio_pci_notify_cap {
    struct virtio_pci_cap cap;
    le32 notify_off_multiplier; /* Multiplier for queue_notify_off. */
};

struct virtio_pci_common_cfg {
    /* About the whole device. */
    le32 device_feature_select; /* read-write */
    le32 device_feature; /* read-only for driver */
    le32 driver_feature_select; /* read-write */
    le32 driver_feature; /* read-write */
    le16 config_msix_vector; /* read-write */
    le16 num_queues; /* read-only for driver */
    u8 device_status; /* read-write */
    u8 config_generation; /* read-only for driver */
    /* About a specific virtqueue. */
    le16 queue_select; /* read-write */
    le16 queue_size; /* read-write */
    le16 queue_msix_vector; /* read-write */
    le16 queue_enable; /* read-write */
    le16 queue_notify_off; /* read-only for driver */
    // le64 queue_desc; /* read-write */
    le32 queue_desc_lo;		/* read-write */
	le32 queue_desc_hi;		/* read-write */
    // le64 queue_driver; /* read-write */
	le32 queue_avail_lo;	/* read-write */
	le32 queue_avail_hi;	/* read-write */
    // le64 queue_device; /* read-write */
	le32 queue_used_lo;		/* read-write */
	le32 queue_used_hi;		/* read-write */
    le16 queue_notify_data; /* read-only for driver */
    le16 queue_reset;       /* read-write */
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
    struct pci_msix msix;
};

enum virtio_msix_status {
	VIRTIO_MSIX_NONE = 0,
	VIRTIO_MSIX_DISABLED = 1,
	VIRTIO_MSIX_ENABLED = 2
};

int virtio_pci_read_caps(u32 pci_base);
u64 virtio_pci_get_device_features(void);
u64 virtio_pci_get_driver_features(void);
void virtio_pci_set_driver_features(u64 features);
u32 virtio_pci_get_queue_size(int qid);
void virtio_pci_set_queue_size(int qid, int qsize);
void virtio_pci_set_queue_addr(int qid, struct vring *vr);
u32 virtio_pci_get_queue_notify_off(int qid);
void *virtio_pci_get_queue_notify_addr(int qid);
void virtio_pci_set_queue_notify(int qid);
void virtio_pci_set_queue_enable(int qid);
u16 virtio_pci_get_queue_enable(int qid);
void virtio_pci_disable_queue_msix(int qid);
void virtio_pci_set_queue_msix(int qid, u16 msix_vector);
void virtio_pci_set_config_msix(u16 msix_vector);
u32 virtio_pci_clear_isr(void);
u8 virtio_pci_get_status(void);
void virtio_pci_set_status(u8 status);
int virtio_pci_negotiate_driver_features(u64 features);
u32 virtio_pci_get_config(int offset, int size);
int virtio_pci_setup_queue(struct vring *vr);
void virtio_pci_print_common_cfg(void);

#endif /* VIRTIO_PCI_H_ */
