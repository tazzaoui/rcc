#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../src/rcc.h"
#include "../src/utils.h"
#include "tests.h"

#define DEBUG 0
#define NUM_PROGS (1 << 12)

static inline void print_optim(Expr *e) {
  print(e);
  printf(" -> ");
  print(optimize(e));
  printf("\n");
}

int main(int argc, char *argv[]) {
  int count = 0, res, rand_depth, res_opt, full_count;
  Expr *expr, *expr_opt;
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
    res = interp(expr, NULL);
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
    res = interp(expr, vars);
    if (DEBUG) {
      print(expr);
      printf(" -> %d\n", res);
    }
    ++count;
  }

  printf("Successfully generated %d/%d programs\n", count, NUM_PROGS);
  assert(count == NUM_PROGS);

  printf("===================================================\n");

  printf("Add Optimizer Checks...\n");

  // Should reduce to the actual value
  Expr *a = new_add(new_num(7), new_num(10));
  print_optim(a);

  // Should combine across the additions
  Expr *b = new_add(new_num(10), new_add(new_num(7), new_read()));
  Expr *c = new_add(new_num(10), new_add(new_read(), new_num(7)));
  Expr *d = new_add(new_add(new_num(42), new_read()), new_num(7));
  Expr *e = new_add(new_add(new_read(), new_num(42)), new_num(7));

  print_optim(b);
  print_optim(c);
  print_optim(d);
  print_optim(e);

  printf("===================================================\n");

  printf("Neg Optimizer Checks...\n");

  Expr *f = new_neg(new_num(7));
  Expr *g = new_neg(new_neg(new_add(new_num(-7), new_num(10))));
  Expr *h = new_neg(new_add(new_num(42), new_read()));

  print_optim(f);
  print_optim(g);
  print_optim(h);

  printf("===================================================\n");

  printf("R1 Specific Optimizer Checks...\n");

  Expr *x = new_var("x");
  Expr *z = new_var("z");
  Expr *l1 = new_let(x, new_num(7), new_add(x, x));
  Expr *l2 = new_let(x, new_num(42), new_neg(x));
  Expr *l3 = new_let(z, new_read(), new_neg(z));
  print_optim(l1);
  print_optim(l2);
  print_optim(l3);

  printf("===================================================\n");

  printf("General Optimizer Checks...\n");

  count = full_count = 0;
  for (size_t i = 0; i < NUM_PROGS; ++i) {
    rand_depth = rand() % 20;
    expr = randp(vars, rand_depth);
    res = interp(expr, NULL);
    if (DEBUG) {
      printf("Normal   : ");
      print(expr);
      printf(" -> %d\n", res);
    }
    expr_opt = optimize(expr);
    res_opt = interp(expr_opt, NULL);
    if (DEBUG) {
      printf("Optimized: ");
      print(expr_opt);
      printf("\n");
    }
    count += (res_opt == res);
    full_count += (res_opt == res && expr_opt->type == NUM);
  }

  printf("%sSuccessfully optimized %d/%d programs.%s\n", GRN, count, NUM_PROGS,
         NRM);

  printf("Fully optimized %d/%d\n", count, NUM_PROGS);
  assert(count == NUM_PROGS);

  printf("===================================================\n");

  printf("Testing a dozen R1 programs...\n");

  test_dozen_r1();

  printf("===================================================\n");

  return 0;
}
