
#include <stdint.h>
#include "kstdlib.h"
#include "kmalloc.h"

#ifdef DEBUG
#define PRINTF(...) fprintf(stderr,__VA_ARGS__)
#else
#define PRINTF(...)
#endif

// defined by the linker
extern uint32_t *_heap_size;
extern uint32_t * _heap_start;
extern uint32_t * _heap_end;

// opaque pointers
struct knode;
typedef struct knode {
  uint32_t size;
  struct knode *prev, *next;
} knode_t;


// allocated and free list head nodes
static knode_t *free_list;
static knode_t *alloc_list;



void kmalloc_init( )
{
  free_list = (knode_t **) &_heap_start;
  free_list->prev = free_list->next = NULL;
  free_list->size = (uint32_t **) &_heap_size;

  alloc_list = (knode_t *) NULL;
}

// find the first node that has size bytes free
static knode_t *kmalloc_find_bysize(knode_t *list, size_t size)
{
  knode_t *curr = list;

  if (size == 0) return NULL; // alloc 0 bytes?
  if (curr == NULL)  return NULL; // no free nodes

  while ((curr != NULL) && (curr->size < size))
    curr = curr->next;

  return curr;
}

// find the node that immediately precedes this node in the list
// by address
static knode_t *kmalloc_find_byaddr(knode_t *list, knode_t *node)
{
  knode_t *curr = list;

  while ((curr->next != NULL) &&
	 ((uint8_t *) node > (((uint8_t *)curr) + curr->size )))
    curr = curr->next;

  return curr;
}



// delete a node from the list
static void kmalloc_delete_node(knode_t **list, knode_t *node)
{
  knode_t *curr = *list;

  while ((curr != NULL) && (curr != node))
    curr = curr->next;

  if (curr == NULL) {
    PRINTF("Error deleting node - not found\n");
    return;
  }

  if (curr->prev == NULL) {


    if (curr->next == NULL) {

      if (curr != *list) {
	PRINTF("Error - a singleton node must be root of list\n");
	return;
      }
      
      *list = NULL;
    }
    else {
      // delete first node in list
      *list = curr->next;
      
      curr->next->prev = NULL;
    }
  }

  else if (curr->next == NULL) {
    if (curr->prev == NULL) {
      PRINTF("Error - 2nd last node's next is null, not right\n");
      return;
    }

    curr->prev->next = NULL;
  }

  else {

    knode_t *prev = curr->prev;
    knode_t *next = curr->next;

    prev->next = next;
    next->prev = prev;
  }
}


static void kmalloc_insert(knode_t **list, knode_t *node)
{

  if (*list == NULL) {
    *list = node;
    return;
  }

  knode_t *curr = *list;
  while ((curr->next != NULL) && (curr < node))
    curr = curr->next;

  // insert before
  if (node < curr) {
    node->prev = curr->prev;
    node->next = curr;
    curr->prev = node;

    if (node->prev == NULL) 
      *list = node;
    else
      node->prev->next = node;
  }
  // insert after
  else {
    node->prev = curr;
    node->next = curr->next;
    curr->next = node;
    if (node->next != NULL)
      node->next->prev = node;
  }
}

// split-off size bytes from the src node, return the remaining node
static knode_t *kmalloc_split(knode_t *src, size_t size)
{
  if (src->size < size) {
    PRINTF("Error - cannot split more space than available\n");
    return NULL;
  }

  size_t orig_size = src->size;
  size_t rem_size = orig_size - size;

  knode_t *remain_node = (knode_t *) (((uint8_t *) src) + size);
  remain_node->size = rem_size;
  remain_node->prev = remain_node->next = NULL;

  src->size = size;
  src->prev = src->next = NULL;
  
  return remain_node;
}



#ifdef DEBUG
static void kmalloc_print_node(knode_t *node)
{

  if (node == NULL) {
    PRINTF("[NULL]\n");
  }
  else {
    uint8_t *ptr = ((uint8_t *)node) + 24;
    uint8_t *end = ((uint8_t *)node)+node->size - 1;
    PRINTF("[%p/%p: len: %u  %p/%p  %p)\n", node, ptr, node->size, node->prev, node->next, end);
  }

}

void kmalloc_debug_walk( )
{
  knode_t *curr;
  
  PRINTF("----------- Allocated Nodes -------------\n");
  curr = alloc_list;
  while (curr != NULL) {
    kmalloc_print_node(curr);
    curr = curr->next;
  }

  PRINTF("\n");
  PRINTF("----------- Free Nodes -------------\n");
  curr = free_list;
  while (curr != NULL) {
    kmalloc_print_node(curr);
    curr = curr->next;
  }
  PRINTF("------------------------------------\n");
}

#endif


void *kmalloc(size_t size)
{
  size_t internal_size = size + sizeof(knode_t);
  
  knode_t *free_node = kmalloc_find_bysize(free_list, internal_size);
  if (free_node == NULL) return NULL;

  kmalloc_delete_node(&free_list, free_node);

  // the free-node will be split, and it will become
  // the used space
  knode_t *remaining = kmalloc_split(free_node, internal_size);
  if (remaining->size > 0)
    kmalloc_insert(&free_list, remaining);

  if (alloc_list == NULL)
    alloc_list = free_node;
  else
    kmalloc_insert(&alloc_list, free_node);

  return (void *)(((uint8_t *) free_node)+sizeof(knode_t));
}


static void kmalloc_merge_into(knode_t *region, knode_t *prev, knode_t *next)
{
  uint8_t *prev_end = prev == NULL ? NULL : ((uint8_t *)prev) + prev->size;
  uint8_t *region_end = ((uint8_t *) region) + region->size;

  // we can merge with both
  if ((prev_end == (uint8_t *) region) && (region_end == (uint8_t *) next)) {
    kmalloc_delete_node(&free_list, next);
    prev->size = prev->size + region->size + next->size;
  }

  // only merge with previous
  else if (prev_end == (uint8_t *) region) {
    prev->size = prev->size + region->size;
  }

  // merge with next -- means deleting next
  // inserting adjusted region
  else if (region_end == (uint8_t *) next) {
    kmalloc_delete_node(&free_list, next);
    region->size = region->size + next->size;
    kmalloc_insert(&free_list, region);
  }

  // else insert the region without merge
  else {
    kmalloc_insert(&free_list, region);
  }
    
}

void kfree(void *block)
{
  knode_t *alloc_node = (knode_t *) (((uint8_t *) block) - sizeof(knode_t));

#ifdef DEBUG
  kmalloc_print_node(alloc_node);
#endif
  
  // delete the allocated node from the alloc list
  kmalloc_delete_node(&alloc_list, alloc_node);
  alloc_node->prev = alloc_node->next = NULL;   // clear pointers

  // put the allocated space back on the free list
  // prepare to insert the node into the free list
  if (alloc_node < free_list){
    kmalloc_merge_into(alloc_node, NULL, free_list);
    free_list = alloc_node;
  }
  else {
    knode_t *prev = kmalloc_find_byaddr(free_list, alloc_node);
    knode_t *next = prev->next;
    kmalloc_merge_into(alloc_node, prev, next);
  }

}
