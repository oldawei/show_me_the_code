#include "riscv.h"
#include "riscv-virt.h"
#include "aclint_mtimer.h"


#define ACLINT_MTIMECMP0            0x0000
#define ACLINT_MTIMECMP1            0x0008
#define ACLINT_MTIME                0x7FF8

#define ACLINT_MTIME_ADDR(reg)      ((ACLINT0_MTIMER_ADDR) + (reg))
#define ACLINT_MTIME_REG32(addr)    (*(volatile u32 *)(ACLINT_MTIME_ADDR(addr)))
#define ACLINT_MTIME_REG64(addr)    (*(volatile u64 *)(ACLINT_MTIME_ADDR(addr)))


void aclint_set_mtime(u64 val)
{
    ACLINT_MTIME_REG64(ACLINT_MTIME) = val;
}

u64 aclint_get_mtime(void)
{
    return ACLINT_MTIME_REG64(ACLINT_MTIME);
}

void aclint_set_mtimecmp(u64 val, int hart_id)
{
    u32 reg = ACLINT_MTIMECMP0 + 8 * hart_id;
    ACLINT_MTIME_REG64(reg) = val;
}

u64 aclint_get_mtimecmp(int hart_id)
{
    u32 reg = ACLINT_MTIMECMP0 + 8 * hart_id;
    return ACLINT_MTIME_REG64(reg);
}

void aclint_add_mtimecmp(u64 val, int hart_id)
{
    u32 reg = ACLINT_MTIMECMP0 + 8 * hart_id;
    ACLINT_MTIME_REG64(reg) += val;
}
