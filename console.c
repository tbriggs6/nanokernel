#include "console.h"

static uint8_t curr_page = 0;
static uint8_t row = 0, col = 0;
static uint8_t width = 80, height = 25;

static color_t fgcolor = COLOR_WHITE;
static color_t bgcolor = COLOR_BLACK;

#define CGA_BASE 0xb8000
#define CGA_PORT 0x3d4

static volatile uint16_t * console_get_page( int page )
{
  return (volatile uint16_t *) (CGA_BASE + (page * width * height * 2));
}


static void cga_output(uint8_t value, uint16_t port)
{
  __asm __volatile ("outb %0, %w1" : : "a" (value), "d" (port) );
}

static void console_cga_set_cursor(uint8_t r, uint8_t c)
{
  cga_output(14, CGA_PORT);
  cga_output(r, CGA_PORT+1);
  cga_output(15, CGA_PORT);
  cga_output(c, CGA_PORT+1);
}
      


void console_puts(const char *str)
{
  int i = 0;
  while (str[i] != 0x00) {
    console_putch(str[i]);
    i++;
  }
}

void console_clear( )
{
  volatile uint16_t *framebuffer = console_get_page(0);
  int i;
  
  for (i = 0; i < (width * height); i++) 
    framebuffer[i] = (bgcolor << 4 | fgcolor) << 8;

}

void console_set_fgcolor(color_t color)
{
  fgcolor = color;
}

void console_set_bgcolor(color_t color)
{
  bgcolor = color;
}


// initialize the console to a clear screen with the bg color
void console_init(color_t color)
{
  bgcolor = color;

  console_clear( );
  
  row = 0;
  col = 0;
  width = 80;
  height = 25;
  
  curr_page = 0;
}


void console_scroll(int rows)
{
  int r, c;
  volatile uint16_t *framebuffer = console_get_page(curr_page);
  
  for (r = rows; r < height; r++) 
    for (c = 0; c < width; c++) 
      framebuffer[(r-rows) * width + c]  = framebuffer[r * width + c];

  // clear that number of rows
  for (r = 0; r < rows; r++) 
    for (c = 0; c < width; c++)
      framebuffer[(height-rows-r) * width + c] = (bgcolor << 4 | fgcolor) << 8;
}

static void console_clear_line(int row_num)
{
  int c;

  volatile uint16_t *framebuffer = console_get_page(curr_page);
  
  for (c = 0; c < width; c++) {
    framebuffer[row_num * width + col] = (bgcolor << 4 | fgcolor) << 8 ;
  }
    
}

void console_putch(char ch)
{
  volatile uint16_t *framebuffer = console_get_page(curr_page);
  int i;

  if (ch == '\t') {
    i = 0;
    while ((col < width) && (i < CONSOLE_TAB_SPACES)) {
      framebuffer[row * width + col] = (bgcolor << 4 | fgcolor) << 8 | ' ';
      col++;
      i++;
    }
  }
  else if ((ch == '\r') || (ch == '\n')) {
    row = row + 1;
    if (row >= height)  {
      console_scroll(1);
      row = 25;
    }
    if (ch == '\n') {
      col = 0;
      console_clear_line(row);
    }
  }
  else if (ch == '\b') {
    if (col > 0) {
      framebuffer[row * width + col] = (bgcolor << 4 | fgcolor) << 8 | ' ';
      col--;
    }
  }
  else if (ch == '\f') {
    console_clear( );
  }
  else if (ch == '\v') {
    row = height-1;
    console_clear_line(row);
  }
  else {
    framebuffer[row * width + col] = (bgcolor << 4 | fgcolor) << 8 | ch;
    col = col + 1;
    if (col >= width) {
      col = 0;
      
      row = row + 1;
      if (row >= height) {
	console_scroll(1);
	row = 24;
      }
    }
  }

  console_set_pos(row, col);
}

void console_set_pos(uint8_t r, uint8_t c)
{
  row = r;
  col = c;

  console_cga_set_cursor(r,c);
}

