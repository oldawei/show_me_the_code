#ifndef PCI_H_
#define PCI_H_

#include "types.h"

#define PCI_ADDR_BAR0    		0x10
#define PCI_ADDR_CAP    		0x34

#define PCI_CAP_ID_VNDR		    0x09
#define PCI_CAP_ID_MSIX		    0x11

#define PCI_BAR_MIN_SZ          (4*1024)

u8 pci_config_read8(u32 offset);
u16 pci_config_read16(u32 offset);
u32 pci_config_read32(u32 offset);
void pci_config_write8(u32 offset, u8 val);
void pci_config_write16(u32 offset, u16 val);
void pci_config_write32(u32 offset, u32 val);
void pci_config_read(void *buf, u32 len, u32 offset);
u32 pci_device_probe(u16 vendor_id, u16 device_id, u32 map_addr);

#endif /* PCI_H_ */
