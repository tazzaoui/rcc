#include <assert.h>
#include "pairs.h"
#include "rco.h"
#include "r.h"

int is_bin_op(R_Expr * expr) {
  if (expr)
    switch (expr->type) {
      case R_EXPR_ADD:
      case R_EXPR_AND:
      case R_EXPR_OR:
      case R_EXPR_CMP:
        return 1;
      default:
        break;
    }
  return 0;
}

R_Expr *rco_e(R_Expr * expr, list_t var2expr, int is_tail) {
  int n = 0;
  list_t new_vars = list_create();
  R_Expr *res = rco_a(expr, var2expr, is_tail, &new_vars, &n);
  return combine_lets(*new_vars, res);
}

R_Expr *rco_a(R_Expr * expr, list_t env, int is_tail,
              list_t * new_vars, int *rco_calls) {
  Node *n;
  R_Expr *r1;
  list_t l1 = list_create();

  if (expr)
    switch (expr->type) {
      case R_EXPR_VAR:
        *new_vars = l1;
        n = list_find(env, new_env_pair(expr, NULL), ep_cmp);
        if (!n)
          die("[rco_a] UNBOUND VAR!");
        return ((env_pair_t *) n->data)->val;
      case R_EXPR_READ:
        r1 = new_var(append_int("X", (*rco_calls)++));
        list_insert(l1, new_env_pair(r1, new_read()));
        *new_vars = l1;
        return r1;
      case R_EXPR_NUM:
      case R_EXPR_TRUE:
      case R_EXPR_FALSE:
        *new_vars = l1;
        return expr;
      case R_EXPR_NEG:
      case R_EXPR_NOT:
        return rco_a_unary(expr, env, 0, new_vars, rco_calls);
      case R_EXPR_OR:
      case R_EXPR_ADD:
      case R_EXPR_AND:
      case R_EXPR_CMP:
        return rco_a_binary(expr, env, is_tail, new_vars, rco_calls);
      case R_EXPR_LET:
        return rco_a_let(expr, env, is_tail, new_vars, rco_calls);
      case R_EXPR_IF:
        return rco_a_if(expr, env, is_tail, new_vars, rco_calls);
      default:
        break;
    };
  die("[RESOLVE_COMPLEX] NO RESOLUTION!");
  return NULL;
}

R_Expr *rco_a_unary(R_Expr * expr, list_t env, int is_tail, list_t * new_vars,
                    int *rco_calls) {
  R_Expr *res = NULL, *inner_expr, *var;

  if (expr && env && new_vars && rco_calls) {
    var = new_var(append_int("X", (*rco_calls)++));
    *new_vars = list_create();

    switch (expr->type) {
      case R_EXPR_NOT:
        inner_expr = ((R_Not *) expr->expr)->expr;
        res = new_not(rco_a(inner_expr, env, 0, new_vars, rco_calls));
        break;
      case R_EXPR_NEG:
        inner_expr = ((R_Neg *) expr->expr)->expr;
        res = new_neg(rco_a(inner_expr, env, 0, new_vars, rco_calls));
        break;
      default:
        die("[rco_a_unary] expected unary operator!");
    }

    list_insert(*new_vars, new_env_pair(var, res));

  }
  return var;
}

R_Expr *rco_a_binary(R_Expr * expr, list_t env, int is_tail, list_t * new_vars,
                     int *rco_calls) {
  R_Expr *res_l, *res_r, *res, *var = NULL;
  list_t l_vars = list_create(), r_vars = list_create();

  if (expr && env && new_vars && rco_calls) {
    res_l = rco_a(get_left(expr), env, is_tail, &l_vars, rco_calls);
    res_r = rco_a(get_right(expr), env, is_tail, &r_vars, rco_calls);

    switch (expr->type) {
      case R_EXPR_ADD:
        res = new_add(res_l, res_r);
        break;
      case R_EXPR_AND:
        res = new_and(res_l, res_r);
        break;
      case R_EXPR_OR:
        res = new_or(res_l, res_r);
        break;
      case R_EXPR_CMP:
        res = new_cmp(((R_Cmp *) expr->expr)->cmp_type, res_l, res_r);
        break;
      default:
        die("[rco_a_binary] expected binary operator!");
    };

    *new_vars = list_concat(l_vars, r_vars);
    var = new_var(append_int("X", (*rco_calls)++));
    list_insert(*new_vars, new_env_pair(var, res));
  }

  return var;
}

