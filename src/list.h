#ifndef LIST_H
#define LIST_H

typedef struct Node Node;
struct Node {
  Node *next;
  void *data;
};

/* A generic comparison function */
typedef int (*cmp_func_t)(void *, void *);

/* A generic printing function */
typedef void (*print_func_t)(void *);

/* A generic function for deep copying */
typedef void *(*deep_cpy_t)(void *);

/*Init the list*/
Node *list_create(void *);

/*Insert an element into the list*/
void list_insert(Node **, void *);

/*Update the data element of a node*/
void list_update(Node **, void *, void *, cmp_func_t);

/*Remove the node by data*/
void list_remove(Node **, void *, cmp_func_t);

/*Return node containing data*/
Node *list_find(Node *, void *, cmp_func_t);

/*Return a copy of the list*/
Node *list_copy(Node *, deep_cpy_t);

/*Print the list*/
void list_print(Node *, print_func_t);

/* Return the num of elements in the list*/
int list_size(Node *);

#endif /* LIST_H */
