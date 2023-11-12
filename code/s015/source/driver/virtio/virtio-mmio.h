#ifndef VIRTIO_MMIO_H_
#define VIRTIO_MMIO_H_

#include "types.h"

#define	VIRTIO_MMIO_MAGIC_VALUE		    0x000   // 0x74726976
#define	VIRTIO_MMIO_VERSION		        0x004   // version; should be 2
#define	VIRTIO_MMIO_DEVICE_ID		    0x008   // device type; 1 is net, 2 is disk, 4 is rng
#define	VIRTIO_MMIO_VENDOR_ID		    0x00c   // 0x554d4551
#define	VIRTIO_MMIO_HOST_FEATURES	    0x010
#define	VIRTIO_MMIO_HOST_FEATURES_SEL	0x014
#define	VIRTIO_MMIO_GUEST_FEATURES	    0x020
#define	VIRTIO_MMIO_GUEST_FEATURES_SEL	0x024
#define	VIRTIO_MMIO_GUEST_PAGE_SIZE	    0x028	/* version 1 only */
#define	VIRTIO_MMIO_QUEUE_SEL		    0x030   // select queue, write-only
#define	VIRTIO_MMIO_QUEUE_NUM_MAX	    0x034   // max size of current queue, read-only
#define	VIRTIO_MMIO_QUEUE_NUM		    0x038   // size of current queue, write-only
#define	VIRTIO_MMIO_QUEUE_ALIGN		    0x03c	/* version 1 only */
#define	VIRTIO_MMIO_QUEUE_PFN		    0x040	/* version 1 only */
#define	VIRTIO_MMIO_QUEUE_READY		    0x044	/* ready bit, requires version 2 */
#define	VIRTIO_MMIO_QUEUE_NOTIFY	    0x050   // write-only
#define	VIRTIO_MMIO_INTERRUPT_STATUS	0x060   // read-only
#define	VIRTIO_MMIO_INTERRUPT_ACK	    0x064   // write-only
#define	VIRTIO_MMIO_STATUS		        0x070   // read/write
#define	VIRTIO_MMIO_QUEUE_DESC_LOW	    0x080	/* physical address for descriptor table, write-only, requires version 2 */
#define	VIRTIO_MMIO_QUEUE_DESC_HIGH	    0x084	/* requires version 2 */
#define	VIRTIO_MMIO_QUEUE_AVAIL_LOW	    0x090	/* physical address for available ring, write-only, requires version 2 */
#define	VIRTIO_MMIO_QUEUE_AVAIL_HIGH	0x094	/* requires version 2 */
#define	VIRTIO_MMIO_QUEUE_USED_LOW	    0x0a0	/* physical address for used ring, write-only, requires version 2 */
#define	VIRTIO_MMIO_QUEUE_USED_HIGH	    0x0a4	/* requires version 2 */
#define	VIRTIO_MMIO_CONFIG_GENERATION	0x0fc	/* requires version 2 */
#define	VIRTIO_MMIO_CONFIG	            0x100	/* requires version 2 */


void virtio_mmio_init(u32 base);
u32 virtio_mmio_read_reg(u32 addr);
u8 virtio_mmio_read_reg8(u32 addr);
u32 virtio_mmio_get_status(void);
void virtio_mmio_set_status(u32 status);
void virtio_mmio_reset_device(void);
u64 virtio_mmio_get_host_features(void);
void virtio_mmio_set_guest_features(u64 features);
int virtio_mmio_get_queue_ready(int qnum);
void virtio_mmio_set_queue_ready(int qnum);
int virtio_mmio_get_queue_size(int qnum);
void virtio_mmio_set_queue_size(int qnum, u32 qsize);
void virtio_mmio_set_queue_addr(int qnum, struct vring *vr);
void virtio_mmio_set_notify(int qnum);
void virtio_mmio_set_ack(void);

#endif /* VIRTIO_MMIO_H_ */
