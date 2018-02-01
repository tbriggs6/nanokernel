#include "intcall.h"


void initregs(struct biosregs *reg)
{
  // memset
  char *ptr = (char *) reg;
  unsigned int i;

  for (i = 0; i < sizeof(struct biosregs); i++)
    ptr[i] = 0;

  reg->ds = ds();
  reg->es = es();
  reg->fs = fs();
  reg->gs = gs();

}
