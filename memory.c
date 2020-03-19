#include <stdint.h>
#include "bitmap.h"
#include "multiboot.h"
#include "memory.h"
#include "kstdlib.h"
#include "i386.h"

extern uint32_t  _kernel_size;
extern uint32_t  _kernel_start;
extern uint32_t  _kernel_end;

page_directory_t *kpage_dir = NULL;
static bitmap_t *mem_free = NULL;
static uint32_t memory_total_pages = 0;
static uint32_t memory_alloc_pages = 0;

#define PAGE_SIZE (4096)

static gdt_entry_t *kernel_gdt = NULL;
static gdt_descriptor_t kernel_gdt_descriptor = {0};

gdt_entry_t make_gdt_entry(uint32_t offset, uint32_t limit, const gdt_type_t type)
{
    gdt_entry_t entry;
    entry.entry_value = 0;

    kprintf("make_gdt_entry(%x, %x, %x/%x\n", (unsigned) offset, 
        (unsigned) limit, (unsigned) type.access_value, 
        (unsigned) type.flag_value);

    entry.limit_low = limit;
    entry.base_low = offset;
    entry.base_middle = offset >> 16;
    entry.access = type.access_value;
    entry.limit_high = limit >> 16;
    entry.flags = type.flag_value;
    entry.base_high = offset >> 24;
    return entry;
}

void gdt_set_value(unsigned segment_index, uint64_t value)
{
    if (segment_index == 0) {
        kprintf("gdt_set_entry() - cannot set 0th entry in GDT\n");
        return;
    }    
    
    if (kernel_gdt == NULL) {
        kernel_gdt = (gdt_entry_t *) memory_find_and_alloc_page( );
    }

    kmemcpy(&kernel_gdt[segment_index], &value, sizeof(gdt_entry_t));
}

void gdt_set_entry(unsigned segment_index, const gdt_entry_t const *entry)
{
    if (segment_index == 0) {
        kprintf("gdt_set_entry() - cannot set 0th entry in GDT\n");
        return;
    }    
    
    if (kernel_gdt == NULL) {
        kernel_gdt = (gdt_entry_t *) memory_find_and_alloc_page( );
    }

    kmemcpy(&kernel_gdt[segment_index], entry, sizeof(gdt_entry_t));
}


static void kernel_gdt_init(void)
{
    gdt_entry_t entry;

    kernel_gdt = (gdt_entry_t *) memory_find_and_alloc_page( );
    kmemset(kernel_gdt, 0, 4096);

    entry = make_gdt_entry(0, 0xffffff, gdt_kernel_code);
    gdt_set_entry(1, &entry);
    
    entry = make_gdt_entry(0, 0xffffff, gdt_kernel_data);
    gdt_set_entry(2, &entry);

    entry = make_gdt_entry(0, 0xffffff, gdt_user_code);
    gdt_set_entry(3, &entry);
    
    entry = make_gdt_entry(0, 0xffffff, gdt_user_data);
    gdt_set_entry(4, &entry);

}

void gdt_make_active( )
{
    if (kernel_gdt == NULL) {
        kprintf("ERROR - gdt has not been allocated.");
        panic();
    }

    int i = 0, max_idx = 0;
    for (i = 0; i < 512; i++) {
        if (kernel_gdt[i].entry_value != 0) {
            kprintf("GDT[%d] = %Lx\n", i, kernel_gdt[i].entry_value);
            max_idx = i;
        }
    }

    if (max_idx == 0) {
        kprintf("ERROR - no entries in GDT table!??\n");
        panic();
    }

    kernel_gdt_descriptor.size = ((max_idx+1) * 8) -1;
    kernel_gdt_descriptor.offset = (uint32_t) kernel_gdt;
  
    lgdt(&kernel_gdt_descriptor);
}


/**
 * multiboot_size_bytes - walk the multiboot memory table and map the available
 * memory.  
 * @param multiboot_ptr - pointer to the multiboot information.
 * @returns  number of free bytes
 * */
static uint32_t multiboot_size_bytes(const multiboot_info_t const *multiboot_ptr)
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

/**
 * Determine the size of memory, and create the memory bitmaps from the 
 * multiboot information.
 * */
static void memory_build_data_structs(multiboot_info_t *multiboot_ptr)
{
    // get the number  of bytes (e.g. 2GB)
    uint32_t bytes = multiboot_size_bytes(multiboot_ptr);

    // get the number of pages (e.g. 2GB / 4KB = 524,288 pages)
    memory_total_pages = bytes / PAGE_SIZE;
    memory_alloc_pages = memory_total_pages;

    bitmap_init(mem_free, memory_total_pages);
    bitmap_set_all(mem_free);
    
}

