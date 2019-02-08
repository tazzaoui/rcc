#ifndef RCC_H
#define RCC_H

#include "list.h"
#include "r.h"
#include "utils.h"
#include "x.h"

typedef struct env_pair_t {
  Expr *var, *val;
} env_pair_t;

typedef struct lbl_blk_pair_t {
  label_t label;
  Block* block;
} lbl_blk_pair_t;

/* Return a new environment pair */
env_pair_t* new_env_pair(Expr*, Expr*);

/* Return a new label-block pair */
lbl_blk_pair_t* new_lbl_blk_pair(label_t, Block*);

/* Compare two Var -> int pairs */
int ep_var_cmp(void*, void*);

/* Compare two Var -> Expr pairs */
int ep_expr_cmp(void*, void*);

/* (shallow) copy an environment pair*/
void* ep_cpy(void* old);

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
