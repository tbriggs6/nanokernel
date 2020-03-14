#ifndef _MEMORY_H
#define _MEMORY_H

#include "multiboot.h"

typedef union {
    uint32_t value;
    struct  {
        unsigned present:1;
        unsigned read_write:1;
        unsigned user_super:1;
        unsigned write_through:1;
        unsigned cache_disabled:1;
        unsigned accessed:1;
        unsigned :1;
        unsigned page_size:1;
        unsigned :4;
        unsigned page_addr:20;
    } ;
} page_dirent_t;

typedef union {
    uint32_t value;
    struct {
        unsigned present:1;
        unsigned read_write:1;
        unsigned user_super:1;
        unsigned write_through:1;
        unsigned cache_disabled:1;
        unsigned accessed:1;
        unsigned dirty:1;
        unsigned :1;
        unsigned global:1;
        unsigned :3;
        unsigned page_addr:20;
    };
} page_entry_t;

typedef struct {
    page_dirent_t entry[1024];
} page_directory_t;

typedef struct {
    page_entry_t entry[1024];
} page_table_t;



void memory_init(multiboot_info_t *multiboot_ptr);
uint32_t memory_find_free_page( );
void memory_free_page(uint32_t page);
void page_init( );
void page_copy_to_user(void *kernel_mem, page_directory_t *dest, uint32_t virtual, uint32_t bytes);
void page_copy_process_memory(page_directory_t *src, page_directory_t *dest);
void page_create_memory(page_directory_t *dir, uint32_t virtual, uint32_t size);
uint32_t page_kernel_alloc_page( );

#endif