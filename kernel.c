#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "intnum.h"
#include "console.h"
#include "handler.h"
#include "pic.h"

void handle_error( )
{
  console_puts("ERROR DETECTED\n");
  
}

void kmain(void)
{
  console_init( COLOR_BLUE );

  init_handler( );
  
  pic_init( );

  pic_enable_interrupt(IRQ1);
  
  asm volatile ("sti");

  
  const char *str = "hello world";
  int i = 0;
  while (str[i] != 0) {
    console_putch(str[i]);
    i++;
  }

  while(1) ; 
}


