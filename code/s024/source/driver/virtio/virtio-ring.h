#ifndef VIRTIO_RING_H_
#define VIRTIO_RING_H_

#include "types.h"

/* Definitions for vring_desc.flags */
#define VRING_DESC_F_NEXT	    1	/* buffer continues via the next field */
#define VRING_DESC_F_WRITE	    2	/* buffer is write-only (otherwise read-only) */
#define VRING_DESC_F_INDIRECT	4	/* buffer contains a list of buffer descriptors */
/* Descriptor table entry - see Virtio Spec chapter 2.3.2 */
struct vring_desc {
	u64 addr;		/* Address (guest-physical) */
	u32 len;		/* Length */
	u16 flags;		/* The flags as indicated above */
	u16 next;		/* Next field if flags & NEXT */
};

/* Definitions for vring_avail.flags */
#define VRING_AVAIL_F_NO_INTERRUPT	1
/* Available ring - see Virtio Spec chapter 2.3.4 */
struct vring_avail {
	u16 flags;
	u16 idx;
	u16 ring[];
};

/* Definitions for vring_used.flags */
#define VRING_USED_F_NO_NOTIFY		1
struct vring_used_elem {
	u32 id;		    /* Index of start of used descriptor chain */
	u32 len;		/* Total length of the descriptor chain which was used */
};
struct vring_used {
	u16 flags;
	u16 idx;
	struct vring_used_elem ring[];
};

struct vring {
	u32                 size;   // power of 2
	int					qid;
	struct vring_desc   *desc;
	struct vring_avail  *avail;
	struct vring_used   *used;
	void 				*notify_addr;
};

/*
 * We publish the used event index at the end of the available ring, and vice
 * versa. They are at the end for backwards compatibility.
 */
#define vring_used_event(vr)  ((vr)->avail->ring[(vr)->num])
#define vring_avail_event(vr) (*(uint16_t *)&(vr)->used->ring[(vr)->num])

u32 virtio_vring_size(u32 qsize);
int virtio_vring_init(struct vring *vr, u8 *buf, u32 buf_len, u32 qsize, int qid);
void virtio_vring_fill_desc(struct vring_desc *desc, u64 addr, u32 len, u16 flags, u16 next);
void virtio_vring_add_avail(struct vring_avail *avail, u16 idx, u32 qsize);

#endif /* VIRTIO_RING_H_ */
