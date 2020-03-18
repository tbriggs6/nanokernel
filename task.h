#ifndef _TASK_H
#define _TASK_H

#include <stdint.h>
#include "memory.h"
/** 
 * Implement the Intel Task State Segment (TSS) Register 
 **/
typedef struct {
    uint32_t link;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldtr;
    uint32_t trace_bitmap;
} task_state_seg_t;

#define LDT_TASK_ENTRIES 8
#define LDT_TASK_SIZE (LDT_TASK_ENTRIES * sizeof(uint64_t))

typedef struct task {
    uint32_t pid;
    uint32_t parent_pid;
    uint32_t state;
    page_directory_t *paging;
    task_state_seg_t tss;
    uint64_t ldt[2];
    uint64_t tss_entry;
    uint64_t ldt_entry;
    uint64_t idt_entry;
    struct task *next;
} task_t;

#define DEFAULT_LDT_CODE (0x00cffa000000ffffULL)
#define DEFAULT_LDT_DATA (0x00cff2000000ffffULL)

extern task_t kernel_task0;
void task_init();
void task_switch(task_t *task);

#endif