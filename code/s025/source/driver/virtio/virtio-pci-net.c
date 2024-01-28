#include "riscv.h"
#include "riscv32.h"
#include "aplic.h"
#include "pci.h"
#include "virtio.h"
#include "virtio-ring.h"
#include "virtio-pci.h"
#include "virtio-net.h"
#include "virtio-pci-net.h"
#include "printf/printf.h"
#include <string.h>

static u8 gs_net_rbuf[3*4096] __attribute__((aligned(4096))) = { 0 };
static u8 gs_net_tbuf[3*4096] __attribute__((aligned(4096))) = { 0 };
static virtio_pci_hw_t gs_virtio_net_hw = { 0 };
static struct virtio_net gs_virtio_net = { 0 };
static trap_handler_fn gs_net_msix_handler[] = {
    virtio_pci_net_cfg_isr,
    virtio_pci_net_intr,
    virtio_pci_net_tx_isr,
    NULL };

int virtio_pci_net_init(void)
{
    u32 pci_base = pci_device_probe(0x1af4, 0x1000);
	if (pci_base) {
		virtio_pci_read_caps(&gs_virtio_net_hw, pci_base, gs_net_msix_handler);
		virtio_pci_print_common_cfg(&gs_virtio_net_hw);
	} else {
        printf("virtion-net-pci device not found!\n");
        return -1;
    }

    // reset device
    virtio_pci_set_status(&gs_virtio_net_hw, 0);

    u32 status = 0;
    // set ACKNOWLEDGE status bit
    status |= VIRTIO_STAT_ACKNOWLEDGE;
    virtio_pci_set_status(&gs_virtio_net_hw, status);

    // set DRIVER status bit
    status |= VIRTIO_STAT_DRIVER;
    virtio_pci_set_status(&gs_virtio_net_hw, status);

    // negotiate features
    u64 features = virtio_pci_get_device_features(&gs_virtio_net_hw);
    printf("device features: 0x%016llx\n", features);
    features = VIRTIO_NET_F_MTU |
               VIRTIO_NET_F_MAC |
               VIRTIO_NET_F_STATUS |
               VIRTIO_F_VERSION_1 |
               VIRTIO_F_RING_RESET;
    printf("driver features: 0x%016llx\n", features);
    virtio_pci_set_driver_features(&gs_virtio_net_hw, features);

    // tell device that feature negotiation is complete.
    status |= VIRTIO_STAT_FEATURES_OK;
    virtio_pci_set_status(&gs_virtio_net_hw, status);

    // re-read status to ensure FEATURES_OK is set.
    status = virtio_pci_get_status(&gs_virtio_net_hw);
    if(!(status & VIRTIO_STAT_FEATURES_OK)) {
        printf("virtio net FEATURES_OK unset");
        return -2;
    }

    // initialize queue 0.
    int qnum = 0;
    int qsize = NET_QSIZE;
    // ensure queue 0 is not in use.
    if (virtio_pci_get_queue_enable(&gs_virtio_net_hw, qnum)) {
        printf("virtio net should not be ready");
        return -3;
    }

    // check maximum queue size.
    u32 max = virtio_pci_get_queue_size(&gs_virtio_net_hw, qnum);
    if(max == 0){
        printf("virtio net has no queue 0");
        return -4;
    }
    if(max < qsize){
        printf("virtio net max queue too short");
        return -5;
    }
    printf("queue_0 max size: %d\n", max);
    gs_virtio_net.qsize = max;

    int r = virtio_vring_init(&gs_virtio_net.rx_vr, gs_net_rbuf, sizeof(gs_net_rbuf), qsize, qnum);
    if (r) {
        printf("virtio_vring_init failed1: %d\n", r);
        return r;
    }

    r = virtio_vring_init(&gs_virtio_net.tx_vr, gs_net_tbuf, sizeof(gs_net_tbuf), qsize, qnum + 1);
    if (r) {
        printf("virtio_vring_init failed2: %d\n", r);
        return r;
    }

    struct virtio_net *net = &gs_virtio_net;
    for (int i = 0; i < qsize; i++) {
        virtio_vring_fill_desc(net->rx_vr.desc + i, (u32)&net->rx_pkt[i],
                sizeof(struct virtio_net_rxpkt), VRING_DESC_F_WRITE, 0);
        virtio_vring_add_avail(net->rx_vr.avail, i, NET_QSIZE);
    }

    // set queue size.
    virtio_pci_set_queue_size(&gs_virtio_net_hw, qnum, qsize);
    virtio_pci_set_queue_size(&gs_virtio_net_hw, qnum + 1, qsize);
    // enable msix
    virtio_pci_set_config_msix(&gs_virtio_net_hw, 0);
    //virtio_pci_disable_queue_msix(&gs_virtio_net_hw, qnum);
    //virtio_pci_disable_queue_msix(&gs_virtio_net_hw, qnum + 1);
    virtio_pci_set_queue_msix(&gs_virtio_net_hw, qnum, 1);
    virtio_pci_set_queue_msix(&gs_virtio_net_hw, qnum + 1, 2);
    // write physical addresses.
    virtio_pci_set_queue_addr(&gs_virtio_net_hw, qnum, &gs_virtio_net.rx_vr);
    virtio_pci_set_queue_addr(&gs_virtio_net_hw, qnum + 1, &gs_virtio_net.tx_vr);
    // queue is ready.
    virtio_pci_set_queue_enable(&gs_virtio_net_hw, qnum);
    virtio_pci_set_queue_enable(&gs_virtio_net_hw, qnum + 1);
    virtio_pci_set_queue_notify(&gs_virtio_net_hw, qnum); // rx queue

    // tell device we're completely ready.
    status |= VIRTIO_STAT_DRIVER_OK;
    virtio_pci_set_status(&gs_virtio_net_hw, status);

    virtio_pci_net_cfg();
    return 0;
}

