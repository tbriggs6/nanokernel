#ifndef _ELF_H
#define _ELF_H

#include <stdint.h>

int read_elf(const char *elf_start, void *task_data, 
    void *(*virt_alloc)(uint32_t virt_start, uint32_t length, void *alloc_data),
    void (*virt_copy)(uint32_t dest, uint32_t src, uint32_t len, void *copy_data));

#endif