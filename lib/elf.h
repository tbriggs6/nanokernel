#ifndef _ELF_H
#define _ELF_H

#include <stdint.h>

int read_elf(const char *elf_start, void *(*virt_alloc)(uint32_t virt_start, uint32_t length));

#endif