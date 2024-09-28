#include "riscv.h"
#include "virtio.h"
#include "virtio-ring.h"
#include "virtio-mmio.h"
#include "virtio-rng.h"
#include "printf/printf.h"

static u8 gs_rng_buf[3*4096] __attribute__((aligned(4096))) = { 0 };
static u32 gs_virtio_base = 0;
static struct virtio_rng gs_virtio_rng = { 0 };

int virtio_rng_init(u32 base)
{
    gs_virtio_base = base;
    virtio_mmio_init(gs_virtio_base);

    if (virtio_mmio_read_reg(gs_virtio_base, VIRTIO_MMIO_MAGIC_VALUE) != 0x74726976 ||
        virtio_mmio_read_reg(gs_virtio_base, VIRTIO_MMIO_VERSION) != 2 ||
        virtio_mmio_read_reg(gs_virtio_base, VIRTIO_MMIO_DEVICE_ID) != 4 ||
        virtio_mmio_read_reg(gs_virtio_base, VIRTIO_MMIO_VENDOR_ID) != 0x554d4551) {
        printf("could not find virtio entropy\n");
        return -1;
    }

    // reset device
    virtio_mmio_reset_device(gs_virtio_base);

    u32 status = 0;
    // set ACKNOWLEDGE status bit
    status |= VIRTIO_STAT_ACKNOWLEDGE;
    virtio_mmio_set_status(gs_virtio_base, status);

    // set DRIVER status bit
    status |= VIRTIO_STAT_DRIVER;
    virtio_mmio_set_status(gs_virtio_base, status);

    // get features
    u64 features = virtio_mmio_get_host_features(gs_virtio_base);
    printf("device features: 0x%016llx\n", features);
    // negotiate features
    features &= ~(1 << VIRTIO_F_EVENT_IDX);
    features &= ~(1 << VIRTIO_F_INDIRECT_DESC);
    printf("driver features: 0x%016llx\n", features);
    virtio_mmio_set_guest_features(gs_virtio_base, features);

    // tell device that feature negotiation is complete.
    status |= VIRTIO_STAT_FEATURES_OK;
    virtio_mmio_set_status(gs_virtio_base, status);

    // re-read status to ensure FEATURES_OK is set.
    status = virtio_mmio_get_status(gs_virtio_base);
    if(!(status & VIRTIO_STAT_FEATURES_OK)) {
        printf("virtio entropy FEATURES_OK unset");
        return -2;
    }

    // initialize queue 0.
    int qnum = 0;
    int qsize = RNG_QSIZE;
    // ensure queue 0 is not in use.
    if (virtio_mmio_get_queue_ready(gs_virtio_base, qnum)) {
        printf("virtio entropy should not be ready");
        return -3;
    }

    // check maximum queue size.
    u32 max = virtio_mmio_get_queue_size(gs_virtio_base, qnum);
    if(max == 0){
        printf("virtio entropy has no queue 0");
        return -4;
    }
    if(max < qsize){
        printf("virtio entropy max queue too short");
        return -5;
    }
    printf("queue_0 max size: %d\n", max);
    gs_virtio_rng.qsize = max;

    int r = virtio_vring_init(&gs_virtio_rng.vr, gs_rng_buf, sizeof(gs_rng_buf), qsize);
    if (r) {
        printf("virtio_vring_init failed: %d\n", r);
        return r;
    }

    // set queue size.
    virtio_mmio_set_queue_size(gs_virtio_base, qnum, qsize);
    // write physical addresses.
    virtio_mmio_set_queue_addr(gs_virtio_base, qnum, &gs_virtio_rng.vr);
    // queue is ready.
    virtio_mmio_set_queue_ready(gs_virtio_base, qnum);

    // tell device we're completely ready.
    status |= VIRTIO_STAT_DRIVER_OK;
    virtio_mmio_set_status(gs_virtio_base, status);

    printf("status:0x%02x\n", virtio_mmio_get_status(gs_virtio_base));

    return 0;
}

int virtio_rng_read(u8 *buf, int len)
{
    int qnum = 0;
    struct virtio_rng *rng = &gs_virtio_rng;
    int idx = rng->avail_idx++ % RNG_QSIZE;

    //printf("buf: %p, len: %d\n", buf, len);
    //printf("idx: %d, avail: %d, used_idx: %d, used->idx: %d\n",
    //    idx, rng->vr.avail->idx, rng->used_idx, rng->vr.used->idx);

    // fill descriptor: rng buf
    virtio_vring_fill_desc(rng->vr.desc + idx, (u32)buf, len,
            VRING_DESC_F_WRITE, 0);

    virtio_vring_add_avail(rng->vr.avail, idx, RNG_QSIZE);
    virtio_mmio_set_notify(gs_virtio_base, qnum);

    //disable_irq();
    //printf("virtio_rng_read waiting, b: %p ...\n", buf);
    //enable_irq();
    volatile u16 *pt_used_idx = &rng->used_idx;
    volatile u16 *pt_idx = &rng->vr.used->idx;
    // wait cmd done
    while (*pt_used_idx == *pt_idx)
        ;
    //printf("virtio_rng_read done, b: %p\n", buf);

    int rlen = rng->vr.used->ring[rng->used_idx % RNG_QSIZE].len;
    rng->used_idx += 1;
    return rlen;
}
