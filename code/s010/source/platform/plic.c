#include "riscv.h"
#include "riscv-virt.h"
#include "types.h"
#include "plic.h"

#define PLIC_PRIORITY           (PLIC0_ADDR + 0x0)
#define PLIC_PENDING            (PLIC0_ADDR + 0x1000)
#define PLIC_MENABLE(hart)      (PLIC0_ADDR + 0x2000 + (hart)*0x100)
#define PLIC_MTHRESHOLD(hart)   (PLIC0_ADDR + 0x200000 + (hart)*0x2000)
#define PLIC_MCLAIM(hart)       (PLIC0_ADDR + 0x200004 + (hart)*0x2000)

#define PLIC_REG(reg) (*((volatile u32 *)(reg)))

void plic_init(void)
{   // set priority
    PLIC_REG(PLIC_PRIORITY + UART0_IRQ*4) = 1;
    // set threshold
    PLIC_REG(PLIC_MTHRESHOLD(0)) = 0;
    // enable
    PLIC_REG(PLIC_MENABLE(0)) = 1 << UART0_IRQ;
}

void plic_enable(int irq)
{
  // set priority
  PLIC_REG(PLIC_PRIORITY + irq*4) = 1;
  // enable
  PLIC_REG(PLIC_MENABLE(0)) |= 1 << irq;
}

// ask the PLIC what interrupt we should serve.
int plic_claim(void)
{
  int irq = PLIC_REG(PLIC_MCLAIM(0));
  return irq;
}

// tell the PLIC we've served this IRQ.
void plic_complete(int irq)
{
  PLIC_REG(PLIC_MCLAIM(0)) = irq;
}
