#include "riscv.h"
#include "riscv-virt.h"
#include "aclint_mswi.h"

#define ACLINT_MSIP0                0x0000
#define ACLINT_MSIP1                0x0004

//#define ACLINT_MSWI_ADDR(reg)       ((ACLINT0_MSWI_ADDR) + (reg))
//#define ACLINT_MSWI_REG32(addr)     (*(volatile u32 *)(ACLINT_MSWI_ADDR(addr)))

void aclint_irq_init(void)
{
    set_csr(mie, MIP_MTIP | MIP_MSIP | MIP_MEIP);
    set_csr(mstatus, MSTATUS_MIE);

    set_csr(mie, MIP_SSIP | MIP_SEIP);
    set_csr(mstatus, MSTATUS_SIE);
}

//void aclint_set_msip(u8 val, int hart_id)
//{
//    u32 reg = ACLINT_MSIP0 + 4 * hart_id;
//    ACLINT_MSWI_REG32(reg) = val ? 1 : 0;
//}
