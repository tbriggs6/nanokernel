#include <stdint.h>
#include "handler.h"
#include "intnum.h"

typedef struct {
  uint16_t offset_lo;   // offset bits 0..15
  uint16_t selector;    // selector
  uint8_t zero;         // zeros
  uint8_t type_attr;    // type and attributes
  uint16_t offset_hi;   // offset bits 31:16
} idt_entry_t;

typedef struct {
  uint16_t limit;
  uint32_t base;
} idtr_t;

/* the low-level template for handlers is
   defined in intr.S 

   -- EXCEP(id, funname)  will create a 
   _excep_id  function that will call:  funname(id)

   -- TRAP(id, funname) will create a 
   _trap_id function that will call funname(id)

   -- INTR(id, funname) will create an
   _intr_id function that will call funname(0)

   The _excep_id, _trap_id, and _intr_id are what 
   must be loaded into the IDT as the handler.
*/


void handle_exception(uint32_t EXCP)
{
  return;
}


void handle_irq(uint32_t IRQ)
{
  return;
}

static idt_entry_t entries[IDT_NUM_ENTRY];

static void exception_set(int number, void (*handler)(void))
{
  entries[number].offset_lo = ((uint32_t) handler & 0xffff);
  entries[number].offset_hi = ((uint32_t) handler >> 16) & 0xffff;
  entries[number].zero = 0;
  entries[number].type_attr = (1 << 7) | 0x0f;
  entries[number].selector = 0 | 0 << 2 | 0x08 << 3;
}

static void trap_set(int number, void (*handler)(void))
{
  entries[number].offset_lo = ((uint32_t)handler & 0xffff);
  entries[number].offset_hi = ((uint32_t)handler >> 16) & 0xffff;
  entries[number].zero = 0;
  entries[number].type_attr = (1 << 7) | 0x0e;
  entries[number].selector = 0 | 0 << 2 | 0x08 << 3;
}

static void interrupt_set(int number, void (*handler)(void))
{
  entries[number].offset_lo = ((uint32_t)handler & 0xffff);
  entries[number].offset_hi = ((uint32_t)handler >> 16) & 0xffff;
  entries[number].zero = 0;
  entries[number].type_attr = (1 << 7) | 0x0e;
  entries[number].selector = 0 | 0 << 2 | 0x08 << 3;
}

static void zero_set(int number)
{
  entries[number].offset_lo = 0;
  entries[number].offset_hi = 0;
  entries[number].zero = 0;
  entries[number].type_attr = 0;
  entries[number].selector = 0;
}


extern void _excp_0(void);
extern void _excp_1(void);
extern void _excp_2(void);
extern void _excp_3(void);
extern void _excp_4(void);
extern void _excp_5(void);
extern void _excp_6(void);
extern void _excp_7(void);
extern void _trap_8(void);
extern void _trap_9(void);
extern void _trap_10(void);
extern void _trap_11(void);
extern void _trap_12(void);
extern void _trap_13(void);
extern void _trap_14(void);
extern void _trap_15(void);
extern void _trap_16(void);
extern void _trap_17(void);
extern void _trap_18(void);
extern void _trap_19(void);
extern void _trap_20(void);
extern void _trap_21(void);
extern void _excp_64(void);
extern void _intr_0(void);
extern void _intr_1(void);
extern void _intr_2(void);
extern void _intr_3(void);
extern void _intr_4(void);
extern void _intr_5(void);
extern void _intr_6(void);
extern void _intr_7(void);
extern void _intr_8(void);
extern void _intr_9(void);
extern void _intr_10(void);
extern void _intr_11(void);
extern void _intr_12(void);
extern void _intr_13(void);
extern void _intr_14(void);
extern void _intr_15(void);


void init_handlers( )
{
  int i;

  for (i = 0; i < IDT_NUM_ENTRY; i++) {
    zero_set(i);
  }

  exception_set(0, _excp_0);
  exception_set(1, _excp_1);
  exception_set(2, _excp_2);
  exception_set(3, _excp_3);
  exception_set(4, _excp_4);
  exception_set(5, _excp_5);
  exception_set(6, _excp_6);
  exception_set(7, _excp_7);
  trap_set(8, _trap_8);
  trap_set(9, _trap_9);
  trap_set(10, _trap_10);
  trap_set(11, _trap_11);
  trap_set(12, _trap_12);
  trap_set(13, _trap_13);
  trap_set(14, _trap_14);
  trap_set(15, _trap_15);
  trap_set(16, _trap_16);
  trap_set(17, _trap_17);
  trap_set(18, _trap_18);
  trap_set(19, _trap_19);
  trap_set(20, _trap_20);
  trap_set(21, _trap_21);

  exception_set(64, _excp_64);


  interrupt_set(32, _intr_0);
  interrupt_set(33, _intr_1);
  interrupt_set(34, _intr_2);
  interrupt_set(35, _intr_3);
  interrupt_set(36, _intr_4);
  interrupt_set(37, _intr_5);
  interrupt_set(38, _intr_6);
  interrupt_set(39, _intr_7);
  interrupt_set(40, _intr_8);
  interrupt_set(41, _intr_9);
  interrupt_set(42, _intr_10);
  interrupt_set(43, _intr_11);
  interrupt_set(44, _intr_12);
  interrupt_set(45, _intr_13);
  interrupt_set(46, _intr_14);
  interrupt_set(47, _intr_15);


  idtr_t idtr;
  idtr.limit = (IDT_NUM_ENTRY * 8) - 1;
  idtr.base = (uint32_t) &entries;

  uint32_t idtr_addr = (uint32_t) &idtr;
  asm (
       "lidt (%0);\n"
       :
       :"r"(idtr_addr)
       :
       );
  
}
