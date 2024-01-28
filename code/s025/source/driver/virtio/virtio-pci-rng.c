#include "riscv.h"
#include "aplic.h"
#include "pci.h"
#include "virtio.h"
#include "virtio-ring.h"
#include "virtio-pci.h"
#include "virtio-rng.h"
#include "virtio-pci-rng.h"
#include "printf/printf.h"
#include <string.h>

static u8 gs_rng_buf[3*4096] __attribute__((aligned(4096))) = { 0 };
static virtio_pci_hw_t gs_virtio_rng_hw = { 0 };
static struct virtio_rng gs_virtio_rng = { 0 };
static trap_handler_fn gs_rng_msix_handler[] = {
    virtio_pci_rng_cfg_isr,
    virtio_pci_rng_intr };

int virtio_pci_rng_init(void)
{
    u32 pci_base = pci_device_probe(0x1af4, 0x1005);
	if (pci_base) {
		virtio_pci_read_caps(&gs_virtio_rng_hw, pci_base, gs_rng_msix_handler);
		virtio_pci_print_common_cfg(&gs_virtio_rng_hw);
	} else {
        printf("virtion-rng-pci device not found!\n");
        return -1;
    }

    // reset device
    virtio_pci_set_status(&gs_virtio_rng_hw, 0);

    u32 status = 0;
    // set ACKNOWLEDGE status bit
    status |= VIRTIO_STAT_ACKNOWLEDGE;
    virtio_pci_set_status(&gs_virtio_rng_hw, status);

    // set DRIVER status bit
    status |= VIRTIO_STAT_DRIVER;
    virtio_pci_set_status(&gs_virtio_rng_hw, status);

    // negotiate features
    u64 features = virtio_pci_get_device_features(&gs_virtio_rng_hw);
    printf("device features: 0x%016llx\n", features);
    // negotiate features
    features &= ~(1 << VIRTIO_F_EVENT_IDX);
    features &= ~(1 << VIRTIO_F_INDIRECT_DESC);
    printf("driver features: 0x%016llx\n", features);
    virtio_pci_set_driver_features(&gs_virtio_rng_hw, features);

    // tell device that feature negotiation is complete.
    status |= VIRTIO_STAT_FEATURES_OK;
    virtio_pci_set_status(&gs_virtio_rng_hw, status);

    // re-read status to ensure FEATURES_OK is set.
    status = virtio_pci_get_status(&gs_virtio_rng_hw);
    if(!(status & VIRTIO_STAT_FEATURES_OK)) {
        printf("virtio rng FEATURES_OK unset");
        return -2;
    }

    // initialize queue 0.
    int qnum = 0;
    int qsize = 8; //RNG_QSIZE;
    // ensure queue 0 is not in use.
    if (virtio_pci_get_queue_enable(&gs_virtio_rng_hw, qnum)) {
        printf("virtio rng should not be ready");
        return -3;
    }

    // check maximum queue size.
    u32 max = virtio_pci_get_queue_size(&gs_virtio_rng_hw, qnum);
    printf("queue_0 max size: %d\n", max);
    if(max == 0){
        printf("virtio rng has no queue 0");
        return -4;
    }
    if(max < qsize){
        printf("virtio rng max queue too short");
        return -5;
    }
    gs_virtio_rng.qsize = max;

    int r = virtio_vring_init(&gs_virtio_rng.vr, gs_rng_buf, sizeof(gs_rng_buf), qsize, qnum);
    if (r) {
        printf("virtio_vring_init failed1: %d\n", r);
        return r;
    }

    // set queue size.
    virtio_pci_set_queue_size(&gs_virtio_rng_hw, qnum, qsize);
    // enable msix
    //virtio_pci_disable_config_msix(&gs_virtio_rng_hw);
    virtio_pci_set_config_msix(&gs_virtio_rng_hw, 0);
    //virtio_pci_disable_queue_msix(&gs_virtio_rng_hw, qnum);
    virtio_pci_set_queue_msix(&gs_virtio_rng_hw, qnum, 1);
    // write physical addresses.
    virtio_pci_set_queue_addr(&gs_virtio_rng_hw, qnum, &gs_virtio_rng.vr);
    // queue is ready.
    virtio_pci_set_queue_enable(&gs_virtio_rng_hw, qnum);

    // tell device we're completely ready.
    status |= VIRTIO_STAT_DRIVER_OK;
    virtio_pci_set_status(&gs_virtio_rng_hw, status);

    return 0;
}

int virtio_pci_rng_read(u8 *buf, int len)
{
    int qnum = 0;
    struct virtio_rng *rng = &gs_virtio_rng;
    int idx = rng->avail_idx++ % rng->vr.size;

    //printf("buf: %p, len: %d\n", buf, len);
    //printf("idx: %d, avail: %d, used_idx: %d, used->idx: %d\n",
    //    idx, rng->vr.avail->idx, rng->used_idx, rng->vr.used->idx);

    // fill descriptor: rng buf
    virtio_vring_fill_desc(rng->vr.desc + idx, (u32)buf, len,
            VRING_DESC_F_WRITE, 0);
    virtio_vring_add_avail(rng->vr.avail, idx, rng->vr.size);
    virtio_pci_set_queue_notify(&gs_virtio_rng_hw, qnum);

    volatile u16 *pt_used_idx = &rng->used_idx;
    volatile u16 *pt_idx = &rng->vr.used->idx;
    // wait cmd done
    while (*pt_used_idx == *pt_idx)
        ;
    int rlen = rng->vr.used->ring[rng->used_idx % rng->vr.size].len;
    rng->used_idx += 1;
    return rlen;
}

int virtio_pci_rng_cfg_isr(int irq)
{
    printf("virtio-rng cfg: %d\n", irq);
    return 0;
}

int virtio_pci_rng_intr(int irq)
{
    printf("virtio-rng isr: %d\n", irq);
    return 0;
}

int virtio_pci_rng_close(void)
{
    /* Quiesce device */
    virtio_pci_set_status(&gs_virtio_rng_hw, VIRTIO_STAT_FAILED);
    // reset device
    virtio_pci_set_status(&gs_virtio_rng_hw, 0);

    return 0;
}
