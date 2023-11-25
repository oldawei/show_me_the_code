#include "riscv.h"
#include "virtio.h"
#include "virtio-ring.h"
#include "virtio-mmio.h"

static u32 gs_virtio_mmio_base = 0;


#define VTMO_ADDR(addr)     (gs_virtio_mmio_base + (addr))
#define VTMO_REG(reg)       (*(volatile u32 *)VTMO_ADDR(reg))
#define VTMO_REG8(reg)       (*(volatile u8 *)VTMO_ADDR(reg))

void virtio_mmio_init(u32 base)
{
    gs_virtio_mmio_base = base;
}

u32 virtio_mmio_read_reg(u32 addr)
{
    return VTMO_REG(addr);
}

u8 virtio_mmio_read_reg8(u32 addr)
{
    return VTMO_REG8(addr);
}

u32 virtio_mmio_get_status(void)
{
    return VTMO_REG(VIRTIO_MMIO_STATUS);
}

void virtio_mmio_set_status(u32 status)
{
    VTMO_REG(VIRTIO_MMIO_STATUS) = status;
}

void virtio_mmio_reset_device(void)
{
	virtio_mmio_set_status(0);
}

u64 virtio_mmio_get_host_features(void)
{
    // Read the full 64-bit device features field
	u64 features = 0;
	VTMO_REG(VIRTIO_MMIO_HOST_FEATURES_SEL) = 0;
	dsb();
	features = VTMO_REG(VIRTIO_MMIO_HOST_FEATURES);

	VTMO_REG(VIRTIO_MMIO_HOST_FEATURES_SEL) = 1;
	dsb();
	features |= ((u64)VTMO_REG(VIRTIO_MMIO_HOST_FEATURES) << 32);

	return features;
}

void virtio_mmio_set_guest_features(u64 features)
{
    u32 f0 = features & 0xFFFFFFFF;
    u32 f1 = (features >> 32) & 0xFFFFFFFF;

    VTMO_REG(VIRTIO_MMIO_HOST_FEATURES_SEL) = 0;
	dsb();
    VTMO_REG(VIRTIO_MMIO_GUEST_FEATURES) = f0;

    VTMO_REG(VIRTIO_MMIO_HOST_FEATURES_SEL) = 1;
	dsb();
    VTMO_REG(VIRTIO_MMIO_GUEST_FEATURES) = f1;
}

int virtio_mmio_get_queue_ready(int qnum)
{
    VTMO_REG(VIRTIO_MMIO_QUEUE_SEL) = qnum;
	dsb();

    return VTMO_REG(VIRTIO_MMIO_QUEUE_READY);
}

void virtio_mmio_set_queue_ready(int qnum)
{
    VTMO_REG(VIRTIO_MMIO_QUEUE_SEL) = qnum;
	dsb();

    VTMO_REG(VIRTIO_MMIO_QUEUE_READY) = 1;
}

int virtio_mmio_get_queue_size(int qnum)
{
    VTMO_REG(VIRTIO_MMIO_QUEUE_SEL) = qnum;
	dsb();
    return VTMO_REG(VIRTIO_MMIO_QUEUE_NUM_MAX);

    // u64 features = virtio_mmio_get_host_features();
    // if (features & VIRTIO_F_VERSION_1) {
    //     return VTMO_REG(VIRTIO_MMIO_QUEUE_NUM);
    // } else {
    //     return VTMO_REG(VIRTIO_MMIO_QUEUE_NUM_MAX);
    // }
}

void virtio_mmio_set_queue_size(int qnum, u32 qsize)
{
    VTMO_REG(VIRTIO_MMIO_QUEUE_SEL) = qnum;
	dsb();

    VTMO_REG(VIRTIO_MMIO_QUEUE_NUM) = qsize;
}

void virtio_mmio_set_queue_addr(int qnum, struct vring *vr)
{
    VTMO_REG(VIRTIO_MMIO_QUEUE_SEL) = qnum;
	dsb();

    VTMO_REG(VIRTIO_MMIO_QUEUE_DESC_LOW) = (u32)vr->desc;
    VTMO_REG(VIRTIO_MMIO_QUEUE_DESC_HIGH) = 0;
    //VTMO_REG(VIRTIO_MMIO_QUEUE_DESC_HIGH) = (u64)vr->desc >> 32;
    VTMO_REG(VIRTIO_MMIO_QUEUE_AVAIL_LOW) = (u32)vr->avail;
    VTMO_REG(VIRTIO_MMIO_QUEUE_AVAIL_HIGH) = 0;
    //VTMO_REG(VIRTIO_MMIO_QUEUE_AVAIL_HIGH) = (u64)vr->avail >> 32;
    VTMO_REG(VIRTIO_MMIO_QUEUE_USED_LOW) = (u32)vr->used;
    VTMO_REG(VIRTIO_MMIO_QUEUE_USED_HIGH) = 0;
    //VTMO_REG(VIRTIO_MMIO_QUEUE_USED_HIGH) = (u64)vr->used >> 32;
}

void virtio_mmio_set_notify(int qnum)
{
    VTMO_REG(VIRTIO_MMIO_QUEUE_NOTIFY) = qnum;
	dsb();
}

void virtio_mmio_set_ack(void)
{
    u32 status = VTMO_REG(VIRTIO_MMIO_INTERRUPT_STATUS);
    VTMO_REG(VIRTIO_MMIO_INTERRUPT_ACK) = status & 0x3;
	dsb();
}