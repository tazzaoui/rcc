#include <stdio.h>
#include <stdlib.h>

#include "src/rcc.h"

int main(int argc, char *argv[]) {
  Expr *f = new_var("f");
  Expr *n2 = new_num(502);
  Expr *let = new_let(f, n2, f);

  print(let);
  printf("\n");

  int val = interp(let, NULL);
  printf("VAL = %d\n", val);
  return 0;
}
