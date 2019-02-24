#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../src/pairs.h"
#include "../src/rcc.h"
#include "../src/utils.h"
#include "tests.h"

#define DEBUG 0
#define NUM_PROGS 1024

static inline void print_optim(R_Expr *e) {
  r_print_expr(e);
  printf(" -> ");
  r_print_expr(r_optimize(e, NULL));
  printf("\n");
}

int main(int argc, char *argv[]) {
  int count = 0, res, rand_depth, res_opt, res_uniq, res_rco, res_econ, res_ul,
      full_count;
  R_Expr *expr, *expr_opt, *uniq, *simple;
  C_Tail *c_tail;
  list_t vars = list_create();
  srand(time(0));

  printf("===================================================\n");
  printf("Testing Linked List...\n");

  test_list();
  printf("%sAll List Tests Passed.\n%s", GRN, NRM);

  printf("===================================================\n");
  printf("Interpreting a dozen r0 programs....\n");

  test_dozen_r0();

  printf("\n===================================================\n");
  printf("Testing 2^n...\n");

  for (size_t i = 0; i < 10; ++i) {
    expr = test_2n(i);
    res = r_interp(expr, NULL);
    if (res == (1 << i))
      printf("%s %d \t==\t %d %s\n", GRN, res, (1 << i), NRM);
    else
      printf("%s %d \t!=\t %d %s\n", GRN, res, (1 << i), NRM);
  }

  printf("===================================================\n");
  printf("Testing Random Program Generation...\n");

  for (size_t i = 0; i < NUM_PROGS; ++i) {
    rand_depth = rand() % 20;
    expr = randp(vars, 2);
    res = r_interp(expr, vars);
    if (DEBUG) {
      r_print_expr(expr);
      printf(" -> %d\n", res);
    }
    ++count;
  }

  printf("Successfully generated %d/%d programs\n", count, NUM_PROGS);
  assert(count == NUM_PROGS);

  printf("===================================================\n");

  printf("Add Optimizer Checks...\n");

  // Should reduce to the actual value
  R_Expr *a = new_add(new_num(7), new_num(10));
  print_optim(a);

  // Should combine across the additions
  R_Expr *b = new_add(new_num(10), new_add(new_num(7), new_read()));
  R_Expr *c = new_add(new_num(10), new_add(new_read(), new_num(7)));
  R_Expr *d = new_add(new_add(new_num(42), new_read()), new_num(7));
  R_Expr *e = new_add(new_add(new_read(), new_num(42)), new_num(7));

  print_optim(b);
  print_optim(c);
  print_optim(d);
  print_optim(e);

  printf("===================================================\n");

  printf("Neg Optimizer Checks...\n");

  R_Expr *f = new_neg(new_num(7));
  R_Expr *g = new_neg(new_neg(new_add(new_num(-7), new_num(10))));
  R_Expr *h = new_neg(new_add(new_num(42), new_read()));

  print_optim(f);
  print_optim(g);
  print_optim(h);

  printf("===================================================\n");

  printf("R1 Specific Optimizer Checks...\n");

  C_Program *cp, *cp_uncovered;
  R_Expr *x = new_var("x");
  R_Expr *z = new_var("z");
  R_Expr *l1 = new_let(x, new_num(7), new_add(x, x));
  R_Expr *l2 = new_let(x, new_num(42), new_neg(x));
  R_Expr *l3 = new_let(z, new_read(), new_neg(z));
  print_optim(l1);
  print_optim(l2);
  print_optim(l3);

  printf("===================================================\n");

  printf("General R1 Checks...\n");

  list_t new_vars, labels;
  count = full_count = 0;
  for (size_t i = 0; i < NUM_PROGS; ++i) {
    rand_depth = rand() % 12;
    expr = randp(vars, rand_depth);
    uniq = unique(expr);
    new_vars = list_create();
    simple = rco(uniq, &new_vars);
    c_tail = econ_expr(simple);
    labels = list_create();
    list_insert(labels, new_lbl_tail_pair("main", c_tail));
    cp = new_c_program(NULL, labels);
    cp_uncovered = uncover_locals(cp);
    res = r_interp(expr, NULL);
    res_uniq = r_interp(uniq, NULL);
    res_rco = r_interp(simple, NULL);
    res_econ = c_t_interp(c_tail, list_create());
    res_ul = c_p_interp(cp_uncovered);
    assert(res == res_uniq);
    assert(res_uniq == res_rco);
    assert(res_rco == res_econ);
    assert(res_econ == res_ul);
    if (DEBUG) {
      printf("Normal   : ");
      r_print_expr(expr);
      printf(" -> %d\n", res);
    }
    expr_opt = r_optimize(expr, NULL);
    res_opt = r_interp(expr_opt, NULL);
    if (DEBUG) {
      printf("Optimized: ");
      r_print_expr(expr_opt);
      printf("\n");
    }
    assert(res_opt == res);
    count += (res_opt == res);
    full_count += (res_opt == res && expr_opt->type == R_EXPR_NUM);
  }

  printf("%sSuccessfully optimized & uniquified %d/%d programs.%s\n", GRN,
         count, NUM_PROGS, NRM);

  printf("Fully optimized %d/%d\n", count, NUM_PROGS);
  assert(count == NUM_PROGS);

  printf("===================================================\n");

  printf("Testing a dozen R1 programs...\n");

  test_dozen_r1();

  printf("===================================================\n");

  printf("Testing X0 emitter...\n");

  test_x0_emit();

  printf("===================================================\n");

  printf("Testing a Dozen X0 Progs...\n");

  test_dozen_x0();
  exit(1);
  printf("===================================================\n");

  printf("Testing a Dozen C0 Progs...\n");

  test_dozen_c0();

  printf("===================================================\n");

  printf("testing uniquify...\n\n");

  test_uniquify();

  printf("===================================================\n");

  printf("testing rco...\n\n");

  test_rco();

  printf("===================================================\n");

  printf("testing econs...\n\n");

  test_econ();

  printf("===================================================\n");

  printf("testing uncover-locals...\n\n");

  test_uncover_locals();

  printf("===================================================\n");

  printf("testing select-instr...\n\n");

  test_select_instr();

  printf("===================================================\n");

  return 0;
}
