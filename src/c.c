#include "utils.h"
#include "pairs.h"
#include "list.h"
#include "c.h"

C_Program *new_c_program(void *info, list_t labels) {
  C_Program *cp = malloc_or_die(sizeof(C_Program));
  cp->info = info;
  if (labels == NULL)
    labels = list_create();
  cp->labels = labels;
  return cp;
}

C_Tail *new_c_tail(C_TAIL_TYPE type, void *tail) {
  C_Tail *ct = malloc_or_die(sizeof(C_Tail));
  ct->type = type;
  ct->tail = tail;
  return ct;
}

C_Smt *new_c_smt(C_Var * var, C_Expr * expr) {
  C_Smt *cs = malloc_or_die(sizeof(C_Smt));
  cs->var = var;
  cs->expr = expr;
  return cs;
}

C_Expr *new_c_expr(C_EXPR_TYPE type, void *expr) {
  C_Expr *ce = malloc_or_die(sizeof(C_Expr));
  ce->type = type;
  ce->expr = expr;
  return ce;
}

C_Arg *new_c_arg(C_ARG_TYPE type, void *arg) {
  C_Arg *ca = malloc_or_die(sizeof(C_Arg));
  ca->type = type;
  ca->arg = arg;
  return ca;
}

C_Num *new_c_num(int n) {
  C_Num *cn = malloc_or_die(sizeof(C_Num));
  cn->num = n;
  return cn;
}

C_Var *new_c_var(const char *name) {
  C_Var *cv = malloc_or_die(sizeof(C_Var));
  cv->name = name;
  return cv;
}

C_Ret *new_c_ret(C_Arg * arg) {
  C_Ret *cr = malloc_or_die(sizeof(C_Ret));
  cr->arg = arg;
  return cr;
}

C_Seq *new_c_seq(C_Smt * cs, C_Tail * ct) {
  C_Seq *seq = malloc_or_die(sizeof(C_Seq));
  seq->smt = cs;
  seq->tail = ct;
  return seq;
}

C_Read *new_c_read(C_Arg * ca) {
  C_Read *cr = malloc_or_die(sizeof(C_Read));
  cr->arg = ca;
  return cr;
}

C_Neg *new_c_neg(C_Arg * arg) {
  C_Neg *cn = malloc_or_die(sizeof(C_Neg));
  cn->arg = arg;
  return cn;
}

C_Add *new_c_add(C_Arg * left, C_Arg * right) {
  C_Add *ca = malloc_or_die(sizeof(C_Add));
  ca->left = left;
  ca->right = right;
  return ca;
}

C_True *new_c_true() {
  C_True *ct = malloc_or_die(sizeof(C_True));
  ct->val = 1;
  return ct;
}

C_False *new_c_false() {
  C_False *cf = malloc_or_die(sizeof(C_False));
  cf->val = 1;
  return cf;
}

C_Cmp *new_c_cmp(C_CMP_TYPE cmp_type, C_Arg * left, C_Arg * right) {
  C_Cmp *cc = malloc_or_die(sizeof(C_Cmp));
  cc->cmp_type = cmp_type;
  cc->left = left;
  cc->right = right;
  return cc;
}

C_Goto *new_c_goto(label_t lbl) {
  C_Goto *cg = malloc_or_die(sizeof(C_Goto));
  cg->lbl = lbl;
  return cg;
}

C_Goto_If *new_c_goto_if(C_Expr * cmp, label_t true_lbl, label_t false_lbl) {
  C_Goto_If *cgif = malloc_or_die(sizeof(C_Goto_If));
  cgif->cmp = cmp;
  cgif->true_lbl = true_lbl;
  cgif->false_lbl = false_lbl;
  return cgif;
}

void c_print_smt(C_Smt * cs) {
  if (cs) {
    printf("(set! %s ", ((C_Var *) cs->var)->name);
    c_print_expr(cs->expr);
    printf(")");
  }
}

