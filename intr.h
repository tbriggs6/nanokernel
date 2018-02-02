#ifndef _INTRDEF_H
#define _INTRDEF_H

// this is so that the ## macro works
// INTR gets the symbolic name
// which is re-written for the INTR2 macro
#define INTR(num,handler)	INTR2(num,handler)
#define INTR2(num,handler) 	  \
  .global _intr_##num		; \
  .type _intr_##num, @function ; \
 _intr_##num: 			  \
	cli                     ; \
        push $(num)		; \
	pusha                   ; \
	mov  %ds, %ax             ; \
	push %eax                ; \
	mov $0x10, %ax           ; \
	mov %ax,%ds               ; \
	mov %ax,%es               ; \
	mov %ax,%fs             ; \
	mov %ax,%gs             ; \
	call handler  		; \
	pop %eax                ; \
	mov %ax,%ds               ; \
	mov %ax,%es               ; \
	mov %ax,%fs             ; \
	mov %ax,%gs             ; \
	popa                    ; \
	add  $4, %esp
	iret

#define EXCP(num,handler)	EXCP2(num,handler)
#define EXCP2(num,handler) 	  \
  .global _excp_##num		; \
  .type _excp_##num, @function ; \
 _excp_##num: 			  \
	cli                     ; \
        push $(num)		; \
	pusha                   ; \
	mov  %ds, %ax             ; \
	push %eax                ; \
	mov $0x10, %ax           ; \
	mov %ax,%ds               ; \
	mov %ax,%es               ; \
	mov %ax,%fs             ; \
	mov %ax,%gs             ; \
	call handler  		; \
	pop %eax                ; \
	mov %ax,%ds               ; \
	mov %ax,%es               ; \
	mov %ax,%fs             ; \
	mov %ax,%gs             ; \
	popa                    ; \
	add  $4, %esp
	iret

#define ERR(num,handler)	ERR2(num,handler)
#define ERR2(num,handler) 	\
  .global _err_##num		; \
  .type _err_##num, @function  ; \
 _err_##num: 			  \
        cli                     ; \
        push $(num)		; \
	pusha                   ; \
	mov  %ds, %ax             ; \
	push %eax                ; \
	mov $0x10, %ax           ; \
	mov %ax,%ds               ; \
	mov %ax,%es               ; \
	mov %ax,%fs             ; \
	mov %ax,%gs             ; \
	call handler  		; \
	pop %eax                ; \
	mov %ax,%ds               ; \
	mov %ax,%es               ; \
	mov %ax,%fs             ; \
	mov %ax,%gs             ; \
	popa                    ; \
	add  $8, %esp
	iret
	
#endif
