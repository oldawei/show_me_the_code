#include "riscv.h"
#include "plic.h"
#include "virtio.h"
#include "virtio-ring.h"
#include "virtio-mmio.h"
#include "virtio-net.h"
#include "printf/printf.h"
#include <string.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "list.h"
/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"

#define VIRTIO_NET_TX_ISR  (0)
#define VIRTIO_NET_RX_ISR  (1)

 #define VIRTIO_BUFFER_SIZE         ( 1526 )
 void vNetworkInterfaceRxNotify( void );

static u8 gs_net_rbuf[3*4096] __attribute__((aligned(4096))) = { 0 };
static u8 gs_net_tbuf[3*4096] __attribute__((aligned(4096))) = { 0 };
static u32 gs_virtio_base = 0;
static struct virtio_net gs_virtio_net = { 0 };

int virtio_net_init(u32 base)
{
    gs_virtio_base = base;
    virtio_mmio_init(gs_virtio_base);

    if (virtio_mmio_read_reg(gs_virtio_base, VIRTIO_MMIO_MAGIC_VALUE) != 0x74726976 ||
        virtio_mmio_read_reg(gs_virtio_base, VIRTIO_MMIO_VERSION) != 2 ||
        virtio_mmio_read_reg(gs_virtio_base, VIRTIO_MMIO_DEVICE_ID) != 1 ||
        virtio_mmio_read_reg(gs_virtio_base, VIRTIO_MMIO_VENDOR_ID) != 0x554d4551) {
        printf("could not find virtio net\n");
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
    features = VIRTIO_NET_F_MTU |
               VIRTIO_NET_F_MAC |
               VIRTIO_NET_F_STATUS |
               VIRTIO_F_VERSION_1 |
               VIRTIO_F_RING_RESET;
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
    int qsize = NET_QSIZE;
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
    gs_virtio_net.qsize = max;

    int r = virtio_vring_init(&gs_virtio_net.rx_vr, gs_net_rbuf, sizeof(gs_net_rbuf), qsize);
    if (r) {
        printf("virtio_vring_init failed1: %d\n", r);
        return r;
    }

    r = virtio_vring_init(&gs_virtio_net.tx_vr, gs_net_tbuf, sizeof(gs_net_tbuf), qsize);
    if (r) {
        printf("virtio_vring_init failed2: %d\n", r);
        return r;
    }

    struct virtio_net *net = &gs_virtio_net;
    for (int i = 0; i < qsize; i++) {
        NetworkBufferDescriptor_t *pxBufferDescriptor = pxGetNetworkBufferWithDescriptor( VIRTIO_BUFFER_SIZE, 0 );
        if ( pxBufferDescriptor == NULL ) {
             printf("virtio_net_init failed3: %d\n", __LINE__);
             return -3;
        }
        net->rx_ptr[i] = (u8 *)pxBufferDescriptor;
        memset((u8*)pxBufferDescriptor->pucEthernetBuffer - 10, 0, 10);
        virtio_vring_fill_desc(net->rx_vr.desc + i, (u32)((u8*)pxBufferDescriptor->pucEthernetBuffer - 10),
                sizeof(struct virtio_net_rxpkt), VRING_DESC_F_WRITE, 0);
        virtio_vring_add_avail(net->rx_vr.avail, i, NET_QSIZE);
    }

    // set queue size.
    virtio_mmio_set_queue_size(gs_virtio_base, qnum, qsize);
    virtio_mmio_set_queue_size(gs_virtio_base, qnum + 1, qsize);
    // write physical addresses.
    virtio_mmio_set_queue_addr(gs_virtio_base, qnum, &gs_virtio_net.rx_vr);
    virtio_mmio_set_queue_addr(gs_virtio_base, qnum + 1, &gs_virtio_net.tx_vr);
    // queue is ready.
    virtio_mmio_set_queue_ready(gs_virtio_base, qnum);
    virtio_mmio_set_queue_ready(gs_virtio_base, qnum + 1);
    virtio_mmio_set_notify(gs_virtio_base, qnum); // rx queue

    // tell device we're completely ready.
    status |= VIRTIO_STAT_DRIVER_OK;
    virtio_mmio_set_status(gs_virtio_base, status);

    virtio_net_cfg(gs_virtio_base + VIRTIO_MMIO_CONFIG);
#if VIRTIO_NET_TX_ISR || VIRTIO_NET_RX_ISR
    plic_enable(VIRTIO2_IRQ);
#endif
    return 0;
}

void virtio_net_cfg(u32 cfg_addr)
{
    struct virtio_net_config *cfg = (struct virtio_net_config *)cfg_addr;

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

int virtio_net_tx(NetworkBufferDescriptor_t * const pxNetworkBuffer, BaseType_t xReleaseAfterSend)
{
    int idx[2];
    int qnum = 1;
    struct virtio_net *net = &gs_virtio_net;

     u8 *buf = pxNetworkBuffer->pucEthernetBuffer;
     int buf_len = pxNetworkBuffer->xDataLength;

    idx[0] = net->tx_avail_idx++ % NET_QSIZE;
    idx[1] = net->tx_avail_idx++ % NET_QSIZE;

    //printf("tx idx: %d, %d\n", idx[0], idx[1]);
    //printf("buf: %p, len: %d\n", buf, len);
    //printf("idx: %d, avail: %d, used_idx: %d, used->idx: %d\n",
    //    idx, net->tx_vr.avail->idx, net->tx_used_idx, net->tx_vr.used->idx);

    // fill descriptor: net hdr
    //net->tx_hdr[idx[0]].flags = VIRTIO_NET_HDR_F_DATA_VALID;
    //net->tx_hdr[idx[0]].num_buffers = 1;

#if VIRTIO_NET_TX_ISR
    net->tx_ptr[idx[0]].ptr = (u8 *)pxNetworkBuffer;
    net->tx_ptr[idx[0]].flag = (u32)xReleaseAfterSend;
#endif

    void *tx_hdr = (void *)(buf - 10);
    memset(tx_hdr, 0, 10);
    dsb();
    virtio_vring_fill_desc(net->tx_vr.desc + idx[0], (u32)tx_hdr,
            sizeof(struct virtio_net_txhdr), VRING_DESC_F_NEXT, idx[1]);

    // fill descriptor: net pkt
    virtio_vring_fill_desc(net->tx_vr.desc + idx[1], (u32)buf, buf_len,
            0, 0);

    dsb();
    virtio_vring_add_avail(net->tx_vr.avail, idx[0], NET_QSIZE);
    dsb();
    virtio_mmio_set_notify(gs_virtio_base, qnum);

    //disable_irq();
    //printf("virtio_rng_read waiting, b: %p ...\n", buf);
    //enable_irq();
    volatile u16 *pt_used_idx = &net->tx_used_idx;
    volatile u16 *pt_idx = &net->tx_vr.used->idx;
#if VIRTIO_NET_TX_ISR
    // wait cmd done
    while (*pt_used_idx == *pt_idx)
       ;
    //printf("virtio_rng_read done, b: %p\n", buf);

    //int rlen = net->tx_vr.used->ring[net->tx_used_idx % NET_QSIZE].len;
    //net->tx_used_idx += 1;
#else
    u32 counter = 0xffffffff;

    dsb();
    // wait cmd done
    while (--counter && (*pt_used_idx == *pt_idx))
       ;
    //printf("virtio_rng_read done, b: %p\n", buf);

    if (counter == 0) {
        printf("\n===============================================================================\n");
        printf("\t\t\tvirtio_net_tx timeout\n");
        printf("===============================================================================\n");
        while(1);
    }

    dsb();
    //int rlen = net->tx_vr.used->ring[net->tx_used_idx % NET_QSIZE].len;
    net->tx_used_idx += 1;

    if( xReleaseAfterSend != pdFALSE ) {
        /* It is assumed SendData() copies the data out of the FreeRTOS-Plus-TCP Ethernet
        buffer.  The Ethernet buffer is therefore no longer needed, and must be
        freed for re-use. */
        vReleaseNetworkBufferAndDescriptor( pxNetworkBuffer );
    }
#endif

    return 0;
}

void virtio_net_tx_done(void)
{
#if VIRTIO_NET_TX_ISR
    struct virtio_net *net = &gs_virtio_net;

    while (net->tx_used_idx != net->tx_vr.used->idx) {
        int nn = net->tx_used_idx % NET_QSIZE;
        net->tx_used_idx += 1;
        int idx = net->tx_vr.used->ring[nn].id;

        NetworkBufferDescriptor_t *pxNetworkBuffer = (NetworkBufferDescriptor_t *)net->tx_ptr[idx].ptr;
        BaseType_t xReleaseAfterSend  = (BaseType_t)net->tx_ptr[idx].flag;
        if( xReleaseAfterSend != pdFALSE )
        {
            /* It is assumed SendData() copies the data out of the FreeRTOS-Plus-TCP Ethernet
            buffer.  The Ethernet buffer is therefore no longer needed, and must be
            freed for re-use. */
            vReleaseNetworkBufferAndDescriptor( pxNetworkBuffer );
        }
    }
#endif
}

int virtio_net_rx(u8 *buf)
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

    NetworkBufferDescriptor_t *pxBufferDescriptor = (NetworkBufferDescriptor_t *)net->rx_ptr[idx];
    u8 *pkt = pxBufferDescriptor->pucEthernetBuffer;
    memcpy(buf, pkt, rlen);

    for (int i = 0; i < 32; ++i) {
        printf("%02x%s", buf[i], ((i + 1) % 16 == 0) ? "\n" : " ");
    }
    printf("\n");

    virtio_vring_add_avail(net->rx_vr.avail, idx, NET_QSIZE);
    virtio_mmio_set_notify(gs_virtio_base, qnum);

    return rlen;
}

int virtio_net_intr(void)
{
    //u8 buf[VIRTIO_NET_PKT_LEN] = { 0 };
    struct virtio_net *net = &gs_virtio_net;

    virtio_mmio_set_ack(gs_virtio_base);

    virtio_net_tx_done();
    if (net->rx_used_idx != net->rx_vr.used->idx) {
        //virtio_net_rx(buf);
        vNetworkInterfaceRxNotify();
        // todo: send pkt to tcp/ip stack
    }

    return 0;
}

int virtio_net_link_status(void)
{
    u32 cfg_addr = gs_virtio_base + VIRTIO_MMIO_CONFIG;
    struct virtio_net_config *cfg = (struct virtio_net_config *)cfg_addr;

    return cfg->status == 1;
}

int virtio_net_close(void)
{
    /* Quiesce device */
    virtio_mmio_set_status(gs_virtio_base, VIRTIO_STAT_FAILED);
    // reset device
    virtio_mmio_reset_device(gs_virtio_base);

    return 0;
}

void *virtio_net_rx_pkt(long int *p_size)
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

    if (net->rx_used_idx == net->rx_vr.used->idx) { // not rx pkt
        return NULL;
    }

    //printf("virtio_rng_read done, b: %p\n", buf);
    int nn = net->rx_used_idx % NET_QSIZE;
    net->rx_used_idx += 1;
    //printf("rx idx: %d\n", nn);

    int idx = net->rx_vr.used->ring[nn].id;
    int rlen = net->rx_vr.used->ring[nn].len - 10;
    //printf("rlen: %d, buf_len %d\n", rlen, buf_len);

    int is_alloc_failed = 0;
    NetworkBufferDescriptor_t *ptr = (NetworkBufferDescriptor_t *)net->rx_ptr[idx];
    NetworkBufferDescriptor_t *pxBufferDescriptor = pxGetNetworkBufferWithDescriptor( VIRTIO_BUFFER_SIZE, 0 );
	if ( pxBufferDescriptor == NULL ) {
        printf("virtio_net_rx_pkt failed: %d\n", __LINE__);

        pxBufferDescriptor = ptr;
        is_alloc_failed = 1;
    }
    net->rx_ptr[idx] = (u8 *)pxBufferDescriptor;
    memset((u8*)pxBufferDescriptor->pucEthernetBuffer - 10, 0, 10);
    virtio_vring_fill_desc(net->rx_vr.desc + idx, (u32)((u8*)pxBufferDescriptor->pucEthernetBuffer - 10),
            sizeof(struct virtio_net_rxpkt), VRING_DESC_F_WRITE, 0);

    virtio_vring_add_avail(net->rx_vr.avail, idx, NET_QSIZE);
    virtio_mmio_set_notify(gs_virtio_base, qnum);

    if (is_alloc_failed)
        return NULL;

    if (p_size) *p_size = rlen;
    return ptr;
}
