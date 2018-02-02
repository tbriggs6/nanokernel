#include "intnum.h"
#include "intr.h"
	
	.text

	# Load the IDTR with the address of the idt (see below)

	.global intr_loaddt
	.type intr_loaddt, @function
	
intr_loaddt:
	mov 4(%esp),%eax
	lidt (%eax)
	ret


EXCP(0, excp_handler)
EXCP(1, excp_handler)
EXCP(2, excp_handler)
EXCP(3, excp_handler)
EXCP(4, excp_handler)
EXCP(5, excp_handler)
EXCP(6, excp_handler)
EXCP(7, excp_handler)
ERR(8,  err_handler)
ERR(10,  err_handler)
ERR(11,  err_handler)
ERR(12,  err_handler)
ERR(13,  err_handler)
ERR(14,  err_handler)
EXCP(16, excp_handler)	
ERR(17,  err_handler)
EXCP(18, excp_handler)
EXCP(19, excp_handler)	

// irq 0 - 7
INTR(32, int_handler)  
INTR(33, int_handler)  
INTR(34, int_handler)
INTR(35, int_handler)
INTR(36, int_handler)
INTR(37, int_handler)
INTR(38, int_handler)
INTR(39, int_handler)

// irq 8-15
INTR(40, int_handler)
INTR(41, int_handler)
INTR(42, int_handler)
INTR(43, int_handler)
INTR(44, int_handler)
INTR(45, int_handler)
INTR(46, int_handler)
INTR(47, int_handler)

// software interrupt (later)	
INTR(48, int_handler)
	
