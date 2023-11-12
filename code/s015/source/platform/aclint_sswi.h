#ifndef ACLINT_SSWI_H_
#define ACLINT_SSWI_H_

#include "types.h"

void aclint_set_ssip(u8 val, int hart_id);
void aclint_clr_ssip(void);

#endif /* ACLINT_SSWI_H_ */
