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
	pusha                   ; \
	mov $0x10, %ax          ; \
	mov %ax,%ds             ; \
	mov %ax,%es             ; \
	mov %ax,%fs             ; \
	mov %ax,%gs             ; \
        push $(num)		; \
        call handler  		; \
        add $4, %esp            ; \
	sti                     ; \
	movb $0x20, %al         ; \
	outb %al, $0x20         ; \
	outb %al, $0xa0         ; \
	popa                    ; \
	iret

#define EXCP(num,handler)	EXCP2(num,handler)
#define EXCP2(num,handler) 	  \
  .global _excp_##num		; \
  .type _excp_##num, @function ; \
 _excp_##num: 			  \
	cli                     ; \
	pusha                   ; \
	mov $0x10, %ax          ; \
	mov %ax,%ds             ; \
	mov %ax,%es             ; \
	mov %ax,%fs             ; \
	mov %ax,%gs             ; \
	push $(num)             ; \
        call handler  		; \
        add  $4, %esp           ; \
	popa                    ; \
	iret

#define ERR(num,handler)	ERR2(num,handler)
#define ERR2(num,handler) 	\
  .global _err_##num		; \
  .type _err_##num, @function  ; \
 _err_##num: 			  \
        cli                     ; \
	pusha                   ; \
	mov $0x10, %ax          ; \
	mov %ax,%ds             ; \
	mov %ax,%es             ; \
	mov %ax,%fs             ; \
	mov %ax,%gs             ; \
	push $(num)             ; \
        call handler  		; \
	add  $4, %esp           ; \
        popa                    ; \
	add  $4, %esp           ; \
        iret
	
#endif