void c_print_tail(C_Tail * ct) {
  if (ct)
    switch (ct->type) {
      case C_TAIL_RET:
        printf("ret ");
        c_print_arg(((C_Ret *) ct->tail)->arg);
        printf("\n");
        break;
      case C_TAIL_SEQ:
        c_print_smt(((C_Seq *) ct->tail)->smt);
        printf("\n");
        c_print_tail(((C_Seq *) ct->tail)->tail);
        break;
      case C_TAIL_GOTO:
        printf("goto %s", ((C_Goto *) ct->tail)->lbl);
        break;
      case C_TAIL_GOTO_IF:
        printf("gotoif ");
        c_print_expr(((C_Goto_If *) ct->tail)->cmp);
        printf(" %s", ((C_Goto_If *) ct->tail)->true_lbl);
        printf(" %s", ((C_Goto_If *) ct->tail)->false_lbl);
        break;
      default:
        die("Invalid c_print_tail!");
    };

}

void c_print_arg(C_Arg * ca) {
  if (ca)
    switch (ca->type) {
      case C_NUM:
        printf("%d", ((C_Num *) ca->arg)->num);
        break;
      case C_VAR:
        printf("%s", ((C_Var *) ca->arg)->name);
        break;
      case C_TRUE:
        printf("true");
        break;
      case C_FALSE:
        printf("false");
        break;
      default:
        die("Invalid c_print_arg!");
    };
}

void c_print_cmp_type(C_CMP_TYPE type) {
  switch (type) {
    case C_CMP_EQUAL:
      printf("==");
      break;
    case C_CMP_LESS:
      printf("<");
      break;
    case C_CMP_LEQ:
      printf("<=");
      break;
    case C_CMP_GEQ:
      printf(">=");
      break;
    case C_CMP_GREATER:
      printf(">");
      break;
  };
}

void c_print_expr(C_Expr * ce) {
  if (ce)
    switch (ce->type) {
      case C_ARG:
        c_print_arg(ce->expr);
        break;
      case C_READ:
        printf("READ");
        break;
      case C_NEG:
        printf("(- ");
        c_print_arg(((C_Neg *) ce->expr)->arg);
        printf(")");
        break;
      case C_ADD:
        printf("(+ ");
        c_print_arg(((C_Add *) ce->expr)->left);
        printf(" ");
        c_print_arg(((C_Add *) ce->expr)->right);
        printf(")");
        break;
      case C_NOT:
        printf("(! ");
        c_print_arg(((C_Not *) ce->expr)->arg);
        printf(")");
        break;
      case C_CMP:
        printf("(");
        c_print_cmp_type(((C_Cmp *) ce->expr)->cmp_type);
        printf(" ");
        c_print_arg(((C_Cmp *) ce->expr)->left);
        printf(" ");
        c_print_arg(((C_Cmp *) ce->expr)->right);
        printf(")");
        break;
      default:
        die("Invalid c_expr_print!");
        break;
    };
}

void c_print(C_Program * cp) {
  if (cp)
    list_print(cp->labels, lbl_tail_print);
}

int c_p_interp(C_Program * cp) {
  C_Tail *ct;
  if (cp) {
    Node *node_main =
      list_find(cp->labels, new_lbl_tail_pair("main", NULL), lbl_tail_cmp);
    Node *node_body =
      list_find(cp->labels, new_lbl_tail_pair("body", NULL), lbl_tail_cmp);
    if (node_main == NULL && node_body == NULL)
      die("[C_P_INTERP] NO MAIN OR BODY LABEL!");
    if (!node_main)
      ct = ((lbl_tail_pair_t *) node_body->data)->tail;
    else
      ct = ((lbl_tail_pair_t *) node_main->data)->tail;
    return c_t_interp(ct, list_create(), cp->labels);
  }
  return I32MIN;
}

