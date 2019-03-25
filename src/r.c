#include <assert.h>
#include "utils.h"
#include "pairs.h"
#include "list.h"
#include "r.h"

R_Expr *new_expr(void *expr, R_EXPR_TYPE type) {
  R_Expr *e = malloc_or_die(sizeof(R_Expr));
  e->expr = expr;
  e->type = type;
  return e;
}

R_Expr *new_var(const char *name) {
  R_Var *v = malloc_or_die(sizeof(R_Var));
  v->name = name;
  return new_expr(v, R_EXPR_VAR);
}

R_Expr *new_let(R_Expr * var, R_Expr * expr, R_Expr * body) {
  assert(var->type == R_EXPR_VAR);
  R_Let *l = malloc_or_die(sizeof(R_Let));
  l->var = var;
  l->expr = expr;
  l->body = body;
  return new_expr(l, R_EXPR_LET);
}

R_Expr *new_neg(R_Expr * expr) {
  R_Neg *n = malloc_or_die(sizeof(R_Neg));
  n->expr = expr;
  return new_expr(n, R_EXPR_NEG);
}

R_Expr *new_add(R_Expr * left, R_Expr * right) {
  R_Add *a = malloc_or_die(sizeof(R_Add));
  a->left = left;
  a->right = right;
  return new_expr(a, R_EXPR_ADD);
}

R_Expr *new_num(int num) {
  R_Num *n = malloc_or_die(sizeof(R_Num));
  n->num = num;
  return new_expr(n, R_EXPR_NUM);
}

R_Expr *new_read() {
  R_Read *r = malloc_or_die(sizeof(R_Read));
  r->num = QUIET_READ ? 7 : 0;
  r->read = QUIET_READ;
  return new_expr(r, R_EXPR_READ);
}

R_Expr *new_sub(R_Expr * left, R_Expr * right) {
  return new_add(left, new_neg(right));
}

R_Expr *new_true() {
  R_True *r = malloc_or_die(sizeof(R_True));
  r->val = 1;
  return new_expr(r, R_EXPR_TRUE);
}

R_Expr *new_false() {
  R_False *r = malloc_or_die(sizeof(R_True));
  r->val = 0;
  return new_expr(r, R_EXPR_FALSE);
}

R_Expr *new_and(R_Expr * left, R_Expr * right) {
  return new_if(left, right, new_false());
}

R_Expr *new_or(R_Expr * left, R_Expr * right) {
  return new_if(left, new_true(), right);
}

R_Expr *new_not(R_Expr * expr) {
  R_Not *n = malloc_or_die(sizeof(R_Not));
  n->expr = expr;
  return new_expr(n, R_EXPR_NOT);
}

R_Expr *new_cmp(R_CMP_TYPE cmp_type, R_Expr * left, R_Expr * right) {
  R_Cmp *r = malloc_or_die(sizeof(R_Expr));
  r->cmp_type = cmp_type;
  r->left = left;
  r->right = right;
  return new_expr(r, R_EXPR_CMP);
}

R_Expr *new_if(R_Expr * test_expr, R_Expr * then_expr, R_Expr * else_expr) {
  R_If *r = malloc_or_die(sizeof(R_If));
  r->test_expr = test_expr;
  r->then_expr = then_expr;
  r->else_expr = else_expr;
  return new_expr(r, R_EXPR_IF);
}

