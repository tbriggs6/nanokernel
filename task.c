#include "kstdlib.h"
#include "task.h"
#include "memory.h"
#include <stdint.h>

extern page_directory_t *kpage_dir;

uint8_t kernel_task0_stack[2048];
const uint32_t kernel_task0_sp = 
    ((uint32_t) &kernel_task0_stack + 2047);


task_t kernel_task0 = {
    .pid = 0,
    .parent_pid = 0,
    .paging = 0,
    .tss = { 
        .link = 0, .esp0 = 0, .ss0 = 0x10,
        .esp1 = 0, .ss1 = 0x10, .esp2 = 0, .ss2 = 0x10,
        .cr3 = 0, .eip = 0, .eflags = 0, 
        .eax = 0, .ecx = 0, .edx = 0, .ebx = 0,
        .esp = 0, .ebp = 0,
        .esi = 0, .edi = 0,
        .es = 0x10, 
        .cs = 0x04,
        .ds = 0x10,
        .ss = 0x10,
        .fs = 0x10,
        .gs = 0x10,
        .ldtr = 0x28,              
        .trace_bitmap = 104
        },
    .ldt = { DEFAULT_LDT_CODE, DEFAULT_LDT_DATA },
    .tss_entry = 0,
    .ldt_entry = 0,
    .idt_entry = 0,
    .state = 0,
    .next = 0
    };


void set_ldt(uint64_t entry)
{
    gdt_set_entry(6, &entry);
    
}

void set_tss(uint64_t entry)
{
    gdt_set_entry(5, &entry);
}
    



unsigned long long skellix_tss(unsigned long long tss) {
        unsigned long long tss_entry = 0x0080890000000067ULL;
        tss_entry |= ((tss)<<16) & 0xffffff0000ULL;
        tss_entry |= ((tss)<<32) & 0xff00000000000000ULL;
        return tss_entry;
}



unsigned long long skellix_ldt(unsigned long long ldt) {
        unsigned long long ldt_entry = 0x008082000000000fULL;
        ldt_entry |= ((ldt)<<16) & 0xffffff0000ULL;
        ldt_entry |= ((ldt)<<32) & 0xff00000000000000ULL;
        return ldt_entry;
}


extern void kernel_main_task( );
task_t task0_init( )
{
 
    kernel_task0.paging = (page_directory_t *) kpage_dir;
    kernel_task0.tss.esp0 = kernel_task0_sp;
    kernel_task0.tss.eip = kernel_main_task;
    gdt_entry_t entry = make_gdt_entry(&kernel_task0.ldt, 15, ldt_kernel_type);
    kernel_task0.ldt_entry = entry.entry_value;

    entry = make_gdt_entry(&kernel_task0.tss, sizeof(task_state_seg_t)-1, gdt_kernel_tss);
    kernel_task0.tss_entry = entry.entry_value;
    

    kprintf("Skelix TSS: %Lx\n", skellix_tss(&kernel_task0.tss));
    kprintf("Skelix LDT: %Lx\n", skellix_ldt(&kernel_task0.ldt));
    

    return kernel_task0;
}

void task_init()
{
    task0_init();

    gdt_set_value(6, kernel_task0.ldt_entry);
    gdt_set_value(5, kernel_task0.tss_entry);
    gdt_make_active();

    kprintf("Task Switch: TSS: %Lx\n", kernel_task0.tss_entry);
    kprintf("             LDT: %Lx\n", kernel_task0.ldt_entry);


// these should happen when the task loads
//    __asm__("ltr  %%ax\n" ::"a"(5*8));
//   __asm__("lldt  %%ax\n" ::"a"(6*8));

    // code = 1/000 and data = 10/000 = 0x40, 0x10
    // do task switch
    // __asm__("   movl    %%esp, %%eax\n"\
    //     "   pushl   %%ecx\n" \
    //     "   pushl   %%eax\n" \
    //     "   pushfl  \n" \
    //     "   pushl   %%ebx\n" \
    //     "   pushl   $1f\n" \
    //     "   iret    \n" \
    //     "1: movw    %%cx, %%ds\n" \
    //     "   movw    %%cx, %%es\n" \
    //     "   movw    %%cx, %%fs\n" \
    //     "   movw    %%cx, %%gs\n"
    //     : 
    //     : "b" (0x04), "c" (0x10)
    //     :  
    // );
}


void task_switch(task_t *task)
{
    // index is 5, so: 101,000 = 28
    __asm__ __volatile__("ljmp $0x28,$0\n");
}