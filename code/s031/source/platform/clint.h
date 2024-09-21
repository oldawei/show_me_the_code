#ifndef CLINT_H_
#define CLINT_H_

#include "types.h"

#define CLINT_MSIP0          0x0000
#define CLINT_MSIP1          0x0004
#define CLINT_MTIMECMP0      0x4000
#define CLINT_MTIMECMP1      0x4008
#define CLINT_MTIME          0xBFF8

void clint_irq_init(void);
extern void clint_set_msip(u8 val, int hart_id);
extern void clint_set_mtime(u64 val);
extern u64 clint_get_mtime(void);
extern void clint_set_mtimecmp(u64 val, int hart_id);
extern u64 clint_get_mtimecmp(int hart_id);
extern void clint_add_mtimecmp(u64 val, int hart_id);

#endif /* CLINT_H_ */
