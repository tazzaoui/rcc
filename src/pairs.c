#include <string.h>
#include "pairs.h"
#include "utils.h"
#include "r.h"
#include "x.h"
#include "rcc.h"

x_arg_pair_t *new_x_arg_pair(X_Arg * arg1, X_Arg * arg2) {
  x_arg_pair_t *x = malloc_or_die(sizeof(x_arg_pair_t));
  x->arg1 = arg1;
  x->arg2 = arg2;
  return x;
}

CMP x_arg_pair_cmp(void *a, void *b) {
  X_Arg *a_arg = ((x_arg_pair_t *) a)->arg1;
  X_Arg *b_arg = ((x_arg_pair_t *) b)->arg1;
  return cmp_x_args(a_arg, b_arg);
}

CMP x_arg_pair_cmp2(void *a, void *b) {
  X_Arg *a_arg = ((x_arg_pair_t *) a)->arg2;
  X_Arg *b_arg = ((x_arg_pair_t *) b)->arg2;
  return cmp_x_args(a_arg, b_arg);
}

void print_x_arg_pair(void *a) {
  x_arg_pair_t *a_arg = (x_arg_pair_t *) a;
  printf("<");
  x_print_arg(a_arg->arg1);
  printf(", ");
  x_print_arg(a_arg->arg2);
  printf(">");
}

void *copy_x_arg(void *arg) {
  return arg;
}

void *copy_x_arg_list_pair(void *a) {
  x_arg_list_pair_t *x = (x_arg_list_pair_t *) a;
  list_t new_list = list_copy(x->list, copy_x_arg);
  return new_x_arg_list_pair(x->arg, new_list);
}

x_arg_int_pair_t *new_x_arg_int_pair(X_Arg * arg, int num) {
  x_arg_int_pair_t *x = malloc_or_die(sizeof(x_arg_int_pair_t));
  x->arg = arg;
  x->num = num;
  return x;
}

CMP x_arg_int_pair_cmp(void *a, void *b) {
  X_Arg *a_arg = ((x_arg_int_pair_t *) a)->arg;
  X_Arg *b_arg = ((x_arg_int_pair_t *) b)->arg;
  return cmp_x_args(a_arg, b_arg);
}

CMP x_arg_int_pair_cmp_by_int(void *a, void *b) {
  int a_num = ((x_arg_int_pair_t *) a)->num;
  int b_num = ((x_arg_int_pair_t *) b)->num;
  if (a_num == b_num)
    return EQUAL;
  else if (a_num < b_num)
    return LESS;
  else
    return GREATER;
}

void print_x_arg_int_pair(void *a) {
  if (a) {
    x_arg_int_pair_t *x = ((x_arg_int_pair_t *) a);
    printf("<");
    x_print_arg(x->arg);
    printf(", %d>", x->num);
  }
}

x_arg_list_pair_t *new_x_arg_list_pair(X_Arg * arg, list_t list) {
  x_arg_list_pair_t *x = malloc_or_die(sizeof(x_arg_list_pair_t));
  x->arg = arg;
  x->list = list;
  return x;
}

CMP x_arg_list_pair_cmp(void *alp1, void *alp2) {
  X_Arg *a1 = ((x_arg_list_pair_t *) alp1)->arg;
  X_Arg *a2 = ((x_arg_list_pair_t *) alp2)->arg;
  return cmp_x_args(a1, a2);
}

void print_x_arg_list_pair(void *data) {
  x_arg_list_pair_t *x = (x_arg_list_pair_t *) data;
  printf("<");
  x_print_arg(x->arg);
  printf(" ");
  list_print(x->list, x_print_arg_void);
  printf(">\n");
}

x_instr_list_pair_t *new_x_instr_list_pair(X_Instr * xi, list_t live) {
  x_instr_list_pair_t *x = malloc_or_die(sizeof(x_instr_list_pair_t));
  x->xi = xi;
  x->live = live;
  return x;
}

void x_print_arg_void(void *data) {
  if (data)
    x_print_arg(data);
}

void print_x_instr_list_pair(void *data) {
  x_instr_list_pair_t *x = (x_instr_list_pair_t *) data;
  if (x) {
    print_x_instr(x->xi);
    printf("Live After = ");
    list_print(x->live, x_print_arg_void);
    printf("\n");
  }
}

c_var_num_pair_t *new_c_var_num_pair(C_Var * cv, int num) {
  c_var_num_pair_t *p = malloc_or_die(sizeof(c_var_num_pair_t));
  p->var = cv;
  p->num = num;
  return p;
}

CMP c_var_num_pair_cmp(void *a, void *b) {
  if (a && b) {
    C_Var *cv_a = ((c_var_num_pair_t *) a)->var;
    C_Var *cv_b = ((c_var_num_pair_t *) b)->var;
    if (strcmp(cv_a->name, cv_b->name) == 0)
      return EQUAL;
  }
  return UNEQUAL;
}

