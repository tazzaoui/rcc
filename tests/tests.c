#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/list.h"
#include "../src/pairs.h"
#include "../src/rcc.h"
#include "../src/utils.h"

#include "tests.h"
#define NUM 1024
#define NUM_TYPES 2

R_Expr *test_2n(int n) {
  if (n <= 0)
    return new_num(1);
  return new_add(test_2n(n - 1), test_2n(n - 1));
}

R_Expr *randp_r0(int n) {
  int rand_num = GET_RAND();
  if (n <= 0) {
    if (rand_num % 2)
      return new_read();
    else
      return new_num(rand_num);
  } else {
    if (rand_num % 2)
      return new_neg(randp_r0(n - 1));
    return new_add(randp_r0(n - 1), randp_r0(n - 1));
  }
}

R_Expr *randp(list_t vars, int n) {
  list_t env;
  Node *node;
  R_Expr *var;
  env_pair_t *ep;
  int rand_num = GET_RAND(), choice = rand() % 3;
  if (n == 0) {
    if (choice == 0 && list_size(vars) == 0)
      choice += ((rand() % 2) + 1);
    switch (choice) {
      case 0:
        node = list_get(vars, rand() % list_size(vars));
        ep = (env_pair_t *) node->data;
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
        var = rand_var(1);
        ep = new_env_pair(var, 0);
        list_insert(env, ep);
        return new_let(var, randp(vars, n - 1), randp(env, n - 1));
    }
  return NULL;
}

R_Expr *randp_typed(int depth) {
  return rande(NULL, rand() % 2, depth);
}

R_Expr *rande(list_t env, R_TYPE type, int depth) {
  Node *node;
  list_t exprs;
  int rand_num;
  R_Expr *res1, *res2;
  r_type_exprs_pair_t *p;
  if (!env)
    env = list_create();
  switch (type) {
    case R_TYPE_BOOL:
      if (depth == 0) {
        rand_num = rand() % 3;
        p = new_r_type_exprs_pair(R_TYPE_BOOL, NULL);
        switch (rand_num) {
          case 0:
            return new_true();
          case 1:
            return new_false();
          default:
            node = list_find(env, p, r_type_exprs_pair_cmp);
            if (!node
                || !list_size(((r_type_exprs_pair_t *) node->data)->exprs))
              return rand() % 2 ? new_true() : new_false();
            exprs = ((r_type_exprs_pair_t *) node->data)->exprs;
            node = list_get(exprs, rand() % list_size(exprs));
            return node->data;
        };
      } else {
        rand_num = rand() % 3;
        switch (rand_num) {
          case 0:
            res1 = rande(env, R_TYPE_S64, depth - 1);
            res2 = rande(env, R_TYPE_S64, depth - 1);
            return new_cmp(rand() % 5, res1, res2);
          case 1:
            return rand_let(env, type, depth);
          default:
            return rand_if(env, type, depth);
        };
      }
    case R_TYPE_S64:
      if (depth == 0) {
        p = new_r_type_exprs_pair(R_TYPE_S64, NULL);
        rand_num = rand() % 2;
        if (rand_num == 0)
          return new_num(rand());
        else {
          node = list_find(env, p, r_type_exprs_pair_cmp);
          if (!node || !list_size(((r_type_exprs_pair_t *) node->data)->exprs))
            return new_num(rand());
          exprs = ((r_type_exprs_pair_t *) node->data)->exprs;
          node = list_get(exprs, rand() % list_size(exprs));
          return node->data;
        }
      } else
        return rand() % 2 ? rand_let(env, type, depth) : rand_if(env, type,
                                                                 depth);
    default:
      break;
  };
  return NULL;
}

R_Expr *rand_var(int len) {
  char *buf = malloc_or_die((len + 1) * sizeof(char));
  for (size_t i = 0; i < len; ++i)
    buf[i] = 0x61 + rand() % (0x7a - 0x61);
  buf[len] = 0x0;
  return new_var(buf);
}

R_Expr *rand_let(list_t env, R_TYPE type, int depth) {
  if (depth <= 0)
    return NULL;
  Node *node;
  list_t env_cpy;
  R_TYPE rand_type = rand() % NUM_TYPES;
  r_type_exprs_pair_t *p;
  R_Expr *var = rand_var(2), *res1, *res2;
  env_cpy = list_copy(env, r_type_exprs_pair_cpy);

  res1 = rande(env, rand_type, depth - 1);
  p = new_r_type_exprs_pair(rand_type, list_create());
  node = list_find(env_cpy, p, r_type_exprs_pair_cmp);

  if (!node)
    node = list_insert(env_cpy, p);
  p = node->data;

  list_remove(p->exprs, var, r_expr_cmp);
  list_insert(p->exprs, var);

  node = *env_cpy;
  while (node) {
    p = node->data;
    if (p->type != rand_type)
      list_remove(p->exprs, var, r_expr_cmp);
    node = node->next;
  }

  res2 = rande(env_cpy, type, depth - 1);
  return new_let(var, res1, res2);
}

R_Expr *rand_if(list_t env, R_TYPE type, int depth) {
  if (depth <= 0)
    return NULL;
  R_Expr *test = rande(env, R_TYPE_BOOL, depth - 1);
  R_Expr *then_expr = rande(env, type, depth - 1);
  R_Expr *else_expr = rande(env, type, depth - 1);
  return new_if(test, then_expr, else_expr);
}

void test_dozen_r0() {
  /* (3) Test numbers */
  R_Expr *x = new_num(42);
  R_Expr *y = new_num(-42);
  R_Expr *z = new_num(0);

  printf("x = ");
  r_print_expr(x);
  printf("\ny = ");
  r_print_expr(y);
  printf("\nz = ");
  r_print_expr(z);

  /* (3) Test Additions */
  R_Expr *a = new_add(x, y);
  printf("\na = ");
  r_print_expr(a);

  R_Expr *b = new_add(x, z);
  printf("\nb = ");
  r_print_expr(b);

  R_Expr *c = new_add(a, b);
  printf("\nc = ");
  r_print_expr(c);

  /* (3) Test Negations */
  R_Expr *n = new_neg(x);
  printf("\nn = ");
  r_print_expr(n);

  R_Expr *m = new_neg(a);
  printf("\nm = ");
  r_print_expr(m);

  R_Expr *l = new_neg(c);
  printf("\nl = ");
  r_print_expr(l);

  /* (3) Test Reads */
  R_Expr *r1 = new_read();
  printf("\nr1 = ");
  r_print_expr(r1);

  R_Expr *r2 = new_read();
  R_Expr *r_add = new_add(r1, r2);
  printf("\nr_add = ");
  r_print_expr(r_add);

  R_Expr *r3 = new_read();
  R_Expr *r_neg = new_neg(r3);
  printf("\nr_neg = ");
  r_print_expr(r_neg);
}

void test_dozen_r1() {
  R_Expr *x = new_var("x");
  R_Expr *y = new_var("y");
  R_Expr *z = new_var("z");

  r_print_expr(x);
  printf("\n");
  r_print_expr(y);
  printf("\n");
  r_print_expr(z);
  printf("\n");

  R_Expr *let_x = new_let(x, new_num(42), new_num(10));
  r_print_expr(let_x);
  printf("\n");

  R_Expr *let_y = new_let(y, new_num(42), new_neg(new_num(18)));
  r_print_expr(let_y);
  printf("\n");

  R_Expr *let_z = new_let(z, new_num(42), new_add(new_num(2), new_num(-2)));
  r_print_expr(let_z);
  printf("\n");

  R_Expr *let_h = new_let(x, new_add(new_num(42), new_num(42)),
                          new_add(new_num(2), new_num(-2)));
  r_print_expr(let_h);
  printf("\n");

  R_Expr *let_i = new_let(y, new_num(12), new_add(new_var("P"), new_num(1)));
  r_print_expr(let_i);
  printf("\n");

  R_Expr *let_j =
    new_let(z, new_add(new_num(4), new_num(2)), new_add(x, new_num(1)));
  r_print_expr(let_j);
  printf("\n");

  R_Expr *let_k = new_let(x, new_neg(new_num(12)), new_neg(new_var("T")));
  r_print_expr(let_k);
  printf("\n");

  R_Expr *let_l = new_let(y, new_read(), new_add(x, new_num(1)));
  r_print_expr(let_l);
  printf("\n");

  R_Expr *let_m =
    new_let(z, new_num(42), new_neg(new_add(new_var("P"), new_num(1))));
  r_print_expr(let_m);
  printf("\n");
}

void print_node(void *data) {
  printf("%lu ", (long) data);
}

CMP cmp_nodes(void *a, void *b) {
  int *x = (int *) a;
  int *y = (int *) b;
  return (a != NULL && b != NULL && *x == *y) ? EQUAL : UNEQUAL;
}

void *cpy_node(void *data) {
  int *x = (int *) data;
  int *y = malloc_or_die(sizeof(int));
  *y = *x;
  return (void *) y;
}

void test_list() {
  int nums[NUM];
  for (int i = 0; i < NUM; ++i)
    nums[i] = rand();

  list_t list = list_create(), copy = NULL;

  assert(list_size(list) == 0);
  assert(list_size(copy) == 0);

  // Nothing in the list, find should always ret NULL
  for (int i = 0; i < NUM; ++i) {
    Node *temp = list_find(list, nums + i, cmp_nodes);
    assert(temp == NULL);
  }

  for (int i = 0; i < NUM; ++i)
    list_insert(list, nums + i);
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

  // Test list concat
  list = list_create(), copy = list_create();
  int *j, *x;
  for (int i = 0; i < NUM; ++i) {
    j = malloc(sizeof(int));
    x = malloc(sizeof(int));
    *j = i;
    list_insert(list, j);
    *x = NUM + i;
    list_insert(copy, x);
  }

  int size_sum = list_size(list) + list_size(copy);
  list_t combined = list_concat(list, copy);

  assert(list_size(combined) == size_sum);

  for (int i = 0; i < NUM; ++i) {
    Node *n1 = list_find(combined, &i, cmp_nodes);
    *x = NUM + i;
    Node *n2 = list_find(combined, x, cmp_nodes);
    assert(n1 != NULL && n2 != NULL);
  }

  // Test list subtract
  list = list_create(), copy = list_create();
  for (int i = 0; i < NUM; ++i) {
    j = malloc(sizeof(int));
    x = malloc(sizeof(int));
    *j = i;
    list_insert(list, j);
    *x = i;
    list_insert(copy, x);
  }

  list_t diff = list_subtract(list, copy, cmp_nodes);
  assert(list_size(diff) == 0);
}

void test_x0_emit() {
  list_t instrs = list_create();
  list_t blks = list_create();

  X_Arg *left = new_x_arg(X_ARG_NUM, new_x_arg_num(10));
  X_Arg *right = new_x_arg(X_ARG_NUM, new_x_arg_num(42));
  X_Arg *rbx = new_x_arg(X_ARG_REG, new_x_arg_reg(RBX));
  X_Arg *rdi = new_x_arg(X_ARG_REG, new_x_arg_reg(RDI));

  X_Instr *i1 = new_x_instr(ADDQ, new_x_addq(left, right));
  X_Instr *i2 = new_x_instr(SUBQ, new_x_subq(right, left));
  X_Instr *i3 = new_x_instr(MOVQ, new_x_movq(rbx, rdi));
  X_Instr *i4 = new_x_instr(NEGQ, new_x_negq(left));
  X_Instr *i5 = new_x_instr(NEGQ, new_x_negq(rbx));

  list_insert(instrs, i1);
  list_insert(instrs, i2);
  list_insert(instrs, i3);
  list_insert(instrs, i4);
  list_insert(instrs, i5);

  X_Block *b = new_x_block(NULL, instrs);
  lbl_blk_pair_t *lbp = new_lbl_blk_pair("body", b);

  list_insert(blks, lbp);
  X_Program *xp = new_x_prog("hi", blks);

  x_emit(xp, NULL);
  printf("\n");
}

void test_prog_interp(X_Program * xp, const char *file_name, int test_num) {
  printf("\n***********************************\n");
  char cmd_buf[2048];
  if (file_name)
    x_emit(xp, file_name);
  sprintf(cmd_buf, "gcc %s", file_name);
  system(cmd_buf);
  int exit_code = system("./a.out");
  int interp_res = x_interp(xp);
  printf("./a.out: Exit Code = %d\n", exit_code);
  printf("Interp Result = %d", interp_res);
  printf("\n***********************************\n");
}

void test_dozen_x0() {
  X_Arg *n_10 = new_x_arg(X_ARG_NUM, new_x_arg_num(10));
  X_Arg *n_42 = new_x_arg(X_ARG_NUM, new_x_arg_num(42));

  X_Arg *rax = new_x_arg(X_ARG_REG, new_x_arg_reg(RAX));
  X_Arg *rbx = new_x_arg(X_ARG_REG, new_x_arg_reg(RBX));
  X_Arg *am, *v_x, *v_y;

  X_Instr *i1, *i2, *i3, *i4, *i5, *i6, *i7;

  i1 = new_x_instr(MOVQ, new_x_movq(n_10, rax));
  i2 = new_x_instr(ADDQ, new_x_addq(n_42, rax));
  i3 = new_x_instr(RETQ, new_x_retq());

  list_t instrs = list_create(), blks = list_create();

  list_insert(instrs, i1);
  list_insert(instrs, i2);
  list_insert(instrs, i3);

  X_Block *b = new_x_block(NULL, instrs);
  lbl_blk_pair_t *lbp = new_lbl_blk_pair("main", b);

  list_insert(blks, lbp);
  X_Program *xp = new_x_prog(NULL, blks);
  test_prog_interp(xp, "test0.s", 0);

  list_t instrs1 = list_create(), blks1 = list_create();

  i1 = new_x_instr(MOVQ, new_x_movq(n_42, rax));
  i2 = new_x_instr(SUBQ, new_x_subq(n_10, rax));
  i3 = new_x_instr(RETQ, new_x_retq());

  list_insert(instrs1, i1);
  list_insert(instrs1, i2);
  list_insert(instrs1, i3);

  b = new_x_block(NULL, instrs1);
  lbp = new_lbl_blk_pair("main", b);

  list_insert(blks1, lbp);
  xp = new_x_prog(NULL, blks1);
  test_prog_interp(xp, "test1.s", 1);

  list_t instrs2 = list_create(), blks2 = list_create();

  i1 = new_x_instr(MOVQ, new_x_movq(n_42, rax));
  i2 = new_x_instr(SUBQ, new_x_subq(n_10, rax));
  i3 = new_x_instr(RETQ, new_x_retq());
  i4 = new_x_instr(NEGQ, new_x_negq(rax));

  list_insert(instrs2, i1);
  list_insert(instrs2, i2);
  list_insert(instrs2, i4);
  list_insert(instrs2, i3);

  b = new_x_block(NULL, instrs2);
  lbp = new_lbl_blk_pair("main", b);

  list_insert(blks2, lbp);
  xp = new_x_prog(NULL, blks2);
  test_prog_interp(xp, "test2.s", 2);

  list_t instrs3 = list_create(), blks3 = list_create();

  i1 = new_x_instr(MOVQ, new_x_movq(n_42, rax));
  i2 = new_x_instr(SUBQ, new_x_subq(n_10, rax));
  i3 = new_x_instr(PUSHQ, new_x_pushq(rax));
  i4 = new_x_instr(POPQ, new_x_popq(rax));
  i5 = new_x_instr(RETQ, new_x_retq());

  list_insert(instrs3, i1);
  list_insert(instrs3, i2);
  list_insert(instrs3, i3);
  list_insert(instrs3, i4);
  list_insert(instrs3, i5);

  b = new_x_block(NULL, instrs3);
  lbp = new_lbl_blk_pair("main", b);

  list_insert(blks3, lbp);
  xp = new_x_prog(NULL, blks3);
  test_prog_interp(xp, "test3.s", 3);

  list_t instrs4 = list_create(), blks4 = list_create();

  i1 = new_x_instr(PUSHQ, new_x_pushq(n_42));
  i2 = new_x_instr(PUSHQ, new_x_pushq(n_10));
  i3 = new_x_instr(POPQ, new_x_popq(rax));
  i4 = new_x_instr(POPQ, new_x_popq(rbx));
  i5 = new_x_instr(ADDQ, new_x_addq(rbx, rax));
  i6 = new_x_instr(RETQ, new_x_retq());

  list_insert(instrs4, i1);
  list_insert(instrs4, i2);
  list_insert(instrs4, i3);
  list_insert(instrs4, i4);
  list_insert(instrs4, i5);
  list_insert(instrs4, i6);

  b = new_x_block(NULL, instrs4);
  lbp = new_lbl_blk_pair("main", b);

  list_insert(blks4, lbp);
  xp = new_x_prog(NULL, blks4);
  test_prog_interp(xp, "test4.s", 4);

  list_t instrs5 = list_create(), blks5 = list_create();

  i1 = new_x_instr(MOVQ, new_x_movq(n_42, rax));
  i2 = new_x_instr(NEGQ, new_x_negq(rax));
  i3 = new_x_instr(NEGQ, new_x_negq(rax));
  i4 = new_x_instr(RETQ, new_x_retq());

  list_insert(instrs5, i1);
  list_insert(instrs5, i2);
  list_insert(instrs5, i3);
  list_insert(instrs5, i4);

  b = new_x_block(NULL, instrs5);
  lbp = new_lbl_blk_pair("main", b);

  list_insert(blks5, lbp);
  xp = new_x_prog(NULL, blks5);
  test_prog_interp(xp, "test5.s", 5);

  list_t instrs6 = list_create(), blks6 = list_create();

  i1 = new_x_instr(MOVQ, new_x_movq(n_10, rbx));
  i2 = new_x_instr(MOVQ, new_x_movq(rbx, rax));
  i3 = new_x_instr(NEGQ, new_x_negq(rax));
  i4 = new_x_instr(NEGQ, new_x_negq(rax));
  i5 = new_x_instr(RETQ, new_x_retq());

  list_insert(instrs6, i1);
  list_insert(instrs6, i2);
  list_insert(instrs6, i3);
  list_insert(instrs6, i4);
  list_insert(instrs6, i5);

  b = new_x_block(NULL, instrs6);
  lbp = new_lbl_blk_pair("main", b);

  list_insert(blks6, lbp);
  xp = new_x_prog(NULL, blks6);
  test_prog_interp(xp, "test6.s", 6);

  list_t instrs7 = list_create(), blks7 = list_create();

  i1 = new_x_instr(MOVQ, new_x_movq(n_10, rbx));
  i2 = new_x_instr(MOVQ, new_x_movq(rbx, rax));
  i3 = new_x_instr(NEGQ, new_x_negq(rax));
  i4 = new_x_instr(NEGQ, new_x_negq(rax));
  i5 = new_x_instr(SUBQ, new_x_subq(rbx, rax));
  i6 = new_x_instr(RETQ, new_x_retq());

  list_insert(instrs7, i1);
  list_insert(instrs7, i2);
  list_insert(instrs7, i3);
  list_insert(instrs7, i4);
  list_insert(instrs7, i5);
  list_insert(instrs7, i6);

  b = new_x_block(NULL, instrs7);
  lbp = new_lbl_blk_pair("main", b);

  list_insert(blks7, lbp);
  xp = new_x_prog(NULL, blks7);
  test_prog_interp(xp, "test7.s", 7);

  list_t instrs8 = list_create(), blks8 = list_create();

  am = new_x_arg(X_ARG_MEM, new_x_arg_mem(RSP, 0));
  i1 = new_x_instr(MOVQ, new_x_movq(n_10, rbx));
  i2 = new_x_instr(PUSHQ, new_x_pushq(rbx));
  i3 = new_x_instr(MOVQ, new_x_movq(am, rax));
  i4 = new_x_instr(RETQ, new_x_retq());

  list_insert(instrs8, i1);
  list_insert(instrs8, i2);
  list_insert(instrs8, i3);
  list_insert(instrs8, i4);

  b = new_x_block(NULL, instrs8);
  lbp = new_lbl_blk_pair("main", b);

  list_insert(blks8, lbp);
  xp = new_x_prog(NULL, blks8);
  test_prog_interp(xp, "test8.s", 8);

  list_t instrs9 = list_create(), blks9 = list_create();

  i1 = new_x_instr(MOVQ, new_x_movq(n_42, rbx));
  i2 = new_x_instr(MOVQ, new_x_movq(rbx, am));
  i3 = new_x_instr(POPQ, new_x_popq(rax));
  i4 = new_x_instr(RETQ, new_x_retq());

  list_insert(instrs9, i1);
  list_insert(instrs9, i2);
  list_insert(instrs9, i3);
  list_insert(instrs9, i4);

  b = new_x_block(NULL, instrs9);
  lbp = new_lbl_blk_pair("main", b);

  list_insert(blks9, lbp);
  xp = new_x_prog(NULL, blks9);
  test_prog_interp(xp, "test9.s", 9);

  list_t instrs10 = list_create(), blks10 = list_create();

  i1 = new_x_instr(MOVQ, new_x_movq(n_10, rbx));
  i2 = new_x_instr(ADDQ, new_x_addq(n_42, rbx));
  i3 = new_x_instr(MOVQ, new_x_movq(n_10, rax));
  i4 = new_x_instr(SUBQ, new_x_subq(rbx, rax));
  i5 = new_x_instr(RETQ, new_x_retq());

  list_insert(instrs10, i1);
  list_insert(instrs10, i2);
  list_insert(instrs10, i3);
  list_insert(instrs10, i4);
  list_insert(instrs10, i5);

  b = new_x_block(NULL, instrs10);
  lbp = new_lbl_blk_pair("main", b);

  list_insert(blks10, lbp);
  xp = new_x_prog(NULL, blks10);
  test_prog_interp(xp, "test10.s", 10);

  list_t instrs11 = list_create(), blks11 = list_create();

  v_x = new_x_arg(X_ARG_VAR, new_x_arg_var("X"));
  v_y = new_x_arg(X_ARG_VAR, new_x_arg_var("Y"));

  i1 = new_x_instr(MOVQ, new_x_movq(n_42, v_x));
  i2 = new_x_instr(MOVQ, new_x_movq(n_10, v_y));
  i3 = new_x_instr(SUBQ, new_x_addq(v_x, v_y));
  i4 = new_x_instr(MOVQ, new_x_movq(v_y, rax));
  i5 = new_x_instr(RETQ, new_x_retq());

  list_insert(instrs11, i1);
  list_insert(instrs11, i2);
  list_insert(instrs11, i3);
  list_insert(instrs11, i4);
  list_insert(instrs11, i5);

  b = new_x_block(NULL, instrs11);
  lbp = new_lbl_blk_pair("main", b);

  list_insert(blks11, lbp);
  xp = new_x_prog(NULL, blks11);
  test_prog_interp(xp, "test11.s", 11);

  list_t instrs12 = list_create(), blks12 = list_create();

  am = new_x_arg(X_ARG_MEM, new_x_arg_mem(RSP, 8));
  i1 = new_x_instr(MOVQ, new_x_movq(n_42, v_x));
  i2 = new_x_instr(MOVQ, new_x_movq(n_10, v_y));
  i3 = new_x_instr(PUSHQ, new_x_pushq(v_x));
  i4 = new_x_instr(PUSHQ, new_x_pushq(v_y));
  i5 = new_x_instr(MOVQ, new_x_movq(am, v_y));
  i6 = new_x_instr(MOVQ, new_x_movq(v_y, rax));
  i7 = new_x_instr(RETQ, new_x_retq());

  list_insert(instrs12, i1);
  list_insert(instrs12, i2);
  list_insert(instrs12, i3);
  list_insert(instrs12, i4);
  list_insert(instrs12, i5);
  list_insert(instrs12, i6);
  list_insert(instrs12, i7);

  b = new_x_block(NULL, instrs12);
  lbp = new_lbl_blk_pair("main", b);

  list_insert(blks12, lbp);
  xp = new_x_prog(NULL, blks12);
  test_prog_interp(xp, "test12.s", 12);
}

void test_dozen_c0() {
  C_Arg *cn_10 = new_c_arg(C_NUM, new_c_num(10));
  C_Arg *cn_42 = new_c_arg(C_NUM, new_c_num(42));
  C_Arg *cn_n42 = new_c_arg(C_NUM, new_c_num(-42));

  C_Arg *cv_x = new_c_arg(C_VAR, new_c_var("X"));
  C_Arg *cv_y = new_c_arg(C_VAR, new_c_var("Y"));
  C_Arg *cv_z = new_c_arg(C_VAR, new_c_var("Z"));

  C_Tail *t = new_c_tail(C_TAIL_RET, new_c_ret(cn_10));

  list_t labels = list_create();
  list_insert(labels, new_lbl_tail_pair("main", t));
  C_Program *cp = new_c_program(NULL, labels);

  c_print(cp);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cn_42));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("main", t));
  cp = new_c_program(NULL, labels);
  c_print(cp);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_x));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("main", t));
  cp = new_c_program(NULL, labels);
  c_print(cp);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_y));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("main", t));
  cp = new_c_program(NULL, labels);
  c_print(cp);

  C_Smt *cs = new_c_smt(new_c_var("Y"), new_c_expr(C_ARG, cn_42));
  C_Seq *cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("main", t));
  cp = new_c_program(NULL, labels);
  c_print(cp);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_x));
  cs = new_c_smt(new_c_var("X"), new_c_expr(C_ARG, cn_10));
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("main", t));
  cp = new_c_program(NULL, labels);
  c_print(cp);

  C_Expr *add1 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_x));
  cs = new_c_smt(new_c_var("X"), add1);
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("main", t));
  cp = new_c_program(NULL, labels);
  c_print(cp);

  C_Expr *add2 = new_c_expr(C_ADD, new_c_add(cn_42, cn_10));
  C_Expr *add3 = new_c_expr(C_ADD, new_c_add(cv_x, cn_10));
  C_Expr *add4 = new_c_expr(C_ADD, new_c_add(cv_y, cn_42));
  C_Expr *add5 = new_c_expr(C_ADD, new_c_add(cv_x, cn_42));

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_y));
  cs = new_c_smt(new_c_var("Y"), add3);
  C_Smt *cs2 = new_c_smt(new_c_var("X"), add2);
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  C_Tail *t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("main", t2));
  cp = new_c_program(NULL, labels);
  c_print(cp);

  add2 = new_c_expr(C_ADD, new_c_add(cn_n42, cn_10));
  add3 = new_c_expr(C_ADD, new_c_add(cv_x, cn_10));

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  C_Smt *cs3 = new_c_smt(new_c_var("Z"), new_c_expr(C_NEG, new_c_neg(cv_y)));
  cseq = new_c_seq(cs3, t);
  C_Tail *t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("main", t3));
  cp = new_c_program(NULL, labels);
  c_print(cp);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add4);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("main", t3));
  cp = new_c_program(NULL, labels);
  c_print(cp);

  add5 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));
  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add5);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("main", t3));
  cp = new_c_program(NULL, labels);
  c_print(cp);

  add5 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));
  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("X"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add3);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("main", t3));
  cp = new_c_program(NULL, labels);
  c_print(cp);
}

