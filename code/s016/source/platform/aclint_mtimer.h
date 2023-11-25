#ifndef ACLINT_MTIMER_H_
#define ACLINT_MTIMER_H_

#include "types.h"

void aclint_set_mtime(u64 val);
u64 aclint_get_mtime(void);
void aclint_set_mtimecmp(u64 val, int hart_id);
u64 aclint_get_mtimecmp(int hart_id);
void aclint_add_mtimecmp(u64 val, int hart_id);

#endif /* ACLINT_MTIMER_H_ */