R_Expr *r_interp(R_Expr * expr, list_t env) {
  if (env == NULL)
    env = list_create();
  Node *n;
  int l_val, r_val;
  R_Expr *res, *right;
  env_pair_t *ep;
  list_t new_env;

  if (expr != NULL)
    switch (expr->type) {
      case R_EXPR_TRUE:
      case R_EXPR_FALSE:
      case R_EXPR_NUM:
        return expr;
      case R_EXPR_VAR:
        n = list_find(env, expr, ep_var_cmp);
        if (n == NULL)
          die("Unbound R_Variable!\n");
        ep = (env_pair_t *) n->data;
        return get_num(ep->val);
      case R_EXPR_LET:
        res = r_interp(get_expr(expr), env);
        ep = new_env_pair(get_var(expr), res);
        new_env = list_copy(env, ep_cpy);
        n = list_find(env, ep, ep_cmp);
        if (n == NULL)
          list_insert(new_env, ep);
        else
          list_update(new_env, new_env_pair(get_var(expr), 0), ep, ep_cmp);
        return r_interp(get_body(expr), new_env);
      case R_EXPR_NEG:
        res = r_interp(((R_Neg *) expr->expr)->expr, env);
        return new_num(-1 * get_int(res));
      case R_EXPR_ADD:
        l_val = get_int(r_interp(get_left(expr), env));
        r_val = get_int(r_interp(get_right(expr), env));
        return new_num(l_val + r_val);
      case R_EXPR_READ:
        if (!QUIET_READ) {
          printf("read> ");
          scanf("%d", &(((R_Read *) expr->expr)->num));
        }
        return get_num(expr);
      case R_EXPR_NOT:
        res = r_interp(((R_Not *) expr->expr)->expr, env);
        return res->type == R_EXPR_TRUE ? new_false() : new_true();
      case R_EXPR_IF:
        res = r_interp(((R_If *) expr->expr)->test_expr, env);
        if (res->type == R_EXPR_TRUE)
          return r_interp(((R_If *) expr->expr)->then_expr, env);
        else
          return r_interp(((R_If *) expr->expr)->else_expr, env);
      case R_EXPR_CMP:
        res = r_interp(((R_Cmp *) expr->expr)->left, env);
        right = r_interp(((R_Cmp *) expr->expr)->right, env);
        l_val = ((R_Num *) res->expr)->num;
        r_val = ((R_Num *) right->expr)->num;
        switch (((R_Cmp *) expr->expr)->cmp_type) {
          case R_CMP_EQUAL:
            return l_val == r_val ? new_true() : new_false();
          case R_CMP_LESS:
            return l_val < r_val ? new_true() : new_false();
          case R_CMP_GREATER:
            return l_val > r_val ? new_true() : new_false();
          case R_CMP_LEQ:
            return l_val <= r_val ? new_true() : new_false();
          case R_CMP_GEQ:
            return l_val >= r_val ? new_true() : new_false();
        }
      default:
        break;
    }
  die("Invalid Program!");
  return NULL;
}

R_Expr *r_optimize_neg(R_Expr * neg, list_t env) {
  if (neg != NULL && neg->type == R_EXPR_NEG) {
    R_Expr *neg_left, *neg_right, *res;

    res = r_optimize(((R_Neg *) neg->expr)->expr, env);

    if (res->type == R_EXPR_NUM)
      return new_num(-1 * get_int(res));

    else if (res->type == R_EXPR_NEG)
      return r_optimize(((R_Neg *) res->expr)->expr, env);

    else if (res->type == R_EXPR_ADD) {
      neg_left = new_neg(get_left(res));
      neg_right = new_neg(get_right(res));
      return new_add(neg_left, neg_right);
    }
  }
  return neg;
}

R_Expr *r_optimize_add(R_Expr * add, list_t env) {
  if (add != NULL && add->type == R_EXPR_ADD) {
    R_Expr *left_opt = r_optimize(get_left(add), env);
    R_Expr *right_opt = r_optimize(get_right(add), env);
    R_Expr *left, *right, *res_num;

    /* (+ R_EXPR_NUM NUM) -> NUM+NUM */
    if (left_opt->type == R_EXPR_NUM && right_opt->type == R_EXPR_NUM)
      return new_num(get_int(left_opt) + get_int(right_opt));

    /* Left is a num, right is another add */
    if (left_opt->type == R_EXPR_NUM && right_opt->type == R_EXPR_ADD) {
      left = get_left(right_opt);
      right = get_right(right_opt);

      /* (+ R_EXPR_NUM  (+(NUM X))) -> (+ NUM+NUM X) */
      if (left->type == R_EXPR_NUM) {
        res_num = new_num(get_int(left_opt) + get_int(left));
        return new_add(res_num, right);
      }

      /* (+ R_EXPR_NUM  (+(X NUM))) -> (+ NUM+NUM X) */
      if (right->type == R_EXPR_NUM) {
        res_num = new_num(get_int(left_opt) + get_int(right));
        return new_add(res_num, left);
      }
    }

    /* Right is a num, left is another add */
    if (right_opt->type == R_EXPR_NUM && left_opt->type == R_EXPR_ADD) {
      left = get_left(left_opt);
      right = get_right(left_opt);

      /* (+ R_EXPR_NUM  (+(NUM X))) -> (+ NUM+NUM X) */
      if (left->type == R_EXPR_NUM) {
        res_num = new_num(get_int(right_opt) + get_int(left));
        return new_add(res_num, right);
      }

      /* (+ R_EXPR_NUM  (+(X NUM))) -> (+ NUM+NUM X) */
      if (right->type == R_EXPR_NUM) {
        res_num = new_num(get_int(right_opt) + get_int(right));
        return new_add(res_num, left);
      }
    }
  }
  return add;
}

