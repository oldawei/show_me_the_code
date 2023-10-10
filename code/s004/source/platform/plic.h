#ifndef PLIC_H_
#define PLIC_H_

#define UART0_IRQ   10

extern void plic_init(void);
extern int plic_claim(void);
extern void plic_complete(int irq);

#endif /* PLIC_H_ */
