#include "riscv.h"
#include "printf/printf.h"
#include "pci.h"
#include "virtio-pci.h"

/*
pci bar layout:
type 	region 		offset 	length	bar
0x01	COMMON_CFG	0x0000	0x1000	4
0x03	ISR_CFG		0x1000	0x1000	4
0x04	DEVICE_CFG	0x2000	0x1000	4
0x02	NOTIFY_CFG	0x3000	0x1000	4
*/

// virtio-net Feature Bits
#define VIRTIO_NET_F_CSUM 					(0)
#define VIRTIO_NET_F_GUEST_CSUM 			(1)
#define VIRTIO_NET_F_CTRL_GUEST_OFFLOADS 	(2)
#define VIRTIO_NET_F_MTU					(3)
#define VIRTIO_NET_F_MAC 					(5)
#define VIRTIO_NET_F_GUEST_TSO4 			(7)
#define VIRTIO_NET_F_GUEST_TSO6 			(8)
#define VIRTIO_NET_F_GUEST_ECN  			(9)
#define VIRTIO_NET_F_GUEST_UFO  			(10)
#define VIRTIO_NET_F_HOST_TSO4  			(11)
#define VIRTIO_NET_F_HOST_TSO6  			(12)
#define VIRTIO_NET_F_HOST_ECN   			(13)
#define VIRTIO_NET_F_HOST_UFO   			(14)
#define VIRTIO_NET_F_MRG_RXBUF  			(15)
#define VIRTIO_NET_F_STATUS    				(16)
#define VIRTIO_NET_F_CTRL_VQ   				(17)
#define VIRTIO_NET_F_CTRL_RX   				(18)
#define VIRTIO_NET_F_CTRL_VLAN 				(19)
#define VIRTIO_NET_F_GUEST_ANNOUNCE			(21)
#define VIRTIO_NET_F_MQ						(22)
#define VIRTIO_NET_F_CTRL_MAC_ADDR			(23)
#define VIRTIO_NET_F_HOST_USO 				(56)
#define VIRTIO_NET_F_HASH_REPORT 			(57)
#define VIRTIO_NET_F_GUEST_HDRLEN			(59)
#define VIRTIO_NET_F_RSS					(60)
#define VIRTIO_NET_F_RSC_EXT				(61)
#define VIRTIO_NET_F_STANDBY				(62)
#define VIRTIO_NET_F_SPEED_DUPLEX			(63)
// Reserved Feature Bits
#define VIRTIO_F_INDIRECT_DESC 				(28)
#define VIRTIO_F_EVENT_IDX					(29)
#define VIRTIO_F_VERSION_1					(32)
#define VIRTIO_F_ACCESS_PLATFORM			(33)
#define VIRTIO_F_RING_PACKED				(34)
#define VIRTIO_F_IN_ORDER					(35)
#define VIRTIO_F_ORDER_PLATFORM				(36)
#define VIRTIO_F_SR_IOV						(37)
#define VIRTIO_F_NOTIFICATION_DATA			(38)
#define VIRTIO_F_NOTIF_CONFIG_DATA			(39)
#define VIRTIO_F_RING_RESET					(40)

/* Status byte for guest to report progress. */
#define VIRTIO_CONFIG_STATUS_RESET          0x00
#define VIRTIO_CONFIG_STATUS_ACK            0x01
#define VIRTIO_CONFIG_STATUS_DRIVER         0x02
#define VIRTIO_CONFIG_STATUS_DRIVER_OK      0x04
#define VIRTIO_CONFIG_STATUS_FEATURES_OK    0x08
#define VIRTIO_CONFIG_STATUS_FAILED         0x80

#define PCI_REG8(reg)   (*(volatile u8 *)(reg))
#define PCI_REG16(reg)  (*(volatile u16 *)(reg))
#define PCI_REG32(reg)  (*(volatile u32 *)(reg))

struct virtio_pci_hw g_hw = { 0 };

static void *get_cfg_addr(u32 pci_base, struct virtio_pci_cap *cap)
{
    u32 reg = pci_base + PCI_ADDR_BAR0 + 4 * cap->bar;
    return (void *)((pci_config_read32(reg) & 0xFFFFFFF0) + cap->offset);
}