R_Expr *rco_a_let(R_Expr * expr, list_t env, int is_tail, list_t * new_vars,
                  int *rco_calls) {
  list_t expr_vars, new_env;
  R_Expr *res_expr, *res_body = NULL;


  if (expr && env && new_vars && rco_calls) {
    res_expr =
      rco_a(((R_Let *) expr->expr)->expr, env, 0, &expr_vars, rco_calls);

    new_env = list_copy(env, ep_cpy);
    list_insert(new_env, new_env_pair(((R_Let *) expr->expr)->var, res_expr));

    res_body = rco_a(((R_Let *) expr->expr)->body, new_env, is_tail,
                     &expr_vars, rco_calls);

    *new_vars = list_concat(expr_vars, expr_vars);
  }


  return res_body;
}

R_Expr *rco_a_if(R_Expr * expr, list_t env, int is_tail, list_t * new_vars,
                 int *rco_calls) {

  if (expr && env && new_vars && rco_calls) {
    assert(expr->type == R_EXPR_IF);
    R_Expr *if_res, *if_var, *e_then, *e_else, *cmp;
    cmp = rco_c(((R_If *) expr->expr)->test_expr, env, new_vars, rco_calls);
    e_then = rco_e(((R_If *) expr->expr)->then_expr, env, is_tail);
    e_else = rco_e(((R_If *) expr->expr)->else_expr, env, is_tail);
    if_res = new_if(cmp, e_then, e_else);
    if (is_tail)
      return if_res;
    else {
      if_var = new_var(append_int("X", (*rco_calls)++));
      list_insert(*new_vars, new_env_pair(if_var, if_res));
      return if_var;
    }
  }
  return expr;
}

R_Expr *rco_c(R_Expr * expr, list_t env, list_t * new_vars, int *rco_calls) {
  if (is_bin_op(expr))
    return rco_c_bin(expr, env, new_vars, rco_calls);
  else if (expr->type == R_EXPR_LET)
    return rco_c_let(expr, env, new_vars, rco_calls);
  else
    return rco_c_other(expr, env, new_vars, rco_calls);
}

R_Expr *rco_c_bin(R_Expr * expr, list_t env, list_t * new_vars, int *rco_calls) {
  list_t nvl, nvr;
  R_Expr *el, *er, *res = NULL;

  el = rco_a(get_left(expr), env, 0, &nvl, rco_calls);
  er = rco_a(get_right(expr), env, 0, &nvr, rco_calls);

  *new_vars = list_concat(nvl, nvr);

  switch (expr->type) {
    case R_EXPR_ADD:
      res = new_and(el, er);
      break;
    case R_EXPR_AND:
      res = new_and(el, er);
      break;
    case R_EXPR_OR:
      res = new_or(el, er);
      break;
    case R_EXPR_CMP:
      res = new_cmp(((R_Cmp *) expr->expr)->cmp_type, el, er);
      break;
    default:
      break;
  }

  return res;
}

R_Expr *rco_c_let(R_Expr * expr, list_t env, list_t * new_vars, int *rco_calls) {
  list_t nv_var, nv_bin, new_env;
  R_Expr *e_var, *e_body;

  e_var = rco_a(((R_Let *) expr->expr)->expr, env, 0, &nv_var, rco_calls);

  new_env = list_copy(env, ep_cpy);
  list_insert(new_env, new_env_pair(((R_Let *) expr->expr)->var, e_var));

  e_body = rco_c(((R_Let *) expr->expr)->body, new_env, &nv_bin, rco_calls);

  *new_vars = list_concat(nv_var, nv_bin);
  return e_body;
}

R_Expr *rco_c_other(R_Expr * expr, list_t env, list_t * new_vars,
                    int *rco_calls) {
  R_Expr *res;
  res = rco_a(expr, env, 0, new_vars, rco_calls);
  return new_cmp(R_CMP_EQUAL, new_true(), res);
}
