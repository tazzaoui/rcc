#ifndef RCC_H
#define RCC_H

#include "list.h"
#include "r.h"
#include "utils.h"
#include "x.h"

/* Uniquify pass */
R_Expr* uniquify(R_Expr*, list_t, int*);

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
