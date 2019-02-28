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
    if (cmp(old, (*list)->data))
        (*list)->data = new;
    else
      list_update(&((*list)->next), old, new, cmp);
  }
}

list_t list_subtract(list_t a, list_t b, cmp_func_t cmp){
    if(a && b && list_size(a) > 0 && list_size(b) > 0){
       Node* head = *b;
        while(head != NULL){
            list_remove_all(a, head->data, cmp);
            head = head->next;
        }
    }
    return a;
}

void list_remove_duplicates(list_t list, cmp_func_t cmp){
    Node *ptr1 = *list, *ptr2;
    while(ptr1 && ptr1->next){
        ptr2 = ptr1;
        while(ptr2->next)
            if(cmp(ptr1->data, ptr2->next->data)){
                ptr2->next = ptr2->next->next;
            } else ptr2 = ptr2->next; 
        ptr1 = ptr1->next;
    }
}

void list_remove_all(list_t list, void* data, cmp_func_t cmp){
    Node *node = list_find(list, data, cmp);
    while(node != NULL){
        list_remove(list, data, cmp);
        node = list_find(list, data, cmp);
    }
}

Node* delete_node(Node* node, void* data, cmp_func_t cmp){
  if (node){
	if (cmp(node->data, data)) 
        return node->next; 
    else 
        node->next = delete_node(node->next, data, cmp);
  }
  return node;
}


void list_remove(list_t node, void* data, cmp_func_t cmp) {
    *node = delete_node(*node, data, cmp);
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


list_t list_concat(list_t left, list_t right){
    if(left == NULL || *left == NULL)
        return right;
    if(right == NULL || *right == NULL)
        return left;
    if(left && *left && right && *right){
        Node *l_head = *left;
        while(l_head && l_head->next != NULL)
            l_head = l_head->next;
        l_head->next = *right;
    }
    return left;
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
