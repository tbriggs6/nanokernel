#include "list.h"
#include "kstdlib.h"
#include "task.h"

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
 **/
void task_fork(task_t *task)
{
    task_t child;

    kmemset(&child, 0, sizeof(task_t));  
    child.pid = task_next_pid( );
    child.parent_pid = task->pid;
    kmemcpy(&child.tss, task->tss, sizeof(task));
    git 
}


void task_create_from_elf(task_t *task, )