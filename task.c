#include "list.h"
#include "kstdlib.h"
#include "task.h"
#include "memory.h"
#include "gdt.h"
#include "lib/elf.h"

static list_t task_list;

extern uint64_t *gdt;
extern uint32_t *gdtdesc;

  void gdt_set_tss(uint32_t address, uint32_t size) {

    kprintf("Setting TSS GDT Entry: %x, %d/%x\n", address, size, size);
    uint64_t limit_low = size & 0xffff;     // low 16-bits
    uint64_t limit_high = (size >> 16) & 0x0f; // high 4-bits
    uint64_t base_low = address & 0xffffff;
    uint64_t base_high = (address >> 24) & 0xff;
    uint64_t type = 0x89;
    uint64_t flags = 0x40;


    uint64_t gdt_value = 
        limit_low << 0 | 
        base_low << 16 |
        base_high << 56 |
        type << 40 |
        flags << 52;

    uint64_t *gdtarr = (uint64_t *)&gdt;
    kprintf("TSS Entry (%x),(%x): new: %Lx, old: %Lx\n", &gdt, &gdtarr[5], gdt_value, gdtarr[5]);
    gdtarr[5] = gdt_value;
  }


void task_init()
{
   // list_init(&task_list);

  kprintf("Size of tss: %d\n", sizeof(task_state_seg_t));
  
}

uint32_t task_next_pid( )
{
    static uint32_t pid = 1;

    return ++pid;
}

/**
 * This is the dumb version of fork() - no modern OS
 * would do it this way.
 * TODO: test this!!!!
 **/
void task_fork(task_t *task)
{
    // task_t child;

    // kmemset(&child, 0, sizeof(task_t));  
    // child.pid = task_next_pid( );
    // child.parent_pid = task->pid;
    // kmemcpy(&child.tss, task->tss, sizeof(task));
}

static void *task_alloc(uint32_t virt_start, uint32_t length, void *alloc_data)
{
    uint32_t phy_addr = memory_find_and_alloc_page( );
    task_t *task = (task_t*)alloc_data;
    page_map(task->paging, virt_start, phy_addr);
    return (void *) virt_start;   
}

static void task_copy_to_user(uint32_t dest, uint32_t src, uint32_t len, void *copy_data)
{
    task_t *task = (task_t *)copy_data;
    // copy from built-in kernel memory to user task

   copy_to_user(src, task->paging, dest, len);
}


extern page_directory_t *kpage_dir;

void task_create_from_kernel(task_t *task, void (*kernel_task)())
{
    kmemset(task, 0, sizeof(task_t));
    task->pid = 0;
    task->parent_pid = 0; 

    task->tss = (task_state_seg_t *) memory_find_and_alloc_page( );
    task->paging = kpage_dir;

    uint32_t stack_page_phys = memory_find_and_alloc_page( );
    page_map(kpage_dir, 0x7fff8000, stack_page_phys);
    
    
    kprintf("create proc for kernel: TSS@%x, PAGING@%x\n", task->tss, task->paging);

task_state_seg_t *t = task->tss;
    kprintf("link: %x\n", ((uint32_t)&(t->link)) - (uint32_t)t);
    kprintf("eflags: %x\n", ((uint32_t)&(t->eflags)) - (uint32_t)t);
    kprintf("es: %x\n", ((uint32_t)&(t->es)) - (uint32_t)t);
    kprintf("ldtr: %x\n", ((uint32_t)&(t->ldtr)) - (uint32_t)t);
    kprintf("iopb: %x\n", ((uint32_t)&(t->iopb_offset)) - (uint32_t)t);

    task->ldt[0] = 0x00cffa000000ffffULL;
    task->ldt[1] = 0x00cff2000000ffffULL;

    kmemset(task->tss, 0, sizeof(task_state_seg_t));
    task->tss->cs = GDT_KERNEL_CODE;
    task->tss->ds = GDT_KERNEL_DATA;
    task->tss->es = GDT_KERNEL_DATA;
    task->tss->fs = GDT_KERNEL_DATA;
    task->tss->gs = GDT_KERNEL_DATA;

    task->tss->ss = GDT_KERNEL_DATA;
    task->tss->ss0 = GDT_KERNEL_DATA;
    task->tss->ss1 = GDT_KERNEL_DATA;
    task->tss->ss2 = GDT_KERNEL_DATA;

    task->tss->esp = 0x7fff8fff;
    task->tss->esp0 = 0x7fff8fff;

    task->tss->cr3 = (uint32_t)task->paging;      // load the tasks' virtual memory directory
    task->tss->eip = (uint32_t)kernel_task;

    task->tss->ldtr = 0;
    task->tss->iopb_offset = sizeof(task_state_seg_t);

    //(uint32_t)task->ldt;

    //list_append(&task_list, task);
}

void task_create_from_elf(task_t *task, const char *elf_data)
{
    kmemset(task, 0, sizeof(task_t));
    task->pid = task_next_pid();
    task->parent_pid = 0; 
    task->tss = (task_state_seg_t *) memory_find_and_alloc_page( );
    task->paging = (page_directory_t *) memory_find_and_alloc_page( );
   
    kmemset(task->tss, 0, sizeof(task_state_seg_t));
    kmemset(task->paging, 0, sizeof(page_directory_t));

    uint32_t start_addr;
    read_elf(elf_data, (uint32_t *) &start_addr, task, task_alloc, task_copy_to_user);

    task->tss->cs = GDT_USER_CODE;
    task->tss->ds = GDT_USER_DATA;
    task->tss->es = GDT_USER_DATA;
    task->tss->fs = GDT_USER_DATA;
    task->tss->gs = GDT_USER_DATA;
    task->tss->ss = GDT_USER_DATA;
    task->tss->ss0 = GDT_KERNEL_DATA;
    task->tss->ss1 = GDT_KERNEL_DATA;
    task->tss->ss2 = GDT_KERNEL_DATA;

    task->tss->cr3 = (uint32_t)task->paging;      // load the tasks' virtual memory directory
    task->tss->eip = (uint32_t)start_addr;
    task->ldt[0] = 0x00cffa000000ffffULL;
    task->ldt[1] = 0x00cff2000000ffffULL;

    //list_append(&task_list, task);
}

static task_t *current = NULL;

void load_ldt(task_t *task)
{
    uint32_t ldt = task->ldt;
    unsigned long long ldt_entry = 0x008082000000000fULL;
    ldt_entry |= ((ldt)<<16) & 0xffffff0000ULL;
    ldt_entry |= ((ldt)<<32) & 0xff00000000000000ULL;


    uint64_t *gdtarr = (uint64_t *)&gdt;
    kprintf("LDT Entry (%x),(%x): new: %Lx, old: %Lx\n", &gdt, &gdtarr[6], ldt_entry, gdtarr[6]);
    gdtarr[6] = ldt_entry;
}

void switchto(task_t *task)
{
    current = task;
  uint32_t task_gate_segment = GDT_TSS_SEG;
  struct { uint16_t segment; uint32_t offset; } _tmp;
  _tmp.segment = task_gate_segment;
  _tmp.offset = 0;

  gdt_set_tss((uint32_t) task->tss, sizeof(task_state_seg_t));
  lgdt(&gdtdesc);     // reload the GDT

  ltr(GDT_TSS_SEG);       // load the TSS  -- byte address into the GDT for the TSS entry

  asm("\nhere: 	ljmp	$0x0028,$here\n"
      "clts\n"
      :   // no output arguments
      :   "m" (*&_tmp.offset)
  );
}