#include "riscv.h"
#include "riscv-virt.h"
#include "aclint_sswi.h"

#define ACLINT_SSIP0                0x0000
#define ACLINT_SSIP1                0x0004

#define ACLINT_SSWI_ADDR(reg)       ((ACLINT0_SSWI_ADDR) + (reg))
#define ACLINT_SSWI_REG32(addr)     (*(volatile u32 *)(ACLINT_SSWI_ADDR(addr)))


void aclint_set_ssip(u8 val, int hart_id)
{
    u32 reg = ACLINT_SSIP0 + 4 * hart_id;
    ACLINT_SSWI_REG32(reg) = val ? 1 : 0;
}

void aclint_clr_ssip(void)
{
#define SSIP (2)
   clear_csr(mip, SSIP);
   //clear_csr(sip, SSIP);

#undef SSIP
}