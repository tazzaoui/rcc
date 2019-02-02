#ifndef LIST_H
#define LIST_H

typedef struct Node Node;
struct Node {
  Node *next;
  void *data;
};

typedef int (*cmp_func_t)(void *, void *);
typedef void (*print_func_t)(void *);

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

/*Return a deep copy of the list*/
Node *list_copy(Node **);

/*Print the list*/
void list_print(Node *, print_func_t);

#endif /* LIST_H */
