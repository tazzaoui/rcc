#include <assert.h>
#include <string.h>
#include "econ.h"
#include "rcc.h"
#include "r.h"
#include "c.h"

C_Tail *kont(void *a) {
  return a;
}

C_Tail *econ_e_simple(R_Expr * expr, kont_func_t k, list_t l2t) {
  l2t = list_create();
  C_Tail *ct = econ_expr(expr);
  return k(ct);
}

C_Tail *econ_e_let(R_Expr * expr, kont_func_t k, list_t l2t) {
  assert(expr && expr->type == R_EXPR_LET);
  l2t = list_create();

  C_Expr *ct_expr = econ_cmplx(((R_Let *) expr->expr)->expr);
  C_Tail *ct_body = econ_e(((R_Let *) expr->expr)->body, k, l2t);

  const char *var = ((R_Var *) ((R_Let *) expr->expr)->var->expr)->name;
  C_Smt *x_smt = new_c_smt(new_c_var(var), ct_expr);
  return new_c_tail(C_TAIL_SEQ, new_c_seq(x_smt, ct_body));
}

C_Tail *econ_e_if(R_Expr * expr, kont_func_t k, list_t l2t) {

  assert(expr && expr->type == R_EXPR_IF);
  assert(((R_If *) expr->expr)->test_expr->type == R_EXPR_CMP);

  C_Arg *arg_l, *arg_r;
  C_Tail *econ_then, *econ_else;
  list_t l2t_then = list_create(), l2t_else = list_create();

  econ_then = econ_e(((R_If *) expr->expr)->then_expr, k, l2t_then);
  econ_else = econ_e(((R_If *) expr->expr)->else_expr, k, l2t_else);

  R_Cmp *cmp = ((R_If *) expr->expr)->test_expr->expr;
  arg_l = econ_arg(cmp->left);
  arg_r = econ_arg(cmp->right);

  label_t then_lbl = get_time_stamp();
  label_t else_lbl = get_time_stamp();

  assert(!strcmp(then_lbl, else_lbl));

  list_insert(l2t, new_lbl_tail_pair(then_lbl, econ_then));
  list_insert(l2t, new_lbl_tail_pair(else_lbl, econ_else));

  list_concat(l2t, list_concat(l2t_then, l2t_else));

  C_Expr *c_cmp = new_c_expr(C_CMP, new_c_cmp(cmp->cmp_type, arg_l, arg_r));
  C_Goto_If *c_gotoif = new_c_goto_if(c_cmp, then_lbl, else_lbl);

  return new_c_tail(C_TAIL_GOTO_IF, c_gotoif);
}

C_Tail *econ_e_non_tail_let(R_Expr *expr, kont_func_t k, list_t l2t){
    
}
