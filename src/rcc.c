#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "list.h"
#include "r.h"
#include "x.h"
#include "rcc.h"

env_pair_t *new_env_pair(Expr* var, Expr* val){
    env_pair_t *ep = malloc_or_die(sizeof(env_pair_t));
    ep->var = var;
    ep->val = val;
    return ep;
}

lbl_blk_pair_t* new_lbl_blk_pair(label_t label, Block* blk){
    lbl_blk_pair_t *lbp = malloc_or_die(sizeof(lbl_blk_pair_t));
    lbp->label = label;
    lbp->block = blk;
    return lbp;
}

int ep_var_cmp(void *a, void *b){
    if(a != NULL && b != NULL){
    Expr *a_expr = (Expr*) a; 
    Expr *b_expr = ((env_pair_t*) b)->var;
    if(a_expr != NULL && b_expr != NULL && a_expr->type == VAR && b_expr->type == VAR)
        return strcmp(((Var*)a_expr->expr)->name, ((Var*)b_expr->expr)->name) == 0; 
    }
    return 0;
}

void* ep_cpy(void *old){
    env_pair_t *old_pair = (env_pair_t*) old;
    return (void*) new_env_pair(old_pair->var, old_pair->val);
}

void ep_print(void *data){
    env_pair_t *ep = (env_pair_t*)data;
    printf("<");
    print(ep->var);
    printf(",");
    print(ep->val);
    printf(">");
}

int interp(Expr* expr, list_t env) {
    Node* n;
    if(env == NULL)
        env = list_create();
    list_t new_env;
    int val;
    env_pair_t *ep;
  if (expr != NULL) 
      switch (expr->type) {
      case VAR: 
          n = list_find(env, expr, ep_var_cmp);
          if(n == NULL) die("Unbound Variable!\n");
          ep = (env_pair_t*) n->data;
          return get_num(ep->val);
      case LET:
        val = interp(get_expr(expr), env); 
        ep = new_env_pair(get_var(expr), new_num(val));
        new_env = list_copy(env, ep_cpy);
        list_insert(new_env, ep);
        return interp(get_body(expr), new_env);
      case NEG:
        return -1 * interp(((Neg*)expr->expr)->expr, env);
      case ADD:
        return interp(get_left(expr), env) + interp(get_right(expr), env);
      case NUM:
        return get_num(expr);
      case READ:
        if (!QUIET_READ) {
          printf("read> ");
          scanf("%d", &(((Read*)expr->expr)->num));
        }
        return ((Read*)expr->expr)->num;
    }
  die("Invalid Program!");
  return -1;
}

Expr* optimize_neg(Expr* neg, list_t env) {
  if (neg != NULL && neg->type == NEG) {
    Expr *neg_left, *neg_right, *res;

    res = optimize(((Neg*)neg->expr)->expr, env);

    if (res->type == NUM)
      return new_num(-1 * ((Num*)res->expr)->num);

    else if (res->type == NEG)
      return optimize(((Neg*)res->expr)->expr, env);

    else if (res->type == ADD) {
      neg_left = new_neg(get_left(res));
      neg_right = new_neg(get_right(res));
      return new_add(neg_left, neg_right);
    }
  }
  return neg;
}

Expr* optimize_add(Expr* add, list_t env) {
  if (add != NULL && add->type == ADD) {
    Expr* left_opt = optimize(get_left(add), env);
    Expr* right_opt = optimize(get_right(add), env);
    Expr *left, *right, *res_num;

    /* (+ NUM NUM) -> NUM+NUM */
    if (left_opt->type == NUM && right_opt->type == NUM)
      return new_num(get_num(left_opt) + get_num(right_opt));

    /* Left is a num, right is another add */
    if (left_opt->type == NUM && right_opt->type == ADD) {
      left = get_left(right_opt);
      right = get_right(right_opt);

      /* (+ NUM  (+(NUM X))) -> (+ NUM+NUM X) */
      if (left->type == NUM) {
        res_num = new_num(get_num(left_opt) + get_num(left));
        return new_add(res_num, right);
      }

      /* (+ NUM  (+(X NUM))) -> (+ NUM+NUM X) */
      if (right->type == NUM) {
        res_num = new_num(get_num(left_opt) + get_num(right));
        return new_add(res_num, left);
      }
    }

    /* Right is a num, left is another add */
    if (right_opt->type == NUM && left_opt->type == ADD) {
      left = get_left(left_opt);
      right = get_right(left_opt);

      /* (+ NUM  (+(NUM X))) -> (+ NUM+NUM X) */
      if (left->type == NUM) {
        res_num = new_num(get_num(right_opt) + get_num(left));
        return new_add(res_num, right);
      }

      /* (+ NUM  (+(X NUM))) -> (+ NUM+NUM X) */
      if (right->type == NUM) {
        res_num = new_num(get_num(right_opt) + get_num(right));
        return new_add(res_num, left);
      }
    }
  }
  return add;
}

Expr* optimize(Expr* expr, list_t env) {
  env_pair_t *ep;
  list_t new_env;
  Expr *tmp, *e;
  Node* node;
  int n;
  if(env == NULL)
      env = list_create();
  if (expr != NULL) switch (expr->type) {
      case VAR:
          node = list_find(env, expr, ep_var_cmp);
          if(node == NULL)
              die("[OPT] Unbound Variable!");
          return ((env_pair_t*) node->data)->val; 
      case LET:
          tmp = optimize(((Let*)expr->expr)->expr, env);
          new_env = list_copy(env, ep_cpy);
          if(is_simple(tmp)){     
              ep = new_env_pair(((Let*)expr->expr)->var, tmp);
              list_insert(new_env, ep);
              return optimize(((Let*)expr->expr)->body, new_env);
          }else{
              ep = new_env_pair(((Let*)expr->expr)->var, ((Let*)expr->expr)->body);
              list_insert(new_env, ep);
              e = optimize(((Let*)expr->expr)->body, new_env);
              return new_let(((Let*)expr->expr)->var, tmp, e);
          }
        return expr;
      case NEG:
        return optimize_neg(expr, env);
      case ADD:
        return optimize_add(expr, env);
      case NUM:
        return expr;
      case READ:
        if (((Read*)expr->expr)->read) {
          n = ((Read*)expr->expr)->num;
          return new_num(n);
        }
        return expr;
        break;
      default:
        break;
    }
  return expr;
}

void print(Expr* expr) {
  if (expr != NULL) switch (expr->type) {
      case VAR:
        printf("%s", ((Var*)expr->expr)->name);
        break;
      case LET:
        printf("(let ");
        print(((Let*)expr->expr)->var);
        printf(" := ");
        print(((Let*)expr->expr)->expr);
        printf(" in ");
        print(((Let*)expr->expr)->body);
        printf(")");
        break;
      case NEG:
        printf("(NEG(");
        print(((Neg*)expr->expr)->expr);
        printf("))");
        break;
      case ADD:
        printf("(+ ");
        print(get_left(expr));
        printf(" ");
        print(get_right(expr));
        printf(")");
        break;
      case NUM:
        printf("%d", ((Num*)expr->expr)->num);
        break;
      case READ:
        if (((Read*)expr->expr)->read)
          printf("%d", ((Read*)expr->expr)->num);
        else
          printf("READ");
        break;
      default:
        break;
    }
}