CMP c_var_cmp(void *a, void *b) {
  C_Var *cv_a = (C_Var *) a;
  C_Var *cv_b = (C_Var *) b;
  if (strcmp(cv_a->name, cv_b->name) == 0)
    return EQUAL;
  return UNEQUAL;
}

void c_var_print(void *a) {
  if (a) {
    C_Var *cv_a = (C_Var *) a;
    printf("%s\n", cv_a->name);
  }
}

env_pair_t *new_env_pair(R_Expr * var, R_Expr * val) {
  env_pair_t *ep = malloc_or_die(sizeof(env_pair_t));
  ep->var = var;
  ep->val = val;
  return ep;
}

CMP ep_cmp(void *a, void *b) {
  if (a != NULL && b != NULL) {
    env_pair_t *ae = (env_pair_t *) a;
    env_pair_t *be = (env_pair_t *) b;
    if (strcmp(((R_Var *) ae->var->expr)->name, ((R_Var *) be->var->expr)->name)
        == 0)
      return EQUAL;
  }
  return UNEQUAL;
}


CMP ep_var_cmp(void *a, void *b) {
  if (a != NULL && b != NULL) {
    R_Expr *a_expr = (R_Expr *) a;
    R_Expr *b_expr = ((env_pair_t *) b)->var;
    if (a_expr->type == R_EXPR_VAR && b_expr->type == R_EXPR_VAR &&
        strcmp(((R_Var *) a_expr->expr)->name,
               ((R_Var *) b_expr->expr)->name) == 0)
      return EQUAL;
  }
  return UNEQUAL;
}

void *ep_cpy(void *old) {
  env_pair_t *old_pair = (env_pair_t *) old;
  return old ? (void *) new_env_pair(old_pair->var, old_pair->val) : NULL;
}

void ep_print(void *data) {
  if (data) {
    env_pair_t *ep = (env_pair_t *) data;
    printf("<");
    r_print_expr(ep->var);
    printf(",");
    r_print_expr(ep->val);
    printf(">\n");
  }
}

lbl_tail_pair_t *new_lbl_tail_pair(label_t lbl, C_Tail * tail) {
  lbl_tail_pair_t *lbt = malloc_or_die(sizeof(lbl_tail_pair_t));
  lbt->label = lbl;
  lbt->tail = tail;
  return lbt;
}

CMP lbl_tail_cmp(void *a, void *b) {
  if (a && b) {
    lbl_tail_pair_t *lt_a = (lbl_tail_pair_t *) a;
    lbl_tail_pair_t *lt_b = (lbl_tail_pair_t *) b;
    if (strcmp(lt_a->label, lt_b->label) == 0)
      return EQUAL;
  }
  return UNEQUAL;
}

void lbl_tail_print(void *a) {
  lbl_tail_pair_t *lt = (lbl_tail_pair_t *) a;
  printf("%s:\n", lt->label);
  c_print_tail(lt->tail);
  printf("\n");
}

lbl_blk_pair_t *new_lbl_blk_pair(label_t label, X_Block * blk) {
  lbl_blk_pair_t *lbp = malloc_or_die(sizeof(lbl_blk_pair_t));
  lbp->label = label;
  lbp->block = blk;
  return lbp;
}

CMP lbl_blk_pair_cmp(void *a, void *b) {
  if (a && b) {
    lbl_blk_pair_t *lbp_a = (lbl_blk_pair_t *) a;
    lbl_blk_pair_t *lbp_b = (lbl_blk_pair_t *) b;
    if (strcmp(lbp_a->label, lbp_b->label) == 0)
      return EQUAL;
  }
  return UNEQUAL;
}

void print_lbl_blk_pair(void *l) {
  if (l) {
    lbl_blk_pair_t *lbp = (lbl_blk_pair_t *) l;
    X_Block *blk = (X_Block *) lbp->block;
    printf("%s:\n", lbp->label);
    list_print(blk->instrs, print_x_instr);
  }
}

reg_num_pair_t *new_reg_num_pair(REGISTER reg, int num) {
  reg_num_pair_t *rnp = malloc_or_die(sizeof(reg_num_pair_t));
  rnp->reg = reg;
  rnp->num = num;
  return rnp;
}

CMP reg_num_pair_cmp(void *a, void *b) {
  if (a && b) {
    reg_num_pair_t *rnp_a = (reg_num_pair_t *) a;
    reg_num_pair_t *rnp_b = (reg_num_pair_t *) b;
    if (rnp_a->reg == rnp_b->reg && rnp_a->num == rnp_b->num)
      return EQUAL;
  }
  return UNEQUAL;
}

void *reg_num_pair_cpy(void *old) {
  reg_num_pair_t *r = (reg_num_pair_t *) old;
  return old ? new_reg_num_pair(r->reg, r->num) : NULL;
}

void print_reg_num_pair(void *rnp) {
  if (rnp) {
    reg_num_pair_t *r = (reg_num_pair_t *) rnp;
    printf("<%s, %d>", registers[r->reg], r->num);
  }
}

