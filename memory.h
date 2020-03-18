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


typedef struct {
    union {
        uint8_t access_value;
        struct {
            unsigned acc:1;
            unsigned rw:1;
            unsigned dc:1;
            unsigned ex:1;
            unsigned s:1;
            unsigned priv:2;
            unsigned pr:1;
        } access;
        struct {
            unsigned type:4;
            unsigned zero:1;
            unsigned priv:2;
            unsigned pr:1;
        } tss;
    };
    union {
        uint8_t flag_value;
        struct {
            unsigned avl:1;
            unsigned :1;
            unsigned sz:1;
            unsigned gr:1;
            unsigned :4;
        } flags;
    };
} gdt_type_t;



typedef union {
    uint64_t entry_value;
    struct {
        uint16_t limit_low;
        uint16_t base_low;
        uint8_t base_middle;
        uint8_t access;
        uint8_t limit_high:4;
        uint8_t flags:4;
        uint8_t base_high;
    };
} gdt_entry_t;


#define GDT_KERN_CODE (0x08)
#define GDT_KERN_DATA (0x10)
#define GDT_USER_CODE (0x18)
#define GDT_USER_DATA (0x20)
#define GDT_TSS (0x28)

extern const gdt_type_t gdt_kernel_code, gdt_kernel_data,
    gdt_kernel_tss, gdt_user_code, gdt_user_data;

extern const gdt_type_t ldt_user_code, ldt_user_data,
    ldt_kernel_type, ldt_user_type;
    
void memory_init(multiboot_info_t *multiboot_ptr);
uint32_t memory_find_and_alloc_page( );

void page_init( );
void page_copy(page_directory_t *src_dir, uint32_t src_virtual, page_directory_t *dst_dir, uint32_t dst_virtual, uint32_t len);
void copy_to_user(uint32_t kernel_virtual, page_directory_t *dst_dir, uint32_t dst_virtual, uint32_t len);
void copy_from_user(page_directory_t *src_dir, uint32_t src_virtual, uint32_t kernel_virtual, uint32_t len);

uint32_t page_physical_for_virtual(page_directory_t *dir, uint32_t virtual);
void page_map(page_directory_t *dir, uint32_t virtual_addr, uint32_t physical_addr);

gdt_entry_t make_gdt_entry(uint32_t offset, uint32_t limit, const gdt_type_t type);
void gdt_make_active( );
void gdt_set_value(unsigned segment_index, uint64_t value);
void gdt_set_entry(unsigned segment_index, const gdt_entry_t const *entry);

#endif