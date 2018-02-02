#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "intnum.h"
#include "console.h"
#include "handler.h"

void handle_error( )
{
  console_puts("ERROR DETECTED\n");
  
}

void kmain(void)
{
  console_init( COLOR_BLUE );

  init_handler( );
  
  const char *str = "hello world";
  int i = 0;
  while (str[i] != 0) {
    console_putch(str[i]);
    i++;
  }

  int x = 3 / str[i];
  console_putch(x);


  while(1) {
    asm("hlt");
  }
}


