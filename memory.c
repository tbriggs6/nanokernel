#include <stdint.h>
#include "multiboot.h"
#include "memory.h"
#include "kstdlib.h"
#include "i386.h"

extern uint32_t  _kernel_size;
extern uint32_t  _kernel_start;
extern uint32_t  _kernel_end;
extern page_directory_t  *_kernel_page_directory;

static uint32_t *memory_alloc_bitmap = NULL;
static uint32_t memory_num_pages = 0;
static uint32_t memory_num_free_pages = 0;
static uint32_t memory_bitmap_entries = 0;

static page_directory_t *kpage_dir = NULL;

#define PAGE_SIZE (4096)


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
 * memory_is_alloc - Determine if this memory address is allocated or not.
 * @param n the physical address.
 * @return 1 if the page is alloc, 0 otherwise.
 * **/
static int memory_is_alloc(unsigned int physical_addr)
{
    // first compute the page number for the address
    uint32_t page = physical_addr / PAGE_SIZE;
    // compute which 32-bit word contains the page
    uint32_t word = page / 32;
    // compute the bit in the bitmap that represents the page
    uint32_t bit = page % 32;

    // access the memory free bitmap, and check its value.
    uint32_t val = memory_alloc_bitmap[word];
    if (((val >> bit) & 1) == 1) return 1;
    else return 0;
}


/**
 * memory_page_alloc - marks a page of memory as being allocated.
 * @param physical_addr - the physical address to mark
 **/
static void memory_page_alloc(unsigned int physical_addr)
{
    uint32_t word = physical_addr / 32;
    uint32_t bit = physical_addr % 32;

    uint32_t val = memory_alloc_bitmap[word];
    val = val | (1 << bit);
    memory_alloc_bitmap[word] = val;
    memory_num_free_pages--;
}

/**
 * memory_page_free - mark a memory page free in the physical bitmap
 * @param physical_addr - the address
 */
static void memory_page_free(unsigned int physical_addr)
{
    uint32_t word = physical_addr / 32;
    uint32_t bit = physical_addr % 32;

    uint32_t val = memory_alloc_bitmap[word];
    val = val & ~(1 << bit);
    memory_alloc_bitmap[word] = val;
    memory_num_free_pages++;
}

/**
 * memory_find_free_page - find a free page in memory
 */
uint32_t memory_find_free_page( )
{
    uint32_t i,j;

    for (i = 0; i < memory_num_pages; i++) {
        // if all pages are used, then we get 32-bits set....
        if (memory_alloc_bitmap[i] != 0xffffffff)
        break;
    }
    
    if (i == memory_num_pages)  {
        kprintf("ERROR - no free memory remains\n");
        panic( );
    }

    // we found a page-alloc thats not used
    // at this point, a page, between (i*32)+0 and (i*32)+31 is free
    for (j = 0; j < 32; j++) {
        if (!memory_is_alloc((i*32)+j))
            break;
    }

    uint32_t page_num = (i * 32) + j;
    uint32_t address = page_num * PAGE_SIZE;

    kprintf("Returning %x as address of free page %x\n", address, page_num);
    return address;
}

/**
 * Determine the size of memory, and create the memory bitmaps from the 
 * multiboot information.
 * */
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
    memory_alloc_bitmap = (uint32_t *) kmalloc(num_bitmap_bytes);

    // start each segment as allocated - we'll mark them as free if they 
    // are usuable later on.
    kmemset(memory_alloc_bitmap, 0xff, num_bitmap_bytes);
}

/**
 * Walk the multiboot structures and mark pages as free if they correspond to
 * usable memory.
 * */
static void memory_build_map_from_multiboot(multiboot_info_t *multiboot_ptr)
{
    // walk the multiboot table for memory, and find usable "high" memory
    uint64_t i,j;

    memory_num_free_pages = 0;

    multiboot_memory_map_t *entries = (multiboot_memory_map_t *) multiboot_ptr->mmap_addr;
    unsigned int num_entries = multiboot_ptr->mmap_length / sizeof(multiboot_memory_map_t);

    for (i = 0; i < num_entries; i++) {
        if (entries[i].type != MULTIBOOT_MEMORY_AVAILABLE) {
            continue;
        }

        for (j = 0; j < entries[i].len ; j+= PAGE_SIZE)  {
            uint32_t address = (uint32_t) (entries[i].addr + +j);
            
            memory_page_free(address);
        }

    }

    // reserve the first few pages of memory
    memory_page_alloc(0);
    memory_page_alloc(PAGE_SIZE);

}

/**
 * Find and allocate a single page of memory.
 * @return the address of the page.
 * **/
uint32_t memory_find_and_alloc_page( )
{
    uint32_t address = memory_find_free_page( );
    memory_page_alloc(address);

    // zero out every page given out
    kmemset((void *) address, 0, PAGE_SIZE);

    return address;
}

/**
 * mark the memory used by the kernel as used.
 **/