void test_uniquify() {
  R_Expr *x = new_var("x");
  R_Expr *let_1 = new_let(x, new_num(7), x);
  R_Expr *let_2 = new_let(x, new_add(x, new_num(1)), new_add(x, x));
  R_Expr *let_3 = new_let(x, new_num(8), let_2);
  R_Expr *add = new_add(let_1, let_3);

  int n = 0;
  R_Expr *uniq = uniquify(add, list_create(), &n);
  r_print_expr(add);
  printf("\n");
  r_print_expr(uniq);
  printf("\n\n");

  let_1 = new_let(x, new_num(10), x);
  add = new_add(let_1, x);
  let_2 = new_let(x, new_num(32), add);

  n = 0;
  uniq = uniquify(let_2, list_create(), &n);
  r_print_expr(let_2);
  printf("\n");
  r_print_expr(uniq);
  printf("\n\n");

  add = new_add(x, new_num(1));
  let_1 = new_let(x, new_num(4), add);
  R_Expr *add1 = new_add(x, new_num(2));
  let_2 = new_let(x, let_1, add1);

  n = 0;
  uniq = uniquify(let_2, list_create(), &n);
  r_print_expr(let_2);
  printf("\n");
  r_print_expr(uniq);
  printf("\n\n");
}

void test_rco() {
  R_Expr *n1 = new_neg(new_num(10));
  R_Expr *n2 = new_num(52);
  R_Expr *add = new_add(n1, n2);
  R_Expr *x = new_var("x");
  R_Expr *y = new_var("y");
  R_Expr *simple;
  list_t res = NULL;

  simple = rco(n1, &res);
  r_print_expr(simple);
  printf("\n");

  res = NULL;
  simple = rco(add, &res);
  r_print_expr(simple);
  printf("\n");

  R_Expr *let_1 = new_let(y, x, y);
  R_Expr *let_2 = new_let(x, n2, let_1);
  res = NULL;
  simple = rco(let_2, &res);
  r_print_expr(simple);
  printf("\n");

  add = new_add(new_num(2), new_num(3));
  R_Expr *add2 = new_add(x, x);
  R_Expr *let_3 = new_let(x, new_read(), add2);
  res = NULL;
  simple = rco(let_3, &res);
  r_print_expr(simple);
  printf("\n");

  let_1 = new_let(x, new_num(7), x);
  let_2 = new_let(x, new_add(x, new_num(1)), new_add(x, x));
  let_3 = new_let(x, new_num(8), let_2);
  add = new_add(let_1, let_3);
  res = NULL;
  simple = rco(add, &res);
  r_print_expr(simple);
  printf("\n");

  add = new_add(x, new_num(1));
  let_1 = new_let(x, new_num(4), add);
  R_Expr *add1 = new_add(x, new_num(2));
  let_2 = new_let(x, let_1, add1);
  res = NULL;
  simple = rco(let_2, &res);
  r_print_expr(simple);
  printf("\n");

  let_1 = new_let(x, new_num(10), x);
  add = new_add(let_1, x);
  let_2 = new_let(x, new_num(32), add);
  res = NULL;
  simple = rco(let_2, &res);
  r_print_expr(simple);
  printf("\n");

  add = new_add(new_num(2), new_num(3));
  let_1 = new_add(add, new_let(x, new_read(), new_add(x, x)));
  res = NULL;
  simple = rco(let_1, &res);
  r_print_expr(simple);
  printf("\n");
}

