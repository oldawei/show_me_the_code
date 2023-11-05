#include "riscv.h"
#include "riscv-virt.h"
#include "clint.h"

#define CLINT_MSIP0          0x0000
#define CLINT_MSIP1          0x0004
#define CLINT_MTIMECMP0      0x4000
#define CLINT_MTIMECMP1      0x4008
#define CLINT_MTIME          0xBFF8

#define CLINT_ADDR(reg)      ((CLINT0_ADDR) + (reg))
#define CLINT_REG32(addr)    (*(volatile u32 *)(CLINT_ADDR(addr)))
#define CLINT_REG64(addr)    (*(volatile u64 *)(CLINT_ADDR(addr)))

void clint_irq_init(void)
{
    set_csr(mie, MIP_MTIP | MIP_MSIP | MIP_MEIP);
    set_csr(mstatus, MSTATUS_MIE);
}

void clint_set_msip(u8 val, int hart_id)
{
    u32 reg = CLINT_MSIP0 + 4 * hart_id;
    CLINT_REG32(reg) = val ? 1 : 0;
}

void clint_set_mtime(u64 val)
{
    CLINT_REG64(CLINT_MTIME) = val;
}

u64 clint_get_mtime(void)
{
    return CLINT_REG64(CLINT_MTIME);
}

void clint_set_mtimecmp(u64 val, int hart_id)
{
    u32 reg = CLINT_MTIMECMP0 + 8 * hart_id;
    CLINT_REG64(reg) = val;
}

u64 clint_get_mtimecmp(int hart_id)
{
    u32 reg = CLINT_MTIMECMP0 + 8 * hart_id;
    return CLINT_REG64(reg);
}

void clint_add_mtimecmp(u64 val, int hart_id)
{
    u32 reg = CLINT_MTIMECMP0 + 8 * hart_id;
    CLINT_REG64(reg) += val;
}
