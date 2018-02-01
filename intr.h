#ifndef _INTR_H
#define_INTR_H


#define TRAP(name,num) \
	.globl trap_##name		; \
	.type trap_##name @function	; \
	.align 2			; \
trap_##name:				; \
	pushl $(num)			; \
	call handler			; \
	pop %eax			; \
	popa				; \
	retfi				; \

#define EXCP(name) \
	.globl trap_##name		; \
	.type trap_##name @function	; \
	.align 2			; \
trap_##name:				; \
	call name			; \
	pop %eax			; \
	popa				; \
	retfi				
	
#define INTR(name,num) 		\
	.globl intr_##name		; \
	.type intr_##name @function	; \
	.align 2			; \
intr_##name:				; \
	call name			; \
	pop %eax			; \
	popa				; \
	retfi				; \

#endif