static void memory_mark_kernel_as_used( )
{
    uint32_t addr;

    uint32_t start = (uint32_t) &_kernel_start;
    uint32_t end = (uint32_t) &_kernel_end;

    for (addr = start; addr <= end; addr+= 4096) {
        if (!memory_is_alloc(addr)) {
            memory_page_alloc(addr);
        }
    }
}

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

static void page_set_map(page_directory_t *directory, uint32_t virtual, uint32_t physical);

/**
 * Given a page directory, allocate the second-level page table, and mark
 * set up its permissions.  Set the top-level directory to point to that 
 * new page.  This does *not* set the entry in the page table, @see page_create_map
 * for that.
 * @param directory - the page table to update, can kernel's or users's
 * @param virtual - the virtual address that is being set up.
 * @returns address of the table that was allocated
 **/
static void page_create_dirent(page_directory_t *directory, uint32_t virtual)
{
    
    uint32_t table_address = memory_find_and_alloc_page( );

    uint32_t index = page_directory_for_virtual(virtual);
    if (directory->entry[index].present == 1) {
        kprintf("Error - page_create_dirent() - page already exists!");
        panic();
    }

    // store upper 20bit of address for physical table
    directory->entry[index].value = 0;
    directory->entry[index].page_addr = table_address >> 12;
    directory->entry[index].present = 1;

    kprintf("Created paging dirent for virt: %x, index: %u, table_addr: %x\n", 
        virtual,index, table_address);
}

/**
 * Map a virtual address to a physical address
 **/
static void page_create_map(page_table_t *table, uint32_t virtual, uint32_t physical)
{
    uint32_t table_index = page_table_for_virtual(virtual);
    if (table->entry[table_index].present) {
        kprintf("panic - page table already mapped.");
        panic();
    }

   // kprintf("page_create_map(%x,%x,%x), idx=%d\n", table, virtual, physical, table_index);
    // store upper 20bit of address for physical table
    table->entry[table_index].value = 0;
    table->entry[table_index].page_addr = physical >> 12;
    table->entry[table_index].present = 1;
}

/*
 * add an entry to map the physical and virtual address in the indicated page table 
 */
static void page_set_map(page_directory_t *directory, uint32_t virtual, uint32_t physical)
{
    // directory entry
    unsigned int directory_pos = page_directory_for_virtual(virtual);
    
    // is page present?
    if (directory->entry[directory_pos].present == 0) {
        // nope, need to create a table there
        page_create_dirent(directory, directory_pos);
    }

    // table exists (now) - get its address from the directory
    uint32_t table_addr = directory->entry[directory_pos].page_addr << 12;
    page_table_t *table = (page_table_t *) table_addr;

    // establish th emapping between physical and virtual addresses.
    page_create_map(table, virtual, physical);
}

void page_turn_on( )
{
    kprintf("Loading %x into CR3\n", &_kernel_page_directory);

    lcr3((uint32_t ) &_kernel_page_directory);
    
    uint32_t old = rcr0();
    uint32_t new = old | 0x80000001;
    kprintf("Loading %x into CR0\n", new);
    lcr0(new);
}

void page_debug( )
{
    kprintf("****| PAGE DIR @ %x|****\n", kpage_dir);
    
    int i,j, num_present =0;
    for (i = 0; i < 1024; i++) {
        if (kpage_dir->entry[i].present) num_present++;
    }
    kprintf("%d entries present\n", num_present);

    // walk the pages
    for (i = 0; i < 1024; i++) {
        if (kpage_dir->entry[i].present) {
            kprintf("range[%x->%x] -> %x\n", (i << 22), (i+1)<<22, kpage_dir->entry[i].page_addr << 12);

            page_table_t *table = (page_table_t *)(kpage_dir->entry[i].page_addr << 12);
            for (j = 0; j < 1024; j++) {
                if (table->entry[j].present) {
                    uint32_t phy = (i << 22) + (j << 12);
                    kprintf(" entry[%d/%x->%x] ", j,phy, table->entry[j].page_addr << 12);
                }
            }
            kprintf("\n");
        }
    }
}


/**
 * Initialize the page-directory for the kernel itself
 */
void page_init( )
{
    // this *must* be called *before* enabling paging ....
    kprintf("**********************\n");
    kprintf("Enabling paging\n");
    kprintf("kernel page start: %x\n",(unsigned) &_kernel_page_directory);

    // find a 4KB page for the page-directory
    kpage_dir = (page_directory_t *) &_kernel_page_directory;
    kmemset(kpage_dir, 0, sizeof(page_directory_t)); // zero out all of the entries

    // create a mapping for each page of the kernel's physical address to a virtual address
    // that is identical.
    uint32_t addr;
    uint32_t start = PAGE_SIZE;
    uint32_t end = (uint32_t) &_kernel_end;

    kprintf("Adding page mapping for %x to %x\n", start, end);
    for (addr= start; addr < end; addr+= 4096) {
        page_set_map(kpage_dir, addr, addr);
    }  

page_debug();
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

    kprintf("*************************\n");
    kprintf("Memory: %u total pages, %u pages free\n", (unsigned int) memory_num_pages, (unsigned int) memory_num_free_pages);
    kprintf("*************************\n");
}


