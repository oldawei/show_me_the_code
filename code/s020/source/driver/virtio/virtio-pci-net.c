#include "riscv.h"
#include "aplic.h"
#include "pci.h"
#include "virtio.h"
#include "virtio-ring.h"
#include "virtio-pci.h"
#include "virtio-net.h"
#include "virtio-pci-net.h"
#include "printf/printf.h"
#include <string.h>

int virtio_pci_net_init(u32 base)
{
    u32 pci_base = pci_device_probe(0x1af4, 0x1000, base);
	if (pci_base) {
		virtio_pci_read_caps(pci_base);
		virtio_pci_print_common_cfg();
	}

    return 0;
}