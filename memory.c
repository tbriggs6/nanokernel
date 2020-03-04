#include <stdint.h>
#include "multiboot.h"
#include "memory.h"
#include "kstdlib.h"

static uint32_t *memory_free_bitmap = NULL;
static uint32_t memory_num_pages = 0;
static uint32_t memory_num_free_pages = 0;
static uint32_t memory_bitmap_entries = 0;

extern uint32_t *_heap_size;
extern uint32_t  *_heap_start;
extern uint32_t  *_heap_end;

#define PAGE_SIZE (4096)
#define MEM_START (0x100000)

static uint32_t multiboot_size_bytes(multiboot_info_t *multiboot_ptr)
{
    uint32_t i;
    unsigned long max_addr = 0;
    multiboot_memory_map_t *entries = (multiboot_memory_map_t *) multiboot_ptr->mmap_addr;
    unsigned int num_entries = multiboot_ptr->mmap_length / sizeof(multiboot_memory_map_t);

    for (i = 0; i < num_entries; i++) {    
        if (entries[i].type != MULTIBOOT_MEMORY_AVAILABLE)
             continue;

        uint32_t entry_addr = entries[i].addr + entries[i].len;
        if (entry_addr > max_addr) 
            max_addr = entry_addr;
    }
    return max_addr;
}

static int memory_is_alloc(unsigned int n)
{
    uint32_t word = n / 32;
    uint32_t bit = n % 32;

    uint32_t val = memory_free_bitmap[word];
    if (((val >> bit) & 1) == 0) return 1;
    else return 0;
}


static void memory_page_alloc(unsigned int n)
{
    uint32_t word = n / 32;
    uint32_t bit = n % 32;

    uint32_t val = memory_free_bitmap[word];
    val = val & ~(1 << bit);
    memory_free_bitmap[word] = val;
    memory_num_free_pages--;
}

static void memory_page_free(unsigned int n)
{
    uint32_t word = n / 32;
    uint32_t bit = n % 32;

    uint32_t val = memory_free_bitmap[word];
    val = val | (1 << bit);
    memory_free_bitmap[word] = val;
    memory_num_free_pages++;
}

uint32_t memory_find_free_page( )
{
    uint32_t i,j;

    for (i = 0; i < memory_num_pages; i++) {
        if (memory_free_bitmap[i] != 0)
        break;
    }

    if (i == memory_num_pages)  {
        kprintf("ERROR - no free memory remains\n");
        panic( );
    }

    for (j = 0; j < 32; j++) {
        if ((memory_free_bitmap[j] >> j) & 0x01) 
            break;
    }

    return (i * 32) + j;
}

static void memory_build_data_structs(multiboot_info_t *multiboot_ptr)
{
    // get the number of bytes (e.g. 2GB)
    uint32_t bytes = multiboot_size_bytes(multiboot_ptr);

    // get the number of pages (e.g. 2GB / 4KB = 524,288 pages)
    memory_num_pages = bytes / PAGE_SIZE;
    
    // each page is a bit, so get the number of 32-bit words to hold the pages
    // each entry holds 32-bits (e.g. 16384 integers)
    memory_bitmap_entries = memory_num_pages / 32;
    if ((memory_bitmap_entries * 32) != memory_num_pages)
        memory_bitmap_entries++;

    // kmalloc this chunk from the heap (e.g. 16384 words = 65,536 bytes)
    uint32_t num_bitmap_bytes = memory_bitmap_entries * sizeof(uint32_t);
    memory_free_bitmap = (uint32_t *) kmalloc(num_bitmap_bytes);

    // start each segment as not-available
    kmemset(memory_free_bitmap, 0, num_bitmap_bytes);
}

static void memory_build_map_from_multiboot(multiboot_info_t *multiboot_ptr)
{
    // walk the multiboot table for memory, and find usable "high" memory
    uint64_t i,j;
    uint32_t c;

    memory_num_free_pages = 0;

    multiboot_memory_map_t *entries = (multiboot_memory_map_t *) multiboot_ptr->mmap_addr;
    unsigned int num_entries = multiboot_ptr->mmap_length / sizeof(multiboot_memory_map_t);

    for (i = 0; i < num_entries; i++) {
        if (entries[i].type != MULTIBOOT_MEMORY_AVAILABLE) {
            continue;
        }

        for (j = 0; j < entries[i].len ; j+= PAGE_SIZE)  {
            uint32_t address = (uint32_t) (entries[i].addr + +j);
            if (address < MEM_START) 
                continue;

            uint32_t page_num = address/PAGE_SIZE;
            memory_page_free(page_num);
        }

    }
}

static void memory_mark_kernel_as_used( )
{
    uint32_t i;

    for (i = 0; i < &_heap_end; i+= 4096)
        if (i >= MEM_START) {
            uint32_t page_num = i / PAGE_SIZE;
            if (!memory_is_alloc(page_num)) 
                memory_page_alloc(page_num);
        }
}


/**
 * Initialize the memory management system, including paging
 */
void memory_init( multiboot_info_t *multiboot_ptr)
{
    memory_build_data_structs(multiboot_ptr);
    memory_build_map_from_multiboot(multiboot_ptr);
    memory_mark_kernel_as_used( );

    kprintf("*************************\n");
    kprintf("Memory: %u total pages, %u pages free\n", memory_num_pages, memory_num_free_pages);
    kprintf("*************************\n");
}



uint32_t memory_alloc_page( )
{
    uint32_t page = memory_find_free_page( );
    if (page == (uint32_t) -1) {
        kprintf("ERROR - no free page remains\n");
        panic();
    }

    if (memory_is_alloc(page)) {
        kprintf("ERROR - found a page marked free but it was also alloc?\n");
        panic();
    }

    memory_page_alloc(page);

    return page;   
}

void memory_free_page(uint32_t page)
{
    if (!memory_is_alloc(page)) {
        kprintf("ERROR - free page %lu, page is not alloc'd()\n", page);
        panic();
    }

    memory_page_free(page);
}