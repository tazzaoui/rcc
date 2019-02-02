#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/list.h"
#include "../src/rcc.h"
#include "../src/utils.h"
#include "tests.h"

#define NUM 1024

Expr *test_2n(int n) {
  if (n <= 0) return new_num(1);
  return new_add(test_2n(n - 1), test_2n(n - 1));
}

Expr *randp(int n) {
  int rand_num = GET_RAND();
  if (n <= 0) {
    if (rand_num % 2)
      return new_read();
    else
      return new_num(rand_num);
  } else {
    if (rand_num % 2) return new_neg(randp(n - 1));
    return new_add(randp(n - 1), randp(n - 1));
  }
}

void test_dozen_r0() {
  /* (3) Test numbers */
  Expr *x = new_num(42);
  Expr *y = new_num(-42);
  Expr *z = new_num(0);

  printf("x = ");
  print(x);
  printf("\ny = ");
  print(y);
  printf("\nz = ");
  print(z);

  /* (3) Test Additions */
  Expr *a = new_add(x, y);
  printf("\na = ");
  print(a);

  Expr *b = new_add(x, z);
  printf("\nb = ");
  print(b);

  Expr *c = new_add(a, b);
  printf("\nc = ");
  print(c);

  /* (3) Test Negations */
  Expr *n = new_neg(x);
  printf("\nn = ");
  print(n);

  Expr *m = new_neg(a);
  printf("\nm = ");
  print(m);

  Expr *l = new_neg(c);
  printf("\nl = ");
  print(l);

  /* (3) Test Reads */
  Expr *r1 = new_read();
  printf("\nr1 = ");
  print(r1);

  Expr *r2 = new_read();
  Expr *r_add = new_add(r1, r2);
  printf("\nr_add = ");
  print(r_add);

  Expr *r3 = new_read();
  Expr *r_neg = new_neg(r3);
  printf("\nr_neg = ");
  print(r_neg);
}

void test_dozen_r1() {
  Expr *x = new_var("x");
  Expr *y = new_var("y");
  Expr *z = new_var("z");

  print(x);
  printf("\n");
  print(y);
  printf("\n");
  print(z);
  printf("\n");

  Expr *let_x = new_let(x, new_num(42), new_num(10));
  print(let_x);
  printf("\n");

  Expr *let_y = new_let(y, new_num(42), new_neg(new_num(18)));
  print(let_y);
  printf("\n");

  Expr *let_z = new_let(z, new_num(42), new_add(new_num(2), new_num(-2)));
  print(let_z);
  printf("\n");

  Expr *let_h = new_let(x, new_add(new_num(42), new_num(42)),
                        new_add(new_num(2), new_num(-2)));
  print(let_h);
  printf("\n");

  Expr *let_i = new_let(y, new_num(12), new_add(new_var("P"), new_num(1)));
  print(let_i);
  printf("\n");

  Expr *let_j =
      new_let(z, new_add(new_num(4), new_num(2)), new_add(x, new_num(1)));
  print(let_j);
  printf("\n");

  Expr *let_k = new_let(x, new_neg(new_num(12)), new_neg(new_var("T")));
  print(let_k);
  printf("\n");

  Expr *let_l = new_let(y, new_read(), new_add(x, new_num(1)));
  print(let_l);
  printf("\n");

  Expr *let_m =
      new_let(z, new_num(42), new_neg(new_add(new_var("P"), new_num(1))));
  print(let_m);
  printf("\n");
}

void print_node(void *data) { printf("%lu ", (long)data); }

int cmp_nodes(void *a, void *b) {
  int *x = (int *)a;
  int *y = (int *)b;
  return !(a != NULL && b != NULL && *x == *y);
}

void test_list() {
  int nums[NUM];
  for (int i = 0; i < NUM; ++i) nums[i] = rand();

  Node *head = list_create(nums);

  // Nothing in the list, find should always ret NULL
  for (int i = 1; i < NUM; ++i) {
    Node *temp = list_find(head, nums + i, cmp_nodes);
    assert(temp == NULL);
  }

  for (int i = 1; i < NUM; ++i) list_insert(&head, nums + i);

  // list_print(head, print_node);

  // List is populated, find should never ret NULL
  for (int i = 0; i < NUM; ++i) {
    Node *temp = list_find(head, nums + i, cmp_nodes);
    assert(temp != NULL);
  }

  // Update the list with new nums
  for (int i = 0; i < NUM; ++i) {
    int *n = malloc_or_die(sizeof(int));
    *n = -1 * nums[i];
    list_update(&head, nums + i, n, cmp_nodes);
  }

  // Old nums should no longer be in the list
  for (int i = 0; i < NUM; ++i) {
    Node *temp = list_find(head, nums + i, cmp_nodes);
    assert(temp == NULL);
  }

  // New nums should be in the list
  for (int i = 0; i < NUM; ++i) {
    int *n = malloc_or_die(sizeof(int));
    *n = -1 * nums[i];
    Node *temp = list_find(head, n, cmp_nodes);
    assert(temp != NULL);
  }

  // Update the list with old nums again
  for (int i = 0; i < NUM; ++i) {
    int *n = malloc_or_die(sizeof(int));
    *n = -1 * nums[i];
    list_update(&head, n, nums + i, cmp_nodes);
  }

  // Find should ret NULL after remove
  for (int i = 0; i < NUM; ++i) {
    list_remove(&head, nums + i, cmp_nodes);
    Node *temp = list_find(head, nums + i, cmp_nodes);
    assert(temp == NULL);
  }
}
