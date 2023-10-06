#ifndef NS16550_H_
#define NS16550_H_

#include "types.h"

void UartInit(u32 base_addr);
int UartGetc(void);
int UartPutc(int c);
int UartOut(int c);

#endif /* NS16550_H_ */
