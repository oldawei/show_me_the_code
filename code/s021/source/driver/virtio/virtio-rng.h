#ifndef VIRTIO_RNG_H_
#define VIRTIO_RNG_H_

#include "types.h"
#include "virtio-ring.h"

#define RNG_QSIZE       (128)   // rng queue0 size

struct virtio_rng {
    struct vring vr;
	u32 qsize;	// queue0 size
    u16 used_idx;
	u16 avail_idx;
};


int virtio_rng_init(u32 base);
int virtio_rng_read(u8 *buf, int len);


#endif /* VIRTIO_RNG_H_ */
