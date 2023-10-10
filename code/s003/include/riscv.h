#ifndef RISCV_H_
#define RISCV_H_

// primary riscv hart id
#define PRIM_HART			      0

#define MSTATUS_MIE         0x00000008
#define MSTATUS_MPIE        0x00000080
#define MSTATUS_MPP         0x00001800

#define IRQ_M_SOFT          3
#define IRQ_M_TIMER         7
#define IRQ_M_EXT           11

#define MIP_MSIP            (1 << IRQ_M_SOFT)
#define MIP_MTIP            (1 << IRQ_M_TIMER)
#define MIP_MEIP            (1 << IRQ_M_EXT)

#if __riscv_xlen == 32
#define REGSIZE		4
#define REGSHIFT	2
#define LOAD		lw
#define STOR		sw
#elif __riscv_xlen == 64
#define REGSIZE		8
#define REGSHIFT	3
#define LOAD		ld
#define STOR		sd
#endif /* __riscv_xlen */

#define read_csr(reg) ({ unsigned long __tmp; \
  __asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
  __tmp; })

#define write_csr(reg, val) ({ \
  __asm volatile ("csrw " #reg ", %0" :: "rK"(val)); })

#define swap_csr(reg, val) ({ unsigned long __tmp; \
  __asm volatile ("csrrw %0, " #reg ", %1" : "=r"(__tmp) : "rK"(val)); \
  __tmp; })

#define set_csr(reg, bit) ({ unsigned long __tmp; \
  __asm volatile ("csrrs %0, " #reg ", %1" : "=r"(__tmp) : "rK"(bit)); \
  __tmp; })

#define clear_csr(reg, bit) ({ unsigned long __tmp; \
  __asm volatile ("csrrc %0, " #reg ", %1" : "=r"(__tmp) : "rK"(bit)); \
  __tmp; })

#define rdtime()        read_csr(time)
#define rdcycle()       read_csr(cycle)
#define rdinstret()     read_csr(instret)

#define disable_irq()   clear_csr(mstatus, MSTATUS_MIE)
#define enable_irq()    set_csr(mstatus, MSTATUS_MIE)
#define save_irq()      disable_irq()
#define restore_irq(x)  write_csr(mstatus, x)

#define cpuid()         read_csr(mhartid)

#endif //RISCV_H_
