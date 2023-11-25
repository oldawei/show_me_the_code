#ifndef RISCV32_H_
#define RISCV32_H_
#include "types.h"

u32 handle_trap(u32 mcause, u32 mepc);
void handle_external_trap(int mode);

#endif /* RISCV32_H_ */
