#ifndef _MEMORY_H
#define _MEMORY_H

#include "multiboot.h"

void memory_init(multiboot_info_t *multiboot_ptr);
uint32_t memory_find_free_page( );
void memory_free_page(uint32_t page);

#endif