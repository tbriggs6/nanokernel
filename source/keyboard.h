#ifndef _KEYBOARD_H
#define _KEYBOARD_H


#include "fifo.h"

struct keyboard_handler;
typedef struct keyboard_handler {
    fifo_t *fifo;                      // hold received characters
    void (*keyboard_send_char)(struct keyboard_handler *handler, uint8_t val);    // write scan code to keyboard queue
    int (*keyboard_haschar)(struct keyboard_handler *handler);                 // true if fifo has characater
    char (*keyboard_getchar)(struct keyboard_handler *handler);                 // return received character
    int state;                                  // scan-code-to-key state
    int num_lock, shift_lock, shift, alt, ctrl;           // shift, alt, or ctrl pressed
} keyboard_handler_t;


void keyboard_init_handler(keyboard_handler_t *handler);

#endif