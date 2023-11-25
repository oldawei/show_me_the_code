#ifndef CLINT_H_
#define CLINT_H_

#include "types.h"

void clint_irq_init(void);
extern void clint_set_msip(u8 val, int hart_id);
extern void clint_set_mtime(u64 val);
extern u64 clint_get_mtime(void);
extern void clint_set_mtimecmp(u64 val, int hart_id);
extern u64 clint_get_mtimecmp(int hart_id);
extern void clint_add_mtimecmp(u64 val, int hart_id);

#endif /* CLINT_H_ */
