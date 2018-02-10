#include "list.h"
#include "kstdlib.h"




void list_init(list_t *list)
{
  list->count = 0;
  list->head = NULL;
  list->tail = NULL;
}

void list_append(list_t *list, void *data)
{
  list_node_t *node = (list_node_t *) kmalloc(sizeof(list_node_t));

  node->data = data;
  node->next = NULL;
  
  if (list->tail == NULL) {
    list->head = list->tail = node;
    list->count = 1;
  }
  else {
    list->tail->next = node;
    list->tail = node;
    list->count++;
  }
}

void list_remove(list_t *list, void *data)
{
  list_node_t *node = list->head;
  list_node_t *prev = NULL;
  
  while(node != NULL) {
    if (node->data == data)
      break;
    
    prev = node;    
    node = node->next;
  }

  if (node == NULL)
    return;

  // delete at front of list
  if (node == list->head) {
    if (node == list->tail)
      list->tail = NULL;
    list->head = node->next;
    kfree(node);
  }

  // delete at end of list
  else if (node == list->tail) {
    list->tail = prev;
    prev->next = NULL;
    kfree(node);
  }

  // delete in middle of list
  else if (prev != NULL) {
    prev->next = node->next;
    kfree(node);
  }

  list->count--;
}


void list_insert(list_t *list, void *data)
{
  list_node_t *node = (list_node_t *) kmalloc(sizeof(list_node_t));

  node->data = data;
  node->next = list->head;
  list->count++;
  list->head = node;
}


void list_iterate(list_t *list, int(*handler)(const void *))
{
  list_node_t *node = list->head;

  while (node != NULL) {
    int rc = handler(node->data);
    if (rc == 1) {
      node = node->next;
    }
    else {
      break;
    }
  }
}
  

void *list_search(list_t *list, const void *needle, int(*matcher)(const void *needle, const void *item))
{
  list_node_t *node = list->head;
  
  while (node != NULL) {
    int rc = matcher(needle, node);
    if (rc == 1) {
      break;
    }
    else {
      node = node->next;
    }
  }
  
  if (node == NULL) return NULL;
  else return node->data;
}