void test_econ() {
  R_Expr *x = new_var("x");
  R_Expr *y = new_var("y");
  R_Expr *let_1 = new_let(x, new_num(7), x);
  R_Expr *let_2 = new_let(x, new_add(x, new_num(1)), new_add(x, x));
  R_Expr *let_3 = new_let(x, new_num(8), let_2);
  R_Expr *add = new_add(let_1, let_3);
  C_Tail *c_tail;
  list_t new_vars = NULL;

  int n = 0;
  R_Expr *uniq = uniquify(add, list_create(), &n);
  R_Expr *simple = rco(uniq, &new_vars);

  let_1 = new_let(x, new_num(10), x);
  add = new_add(let_1, x);
  let_2 = new_let(x, new_num(32), add);

  n = 0;
  new_vars = NULL;
  uniq = uniquify(let_2, list_create(), &n);
  simple = rco(uniq, &new_vars);
  c_tail = econ_expr(simple);
  c_print_tail(c_tail);

  add = new_add(x, new_num(1));
  let_1 = new_let(x, new_num(4), add);
  R_Expr *add1 = new_add(x, new_num(2));
  let_2 = new_let(x, let_1, add1);

  n = 0;
  new_vars = NULL;
  uniq = uniquify(let_2, list_create(), &n);
  simple = rco(uniq, &new_vars);
  c_tail = econ_expr(simple);
  c_print_tail(c_tail);

  n = 0;
  new_vars = NULL;
  add = new_add(new_num(2), new_num(3));
  let_1 = new_add(add, new_let(x, new_read(), new_add(x, x)));
  uniq = uniquify(let_1, list_create(), &n);
  simple = rco(uniq, &new_vars);
  c_tail = econ_expr(simple);
  c_print_tail(c_tail);

  add = new_add(x, new_let(x, new_num(22), x));
  let_1 = new_let(x, new_num(20), add);
  let_2 = new_let(y, let_1, y);

  uniq = uniquify(let_2, list_create(), &n);
  simple = rco(uniq, &new_vars);
  c_tail = econ_expr(simple);
  c_print_tail(c_tail);
}

