#ifndef PAIRS_H
#define PAIRS_H

#include "c.h"
#include "r.h"
#include "x.h"

typedef struct env_pair_t {
  R_Expr *var, *val;
} env_pair_t;

typedef struct c_var_num_pair_t {
  C_Var *var;
  int num;
} c_var_num_pair_t;

typedef struct lbl_blk_pair_t {
  label_t label;
  X_Block *block;
} lbl_blk_pair_t;

typedef struct reg_num_pair_t {
  REGISTER reg;
  int num;
} reg_num_pair_t;

typedef struct num_pair_t {
  int n1, n2;
} num_pair_t;

typedef struct var_num_pair_t {
  X_Arg_Var *var;
  int num;
} var_num_pair_t;

typedef struct lbl_tail_pair_t {
  label_t label;
  C_Tail *tail;
} lbl_tail_pair_t;

typedef struct r_var_int_pair_t {
  R_Var *var;
  int n;
} r_var_int_pair_t;

typedef struct r_var_var_pair_t {
  R_Var *v1, *v2;
} r_var_var_pair_t;

typedef struct x_arg_list_pair_t {
  X_Arg *arg;
  list_t list;
} x_arg_list_pair_t;

typedef struct x_instr_list_pair_t {
  X_Instr *xi;
  list_t live;
} x_instr_list_pair_t;

typedef struct x_arg_int_pair_t {
  X_Arg *arg;
  int num;
} x_arg_int_pair_t;

typedef struct x_arg_pair_t {
  X_Arg *arg1, *arg2;
} x_arg_pair_t;

typedef struct r_expr_type_pair_t {
  R_Expr *expr;
  R_EXPR_TYPE type;
} r_expr_type_pair_t;

/* Return a new R_Expr -> R_EXPR_TYPE pair */
r_expr_type_pair_t *new_r_expr_type_pair(R_Expr *, R_EXPR_TYPE);

/* Compare two R_Expr -> R_EXPR_TYPE pairs */
CMP r_expr_type_pair_cmp(void *, void *);

/* Comapare two R_Exprs */
CMP r_expr_cmp(R_Expr *, R_Expr *);

/* Return a new X_Arg -> X_Arg pairs */
x_arg_pair_t *new_x_arg_pair(X_Arg *, X_Arg *);

/* Compare two X_Arg -> X_Arg pairs */
CMP x_arg_pair_cmp(void *, void *);

/* Compare two arg pairs wrt the second arg */
CMP x_arg_pair_cmp2(void *, void *);

/* Print an X_Arg -> X_Arg pair */
void print_x_arg_pair(void *);

/* Return a new X_Arg -> int pair */
x_arg_int_pair_t *new_x_arg_int_pair(X_Arg *, int);

/* Compare two X_Arg -> int pairs */
CMP x_arg_int_pair_cmp(void *, void *);

/* Compare two X_Arg -> int pairs by value */
CMP x_arg_int_pair_cmp_by_int(void *, void *);

/* Print an x_arg_int_pair_t */
void print_x_arg_int_pair(void *);

/* Return a new X_Arg -> X_Arg pair */
x_arg_list_pair_t *new_x_arg_list_pair(X_Arg *, list_t);

/* Compare two X_Arg -> X_Arg pairs */
CMP x_arg_list_pair_cmp(void *, void *);

/* Print an x_arg_list_pair_t */
void print_x_arg_list_pair(void *);

/* Return a new X_Instr -> list_t pair */
x_instr_list_pair_t *new_x_instr_list_pair(X_Instr *, list_t);

/* Print an x_instr_lit_pair_t */
void print_x_instr_list_pair(void *);

/* Return a new r_var -> int pair  */
r_var_int_pair_t *new_r_var_int_pair(R_Var *, int);

/* Compare two r_var -> int pairs */
CMP r_var_var_pair_cmp(void *, void *);

/* Return a new r_var_int_pair_t -> r_var_int_pair_t pair */
r_var_var_pair_t *new_r_var_var_pair(R_Var *, R_Var *);

/* Print a r_var -> r_var pair */
void r_var_var_print(void *);

/* Deep copy an r_var_var pair */
void *r_var_var_cpy(void *old);

/* Compare two r_var -> r_var pairs */
CMP r_var_var_pair_cmp(void *, void *);

/* Return a new c_var -> num pair */
c_var_num_pair_t *new_c_var_num_pair(C_Var *, int);

/* Compare two c_var -> num pairs */
CMP c_var_num_pair_cmp(void *, void *);

/* Return a new lbl_tail_pair_t */
lbl_tail_pair_t *new_lbl_tail_pair(label_t, C_Tail *);

/* Compare two lbl -> tail pairs */
CMP lbl_tail_cmp(void *, void *);

/* Print a lbl -> tail pair */
void lbl_tail_print(void *);

/* Compare two env pairs */
CMP ep_cmp(void *, void *);

/* Print an env pair */
void ep_print(void *);

/* Return a new environment pair */
env_pair_t *new_env_pair(R_Expr *, R_Expr *);

/* Compare two Var -> int pairs */
CMP ep_var_cmp(void *, void *);

/* Compare two Var -> R_Expr pairs */
int ep_expr_cmp(void *, void *);

/* (shallow) copy an environment pair*/
void *ep_cpy(void *old);

/* Return a new label-block pair */
lbl_blk_pair_t *new_lbl_blk_pair(label_t, X_Block *);

/* Compare a label with a lbl_blk_pair_t pair */
CMP lbl_blk_pair_cmp(void *, void *);

/* Print the label portion of a lbl_blk_pair */
void print_lbl_blk_pair(void *);

/* Return new reg -> num pair */
reg_num_pair_t *new_reg_num_pair(REGISTER, int);

/* Return a new num -> num pair */
num_pair_t *new_num_pair(int, int);

/* Return new var -> num pair */
var_num_pair_t *new_var_num_pair(X_Arg_Var *, int);

/* Comparison function for reg->num pairs */
CMP reg_num_pair_cmp(void *, void *);

/* Copy function for reg->num pairs */
void *reg_num_pair_cpy(void *);

/* Printing function for reg->num pairs */
void print_reg_num_pair(void *);

/* Comparison function for num pairs */
CMP num_pair_cmp(void *, void *);

/* Copy function for num pairs*/
void *num_pair_cpy(void *);

/* Printing function for num pairs */
void print_num_pair(void *);

/* Comparison function for var->num pairs*/
CMP var_num_pair_cmp(void *, void *);

/* Copy function for var->num pairs */
void *var_num_pair_cpy(void *);

/* Printing function for var->num pairs*/
void print_var_num_pair(void *);

/* Compare two C_Vars */
CMP c_var_cmp(void *, void *);

/* Print a C_Var */
void c_var_print(void *);

/* Compare two X_Args */
CMP cmp_x_args(void *, void *);

/* Wraper around x_print_arg */
void x_print_arg_void(void *);

/* Print an instr list pair */
void print_x_instr_list_pair(void *);

/* Copy a single x arg */
void *copy_x_arg(void *);

/* Copy an x_arg_list_pair_t */
void *copy_x_arg_list_pair(void *);
#endif                          /* PAIRS_H */
