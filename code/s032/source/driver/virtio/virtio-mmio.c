#include "riscv.h"
#include "virtio.h"
#include "virtio-mmio.h"
#include "printf/printf.h"

//static u32 gs_virtio_mmio_base = 0;


#define VTMO_ADDR(base, addr)     ((u32)(base) + (u32)(addr))
#define VTMO_REG(base, reg)       (*(volatile u32 *)VTMO_ADDR(base, reg))
#define VTMO_REG8(base, reg)       (*(volatile u8 *)VTMO_ADDR(base, reg))

void virtio_mmio_init(u32 base)
{
    //gs_virtio_mmio_base = base;

    printf("virtio_mmio_init:\n");
    printf("0x%08x\n", virtio_mmio_read_reg(base, VIRTIO_MMIO_MAGIC_VALUE));
    printf("0x%08x\n", virtio_mmio_read_reg(base, VIRTIO_MMIO_VERSION));
    printf("0x%08x\n", virtio_mmio_read_reg(base, VIRTIO_MMIO_DEVICE_ID));
    printf("0x%08x\n", virtio_mmio_read_reg(base, VIRTIO_MMIO_VENDOR_ID));
}

u32 virtio_mmio_read_reg(u32 base, u32 addr)
{
    return VTMO_REG(base, addr);
}

u8 virtio_mmio_read_reg8(u32 base, u32 addr)
{
    return VTMO_REG8(base, addr);
}

u32 virtio_mmio_get_status(u32 base)
{
    return VTMO_REG(base, VIRTIO_MMIO_STATUS);
}

void virtio_mmio_set_status(u32 base, u32 status)
{
    VTMO_REG(base, VIRTIO_MMIO_STATUS) = status;
}

void virtio_mmio_reset_device(u32 base)
{
	virtio_mmio_set_status(base, 0);
}

u64 virtio_mmio_get_host_features(u32 base)
{
    // Read the full 64-bit device features field
	u64 features = 0;
	VTMO_REG(base, VIRTIO_MMIO_HOST_FEATURES_SEL) = 0;
	dsb();
	features = VTMO_REG(base, VIRTIO_MMIO_HOST_FEATURES);

	VTMO_REG(base, VIRTIO_MMIO_HOST_FEATURES_SEL) = 1;
	dsb();
	features |= ((u64)VTMO_REG(base, VIRTIO_MMIO_HOST_FEATURES) << 32);

	return features;
}

void virtio_mmio_set_guest_features(u32 base, u64 features)
{
    u32 f0 = features & 0xFFFFFFFF;
    u32 f1 = (features >> 32) & 0xFFFFFFFF;

    VTMO_REG(base, VIRTIO_MMIO_HOST_FEATURES_SEL) = 0;
	dsb();
    VTMO_REG(base, VIRTIO_MMIO_GUEST_FEATURES) = f0;

    VTMO_REG(base, VIRTIO_MMIO_HOST_FEATURES_SEL) = 1;
	dsb();
    VTMO_REG(base, VIRTIO_MMIO_GUEST_FEATURES) = f1;
}

int virtio_mmio_get_queue_ready(u32 base, int qnum)
{
    VTMO_REG(base, VIRTIO_MMIO_QUEUE_SEL) = qnum;
	dsb();

    return VTMO_REG(base, VIRTIO_MMIO_QUEUE_READY);
}

void virtio_mmio_set_queue_ready(u32 base, int qnum)
{
    VTMO_REG(base, VIRTIO_MMIO_QUEUE_SEL) = qnum;
	dsb();

    VTMO_REG(base, VIRTIO_MMIO_QUEUE_READY) = 1;
}

int virtio_mmio_get_queue_size(u32 base, int qnum)
{
    VTMO_REG(base, VIRTIO_MMIO_QUEUE_SEL) = qnum;
	dsb();
    return VTMO_REG(base, VIRTIO_MMIO_QUEUE_NUM_MAX);

    // u64 features = virtio_mmio_get_host_features();
    // if (features & VIRTIO_F_VERSION_1) {
    //     return VTMO_REG(base, VIRTIO_MMIO_QUEUE_NUM);
    // } else {
    //     return VTMO_REG(base, VIRTIO_MMIO_QUEUE_NUM_MAX);
    // }
}

void virtio_mmio_set_queue_size(u32 base, int qnum, u32 qsize)
{
    VTMO_REG(base, VIRTIO_MMIO_QUEUE_SEL) = qnum;
	dsb();

    VTMO_REG(base, VIRTIO_MMIO_QUEUE_NUM) = qsize;
}

void virtio_mmio_set_queue_addr(u32 base, int qnum, struct vring *vr)
{
    VTMO_REG(base, VIRTIO_MMIO_QUEUE_SEL) = qnum;
	dsb();

    VTMO_REG(base, VIRTIO_MMIO_QUEUE_DESC_LOW) = (u32)vr->desc;
    VTMO_REG(base, VIRTIO_MMIO_QUEUE_DESC_HIGH) = 0;
    //VTMO_REG(base, VIRTIO_MMIO_QUEUE_DESC_HIGH) = (u64)vr->desc >> 32;
    VTMO_REG(base, VIRTIO_MMIO_QUEUE_AVAIL_LOW) = (u32)vr->avail;
    VTMO_REG(base, VIRTIO_MMIO_QUEUE_AVAIL_HIGH) = 0;
    //VTMO_REG(base, VIRTIO_MMIO_QUEUE_AVAIL_HIGH) = (u64)vr->avail >> 32;
    VTMO_REG(base, VIRTIO_MMIO_QUEUE_USED_LOW) = (u32)vr->used;
    VTMO_REG(base, VIRTIO_MMIO_QUEUE_USED_HIGH) = 0;
    //VTMO_REG(base, VIRTIO_MMIO_QUEUE_USED_HIGH) = (u64)vr->used >> 32;
}

void virtio_mmio_set_notify(u32 base, int qnum)
{
    VTMO_REG(base, VIRTIO_MMIO_QUEUE_NOTIFY) = qnum;
	dsb();
}

void virtio_mmio_set_ack(u32 base)
{
    u32 status = VTMO_REG(base, VIRTIO_MMIO_INTERRUPT_STATUS);
    VTMO_REG(base, VIRTIO_MMIO_INTERRUPT_ACK) = status & 0x3;
	dsb();
}