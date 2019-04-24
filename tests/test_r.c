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

static inline void print_optim(R_Expr * e) {
  r_print_expr(e);
  printf(" -> ");
  r_print_expr(r_optimize(e, NULL));
  printf("\n");
}

int main(int argc, char *argv[]) {
  int count = 0, res, rand_depth;
  R_Expr *expr, *expr_opt, *uniq, *rco, *res_expr, *res_optim;
  R_TYPE type, res_type, uniq_type, res_uniq_type, rco_type, res_rco_type,
    optim_type, res_optim_type;
  list_t vars = list_create();
  srand(time(0));

  printf("\n===================================================\n");
  printf("Testing 2^n...\n");

  for (size_t i = 0; i < 10; ++i) {
    expr = test_2n(i);
    res = r_int_interp(expr, NULL);
    if (res == (1 << i))
      printf("%s %d \t==\t %d %s\n", GRN, res, (1 << i), NRM);
    else
      printf("%s %d \t!=\t %d %s\n", GRN, res, (1 << i), NRM);
  }

  printf("===================================================\n");
  printf("Testing Random Program Generation...\n");

  for (size_t i = 0; i < NUM_PROGS; ++i) {
    rand_depth = rand() % 20;
    expr = randp(vars, rand_depth);
    res = r_int_interp(expr, vars);
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
  R_Expr *x = new_var("x");
  R_Expr *z = new_var("z");
  R_Expr *l1 = new_let(x, new_num(7), new_add(x, x));
  R_Expr *l2 = new_let(x, new_num(42), new_neg(x));
  R_Expr *l3 = new_let(z, new_read(), new_neg(z));
  print_optim(l1);
  print_optim(l2);
  print_optim(l3);

  printf("===================================================\n");

  printf("General R2 Tests...\n\n");

  for (size_t i = 0; i < NUM_PROGS - NUM_PROGS; ++i) {
    rand_depth = rand() % 15;

    /*  Generate random typed expression */
    expr = randp_typed(rand_depth);
    res_expr = r_interp(expr, NULL);

    /*  Uniquify  pass */
    uniq = unique(expr);
    uniq = r_interp(uniq, NULL);

    /* Optimizer pass */
    expr_opt = r2_optimize(expr, NULL);
    res_optim = r_interp(expr_opt, NULL);

    assert(get_int(res_expr) == get_int(uniq));
    assert(get_int(res_expr) == get_int(res_optim));
  }
  printf("===================================================\n");

  printf("R2 type checker tests...\n\n");

  for (size_t i = 0; i < NUM_PROGS; ++i) {
    rand_depth = rand() % 10;

    /*  Generate random typed expression */
    expr = randp_typed(rand_depth);
    type = r_type_check(expr, NULL);
    res_type = r_type_check(r_interp(expr, NULL), NULL);
    assert(type == res_type);

    /*  Uniquify */
    uniq = unique(expr);
    uniq_type = r_type_check(uniq, NULL);
    res_uniq_type = r_type_check(r_interp(uniq, NULL), NULL);
    assert(uniq_type == res_uniq_type);

    /* RCO */
    rco = rco_e(expr, list_create(), 0);
    rco_type = r_type_check(rco, NULL);
    res_rco_type = r_type_check(r_interp(rco, NULL), NULL);
    assert(rco_type == res_rco_type);

    /* Optimize the expression */
    expr_opt = r2_optimize(expr, NULL);
    optim_type = r_type_check(expr_opt, NULL);
    res_optim_type = r_type_check(r_interp(expr_opt, NULL), NULL);
    assert(optim_type == res_optim_type);

    /*  Cross-pass type checks */
    assert(type == uniq_type);
    assert(type == optim_type);
    assert(type == rco_type);
  }

  return 0;
}
