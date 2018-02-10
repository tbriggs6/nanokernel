#ifndef _LIST_H
#define _LIST_H

typedef struct list_node {
  void *data;
  struct list_node *next;
} list_node_t;


typedef struct list {
  list_node_t *head;
  list_node_t *tail;
  int count;
} list_t;

void list_init(list_t *list);
void list_append(list_t *list, void *data);
void list_remove(list_t *list, void *data);
void list_insert(list_t *list, void *data);
void list_iterate(list_t *list, int(*handler)(const void *));
void *list_search(list_t *list, const void *needle, int(*matcher)(const void *needle, const void *item));

#endif
