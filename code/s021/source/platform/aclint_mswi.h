#ifndef ACLINT_MSWI_H_
#define ACLINT_MSWI_H_

#include "types.h"

void aclint_irq_init(void);
void aclint_set_msip(u8 val, int hart_id);

#endif /* ACLINT_MSWI_H_ */
