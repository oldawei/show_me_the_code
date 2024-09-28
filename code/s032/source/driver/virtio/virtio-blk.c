#include "riscv.h"
#include "plic.h"
#include "virtio.h"
#include "virtio-ring.h"
#include "virtio-mmio.h"
#include "virtio-blk.h"
#include "printf/printf.h"

#define VIRTIO_BLK_ISR  (0)

static u8 gs_blk_buf[3*4096] __attribute__((aligned(4096))) = { 0 };
static u32 gs_virtio_base = 0;
static struct virtio_blk gs_virtio_blk = { 0 };

int virtio_blk_init(u32 base)
{
    gs_virtio_base = base;
    virtio_mmio_init(gs_virtio_base);

    if (virtio_mmio_read_reg(gs_virtio_base, VIRTIO_MMIO_MAGIC_VALUE) != 0x74726976 ||
        virtio_mmio_read_reg(gs_virtio_base, VIRTIO_MMIO_VERSION) != 2 ||
        virtio_mmio_read_reg(gs_virtio_base, VIRTIO_MMIO_DEVICE_ID) != 2 ||
        virtio_mmio_read_reg(gs_virtio_base, VIRTIO_MMIO_VENDOR_ID) != 0x554d4551) {
        printf("could not find virtio blk\n");
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

    // negotiate features
    u64 features = virtio_mmio_get_host_features(gs_virtio_base);
    printf("device features: 0x%016llx\n", features);
    features &= ~(1 << VIRTIO_BLK_F_RO);
    features &= ~(1 << VIRTIO_BLK_F_SCSI);
    features &= ~(1 << VIRTIO_BLK_F_FLUSH);
    features &= ~(1 << VIRTIO_BLK_F_CONFIG_WCE);
    features &= ~(1 << VIRTIO_BLK_F_MQ);
    features &= ~(1 << VIRTIO_F_ANY_LAYOUT);
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
        printf("virtio disk FEATURES_OK unset");
        return -2;
    }

    // initialize queue 0.
    int qnum = 0;
    int qsize = BLK_QSIZE;
    // ensure queue 0 is not in use.
    if (virtio_mmio_get_queue_ready(gs_virtio_base, qnum)) {
        printf("virtio disk should not be ready");
        return -3;
    }

    // check maximum queue size.
    u32 max = virtio_mmio_get_queue_size(gs_virtio_base, qnum);
    if(max == 0){
        printf("virtio disk has no queue 0");
        return -4;
    }
    if(max < qsize){
        printf("virtio disk max queue too short");
        return -5;
    }
    printf("queue_0 max size: %d\n", max);
    gs_virtio_blk.qsize = max;

    int r = virtio_vring_init(&gs_virtio_blk.vr, gs_blk_buf, sizeof(gs_blk_buf), qsize);
    if (r) {
        printf("virtio_vring_init failed: %d\n", r);
        return r;
    }

    // set queue size.
    virtio_mmio_set_queue_size(gs_virtio_base, qnum, qsize);
    // write physical addresses.
    virtio_mmio_set_queue_addr(gs_virtio_base, qnum, &gs_virtio_blk.vr);
    // queue is ready.
    virtio_mmio_set_queue_ready(gs_virtio_base, qnum);

    // tell device we're completely ready.
    status |= VIRTIO_STAT_DRIVER_OK;
    virtio_mmio_set_status(gs_virtio_base, status);

    virtio_blk_cfg(gs_virtio_base + VIRTIO_MMIO_CONFIG);
#if VIRTIO_BLK_ISR
    plic_enable(VIRTIO1_IRQ);
#endif
    return 0;
}

void virtio_blk_cfg(u32 cfg_addr)
{
    struct virtio_blk_cfg *cfg = (struct virtio_blk_cfg *)cfg_addr;

    gs_virtio_blk.capacity = cfg->capacity;
    printf("capacity: %lld\n", cfg->capacity);
    printf("size_max: %d\n", cfg->size_max);
    printf("seg_max: %d\n", cfg->seg_max);
    printf("geometry.cylinders: %d\n", cfg->geometry.cylinders);
    printf("geometry.heads: %d\n", cfg->geometry.heads);
    printf("geometry.sectors: %d\n", cfg->geometry.sectors);
    printf("blk_size: %d\n", cfg->blk_size);
}

