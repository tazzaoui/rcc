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

  //(let h := (if false 1062676757 1350010754) in (let t := false in h))

  R_Expr *test = new_let(new_var("h"),
                         new_if(new_false(), new_num(1062676757),
                                new_num(1350010754)),
                         new_let(new_var("t"), new_false(), new_var("h")));
  r_print_expr(test);
  r_print_type(r_type_check(test, NULL));


  //(let q := (if false true true) in (if q 1377512886 809959203))

  R_Expr *q = new_if(new_false(), new_true(), new_true());
  R_Expr *q2 = new_if(new_var("q"), new_num(1377512886), new_num(809959203));
  R_Expr *q3 = new_let(new_var("q"), q, q2);
  r_print_expr(q3);
  printf("\n\n");
  r_print_type(r_type_check(q3, NULL));
  printf("\n\n");
  R_Expr *res2 = r_interp(q3, NULL);
  r_print_type(r_type_check(res2, NULL));


  // (if (let f := true in f) (let f := 1335084035 in f) (if true 720609117 1523680320))
  R_Expr *n1 = new_num(1335084035);
  R_Expr *n4 = new_num(720609117);
  R_Expr *n3 = new_num(720609117);

  R_Expr *let1 = new_let(f, new_true(), f);
  R_Expr *let2 = new_let(f, n1, f);
  R_Expr *if1 = new_if(new_true(), n4, n3);

  R_Expr *if2 = new_if(let1, let2, if1);
  r_print_expr(if2);
  printf(" -> ");
  R_TYPE type = r_type_check(if2, NULL);
  r_print_type(type);
  printf("\n");

  // (let i := (let w := (let i := 1306792961 in i) in (> 993769250 w)) in 
  // (> (let i := 765926119 in i) (if true 1085112599 1434772707)))
  R_Expr *i = new_var("i");
  R_Expr *w = new_var("w");
  R_Expr *a = new_if(new_true(), new_num(1085112599), new_num(1085112599));
  R_Expr *b = new_let(i, new_num(765926119), i);
  R_Expr *c = new_cmp(R_CMP_GREATER, b, a);
  R_Expr *d = new_cmp(R_CMP_GREATER, new_num(993769250), w);
  R_Expr *e = new_let(i, new_num(1306792961), i);
  R_Expr *g = new_let(w, e, d);
  r_print_type(r_type_check(c, NULL));
  R_Expr *h = new_let(i, g, c);
  r_print_expr(h);
  r_print_type(r_type_check(h, NULL));
  return 0;
}
