#include <assert.h>
#include "utils.h"
#include "r.h"

Expr* new_expr(void* expr, EXPR_TYPE type) {
  Expr* e = malloc_or_die(sizeof(Expr));
  e->expr = expr;
  e->type = type;
  return e;
}

Expr* new_var(const char* name) {
  Var* v = malloc_or_die(sizeof(Var));
  v->name = name;
  return new_expr(v, VAR);
}

Expr* new_let(Expr* var, Expr* expr, Expr* body) {
  assert(var->type == VAR);
  Let* l = malloc_or_die(sizeof(Let));
  l->var = var;
  l->expr = expr;
  l->body = body;
  return new_expr(l, LET);
}

Expr* new_neg(Expr* expr) {
  Neg* n = malloc_or_die(sizeof(Neg));
  n->expr = expr;
  return new_expr(n, NEG);
}

Expr* new_add(Expr* left, Expr* right) {
  Add* a = malloc_or_die(sizeof(Add));
  a->left = left;
  a->right = right;
  return new_expr(a, ADD);
}

Expr* new_num(int num) {
  Num* n = malloc_or_die(sizeof(Num));
  n->num = num;
  return new_expr(n, NUM);
}

Expr* new_read() {
  Read* r = malloc_or_die(sizeof(Read));
  r->num = QUIET_READ ? GET_RAND() : 0;
  r->read = QUIET_READ;
  return new_expr(r, READ);
}
