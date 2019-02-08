#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/list.h"
#include "../src/pairs.h"
#include "../src/rcc.h"
#include "../src/utils.h"

#include "tests.h"

#define NUM 1024

Expr *test_2n(int n) {
  if (n <= 0) return new_num(1);
  return new_add(test_2n(n - 1), test_2n(n - 1));
}

Expr *randp_r0(int n) {
  int rand_num = GET_RAND();
  if (n <= 0) {
    if (rand_num % 2)
      return new_read();
    else
      return new_num(rand_num);
  } else {
    if (rand_num % 2) return new_neg(randp_r0(n - 1));
    return new_add(randp_r0(n - 1), randp_r0(n - 1));
  }
}

Expr *randp(list_t vars, int n) {
  list_t env;
  Node *node;
  Expr *rand_var;
  env_pair_t *ep;
  char *buf;
  int rand_num = GET_RAND(), choice = rand() % 3;
  char rand_char;
  if (n == 0) {
    if (choice == 0 && list_size(vars) == 0) choice += ((rand() % 2) + 1);
    switch (choice) {
      case 0:
        node = list_get(vars, rand() % list_size(vars));
        ep = (env_pair_t *)node->data;
        return ep->var;
      case 1:
        return new_read();
      case 2:
        return new_num(rand_num);
    }
  } else
    switch (choice) {
      case 0:
        return new_neg(randp(vars, n - 1));
      case 1:
        return new_add(randp(vars, n - 1), randp(vars, n - 1));
      case 2:
        env = list_copy(vars, ep_cpy);
        buf = malloc_or_die(2 * sizeof(char));
        rand_char = 0x61 + rand() % (0x7a - 0x61);
        buf[0] = rand_char;
        buf[1] = 0x0;
        rand_var = new_var(buf);
        ep = new_env_pair(rand_var, 0);
        list_insert(env, ep);
        return new_let(rand_var, randp(vars, n - 1), randp(env, n - 1));
    }
  return NULL;
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
  return (a != NULL && b != NULL && *x == *y);
}

void *cpy_node(void *data) {
  int *x = (int *)data;
  int *y = malloc_or_die(sizeof(int));
  *y = *x;
  return (void *)y;
}

void test_list() {
  int nums[NUM];
  for (int i = 0; i < NUM; ++i) nums[i] = rand();

  list_t list = list_create(), copy = NULL;

  assert(list_size(list) == 0);
  assert(list_size(copy) == 0);

  // Nothing in the list, find should always ret NULL
  for (int i = 0; i < NUM; ++i) {
    Node *temp = list_find(list, nums + i, cmp_nodes);
    assert(temp == NULL);
  }

  for (int i = 0; i < NUM; ++i) list_insert(list, nums + i);
  assert(list_size(list) == NUM);

  copy = list_copy(list, cpy_node);
  assert(list_size(copy) == NUM);

  // Make sure copy was deep
  for (int i = 0; i < NUM; ++i) {
    Node *temp_copy = list_find(copy, nums + i, cmp_nodes);
    Node *temp_head = list_find(list, nums + i, cmp_nodes);
    assert(temp_copy->data != temp_head->data);
  }

  // List is populated, find should never ret NULL
  for (int i = 0; i < NUM; ++i) {
    Node *temp_head = list_find(list, nums + i, cmp_nodes);
    Node *temp_copy = list_find(copy, nums + i, cmp_nodes);
    assert(temp_head != NULL && temp_copy != NULL);
  }

  // Test find by index
  for (int i = 0; i < NUM; ++i) {
    Node *temp_head = list_find(list, nums + i, cmp_nodes);
    Node *get = list_get(list, i);
    assert(temp_head == get);
  }

  // Update the list with new nums
  for (int i = 0; i < NUM; ++i) {
    int *n = malloc_or_die(sizeof(int));
    *n = -1 * nums[i];
    list_update(list, nums + i, n, cmp_nodes);
  }

  // Old nums should no longer be in the old list
  for (int i = 0; i < NUM; ++i) {
    Node *temp_head = list_find(list, nums + i, cmp_nodes);
    Node *temp_copy = list_find(copy, nums + i, cmp_nodes);
    assert(temp_head == NULL && temp_copy != NULL);
  }

  // New nums should be in the list
  for (int i = 0; i < NUM; ++i) {
    int *n = malloc_or_die(sizeof(int));
    *n = -1 * nums[i];
    Node *temp_head = list_find(list, n, cmp_nodes);
    Node *temp_copy = list_find(copy, n, cmp_nodes);
    assert(temp_head != NULL && temp_copy == NULL);
  }

  // Update the list with old nums again
  for (int i = 0; i < NUM; ++i) {
    int *n = malloc_or_die(sizeof(int));
    *n = -1 * nums[i];
    list_update(list, n, nums + i, cmp_nodes);
  }

  // Find should ret NULL after remove
  for (int i = 0; i < NUM; ++i) {
    list_remove(list, nums + i, cmp_nodes);
    Node *temp = list_find(list, nums + i, cmp_nodes);
    assert(temp == NULL);
  }
}