void virtio_pci_net_cfg(void)
{
    u32 pt[(sizeof(struct virtio_net_config) + 3)/4] = { 0 };
    struct virtio_net_config *cfg = (struct virtio_net_config *)pt;

    for (int i = 0; i < sizeof(cfg)/4; ++i) {
        pt[i] = virtio_pci_get_config(&gs_virtio_net_hw, 4*i, 4);
    }

    printf("mac: %02x:%02x:%02x:%02x:%02x:%02x\n", cfg->mac[0], cfg->mac[1], cfg->mac[2],
            cfg->mac[3], cfg->mac[4], cfg->mac[5]);
    printf("status: %d\n", cfg->status);
    printf("max_virtqueue_pairs: %d\n", cfg->max_virtqueue_pairs);
    printf("mtu: %d\n", cfg->mtu);
    printf("speed: %d\n", cfg->speed);
    printf("duplex: %d\n", cfg->duplex);
    printf("rss_max_key_size: %d\n", cfg->rss_max_key_size);
    printf("rss_max_indirection_table_length: %d\n", cfg->rss_max_indirection_table_length);
    printf("supported_hash_types: %d\n", cfg->supported_hash_types);
}

int virtio_pci_net_tx(u8 *buf, int buf_len)
{
    int idx[2];
    int qnum = 1;
    struct virtio_net *net = &gs_virtio_net;

    idx[0] = net->tx_avail_idx++ % NET_QSIZE;
    idx[1] = net->tx_avail_idx++ % NET_QSIZE;

    printf("tx idx: %d, %d\n", idx[0], idx[1]);
    //printf("buf: %p, len: %d\n", buf, len);
    //printf("idx: %d, avail: %d, used_idx: %d, used->idx: %d\n",
    //    idx, net->tx_vr.avail->idx, net->tx_used_idx, net->tx_vr.used->idx);

    // fill descriptor: net hdr
    //net->tx_hdr[idx[0]].flags = VIRTIO_NET_HDR_F_DATA_VALID;
    //net->tx_hdr[idx[0]].num_buffers = 1;
    virtio_vring_fill_desc(net->tx_vr.desc + idx[0], (u32)&net->tx_hdr[idx[0]],
            sizeof(struct virtio_net_txhdr), VRING_DESC_F_NEXT, idx[1]);

    // fill descriptor: net pkt
    virtio_vring_fill_desc(net->tx_vr.desc + idx[1], (u32)buf, buf_len,
            0, 0);

    virtio_vring_add_avail(net->tx_vr.avail, idx[0], NET_QSIZE);
    virtio_pci_set_queue_notify(&gs_virtio_net_hw, qnum);

    //disable_irq();
    //printf("virtio_rng_read waiting, b: %p ...\n", buf);
    //enable_irq();
    volatile u16 *pt_used_idx = &net->tx_used_idx;
    volatile u16 *pt_idx = &net->tx_vr.used->idx;
    // wait cmd done
    while (*pt_used_idx == *pt_idx)
        ;
    //printf("virtio_rng_read done, b: %p\n", buf);

    int rlen = net->tx_vr.used->ring[net->tx_used_idx % NET_QSIZE].len;
    net->tx_used_idx += 1;
    return rlen;
}