void test_uncover_locals() {
  C_Arg *cn_10 = new_c_arg(C_NUM, new_c_num(10));
  C_Arg *cn_42 = new_c_arg(C_NUM, new_c_num(42));
  C_Arg *cn_n42 = new_c_arg(C_NUM, new_c_num(-42));

  C_Arg *cv_x = new_c_arg(C_VAR, new_c_var("X"));
  C_Arg *cv_y = new_c_arg(C_VAR, new_c_var("Y"));
  C_Arg *cv_z = new_c_arg(C_VAR, new_c_var("Z"));

  C_Tail *t = new_c_tail(C_TAIL_RET, new_c_ret(cn_10));

  list_t labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  C_Program *cp = new_c_program(NULL, labels);

  c_print(cp);
  uncover_locals(cp);

  C_Smt *cs = new_c_smt(new_c_var("Y"), new_c_expr(C_ARG, cn_42));
  C_Seq *cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  c_print(cp);
  uncover_locals(cp);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_x));
  cs = new_c_smt(new_c_var("X"), new_c_expr(C_ARG, cn_10));
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  c_print(cp);
  uncover_locals(cp);

  C_Expr *add1 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_x));
  cs = new_c_smt(new_c_var("X"), add1);
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  c_print(cp);
  uncover_locals(cp);

  C_Expr *add2 = new_c_expr(C_ADD, new_c_add(cn_42, cn_10));
  C_Expr *add3 = new_c_expr(C_ADD, new_c_add(cv_x, cn_10));
  C_Expr *add4 = new_c_expr(C_ADD, new_c_add(cv_y, cn_42));
  C_Expr *add5 = new_c_expr(C_ADD, new_c_add(cv_x, cn_42));

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_y));
  cs = new_c_smt(new_c_var("Y"), add3);
  C_Smt *cs2 = new_c_smt(new_c_var("X"), add2);
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  C_Tail *t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t2));
  cp = new_c_program(NULL, labels);
  c_print(cp);
  uncover_locals(cp);

  add2 = new_c_expr(C_ADD, new_c_add(cn_n42, cn_10));
  add3 = new_c_expr(C_ADD, new_c_add(cv_x, cn_10));

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  C_Smt *cs3 = new_c_smt(new_c_var("Z"), new_c_expr(C_NEG, new_c_neg(cv_y)));
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  C_Tail *t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  c_print(cp);
  uncover_locals(cp);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add4);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  c_print(cp);
  uncover_locals(cp);

  add5 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));
  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add5);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  c_print(cp);
  uncover_locals(cp);

  add5 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));
  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("X"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add3);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  c_print(cp);
  uncover_locals(cp);
}

void test_select_instr() {
  C_Arg *cn_10 = new_c_arg(C_NUM, new_c_num(10));
  C_Arg *cn_42 = new_c_arg(C_NUM, new_c_num(42));
  C_Arg *cn_n42 = new_c_arg(C_NUM, new_c_num(-42));

  C_Arg *cv_x = new_c_arg(C_VAR, new_c_var("X"));
  C_Arg *cv_y = new_c_arg(C_VAR, new_c_var("Y"));
  C_Arg *cv_z = new_c_arg(C_VAR, new_c_var("Z"));

  C_Tail *t = new_c_tail(C_TAIL_RET, new_c_ret(cn_10));

  list_t labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  C_Program *cp = new_c_program(NULL, labels);

  X_Program *xp = select_instr(cp);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cn_42));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  xp = select_instr(cp);

  C_Smt *cs = new_c_smt(new_c_var("Y"), new_c_expr(C_ARG, cn_42));
  C_Seq *cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  xp = select_instr(cp);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_x));
  cs = new_c_smt(new_c_var("X"), new_c_expr(C_ARG, cn_10));
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  xp = select_instr(cp);

  C_Expr *add1 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_x));
  cs = new_c_smt(new_c_var("X"), add1);
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  xp = select_instr(cp);

  C_Expr *add2 = new_c_expr(C_ADD, new_c_add(cn_42, cn_10));
  C_Expr *add3 = new_c_expr(C_ADD, new_c_add(cv_x, cn_10));
  C_Expr *add4 = new_c_expr(C_ADD, new_c_add(cv_y, cn_42));
  C_Expr *add5 = new_c_expr(C_ADD, new_c_add(cv_x, cn_42));

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_y));
  cs = new_c_smt(new_c_var("Y"), add3);
  C_Smt *cs2 = new_c_smt(new_c_var("X"), add2);
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  C_Tail *t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t2));
  cp = new_c_program(NULL, labels);
  xp = select_instr(cp);

  add2 = new_c_expr(C_ADD, new_c_add(cn_n42, cn_10));
  add3 = new_c_expr(C_ADD, new_c_add(cv_x, cn_10));

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  C_Smt *cs3 = new_c_smt(new_c_var("Z"), new_c_expr(C_NEG, new_c_neg(cv_y)));
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  C_Tail *t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  xp = select_instr(cp);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add4);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  xp = select_instr(cp);

  add5 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));
  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add5);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  xp = select_instr(cp);

  add5 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));
  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("X"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add3);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  xp = select_instr(cp);

  x_emit(xp, NULL);
}

void test_assign_homes() {
  C_Arg *cn_10 = new_c_arg(C_NUM, new_c_num(10));
  C_Arg *cn_42 = new_c_arg(C_NUM, new_c_num(42));
  C_Arg *cn_n42 = new_c_arg(C_NUM, new_c_num(-42));

  C_Arg *cv_x = new_c_arg(C_VAR, new_c_var("X"));
  C_Arg *cv_y = new_c_arg(C_VAR, new_c_var("Y"));
  C_Arg *cv_z = new_c_arg(C_VAR, new_c_var("Z"));

  C_Tail *t = new_c_tail(C_TAIL_RET, new_c_ret(cn_10));

  list_t labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  C_Program *cp = new_c_program(NULL, labels);

  X_Program *xp = select_instr(cp);
  X_Program *ah;

  t = new_c_tail(C_TAIL_RET, new_c_ret(cn_42));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);

  C_Smt *cs = new_c_smt(new_c_var("Y"), new_c_expr(C_ARG, cn_42));
  C_Seq *cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_x));
  cs = new_c_smt(new_c_var("X"), new_c_expr(C_ARG, cn_10));
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);

  C_Expr *add1 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_x));
  cs = new_c_smt(new_c_var("X"), add1);
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);

  C_Expr *add2 = new_c_expr(C_ADD, new_c_add(cn_42, cn_10));
  C_Expr *add3 = new_c_expr(C_ADD, new_c_add(cv_x, cn_10));
  C_Expr *add4 = new_c_expr(C_ADD, new_c_add(cv_y, cn_42));
  C_Expr *add5 = new_c_expr(C_ADD, new_c_add(cv_x, cn_42));

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_y));
  cs = new_c_smt(new_c_var("Y"), add3);
  C_Smt *cs2 = new_c_smt(new_c_var("X"), add2);
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  C_Tail *t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t2));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);

  add2 = new_c_expr(C_ADD, new_c_add(cn_n42, cn_10));
  add3 = new_c_expr(C_ADD, new_c_add(cv_x, cn_10));

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  C_Smt *cs3 = new_c_smt(new_c_var("Z"), new_c_expr(C_NEG, new_c_neg(cv_y)));
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  C_Tail *t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add4);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);

  add5 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));
  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add5);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);

  add5 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));
  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("X"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add3);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  x_emit(ah, NULL);
}

