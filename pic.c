#include "pic.h"

static uint32_t pic_intr_counts[16];
static uint16_t pic_mask;

#define MASTER_ADDR (0x20)
#define SLAVE_ADDR (0xA0)

static void pic_output(uint8_t value, uint16_t port)
{
  __asm __volatile ("outb %0, %w1" : : "a" (value), "d" (port) );
}

#define PIC8259_MASTER_CMD      0x20
#define PIC8259_MASTER_DATA     0x21
#define PIC8259_SLAVE_CMD       0xa0
#define PIC8259_SLAVE_DATA      0xa1
#define ICW1_IC4_NEEDED     (1 << 0)
#define ICW1_D4_BEGIN_ICW   (1 << 4)
#define ICW4_uPM_8086       (1 << 0)



void pic_init( )
{
  int i;

  __asm __volatile("cli");
  
  for (i = 0; i < 16; i++) {
    pic_intr_counts[i] = 0;
  }
  pic_mask = 0xffff;

  
  // handle master
  pic_output(0xff, MASTER_ADDR+1);
  pic_output(0x11, MASTER_ADDR);     // ICW1
  pic_output(0x20, MASTER_ADDR+1);   // ICW2
  pic_output(0x04, MASTER_ADDR+1);   // ICW3
  pic_output(0x01, MASTER_ADDR+1);   // ICW4

  pic_output(0x0a, MASTER_ADDR);     // OCW3

   // handle slave
  pic_output(0xff, SLAVE_ADDR+1);    
  pic_output(0x11, SLAVE_ADDR);      // ICW1
  pic_output(0x28, SLAVE_ADDR+1);    // ICW2
  pic_output(0x02, SLAVE_ADDR+1);    // ICW3
  pic_output(0x01, SLAVE_ADDR+1);    // ICW4

  pic_output(0x0a, SLAVE_ADDR);      // OCW3
  pic_setmask(0xffff);
}

void pic_setmask(uint16_t mask)
{
  pic_mask = mask;

  uint8_t low = (mask & 0xff) & (~(1 << 2));
  uint8_t hi = (mask >> 8) & 0xff;
  
  pic_output(low, MASTER_ADDR+1);
  pic_output(hi, SLAVE_ADDR+1);

}

uint16_t pic_getmask( void )
{
  return pic_mask;
}

void pic_increment(irq_t irq)
{
  pic_intr_counts[irq]++;
}

uint32_t pic_getcount(irq_t irq)
{
  return pic_intr_counts[irq];
}
  
void pic_end_interrupt( irq_t irq )
{
  if (irq >= 8) {
    pic_output(0x20, SLAVE_ADDR);
  }
  pic_output(0x20, MASTER_ADDR);
}

void pic_enable_interrupt(irq_t irq)
{
  pic_setmask( pic_mask & ~(1 << irq));
}

void pic_disable_interrupt(irq_t irq)
{
  pic_setmask( pic_mask | (1 << irq));
}
