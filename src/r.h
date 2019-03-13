#ifndef R_H
#define R_H

#include "list.h"
#include "utils.h"

typedef enum R_EXPR_TYPE {
  R_EXPR_NEG,
  R_EXPR_ADD,
  R_EXPR_READ,
  R_EXPR_NUM,
  R_EXPR_VAR,
  R_EXPR_LET,
  R_EXPR_TRUE,
  R_EXPR_FALSE,
  R_EXPR_AND,
  R_EXPR_OR,
  R_EXPR_NOT,
  R_EXPR_CMP,
  R_EXPR_IF
} R_EXPR_TYPE;

typedef enum R_CMP_TYPE {
  R_CMP_EQUAL,
  R_CMP_LESS,
  R_CMP_LEQ,
  R_CMP_GEQ,
  R_CMP_GREATER
} R_CMP_TYPE;

typedef enum R_TYPE { R_TYPE_S64, R_TYPE_BOOL } R_TYPE;

typedef struct R_Expr {
  R_EXPR_TYPE type;
  void* expr;
} R_Expr;

typedef struct R_Let {
  R_Expr *var, *expr, *body;
} R_Let;

typedef struct R_Neg {
  R_Expr* expr;
} R_Neg;

typedef struct R_Add {
  R_Expr *left, *right;
} R_Add;

typedef struct R_Num {
  int num;
} R_Num;

typedef struct R_Read {
  int num, read;
} R_Read;

typedef struct R_Var {
  const char* name;
} R_Var;

typedef struct R_True {
  int val;
} R_True;

typedef struct R_False {
  int val;
} R_False;

typedef struct R_And {
  R_Expr *left, *right;
} R_And;

typedef struct R_Or {
  R_Expr *left, *right;
} R_Or;

typedef struct R_Not {
  R_Expr* expr;
} R_Not;

typedef struct R_Cmp {
  R_CMP_TYPE cmp_type;
  R_Expr *left, *right;
} R_Cmp;

typedef struct R_If {
  R_Expr *test_expr, *then_expr, *else_expr;
} R_If;

/* Return a new expression */
R_Expr* new_expr(void*, R_EXPR_TYPE);

/* Return a new let expression */
R_Expr* new_let(R_Expr*, R_Expr*, R_Expr*);

/* Return a new neg expression */
R_Expr* new_neg(R_Expr*);

/* Return a new add expression */
R_Expr* new_add(R_Expr*, R_Expr*);

/* Return a new num expression */
R_Expr* new_num(int);

/* Return a new read expression */
R_Expr* new_read(void);

/* Return a new variable */
R_Expr* new_var(const char*);

/* Return a new negative addition expression */
R_Expr* new_sub(R_Expr*, R_Expr*);

/* Return a new true expression */
R_Expr* new_true(void);

/* Return a new false expression */
R_Expr* new_false(void);

/* Return a new boolean and expression */
R_Expr* new_and(R_Expr*, R_Expr*);

/* Return a new boolean or expression */
R_Expr* new_or(R_Expr*, R_Expr*);

/* Return a new boolean not expression */
R_Expr* new_not(R_Expr*);

/* Return a new comparison expression */
R_Expr* new_cmp(R_CMP_TYPE, R_Expr*, R_Expr*);

/* Return a new if expression */
R_Expr* new_if(R_Expr*, R_Expr*, R_Expr*);

/* Interpret an expression in an environment*/
int r_interp(R_Expr*, list_t);

/* Optimize an arbitrary (valid) expression */
R_Expr* r_optimize(R_Expr*, list_t);

/* Optimize a neg expression */
R_Expr* r_optimize_neg(R_Expr*, list_t);

/* Optimize an add expression */
R_Expr* r_optimize_add(R_Expr*, list_t);

/* Print an expression to stdout */
void r_print_expr(R_Expr*);

static inline int is_simple(R_Expr* e) {
  return e && (e->type == R_EXPR_NUM || e->type == R_EXPR_VAR);
}

static inline R_Expr* get_left(R_Expr* expr) {
  if (expr && expr->type == R_EXPR_ADD) return ((R_Add*)expr->expr)->left;
  return expr;
}

static inline R_Expr* get_right(R_Expr* expr) {
  if (expr && expr->type == R_EXPR_ADD) return ((R_Add*)expr->expr)->right;
  return expr;
}

static inline int get_num(R_Expr* expr) {
  if (expr && expr->type == R_EXPR_NUM) return ((R_Num*)expr->expr)->num;
  if (expr && expr->type == R_EXPR_READ) return ((R_Read*)expr->expr)->num;
  return I32MIN;
}

static inline R_Expr* get_var(R_Expr* expr) {
  if (expr && expr->type == R_EXPR_LET) return ((R_Let*)expr->expr)->var;
  return expr;
}

static inline R_Expr* get_expr(R_Expr* expr) {
  if (expr && expr->type == R_EXPR_LET) return ((R_Let*)expr->expr)->expr;
  return expr;
}

static inline R_Expr* get_body(R_Expr* expr) {
  if (expr && expr->type == R_EXPR_LET) return ((R_Let*)expr->expr)->body;
  return expr;
}

#endif /* R_H */
