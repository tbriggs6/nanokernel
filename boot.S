#include "gdt.h"
	
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Declare a header as in the Multiboot Standard.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Allocate room for a small temporary stack as a global variable called stack.
.section .bootstrap_stack

stack_bottom:
.skip 16384 # 16 KiB
stack_top:

# The linker script specifies _start as the entry point to the kernel and the
# bootloader will jump to this position once the kernel has been loaded.
.section .text
	.global _start
_start:
	# grub put multiboot info table and magic in %ebx  and %eax respectively
	movl	$stack_top, %esp
	push	%eax
	push 	%ebx

	# make sure that we are in protected mode
	movl 	%cr0, %eax
	orl	$(1<<0), %eax
	movl	%eax, %cr0

	# load the Global Descriptor Table (GDT)
	lgdt	gdtdesc

	# reload the segment registers 
	ljmp	$0x0008,$_next
_next:	
	# setup segment registers
	movw	 $0x0010, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs	
	movw	%ax, %gs	
	movw	%ax, %ss

  	# setup a stack
	call 	kmain

	cli
hang:	hlt
	jmp hang

# Bootstrap GDT
.p2align 2                                # force 4 byte alignment
.global gdt
gdt:
  GDTENTRY( 0, 0, 0, 0)                         # null seg
  GDTENTRY(0, 0xffffffff, ACC_KERN_CODE, FLAG_DEFAULT)
  GDTENTRY(0, 0xffffffff, ACC_KERN_DATA, FLAG_DEFAULT)	
  GDTENTRY(0, 0xffffffff, ACC_USER_CODE, FLAG_DEFAULT)	
  GDTENTRY(0, 0xffffffff, ACC_USER_DATA, FLAG_DEFAULT)
  GDTENTRY(0, 0, 0, 0)
.global gdtdesc
gdtdesc:
  .word   (gdtdesc - gdt - 1)             # sizeof(gdt) - 1
  .long   gdt                             # address gdt
