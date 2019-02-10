#ifndef R_H
#define R_H

#define QUIET_READ 1

typedef enum EXPR_TYPE { NEG, ADD, READ, NUM, VAR, LET } EXPR_TYPE;

typedef struct Expr {
  EXPR_TYPE type;
  void* expr;
} Expr;

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

typedef struct Var {
  const char* name;
} Var;

/* Return a new expression */
Expr* new_expr(void*, EXPR_TYPE);

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

/* Return a new variable */
Expr* new_var(const char*);

static inline int is_simple(Expr* e) {
  return e && (e->type == NUM || e->type == VAR);
}

static inline Expr* get_left(Expr* expr) {
  if (expr && expr->type == ADD) return ((Add*)expr->expr)->left;
  return expr;
}

static inline Expr* get_right(Expr* expr) {
  if (expr && expr->type == ADD) return ((Add*)expr->expr)->right;
  return expr;
}

static inline int get_num(Expr* expr) {
  if (expr && expr->type == NUM) return ((Num*)expr->expr)->num;
  if (expr && expr->type == READ) return ((Read*)expr->expr)->num;
  return I32MIN;
}

static inline Expr* get_var(Expr* expr) {
  if (expr && expr->type == LET) return ((Let*)expr->expr)->var;
  return expr;
}

static inline Expr* get_expr(Expr* expr) {
  if (expr && expr->type == LET) return ((Let*)expr->expr)->expr;
  return expr;
}

static inline Expr* get_body(Expr* expr) {
  if (expr && expr->type == LET) return ((Let*)expr->expr)->body;
  return expr;
}

#endif /* R_H */