void test_patch_instrs() {
  C_Arg *cn_10 = new_c_arg(C_NUM, new_c_num(10));
  C_Arg *cn_42 = new_c_arg(C_NUM, new_c_num(42));
  C_Arg *cn_n42 = new_c_arg(C_NUM, new_c_num(-42));

  C_Arg *cv_x = new_c_arg(C_VAR, new_c_var("X"));
  C_Arg *cv_y = new_c_arg(C_VAR, new_c_var("Y"));
  C_Arg *cv_z = new_c_arg(C_VAR, new_c_var("Z"));

  C_Tail *t = new_c_tail(C_TAIL_RET, new_c_ret(cn_10));

  list_t labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  C_Program *cp = new_c_program(NULL, labels);

  X_Program *ah, *pi, *xp = select_instr(cp);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cn_42));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);

  C_Smt *cs = new_c_smt(new_c_var("Y"), new_c_expr(C_ARG, cn_42));
  C_Seq *cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_x));
  cs = new_c_smt(new_c_var("X"), new_c_expr(C_ARG, cn_10));
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);

  C_Expr *add1 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_x));
  cs = new_c_smt(new_c_var("X"), add1);
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);

  C_Expr *add2 = new_c_expr(C_ADD, new_c_add(cn_42, cn_10));
  C_Expr *add3 = new_c_expr(C_ADD, new_c_add(cv_x, cn_10));
  C_Expr *add4 = new_c_expr(C_ADD, new_c_add(cv_y, cn_42));
  C_Expr *add5 = new_c_expr(C_ADD, new_c_add(cv_x, cn_42));

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_y));
  cs = new_c_smt(new_c_var("Y"), add3);
  C_Smt *cs2 = new_c_smt(new_c_var("X"), add2);
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  C_Tail *t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t2));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);

  add2 = new_c_expr(C_ADD, new_c_add(cn_n42, cn_10));
  add3 = new_c_expr(C_ADD, new_c_add(cv_x, cn_10));

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  C_Smt *cs3 = new_c_smt(new_c_var("Z"), new_c_expr(C_NEG, new_c_neg(cv_y)));
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  C_Tail *t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add4);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);

  add5 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));
  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add5);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);

  add5 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));
  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("X"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add3);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  x_emit(pi, NULL);
}

void test_uncover_live() {
  C_Arg *cn_10 = new_c_arg(C_NUM, new_c_num(10));
  C_Arg *cn_42 = new_c_arg(C_NUM, new_c_num(42));
  C_Arg *cn_n42 = new_c_arg(C_NUM, new_c_num(-42));
  C_Arg *cv_x = new_c_arg(C_VAR, new_c_var("X"));
  C_Arg *cv_y = new_c_arg(C_VAR, new_c_var("Y"));
  C_Arg *cv_z = new_c_arg(C_VAR, new_c_var("Z"));

  C_Expr *add2 = new_c_expr(C_ADD, new_c_add(cn_42, cn_10));
  C_Expr *add3 = new_c_expr(C_ADD, new_c_add(cv_x, cn_10));
  C_Expr *add4 = new_c_expr(C_ADD, new_c_add(cv_y, cn_42));
  C_Expr *add5 = new_c_expr(C_ADD, new_c_add(cv_x, cn_42));
  C_Expr *add1 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));

  C_Tail *t = new_c_tail(C_TAIL_RET, new_c_ret(cn_10));
  X_Arg *rax = new_x_arg(X_ARG_REG, new_x_arg_reg(RAX));
  X_Arg *n_1 = new_x_arg(X_ARG_NUM, new_x_arg_num(1));
  X_Arg *n_46 = new_x_arg(X_ARG_NUM, new_x_arg_num(46));
  X_Arg *n_4 = new_x_arg(X_ARG_NUM, new_x_arg_num(4));
  X_Arg *n_7 = new_x_arg(X_ARG_NUM, new_x_arg_num(7));

  X_Arg *v = new_x_arg(X_ARG_VAR, new_x_arg_var("V"));
  X_Arg *w = new_x_arg(X_ARG_VAR, new_x_arg_var("W"));
  X_Arg *x = new_x_arg(X_ARG_VAR, new_x_arg_var("X"));
  X_Arg *y = new_x_arg(X_ARG_VAR, new_x_arg_var("Y"));
  X_Arg *z = new_x_arg(X_ARG_VAR, new_x_arg_var("Z"));
  X_Arg *tv = new_x_arg(X_ARG_VAR, new_x_arg_var("T"));

  list_t labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  C_Program *cp = new_c_program(NULL, labels);

  X_Program *ah, *pi, *xp = select_instr(cp);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cn_42));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);

  C_Smt *cs = new_c_smt(new_c_var("Y"), new_c_expr(C_ARG, cn_42));
  C_Seq *cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_x));
  cs = new_c_smt(new_c_var("X"), new_c_expr(C_ARG, cn_10));
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_x));
  cs = new_c_smt(new_c_var("X"), add1);
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_y));
  cs = new_c_smt(new_c_var("Y"), add3);
  C_Smt *cs2 = new_c_smt(new_c_var("X"), add2);
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  C_Tail *t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t2));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);

  add2 = new_c_expr(C_ADD, new_c_add(cn_n42, cn_10));
  add3 = new_c_expr(C_ADD, new_c_add(cv_x, cn_10));

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  C_Smt *cs3 = new_c_smt(new_c_var("Z"), new_c_expr(C_NEG, new_c_neg(cv_y)));
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  C_Tail *t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add4);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);

  add5 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));
  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add5);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);

  add5 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));
  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("X"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add3);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  x_emit(xp, NULL);
  pi = uncover_live(xp);
  pi = uncover_live(pi);

  list_t instrs = list_create(), blks = list_create();

  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(n_1, v)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(n_46, w)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(v, x)));
  list_insert(instrs, new_x_instr(ADDQ, new_x_addq(n_7, x)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(x, y)));
  list_insert(instrs, new_x_instr(ADDQ, new_x_addq(n_4, y)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(x, z)));
  list_insert(instrs, new_x_instr(ADDQ, new_x_addq(w, z)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(y, tv)));
  list_insert(instrs, new_x_instr(NEGQ, new_x_negq(tv)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(z, rax)));
  list_insert(instrs, new_x_instr(ADDQ, new_x_addq(tv, rax)));
  list_insert(instrs, new_x_instr(JMP, new_x_jmp("end")));

  X_Block *b = new_x_block(NULL, instrs);
  lbl_blk_pair_t *lbp = new_lbl_blk_pair("body", b);

  blks = list_create();
  list_insert(blks, lbp);
  xp = new_x_prog(NULL, blks);
  xp = uncover_live(xp);
}

void test_build_interferences() {
  C_Arg *cn_10 = new_c_arg(C_NUM, new_c_num(10));
  C_Arg *cn_42 = new_c_arg(C_NUM, new_c_num(42));
  C_Arg *cn_n42 = new_c_arg(C_NUM, new_c_num(-42));
  C_Arg *cv_x = new_c_arg(C_VAR, new_c_var("X"));
  C_Arg *cv_y = new_c_arg(C_VAR, new_c_var("Y"));
  C_Arg *cv_z = new_c_arg(C_VAR, new_c_var("Z"));

  C_Expr *add2 = new_c_expr(C_ADD, new_c_add(cn_42, cn_10));
  C_Expr *add3 = new_c_expr(C_ADD, new_c_add(cv_x, cn_10));
  C_Expr *add4 = new_c_expr(C_ADD, new_c_add(cv_y, cn_42));
  C_Expr *add5 = new_c_expr(C_ADD, new_c_add(cv_x, cn_42));
  C_Expr *add1 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));

  C_Tail *t = new_c_tail(C_TAIL_RET, new_c_ret(cn_10));
  X_Arg *rax = new_x_arg(X_ARG_REG, new_x_arg_reg(RAX));
  X_Arg *n_1 = new_x_arg(X_ARG_NUM, new_x_arg_num(1));
  X_Arg *n_46 = new_x_arg(X_ARG_NUM, new_x_arg_num(46));
  X_Arg *n_4 = new_x_arg(X_ARG_NUM, new_x_arg_num(4));
  X_Arg *n_7 = new_x_arg(X_ARG_NUM, new_x_arg_num(7));

  X_Arg *v = new_x_arg(X_ARG_VAR, new_x_arg_var("V"));
  X_Arg *w = new_x_arg(X_ARG_VAR, new_x_arg_var("W"));
  X_Arg *x = new_x_arg(X_ARG_VAR, new_x_arg_var("X"));
  X_Arg *y = new_x_arg(X_ARG_VAR, new_x_arg_var("Y"));
  X_Arg *z = new_x_arg(X_ARG_VAR, new_x_arg_var("Z"));
  X_Arg *tv = new_x_arg(X_ARG_VAR, new_x_arg_var("T"));

  list_t labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  C_Program *cp = new_c_program(NULL, labels);

  X_Program *ah, *pi, *xp = select_instr(cp);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cn_42));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);
  pi = build_interferences(pi);

  C_Smt *cs = new_c_smt(new_c_var("Y"), new_c_expr(C_ARG, cn_42));
  C_Seq *cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);
  pi = build_interferences(pi);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_x));
  cs = new_c_smt(new_c_var("X"), new_c_expr(C_ARG, cn_10));
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);
  pi = build_interferences(pi);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_x));
  cs = new_c_smt(new_c_var("X"), add1);
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);
  pi = build_interferences(pi);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_y));
  cs = new_c_smt(new_c_var("Y"), add3);
  C_Smt *cs2 = new_c_smt(new_c_var("X"), add2);
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  C_Tail *t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t2));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);
  pi = build_interferences(pi);

  add2 = new_c_expr(C_ADD, new_c_add(cn_n42, cn_10));
  add3 = new_c_expr(C_ADD, new_c_add(cv_x, cn_10));

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  C_Smt *cs3 = new_c_smt(new_c_var("Z"), new_c_expr(C_NEG, new_c_neg(cv_y)));
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  C_Tail *t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);
  pi = build_interferences(pi);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add4);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);
  pi = build_interferences(pi);

  add5 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));
  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add5);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);
  pi = build_interferences(pi);

  add5 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));
  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("X"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add3);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  x_emit(xp, NULL);
  pi = uncover_live(xp);
  pi = uncover_live(pi);
  pi = build_interferences(pi);

  list_t instrs = list_create(), blks = list_create();

  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(n_1, v)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(n_46, w)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(v, x)));
  list_insert(instrs, new_x_instr(ADDQ, new_x_addq(n_7, x)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(x, y)));
  list_insert(instrs, new_x_instr(ADDQ, new_x_addq(n_4, y)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(x, z)));
  list_insert(instrs, new_x_instr(ADDQ, new_x_addq(w, z)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(y, tv)));
  list_insert(instrs, new_x_instr(NEGQ, new_x_negq(tv)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(z, rax)));
  list_insert(instrs, new_x_instr(ADDQ, new_x_addq(tv, rax)));
  list_insert(instrs, new_x_instr(JMP, new_x_jmp("end")));

  X_Block *b = new_x_block(NULL, instrs);
  lbl_blk_pair_t *lbp = new_lbl_blk_pair("body", b);

  list_insert(blks, lbp);
  xp = new_x_prog(NULL, blks);
  xp = uncover_live(xp);
  xp = build_interferences(xp);
  list_print(xp->info->i_graph, print_x_arg_list_pair);
  printf("\n");
  list_print(xp->info->m_graph, print_x_arg_list_pair);
}

