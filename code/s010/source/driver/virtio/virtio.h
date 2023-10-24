#ifndef VIRTIO_H_
#define VIRTIO_H_

#include "types.h"

/* Device status bits */
#define VIRTIO_STAT_ACKNOWLEDGE		1
#define VIRTIO_STAT_DRIVER		    2
#define VIRTIO_STAT_DRIVER_OK		4
#define VIRTIO_STAT_FEATURES_OK		8
#define VIRTIO_STAT_NEEDS_RESET		64
#define VIRTIO_STAT_FAILED		    128

/* VIRTIO 1.0 Device independent feature bits */
#define VIRTIO_F_ANY_LAYOUT         (27)
#define VIRTIO_F_INDIRECT_DESC	    (28)
#define VIRTIO_F_EVENT_IDX	        (29)
#define VIRTIO_F_VERSION_1	        (32)
#define VIRTIO_F_RING_PACKED	    (34)
#define VIRTIO_F_IN_ORDER	        (35)
#define VIRTIO_F_ORDER_PLATFORM	    (36)
#define VIRTIO_F_SR_IOV	            (37)
#define VIRTIO_F_NOTIFICATION_DATA  (38)
#define VIRTIO_F_NOTIF_CONFIG_DATA  (39)
#define VIRTIO_F_RING_RESET	        (40)

#endif /* VIRTIO_H_ */
