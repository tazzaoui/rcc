#include <stdlib.h>
#include "utils.h"
#include "list.h" 

list_t list_create() {
  list_t list = malloc_or_die(sizeof(list_t));
  *list = NULL;
  return list;
}

Node* node_create(void* data) {
  Node *n = malloc_or_die(sizeof(Node));
  n->next = NULL;
  n->data = data;
  return n;
}

void list_insert(list_t list, void* data){ 
  if (list != NULL && *list == NULL) {
    *list = malloc_or_die(sizeof(Node));
    (*list)->next = NULL;
    (*list)->data = data;
  } else
    list_insert(&((*list)->next), data);
}

void list_update(list_t list, void* old, void* new, cmp_func_t cmp) {
  if (list != NULL && *list != NULL) {
    if (cmp((*list)->data, old))
        (*list)->data = new;
    else
      list_update(&((*list)->next), old, new, cmp);
  }
}

void list_remove(Node** node, void* data, cmp_func_t cmp) {
  if (node != NULL && *node != NULL) {
    Node *n, *temp = *node;
    if ( cmp((*node)->data, data)) {
      *node = (*node)->next;
      free(temp);
      temp = NULL;
    } else {
      while (temp->next &&  !cmp((*node)->data, data)) 
          temp = temp->next;
      if (temp->next) {
        n = temp->next;
        temp->next = temp->next->next;
        free(n);
        n = NULL;
      }
    }
  }
}

Node* list_find(const list_t list, void *data, cmp_func_t cmp){
    Node *node = *list;
    if(node == NULL)
        return NULL;
    else if(cmp(data, node->data))
        return node;
    else
        return list_find(&(node->next), data, cmp);
}

void list_print(const list_t list, print_func_t p){ 
    if(list != NULL && *list != NULL){
        p((*list)->data);
        list_print(&(*list)->next, p);
    }
}

list_t list_copy(const list_t list, deep_cpy_t dc){
	Node *head = *list;
    list_t new_list = list_create(); 
    void *new_data;
    while(head != NULL){
        new_data = dc(head->data);
        list_insert(new_list, new_data);
        head = head->next;
    }
    return new_list; 
}


size_t list_size(const list_t list){
    size_t size = 0;
    if(list){
    Node *head = *list;
    while(head != NULL){
        ++size;
        head = head->next;
    }
    }
    return size; 
}

Node* list_get(const list_t list, size_t index){
    size_t i = 0;
    Node *head = *list, *res = head;
    if(head && index < list_size(&head))
        while(i++ < index)
            res = res->next; 
    return res;
}
