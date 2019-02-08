#ifndef PAIRS_H
#define PAIRS_H

#include "r.h"
#include "x.h"

typedef struct env_pair_t {
  Expr *var, *val;
} env_pair_t;

typedef struct lbl_blk_pair_t {
  label_t label;
  Block* block;
} lbl_blk_pair_t;

typedef struct reg_num_pair_t {
  REGISTER reg;
  int num;
} reg_num_pair_t;

typedef struct num_pair_t {
  int n1, n2;
} num_pair_t;

typedef struct var_num_pair_t {
  Arg_Var* var;
  int num;
} var_num_pair_t;

/* Return a new environment pair */
env_pair_t* new_env_pair(Expr*, Expr*);

/* Compare two Var -> int pairs */
int ep_var_cmp(void*, void*);

/* Compare two Var -> Expr pairs */
int ep_expr_cmp(void*, void*);

/* (shallow) copy an environment pair*/
void* ep_cpy(void* old);

/* Return a new label-block pair */
lbl_blk_pair_t* new_lbl_blk_pair(label_t, Block*);

/* Return new reg -> num pair */
reg_num_pair_t* new_reg_num_pair(REGISTER, int);

/* Return a new num -> num pair */
num_pair_t* new_num_pair(int, int);

/* Return new var -> num pair */
var_num_pair_t* new_var_num_pair(Arg_Var*, int);

/* Comparison function for reg->num pairs */
int reg_num_pair_cmp(void*, void*);

/* Copy function for reg->num pairs */
void* reg_num_pair_cpy(void*);

/* Printing function for reg->num pairs */
void print_reg_num_pair(void*);

/* Comparison function for num pairs */
int num_pair_cmp(void*, void*);

/* Copy function for num pairs*/
void* num_pair_cpy(void*);

/* Printing function for num pairs */
void print_num_pair(void*);

/* Comparison function for var->num pairs*/
int var_num_pair_cmp(void*, void*);

/* Copy function for var->num pairs */
void* var_num_pair_cpy(void*);

/* Printing function for var->num pairs*/
void print_var_num_pair(void*);

#endif /* PAIRS_H */
