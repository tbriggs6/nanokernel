#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "console.h"

void kmain(void)
{
  console_init( COLOR_BLUE );

  init_handlers( );
  
  const char *str = "hello world";
  int i = 0;
  while (str[i] != 0) {
    console_putch(str[i]);
    i++;
  }
  
}


