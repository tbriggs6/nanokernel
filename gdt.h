#ifndef _GDT_H
#define _GDT_H

#define ACC_PRESENT ((1 << 7) | (1 << 4))

#define ACC_PRIV_KERN (0 << 5)
#define ACC_PRIV_USER (3 << 5)

#define ACC_EXEC_CONFORM (1 << 3)
#define ACC_EXEC_NON_CONFORM ((1 << 3 ) | (1 << 2))

#define ACC_DATA (0 << 3)
#define ACC_DATA_DN ((0 << 3) | (1 << 2))

#define ACC_DATA_WR (1 << 1)
#define ACC_DATA_RDONLY (0 << 1)

#define ACC_CODE_RD (1 << 1)
#define ACC_CODE_EXONLY (0 << 1)

#define FLAG_GRAN_4K (1 << 7)
#define FLAG_GRAN_BYTE (0 << 7)

#define FLAG_SIZE_16 (0 << 6)
#define FLAG_SIZE_32 (1 << 6)


#define ACC_KERN_CODE (ACC_PRESENT | ACC_PRIV_KERN | \
		       ACC_EXEC_CONFORM | ACC_CODE_RD)

#define ACC_KERN_DATA (ACC_PRESENT | ACC_PRIV_KERN |\
		       ACC_DATA | ACC_DATA_WR)

#define ACC_USER_CODE (ACC_PRESENT | ACC_PRIV_USER | \
                       ACC_EXEC_CONFORM | ACC_CODE_RD)

#define ACC_USER_DATA (ACC_PRESENT | ACC_PRIV_USER |\
                       ACC_DATA | ACC_DATA_WR)

#define FLAG_DEFAULT (FLAG_GRAN_4K | FLAG_SIZE_32)
  
#define GDTENTRY(base,limit,access,flags)		\
  .byte ((limit >> 0) & 0xff),				\
    ((limit >> 8) & 0xff),				\
    ((base >> 0) & 0xff),				\
    ((base >> 8) & 0xff),				\
    ((base >> 16) & 0xff),		\
    ((access >> 0) & 0xff),		\
    ((flags & 0xf0) | ((limit >> 16) & 0x0f)),	\
    ((base >> 24) & 0xff); 

#endif

