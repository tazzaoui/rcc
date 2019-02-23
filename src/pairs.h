#ifndef PAIRS_H
#define PAIRS_H

#include "c.h"
#include "r.h"
#include "x.h"

typedef struct env_pair_t {
  R_Expr *var, *val;
} env_pair_t;

typedef struct c_var_num_pair_t {
  C_Var* var;
  int num;
} c_var_num_pair_t;

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

typedef struct lbl_tail_pair_t {
  label_t label;
  C_Tail* tail;
} lbl_tail_pair_t;

typedef struct r_var_int_pair_t {
  R_Var* var;
  int n;
} r_var_int_pair_t;

typedef struct r_var_var_pair_t {
  R_Var *v1, *v2;
} r_var_var_pair_t;

/* Return a new r_var -> int pair  */
r_var_int_pair_t* new_r_var_int_pair(R_Var*, int);

/* Compare two r_var -> int pairs */
int r_var_var_pair_cmp(void*, void*);

/* Return a new r_var_int_pair_t -> r_var_int_pair_t pair */
r_var_var_pair_t* new_r_var_var_pair(R_Var*, R_Var*);

/* Print a r_var -> r_var pair */
void r_var_var_print(void*);

/* Deep copy an r_var_var pair */
void* r_var_var_cpy(void* old);

/* Compare two r_var -> r_var pairs */
int r_var_var_pair_cmp(void*, void*);

/* Return a new c_var -> num pair */
c_var_num_pair_t* new_c_var_num_pair(C_Var*, int);

/* Compare two c_var -> num pairs */
int c_var_num_pair_cmp(void*, void*);

/* Return a new lbl_tail_pair_t */
lbl_tail_pair_t* new_lbl_tail_pair(label_t, C_Tail*);

/* Compare two lbl -> tail pairs */
int lbl_tail_cmp(void*, void*);

/* Print a lbl -> tail pair */
void lbl_tail_print(void*);

/* Compare two env pairs */
int ep_cmp(void*, void*);

/* Print an env pair */
void ep_print(void*);

/* Return a new environment pair */
env_pair_t* new_env_pair(R_Expr*, R_Expr*);

/* Compare two Var -> int pairs */
int ep_var_cmp(void*, void*);

/* Compare two Var -> R_Expr pairs */
int ep_expr_cmp(void*, void*);

/* (shallow) copy an environment pair*/
void* ep_cpy(void* old);

/* Return a new label-block pair */
lbl_blk_pair_t* new_lbl_blk_pair(label_t, Block*);

/* Compare a label with a lbl_blk_pair_t pair */
int lbl_blk_pair_cmp(void*, void*);

/* Print the label portion of a lbl_blk_pair */
void print_lbl_blk_pair(void*);

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
