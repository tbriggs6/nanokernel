#ifndef _KMALLOC_H
#define _KMALLOC_H


void kmalloc_init( );
void *kmalloc(size_t size);
void kfree(void *ptr);
void kmalloc_debug_walk( );


#endif
