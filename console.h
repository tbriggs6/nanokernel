#ifndef _CONSOLE_H
#define _CONSOLE_H

#include <stdint.h>

#define COLOR_BLACK 0
#define COLOR_BLUE 1
#define COLOR_GREEN 2
#define COLOR_CYAN 3
#define COLOR_RED 4
#define COLOR_MAGENTA 5
#define COLOR_BROWN
#define COLOR_LIGHT_GREY 7
#define COLOR_DARK_GREY 8
#define COLOR_LIGHT_BLUE 9
#define COLOR_LIGHT_GREEN 10
#define COLOR_LIGHT_CYAN 11
#define COLOR_LIGHT_RED 12
#define COLOR_LIGHT_MAGENTA 13
#define COLOR_LIGHT_BROWN 14
#define COLOR_WHITE 15

#define CONSOLE_TAB_SPACES 4

typedef uint8_t color_t;

void console_init(color_t color);
void console_clear(void);
void console_scroll(int rows);
int console_putch(uint8_t ch);
void console_puts(const char *str);
void console_set_pos(uint8_t r, uint8_t c);
void console_set_fgcolor(color_t fgcolor);
void console_set_bgcolor(color_t bgcolor);
#endif
