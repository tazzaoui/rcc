#include <stdio.h>
#include <stdlib.h>

#include "src/rcc.h"

int main(int argc, char *argv[]) {
  R_Expr *f = new_var("f");
  R_Expr *n2 = new_num(502);
  R_Expr *let = new_let(f, n2, f);

  r_print_expr(let);
  printf("\n");

  R_Expr *res = r_interp(let, NULL);
  int val = get_int(res);
  printf("VAL = %d\n", val);

  return 0;
}