int virtio_pci_read_caps(u32 pci_base)
{
    struct virtio_pci_cap cap;
    u32 pos = 0;
    struct virtio_pci_hw *hw = &g_hw;

    pos = pci_config_read8(pci_base + PCI_ADDR_CAP);
    printf("cap: 0x%08x\n", pci_base + PCI_ADDR_CAP);

    while (pos) {
        printf("pos: 0x%02x\n", pos);
        pos += pci_base;
        printf("cap: 0x%08x\n", pos);
        pci_config_read(&cap, sizeof(cap), pos);

        if (cap.cap_vndr == PCI_CAP_ID_MSIX) {
            /* Transitional devices would also have this capability,
			 * that's why we also check if msix is enabled.
			 * 1st byte is cap ID; 2nd byte is the position of next
			 * cap; next two bytes are the flags.
			 */
			u16 flags = ((u16 *)&cap)[1];
            printf("flags: 0x%04x\n", flags);
        }

        if (cap.cap_vndr != PCI_CAP_ID_VNDR) {
			printf("[%2x] skipping non VNDR cap id: %02x\n",
				    pos, cap.cap_vndr);
			goto next;
		}

        printf("[%2x] cfg type: %u, bar: %u, offset: %04x, len: %u\n",
			    pos, cap.cfg_type, cap.bar, cap.offset, cap.length);

        switch (cap.cfg_type) {
		case VIRTIO_PCI_CAP_COMMON_CFG:
			hw->common_cfg = get_cfg_addr(pci_base, &cap);
			break;
		case VIRTIO_PCI_CAP_NOTIFY_CFG:
			pci_config_read(&hw->notify_off_multiplier,
					4, pos + sizeof(cap));
            hw->notify_cfg = get_cfg_addr(pci_base, &cap);
			break;
		case VIRTIO_PCI_CAP_DEVICE_CFG:
			hw->device_cfg = get_cfg_addr(pci_base, &cap);
			break;
		case VIRTIO_PCI_CAP_ISR_CFG:
			hw->isr_cfg = get_cfg_addr(pci_base, &cap);
			break;
		}
next:
		pos = cap.cap_next;
    }

    if (hw->common_cfg == NULL || hw->notify_cfg == NULL ||
	    hw->device_cfg == NULL    || hw->isr_cfg == NULL) {
		printf("no modern virtio pci device found.\n");
		return -1;
	}

    printf("found modern virtio pci device.\n");
	printf("common cfg mapped at: %p\n", hw->common_cfg);
    printf("isr cfg mapped at: %p\n", hw->isr_cfg);
	printf("device cfg mapped at: %p\n", hw->device_cfg);
	printf("notify base: %p, notify off multiplier: %u\n", hw->notify_cfg, hw->notify_off_multiplier);

    return 0;
}

u64 virtio_pci_get_device_features(void)
{
    struct virtio_pci_common_cfg *hw = g_hw.common_cfg;

    PCI_REG32(&hw->device_feature_select) = 0;
    u64 f1 = PCI_REG32(&hw->device_feature);

    PCI_REG32(&hw->device_feature_select) = 1;
    u64 f2 = PCI_REG32(&hw->device_feature);

    return (f2 << 32) | f1;
}

u64 virtio_pci_get_driver_features(void)
{
    struct virtio_pci_common_cfg *hw = g_hw.common_cfg;

    PCI_REG32(&hw->driver_feature_select) = 0;
    u64 f1 = PCI_REG32(&hw->driver_feature);

    PCI_REG32(&hw->driver_feature_select) = 1;
    u64 f2 = PCI_REG32(&hw->driver_feature);

    return (f2 << 32) | f1;
}

void virtio_pci_set_driver_features(u64 features)
{
    struct virtio_pci_common_cfg *hw = g_hw.common_cfg;
    PCI_REG32(&hw->driver_feature_select) = 0;
    dsb();
    PCI_REG32(&hw->driver_feature) = features & 0xFFFFFFFF;

    PCI_REG32(&hw->driver_feature_select) = 1;
    dsb();
    PCI_REG32(&hw->driver_feature) = features >> 32;
}

u32 virtio_pci_get_queue_size(int qid)
{
    struct virtio_pci_common_cfg *hw = g_hw.common_cfg;

    PCI_REG16(&hw->queue_select) = qid;
    dsb();

    return PCI_REG16(&hw->queue_size);
}

void virtio_pci_set_queue_size(int qid, int qsize)
{
    struct virtio_pci_common_cfg *hw = g_hw.common_cfg;

    PCI_REG16(&hw->queue_select) = qid;
    dsb();

    PCI_REG16(&hw->queue_size) = qsize;
}

void virtio_pci_set_queue_addr(int qid, struct vring *vr)
{
    struct virtio_pci_common_cfg *hw = g_hw.common_cfg;

    PCI_REG16(&hw->queue_select) = qid;
    dsb();

    PCI_REG32(&hw->queue_desc_lo) = (u32)vr->desc;
    PCI_REG32(&hw->queue_avail_lo) = (u32)vr->avail;
    PCI_REG32(&hw->queue_used_lo) = (u32)vr->used;
}

u32 virtio_pci_get_queue_notify_off(int qid)
{
    struct virtio_pci_common_cfg *hw = g_hw.common_cfg;

    PCI_REG16(&hw->queue_select) = qid;
    dsb();

    return PCI_REG16(&hw->queue_notify_off);
}

void *virtio_pci_get_queue_notify_addr(int qid)
{
    struct virtio_pci_common_cfg *hw = g_hw.common_cfg;

    PCI_REG16(&hw->queue_select) = qid;
    dsb();

    u16 notify_off = PCI_REG16(&hw->queue_notify_off);
    return (void *)((u32)g_hw.notify_cfg + notify_off * g_hw.notify_off_multiplier);
}

void virtio_pci_set_queue_notify(int qid)
{
    void *pt = virtio_pci_get_queue_notify_addr(qid);
    PCI_REG32(pt) = 1;
}