void test_color_graph() {
  C_Arg *cn_10 = new_c_arg(C_NUM, new_c_num(10));
  C_Arg *cn_42 = new_c_arg(C_NUM, new_c_num(42));
  C_Arg *cn_n42 = new_c_arg(C_NUM, new_c_num(-42));
  C_Arg *cv_x = new_c_arg(C_VAR, new_c_var("X"));
  C_Arg *cv_y = new_c_arg(C_VAR, new_c_var("Y"));
  C_Arg *cv_z = new_c_arg(C_VAR, new_c_var("Z"));

  C_Expr *add2 = new_c_expr(C_ADD, new_c_add(cn_42, cn_10));
  C_Expr *add3 = new_c_expr(C_ADD, new_c_add(cv_x, cn_10));
  C_Expr *add4 = new_c_expr(C_ADD, new_c_add(cv_y, cn_42));
  C_Expr *add5 = new_c_expr(C_ADD, new_c_add(cv_x, cn_42));
  C_Expr *add1 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));

  C_Tail *t = new_c_tail(C_TAIL_RET, new_c_ret(cn_10));
  X_Arg *rax = new_x_arg(X_ARG_REG, new_x_arg_reg(RAX));
  X_Arg *n_1 = new_x_arg(X_ARG_NUM, new_x_arg_num(1));
  X_Arg *n_46 = new_x_arg(X_ARG_NUM, new_x_arg_num(46));
  X_Arg *n_4 = new_x_arg(X_ARG_NUM, new_x_arg_num(4));
  X_Arg *n_7 = new_x_arg(X_ARG_NUM, new_x_arg_num(7));

  X_Arg *v = new_x_arg(X_ARG_VAR, new_x_arg_var("V"));
  X_Arg *w = new_x_arg(X_ARG_VAR, new_x_arg_var("W"));
  X_Arg *x = new_x_arg(X_ARG_VAR, new_x_arg_var("X"));
  X_Arg *y = new_x_arg(X_ARG_VAR, new_x_arg_var("Y"));
  X_Arg *z = new_x_arg(X_ARG_VAR, new_x_arg_var("Z"));
  X_Arg *tv = new_x_arg(X_ARG_VAR, new_x_arg_var("T"));

  list_t labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  C_Program *cp = new_c_program(NULL, labels);

  X_Program *ah, *pi, *xp = select_instr(cp);
  list_t instrs = list_create(), blks = list_create();

  t = new_c_tail(C_TAIL_RET, new_c_ret(cn_42));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);
  pi = build_interferences(pi);
  pi = color_graph(pi);

  C_Smt *cs = new_c_smt(new_c_var("Y"), new_c_expr(C_ARG, cn_42));
  C_Seq *cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);
  pi = build_interferences(pi);
  pi = color_graph(pi);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_x));
  cs = new_c_smt(new_c_var("X"), new_c_expr(C_ARG, cn_10));
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);
  pi = build_interferences(pi);
  pi = color_graph(pi);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_x));
  cs = new_c_smt(new_c_var("X"), add1);
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);
  pi = build_interferences(pi);
  pi = color_graph(pi);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_y));
  cs = new_c_smt(new_c_var("Y"), add3);
  C_Smt *cs2 = new_c_smt(new_c_var("X"), add2);
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  C_Tail *t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t2));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);
  pi = build_interferences(pi);
  pi = color_graph(pi);

  add2 = new_c_expr(C_ADD, new_c_add(cn_n42, cn_10));
  add3 = new_c_expr(C_ADD, new_c_add(cv_x, cn_10));

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  C_Smt *cs3 = new_c_smt(new_c_var("Z"), new_c_expr(C_NEG, new_c_neg(cv_y)));
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  C_Tail *t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);
  pi = build_interferences(pi);
  pi = color_graph(pi);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add4);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);
  pi = build_interferences(pi);
  pi = color_graph(pi);

  add5 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));
  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add5);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);
  pi = build_interferences(pi);
  pi = color_graph(pi);

  add5 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));
  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("X"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add3);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  x_emit(xp, NULL);
  pi = uncover_live(xp);
  pi = uncover_live(pi);
  pi = build_interferences(pi);

  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(n_1, v)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(n_46, w)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(v, x)));
  list_insert(instrs, new_x_instr(ADDQ, new_x_addq(n_7, x)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(x, y)));
  list_insert(instrs, new_x_instr(ADDQ, new_x_addq(n_4, y)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(x, z)));
  list_insert(instrs, new_x_instr(ADDQ, new_x_addq(w, z)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(y, tv)));
  list_insert(instrs, new_x_instr(NEGQ, new_x_negq(tv)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(z, rax)));
  list_insert(instrs, new_x_instr(ADDQ, new_x_addq(tv, rax)));
  list_insert(instrs, new_x_instr(JMP, new_x_jmp("end")));

  X_Block *b = new_x_block(NULL, instrs);
  lbl_blk_pair_t *lbp = new_lbl_blk_pair("body", b);

  list_insert(blks, lbp);
  xp = new_x_prog(NULL, blks);
  xp = uncover_live(xp);
  xp = build_interferences(xp);
  xp = color_graph(xp);
}

void test_assign_registers() {
  C_Arg *cn_10 = new_c_arg(C_NUM, new_c_num(10));
  C_Arg *cn_42 = new_c_arg(C_NUM, new_c_num(42));
  C_Arg *cn_n42 = new_c_arg(C_NUM, new_c_num(-42));
  C_Arg *cv_x = new_c_arg(C_VAR, new_c_var("X"));
  C_Arg *cv_y = new_c_arg(C_VAR, new_c_var("Y"));
  C_Arg *cv_z = new_c_arg(C_VAR, new_c_var("Z"));

  C_Expr *add2 = new_c_expr(C_ADD, new_c_add(cn_42, cn_10));
  C_Expr *add3 = new_c_expr(C_ADD, new_c_add(cv_x, cn_10));
  C_Expr *add4 = new_c_expr(C_ADD, new_c_add(cv_y, cn_42));
  C_Expr *add5 = new_c_expr(C_ADD, new_c_add(cv_x, cn_42));
  C_Expr *add1 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));

  C_Tail *t = new_c_tail(C_TAIL_RET, new_c_ret(cn_10));
  X_Arg *rax = new_x_arg(X_ARG_REG, new_x_arg_reg(RAX));
  X_Arg *n_1 = new_x_arg(X_ARG_NUM, new_x_arg_num(1));
  X_Arg *n_46 = new_x_arg(X_ARG_NUM, new_x_arg_num(46));
  X_Arg *n_4 = new_x_arg(X_ARG_NUM, new_x_arg_num(4));
  X_Arg *n_7 = new_x_arg(X_ARG_NUM, new_x_arg_num(7));

  X_Arg *v = new_x_arg(X_ARG_VAR, new_x_arg_var("V"));
  X_Arg *w = new_x_arg(X_ARG_VAR, new_x_arg_var("W"));
  X_Arg *x = new_x_arg(X_ARG_VAR, new_x_arg_var("X"));
  X_Arg *y = new_x_arg(X_ARG_VAR, new_x_arg_var("Y"));
  X_Arg *z = new_x_arg(X_ARG_VAR, new_x_arg_var("Z"));
  X_Arg *tv = new_x_arg(X_ARG_VAR, new_x_arg_var("T"));

  list_t labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  C_Program *cp = new_c_program(NULL, labels);

  X_Program *pi, *xp = select_instr(cp);
  list_t instrs = list_create(), blks = list_create();

  t = new_c_tail(C_TAIL_RET, new_c_ret(cn_42));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  pi = uncover_live(xp);
  pi = build_interferences(pi);
  pi = color_graph(pi);
  pi = assign_registers(pi);

  C_Smt *cs = new_c_smt(new_c_var("Y"), new_c_expr(C_ARG, cn_42));
  C_Seq *cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  pi = uncover_live(xp);
  pi = build_interferences(pi);
  pi = color_graph(pi);
  pi = assign_registers(pi);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_x));
  cs = new_c_smt(new_c_var("X"), new_c_expr(C_ARG, cn_10));
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  pi = uncover_live(xp);
  pi = build_interferences(pi);
  pi = color_graph(pi);
  pi = assign_registers(pi);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_x));
  cs = new_c_smt(new_c_var("X"), add1);
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  pi = uncover_live(xp);
  pi = build_interferences(pi);
  pi = color_graph(pi);
  pi = assign_registers(pi);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_y));
  cs = new_c_smt(new_c_var("Y"), add3);
  C_Smt *cs2 = new_c_smt(new_c_var("X"), add2);
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  C_Tail *t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t2));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  pi = uncover_live(pi);
  pi = build_interferences(pi);
  pi = color_graph(pi);
  pi = assign_registers(pi);

  add2 = new_c_expr(C_ADD, new_c_add(cn_n42, cn_10));
  add3 = new_c_expr(C_ADD, new_c_add(cv_x, cn_10));

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  C_Smt *cs3 = new_c_smt(new_c_var("Z"), new_c_expr(C_NEG, new_c_neg(cv_y)));
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  C_Tail *t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  pi = uncover_live(xp);
  pi = build_interferences(pi);
  x_emit(pi, NULL);
  list_print(pi->info->i_graph, print_x_arg_list_pair);
  pi = color_graph(pi);
  pi = assign_registers(pi);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add4);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  pi = uncover_live(xp);
  pi = build_interferences(pi);
  pi = color_graph(pi);
  pi = assign_registers(pi);

  add5 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));
  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add5);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  pi = uncover_live(xp);
  pi = build_interferences(pi);
  pi = color_graph(pi);
  pi = assign_registers(pi);

  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(n_1, v)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(n_46, w)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(v, x)));
  list_insert(instrs, new_x_instr(ADDQ, new_x_addq(n_7, x)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(x, y)));
  list_insert(instrs, new_x_instr(ADDQ, new_x_addq(n_4, y)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(x, z)));
  list_insert(instrs, new_x_instr(ADDQ, new_x_addq(w, z)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(y, tv)));
  list_insert(instrs, new_x_instr(NEGQ, new_x_negq(tv)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(z, rax)));
  list_insert(instrs, new_x_instr(ADDQ, new_x_addq(tv, rax)));
  list_insert(instrs, new_x_instr(JMP, new_x_jmp("end")));

  X_Block *b = new_x_block(NULL, instrs);
  lbl_blk_pair_t *lbp = new_lbl_blk_pair("body", b);

  list_insert(blks, lbp);
  xp = new_x_prog(NULL, blks);
  x_emit(xp, NULL);
  xp = uncover_live(xp);
  xp = build_interferences(xp);
  xp = color_graph(xp);
  printf("\nMapping = ");
  list_print(xp->info->colors, print_x_arg_int_pair);
  printf("\n");
  xp = assign_registers(xp);
  x_emit(xp, NULL);
}

