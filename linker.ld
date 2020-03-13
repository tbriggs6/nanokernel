/* The bootloader will look at this image and start execution at the symbol
   designated at the entry point. */
ENTRY(_start)

/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
	/* Begin putting sections at 1 MiB, a conventional place for kernels to be
	   loaded at by the bootloader. */
	. = 1M;
	_kernel_start = .;

	/* First put the multiboot header, as it is required to be put very early
	   early in the image or the bootloader won't recognize the file format.
	   Next we'll put the .text section. */
	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.multiboot)
		*(.text)
		*(.inittext)	
	}

	/* Read-only data. */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
		/* built-in executables */
		*(.builtin*)
	}

	/* Read-write data (initialized) */
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
		_kernel_gdt = ALIGN(4K);
		. += 4K;
		_kernel_page_directory = ALIGN(4K);
		. += 4K;
		_kernel_tss = ALIGN(4K);
		. += 4K;
	}	
	/* Read-write data (uninitialized) and stack */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
		*(.stack)
		. += 64K;
	}
	.heap BLOCK(4K) : ALIGN(4K) 
	{
  	    _heap_start = ABSOLUTE(.) ; 
	    . += 1M;
	    _heap_end = _heap_start + 1M;
	    _heap_size = (_heap_end - _heap_start);
	    KEEP(*(.heap)) 
    }
	_kernel_end = .;
	_kernel_size = (_kernel_end - _kernel_start);
	/* The compiler may produce other sections, put them in the proper place in
	   in this file, if you'd like to include them in the final kernel. */
}
