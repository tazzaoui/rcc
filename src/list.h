#ifndef LIST_H
#define LIST_H

#include <stddef.h>

typedef struct Node Node;
struct Node {
  Node *next;
  void *data;
};

typedef Node **list_t;

/* A generic comparison function */
typedef int (*cmp_func_t)(void *, void *);

/* A generic printing function */
typedef void (*print_func_t)(void *);

/* A generic function for deep copying */
typedef void *(*deep_cpy_t)(void *);

/*Init the list*/
list_t list_create(void);

/*Create a node*/
Node *node_create(void *);

/*Insert an element into the list*/
void list_insert(list_t, void *);

/*Update the data element of a node*/
void list_update(list_t, void *, void *, cmp_func_t);

/*Remove the node by data*/
void list_remove(list_t, void *, cmp_func_t);

/*Return node containing data*/
Node *list_find(const list_t, void *, cmp_func_t);

/* Return a node at a given index */
Node *list_get(const list_t, size_t);

/* Return a copy of the list */
list_t list_copy(const list_t, deep_cpy_t);

/* Combine two lists into one */
list_t list_concat(list_t, list_t);

/*Print the list*/
void list_print(const list_t, print_func_t);

/* Return the num of elements in the list*/
size_t list_size(const list_t);

#endif /* LIST_H */
