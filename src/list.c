#include <stdlib.h>
#include "utils.h"
#include "list.h" 

Node* list_create(void* data) {
  Node *n = malloc_or_die(sizeof(Node));
  n->next = NULL;
  n->data = data;
  return n;
}

void list_insert(Node** node, void* data){ 
  if (node != NULL && *node == NULL) {
    *node = malloc_or_die(sizeof(Node));
    (*node)->next = NULL;
    (*node)->data = data;
  } else
    list_insert(&((*node)->next), data);
}

void list_update(Node **node, void* old, void* new, cmp_func_t cmp) {
  if (node != NULL && *node != NULL) {
    if (cmp((*node)->data, old))
        (*node)->data = new;
    else
      list_update(&((*node)->next), old, new, cmp);
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

Node* list_find(Node* node, void *data, cmp_func_t cmp){
    if(node == NULL)
        return NULL;
    else if(cmp(node->data, data))
        return node;
    else
        return list_find(node->next, data, cmp);
}

void list_print(Node* node, print_func_t p){ 
    if(node != NULL){
        p(node->data);
        list_print(node->next, p);
    }
}

Node *list_copy(Node *old_head, deep_cpy_t dc){
    Node *new_node = NULL; 
    void *new_data;
    if(old_head != NULL){
        new_data = dc(old_head->data);
        new_node = list_create(new_data);
        new_node->next = list_copy(old_head->next, dc);
    }
    return new_node;
}


int list_size(Node* head){
    int size = 0;
    while(head != NULL){
        ++size;
        head = head->next;
    }
    return size; 
}
