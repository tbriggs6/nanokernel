#ifndef _FIFO_H
#define _FIFO_H

#include "kstdlib.h"

// create an opaque pointer to the fifo structure
struct fifo;
typedef struct fifo fifo_t;

fifo_t *fifo_create( );
int fifo_sizeof( );
void fifo_init(fifo_t *fifo, size_t max_entries, size_t entry_size);
void fifo_add(fifo_t *fifo, void *entry);
void fifo_get(fifo_t *fifo, void *entry);
int fifo_avail(fifo_t *fifo);
int fifo_size(fifo_t *fifo);
int fifo_full(fifo_t *fifo);
int fifo_empty(fifo_t *fifo);

#endif
