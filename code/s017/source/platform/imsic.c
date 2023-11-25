#include "riscv-virt.h"
#include "aplic.h"
#include "imsic.h"
#include "printf/printf.h"

#define IMSIC_HART_STRIDE       (0x1000)
// M-mode IMSIC CSRs
#define MISELECT                (0x350)
#define MIREG                   (0x351)
#define MTOPEI                  (0x35C)
#define MTOPI                   (0xFB0)
// S-mode IMSIC CSRs
#define SISELECT                (0x150)
#define SIREG                   (0x151)
#define STOPEI                  (0x15C)
#define STOPI                   (0xDB0)
// Constants for MISELECT/MIREG
// Pass one of these into MISELECT
// Then the MIREG will reflect that register
#define EIDELIVERY              (0x70)
#define EITHRESHOLD             (0x72)
#define EIP                     (0x80)
#define EIE                     (0xC0)

#define XLEN        (32)
#define XLEN_STRIDE (XLEN/32)

#define __ASM_STR(x)	#x

#define csr_swap(csr, val)                                              \
	({                                                              \
		unsigned long __v = (unsigned long)(val);               \
		__asm__ __volatile__("csrrw %0, " __ASM_STR(csr) ", %1" \
				     : "=r"(__v)                        \
				     : "rK"(__v)                        \
				     : "memory");                       \
		__v;                                                    \
	})

#define csr_read(csr)                                           \
	({                                                      \
		register unsigned long __v;                     \
		__asm__ __volatile__("csrr %0, " __ASM_STR(csr) \
				     : "=r"(__v)                \
				     :                          \
				     : "memory");               \
		__v;                                            \
	})

#define csr_write(csr, val)                                        \
	({                                                         \
		unsigned long __v = (unsigned long)(val);          \
		__asm__ __volatile__("csrw " __ASM_STR(csr) ", %0" \
				     :                             \
				     : "rK"(__v)                   \
				     : "memory");                  \
	})

#define imsic_write(__c, __v)	csr_write(__c, __v)
#define imsic_read(__c)	        csr_read(__c)


// mode: 0 - machine mode, 1 - supervisor mode
u32 imsic_get_addr(int mode, int hart)
{
    if (mode)
        return IMSIC0_S_ADDR + IMSIC_HART_STRIDE * hart;
    else
        return IMSIC0_M_ADDR + IMSIC_HART_STRIDE * hart;
}

void imsic_ipi_send(int mode, int hart)
{
    volatile u32 *imsi_pt = (volatile u32 *)imsic_get_addr(mode, hart);
    *imsi_pt = APLIC_IPI_IRQ;
}


// mode: 0 - machine mode, 1 - supervisor mode
void imsic_enable(int mode, int which)
{
    u32 eiebyte = EIE + XLEN_STRIDE * which / XLEN;
    u32 bit = which % XLEN;

    if (mode) {
        imsic_write(SISELECT, eiebyte);
        u32 reg = imsic_read(SIREG);
        imsic_write(SIREG, reg | 1 << bit);
    } else {
        imsic_write(MISELECT, eiebyte);
        u32 reg = imsic_read(MIREG);
        imsic_write(MIREG, reg | 1 << bit);
    }
}

// mode: 0 - machine mode, 1 - supervisor mode
void imsic_disable(int mode, int which)
{
    u32 eiebyte = EIE + XLEN_STRIDE * which / XLEN;
    u32 bit = which % XLEN;

    if (mode) {
        imsic_write(SISELECT, eiebyte);
        u32 reg = imsic_read(SIREG);
        imsic_write(SIREG, reg & ~(1 << bit));
    } else {
        imsic_write(MISELECT, eiebyte);
        u32 reg = imsic_read(MIREG);
        imsic_write(MIREG, reg & ~(1 << bit));
    }
}

// mode: 0 - machine mode, 1 - supervisor mode
void imsic_trigger(int mode, int which)
{
    u32 eipbyte = EIP + XLEN_STRIDE * which / XLEN;
    u32 bit = which % XLEN;

    if (mode) {
        imsic_write(SISELECT, eipbyte);
        u32 reg = imsic_read(SIREG);
        imsic_write(SIREG, reg | 1 << bit);
    } else {
        imsic_write(MISELECT, eipbyte);
        u32 reg = imsic_read(MIREG);
        imsic_write(MIREG, reg | 1 << bit);
    }
}

// mode: 0 - machine mode, 1 - supervisor mode
void imsic_clear(int mode, int which)
{
    u32 eipbyte = EIP + XLEN_STRIDE * which / XLEN;
    u32 bit = which % XLEN;

    if (mode) {
        imsic_write(SISELECT, eipbyte);
        u32 reg = imsic_read(SIREG);
        imsic_write(SIREG, reg & ~(1 << bit));
    } else {
        imsic_write(MISELECT, eipbyte);
        u32 reg = imsic_read(MIREG);
        imsic_write(MIREG, reg & ~(1 << bit));
    }
}

u32 imsic_get_irq(int mode)
{
    u32 val = 0;
    if (mode) {
        val = csr_swap(STOPEI, 0);
    } else {
        val = csr_swap(MTOPEI, 0);
    }

    return val >> 16;
}

void imsic_init(void)
{
    // First, enable the interrupt file
    // 0 = disabled
    // 1 = enabled
    // 0x4000_0000 = use PLIC instead
    imsic_write(MISELECT, EIDELIVERY);
    imsic_write(MIREG, 1);
    imsic_write(SISELECT, EIDELIVERY);
    imsic_write(SIREG, 1);

    // Set the interrupt threshold.
    // 0 = enable all interrupts
    // P = enable < P only
    // Priorities come from the interrupt number directly
    imsic_write(MISELECT, EITHRESHOLD);
    imsic_write(MIREG, 0);
    imsic_write(SISELECT, EITHRESHOLD);
    imsic_write(SIREG, 0);


#if 0
    // Hear message 10
    //imsic_write(SISELECT, EITHRESHOLD);
    //imsic_write(SIREG, 11);

    // Enable message #10. This will be UART when delegated by the
    // APLIC.
    imsic_enable(0, 2);
    imsic_enable(0, 4);
    //imsic_enable(1, 10);
    imsic_enable(0, 10);

    // Trigger interrupt #2
    volatile u32 *imsi_pt = (volatile u32 *)imsic_get_addr(0, 0);
    *imsi_pt = 2;

    // Trigger interrupt #4
    imsic_trigger(0, 4);

    // Trigger interrupt #10
    //imsi_pt = (volatile u32 *)imsic_get_addr(1, 0);
    *imsi_pt = 10;
    imsic_trigger(1, 10);
#endif
}