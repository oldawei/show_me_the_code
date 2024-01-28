#ifndef IMSIC_H_
#define IMSIC_H_

#include "types.h"

void imsic_init(void);
u32 imsic_get_irq(int mode);
void imsic_clear(int mode, int which);
void imsic_trigger(int mode, int which);
void imsic_disable(int mode, int which);
void imsic_enable(int mode, int which);
u32 imsic_get_addr(int mode, int hart);
void imsic_ipi_send(int mode, int hart);

#endif /* IMSIC_H_ */
