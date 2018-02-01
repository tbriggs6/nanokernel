#include "intnum.h"

#define EXCP(id, funname) EXCP2(id,funname)
	
#define EXCP2(id, funname) \
	.global _excp_##id		;\
	.type function _excp_##id	;\
_excp_##id				;\
	pushl $(id)			;\
	call funname			;\
	iret				;\


#define TRAP(id, funname) TRAP2(id,funname)
	
#define TRAP2(id, funname) \
	.globl _trap_##id		;\
	.type function _trap_##id	;\
_trap_##id				;\
	pushl $(id)			;\
	call funname			;\
	iret				;\

#define INTRM(id, funname) INTRM2(id, funname)
#define INTRM2(id, funname) \
	.globl _intr_##id		;\
	.type function _intr_##id 	;\
_intr_##id				;\
	pushl $(id)			;\
	call funname			;\
	pushl %eax			;\
	movb $0x20, %ah			;\
	outb %ah, $0x20			;\
	popl %eax			;\
	iret				;\


#define INTRS(id, funname) INTRS2(id, funname)
#define INTRS2(id, funname) \
	.globl _intr_##id		;\
	.type function _intr_##id 	;\
_intr_id				;\
	pushl $(id)			;\
	call funname			;\
	pushl %eax			;\
	movb $0x20, %ah			;\
	outb %ah, $0x20			;\
	popl %eax			;\
	iret				;\
	
	.text

EXCP(EXCP_DIVIDE, handle_exception)
EXCP(EXCP_DEBUG, handle_exception)
EXCP(EXCP_NMI, handle_exception)
EXCP(EXCP_BRKPT, handle_exception)
EXCP(EXCP_OFLOW, handle_exception)
EXCP(EXCP_BOUND, handle_exception)
EXCP(EXCP_ILLOP, handle_exception)
EXCP(EXCP_DEVICE, handle_exception)
TRAP(TRAP_DBLFLT, handle_exception)
TRAP(TRAP_CPSEG, handle_exception)
TRAP(TRAP_TSS, handle_exception)
TRAP(TRAP_SEGNP, handle_exception)
TRAP(TRAP_STACK, handle_exception)
TRAP(TRAP_GPFLT, handle_exception)
TRAP(TRAP_PGFLT, handle_exception)
TRAP(TRAP_FPERR, handle_exception)
TRAP(TRAP_ALIGN, handle_exception)
TRAP(TRAP_MCHK, handle_exception)
TRAP(TRAP_SIMDERR, handle_exception)
TRAP(TRAP_VIRTERR, handle_exception)
TRAP(TRAP_CTRLERR, handle_exception)
EXCP(EXCP_SYSCALL, handle_exception)


INTRM(0, handle_irq)
INTRM(1, handle_irq)
INTRM(2, handle_irq)
INTRM(3, handle_irq)

INTRM(4, handle_irq)
INTRM(5, handle_irq)
INTRM(6, handle_irq)
INTRM(7, handle_irq)

INTRS(8, handle_irq)
INTRS(9, handle_irq)
INTRS(10, handle_irq)
INTRS(11, handle_irq)

INTRS(12, handle_irq)
INTRS(13, handle_irq)
INTRS(14, handle_irq)
INTRS(15, handle_irq)












	
	