num_pair_t *new_num_pair(int n1, int n2) {
  num_pair_t *np = malloc_or_die(sizeof(num_pair_t));
  np->n1 = n1;
  np->n2 = n2;
  return np;
}

CMP num_pair_cmp(void *a, void *b) {
  if (a && b) {
    num_pair_t *np_a = (num_pair_t *) a;
    num_pair_t *np_b = (num_pair_t *) b;
    if (np_a->n1 == np_b->n1)
      return EQUAL;
  }
  return UNEQUAL;
}

void *num_pair_cpy(void *old) {
  num_pair_t *n = (num_pair_t *) old;
  return old ? new_num_pair(n->n1, n->n2) : NULL;
}

void print_num_pair(void *np) {
  if (np) {
    num_pair_t *n = (num_pair_t *) np;
    printf("<%d, %d>", n->n1, n->n2);
  }
}

var_num_pair_t *new_var_num_pair(X_Arg_Var * var, int num) {
  var_num_pair_t *vnp = malloc_or_die(sizeof(var_num_pair_t *));
  vnp->var = var;
  vnp->num = num;
  return vnp;
}

CMP var_num_pair_cmp(void *a, void *b) {
  if (a && b) {
    var_num_pair_t *vnp_a = (var_num_pair_t *) a;
    var_num_pair_t *vnp_b = (var_num_pair_t *) b;
    if (strcmp(vnp_a->var->name, vnp_b->var->name) == 0)
      return EQUAL;
  }
  return UNEQUAL;
}

void *var_num_pair_cpy(void *old) {
  var_num_pair_t *v = (var_num_pair_t *) old;
  return old ? new_var_num_pair(v->var, v->num) : NULL;
}

void print_var_num_pair(void *vnp) {
  if (vnp) {
    var_num_pair_t *v = (var_num_pair_t *) vnp;
    printf("<%s, %d>", v->var->name, v->num);
  }
}

r_var_int_pair_t *new_r_var_int_pair(R_Var * v, int n) {
  r_var_int_pair_t *r = malloc_or_die(sizeof(r_var_int_pair_t));
  r->var = v;
  r->n = n;
  return r;
}

CMP r_var_int_pair_cmp(void *r1, void *r2) {
  r_var_int_pair_t *rvv_a = (r_var_int_pair_t *) r1;
  r_var_int_pair_t *rvv_b = (r_var_int_pair_t *) r2;
  return strcmp(rvv_a->var->name, rvv_b->var->name) == 0 ? EQUAL : UNEQUAL;
}

r_var_var_pair_t *new_r_var_var_pair(R_Var * v1, R_Var * v2) {
  r_var_var_pair_t *r = malloc_or_die(sizeof(r_var_var_pair_t));
  r->v1 = v1;
  r->v2 = v2;
  return r;
}

CMP r_var_var_pair_cmp(void *r1, void *r2) {
  r_var_var_pair_t *rvv_a = (r_var_var_pair_t *) r1;
  r_var_var_pair_t *rvv_b = (r_var_var_pair_t *) r2;
  return strcmp(rvv_a->v1->name, rvv_b->v1->name) == 0 ? EQUAL : UNEQUAL;
}

void *r_var_var_cpy(void *old) {
  r_var_var_pair_t *old_pair = (r_var_var_pair_t *) old;
  return old ? (void *) new_r_var_var_pair(old_pair->v1, old_pair->v2) : NULL;
}


void r_var_var_print(void *r) {
  if (r) {
    r_var_var_pair_t *v = (r_var_var_pair_t *) r;
    r_print_expr(new_expr(v->v1, R_EXPR_VAR));
    printf(" -> ");
    r_print_expr(new_expr(v->v2, R_EXPR_VAR));
    printf("\n");
  }
}

CMP cmp_x_args(void *a, void *b) {
  X_Arg *xa = (X_Arg *) a;
  X_Arg *xb = (X_Arg *) b;
  if (xa && xb && xa->type == xb->type) {
    switch (xa->type) {
      case X_ARG_NUM:
        return ((X_Arg_Num *) xa->arg)->num ==
          ((X_Arg_Num *) xb->arg)->num ? EQUAL : UNEQUAL;
      case X_ARG_REG:
        return ((X_Arg_Reg *) xa->arg)->reg ==
          ((X_Arg_Reg *) xb->arg)->reg ? EQUAL : UNEQUAL;
      case X_ARG_MEM:
        return (((X_Arg_Mem *) xa->arg)->reg == ((X_Arg_Mem *) xb->arg)->reg)
          && (((X_Arg_Mem *) xa->arg)->offset ==
              ((X_Arg_Mem *) xb->arg)->offset) ? EQUAL : UNEQUAL;
      case X_ARG_VAR:
        return strcmp(((X_Arg_Var *) xa->arg)->name,
                      ((X_Arg_Var *) xb->arg)->name) == 0 ? EQUAL : UNEQUAL;
    };
  }
  return UNEQUAL;
}