/**
 * Walk the multiboot structures and mark pages as free if they correspond to
 * usable memory.
 * */
static void memory_build_map_from_multiboot(multiboot_info_t *multiboot_ptr)
{
    // walk the multiboot table for memory, and find usable "high" memory
    uint64_t i,j;

    multiboot_memory_map_t *entries = (multiboot_memory_map_t *) multiboot_ptr->mmap_addr;
    unsigned int num_entries = multiboot_ptr->mmap_length / sizeof(multiboot_memory_map_t);

    for (i = 0; i < num_entries; i++) {
        if (entries[i].type != MULTIBOOT_MEMORY_AVAILABLE) {
            continue;
        }

        for (j = 0; j < entries[i].len ; j+= PAGE_SIZE)  {
            uint32_t address = (uint32_t) (entries[i].addr +j);
            uint32_t page = address >> 12;
            bitmap_clr(mem_free, page);
            memory_alloc_pages--;
        }

    }
}

/**
 * mark the memory used by the kernel as used.
 **/
static void memory_mark_kernel_as_used( )
{
    uint32_t addr;

    uint32_t start = (uint32_t) &_kernel_start;
    uint32_t end = (uint32_t) &_kernel_end;

    // reserve the first 64KB of low memory
    for (addr = 0; addr < 64 * 1024; addr += 4096) {
        uint32_t page = addr >> 12;
        bitmap_set(mem_free, page);
        memory_alloc_pages++;
    }


    for (addr = start; addr <= end; addr+= 4096) {
        uint32_t page = addr >> 12;
        bitmap_set(mem_free, page);
        memory_alloc_pages++;
    }
}

uint32_t memory_find_and_alloc_page( )
{
    uint32_t page_num = bitmap_first_clear(mem_free);
    bitmap_set(mem_free, page_num);
    memory_alloc_pages++;

    uint32_t addr = page_num << 12;
    return addr;
}

// static void memory_free_page(uint32_t addr)
// {
//     uint32_t page_num = addr >> 12;
//     bitmap_clr(mem_free, page_num);
//     memory_alloc_pages--;
// }


/**
 * +----------------------+----------------------+------------------+
 * | 31 ..            22  | 21  ...         12   |11   ...         0|
 * +----------------------+----------------------+------------------+
 * | dir. entry (10 bits) | page table (10 bits) | 4KB page (12 bis)|
 * +----------------------+----------------------+------------------+
 */
unsigned int page_directory_for_virtual(uint32_t virtual)
{
    return (virtual >> 22) & 0x3ff;  // most-significant 10 bits
}

unsigned int page_table_for_virtual(uint32_t virtual)
{
    return (virtual >> 12) & 0x3ff;  // middle-10 bits
}


static void page_turn_on( )
{
    kprintf("Skipping paging for now\n");
    
    // lcr3((uint32_t ) kpage_dir);
    
    // uint32_t old = rcr0();
    // uint32_t new = old | 0x80000001;
    
    // lcr0(new);
}


void page_map(page_directory_t *dir, uint32_t virtual_addr, uint32_t physical_addr)
{
    if (dir == NULL) {
        kprintf("Error - page directory is null\n");
        panic();
    }

    unsigned int idx = page_directory_for_virtual(virtual_addr);
    if (dir->entry[idx].present == 0) {
        dir->entry[idx].present = 1;
        uint32_t table_addr = memory_find_and_alloc_page();
        dir->entry[idx].page_addr = table_addr >> 12;
    }

    uint32_t addr = (uint32_t)(dir->entry[idx].page_addr << 12);
    page_table_t *table = (page_table_t *) addr;
    
    idx = page_table_for_virtual(virtual_addr);
    table->entry[idx].present = 1;
    table->entry[idx].page_addr = physical_addr >> 12;
 }

/** 
 * Create the kernel's page directory, mapping 
 * all physical memory to virtual memory
 */
static void page_setup_kernel( )
{
    uint32_t addr = memory_find_and_alloc_page( );
    kpage_dir = (page_directory_t *) addr;
    kmemset(kpage_dir, 0, 4096); // zero out all entries

    uint32_t num_pages = memory_total_pages;
    uint32_t i;

    kprintf("Enabling kernel acces to pages 1 through %d\n", (int) num_pages);
    for (i = 0; i < num_pages; i++) 
    {
        // map all of system memory idempotentally
        page_map(kpage_dir, i << 12, i << 12);
    }
}

