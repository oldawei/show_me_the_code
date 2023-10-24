#ifndef VIRTIO_BLK_H_
#define VIRTIO_BLK_H_

#include "types.h"
#include "virtio-ring.h"

#define BLK_QSIZE       (128)   // blk queue0 size
#define SECTOR_SZIE     (512)   // blk sector size

// block device feature bits
#define VIRTIO_BLK_F_RO              5	/* Disk is read-only */
#define VIRTIO_BLK_F_SCSI            7	/* Supports scsi command passthru */
#define VIRTIO_BLK_F_FLUSH           9  // Cache flush command support
#define VIRTIO_BLK_F_CONFIG_WCE     11	/* Writeback mode available in config */
#define VIRTIO_BLK_F_MQ             12	/* support more than one vq */

// As per VirtIO spec Version 1.1: 5.2.4 Device configuration layout
struct virtio_blk_cfg {
	u64	capacity;
	u32	size_max;
	u32	seg_max;
	struct	virtio_blk_geometry {
		u16	cylinders;
		u8 	heads;
		u8 	sectors;
	} geometry;
	u32	blk_size;
	struct virtio_blk_topology {
		// # of logical blocks per physical block (log2)
		u8 physical_block_exp;
		// offset of first aligned logical block
		u8 alignment_offset;
		// suggested minimum I/O size in blocks
			u16 min_io_size;
		// optimal (suggested maximum) I/O size in blocks
		u32 opt_io_size;
	} topology;
	u8 writeback;
	u8 unused0[3];
	u32 max_discard_sectors;
	u32 max_discard_seg;
	u32 discard_sector_alignment;
	u32 max_write_zeroes_sectors;
	u32 max_write_zeroes_seg;
	u8 write_zeroes_may_unmap;
	u8 unused1[3];
} __attribute__((packed));

// these are specific to virtio block devices, e.g. disks,
// described in Section 5.2 of the spec.

#define VIRTIO_BLK_T_IN     0 // read the disk
#define VIRTIO_BLK_T_OUT    1 // write the disk
#define VIRTIO_BLK_T_FLUSH  4 // flush the disk

// the format of the first descriptor in a disk request.
// to be followed by two more descriptors containing
// the block, and a one-byte status.
struct virtio_blk_req {
  u32 type; // VIRTIO_BLK_T_IN or ..._OUT
  u32 reserved;
  u64 sector;
};

struct virtio_blk {
    u8  status[BLK_QSIZE];
    void *info[BLK_QSIZE];
    // disk command headers.
    // one-for-one with descriptors, for convenience.
    struct virtio_blk_req ops[BLK_QSIZE];
    struct vring vr;
	u32 capacity;
	u32 qsize;	// queue0 size
    u16 used_idx;
	u16 avail_idx;
};

struct blk_buf {
    u32 	addr;       // bytes address
    void  	*data;
    u32 	data_len;
    u16  	is_write;
    u16  	flag;
};

int virtio_blk_init(u32 base);
void virtio_blk_cfg(void);
void virtio_blk_rw(struct blk_buf *b);
void virtio_blk_intr(void);

#endif /* VIRTIO_BLK_H_ */
