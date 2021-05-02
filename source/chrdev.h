#ifndef _CHRDEV_C
#define _CHRDEV_C

#include <stdint.h>

typedef struct {
  char driver_name[32];
  uint8_t (*getch)( );
  int (*putch)(uint8_t ch);
  int (*isempty)();
  int (*isfull)();

  int rxcount;
  int txcount;
} chrdev_t;

void chrdev_init( );
void chrdev_register(chrdev_t *chrdev);
char chrdev_getch(chrdev_t *dev);
int chrdev_putch(chrdev_t *dev, uint8_t ch);
chrdev_t *chrdev_forname(const char *name);

#endif
