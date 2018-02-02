#include <stdint.h>
#include "handler.h"
#include "intnum.h"
#include "console.h"


void (*handlers[NUM_IRQ])(int);

static idt_entry_t idt_entries[NUM_IRQ];

static void idt_register_handler(uint16_t intnum, idt_gate_t type, void (*handler)(void))
{
   uint32_t handler_address_asint = (uint32_t) handler;

  idt_entries[intnum].offset_lo = (handler_address_asint & 0xffff);
  idt_entries[intnum].selector = 0x08;
  idt_entries[intnum].zero = 0;
  idt_entries[intnum].type = 0x8e;
  idt_entries[intnum].offset_hi = (handler_address_asint >> 16) & 0xffff;
}

void excp_handler(int excp_num)
{
  switch(excp_num) {
  case 0: console_puts("EXP 0"); break;
  case 1: console_puts("EXP 0"); break;
  case 2: console_puts("EXP 0"); break;
  case 3: console_puts("EXP 0"); break;
  case 4: console_puts("EXP 0"); break;
  case 5: console_puts("EXP 0"); break;
  case 6: console_puts("EXP 0"); break;
  case 7: console_puts("EXP 0"); break;
  case 16: console_puts("EXP 0"); break;
  case 18: console_puts("EXP 0"); break;
  case 19: console_puts("EXP 0"); break;
  default: console_puts("EXP UNKNOWN"); break;
  }
  asm("hang_excp: hlt; jmp hang_excp;\n");
  return;
}

void err_handler(int err_num)
{

  switch(err_num) {
  case 8: console_puts("ERR 8"); break;
  case 10: console_puts("ERR 10"); break;
  case 11: console_puts("ERR 11"); break;
  case 12: console_puts("ERR 12"); break;
  case 13: console_puts("ERR 13"); break;
  case 14: console_puts("ERR 14"); break;
  case 17: console_puts("ERR 15"); break;
  default: console_puts("EXP UNKNOWN"); break;
  }
  asm("hang_err: hlt; jmp hang_err;\n");
  return;
}

void int_handler(int int_num)
{
  switch(int_num) {
  case 32: console_puts("INT 32"); break;
  case 33: console_puts("INT 33"); break;
  case 34: console_puts("INT 34"); break;
  case 35: console_puts("INT 35"); break;
  case 36: console_puts("INT 36"); break;
  case 37: console_puts("INT 37"); break;
  case 38: console_puts("INT 38"); break;
  case 39: console_puts("INT 39"); break;
  case 40: console_puts("INT 40"); break;
  case 41: console_puts("INT 41"); break;
  case 42: console_puts("INT 42"); break;
  case 43: console_puts("INT 43"); break;
  case 44: console_puts("INT 44"); break;
  case 45: console_puts("INT 45"); break;
  case 46: console_puts("INT 46"); break;
  case 47: console_puts("INT 47"); break;
  case 48: console_puts("INT 48"); break;
  default: console_puts("INT UNKNOWN"); break;
  }
  return;
}

static void zero_handlers( )
{
  int i;
  for (i = 0; i < NUM_IRQ; i++) {
    handlers[i] = (void *) 0;
  }
}
    
void register_handler(uint16_t intnum,  void (*handler)(int))
{
  if ((intnum > 0) && (intnum < NUM_IRQ))
    handlers[intnum] = handler;
}


void init_handler( )
{

  zero_handlers( );
  
  idt_register_handler(0, TRAP32, _excp_0);
  idt_register_handler(1, TRAP32, _excp_1);
  idt_register_handler(2, TRAP32, _excp_2);
  idt_register_handler(3, TRAP32, _excp_3);
  idt_register_handler(4, TRAP32, _excp_4);
  idt_register_handler(5, TRAP32, _excp_5);
  idt_register_handler(6, TRAP32, _excp_6);
  idt_register_handler(7, TRAP32, _excp_7);
  idt_register_handler(8, TRAP32, _err_8);

  idt_register_handler(10, TRAP32, _err_10);
  idt_register_handler(11, TRAP32, _err_11);
  idt_register_handler(12, TRAP32, _err_12);
  idt_register_handler(13, TRAP32, _err_13);
  idt_register_handler(14, TRAP32, _err_14);

  idt_register_handler(16, TRAP32, _excp_16);
  idt_register_handler(17, TRAP32, _err_17);
  idt_register_handler(18, TRAP32, _excp_18);
  idt_register_handler(19, TRAP32, _excp_19);
  idt_register_handler(32, INTR32, _intr_32);
  idt_register_handler(33, INTR32, _intr_33);
  idt_register_handler(34, INTR32, _intr_34);
  idt_register_handler(35, INTR32, _intr_35);
  idt_register_handler(36, INTR32, _intr_36);
  idt_register_handler(37, INTR32, _intr_37);
  idt_register_handler(38, INTR32, _intr_38);
  idt_register_handler(39, INTR32, _intr_39);
  idt_register_handler(40, INTR32, _intr_40);
  idt_register_handler(41, INTR32, _intr_41);
  idt_register_handler(42, INTR32, _intr_42);
  idt_register_handler(43, INTR32, _intr_43);
  idt_register_handler(44, INTR32, _intr_44);
  idt_register_handler(45, INTR32, _intr_45);
  idt_register_handler(46, INTR32, _intr_46);
  idt_register_handler(47, INTR32, _intr_47);
  idt_register_handler(48, INTR32, _intr_48);


  idt_desc_t idt_desc = { .limit = sizeof(idt_entry_t) * 50 - 1,
			  .base = (uint32_t) &idt_entries };
  
  intr_loaddt(&idt_desc);
}
