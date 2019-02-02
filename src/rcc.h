#ifndef RCC_H
#define RCC_H

#include <limits.h>
#include "list.h"

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
env_pair_t* new_env_pair(Expr*, int);

/* Compare two environment pairs */
int ep_cmp(void*, void*);

/* (shallow) copy an environment pair*/
void* ep_cpy(void* old);

/* Interpret an expression in an environment*/
int interp(Expr*, Node*);

/* Optimize an expression */
Expr* optimize(Expr*);

/* Optimize a neg expression */
Expr* optimize_neg(Expr*);

/* Optimize an add expression */
Expr* optimize_add(Expr*);

/* Print an expression to stdout */
void print(Expr*);

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
#endif /* RCC_H */
