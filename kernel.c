#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "intnum.h"
#include "console.h"
#include "handler.h"
#include "pic.h"
#include "kstdlib.h"
#include "ps2.h"
#include "keyboard.h"
#include "kmalloc.h"
#include "multiboot.h"
#include "memory.h"
#include "task.h"

void handle_error( )
{
  console_puts("ERROR DETECTED\n");
  
}

extern uint32_t *_heap_size;
extern uint32_t *_heap_start;
extern uint32_t *_heap_end;
extern uint32_t *_idletask_start;

void show_mem(multiboot_info_t *multiboot_ptr)
{
  kprintf("Memory segments: \n");
  int num_entries = multiboot_ptr->mmap_length / (sizeof(multiboot_memory_map_t));
  multiboot_memory_map_t *entry = (multiboot_memory_map_t *) multiboot_ptr->mmap_addr;
  int i;
  for (i = 0; i < num_entries; i++) {
    kprintf("%Lx %Lx %x %x\n", entry[i].addr, entry[i].len, entry[i].size, entry[i].type);
  }

}

void kernel_main_task( )
{
  while(1) {
    asm("nop");
  }
}


void kmain(multiboot_info_t *multiboot_ptr, uint32_t multiboot_magic)
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

  kprintf("*********** KERNEL *****************\n");
  kprintf("Multiboot ptr: %p, magic: %lx\n", multiboot_ptr, multiboot_magic);
  show_mem(multiboot_ptr);
  
  memory_init(multiboot_ptr);

  // initialze paging....
  page_init( );

  kprintf("Enabling keyboard\n");
  
  keyboard_handler_t keyboard;
  keyboard.fifo = fifo_create( );

  keyboard_init_handler(&keyboard);
  ps2_init(&keyboard);


  // sigh...
  kprintf("Starting task manager.....\n");
  task_init( );
  task_t kernel_task;
  task_create_from_kernel(&kernel_task, &kernel_main_task);

  task_t task;
  //task_create_from_elf(&task, &_idletask_start);
  kprintf("Built-In tasks are loaded!\n");
  
  switchto(&kernel_task);
  
  asm volatile ("sti");
  kprintf("Finished init\n");

  // while(1) {
  //   if (keyboard.keyboard_haschar(&keyboard))
  //   {
  //     char ch = keyboard.keyboard_getchar(&keyboard);
  //     putc(ch);
  //   }
  //   else {
  //     nop();
  //   }
  // }



  // stoopd program
//   while(1) {
//     char ch;
//     ch = getchar( );
//     console_clear( );
//     console_set_pos(0,0);
//     kprintf("%x", ch);
    
//   }

}

