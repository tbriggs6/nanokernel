#ifndef _IDT_H
#define _DDT_H


#define FLAG_PRESENT (1 << 7)

#define FLAG_DPL_USER (3 << 5)
#define FLAG_DPL_KERN (0 << 5)

#define FLAG_INTR (0 << 4)

#define FLAG_TYPE_TASK32 (0x5 << 0)
#define FLAG_TYPE_INTR16 (0x6 << 0)
#define FLAG_TYPE_TRAP16 (0x7 << 0)
#define FLAG_TYPE_INTR32 (0xE << 0)
#define FLAG_TYPE_TRAP32 (0xF << 0)


#define FLAG_INTR32 (FLAG_PRESET | FLAG_DPL_KERN | FLAG_INTR | FLAG_INTR32)
#define FLAG_TRAP32 (FLAG_PRESET | FLAG_DPL_KERN | FLAG_INTR | FLAG_TRAP32)

#define IDTENTRY(offset, flag, selector) \
  .byte ((offset >> 0) & 0xff), \
    ((offset >> 8) & 0xff), \
    ((selector >> 0) & 0xff), \
    ((selector >> 8) & 0xff), \
    0, \
    (flag & 0xff), \
    ((offset >> 16) & 0xff), \
    ((offset >> 24) & 0xff)


intignore:
       pop %ax
       ret


#ifndef NEWLINE
#define NEWLINE ??/n
       
#define INTR(intr, offset) \
INTR00: \
  pushl %eax \
  pushl $0                          \
  call intignore                    \
  movb $0x20, %al                   \
  outb %al, $0x20                   \
  popl %eax                         \
  iret 

       
#endif

