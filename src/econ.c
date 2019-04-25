#include <assert.h>
#include <string.h>
#include "econ.h"
#include "rcc.h"
#include "r.h"
#include "c.h"

C_Tail *kont(void *fa, void *_) {
  return fa;
}

C_Tail *nkont(void *farg, void *ctail) {
  assert(farg && ctail);
  C_Expr *fa = (C_Expr *) farg;
  C_Tail *ct = (C_Tail *) ctail;

  assert(ct->type == C_TAIL_SEQ);

  C_Seq *cs = ct->tail;
  C_Smt *c_smt = cs->smt;

  // Replace the smt expr with the new one
  c_smt->expr = fa;
  return ct;
}

C_Tail *new_kont_tail(C_Var * cv, label_t suffixlab) {
  C_Tail *ct = new_c_tail(C_TAIL_GOTO, new_c_goto(suffixlab));
  C_Seq *cseq = new_c_seq(new_c_smt(cv, NULL), ct);
  return new_c_tail(C_TAIL_SEQ, cseq);
}

C_Tail *econ_e(R_Expr * expr, kont_func_t k, list_t l2t, C_Tail * ct) {
  R_Let *r_let;
  if (expr)
    switch (expr->type) {
      case R_EXPR_NUM:
      case R_EXPR_VAR:
      case R_EXPR_TRUE:
      case R_EXPR_FALSE:
        return econ_e_simple(expr, k, l2t, ct);
      case R_EXPR_IF:
        return econ_e_if(expr, k, l2t, ct);
      case R_EXPR_LET:
        r_let = expr->expr;
        if (r_let->expr->type == R_EXPR_IF)
          return econ_e_non_tail_let(expr, k, l2t, ct);
        return econ_e_let(expr, k, l2t, ct);
      default:
        break;
    };
  return NULL;
}

C_Tail *econ_e_simple(R_Expr * expr, kont_func_t k, list_t l2t, C_Tail * ct) {
  l2t = list_create();
  C_Tail *t = econ_expr(expr);
  return k(t, ct);
}

C_Tail *econ_e_let(R_Expr * expr, kont_func_t k, list_t l2t, C_Tail * ct) {
  assert(expr && expr->type == R_EXPR_LET);
  l2t = list_create();

  C_Expr *ct_expr = econ_cmplx(((R_Let *) expr->expr)->expr);
  C_Tail *ct_body = econ_e(((R_Let *) expr->expr)->body, k, l2t, ct);

  const char *var = ((R_Var *) ((R_Let *) expr->expr)->var->expr)->name;
  C_Smt *x_smt = new_c_smt(new_c_var(var), ct_expr);
  return new_c_tail(C_TAIL_SEQ, new_c_seq(x_smt, ct_body));
}

C_Tail *econ_e_if(R_Expr * expr, kont_func_t k, list_t l2t, C_Tail * ct) {

  assert(expr && expr->type == R_EXPR_IF);
  assert(((R_If *) expr->expr)->test_expr->type == R_EXPR_CMP);

  C_Arg *arg_l, *arg_r;
  C_Tail *econ_then, *econ_else;
  list_t l2t_then = list_create(), l2t_else = list_create();

  econ_then = econ_e(((R_If *) expr->expr)->then_expr, k, l2t_then, ct);
  econ_else = econ_e(((R_If *) expr->expr)->else_expr, k, l2t_else, ct);

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

C_Tail *econ_e_non_tail_let(R_Expr * expr, kont_func_t k, list_t l2t,
                            C_Tail * ct) {
  assert(expr && expr->type == R_EXPR_LET);

  R_Expr *let_expr = ((R_Let *) expr->expr)->expr;
  assert(let_expr->type == R_EXPR_IF);

  R_Cmp *cmp = ((R_If *) let_expr->expr)->test_expr->expr;
  C_Arg *arg_l = econ_arg(cmp->left);
  C_Arg *arg_r = econ_arg(cmp->right);

  list_t l2t_then = list_create(), l2t_else = list_create();
  list_t l2t_body = list_create(), new_lbls = list_create();

  label_t true_lbl = get_time_stamp();
  label_t false_lbl = get_time_stamp();
  label_t suffix_lbl = get_time_stamp();

  C_Var *cv = new_c_var(((R_Var *) ((R_Let *) expr->expr)->var->expr)->name);
  C_Tail *then_ct = new_kont_tail(cv, suffix_lbl);
  C_Tail *else_ct = new_kont_tail(cv, suffix_lbl);

  C_Tail *then_t =
    econ_e(((R_If *) let_expr->expr)->then_expr, nkont, l2t_then, then_ct);
  C_Tail *else_t =
    econ_e(((R_If *) let_expr->expr)->else_expr, nkont, l2t_else, else_ct);
  C_Tail *body_t = econ_e(((R_Let *) expr->expr)->body, k, l2t_body, ct);

  list_insert(l2t, new_lbl_tail_pair(true_lbl, then_t));
  list_insert(l2t, new_lbl_tail_pair(false_lbl, else_t));
  list_insert(l2t, new_lbl_tail_pair(suffix_lbl, body_t));

  list_concat(l2t_then, list_concat(l2t_else, list_concat(l2t_body, new_lbls)));
  list_concat(l2t, l2t_then);

  C_Cmp *new_cmp = new_c_cmp(cmp->cmp_type, arg_l, arg_r);
  C_Goto_If *cg =
    new_c_goto_if(new_c_expr(C_CMP, new_cmp), true_lbl, false_lbl);
  return new_c_tail(C_TAIL_GOTO_IF, cg);
}
