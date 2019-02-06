#ifndef RCC_H
#define RCC_H

#include <limits.h>
#include "list.h"
#include "utils.h"

#define RAND_RANGE 1024
#define GET_RAND() (rand() % (2 * RAND_RANGE)) - RAND_RANGE

typedef enum EXPR_TYPE { NEG, ADD, READ, NUM, VAR, LET } EXPR_TYPE;

typedef struct Expr {
  EXPR_TYPE type;
  void* expr;
} Expr;

typedef struct Program {
  void* info;
  Expr* expr;
} Program;

typedef struct Var {
  const char* name;
} Var;

typedef struct Let {
  Expr *var, *expr, *body;
} Let;

typedef struct Neg {
  Expr* expr;
} Neg;

typedef struct Add {
  Expr *left, *right;
} Add;

typedef struct Num {
  int num;
} Num;

typedef struct Read {
  int num, read;
} Read;

typedef struct env_pair_t {
  Expr *var, *val;
} env_pair_t;

/* Return a new program */
Program* new_prog(void*, Expr*);

/* Return a new expression */
Expr* new_expr(void*, EXPR_TYPE);

/* Return a new variable */
Expr* new_var(const char*);

/* Return a new let expression */
Expr* new_let(Expr*, Expr*, Expr*);

/* Return a new neg expression */
Expr* new_neg(Expr*);

/* Return a new add expression */
Expr* new_add(Expr*, Expr*);

/* Return a new num expression */
Expr* new_num(int);

/* Return a new read expression */
Expr* new_read(void);

/* Return a new environment pair */
env_pair_t* new_env_pair(Expr*, Expr*);

/* Compare two Var -> int pairs */
int ep_var_cmp(void*, void*);

/* Compare two Var -> Expr pairs */
int ep_expr_cmp(void*, void*);

/* (shallow) copy an environment pair*/
void* ep_cpy(void* old);

/* Interpret an expression in an environment*/
int interp(Expr*, list_t);

Expr* optimize(Expr*, list_t);

/* Optimize a neg expression */
Expr* optimize_neg(Expr*, list_t);

/* Optimize an add expression */
Expr* optimize_add(Expr*, list_t);

/* Print an expression to stdout */
void print(Expr*);

/* Returns 1 for simple expressions */
int is_simple(Expr*);

static inline Expr* get_left(Expr* expr) {
  if (expr != NULL && expr->type == ADD) return ((Add*)expr->expr)->left;
  return expr;
}

static inline Expr* get_right(Expr* expr) {
  if (expr != NULL && expr->type == ADD) return ((Add*)expr->expr)->right;
  return expr;
}

static inline int get_num(Expr* expr) {
  if (expr != NULL) {
    if (expr->type == NUM) return ((Num*)expr->expr)->num;
    if (expr->type == READ) return ((Read*)expr->expr)->num;
  }
  return INT_MIN;
}

static inline Expr* get_var(Expr* expr) {
  if (expr != NULL && expr->type == LET) return ((Let*)expr->expr)->var;
  return expr;
}

static inline Expr* get_expr(Expr* expr) {
  if (expr != NULL && expr->type == LET) return ((Let*)expr->expr)->expr;
  return expr;
}

static inline Expr* get_body(Expr* expr) {
  if (expr != NULL && expr->type == LET) return ((Let*)expr->expr)->body;
  return expr;
}

#endif /* RCC_H */
