#include "riscv.h"
#include "virtio-ring.h"
#include "printf/printf.h"

/* Parts of the virtqueue are aligned on a 4096 byte page boundary */
#define VQ_ALIGN(addr)	((((u32)addr) + 0xfff) & ~0xfff)


/**
 * Calculate ring size according to queue size number
 */
u32 virtio_vring_size(u32 qsize)
{
	u32 size;

	size = VQ_ALIGN(qsize * sizeof(struct vring_desc)); // 4k aligned
	size += VQ_ALIGN(sizeof(struct vring_avail) + (qsize * sizeof(u16))); // 4k aligned
	size += VQ_ALIGN(sizeof(struct vring_used)
						+ (qsize * sizeof(struct vring_used_elem))); // 4k aligned
	return size;
}

int virtio_vring_init(struct vring *vr, u8 *buf, u32 buf_len, u32 qsize, int qid)
{
    u32 vr_size = virtio_vring_size(qsize);
    if (vr_size > buf_len) { // buf is small
        return -1;
    }

	printf("buf: %p, len: %d, qsize: %d\n", buf, buf_len, qsize);

	vr->size = qsize;
	vr->qid = qid;
	vr->desc = (struct vring_desc *)VQ_ALIGN(buf);
	printf("vr->desc: %p\n", vr->desc);
	vr->avail = (struct vring_avail *)VQ_ALIGN(buf + qsize * sizeof(struct vring_desc));
	printf("vr->avail: %p\n", vr->avail);
	vr->used = (struct vring_used *)VQ_ALIGN(&vr->avail->ring[qsize]);
	printf("vr->used: %p\n", vr->used);

    u8 *pt = (u8 *)(&vr->used->ring[qsize].len) + 4;
    if (pt > (buf + buf_len)) { // buf overflow
        return -2;
    }

    return 0;
}

void virtio_vring_fill_desc(struct vring_desc *desc, u64 addr, u32 len, u16 flags, u16 next)
{
	volatile struct vring_desc *pt = (volatile struct vring_desc *)desc;
	pt->addr = addr;
	pt->len = len;
	pt->flags = flags;
	pt->next = next;
	dsb();
}

void virtio_vring_add_avail(struct vring_avail *avail, u16 idx, u32 qsize)
{
	volatile struct vring_avail *pt = (volatile struct vring_avail *)avail;
	pt->ring[pt->idx % qsize] = idx;
    dsb();
    // tell the device another avail ring entry is available.
    pt->idx += 1; // not % NUM ...
	dsb();
}
