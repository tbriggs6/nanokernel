#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "intnum.h"
#include "console.h"
#include "handler.h"
#include "pic.h"
#include "kstdlib.h"

void handle_error( )
{
  console_puts("ERROR DETECTED\n");
  
}

extern uint32_t *_heap_size;
extern uint32_t *_heap_start;
extern uint32_t *_heap_end;

void kmain(void)
{

  // create a heap
  kmalloc_init( );
  
  // initialize chrdevs
  chrdev_init( );

  // handle interrupts
  init_handler( );
    
  // handle PIC
  pic_init( );
  pic_enable_interrupt(IRQ1);

  // sets stdout
  console_init( COLOR_BLUE );

  kprintf("Enabling keyboard\n");
  // sets stdin
  keyboard_init( );

  kprintf("Finished init\n");
  
  asm volatile ("sti");

  // stoopd program
  while(1) {
    char ch;
    ch = getchar( );
    console_clear( );
    console_set_pos(0,0);
    kprintf("%x", ch);
    
  }

}


