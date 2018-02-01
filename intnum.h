#ifndef _INTNUM_H
#define _INTNUM_H


#define EXCP_DIVIDE 0
#define EXCP_DEBUG  1
#define EXCP_NMI    2
#define EXCP_BRKPT  3
#define EXCP_OFLOW  4
#define EXCP_BOUND  5
#define EXCP_ILLOP  6
#define EXCP_DEVICE 7
#define TRAP_DBLFLT 8
#define TRAP_CPSEG  9
#define TRAP_TSS   10 
#define TRAP_SEGNP 11
#define TRAP_STACK 12
#define TRAP_GPFLT 13
#define TRAP_PGFLT 14
#define TRAP_FPERR 16
#define TRAP_ALIGN 17
#define TRAP_MCHK  18
#define TRAP_SIMDERR 19
#define TRAP_VIRTERR 20
#define TRAP_CTRLERR 21

#define EXCP_SYSCALL 64


#define IRQ_BASE 32


#endif
