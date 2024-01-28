#ifndef VIRTIO_NET_H_
#define VIRTIO_NET_H_

#include "types.h"
#include "virtio-ring.h"

#define NET_QSIZE       (128)   // net queue0 size

// net device feature bits
#define VIRTIO_NET_F_CSUM (0) //Device handles packets with partial checksum.
#define VIRTIO_NET_F_GUEST_CSUM (1) //Driver handles packets with partial checksum.
#define VIRTIO_NET_F_CTRL_GUEST_OFFLOADS (2) //Control channel offloads reconfiguration support.
#define VIRTIO_NET_F_MTU (3) //Device maximum MTU reporting is supported
#define VIRTIO_NET_F_MAC (5)//Device has given MAC address
#define VIRTIO_NET_F_GUEST_TSO4 (7) //Driver can receive TSOv4.
#define VIRTIO_NET_F_GUEST_TSO6 (8) //Driver can receive TSOv6.
#define VIRTIO_NET_F_GUEST_ECN (9) //Driver can receive TSO with ECN.
#define VIRTIO_NET_F_GUEST_UFO (10) //Driver can receive UFO.
#define VIRTIO_NET_F_HOST_TSO4 (11) //Device can receive TSOv4.
#define VIRTIO_NET_F_HOST_TSO6 (12) //Device can receive TSOv6.
#define VIRTIO_NET_F_HOST_ECN (13) //Device can receive TSO with ECN.
#define VIRTIO_NET_F_HOST_UFO (14) //Device can receive UFO.
#define VIRTIO_NET_F_MRG_RXBUF (15) //Driver can merge receive buffers.
#define VIRTIO_NET_F_STATUS (16) //Configuration status field is available.
#define VIRTIO_NET_F_CTRL_VQ (17) //Control channel is available.
#define VIRTIO_NET_F_CTRL_RX (18) //Control channel RX mode support.
#define VIRTIO_NET_F_CTRL_VLAN (19) //Control channel VLAN filtering.
#define VIRTIO_NET_F_GUEST_ANNOUNCE (21) //Driver can send gratuitous packets.
#define VIRTIO_NET_F_MQ (22) //Device supports multiqueue with automatic receive steering.
#define VIRTIO_NET_F_CTRL_MAC_ADDR (23) //Set MAC address through control channel.
#define VIRTIO_NET_F_HOST_USO (56) //Device can receive USO packets.
#define VIRTIO_NET_F_HASH_REPORT (57) //Device can report per-packet hash value and a type of calculated hash.
#define VIRTIO_NET_F_GUEST_HDRLEN (59) //Driver can provide the exact hdr_len value.
#define VIRTIO_NET_F_RSS (60) //Device supports RSS (receive-side scaling)
#define VIRTIO_NET_F_RSC_EXT (61) //Device can process duplicated ACKs and report number of coalesced segments and duplicated ACKs.
#define VIRTIO_NET_F_STANDBY (62) //Device may act as a standby for a primary device with the same MAC address.
#define VIRTIO_NET_F_SPEED_DUPLEX (63) //Device reports speed and duplex.


// status for virtio_net_config.status
#define VIRTIO_NET_S_LINK_UP 1
#define VIRTIO_NET_S_ANNOUNCE 2
// As per VirtIO spec Version 1.1: 5.2.4 Device configuration layout
struct virtio_net_config {
	u8 mac[6];
	u16 status;
	u16 max_virtqueue_pairs;
	u16 mtu;
	u32 speed;
	u8 duplex;
	u8 rss_max_key_size;
	u16 rss_max_indirection_table_length;
	u32 supported_hash_types;
} __attribute__ ((packed));

struct virtio_net_rxpkt {
#define VIRTIO_NET_HDR_F_NEEDS_CSUM 1
#define VIRTIO_NET_HDR_F_DATA_VALID 2
#define VIRTIO_NET_HDR_F_RSC_INFO 4
	u8 flags;
#define VIRTIO_NET_HDR_GSO_NONE 0
#define VIRTIO_NET_HDR_GSO_TCPV4 1
#define VIRTIO_NET_HDR_GSO_UDP 3
#define VIRTIO_NET_HDR_GSO_TCPV6 4
#define VIRTIO_NET_HDR_GSO_UDP_L4 5
#define VIRTIO_NET_HDR_GSO_ECN 0x80
	u8 gso_type;
	u16 hdr_len;
	u16 gso_size;
	u16 csum_start;
	u16 csum_offset;
	//u16 num_buffers; (Only if VIRTIO_NET_F_MRG_RXBUFT negotiated)
	//u32 hash_value; (Only if VIRTIO_NET_F_HASH_REPORT negotiated)
	//u16 hash_report; (Only if VIRTIO_NET_F_HASH_REPORT negotiated)
	//u16 padding_reserved; (Only if VIRTIO_NET_F_HASH_REPORT negotiated)
#define VIRTIO_NET_PKT_LEN 1514
	u8 pkt[VIRTIO_NET_PKT_LEN];
} __attribute__((packed));

struct virtio_net_txhdr {
	u8 flags;
	u8 gso_type;
	u16 hdr_len;
	u16 gso_size;
	u16 csum_start;
	u16 csum_offset;
	//u16 num_buffers; (Only if VIRTIO_NET_F_MRG_RXBUFT negotiated)
	//u32 hash_value; (Only if VIRTIO_NET_F_HASH_REPORT negotiated)
	//u16 hash_report; (Only if VIRTIO_NET_F_HASH_REPORT negotiated)
	//u16 padding_reserved; (Only if VIRTIO_NET_F_HASH_REPORT negotiated)
} __attribute__((packed));


struct virtio_net {
    // disk command headers.
    // one-for-one with descriptors, for convenience.
    struct virtio_net_rxpkt rx_pkt[NET_QSIZE];
	struct virtio_net_txhdr tx_hdr[NET_QSIZE];
    struct vring rx_vr;
	struct vring tx_vr;
	u32 qsize;	// queue0 size
    u16 rx_used_idx;
	u16 rx_avail_idx;
	u16 tx_used_idx;
	u16 tx_avail_idx;
};

int virtio_net_init(u32 base);
void virtio_net_cfg(void);
int virtio_net_tx(u8 *buf, int buf_len);
int virtio_net_rx(u8 *buf);
int virtio_net_intr(void);
int virtio_net_close(void);

#endif /* VIRTIO_NET_H_ */