int virtio_pci_net_rx(u8 *buf)
{
    int qnum = 0;
    struct virtio_net *net = &gs_virtio_net;

    //volatile u16 *pt_used_idx = &net->rx_used_idx;
    //volatile u16 *pt_idx = &net->rx_vr.used->idx;
    //printf("rx_used_idx: %d\n", net->rx_used_idx);
    //printf("rx_vr.used->idx: %d\n", net->rx_vr.used->idx);
    // wait cmd done
    //while (*pt_used_idx == *pt_idx)
    //    ;

    //printf("plic pending: 0x%016llx\n", plic_get_pending());
    if (net->rx_used_idx == net->rx_vr.used->idx) { // not rx pkt
        return 0;
    }

    //printf("virtio_rng_read done, b: %p\n", buf);
    int nn = net->rx_used_idx % NET_QSIZE;
    net->rx_used_idx += 1;
    printf("rx idx: %d\n", nn);

    int idx = net->rx_vr.used->ring[nn].id;
    int rlen = net->rx_vr.used->ring[nn].len - 10;
    printf("rlen: %d\n", rlen);
    memcpy(buf, net->rx_pkt[idx].pkt, rlen);

    for (int i = 0; i < 32; ++i) {
        printf("%02x%s", buf[i], ((i + 1) % 16 == 0) ? "\n" : " ");
    }
    printf("\n");

    virtio_vring_add_avail(net->rx_vr.avail, idx, NET_QSIZE);
    virtio_pci_set_queue_notify(&gs_virtio_net_hw, qnum);

    return rlen;
}

int virtio_pci_net_cfg_isr(int irq)
{
    printf("virtio-net cfg: %d\n", irq);
    return 0;
}

int virtio_pci_net_tx_isr(int irq)
{
    printf("virtio-net tx: %d\n", irq);
    return 0;
}

int virtio_pci_net_intr(int irq)
{
    u8 buf[VIRTIO_NET_PKT_LEN] = { 0 };
    struct virtio_net *net = &gs_virtio_net;

    //virtio_pci_clear_isr();
    while (net->rx_used_idx != net->rx_vr.used->idx) {
        virtio_pci_net_rx(buf);
        // todo: send pkt to tcp/ip stack
    }
    return 0;
}

int virtio_pci_net_close(void)
{
    /* Quiesce device */
    virtio_pci_set_status(&gs_virtio_net_hw, VIRTIO_STAT_FAILED);
    // reset device
    virtio_pci_set_status(&gs_virtio_net_hw, 0);

    return 0;
}
