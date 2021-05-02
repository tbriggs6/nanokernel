#include "keyboard.h"
#include "fifo.h"

static const char charmap2_lower[] = {
//  0/8  1/9  2/a 3/b  4/c  5/d  6/e  7/f
      0,   0,   0,  0,   0,   0,   0,   0,  // 0 - 07
      0,   0,   0,  0,   0, '\t', '`',  0,  // 8 - F
      0,   0,   0,  0,   0, 'q',  '1', 0,   // 10-17
      0,   0, 'z', 's', 'a', 'w', '2', 0,   // 18-1F
      0, 'c', 'x', 'd', 'e', '4', '3', 0,   // 20 - 27
      0, ' ', 'v', 'f', 't', 'r', '5', 0,   // 28 - 2F
      0, 'n', 'b', 'h', 'g', 'y', '6', 0,   // 30 - 37
      0,  0,  'm', 'j', 'u', '7', '8', 0,   // 38 - 3F
      0, ',', 'k', 'i', 'o', '0', '9', 0,   // 40 - 47
      0, '.', '/', 'l', ';', 'p', '-', 0,   // 48 - 4f
      0,   0, '\'', 0, '[', '=',   0,  0,   // 50 - 57
      0,   0, '\n', ']', 0, '\\', 0,   0,   // 58-5f
      0,   0,   0,  0,   0,   0, 0x08, 0,   // 60 - 67
      0, '1',   0, '4', '7', 0, 0, 0,        // 68-6f
      '0', '.', '2', '5', '6', '8', 0x1b, 0,  // 70 - 77
      0, '+', '3', '-', '*', '9', 0, 0        // 78-7f
};

static const char charmap2_shift[] = {
//  0/8  1/9  2/a 3/b  4/c  5/d  6/e  7/f
      0,   0,   0,  0,   0,   0,   0,   0,  // 0 - 07
      0,   0,   0,  0,   0, '\t', '~',  0,  // 8 - F
      0,   0,   0,  0,   0, 'Q',  '!', 0,   // 10-17
      0,   0, 'Z', 'S', 'A', 'W', '@', 0,   // 18-1F
      0, 'C', 'X', 'D', 'E', '$', '#', 0,   // 20 - 27
      0, ' ', 'V', 'F', 'T', 'R', '%', 0,   // 28 - 2F
      0, 'N', 'B', 'H', 'G', 'Y', '^', 0,   // 30 - 37
      0,  0,  'M', 'J', 'U', '&', '*', 0,   // 38 - 3F
      0, '<', 'K', 'I', 'O', ')', '(', 0,   // 40 - 47
      0, '>', '|', 'L', ':', 'P', '_', 0,   // 48 - 4f
      0,   0, '"', 0, '{', '+',   0,  0,   // 50 - 57
      0,   0, '\n', '}', 0, '|', 0,   0,   // 58-5f
      0,   0,   0,  0,   0,   0, 0x08, 0,   // 60 - 67
      0, '1',   0, '4', '7', 0, 0, 0,        // 68-6f
      '0', '.', '2', '5', '6', '8', 0x1b, 0,  // 70 - 77
      0, '+', '3', '-', '*', '9', 0, 0        // 78-7f
};

static const char charmap2_ctrl[] = {
      0,   0,   0,  0,   0,   0,   0,  0,  // 0 - 07
      0,   0,   0,  0,   0,   0,   0,  0,  // 8 - F
      0,   0,   0,  0,   0,  17,   0,  0,   // 10-17
      0,   0,  26, 19,   1,  21,   0,  0,   // 18-1F
      0,   3,  24,  4,   5,   0,   0,  0,   // 20 - 27
      0,   0,  22,  6,  20,  18,   0,  0,   // 28 - 2F
      0,  14,   2,  8,   7,  25,   0,  0,   // 30 - 37
      0,   0,  13, 10,  21,   0,   0,  0,   // 38 - 3F
      0,   0,  11,  9,  15,   0,   0,  0,   // 40 - 47
      0,   0,   0,  0,   0,  16,   0,  0,   // 48 - 4f
      0,   0,   0,  0,   0,   0,   0,  0,   // 50 - 57
      0,   0, '\r', 0,   0,   0,   0,  0,   // 58-5f
      0,   0,   0,  0,   0,   0,   0,  0,   // 60 - 67
      0,   0,   0,  0,   0,   0,   0,  0,        // 68-6f
      0,   0,   0,  0,   0,   0,   0,  0,  // 70 - 77
      0,   0,   0,  0,   0 ,  0,   0,  0        // 78-7f
};



static char charmap_lookup(keyboard_handler_t *handler, char ch)
{
    if (handler->alt == 1) return 0;
    if (handler->ctrl == 1) return charmap2_ctrl[(int)ch];
    if (handler->shift ^ handler->shift_lock) return charmap2_shift[(int)ch];
    return charmap2_lower[(int)ch];
}

static void keyboard_handle_rcvd_char(keyboard_handler_t *handler, uint8_t val)
{
    if (handler->state == 0) {
        if (val == 0x11) handler->alt = 1;
        else if (val == 0x14) handler->ctrl = 1;
        else if (val == 0x58) handler->shift_lock = 1;
        else if (val == 0x77) handler->num_lock = 1;
        else if ((val == 0x12) || (val == 0x59)) handler->shift = 1;
        else if (val == 0xf0) handler->state = 1;
        else if (val == 0xe0) handler->state = 2;
        else if (val == 0xe1) handler->state = 4;
        else {
            char ch = charmap_lookup(handler, val);
            if (ch != 0)
                fifo_add(handler->fifo, &ch);
        }
    }
    else if (handler->state == 1) {
        if (val == 0x11) handler->alt = 0;
        else if (val == 0x14) handler->ctrl = 0;
        else if (val == 0x58) handler->shift_lock = 0;
        else if (val == 0x77) handler->num_lock = 0;
        else if ((val == 0x12) || (val == 0x59)) handler->shift = 0;

        handler->state = 0;
    }
    // e0 was scanned
    else if (handler->state == 2) {
        if (val == 0xf0)
            handler->state = 3;
        else 
            handler->state = 0;
    }
    // f0 was scanned
    else if (handler->state == 3) { 
        handler->state = 0;
    }
    // e1 was scanned        
    else if (handler->state == 4) {
        if (val == 0x77) handler->state = 0;
        else handler->state = 4;
    }
}


static int keyboard_haschar(keyboard_handler_t *handler)
{
    if (fifo_empty(handler->fifo)) 
        return 0;
    else return 1;
}

static char keyboard_getchar(keyboard_handler_t *handler)
{
    if (fifo_empty(handler->fifo)) return -1;
        
    char ch;
    fifo_get(handler->fifo, &ch);
    return ch;
}

void keyboard_init_handler(keyboard_handler_t *handler)
{
    fifo_init(handler->fifo, 32, 1);
    handler->state = 0;
    handler->shift_lock = 0;
    handler->shift = 0;
    handler->alt = 0;
    handler->ctrl = 0;

    handler->keyboard_send_char = keyboard_handle_rcvd_char;
    handler->keyboard_getchar = keyboard_getchar;
    handler->keyboard_haschar = keyboard_haschar;

}

