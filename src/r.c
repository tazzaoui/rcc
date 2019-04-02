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
        return ep->val->type == R_EXPR_READ ? get_num(ep->val) : ep->val;
      case R_EXPR_LET:
        res = r_interp(get_expr(expr), env);
        ep = new_env_pair(get_var(expr), res);
        new_env = list_copy(env, ep_cpy);
        n = list_find(new_env, ep, ep_cmp);
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

R_Expr *r2_optimize(R_Expr * expr, list_t env) {
  int left, right;
  R_Cmp *cmp;
  R_If *if_expr;
  R_Not *not;
  R_Expr *l, *r, *m, *res;
  if (expr)
    switch (expr->type) {
      case R_EXPR_CMP:
        cmp = expr->expr;
        l = r2_optimize(cmp->left, env);
        r = r2_optimize(cmp->right, env);
        res = new_cmp(cmp->cmp_type, l, r);
        cmp = res->expr;
        if (cmp->left->type == R_EXPR_NUM && cmp->right->type == R_EXPR_NUM) {
          left = get_int(cmp->left);
          right = get_int(cmp->right);
          switch (cmp->cmp_type) {
            case R_CMP_EQUAL:
              return left == right ? new_true() : new_false();
            case R_CMP_LESS:
              return left < right ? new_true() : new_false();
            case R_CMP_GEQ:
              return left >= right ? new_true() : new_false();
            case R_CMP_LEQ:
              return left <= right ? new_true() : new_false();
            case R_CMP_GREATER:
              return left > right ? new_true() : new_false();
          };
        }
        return res;
      case R_EXPR_IF:
        if_expr = expr->expr;
        l = r2_optimize(if_expr->test_expr, env);
        r = r2_optimize(if_expr->then_expr, env);
        m = r2_optimize(if_expr->else_expr, env);
        res = new_if(l, r, m);
        if_expr = res->expr;
        if (if_expr->test_expr->type == R_EXPR_TRUE)
          return if_expr->then_expr;
        if (if_expr->test_expr->type == R_EXPR_FALSE)
          return if_expr->else_expr;
        if (if_expr->test_expr->type == R_EXPR_NOT) {
          not = if_expr->test_expr->expr;
          return new_if(not->expr, if_expr->else_expr, if_expr->then_expr);
        }
        return res;
      case R_EXPR_NOT:
        not = expr->expr;
        res = r2_optimize(not->expr, env);
        res = new_not(res);
        not = res->expr;
        if (not->expr->type == R_EXPR_NOT)
          return ((R_Not *) not->expr)->expr;
        return res;
      default:
        break;
    };
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

void r_print_type(R_EXPR_TYPE type) {
  switch (type) {
    case R_TYPE_S64:
      printf("<S64>");
      break;
    case R_TYPE_BOOL:
      printf("<BOOL>");
      break;
    case R_TYPE_ERROR:
      printf("<ERROR>");
      break;
    default:
      printf("<UNKOWN>");
  };
}

R_TYPE r_type_check(R_Expr * expr, list_t env) {
  R_TYPE l_type, r_type, t_type;
  r_expr_type_pair_t *p;
  list_t env_cpy;
  Node *node;
  if (env == NULL)
    env = list_create();
  if (expr)
    switch (expr->type) {
      case R_EXPR_NUM:
        return R_TYPE_S64;
      case R_EXPR_TRUE:
      case R_EXPR_FALSE:
        return R_TYPE_BOOL;
      case R_EXPR_VAR:
        p = new_r_expr_type_pair(expr, R_TYPE_ERROR);
        node = list_find(env, p, r_expr_type_pair_cmp);
        if (node)
          return ((r_expr_type_pair_t *) node->data)->type;
        break;
      case R_EXPR_ADD:
        l_type = r_type_check(get_left(expr), env);
        r_type = r_type_check(get_right(expr), env);
        if (l_type == R_TYPE_S64 && r_type == R_TYPE_S64)
          return R_TYPE_S64;
        break;
      case R_EXPR_NEG:
        if (r_type_check(((R_Neg *) expr->expr)->expr, env) == R_TYPE_S64)
          return R_TYPE_S64;
        break;
      case R_EXPR_CMP:
        l_type = r_type_check(get_left(expr), env);
        r_type = r_type_check(get_right(expr), env);
        if (l_type == R_TYPE_S64 && r_type == R_TYPE_S64)
          return R_TYPE_BOOL;
        break;
      case R_EXPR_NOT:
        l_type = r_type_check(((R_Not *) expr->expr)->expr, env);
        if (l_type == R_TYPE_BOOL)
          return R_TYPE_BOOL;
        break;
      case R_EXPR_IF:
        t_type = r_type_check(((R_If *) expr->expr)->test_expr, env);
        l_type = r_type_check(((R_If *) expr->expr)->then_expr, env);
        r_type = r_type_check(((R_If *) expr->expr)->else_expr, env);
        if (t_type == R_TYPE_BOOL && l_type == r_type)
          return r_type;
        break;
      case R_EXPR_LET:
        t_type = r_type_check(((R_Let *) expr->expr)->expr, env);
        p = new_r_expr_type_pair(((R_Let *) expr->expr)->var, t_type);
        env_cpy = list_copy(env, r_expr_type_pair_cpy);
        list_remove_all(env_cpy, p, r_expr_type_pair_cmp);
        list_insert(env_cpy, p);
        l_type = r_type_check(((R_Let *) expr->expr)->body, env_cpy);
        return l_type;
      default:
        break;
    };
  return R_TYPE_ERROR;
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

R_Expr *get_left(R_Expr * expr) {
  if (expr && expr->type == R_EXPR_ADD)
    return ((R_Add *) expr->expr)->left;
  if (expr && expr->type == R_EXPR_CMP)
    return ((R_Cmp *) expr->expr)->left;
  return expr;
}

R_Expr *get_right(R_Expr * expr) {
  if (expr && expr->type == R_EXPR_ADD)
    return ((R_Add *) expr->expr)->right;
  if (expr && expr->type == R_EXPR_CMP)
    return ((R_Cmp *) expr->expr)->left;
  return expr;
}

R_Expr *get_num(R_Expr * expr) {
  if (expr && expr->type == R_EXPR_NUM)
    return expr;
  if (expr && expr->type == R_EXPR_READ)
    return new_num(((R_Read *) expr->expr)->num);
  return NULL;
}
