#ifndef _386_H
#define _386_H

/**
   I386 intrinsic instructions.  Much of this is based on code from the
   JOS project: 
   URL: https://github.com/lewischeng-ms/mit-jos/blob/master/inc/x86.h
*/

// Read a byte from the given input port
static __inline uint8_t inb(uint16_t port) {
  uint8_t data;
  __asm __volatile("inb %w1, %0" : "=a" (data) : "d" (port));
  return data;
}

// Read a 16-bit word from the given input port
static __inline uint16_t inw(uint16_t port)
{
  uint16_t data;
  __asm __volatile("inw %w1,%0" : "=a" (data) : "d" (port));
  return data;
}


// Read a 32-bit long-word from the given input port
static __inline uint32_t inl(uint16_t port)
{
  uint32_t data;
  __asm __volatile("inl %w1,%0" : "=a" (data) : "d" (port));
  return data;
}

// Write a byte to the given output port
static __inline void outb(uint16_t port, uint8_t value)
{
  __asm __volatile ("outb %0, %w1" : : "a" (value), "d" (port) );
}


// write cnt bytes starting at addr to the port
static __inline void outsb(uint16_t port, const void *addr, int cnt)
{
  __asm __volatile("cld\n\trepne\n\toutsb"		:
		   "=S" (addr), "=c" (cnt)		:
		   "d" (port), "0" (addr), "1" (cnt)	:
		   "cc");
}

// Invalidate a page identified by address
static __inline void invlpg(void *addr)
{ 
  __asm __volatile("invlpg (%0)" : : "r" (addr) : "memory");
}  

// Load interrupt descriptor table
static __inline void lidt(void *p)
{
  __asm __volatile("lidt (%0)" : : "r" (p));
}

// Load global descriptor table
static __inline void lgdt(void *p)
{
  __asm __volatile("lgdt (%0)" : : "r" (p));
}

// Load local descriptor table
static __inline void lldt(uint16_t sel)
{
  __asm __volatile("lldt %0" : : "r" (sel));
}

// Load task register
static __inline void ltr(uint16_t sel)
{
  __asm __volatile("ltr %0" : : "r" (sel));
}

// Load configuration register 0
static __inline void lcr0(uint32_t val)
{
  __asm __volatile("movl %0,%%cr0" : : "r" (val));
}

// Read configuration register 0
static __inline uint32_t rcr0(void)
{
  uint32_t val;
  __asm __volatile("movl %%cr0,%0" : "=r" (val));
  return val;
}

// Read configuration register 2
static __inline uint32_t rcr2(void)
{
  uint32_t val;
  __asm __volatile("movl %%cr2,%0" : "=r" (val));
  return val;
}

// Load configuration register 3
static __inline void lcr3(uint32_t val)
{
  __asm __volatile("movl %0,%%cr3" : : "r" (val));
}

// Read configuration register 3
static __inline uint32_t rcr3(void)
{
  uint32_t val;
  __asm __volatile("movl %%cr3,%0" : "=r" (val));
  return val;
}

// Load configuration register 4
static __inline void lcr4(uint32_t val)
{
  __asm __volatile("movl %0,%%cr4" : : "r" (val));
}

// Read configuration register 4
static __inline uint32_t rcr4(void)
{
  uint32_t cr4;
  __asm __volatile("movl %%cr4,%0" : "=r" (cr4));
  return cr4;
}

// Flush the TLB
static __inline void tlbflush(void)
{
  uint32_t cr3;
  __asm __volatile("movl %%cr3,%0" : "=r" (cr3));
  __asm __volatile("movl %0,%%cr3" : : "r" (cr3));
}


// Read the EFLAGS register
static __inline uint32_t read_eflags(void)
{
  uint32_t eflags;
  __asm __volatile("pushfl; popl %0" : "=r" (eflags));
  return eflags;
}


// Write to the EFLAGS register
static __inline void write_eflags(uint32_t eflags)
{
  __asm __volatile("pushl %0; popfl" : : "r" (eflags));
}

// Read EPB register
static __inline uint32_t read_ebp(void)
{ 
  uint32_t ebp;
  __asm __volatile("movl %%ebp,%0" : "=r" (ebp));
  return ebp;
}

// Read ESP register
static __inline uint32_t read_esp(void)
{
  uint32_t esp;
  __asm __volatile("movl %%esp,%0" : "=r" (esp));
  return esp;
}

// Load CPUID instruction data into given values
static __inline void cpuid(uint32_t info, uint32_t *eaxp,
			   uint32_t *ebxp, uint32_t *ecxp,
			   uint32_t *edxp)
{
  uint32_t eax, ebx, ecx, edx;
  asm volatile("cpuid" 
	       : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
	       : "a" (info));
  if (eaxp)
    *eaxp = eax;
  if (ebxp)
    *ebxp = ebx;
  if (ecxp)
    *ecxp = ecx;
  if (edxp)
    *edxp = edx;
}

// Read the system's Time Stamp Counter
static __inline uint64_t read_tsc(void)
{
  uint64_t tsc;
  __asm __volatile("rdtsc" : "=A" (tsc));
  return tsc;
}


// Atomically exchange values in memory
static inline uint32_t xchg(volatile uint32_t *addr, uint32_t newval)
{
  uint32_t result;
  
  // The + in "+m" denotes a read-modify-write operand.
  asm volatile("lock; xchgl %0, %1" :
	       "+m" (*addr), "=a" (result) :
	       "1" (newval) :
	       "cc");
  return result;
}


// Disable interrupts
static inline void cli( )
{
  asm volatile ("cli;\n");
}

static inline void sti( )
{
  asm volatile ("sti;\n");
}


#endif
