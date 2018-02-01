#ifndef _INTR_H
#define _INTR_H


#define TRAP(name,num) \
	.globl trap_#num		; \
	.type trap_##num @function	; \
	.align 2			; \
trap_##num:				; \
	pushl $(num)			; \
	call handler			; \
	pop %eax			; \
	popa				; \
	retfi				; \

#define EXCP(name,num)			  \
	.globl excp_##num		; \
	.type excp_##num @function	; \
	.align 2			; \
excp_##num:				; \
        call name			;	  \
	pop %eax			; \
	popa				; \
	retfi				
	
#define INTR(name,num) 		\
	.globl intr_##num		; \
	.type intr_##num @function	; \
	.align 2			; \
intr_##num:				; \
	call name			; \
	pop %eax			; \
	popa				; \
	retfi				; \

#endif
