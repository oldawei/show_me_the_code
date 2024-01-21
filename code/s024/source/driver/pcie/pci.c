#include "riscv-virt.h"
#include "aplic.h"
#include "imsic.h"
#include "pci.h"
#include "printf/printf.h"

#define PCI_ADDR(addr)  (PCIE0_ECAM + (u32)(addr))
#define PCI_REG8(reg)  (*(volatile u8 *)PCI_ADDR(reg))
#define PCI_REG16(reg)  (*(volatile u16 *)PCI_ADDR(reg))
#define PCI_REG32(reg)  (*(volatile u32 *)PCI_ADDR(reg))

u8 pci_config_read8(u32 offset)
{
    return PCI_REG8(offset);
}

u16 pci_config_read16(u32 offset)
{
    return PCI_REG16(offset);
}

u32 pci_config_read32(u32 offset)
{
    return PCI_REG32(offset);
}

void pci_config_write8(u32 offset, u8 val)
{
    PCI_REG8(offset) = val;
}

void pci_config_write16(u32 offset, u16 val)
{
    PCI_REG16(offset) = val;
}

void pci_config_write32(u32 offset, u32 val)
{
    PCI_REG32(offset) = val;
}

void pci_config_read(void *buf, u32 len, u32 offset)
{
    volatile u8 *dst = (volatile u8 *)buf;
    while (len) {
        *dst = PCI_REG8(offset);
        --len;
        ++dst;
        ++offset;
    }
}

u32 pci_device_probe(u16 vendor_id, u16 device_id, u32 map_addr)
{
    u32 pci_id = (device_id << 16) | vendor_id;
    u32 ret = 0;

    for (int dev = 0; dev < 32; dev++) {
        int bus = 0;
        int func = 0;
        int offset = 0;
        u32 off = (bus << 16) | (dev << 11) | (func << 8) | (offset);
        volatile u32 *base = (volatile u32 *)PCI_ADDR(off);
        u32 id = base[0];

        if (id != pci_id) continue;

        ret = off;
        //printf("cap: 0x%02x\n", pci_config_read8(off + PCI_ADDR_CAP));

        // command and status register.
        // bit 0 : I/O access enable
        // bit 1 : memory access enable
        // bit 2 : enable mastering
        base[1] = 7;
        __sync_synchronize();

        for(int i = 0; i < 6; i++) {
            u32 old = base[4+i];
            printf("bar%d old: 0x%08x\n", i, old);

            // writing all 1's to the BAR causes it to be
            // replaced with its size.
            base[4+i] = 0xffffffff;
            __sync_synchronize();

            u32 sz = base[4+i];
            sz = ~(sz & 0xFFFFFFF0) + 1;
            printf("bar%d size: 0x%08x -> 0x%08x\n", i, base[4+i], sz);

            if (i == 1) {
                u32 addr = (map_addr + (sz -1)) & (~(sz -1));
                base[4+i] = addr + 0x8000;
            } else if (i == 4) {
                u32 addr = (map_addr + (sz -1)) & (~(sz -1));
                base[4+i] = addr;
                map_addr = addr + sz;
            } else {
                base[4+i] = 0x00;
            }

            printf("bar%d mem_addr: 0x%08x\n", i, base[4+i]);
        }
    }
    printf("vendor_id: 0x%08x\n", vendor_id);
    printf("device_id: 0x%08x\n", device_id);
    printf("bar_addr : 0x%08x\n", ret);
    return ret;
}

void pci_set_msix(struct pci_msix *msix, u32 pci_base, u32 pos)
{
    struct pci_msix_cap cap;
    // enable msi-x
    struct pci_msix_cap *msix_cap = (struct pci_msix_cap *)&cap;
    pci_config_read(&cap, sizeof(cap), pos);

    pci_config_write16(pos + 2, msix_cap->ctrl | PCI_MSIX_ENABLE);
    // read cap again
    pci_config_read(&cap, sizeof(cap), pos);

    /* Transitional devices would also have this capability,
        * that's why we also check if msix is enabled.
        * 1st byte is cap ID; 2nd byte is the position of next
        * cap; next two bytes are the flags.
        */
    printf("ctrl: 0x%04x\n", msix_cap->ctrl);
    printf("table: 0x%08x\n", msix_cap->table);
    printf("pba: 0x%08x\n", msix_cap->pba);

    int irq_num = (msix_cap->ctrl & MSIX_SIZE_MASK) + 1;
    int msix_bar = (msix_cap->table & MSIX_BAR_MASK);
    int tbl_off = (msix_cap->table & ~MSIX_BAR_MASK);
    int pba_off = (msix_cap->pba & ~MSIX_BAR_MASK);
    printf("irq_num: %d\n", irq_num);
    printf("msix_bar: %d\n", msix_bar);
    printf("tbl_off: 0x%08x\n", tbl_off);
    printf("pba_off: 0x%08x\n", pba_off);

    u32 reg = pci_base + PCI_ADDR_BAR0 + 4 * msix_bar;
    u32 msix_addr = pci_config_read32(reg) & 0xFFFFFFF0;
    u32 tbl_addr = msix_addr + tbl_off;
    u32 pba_addr = msix_addr + pba_off;
    printf("msix_addr: 0x%08x\n", msix_addr);
    printf("tbl_addr: 0x%08x\n", tbl_addr);
    printf("pba_addr: 0x%08x\n", pba_addr);

    msix->bar_num = msix_bar;
    msix->irq_num = irq_num;
    msix->tbl_addr = tbl_addr;
    msix->pba_addr = pba_addr;

    struct pci_msix_tbl *tbl = (struct pci_msix_tbl *)msix->tbl_addr;
    u32 imsic_addr = imsic_get_addr(APLIC_MACHINE, 0);
    u32 imsic_irq = APLIC_MSIX0_IRQ;

    for (int i = 0; i < msix->irq_num; ++i) {
        tbl->addr_l = imsic_addr;
        tbl->addr_h = 0;
        tbl->msg_data = imsic_irq + i;
        tbl->ctrl = 0;
        ++tbl;
        aplic_enable(imsic_irq + i);
    }
}
