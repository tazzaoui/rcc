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

void test_x0_emit() {
  list_t instrs = list_create();
  list_t blks = list_create();

  Arg *left = new_arg(ARG_NUM, new_arg_num(10));
  Arg *right = new_arg(ARG_NUM, new_arg_num(42));
  Arg *rbx = new_arg(ARG_REG, new_arg_reg(RBX));
  Arg *rdi = new_arg(ARG_REG, new_arg_reg(RDI));

  Instr *i1 = new_instr(ADDQ, new_addq(left, right));
  Instr *i2 = new_instr(SUBQ, new_subq(right, left));
  Instr *i3 = new_instr(MOVQ, new_movq(rbx, rdi));
  Instr *i4 = new_instr(NEGQ, new_negq(left));
  Instr *i5 = new_instr(NEGQ, new_negq(rbx));

  list_insert(instrs, i1);
  list_insert(instrs, i2);
  list_insert(instrs, i3);
  list_insert(instrs, i4);
  list_insert(instrs, i5);

  Block *b = new_block(NULL, instrs);
  lbl_blk_pair_t *lbp = new_lbl_blk_pair("main", b);

  list_insert(blks, lbp);
  X_Program *xp = new_prog("hi", blks);

  x_emit(xp, NULL);
  printf("\n");
}

void test_prog_interp(X_Program *xp, const char *file_name, int test_num) {
    printf("\n***********************************\n");
    char cmd_buf[2048];
    if (file_name) x_emit(xp, file_name);
    sprintf(cmd_buf, "gcc %s", file_name);
    system(cmd_buf);  
    int exit_code = system("./a.out"); 
    int interp_res = x_interp(xp); 
    printf("./a.out: Exit Code = %d\n", exit_code);
    printf("Interp Result = %d", interp_res);
    printf("\n***********************************\n");
}

