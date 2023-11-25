#include "riscv.h"

	.section .init
	.globl _start
	.type _start,@function
_start:
	.cfi_startproc
	.cfi_undefined ra
.option push
.option norelax
	la  gp, __global_pointer$
.option pop

	// Continue primary hart
	csrr a0, mhartid
	li   a1, PRIM_HART
	bne  a0, a1, secondary

	// set interrupt vector
	la     t0, trap_entry
  	csrw   mtvec, t0

	// Primary hart
	la sp, _stack_top

	// Load data section
	la a0, _data_lma
	la a1, _data
	la a2, _edata
	bgeu a1, a2, 2f
1:
	LOAD t0, (a0)
	STOR t0, (a1)
	addi a0, a0, REGSIZE
	addi a1, a1, REGSIZE
	bltu a1, a2, 1b
2:

	// Clear bss section
	la a0, _bss
	la a1, _ebss
	bgeu a0, a1, 2f
1:
	STOR zero, (a0)
	addi a0, a0, REGSIZE
	bltu a0, a1, 1b
2:

	// argc, argv, envp is 0
	li  a0, 0
	li  a1, 0
	li  a2, 0
	jal main
1:
	wfi
	j 1b

secondary:
	// TODO: Multicore is not supported
	wfi
	j secondary
	.cfi_endproc


#define portCONTEXT_SIZE ( 32 * REGSIZE )
 .align 2
trap_entry:
	addi sp, sp, -portCONTEXT_SIZE
	STOR x1, 1 * REGSIZE( sp )
	STOR x5, 2 * REGSIZE( sp )
	STOR x6, 3 * REGSIZE( sp )
	STOR x7, 4 * REGSIZE( sp )
	STOR x8, 5 * REGSIZE( sp )
	STOR x9, 6 * REGSIZE( sp )
	STOR x10, 7 * REGSIZE( sp )
	STOR x11, 8 * REGSIZE( sp )
	STOR x12, 9 * REGSIZE( sp )
	STOR x13, 10 * REGSIZE( sp )
	STOR x14, 11 * REGSIZE( sp )
	STOR x15, 12 * REGSIZE( sp )
	STOR x16, 13 * REGSIZE( sp )
	STOR x17, 14 * REGSIZE( sp )
	STOR x18, 15 * REGSIZE( sp )
	STOR x19, 16 * REGSIZE( sp )
	STOR x20, 17 * REGSIZE( sp )
	STOR x21, 18 * REGSIZE( sp )
	STOR x22, 19 * REGSIZE( sp )
	STOR x23, 20 * REGSIZE( sp )
	STOR x24, 21 * REGSIZE( sp )
	STOR x25, 22 * REGSIZE( sp )
	STOR x26, 23 * REGSIZE( sp )
	STOR x27, 24 * REGSIZE( sp )
	STOR x28, 25 * REGSIZE( sp )
	STOR x29, 26 * REGSIZE( sp )
	STOR x30, 27 * REGSIZE( sp )
	STOR x31, 28 * REGSIZE( sp )

	csrr t0, mstatus					/* Required for MPIE bit. */
	STOR t0, 29 * REGSIZE( sp )

	// call C function: handle_trap
	csrr a0, mcause
	csrr a1, mepc
	jal handle_trap
	csrw mepc, a0

	/* Load mstatus with the interrupt enable bits used by the task. */
	LOAD  t0, 29 * REGSIZE( sp )
	csrw mstatus, t0						/* Required for MPIE bit. */

	LOAD  x1, 1 * REGSIZE( sp )
	LOAD  x5, 2 * REGSIZE( sp )		/* t0 */
	LOAD  x6, 3 * REGSIZE( sp )		/* t1 */
	LOAD  x7, 4 * REGSIZE( sp )		/* t2 */
	LOAD  x8, 5 * REGSIZE( sp )		/* s0/fp */
	LOAD  x9, 6 * REGSIZE( sp )		/* s1 */
	LOAD  x10, 7 * REGSIZE( sp )	/* a0 */
	LOAD  x11, 8 * REGSIZE( sp )	/* a1 */
	LOAD  x12, 9 * REGSIZE( sp )	/* a2 */
	LOAD  x13, 10 * REGSIZE( sp )	/* a3 */
	LOAD  x14, 11 * REGSIZE( sp )	/* a4 */
	LOAD  x15, 12 * REGSIZE( sp )	/* a5 */
	LOAD  x16, 13 * REGSIZE( sp )	/* a6 */
	LOAD  x17, 14 * REGSIZE( sp )	/* a7 */
	LOAD  x18, 15 * REGSIZE( sp )	/* s2 */
	LOAD  x19, 16 * REGSIZE( sp )	/* s3 */
	LOAD  x20, 17 * REGSIZE( sp )	/* s4 */
	LOAD  x21, 18 * REGSIZE( sp )	/* s5 */
	LOAD  x22, 19 * REGSIZE( sp )	/* s6 */
	LOAD  x23, 20 * REGSIZE( sp )	/* s7 */
	LOAD  x24, 21 * REGSIZE( sp )	/* s8 */
	LOAD  x25, 22 * REGSIZE( sp )	/* s9 */
	LOAD  x26, 23 * REGSIZE( sp )	/* s10 */
	LOAD  x27, 24 * REGSIZE( sp )	/* s11 */
	LOAD  x28, 25 * REGSIZE( sp )	/* t3 */
	LOAD  x29, 26 * REGSIZE( sp )	/* t4 */
	LOAD  x30, 27 * REGSIZE( sp )	/* t5 */
	LOAD  x31, 28 * REGSIZE( sp )	/* t6 */
	addi sp, sp, portCONTEXT_SIZE

	mret