int c_t_interp(C_Tail * ct, list_t env, list_t lbl2tail) {
  int res;
  Node *lbl_node;
  lbl_tail_pair_t *p;
  if (ct && env)
    switch (ct->type) {
      case C_TAIL_RET:
        return c_a_interp(((C_Ret *) ct->tail)->arg, env);
      case C_TAIL_SEQ:
        c_s_interp(((C_Seq *) ct->tail)->smt, env);
        return c_t_interp(((C_Seq *) ct->tail)->tail, env, lbl2tail);
      case C_TAIL_GOTO:
        lbl_node =
          list_find(lbl2tail,
                    new_lbl_tail_pair(((C_Goto *) ct->tail)->lbl, NULL),
                    lbl_tail_cmp);
        if (!lbl_node)
          die("[c_t_interp] GOTO INVALID LABEL!");
        p = lbl_node->data;
        return c_t_interp(p->tail, list_create(), lbl2tail);
      case C_TAIL_GOTO_IF:
        res = c_e_interp(((C_Goto_If *) ct->tail)->cmp, env);
        if (res)
          lbl_node =
            list_find(lbl2tail,
                      new_lbl_tail_pair(((C_Goto_If *) ct->tail)->true_lbl,
                                        NULL), lbl_tail_cmp);
        else
          lbl_node =
            list_find(lbl2tail,
                      new_lbl_tail_pair(((C_Goto_If *) ct->tail)->false_lbl,
                                        NULL), lbl_tail_cmp);
        if (!lbl_node)
          die("[c_t_interp] GOTOIF INVALID LABEL!");
        p = lbl_node->data;
        return c_t_interp(p->tail, list_create(), lbl2tail);
      default:
        die("INVALID c_t_interp!");
    }
  return I32MIN;
}

int c_s_interp(C_Smt * cs, list_t env) {
  int res = I32MIN;
  if (cs && env) {
    Node *node =
      list_find(env, new_c_var_num_pair(cs->var, 0), c_var_num_pair_cmp);
    res = c_e_interp(cs->expr, env);
    c_var_num_pair_t *cvnp = new_c_var_num_pair(cs->var, res);
    if (node)
      list_update(env, node->data, cvnp, c_var_num_pair_cmp);
    else
      list_insert(env, cvnp);
  }
  return res;
}

int c_e_interp(C_Expr * ce, list_t env) {
  int r, left, right;
  C_Cmp *cmp;
  if (ce && env)
    switch (ce->type) {
      case C_ARG:
        return c_a_interp((C_Arg *) ce->expr, env);
      case C_READ:
        if (QUIET_READ)
          return 7;
        scanf("%d", &r);
        return r;
      case C_NEG:
        return -1 * c_a_interp(((C_Neg *) ce->expr)->arg, env);
      case C_ADD:
        return c_a_interp(((C_Add *) ce->expr)->left,
                          env) + c_a_interp(((C_Add *) ce->expr)->right, env);
      case C_NOT:
        return !c_a_interp(((C_Not *) ce->expr)->arg, env);
      case C_CMP:
        cmp = ce->expr;
        left = c_a_interp(cmp->left, env);
        right = c_a_interp(cmp->right, env);
        switch (cmp->cmp_type) {
          case C_CMP_EQUAL:
            return left == right;
          case C_CMP_LESS:
            return left < right;
          case C_CMP_LEQ:
            return left <= right;
          case C_CMP_GEQ:
            return left >= right;
          case C_CMP_GREATER:
            return left > right;
        };
      default:
        die("INVALID c_e_interp!");
    };
  return I32MIN;
}

int c_a_interp(C_Arg * ca, list_t env) {
  Node *node;
  if (ca && env)
    switch (ca->type) {
      case C_TRUE:
        return ((C_True *) ca->arg)->val;
      case C_FALSE:
        return ((C_False *) ca->arg)->val;
      case C_NUM:
        return ((C_Num *) ca->arg)->num;
      case C_VAR:
        node =
          list_find(env, new_c_var_num_pair((C_Var *) ca->arg, 0),
                    c_var_num_pair_cmp);
        if (node == NULL)
          die("[C_A_INTERP] Unbound C_Variable!");
        return ((c_var_num_pair_t *) node->data)->num;
      default:
        die("INVALID c_a_interp!");
    };
  return I32MIN;
}
