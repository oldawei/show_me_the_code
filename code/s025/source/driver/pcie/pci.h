#ifndef PCI_H_
#define PCI_H_

#include "types.h"
#include "riscv32.h"

#define PCI_ADDR_BAR0    		0x10
#define PCI_ADDR_CAP    		0x34

#define PCI_CAP_ID_VNDR		    0x09
#define PCI_CAP_ID_MSIX		    0x11

#define PCI_BAR_MIN_SZ          (4*1024)

#define PCI_MSIX_ENABLE 0x8000
struct pci_msix_cap {
    u8 cap_vndr; /* Generic PCI field: PCI_CAP_ID_VNDR */
    u8 cap_next; /* Generic PCI field: next ptr. */
#define MSIX_ENABLE_BIT (1 << 15)
#define MSIX_MASK_BIT   (1 << 14)
#define MSIX_SIZE_MASK  (0x7FF)
    u16 ctrl;
#define MSIX_BAR_MASK  (0x07)
    u32 table;
    u32 pba;
};

struct pci_msix {
    int bar_num;
    int irq_num;
    u32 tbl_addr;
    u32 pba_addr;
};

struct pci_msix_tbl {
    u32 addr_l;
    u32 addr_h;
    u32 msg_data;
    u32 ctrl;
};


u8 pci_config_read8(u32 offset);
u16 pci_config_read16(u32 offset);
u32 pci_config_read32(u32 offset);
void pci_config_write8(u32 offset, u8 val);
void pci_config_write16(u32 offset, u16 val);
void pci_config_write32(u32 offset, u32 val);
void pci_config_read(void *buf, u32 len, u32 offset);
u32 pci_device_probe(u16 vendor_id, u16 device_id);
void pci_set_msix(struct pci_msix *msix, u32 pci_base, u32 pos, trap_handler_fn *msix_isr);
u32 pci_alloc_irq_number(void);
u32 pci_alloc_mmio(u32 sz);
#endif /* PCI_H_ */
