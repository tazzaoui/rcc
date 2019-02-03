#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "rcc.h"
#include "utils.h"
#include "list.h"

#define QUIET_READ 1
#define DEBUG 0

Expr* new_expr(void* expr, EXPR_TYPE type) {
  Expr* e = malloc_or_die(sizeof(Expr));
  e->expr = expr;
  e->type = type;
  return e;
}

Program* new_prog(void* info, Expr* expr) {
  Program* p = malloc_or_die(sizeof(Program));
  p->info = info;
  p->expr = expr;
  return p;
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

env_pair_t *new_env_pair(Expr* var, int val){
    env_pair_t *ep = malloc_or_die(sizeof(env_pair_t));
    ep->var = var;
    ep->val = val;
    return ep;
}

int ep_cmp(void *a, void *b){
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
    printf(",%d> ", ep->val);
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
          n = list_find(env, expr, ep_cmp);
          if(n == NULL) die("Unbound Variable!\n");
          ep = (env_pair_t*) n->data;
          return ep->val;
      case LET:
        val = interp(get_expr(expr), env); 
        ep = new_env_pair(get_var(expr), val);
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

Expr* optimize_neg(Expr* neg) {
  if (neg != NULL && neg->type == NEG) {
    Expr *neg_left, *neg_right, *res;

    res = optimize(((Neg*)neg->expr)->expr);

    if (res->type == NUM)
      return new_num(-1 * ((Num*)res->expr)->num);

    else if (res->type == NEG)
      return optimize(((Neg*)res->expr)->expr);

    else if (res->type == ADD) {
      neg_left = new_neg(get_left(res));
      neg_right = new_neg(get_right(res));
      return new_add(neg_left, neg_right);
    }
  }
  return neg;
}

Expr* optimize_add(Expr* add) {
  if (add != NULL && add->type == ADD) {
    Expr* left_opt = optimize(get_left(add));
    Expr* right_opt = optimize(get_right(add));
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

Expr* optimize(Expr* expr) {
  int n;
  if (expr != NULL) switch (expr->type) {
      case VAR:
        return expr;
      case LET:
        return expr;
      case NEG:
        return optimize_neg(expr);
      case ADD:
        return optimize_add(expr);
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
