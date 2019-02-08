#ifndef PAIRS_H
#define PAIRS_H

#include "r.h"
#include "x.h"

typedef struct env_pair_t {
  Expr *var, *val;
} env_pair_t;

/* Return a new environment pair */
env_pair_t* new_env_pair(Expr*, Expr*);

/* Compare two Var -> int pairs */
int ep_var_cmp(void*, void*);

/* Compare two Var -> Expr pairs */
int ep_expr_cmp(void*, void*);

/* (shallow) copy an environment pair*/
void* ep_cpy(void* old);

typedef struct lbl_blk_pair_t {
  label_t label;
  Block* block;
} lbl_blk_pair_t;

/* Return a new label-block pair */
lbl_blk_pair_t* new_lbl_blk_pair(label_t, Block*);

#endif /* PAIRS_H */