void test_allocate_registers() {
  C_Arg *cn_10 = new_c_arg(C_NUM, new_c_num(10));
  C_Arg *cn_42 = new_c_arg(C_NUM, new_c_num(42));
  C_Arg *cn_n42 = new_c_arg(C_NUM, new_c_num(-42));
  C_Arg *cv_x = new_c_arg(C_VAR, new_c_var("X"));
  C_Arg *cv_y = new_c_arg(C_VAR, new_c_var("Y"));
  C_Arg *cv_z = new_c_arg(C_VAR, new_c_var("Z"));

  C_Expr *add2 = new_c_expr(C_ADD, new_c_add(cn_42, cn_10));
  C_Expr *add3 = new_c_expr(C_ADD, new_c_add(cv_x, cn_10));
  C_Expr *add4 = new_c_expr(C_ADD, new_c_add(cv_y, cn_42));
  C_Expr *add5 = new_c_expr(C_ADD, new_c_add(cv_x, cn_42));
  C_Expr *add1 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));

  C_Tail *t = new_c_tail(C_TAIL_RET, new_c_ret(cn_10));
  X_Arg *rax = new_x_arg(X_ARG_REG, new_x_arg_reg(RAX));
  X_Arg *n_1 = new_x_arg(X_ARG_NUM, new_x_arg_num(1));
  X_Arg *n_46 = new_x_arg(X_ARG_NUM, new_x_arg_num(46));
  X_Arg *n_4 = new_x_arg(X_ARG_NUM, new_x_arg_num(4));
  X_Arg *n_7 = new_x_arg(X_ARG_NUM, new_x_arg_num(7));

  X_Arg *v = new_x_arg(X_ARG_VAR, new_x_arg_var("V"));
  X_Arg *w = new_x_arg(X_ARG_VAR, new_x_arg_var("W"));
  X_Arg *x = new_x_arg(X_ARG_VAR, new_x_arg_var("X"));
  X_Arg *y = new_x_arg(X_ARG_VAR, new_x_arg_var("Y"));
  X_Arg *z = new_x_arg(X_ARG_VAR, new_x_arg_var("Z"));
  X_Arg *tv = new_x_arg(X_ARG_VAR, new_x_arg_var("T"));

  list_t labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  C_Program *cp = new_c_program(NULL, labels);

  X_Program *ah, *pi, *xp = select_instr(cp);
  list_t mapping, instrs = list_create(), blks = list_create();

  t = new_c_tail(C_TAIL_RET, new_c_ret(cn_42));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);
  pi = build_interferences(pi);
  pi = color_graph(pi);
  mapping = allocate_registers(pi);

  C_Smt *cs = new_c_smt(new_c_var("Y"), new_c_expr(C_ARG, cn_42));
  C_Seq *cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);
  pi = build_interferences(pi);
  pi = color_graph(pi);
  mapping = allocate_registers(pi);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_x));
  cs = new_c_smt(new_c_var("X"), new_c_expr(C_ARG, cn_10));
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);
  pi = build_interferences(pi);
  pi = color_graph(pi);
  mapping = allocate_registers(pi);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_x));
  cs = new_c_smt(new_c_var("X"), add1);
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);
  pi = build_interferences(pi);
  pi = color_graph(pi);
  mapping = allocate_registers(pi);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_y));
  cs = new_c_smt(new_c_var("Y"), add3);
  C_Smt *cs2 = new_c_smt(new_c_var("X"), add2);
  cseq = new_c_seq(cs, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  C_Tail *t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t2));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);
  pi = build_interferences(pi);
  pi = color_graph(pi);
  mapping = allocate_registers(pi);

  add2 = new_c_expr(C_ADD, new_c_add(cn_n42, cn_10));
  add3 = new_c_expr(C_ADD, new_c_add(cv_x, cn_10));

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  C_Smt *cs3 = new_c_smt(new_c_var("Z"), new_c_expr(C_NEG, new_c_neg(cv_y)));
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  C_Tail *t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);
  pi = build_interferences(pi);
  pi = color_graph(pi);
  mapping = allocate_registers(pi);

  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add4);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);
  pi = build_interferences(pi);
  pi = color_graph(pi);
  mapping = allocate_registers(pi);

  add5 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));
  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("Y"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add5);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  ah = assign_homes(xp);
  pi = patch_instrs(ah);
  pi = uncover_live(pi);
  pi = build_interferences(pi);
  pi = color_graph(pi);
  mapping = allocate_registers(pi);

  add5 = new_c_expr(C_ADD, new_c_add(cn_10, cn_42));
  t = new_c_tail(C_TAIL_RET, new_c_ret(cv_z));
  cs = new_c_smt(new_c_var("X"), add3);
  cs2 = new_c_smt(new_c_var("X"), add2);
  cs3 = new_c_smt(new_c_var("Z"), add3);
  cseq = new_c_seq(cs3, t);
  t = new_c_tail(C_TAIL_SEQ, cseq);
  t2 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs, t));
  t3 = new_c_tail(C_TAIL_SEQ, new_c_seq(cs2, t2));
  labels = list_create();
  list_insert(labels, new_lbl_tail_pair("body", t3));
  cp = new_c_program(NULL, labels);
  cp = uncover_locals(cp);
  xp = select_instr(cp);
  pi = uncover_live(xp);
  pi = uncover_live(pi);
  pi = build_interferences(pi);
  pi = color_graph(pi);
  mapping = allocate_registers(pi);

  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(n_1, v)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(n_46, w)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(v, x)));
  list_insert(instrs, new_x_instr(ADDQ, new_x_addq(n_7, x)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(x, y)));
  list_insert(instrs, new_x_instr(ADDQ, new_x_addq(n_4, y)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(x, z)));
  list_insert(instrs, new_x_instr(ADDQ, new_x_addq(w, z)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(y, tv)));
  list_insert(instrs, new_x_instr(NEGQ, new_x_negq(tv)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(z, rax)));
  list_insert(instrs, new_x_instr(ADDQ, new_x_addq(tv, rax)));
  list_insert(instrs, new_x_instr(JMP, new_x_jmp("end")));

  X_Block *b = new_x_block(NULL, instrs);
  lbl_blk_pair_t *lbp = new_lbl_blk_pair("body", b);

  blks = list_create();
  list_insert(blks, lbp);
  xp = new_x_prog(NULL, blks);
  xp = uncover_live(xp);
  xp = build_interferences(xp);
  xp = color_graph(xp);
  x_emit(xp, NULL);
  mapping = allocate_registers(xp);
  printf("\nMapping = ");
  list_print(mapping, print_x_arg_pair);
  printf("\n");
}