void test_dozen_x0() {
  Arg *n_10 = new_arg(ARG_NUM, new_arg_num(10));
  Arg *n_42 = new_arg(ARG_NUM, new_arg_num(42));

  Arg *rax = new_arg(ARG_REG, new_arg_reg(RAX));
  Arg *rbx = new_arg(ARG_REG, new_arg_reg(RBX));
  Arg *am, *v_x, *v_y;

  Instr *i1, *i2, *i3, *i4, *i5, *i6, *i7;

  i1 = new_instr(MOVQ, new_movq(n_10, rax));
  i2 = new_instr(ADDQ, new_addq(n_42, rax));
  i3 = new_instr(RETQ, new_retq());

  list_t instrs = list_create(), blks = list_create();

  list_insert(instrs, i1);
  list_insert(instrs, i2);
  list_insert(instrs, i3);

  Block *b = new_block(NULL, instrs);
  lbl_blk_pair_t *lbp = new_lbl_blk_pair("main", b);

  list_insert(blks, lbp);
  X_Program *xp = new_prog(NULL, blks);
  test_prog_interp(xp, "test0.s", 0);

  list_t instrs1 = list_create(), blks1 = list_create();

  i1 = new_instr(MOVQ, new_movq(n_42, rax));
  i2 = new_instr(SUBQ, new_subq(n_10, rax));
  i3 = new_instr(RETQ, new_retq());

  list_insert(instrs1, i1);
  list_insert(instrs1, i2);
  list_insert(instrs1, i3);

  b = new_block(NULL, instrs1);
  lbp = new_lbl_blk_pair("main", b);

  list_insert(blks1, lbp);
  xp = new_prog(NULL, blks1);
  test_prog_interp(xp, "test1.s", 1);

  list_t instrs2 = list_create(), blks2 = list_create();

  i1 = new_instr(MOVQ, new_movq(n_42, rax));
  i2 = new_instr(SUBQ, new_subq(n_10, rax));
  i3 = new_instr(RETQ, new_retq());
  i4 = new_instr(NEGQ, new_negq(rax));

  list_insert(instrs2, i1);
  list_insert(instrs2, i2);
  list_insert(instrs2, i4);
  list_insert(instrs2, i3);

  b = new_block(NULL, instrs2);
  lbp = new_lbl_blk_pair("main", b);

  list_insert(blks2, lbp);
  xp = new_prog(NULL, blks2);
  test_prog_interp(xp, "test2.s", 2);

  list_t instrs3 = list_create(), blks3 = list_create();

  i1 = new_instr(MOVQ, new_movq(n_42, rax));
  i2 = new_instr(SUBQ, new_subq(n_10, rax));
  i3 = new_instr(PUSHQ, new_pushq(rax));
  i4 = new_instr(POPQ, new_popq(rax));
  i5 = new_instr(RETQ, new_retq());

  list_insert(instrs3, i1);
  list_insert(instrs3, i2);
  list_insert(instrs3, i3);
  list_insert(instrs3, i4);
  list_insert(instrs3, i5);

  b = new_block(NULL, instrs3);
  lbp = new_lbl_blk_pair("main", b);

  list_insert(blks3, lbp);
  xp = new_prog(NULL, blks3);
  test_prog_interp(xp, "test3.s", 3);

  list_t instrs4 = list_create(), blks4 = list_create();

  i1 = new_instr(PUSHQ, new_pushq(n_42));
  i2 = new_instr(PUSHQ, new_pushq(n_10));
  i3 = new_instr(POPQ, new_popq(rax));
  i4 = new_instr(POPQ, new_popq(rbx));
  i5 = new_instr(ADDQ, new_addq(rbx, rax));
  i6 = new_instr(RETQ, new_retq());

  list_insert(instrs4, i1);
  list_insert(instrs4, i2);
  list_insert(instrs4, i3);
  list_insert(instrs4, i4);
  list_insert(instrs4, i5);
  list_insert(instrs4, i6);

  b = new_block(NULL, instrs4);
  lbp = new_lbl_blk_pair("main", b);

  list_insert(blks4, lbp);
  xp = new_prog(NULL, blks4);
  test_prog_interp(xp, "test4.s", 4);

  list_t instrs5 = list_create(), blks5 = list_create();

  i1 = new_instr(MOVQ, new_movq(n_42, rax));
  i2 = new_instr(NEGQ, new_negq(rax));
  i3 = new_instr(NEGQ, new_negq(rax));
  i4 = new_instr(RETQ, new_retq());

  list_insert(instrs5, i1);
  list_insert(instrs5, i2);
  list_insert(instrs5, i3);
  list_insert(instrs5, i4);

  b = new_block(NULL, instrs5);
  lbp = new_lbl_blk_pair("main", b);

  list_insert(blks5, lbp);
  xp = new_prog(NULL, blks5);
  test_prog_interp(xp, "test5.s", 5);

  list_t instrs6 = list_create(), blks6 = list_create();

  i1 = new_instr(MOVQ, new_movq(n_10, rbx));
  i2 = new_instr(MOVQ, new_movq(rbx, rax));
  i3 = new_instr(NEGQ, new_negq(rax));
  i4 = new_instr(NEGQ, new_negq(rax));
  i5 = new_instr(RETQ, new_retq());

  list_insert(instrs6, i1);
  list_insert(instrs6, i2);
  list_insert(instrs6, i3);
  list_insert(instrs6, i4);
  list_insert(instrs6, i5);

  b = new_block(NULL, instrs6);
  lbp = new_lbl_blk_pair("main", b);

  list_insert(blks6, lbp);
  xp = new_prog(NULL, blks6);
  test_prog_interp(xp, "test6.s", 6);

  list_t instrs7 = list_create(), blks7 = list_create();

  i1 = new_instr(MOVQ, new_movq(n_10, rbx));
  i2 = new_instr(MOVQ, new_movq(rbx, rax));
  i3 = new_instr(NEGQ, new_negq(rax));
  i4 = new_instr(NEGQ, new_negq(rax));
  i5 = new_instr(SUBQ, new_subq(rbx, rax));
  i6 = new_instr(RETQ, new_retq());

  list_insert(instrs7, i1);
  list_insert(instrs7, i2);
  list_insert(instrs7, i3);
  list_insert(instrs7, i4);
  list_insert(instrs7, i5);
  list_insert(instrs7, i6);

  b = new_block(NULL, instrs7);
  lbp = new_lbl_blk_pair("main", b);

  list_insert(blks7, lbp);
  xp = new_prog(NULL, blks7);
  test_prog_interp(xp, "test7.s", 7);

  list_t instrs8 = list_create(), blks8 = list_create();

  am = new_arg(ARG_MEM, new_arg_mem(RSP, 0));
  i1 = new_instr(MOVQ, new_movq(n_10, rbx));
  i2 = new_instr(PUSHQ, new_pushq(rbx));
  i3 = new_instr(MOVQ, new_movq(am, rax));
  i4 = new_instr(RETQ, new_retq());

  list_insert(instrs8, i1);
  list_insert(instrs8, i2);
  list_insert(instrs8, i3);
  list_insert(instrs8, i4);

  b = new_block(NULL, instrs8);
  lbp = new_lbl_blk_pair("main", b);

  list_insert(blks8, lbp);
  xp = new_prog(NULL, blks8);
  test_prog_interp(xp, "test8.s", 8);

  list_t instrs9 = list_create(), blks9 = list_create();

  i1 = new_instr(MOVQ, new_movq(n_42, rbx));
  i2 = new_instr(MOVQ, new_movq(rbx, am));
  i3 = new_instr(POPQ, new_popq(rax));
  i4 = new_instr(RETQ, new_retq());

  list_insert(instrs9, i1);
  list_insert(instrs9, i2);
  list_insert(instrs9, i3);
  list_insert(instrs9, i4);

  b = new_block(NULL, instrs9);
  lbp = new_lbl_blk_pair("main", b);

  list_insert(blks9, lbp);
  xp = new_prog(NULL, blks9);
  test_prog_interp(xp, "test9.s", 9);

  list_t instrs10 = list_create(), blks10 = list_create();

  i1 = new_instr(MOVQ, new_movq(n_10, rbx));
  i2 = new_instr(ADDQ, new_addq(n_42, rbx));
  i3 = new_instr(MOVQ, new_movq(n_10, rax));
  i4 = new_instr(SUBQ, new_subq(rbx, rax));
  i5 = new_instr(RETQ, new_retq());

  list_insert(instrs10, i1);
  list_insert(instrs10, i2);
  list_insert(instrs10, i3);
  list_insert(instrs10, i4);
  list_insert(instrs10, i5);

  b = new_block(NULL, instrs10);
  lbp = new_lbl_blk_pair("main", b);

  list_insert(blks10, lbp);
  xp = new_prog(NULL, blks10);
  test_prog_interp(xp, "test10.s", 10);

  list_t instrs11 = list_create(), blks11 = list_create();

  v_x = new_arg(ARG_VAR, new_arg_var("X"));
  v_y = new_arg(ARG_VAR, new_arg_var("Y"));

  i1 = new_instr(MOVQ, new_movq(n_42, v_x));
  i2 = new_instr(MOVQ, new_movq(n_10, v_y));
  i3 = new_instr(SUBQ, new_addq(v_x, v_y));
  i4 = new_instr(MOVQ, new_movq(v_y, rax));
  i5 = new_instr(RETQ, new_retq());

  list_insert(instrs11, i1);
  list_insert(instrs11, i2);
  list_insert(instrs11, i3);
  list_insert(instrs11, i4);
  list_insert(instrs11, i5);

  b = new_block(NULL, instrs11);
  lbp = new_lbl_blk_pair("main", b);

  list_insert(blks11, lbp);
  xp = new_prog(NULL, blks11);
  test_prog_interp(xp, "test11.s", 11);

  list_t instrs12 = list_create(), blks12 = list_create();

  am = new_arg(ARG_MEM, new_arg_mem(RSP, 8));
  i1 = new_instr(MOVQ, new_movq(n_42, v_x));
  i2 = new_instr(MOVQ, new_movq(n_10, v_y));
  i3 = new_instr(PUSHQ, new_pushq(v_x));
  i4 = new_instr(PUSHQ, new_pushq(v_y));
  i5 = new_instr(MOVQ, new_movq(am, v_y));
  i6 = new_instr(MOVQ, new_movq(v_y, rax));
  i7 = new_instr(RETQ, new_retq());

  list_insert(instrs12, i1);
  list_insert(instrs12, i2);
  list_insert(instrs12, i3);
  list_insert(instrs12, i4);
  list_insert(instrs12, i5);
  list_insert(instrs12, i6);
  list_insert(instrs12, i7);

  b = new_block(NULL, instrs12);
  lbp = new_lbl_blk_pair("main", b);

  list_insert(blks12, lbp);
  xp = new_prog(NULL, blks12);
  test_prog_interp(xp, "test12.s", 12);
}
