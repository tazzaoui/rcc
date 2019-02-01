#ifndef RCC_H
#define RCC_H

#include <limits.h>

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

Program* new_prog(void*, Expr*);
Expr* new_expr(void*, EXPR_TYPE);
Expr* new_var(const char*);
Expr* new_let(Expr*, Expr*, Expr*);
Expr* new_neg(Expr*);
Expr* new_add(Expr*, Expr*);
Expr* new_num(int);
Expr* new_read(void);
int interp(Expr*);
Expr* optimize(Expr*);
Expr* optimize_neg(Expr*);
Expr* optimize_add(Expr*);
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
