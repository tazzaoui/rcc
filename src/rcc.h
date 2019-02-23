#ifndef RCC_H
#define RCC_H

#include "c.h"
#include "list.h"
#include "r.h"
#include "utils.h"
#include "x.h"

/* Uniquify pass */
R_Expr* uniquify(R_Expr*, list_t, int*);

/* RCO Pass */
R_Expr* rco(R_Expr*, list_t*);

C_Tail* econ_expr(R_Expr*);

/* Turn an R_Arg to a C_Arg */
C_Arg* econ_arg(R_Expr*);

C_Expr* econ_cmplx(R_Expr*);

/* Explicate Control Pass */
C_Expr* explicate_control(R_Expr*, list_t*);

/* Combine simple exprs into nested lets */
R_Expr* combine_lets(Node*, R_Expr*);

/* Resolve complex expressions */
R_Expr* resolve_complex_expr(R_Expr*, list_t, list_t*, int*);

/* Rename an R_Var */
R_Expr* rename_var(R_Expr*);

/* Helper function to append an int to a c_str */
char* append_int(const char*, int);

/* Wrapper around uniquify */
static inline R_Expr* unique(R_Expr* e) {
  int n = 0;
  return uniquify(e, list_create(), &n);
}

#endif /* RCC_H */
