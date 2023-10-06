#ifndef RISCV_VIRT_H_
#define RISCV_VIRT_H_

#define NS16550_ADDR		0x10000000UL

void plt_virt_init(void);
void println( const char *s );

#endif /* RISCV_VIRT_H_ */
