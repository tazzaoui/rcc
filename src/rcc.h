#ifndef RCC_H
#define RCC_H

#include "list.h"
#include "r.h"
#include "utils.h"
#include "x.h"

/* Interpret an expression in an environment*/
int interp(Expr*, list_t);

/* Optimize an arbitrary (valid) expression */
Expr* optimize(Expr*, list_t);

/* Optimize a neg expression */
Expr* optimize_neg(Expr*, list_t);

/* Optimize an add expression */
Expr* optimize_add(Expr*, list_t);

/* Print an expression to stdout */
void print(Expr*);

#endif /* RCC_H */
