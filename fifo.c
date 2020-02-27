#include <stdint.h>

#include "fifo.h"
#include "kstdlib.h"


struct fifo {
  int read_pos;       // where to read from
  int write_pos;      // where to write to
  int num_entries;    // how many items currently in FIFO
  int max_entries;    // how many items can be in FIFO
  int entry_size;     // how big is each item
  uint8_t *entries;      // pointer to (max_entries * entry_size) bytes
};


fifo_t *fifo_create( )
{
  return kmalloc(sizeof(struct fifo));
}

void fifo_init(fifo_t *fifo, size_t max_entries, size_t entry_size)
{

  fifo->read_pos = 0;
  fifo->write_pos = 0;
  fifo->num_entries = 0;
  fifo->max_entries = max_entries;
  fifo->entry_size = entry_size;
  fifo->entries = (uint8_t *) kmalloc( max_entries * entry_size);
  if (fifo->entries == NULL) {
    kprintf("Error - cannot allocate %d bytes\n", (max_entries * entry_size));
  }
}

void fifo_add(fifo_t *fifo, void *entry)
{

  if (fifo->num_entries < fifo->max_entries) {
    int offset = fifo->write_pos * fifo->entry_size;
    kmemcpy(fifo->entries+offset, entry, fifo->entry_size);
    fifo->num_entries++;
    fifo->write_pos = (fifo->write_pos + 1) % fifo->max_entries;
  }
  else {
    kprintf("Error - could not add to FIFO, out of space\n");
  }
}

void fifo_get(fifo_t *fifo, void *entry)
{
  if (fifo->num_entries > 0) {
    int offset = fifo->read_pos * fifo->entry_size;
    kmemcpy(entry, fifo->entries + offset, fifo->entry_size);
    fifo->num_entries--;
    fifo->read_pos = (fifo->read_pos + 1)  % fifo->max_entries;
    
  }
  else {
    kprintf("Error - could not get from the FIFO, no entries!\n");
  }
}

int fifo_avail(fifo_t *fifo)
{
  return fifo->num_entries;
}

int fifo_size(fifo_t *fifo)
{
  return fifo->max_entries;
}

int fifo_full(fifo_t *fifo)
{
  return fifo->num_entries == fifo->max_entries;
}

int fifo_empty(fifo_t *fifo)
{
  return fifo->num_entries == 0;
}

int fifo_sizeof( )
{
  return sizeof(fifo_t);
}