R_Expr *r_optimize(R_Expr * expr, list_t env) {
  env_pair_t *ep;
  list_t new_env;
  R_Expr *tmp, *e;
  Node *node;
  int n;
  if (env == NULL)
    env = list_create();
  if (expr != NULL)
    switch (expr->type) {
      case R_EXPR_VAR:
        node = list_find(env, expr, ep_var_cmp);
        if (node == NULL)
          die("[OPT] Unbound R_Variable!");
        return ((env_pair_t *) node->data)->val;
      case R_EXPR_LET:
        tmp = r_optimize(((R_Let *) expr->expr)->expr, env);
        new_env = list_copy(env, ep_cpy);
        if (is_simple(tmp)) {
          ep = new_env_pair(((R_Let *) expr->expr)->var, tmp);
          node = list_find(env, ep, ep_cmp);
          if (node == NULL)
            list_insert(new_env, ep);
          else
            list_update(new_env, new_env_pair(((R_Let *) expr->expr)->var, 0),
                        ep, ep_cmp);
          return r_optimize(((R_Let *) expr->expr)->body, new_env);
        } else {
          ep =
            new_env_pair(((R_Let *) expr->expr)->var,
                         ((R_Let *) expr->expr)->body);
          node = list_find(env, ep, ep_cmp);
          if (node == NULL)
            list_insert(new_env, ep);
          else
            list_update(new_env, new_env_pair(((R_Let *) expr->expr)->var, 0),
                        ep, ep_cmp);
          e = r_optimize(((R_Let *) expr->expr)->body, new_env);
          return new_let(((R_Let *) expr->expr)->var, tmp, e);
        }
        return expr;
      case R_EXPR_NEG:
        return r_optimize_neg(expr, env);
      case R_EXPR_ADD:
        return r_optimize_add(expr, env);
      case R_EXPR_NUM:
        return expr;
      case R_EXPR_READ:
        if (((R_Read *) expr->expr)->read) {
          n = ((R_Read *) expr->expr)->num;
          return new_num(n);
        }
        return expr;
        break;
      default:
        break;
    }
  return expr;
}

void r_print_expr(R_Expr * expr) {
  if (expr != NULL)
    switch (expr->type) {
      case R_EXPR_VAR:
        printf("%s", ((R_Var *) expr->expr)->name);
        break;
      case R_EXPR_LET:
        printf("(let ");
        r_print_expr(((R_Let *) expr->expr)->var);
        printf(" := ");
        r_print_expr(((R_Let *) expr->expr)->expr);
        printf(" in ");
        r_print_expr(((R_Let *) expr->expr)->body);
        printf(")");
        break;
      case R_EXPR_NEG:
        printf("(R_EXPR_NEG(");
        r_print_expr(((R_Neg *) expr->expr)->expr);
        printf("))");
        break;
      case R_EXPR_ADD:
        printf("(+ ");
        r_print_expr(get_left(expr));
        printf(" ");
        r_print_expr(get_right(expr));
        printf(")");
        break;
      case R_EXPR_NUM:
        printf("%d", ((R_Num *) expr->expr)->num);
        break;
      case R_EXPR_READ:
        if (((R_Read *) expr->expr)->read)
          printf("%d", ((R_Read *) expr->expr)->num);
        else
          printf("R_EXPR_READ");
        break;
      case R_EXPR_TRUE:
        printf("true");
        break;
      case R_EXPR_FALSE:
        printf("false");
        break;
      case R_EXPR_NOT:
        printf("(! ");
        r_print_expr(((R_Not *) expr->expr)->expr);
        printf(")");
        break;
      case R_EXPR_CMP:
        switch (((R_Cmp *) expr->expr)->cmp_type) {
          case R_CMP_EQUAL:
            printf("(== ");
            break;
          case R_CMP_LESS:
            printf("(< ");
            break;
          case R_CMP_GREATER:
            printf("(> ");
            break;
          case R_CMP_LEQ:
            printf("(<= ");
            break;
          case R_CMP_GEQ:
            printf("(>= ");
            break;
        };
        r_print_expr(((R_Cmp *) expr->expr)->left);
        printf(" ");
        r_print_expr(((R_Cmp *) expr->expr)->right);
        printf(")");
        break;
      case R_EXPR_IF:
        printf("(if ");
        r_print_expr(((R_If *) expr->expr)->test_expr);
        printf(" ");
        r_print_expr(((R_If *) expr->expr)->then_expr);
        printf(" ");
        r_print_expr(((R_If *) expr->expr)->else_expr);
        printf(")");
        break;
      default:
        break;
    };
}

int get_int(R_Expr * expr) {
  if (expr)
    switch (expr->type) {
      case R_EXPR_NUM:
        return ((R_Num *) expr->expr)->num;
      case R_EXPR_READ:
        return ((R_Read *) expr->expr)->num;
      case R_EXPR_TRUE:
        return 1;
      case R_EXPR_FALSE:
        return 0;
      default:
        break;
    };
  return I32MIN;
}
