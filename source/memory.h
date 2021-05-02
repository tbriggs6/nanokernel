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

typedef struct __attribute__((__packed__)) {
    uint16_t size;
    uint32_t offset;
} gdt_descriptor_t;


    
void memory_init(multiboot_info_t *multiboot_ptr);
uint32_t memory_find_and_alloc_page( );

void page_init( );
void page_copy(page_directory_t *src_dir, uint32_t src_virtual, page_directory_t *dst_dir, uint32_t dst_virtual, uint32_t len);
void copy_to_user(uint32_t kernel_virtual, page_directory_t *dst_dir, uint32_t dst_virtual, uint32_t len);
void copy_from_user(page_directory_t *src_dir, uint32_t src_virtual, uint32_t kernel_virtual, uint32_t len);

uint32_t page_physical_for_virtual(page_directory_t *dir, uint32_t virtual);
void page_map(page_directory_t *dir, uint32_t virtual_addr, uint32_t physical_addr);

#endif