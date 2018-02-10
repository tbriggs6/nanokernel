#include "kstdlib.h"
#include "chrdev.h"
#include "list.h"


list_t drivers;

void chrdev_init( )
{
  list_init(&drivers);
}


void chrdev_register(chrdev_t *chrdev)
{
  list_append(&drivers, chrdev);
}


int chrdev_namematch(const void *vname, const void *vdev)
{
  const char *name = (const char *) vname;
  const chrdev_t *dev = (const chrdev_t *) vdev;
  
  if (! kstrcmp(name, dev->driver_name)) return 1;
  else return 0;
}

chrdev_t *chrdev_forname(const char *name)
{
  return list_search( (void *) &drivers, (void *) name, chrdev_namematch);
}



char chrdev_getch(chrdev_t *dev)
{
  // wait for a character -- this is totally bad
  while(dev->isempty() == 1) ;

  dev->rxcount++;

  return dev->getch( );
}

int chrdev_putch(chrdev_t *dev, uint8_t ch)
{
  // wait for space -- this is totally bad
  while (dev->isfull() == 1) ;

  int rc = dev->putch(ch);
  dev->txcount+= rc;

  return rc;
}