void virtio_blk_rw(struct blk_buf *b)
{
    int idx[3];
    int qnum = 0;
    u64 sector = b->addr / SECTOR_SZIE;
    struct virtio_blk *blk = &gs_virtio_blk;
    u64 sector_end = (b->addr + b->data_len) / SECTOR_SZIE;

    if (sector_end > blk->capacity) {
        printf("virtio_blk_rw: invalid data length!\n");
        return;
    }

    for (int i = 0; i < 3; ++i) {
        idx[i] = blk->avail_idx++ % BLK_QSIZE;
    }

    struct virtio_blk_req *req = &blk->ops[idx[0]];
    req->type = b->is_write ? VIRTIO_BLK_T_OUT : VIRTIO_BLK_T_IN;
    req->reserved = 0;
    req->sector = sector;

    // fill descriptor: blk request header
    virtio_vring_fill_desc(blk->vr.desc + idx[0], (u32)req, sizeof(struct virtio_blk_req),
            VRING_DESC_F_NEXT, idx[1]);

    // fill descriptor: blk data
    virtio_vring_fill_desc(blk->vr.desc + idx[1], (u32)b->data, b->data_len,
            (b->is_write ? 0 : VRING_DESC_F_WRITE) | VRING_DESC_F_NEXT, idx[2]);

    // fill descriptor: blk request status
    blk->status[idx[0]] = 0xff; // device writes 0 on success
    virtio_vring_fill_desc(blk->vr.desc + idx[2], (u32)&blk->status[idx[0]], 1,
            VRING_DESC_F_WRITE, 0);

#if VIRTIO_BLK_ISR
    // set blk flag
    b->flag = 1;
    blk->info[idx[0]] = b;
#endif

    virtio_vring_add_avail(blk->vr.avail, idx[0], BLK_QSIZE);
    virtio_mmio_set_notify(gs_virtio_base, qnum);

#if VIRTIO_BLK_ISR
    //disable_irq();
    //printf("virtio_blk_rw waiting, b: %p ...\n", b);
    //enable_irq();
    volatile u16 *pflag = &b->flag;
    // wait cmd done
    while (*pflag == 1)
        ;
    blk->info[idx[0]] = NULL;
    //printf("virtio_blk_rw done, b->flag: %d\n", b->flag);
#else
    dsb();

    // printf("used_idx: %d, idx: %d\n", blk->used_idx, blk->vr.used->idx);
    volatile u16 *pt_used_idx = &blk->used_idx;
    volatile u16 *pt_idx = &blk->vr.used->idx;
    u32 counter = 0xffffffff;
    // wait cmd done
    while (--counter && (*pt_used_idx == *pt_idx))
        ;

    if (counter == 0) {
        printf("\n===============================================================================\n");
        printf("\t\t\tvirtio_blk_rw timeout\n");
        printf("===============================================================================\n");
        while(1);
    }

    virtio_mmio_set_ack(gs_virtio_base);
    dsb();
    if (blk->status[idx[0]] != 0) {
        printf("virtio_blk_intr status: %d\n", blk->status[idx[0]]);
    }
    blk->used_idx += 1;
    dsb();
#endif
}

void virtio_blk_intr(void)
{
    struct virtio_blk *blk = &gs_virtio_blk;

    //printf("virtio_blk_intr, used_idx %d, used->idx %d\n", blk->used_idx, blk->vr.used->idx);
    virtio_mmio_set_ack(gs_virtio_base);

    // the device increments disk.used->idx when it
    // adds an entry to the used ring.
    dsb();
    while (blk->used_idx != blk->vr.used->idx) {
        int id = blk->vr.used->ring[blk->used_idx % BLK_QSIZE].id;
        //printf("virtio_blk_intr id: %d, status: 0x%02x\n", id, blk->status[id]);
        if (blk->status[id] != 0) {
            printf("virtio_blk_intr status: %d\n", blk->status[id]);
        }

        struct blk_buf *b = blk->info[id];
        //printf("virtio_blk_intr b: %p\n", b);
        b->flag = 0;   // blk is done
        blk->used_idx += 1;
        dsb();
        //printf("virtio_blk_intr b->flag: %d\n", b->flag);
    }
}

void virtio_blk_read(u8 *buf, u32 addr, u32 len)
{
    struct blk_buf req = { 0 };
	// blk read
	req.addr = addr;
	req.data = buf;
	req.data_len = len;
	req.is_write = 0;
	virtio_blk_rw(&req);
}

void virtio_blk_write(u8 *buf, u32 addr, u32 len)
{
    struct blk_buf req = { 0 };
	// blk write
	req.addr = addr;
	req.data = buf;
	req.data_len = len;
	req.is_write = 1;
	virtio_blk_rw(&req);
}