uint32_t page_physical_for_virtual(page_directory_t *dir, uint32_t virtual)
{
    uint32_t idx = page_directory_for_virtual(virtual);
    if (dir->entry[idx].present == 0) {
        kprintf("Error - physical for virtual, table not found.");
        panic();
    }
    uint32_t table_addr = dir->entry[idx].page_addr << 12;
    page_table_t *table = (page_table_t *) table_addr;
    idx = page_table_for_virtual(virtual);
    if (table->entry[idx].present == 0) {
        kprintf("Error - physical for virtual, page not found.");
        panic();
    }
    uint32_t physical = table->entry[idx].page_addr << 12;
    return physical;
}

// this is really non-optimal, but it works.  
//TODO: refactor the hell out of this
void page_copy(page_directory_t *src_dir, uint32_t src_virtual, page_directory_t *dst_dir, uint32_t dst_virtual, uint32_t len)
{
    uint32_t curr = 0;

    uint32_t src_offset = src_virtual % 4096;
    uint32_t dst_offset = dst_virtual % 4096;

    while (curr < len)
    {
        uint32_t src_phys = page_physical_for_virtual(src_dir, src_virtual + src_offset + curr);
        uint32_t dst_phys = page_physical_for_virtual(dst_dir, dst_virtual + dst_offset + curr);

        uint8_t *src_addr = (uint8_t *) src_phys;
        uint8_t *dst_addr = (uint8_t *) dst_phys;

        *dst_addr = *src_addr;
        curr++;
    }
}

void copy_to_user(uint32_t kernel_virtual, page_directory_t *dst_dir, uint32_t dst_virtual, uint32_t len)
{
    page_copy(kpage_dir, kernel_virtual, dst_dir, dst_virtual, len);
}

void copy_from_user(page_directory_t *src_dir, uint32_t src_virtual, uint32_t kernel_virtual, uint32_t len)
{
    page_copy(src_dir, src_virtual, kpage_dir, kernel_virtual, len);
}


/**
 * Initialize the page-directory for the kernel itself
 */
void page_init( )
{
    // find a 4KB page for the page-directory
    page_setup_kernel();
    page_turn_on();
}

/**
 * Initialize the memory management system, including paging
 */
void memory_init( multiboot_info_t *multiboot_ptr)
{
    memory_build_data_structs(multiboot_ptr);
    memory_build_map_from_multiboot(multiboot_ptr);
    memory_mark_kernel_as_used( );

    kernel_gdt_init( );

    kprintf("*************************\n");
    kprintf("Memory: %u total pages, %u pages free\n", (unsigned int) memory_total_pages, (unsigned int) (memory_total_pages - memory_alloc_pages));
    kprintf("*************************\n");
}



const gdt_type_t gdt_kernel_code = {
    .access = { .acc = 0, .rw = 1, .dc = 0,
                .ex = 1, .s = 1, .priv = 0, 
                .pr = 1 },
    .flags = { .sz = 1, .gr = 1 }
};

const gdt_type_t gdt_kernel_data = {
    .access = { .acc = 0, .rw = 1, .dc = 0,
                .ex = 0, .s = 1, .priv = 0, 
                .pr = 1 },
    .flags = { .sz = 1, .gr = 1 }
};


const gdt_type_t gdt_kernel_tss = {
    .tss = { .type = 0x09, .zero = 0, .priv = 0, .pr = 1},
    .flag_value = 0x08
};

const gdt_type_t ldt_kernel_type = { 
    .tss = { .type = 0x02, .zero = 0, .priv = 0, .pr = 1},
    .flag_value = 0x08
};

const gdt_type_t ldt_user_type = { 
    .tss = { .type = 0x02, .zero = 0, .priv = 3, .pr = 1},
    .flag_value = 0x08
};


const gdt_type_t gdt_user_code = {
    .access = { .acc = 0, .rw = 1, .dc = 0,
                .ex = 1, .s = 1, .priv = 3, 
                .pr = 1 },
    .flags = { .sz = 1, .gr = 1 }
};

const gdt_type_t gdt_user_data = {
    .access = { .acc = 0, .rw = 1, .dc = 0,
                .ex = 0, .s = 1, .priv = 3, 
                .pr = 1 },
    .flags = { .sz = 1, .gr = 1 }
};

const gdt_type_t ldt_user_code = {
    .access = { .acc = 0, .rw = 1, .dc = 0,
                .ex = 1, .s = 1, .priv = 3, 
                .pr = 1 },
    .flags = { .sz = 1, .gr = 1 }
};

const gdt_type_t ldt_user_data = {
    .access = { .acc = 0, .rw = 1, .dc = 0,
                .ex = 0, .s = 1, .priv = 3, 
                .pr = 1 },
    .flags = { .sz = 1, .gr = 1 }
};

