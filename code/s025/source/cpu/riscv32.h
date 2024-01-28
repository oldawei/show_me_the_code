#ifndef RISCV32_H_
#define RISCV32_H_
#include "types.h"

u32 handle_trap(u32 mcause, u32 mepc);
void handle_external_trap(int mode);

typedef int (*trap_handler_fn)(int);
void set_msix_handler(trap_handler_fn handler, int irq);

#endif /* RISCV32_H_ */
