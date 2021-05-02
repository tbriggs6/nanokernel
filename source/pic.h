#ifndef PIC_H
#define PIC_H

#include <stdint.h>

typedef enum {
  IRQ0=0, IRQ1, IRQ2, IRQ3, IRQ4, IRQ5, IRQ6, IRQ7,
  IRQ8, IRQ9, IRQ10, IRQ11, IRQ12, IRQ13, IRQ14, IRQ15
} irq_t;

void pic_init( );
void pic_setmask(uint16_t mask);
uint16_t pic_getmask( );
void pic_increment(irq_t irq);
uint32_t pic_getcount(irq_t irq);
void pic_end_interrupt( irq_t intr);
void pic_enable_interrupt(irq_t intr);
void pic_disable_interrupt(irq_t intr);

#endif
