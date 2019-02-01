#include <stdio.h>
#include <stdlib.h>
#include "../rcc.h"
#include "../utils.h"

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
