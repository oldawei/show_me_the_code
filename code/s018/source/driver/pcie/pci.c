#include "riscv-virt.h"
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
            printf("bar%d: 0x%08x\n", i, old);

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