void virtio_pci_set_queue_enable(int qid)
{
    struct virtio_pci_common_cfg *hw = g_hw.common_cfg;

    PCI_REG16(&hw->queue_select) = qid;
    dsb();

    PCI_REG16(&hw->queue_enable) = 1;;
}

u16 virtio_pci_get_queue_enable(int qid)
{
    struct virtio_pci_common_cfg *hw = g_hw.common_cfg;

    PCI_REG16(&hw->queue_select) = qid;
    dsb();

    return PCI_REG16(&hw->queue_enable);
}

void virtio_pci_disable_queue_msix(int qid)
{
    struct virtio_pci_common_cfg *hw = g_hw.common_cfg;

    PCI_REG16(&hw->queue_select) = qid;
    dsb();

    PCI_REG16(&hw->queue_msix_vector) = 0xffff;
}

u32 virtio_pci_clear_isr(void)
{
    u32 irq = PCI_REG32(g_hw.isr_cfg);
    return irq;
}

u8 virtio_pci_get_status(void)
{
    struct virtio_pci_common_cfg *hw = g_hw.common_cfg;
    return PCI_REG8(&hw->device_status);
}

void virtio_pci_set_status(u8 status)
{
    struct virtio_pci_common_cfg *hw = g_hw.common_cfg;
    PCI_REG8(&hw->device_status) = status;
}

int virtio_pci_negotiate_driver_features(u64 features)
{
    int status;
	u64 device_features = 0;

	/* Negotiate features */
	device_features = virtio_pci_get_device_features();
	if (!(device_features & VIRTIO_F_VERSION_1)) {
		printf("Device does not support virtio 1.0 %llx\n", device_features);
		return -1;
	}

	//if (device_features & VIRTIO_F_ACCESS_PLATFORM)
	//	features |= VIRTIO_F_ACCESS_PLATFORM;

	virtio_pci_set_driver_features(features);
	device_features = virtio_pci_get_device_features();
	if ((device_features & features) != features) {
		printf("Features error %llx\n", features);
		return -2;
	}

	status = virtio_pci_get_status();
	status |= VIRTIO_CONFIG_STATUS_FEATURES_OK;
	virtio_pci_set_status(status);

	/* Read back to verify the FEATURES_OK bit */
	status = virtio_pci_get_status();
	if ((status & VIRTIO_CONFIG_STATUS_FEATURES_OK) != VIRTIO_CONFIG_STATUS_FEATURES_OK)
		return -3;

	return 0;
}

void virtio_pci_print_common_cfg(void)
{
    volatile u32 *cap = (volatile u32 *)g_hw.common_cfg;
    for (int i = 0; i < sizeof(struct virtio_pci_common_cfg)/sizeof(u32); ++i) {
        printf("cap[%d]: 0x%08x\n", i, cap[i]);
    }

    for (int i = 0; i < 8; ++i) {
        u32 qsize = virtio_pci_get_queue_size(i);
        if (qsize == 0) continue;

        u32 notify_off = virtio_pci_get_queue_notify_off(i);
        u32 vsize = virtio_vring_size(qsize);
        printf("queue[%d] qsize: 0x%08x, vsize: 0x%08x, notify_off: 0x%08x\n", i, qsize, vsize, notify_off);
    }
    u64 features = virtio_pci_get_device_features();
    printf("features: 0x%016llx\n", features);
}

u32 virtio_pci_get_config(int offset, int size)
{
    u32 val = 0;
    u32 addr = (u32)g_hw.device_cfg;

    switch (size) {
	case 1:
		val = PCI_REG8(addr + offset);
		break;
	case 2:
		val = PCI_REG16(addr + offset);
		break;
	case 4:
		val = PCI_REG32(addr + offset);
		break;
	}

	return val;
}

int virtio_pci_setup_queue(struct vring *vr)
{
    u16 notify_off;
    u32 desc_addr, avail_addr, used_addr;
    struct virtio_pci_common_cfg *hw = g_hw.common_cfg;

	desc_addr = (u32)vr->desc;
	avail_addr = (u32)vr->avail;
	used_addr = (u32)vr->used;

    PCI_REG32(&hw->queue_select) = vr->qid;
    PCI_REG32(&hw->queue_desc_lo) = desc_addr;
    PCI_REG32(&hw->queue_avail_lo) = avail_addr;
    PCI_REG32(&hw->queue_used_lo) = used_addr;

    notify_off = PCI_REG16(&hw->queue_notify_off);
    vr->notify_addr = (void *)((u8 *)g_hw.notify_cfg +
				notify_off * g_hw.notify_off_multiplier);

    PCI_REG16(&hw->queue_enable) = 1;

	printf("queue %u addresses:\n", vr->qid);
	printf("\t desc_addr: 0x%08x\n", desc_addr);
	printf("\t aval_addr: 0x%08x\n", avail_addr);
	printf("\t used_addr: 0x%08x\n", used_addr);
	printf("\t notify addr: %p (notify offset: %u)\n",
		vr->notify_addr, notify_off);

	return 0;
}
