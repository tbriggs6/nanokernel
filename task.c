#include "list.h"
#include "kstdlib.h"
#include "task.h"
#include "memory.h"
#include "lib/elf.h"

static list_t task_list;


void task_init()
{
    list_init(&task_list);
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
    task_t child;

    kmemset(&child, 0, sizeof(task_t));  
    child.pid = task_next_pid( );
    child.parent_pid = task->pid;
    kmemcpy(&child.tss, task->tss, sizeof(task));
}

static void *task_alloc(uint32_t virt_start, uint32_t length, void *alloc_data)
{
    task_t *task = (task_t*)alloc_data;
    page_create_memory(task->paging, virt_start, length);
    return (void *) virt_start;   
}

static void task_copy(uint32_t dest, uint32_t src, uint32_t len, void *copy_data)
{
    task_t *task = (task_t *)copy_data;
    page_copy_to_user((void *) src, task->paging, dest, len);
}

void task_create_from_elf(task_t *task, const char *elf_data)
{
    kmemset(task, 0, sizeof(task_t));
    task->pid = task_next_pid();
    task->parent_pid = 0; 
    task->tss = (task_state_seg_t *) page_kernel_alloc_page( );
    task->paging = (page_directory_t *) page_kernel_alloc_page( );
    kmemset(task->tss, 0, sizeof(task_state_seg_t));
    kmemset(task->paging, 0, sizeof(page_directory_t));

    read_elf(elf_data, task, task_alloc, task_copy);
}