void test_move_biasing() {
  X_Program *xp;
  X_Arg *rax = new_x_arg(X_ARG_REG, new_x_arg_reg(RAX));
  X_Arg *n_1 = new_x_arg(X_ARG_NUM, new_x_arg_num(1));
  X_Arg *n_46 = new_x_arg(X_ARG_NUM, new_x_arg_num(46));
  X_Arg *n_4 = new_x_arg(X_ARG_NUM, new_x_arg_num(4));
  X_Arg *n_7 = new_x_arg(X_ARG_NUM, new_x_arg_num(7));
  X_Arg *v = new_x_arg(X_ARG_VAR, new_x_arg_var("V"));
  X_Arg *w = new_x_arg(X_ARG_VAR, new_x_arg_var("W"));
  X_Arg *x = new_x_arg(X_ARG_VAR, new_x_arg_var("X"));
  X_Arg *y = new_x_arg(X_ARG_VAR, new_x_arg_var("Y"));
  X_Arg *z = new_x_arg(X_ARG_VAR, new_x_arg_var("Z"));
  X_Arg *tv = new_x_arg(X_ARG_VAR, new_x_arg_var("T"));

  list_t instrs = list_create(), blks = list_create();

  R_Expr *r_expr = new_let(new_var("v"), new_num(1),
                           new_let(new_var("w"), new_num(46),
                                   new_let(new_var("x"),
                                           new_add(new_var("v"), new_num(7)),
                                           new_let(new_var("y"),
                                                   new_add(new_num(4),
                                                           new_var("x")),
                                                   new_let(new_var("z"),
                                                           new_add(new_var("x"),
                                                                   new_var
                                                                   ("w")),
                                                           new_add(new_var("z"),
                                                                   new_neg
                                                                   (new_var
                                                                    ("y"))))))));

  xp = compile(r_expr);
  x_emit(xp, NULL);

  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(n_1, v)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(n_46, w)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(v, x)));
  list_insert(instrs, new_x_instr(ADDQ, new_x_addq(n_7, x)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(x, y)));
  list_insert(instrs, new_x_instr(ADDQ, new_x_addq(n_4, y)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(x, z)));
  list_insert(instrs, new_x_instr(ADDQ, new_x_addq(w, z)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(y, tv)));
  list_insert(instrs, new_x_instr(NEGQ, new_x_negq(tv)));
  list_insert(instrs, new_x_instr(MOVQ, new_x_movq(z, rax)));
  list_insert(instrs, new_x_instr(ADDQ, new_x_addq(tv, rax)));
  list_insert(instrs, new_x_instr(JMP, new_x_jmp("end")));

  X_Block *b = new_x_block(NULL, instrs);
  lbl_blk_pair_t *lbp = new_lbl_blk_pair("body", b);

  list_insert(blks, lbp);
  xp = new_x_prog(NULL, blks);
  xp = reg_alloc(xp);
  x_emit(xp, NULL);
}

void test_r2() {
  R_Expr *res;

  R_Expr *r1 = new_and(new_true(), new_false());
  r_print_expr(r1);
  printf(" -> ");
  res = r_interp(r1, NULL);
  r_print_expr(res);
  printf("\n");

  R_Expr *r8 = new_and(new_false(), new_true());
  r_print_expr(r8);
  printf(" -> ");
  res = r_interp(r8, NULL);
  r_print_expr(res);
  printf("\n");

  R_Expr *r2 = new_or(new_false(), new_false());
  r_print_expr(r2);
  printf(" -> ");
  res = r_interp(r2, NULL);
  r_print_expr(res);
  printf("\n");

  R_Expr *r9 = new_or(new_true(), new_true());
  r_print_expr(r9);
  printf(" -> ");
  res = r_interp(r9, NULL);
  r_print_expr(res);
  printf("\n");

  R_Expr *r3 = new_not(new_true());
  r_print_expr(r3);
  printf(" -> ");
  res = r_interp(r3, NULL);
  r_print_expr(res);
  printf("\n");

  R_Expr *r10 = new_not(new_false());
  r_print_expr(r10);
  printf(" -> ");
  res = r_interp(r10, NULL);
  r_print_expr(res);
  printf("\n");

  R_Expr *r4 = new_cmp(R_CMP_EQUAL, new_num(7), new_num(8));
  r_print_expr(r4);
  printf(" -> ");
  res = r_interp(r4, NULL);
  r_print_expr(res);
  printf("\n");

  R_Expr *r11 = new_cmp(R_CMP_LESS, new_num(7), new_num(8));
  r_print_expr(r11);
  printf(" -> ");
  res = r_interp(r11, NULL);
  r_print_expr(res);
  printf("\n");

  R_Expr *r5 = new_cmp(R_CMP_EQUAL, new_num(7), new_num(7));
  r_print_expr(r5);
  printf(" -> ");
  res = r_interp(r5, NULL);
  r_print_expr(res);
  printf("\n");

  R_Expr *r12 = new_cmp(R_CMP_GEQ, new_num(7), new_num(7));
  r_print_expr(r12);
  printf(" -> ");
  res = r_interp(r12, NULL);
  r_print_expr(res);
  printf("\n");

  R_Expr *r6 = new_if(r4, new_num(9), new_num(8));
  r_print_expr(r6);
  printf(" -> ");
  res = r_interp(r6, NULL);
  r_print_expr(res);
  printf("\n");

  R_Expr *r7 = new_sub(new_num(9), new_num(1));
  r_print_expr(r7);
  printf(" -> ");
  res = r_interp(r7, NULL);
  r_print_expr(res);
  printf("\n");
}

void test_type_checker() {
  R_TYPE type;

  R_Expr *r1 = new_add(new_true(), new_false());
  type = r_type_check(r1, NULL);
  r_print_expr(r1);
  printf(" -> ");
  r_print_type(type);
  printf("\n");


  R_Expr *r2 = new_add(new_num(7), new_false());
  type = r_type_check(r2, NULL);
  r_print_expr(r2);
  printf(" -> ");
  r_print_type(type);
  printf("\n");

  R_Expr *r3 = new_not(new_num(7));
  type = r_type_check(r3, NULL);
  r_print_expr(r3);
  printf(" -> ");
  r_print_type(type);
  printf("\n");

  R_Expr *r4 = new_neg(new_false());
  type = r_type_check(r4, NULL);
  r_print_expr(r4);
  printf(" -> ");
  r_print_type(type);
  printf("\n");

  R_Expr *r5 = new_or(new_num(7), new_num(8));
  type = r_type_check(r5, NULL);
  r_print_expr(r5);
  printf(" -> ");
  r_print_type(type);
  printf("\n");

  R_Expr *r6 = new_or(new_num(7), new_false());
  type = r_type_check(r6, NULL);
  r_print_expr(r6);
  printf(" -> ");
  r_print_type(type);
  printf("\n");

  R_Expr *r7 = new_or(new_true(), new_false());
  type = r_type_check(r7, NULL);
  r_print_expr(r7);
  printf(" -> ");
  r_print_type(type);
  printf("\n");

  R_Expr *r8 = new_and(new_true(), new_false());
  type = r_type_check(r8, NULL);
  r_print_expr(r8);
  printf(" -> ");
  r_print_type(type);
  printf("\n");

  R_Expr *r9 = new_add(new_num(7), new_num(8));
  type = r_type_check(r9, NULL);
  r_print_expr(r9);
  printf(" -> ");
  r_print_type(type);
  printf("\n");

  R_Expr *r10 = new_neg(new_num(42));
  type = r_type_check(r10, NULL);
  r_print_expr(r10);
  printf(" -> ");
  r_print_type(type);
  printf("\n");
}

void test_r2_optimizer() {

  R_Expr *r1 = new_add(new_true(), new_false());
  r2_optimize(r1, NULL);

  R_Expr *r2 = new_add(new_num(7), new_false());
  r2_optimize(r2, NULL);

  R_Expr *r3 = new_not(new_num(7));
  r2_optimize(r3, NULL);

  R_Expr *r4 = new_neg(new_false());
  r2_optimize(r4, NULL);

  R_Expr *r5 = new_or(new_num(7), new_num(8));
  r2_optimize(r5, NULL);

  R_Expr *r6 = new_or(new_num(7), new_false());
  r2_optimize(r6, NULL);

  R_Expr *r7 = new_or(new_true(), new_false());
  r2_optimize(r7, NULL);

  R_Expr *r8 = new_and(new_true(), new_false());
  r2_optimize(r8, NULL);

  R_Expr *r9 = new_add(new_num(7), new_num(8));
  r2_optimize(r9, NULL);

  R_Expr *r10 = new_neg(new_num(42));
  r2_optimize(r10, NULL);

  R_Expr *r11 = new_cmp(R_CMP_GREATER, new_num(3), new_num(4));
  r2_optimize(r11, NULL);

  R_Expr *r12 = new_if(new_true(), new_false(), new_false());
  r2_optimize(r12, NULL);
}

void test_c2() {
  C_Arg *c7 = new_c_arg(C_NUM, new_c_num(7));
  C_Arg *c42 = new_c_arg(C_NUM, new_c_num(42));

  C_Arg *c6 = new_c_arg(C_TRUE, new_c_true());
  c_print_arg(c6);
  printf("\n");

  C_Expr *c1 = new_c_expr(C_CMP, new_c_cmp(C_CMP_EQUAL, c7, c42));
  c_print_expr(c1);
  printf("\n");

  C_Cmp *cp = new_c_cmp(C_CMP_LEQ, c7, c42);
  C_Cmp *cp2 = new_c_cmp(C_CMP_EQUAL, c7, c42);

  C_Expr *c2 = new_c_expr(C_CMP, cp);
  c_print_expr(c2);
  printf("\n");

  C_Expr *c3 = new_c_expr(C_CMP, cp2);
  c_print_expr(c3);
  printf("\n");

  C_Tail *c4 = new_c_tail(C_TAIL_GOTO, new_c_goto("label"));
  c_print_tail(c4);
  printf("\n");

  C_Tail *c5 =
    new_c_tail(C_TAIL_GOTO_IF, new_c_goto_if(c2, "true_lbl", "false_lbl"));
  c_print_tail(c5);
  printf("\n");
}

void test_x1() {
  X_Arg *x42 = new_x_arg(X_ARG_NUM, new_x_arg_num(42));
  X_Arg *rax = new_x_arg(X_ARG_REG, new_x_arg_reg(RAX));

  X_Instr *x1 = new_x_instr(CMPQ, new_x_cmpq(x42, x42));
  print_x_instr(x1);
  printf("\n");

  X_Instr *x2 = new_x_instr(XORQ, new_x_xorq(x42, x42));
  print_x_instr(x2);
  printf("\n");

  X_Instr *x3 = new_x_instr(SETCC, new_x_setcc(E, x42));
  print_x_instr(x3);
  printf("\n");

  X_Instr *x4 = new_x_instr(MOVZBQ, new_x_movzbq(x42, rax));
  print_x_instr(x4);
  printf("\n");

  X_Instr *x5 = new_x_instr(JMPIF, new_x_jmpif(GE, "someLabel"));
  print_x_instr(x5);
  printf("\n");

  X_Instr *x6 = new_x_instr(JMPIF, new_x_jmpif(L, "someLabel"));
  print_x_instr(x6);
  printf("\n");
}
