#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "utils.h"
#include "list.h"
#include "pairs.h"
#include "r.h"
#include "x.h"
#include "rcc.h"

#define TMP_REG RAX

const REGISTER Caller_Saved_Regs[NUM_CALLER_SAVED_REGS] =
  { RAX, RDX, RCX, RSI, RDI, R8, R9, R10, R11 };
const REGISTER Callee_Saved_Regs[NUM_CALLEE_SAVED_REGS] =
  { RSP, RBP, R12, R13, R14, R15 };

X_Program *compile(R_Expr * expr) {
  R_Expr *uniq, *simple;
  C_Tail *c_tail;
  C_Program *cp, *cp_uncovered;
  X_Program *xp;
  list_t labels = list_create(), new_vars = list_create();
  uniq = unique(expr);
  simple = rco(uniq, &new_vars);
  c_tail = econ_expr(simple);
  list_insert(labels, new_lbl_tail_pair("body", c_tail));
  cp = new_c_program(NULL, labels);
  cp_uncovered = uncover_locals(cp);
  xp = select_instr(cp_uncovered);
  xp = reg_alloc(xp);
  xp = patch_instrs(xp);
  xp = main_pass(xp);
  return xp;
}

X_Program *reg_alloc(X_Program * xp) {
  xp = uncover_live(xp);
  xp = build_interferences(xp);
  xp = color_graph(xp);
  return assign_registers(xp);
}

char *append_int(const char *old, int n) {
  int num_digits = n == 0 ? 1 : floor(log10(abs(n))) + 1;
  char *new = malloc_or_die(strlen(old) + num_digits + 1);
  sprintf(new, "%s_%d", old, n);
  return new;
}

void c_tail_extract_vars(C_Tail * tail, list_t vars) {
  if (tail && tail->type == C_TAIL_SEQ) {
    C_Seq *cs = tail->tail;
    C_Var *cv = cs->smt->var;
    Node *node = list_find(vars, cv, c_var_cmp);
    if (node == NULL)
      list_insert(vars, cv);
    c_tail_extract_vars(cs->tail, vars);
  }
}

int x_compile(X_Program * xp) {
  FILE *fp;
  int cc_result;
  char buf[1024];
  sprintf(buf, "cc %s test_compile.s -o test.bin", RUNTIME_PATH);
  x_emit(xp, "test_compile.s");
  system(buf);
  if ((fp = popen("./test.bin", "r")) == NULL)
    die("[x_compile] Failed to run test.bin!");
  fscanf(fp, "%d", &cc_result);
  pclose(fp);
  return cc_result;
}

X_Program *uncover_live(X_Program * xp) {
  Node *node =
    list_find(xp->labels, new_lbl_blk_pair("body", NULL), lbl_blk_pair_cmp);
  if (node == NULL)
    die("[UNCOVER LIVE] NO BODY LABEL!");
  Node *head = *((lbl_blk_pair_t *) node->data)->block->instrs;
  list_t live_after_set = list_create();
  while (head) {
    list_t live = live_before(head->next);
    list_insert(live_after_set, new_x_instr_list_pair(head->data, live));
    head = head->next;
  }
  if (xp->info == NULL)
    xp->info = new_info(NULL, live_after_set, NULL);
  else
    xp->info->live = live_after_set;
  return xp;
}

void add_live_after(const list_t live_after, list_t graph, X_Arg * key,
                    X_Arg * ignore) {
  if (live_after && graph && key) {
    X_Arg *x_arg;
    x_arg_list_pair_t *key_pair = new_x_arg_list_pair(key, list_create());
    Node *la_head = *live_after, *key_node =
      list_find(graph, key_pair, x_arg_list_pair_cmp);

    if (!key_node) {
      list_insert(graph, key_pair);
      key_node = list_find(graph, key_pair, x_arg_list_pair_cmp);
    }

    assert(key_node);
    key_pair = key_node->data;

    while (la_head) {
      x_arg = la_head->data;
      if (!cmp_x_args(key, x_arg) && (!ignore || !cmp_x_args(ignore, x_arg)))
        list_insert(key_pair->list, x_arg);
      la_head = la_head->next;
    }
  }
}

void add_caller_saved_regs(list_t list, list_t graph) {
  X_Arg *xa;
  Node *head, *node;
  x_arg_list_pair_t *key_pair;
  for (int i = 0; i < NUM_CALLER_SAVED_REGS; ++i) {
    head = *list;
    xa = new_x_arg(X_ARG_REG, new_x_arg_reg(Caller_Saved_Regs[i]));
    while (head) {
      key_pair = new_x_arg_list_pair(head->data, list_create());
      node = list_find(graph, key_pair, x_arg_list_pair_cmp);
      if (node == NULL) {
        list_insert(key_pair->list, xa);
        list_insert(graph, key_pair);
      } else
        list_insert(((x_arg_list_pair_t *) node->data)->list, xa);
      head = head->next;
    }
  }
}

void move_bias(list_t m_graph, X_Instr * xi) {
  if (xi && xi->type == MOVQ) {
    X_Arg *left = ((X_Movq *) xi->instr)->left;
    X_Arg *right = ((X_Movq *) xi->instr)->right;
    if (left->type == X_ARG_VAR && right->type == X_ARG_VAR) {
      x_arg_list_pair_t *m = new_x_arg_list_pair(left, list_create());
      Node *mov = list_find(m_graph, m, x_arg_list_pair_cmp);
      if (mov == NULL) {
        list_insert(m->list, right);
        list_insert(m_graph, m);
      } else
        list_insert(((x_arg_list_pair_t *) mov->data)->list, right);
    }
  }
}

void make_symmetric(list_t graph) {
  Node *node = *graph, *tmp, *ptr;
  x_arg_list_pair_t *pair;
  list_t old = list_copy(graph, copy_x_arg_list_pair);
  node = *old;
  while (node) {
    pair = node->data;
    tmp = *(pair->list);
    while (tmp) {
      ptr =
        list_find(graph, new_x_arg_list_pair(tmp->data, NULL),
                  x_arg_list_pair_cmp);
      if (!ptr)
        ptr = list_insert(graph, new_x_arg_list_pair(tmp->data, list_create()));
      list_insert(((x_arg_list_pair_t *) ptr->data)->list, pair->arg);
      tmp = tmp->next;
    }
    node = node->next;
  }
}

X_Program *build_interferences(X_Program * xp) {
  if (xp && xp->info && xp->info->live) {
    x_instr_list_pair_t *x;
    list_t graph = list_create(), move_graph = list_create();
    Node *head = *(xp->info->live);
    while (head) {
      x = head->data;
      switch (x->xi->type) {
        case ADDQ:
          add_live_after(x->live, graph, ((X_Addq *) x->xi->instr)->right,
                         NULL);
          break;
        case SUBQ:
          add_live_after(x->live, graph, ((X_Subq *) x->xi->instr)->right,
                         NULL);
          break;
        case NEGQ:
          add_live_after(x->live, graph, ((X_Negq *) x->xi->instr)->arg, NULL);
          break;
        case MOVQ:
          add_live_after(x->live, graph, ((X_Movq *) x->xi->instr)->right,
                         ((X_Movq *) x->xi->instr)->left);
          move_bias(move_graph, x->xi);
          break;
        case CALLQ:
          add_caller_saved_regs(x->live, graph);
          break;
        default:
          break;
      }
      head = head->next;
    }
    head = *graph;
    while (head) {
      x_arg_list_pair_t *pair = head->data;
      list_remove_duplicates(pair->list, cmp_x_args);
      head = head->next;
    }
    make_symmetric(graph);
    make_symmetric(move_graph);
    xp->info->i_graph = graph;
    xp->info->m_graph = move_graph;
  }
  return xp;
}

int saturation(X_Arg * xa, list_t i_graph, list_t coloring, list_t sat) {
  int curr_max = NO_COLOR;
  Node *head, *node;
  x_arg_int_pair_t *xp;
  node = list_find(i_graph, new_x_arg_list_pair(xa, NULL), x_arg_list_pair_cmp);

  if (node) {
    head = *(((x_arg_list_pair_t *) node->data)->list);
    while (head) {
      node =
        list_find(coloring, new_x_arg_int_pair(head->data, 0),
                  x_arg_int_pair_cmp);
      if (node) {
        xp = node->data;
        if (xp->num != NO_COLOR)
          list_insert(sat, node->data);
        curr_max = xp->num > curr_max ? xp->num : curr_max;
      }
      head = head->next;
    }
  }

  return curr_max;
}

x_arg_list_pair_t *max_sat(list_t i_graph, list_t coloring) {
  Node *head = *i_graph;
  list_t sat;
  int ms, curr_max = I32MIN;
  x_arg_list_pair_t *pair, *max_pair = NULL;
  if (list_size(coloring) == 0)
    return (*i_graph)->data;
  while (head) {
    pair = head->data;
    sat = list_create();
    ms = saturation(pair->arg, i_graph, coloring, sat);
    if (ms > curr_max) {
      curr_max = ms;
      max_pair = pair;
    }
    head = head->next;
  }
  return max_pair;
}

list_t neighbor_colors(list_t neighbors, list_t colors) {
  list_t nc = list_create();
  Node *head = *neighbors, *node;
  while (head) {
    node =
      list_find(colors, new_x_arg_int_pair(head->data, 0), x_arg_int_pair_cmp);
    if (node)
      list_insert(nc, node->data);
    head = head->next;
  }
  return nc;
}

int find_lowest_color(list_t nc) {
  int i = 0;
  while (1)
    if (!list_find(nc, new_x_arg_int_pair(NULL, i), x_arg_int_pair_cmp_by_int))
      return i;
    else
      ++i;
}

int lowest_color(list_t neighbors, list_t colors) {
  return list_size(colors) ?
    find_lowest_color(neighbor_colors(neighbors, colors)) : 0;
}

int lowest_color_mb(x_arg_list_pair_t * vrtx, list_t m_graph, list_t colors) {
  Node *head, *min;
  list_t res, sat, neighbors, nc;
  if (list_size(colors) <= 0)
    return 0;
  if ((head = list_find(m_graph, vrtx, x_arg_list_pair_cmp))) {
    sat = list_create();
    neighbors = ((x_arg_list_pair_t *) head->data)->list;
    nc = neighbor_colors(neighbors, colors);
    saturation(vrtx->arg, m_graph, colors, sat);
    res = list_subtract(nc, sat, x_arg_int_pair_cmp);
    min = list_min(res, x_arg_int_pair_cmp);
    if (min)
      return ((x_arg_int_pair_t *) min->data)->num;
  }
  return I32MIN;
}

X_Program *color_graph_mb(X_Program * xp, int mb) {
  if (xp) {
    int color = I32MIN;
    Node *head;
    list_t i_graph = xp->info->i_graph, colors = list_create();
    X_Arg *reg = new_x_arg(X_ARG_REG, new_x_arg_reg(RAX));
    x_arg_list_pair_t *vrtx;

    // Remove RAX from the graph
    list_remove(i_graph, new_x_arg_int_pair(reg, 0), x_arg_int_pair_cmp);
    head = *i_graph;
    while (head) {
      vrtx = head->data;
      list_remove(vrtx->list, reg, cmp_x_args);
      head = head->next;
    }

    // Pre-assign intrinsic colors (registers) 
    for (int i = 3; i < NUM_REGS; ++i) {
      reg = new_x_arg(X_ARG_REG, new_x_arg_reg(i));
      list_insert(colors, new_x_arg_int_pair(reg, i - 3));
    }

    while (list_size(i_graph) > 0) {
      vrtx = max_sat(i_graph, colors);
      if (mb)
        color = lowest_color_mb(vrtx, xp->info->m_graph, colors);
      if (color == I32MIN)
        color = lowest_color(vrtx->list, colors);
      list_insert(colors, new_x_arg_int_pair(vrtx->arg, color));
      list_remove(i_graph, vrtx, x_arg_list_pair_cmp);
      color = I32MIN;
    }
    xp->info->colors = colors;
  }
  return xp;
}

X_Program *color_graph(X_Program * xp) {
  return color_graph_mb(xp, 1);
}

X_Program *assign_registers(X_Program * xp) {
  if (xp && xp->info) {
    int variable_count = 0;
    list_t bi = list_create(), ei = list_create(), new_instrs = list_create(),
      var_to_num = list_create(), lbls = list_create();

    Node *ptr, *node =
      list_find(xp->labels, new_lbl_blk_pair("body", NULL), lbl_blk_pair_cmp);
    if (node == NULL)
      die("[assign_registers] NO BODY LABEL!");
    X_Block *xb = ((lbl_blk_pair_t *) node->data)->block;

    var_to_num = allocate_registers(xp);
    node = *(xb->instrs);
    while (node != NULL) {
      list_insert(new_instrs, map_instr(node->data, var_to_num));
      node = node->next;
    }

    ptr = *var_to_num;
    while (ptr) {
      variable_count += (((x_arg_pair_t *) ptr->data)->arg2->type == X_ARG_MEM);
      ptr = ptr->next;
    }
    variable_count *= 8;
    variable_count =
      (variable_count % 16 == 0) ? variable_count : variable_count + 8;

    lbl_blk_pair_t *begin_lbp =
      new_lbl_blk_pair("begin", new_x_block(NULL, bi));
    lbl_blk_pair_t *end_lbp = new_lbl_blk_pair("end", new_x_block(NULL, ei));
    lbl_blk_pair_t *body_lbp =
      new_lbl_blk_pair("body", new_x_block(NULL, new_instrs));

    list_insert(lbls, begin_lbp);
    list_insert(lbls, body_lbp);
    list_insert(lbls, end_lbp);

    X_Arg *rbp = new_x_arg(X_ARG_REG, new_x_arg_reg(RBP));
    X_Arg *rsp = new_x_arg(X_ARG_REG, new_x_arg_reg(RSP));
    X_Arg *vc = new_x_arg(X_ARG_NUM, new_x_arg_num(variable_count));
    X_Arg *reg;

    list_insert(bi, new_x_instr(PUSHQ, new_x_pushq(rbp)));
    list_insert(bi, new_x_instr(MOVQ, new_x_movq(rsp, rbp)));
    list_insert(ei, new_x_instr(ADDQ, new_x_addq(vc, rsp)));

    // Save & Restore Any Used Callee-Saved Registers
    for (int i = 0; i < NUM_CALLEE_SAVED_REGS; ++i) {
      reg = new_x_arg(X_ARG_REG, new_x_arg_reg(Callee_Saved_Regs[i]));
      ptr = list_find(var_to_num, new_x_arg_pair(NULL, reg), x_arg_pair_cmp2);
      if (ptr)
        list_insert(bi, new_x_instr(PUSHQ, new_x_pushq(reg)));

      reg =
        new_x_arg(X_ARG_REG,
                  new_x_arg_reg(Callee_Saved_Regs
                                [NUM_CALLEE_SAVED_REGS - i - 1]));
      ptr = list_find(var_to_num, new_x_arg_pair(NULL, reg), x_arg_pair_cmp2);
      if (ptr)
        list_insert(ei, new_x_instr(POPQ, new_x_popq(reg)));
    }

    list_insert(bi, new_x_instr(SUBQ, new_x_subq(vc, rsp)));
    list_insert(bi, new_x_instr(JMP, new_x_jmp("body")));
    list_insert(ei, new_x_instr(POPQ, new_x_popq(rbp)));
    list_insert(ei, new_x_instr(RETQ, new_x_retq()));

    return new_x_prog(NULL, lbls);
  }
  die("[assign_registers] XP OR INFO IS NULL!");
  return NULL;
}

list_t allocate_registers(X_Program * xp) {
  list_t mapping;
  if (xp) {
    mapping = list_create();
    Node *head = *(xp->info->colors);
    x_arg_int_pair_t *pair;
    while (head) {
      pair = head->data;
      if (pair->num < NUM_REGS - 3)
        list_insert(mapping, new_x_arg_pair(pair->arg, new_x_arg(X_ARG_REG,
                                                                 new_x_arg_reg
                                                                 (pair->num +
                                                                  3))));
      else
        list_insert(mapping, new_x_arg_pair(pair->arg, new_x_arg(X_ARG_MEM,
                                                                 new_x_arg_mem
                                                                 (RSP,
                                                                  8 *
                                                                  (pair->num -
                                                                   12)))));
      head = head->next;
    }
  }
  return mapping;
}

list_t live_after(Node * head) {
  list_t live = list_create();
  if (head && head->next)
    live = live_before(head->next);
  return live;
}

list_t live_before(Node * head) {
  if (head) {
    list_t la, diff, written = list_create(), read = list_create();
    la = live_after(head);
    instrs_written(head->data, written);
    instrs_read(head->data, read);
    diff = list_subtract(la, written, cmp_x_args);
    return list_concat(diff, read);
  }
  return list_create();
}

void instrs_written(X_Instr * xi, list_t args) {
  if (xi && args)
    switch (xi->type) {
      case ADDQ:
        args_written(((X_Addq *) xi->instr)->right, args);
        break;
      case SUBQ:
        args_written(((X_Subq *) xi->instr)->right, args);
        break;
      case MOVQ:
        args_written(((X_Movq *) xi->instr)->right, args);
        break;
      case POPQ:
        args_written(((X_Popq *) xi->instr)->arg, args);
        break;
      case NEGQ:
        args_written(((X_Negq *) xi->instr)->arg, args);
        break;
      default:
        break;
    };
}

void instrs_read(X_Instr * xi, list_t args) {
  if (xi && args)
    switch (xi->type) {
      case ADDQ:
        args_read(((X_Addq *) xi->instr)->left, args);
        args_read(((X_Addq *) xi->instr)->right, args);
        break;
      case SUBQ:
        args_read(((X_Subq *) xi->instr)->left, args);
        args_read(((X_Subq *) xi->instr)->right, args);
        break;
      case MOVQ:
        args_read(((X_Movq *) xi->instr)->left, args);
        break;
      case PUSHQ:
        args_read(((X_Pushq *) xi->instr)->arg, args);
        break;
      case NEGQ:
        args_read(((X_Negq *) xi->instr)->arg, args);
        break;
      default:
        break;
    };
}

void args_written(X_Arg * xa, list_t args) {
  // TODO: VERIFY THIS MAPING
  if (xa && args && (xa->type == X_ARG_VAR || xa->type == X_ARG_REG))
    list_insert(args, xa);
}

void args_read(X_Arg * xa, list_t args) {
  // TODO: VERIFY THIS MAPING
  if (xa && args && (xa->type == X_ARG_VAR || xa->type == X_ARG_REG))
    list_insert(args, xa);
}

R_Expr *uniquify(R_Expr * expr, list_t env, int *cnt) {
  Node *node;
  list_t env_p;
  R_Expr *re, *nv, *ne, *nb;
  char *c;
  if (expr && env)
    switch (expr->type) {
      case R_EXPR_VAR:
        node =
          list_find(env, new_r_var_var_pair(expr->expr, NULL),
                    r_var_var_pair_cmp);
        if (node != NULL)
          return new_expr(((r_var_var_pair_t *) node->data)->v2, R_EXPR_VAR);
        break;
      case R_EXPR_LET:
        re = ((R_Let *) expr->expr)->var;
        c = append_int(((R_Var *) re->expr)->name, (*cnt)++);
        nv = new_var(c);
        env_p = list_copy(env, r_var_var_cpy);
        node =
          list_find(env, new_r_var_var_pair(re->expr, NULL),
                    r_var_var_pair_cmp);
        if (node == NULL)
          list_insert(env_p, new_r_var_var_pair(re->expr, nv->expr));
        else
          list_update(env_p, new_r_var_var_pair(re->expr, NULL),
                      new_r_var_var_pair(re->expr, nv->expr),
                      r_var_var_pair_cmp);
        ne = uniquify(((R_Let *) expr->expr)->expr, env, cnt);
        nb = uniquify(((R_Let *) expr->expr)->body, env_p, cnt);
        return new_let(nv, ne, nb);
      case R_EXPR_NEG:
        re = uniquify(((R_Neg *) expr->expr)->expr, env, cnt);
        return new_neg(re);
      case R_EXPR_ADD:
        re = uniquify(((R_Add *) expr->expr)->left, env, cnt);
        nv = uniquify(((R_Add *) expr->expr)->right, env, cnt);
        return new_add(re, nv);
      case R_EXPR_AND:
        re = uniquify(((R_And *) expr->expr)->left, env, cnt);
        nv = uniquify(((R_And *) expr->expr)->right, env, cnt);
        return new_and(re, nv);
      case R_EXPR_OR:
        re = uniquify(((R_Or *) expr->expr)->left, env, cnt);
        nv = uniquify(((R_Or *) expr->expr)->right, env, cnt);
        return new_or(re, nv);
      case R_EXPR_NOT:
        re = uniquify(((R_Not *) expr->expr)->expr, env, cnt);
        return new_not(re);
      case R_EXPR_CMP:
        re = uniquify(((R_Cmp *) expr->expr)->left, env, cnt);
        nv = uniquify(((R_Cmp *) expr->expr)->right, env, cnt);
        return new_cmp(((R_Cmp *) expr->expr)->cmp_type, re, nv);
      case R_EXPR_IF:
        re = uniquify(((R_If *) expr->expr)->test_expr, env, cnt);
        nv = uniquify(((R_If *) expr->expr)->then_expr, env, cnt);
        ne = uniquify(((R_If *) expr->expr)->else_expr, env, cnt);
        return new_if(re, nv, ne);
      default:
        break;
    };
  return expr;
}

R_Expr *combine_lets(Node * head, R_Expr * var) {
  if (head == NULL)
    return var;
  env_pair_t *ep = head->data;
  return new_let(ep->var, ep->val, combine_lets(head->next, var));
}

R_Expr *rco(R_Expr * expr, list_t * new_vars) {
  int var_seed = 0;
  list_t env = list_create();
  R_Expr *res = resolve_complex_expr(expr, env, new_vars, &var_seed);
  return combine_lets(**new_vars, res);
}

R_Expr *resolve_complex_expr(R_Expr * expr, list_t env, list_t * new_vars,
                             int *rco_calls) {
  char *c;
  Node *n;
  list_t l1 = list_create(), l2, l3;
  R_Expr *r1, *r2, *r3, *r4;
  if (expr)
    switch (expr->type) {
      case R_EXPR_VAR:
        *new_vars = l1;
        n = list_find(env, new_env_pair(expr, NULL), ep_cmp);
        if (n == NULL)
          die("[RESOLVE_COMPLEX] UNBOUND VAR!");
        return ((env_pair_t *) n->data)->val;
      case R_EXPR_NUM:
        *new_vars = l1;
        return expr;
      case R_EXPR_ADD:
        l2 = list_create();
        r1 =
          resolve_complex_expr(((R_Add *) expr->expr)->left, env, &l1,
                               rco_calls);
        r2 =
          resolve_complex_expr(((R_Add *) expr->expr)->right, env, &l2,
                               rco_calls);
        r3 = new_add(r1, r2);
        l3 = list_concat(l1, l2);
        c = append_int("X", (*rco_calls)++);
        r4 = new_var(c);
        list_insert(l3, new_env_pair(r4, r3));
        *new_vars = l3;
        return r4;
      case R_EXPR_NEG:
        r1 =
          resolve_complex_expr(((R_Neg *) expr->expr)->expr, env, &l1,
                               rco_calls);
        c = append_int("X", (*rco_calls)++);
        r2 = new_var(c);
        list_insert(l1, new_env_pair(r2, new_neg(r1)));
        *new_vars = l1;
        return r2;
      case R_EXPR_READ:
        r1 = new_var(append_int("X", (*rco_calls)++));
        list_insert(l1, new_env_pair(r1, new_read()));
        *new_vars = l1;
        return r1;
      case R_EXPR_LET:
        l3 = list_create();
        r1 =
          resolve_complex_expr(((R_Let *) expr->expr)->expr, env, &l1,
                               rco_calls);
        l2 = list_copy(env, ep_cpy);
        list_insert(l2, new_env_pair(((R_Let *) expr->expr)->var, r1));
        r2 =
          resolve_complex_expr(((R_Let *) expr->expr)->body, l2, &l3,
                               rco_calls);
        *new_vars = list_concat(l1, l3);
        return r2;
      default:
        die("[RESOLVE_COMPLEX] INVALID EXPR!\n");
    };
  die("[RESOLVE_COMPLEX] NO RESOLUTION!");
  return NULL;
}

C_Tail *econ_expr(R_Expr * r_expr) {
  C_Smt *st;
  C_Seq *cs;
  R_Var *rv;
  if (r_expr)
    switch (r_expr->type) {
      case R_EXPR_NUM:
      case R_EXPR_VAR:
        return new_c_tail(C_TAIL_RET, new_c_ret(econ_arg(r_expr)));
      case R_EXPR_LET:
        rv = ((R_Let *) r_expr->expr)->var->expr;
        st =
          new_c_smt(new_c_var(rv->name),
                    econ_cmplx(((R_Let *) r_expr->expr)->expr));
        cs = new_c_seq(st, econ_expr(((R_Let *) r_expr->expr)->body));
        return new_c_tail(C_TAIL_SEQ, cs);
      default:
        break;
    };
  die("[econ_expr] INVALID EXPR");
  return NULL;
}

C_Expr *econ_cmplx(R_Expr * r_expr) {
  R_Add *ra;
  if (r_expr)
    switch (r_expr->type) {
      case R_EXPR_READ:
        return new_c_expr(C_READ, new_c_read(NULL));
      case R_EXPR_NEG:
        return new_c_expr(C_NEG,
                          new_c_neg(econ_arg(((R_Neg *) r_expr->expr)->expr)));
      case R_EXPR_ADD:
        ra = r_expr->expr;
        return new_c_expr(C_ADD,
                          new_c_add(econ_arg(ra->left), econ_arg(ra->right)));
      default:
        break;
    };
  die("[econ_cmplx] INVALID EXPR");
  return NULL;
}

C_Arg *econ_arg(R_Expr * r_expr) {
  if (r_expr)
    switch (r_expr->type) {
      case R_EXPR_NUM:
        return new_c_arg(C_NUM, new_c_num(((R_Num *) r_expr->expr)->num));
      case R_EXPR_VAR:
        return new_c_arg(C_VAR, new_c_var(((R_Var *) r_expr->expr)->name));
      default:
        break;
    };
  die("[econ_arg] INVALID EXPR");
  return NULL;
}

C_Program *uncover_locals(C_Program * cp) {
  if (cp) {
    Node *node =
      list_find(cp->labels, new_lbl_tail_pair("body", NULL), lbl_tail_cmp);
    if (node == NULL)
      die("[UNCOVER_LOCALS] NO BODY LABEL!");
    C_Tail *tail = ((lbl_tail_pair_t *) node->data)->tail;
    list_t vars = list_create();
    c_tail_extract_vars(tail, vars);
    return new_c_program(new_info(vars, NULL, NULL), cp->labels);
  }
  return cp;
}

X_Program *select_instr(C_Program * cp) {
  Node *node =
    list_find(cp->labels, new_lbl_tail_pair("body", NULL), lbl_tail_cmp);
  if (node == NULL)
    die("[SELECT_INSTR] NO BODY LABEL!");
  C_Tail *ct = ((lbl_tail_pair_t *) node->data)->tail;
  list_t x_instrs = select_instr_tail(ct);
  list_t lbls = list_create();
  lbl_blk_pair_t *lbl = new_lbl_blk_pair("body", new_x_block(NULL, x_instrs));
  list_insert(lbls, lbl);
  list_t ret_instr = list_create();
  list_insert(ret_instr, new_x_instr(RETQ, new_x_retq()));
  lbl_blk_pair_t *end_lbl =
    new_lbl_blk_pair("end", new_x_block(NULL, ret_instr));
  list_insert(lbls, end_lbl);
  return new_x_prog(cp->info, lbls);
}

X_Program *assign_homes(X_Program * xp) {
  if (xp && xp->info) {
    int offset = 1, variable_count;
    Info *info = xp->info;
    list_t bi = list_create(), ei = list_create(), new_instrs = list_create(),
      var_to_num = list_create(), lbls = list_create();

    lbl_blk_pair_t *begin_lbp =
      new_lbl_blk_pair("begin", new_x_block(NULL, bi));
    lbl_blk_pair_t *end_lbp = new_lbl_blk_pair("end", new_x_block(NULL, ei));
    lbl_blk_pair_t *body_lbp =
      new_lbl_blk_pair("body", new_x_block(NULL, new_instrs));

    list_insert(lbls, begin_lbp);
    list_insert(lbls, body_lbp);
    list_insert(lbls, end_lbp);

    variable_count = list_size(info->vars) * 8;
    variable_count =
      (variable_count % 16 == 0) ? variable_count : variable_count + 8;

    X_Arg *rbp = new_x_arg(X_ARG_REG, new_x_arg_reg(RBP));
    X_Arg *rsp = new_x_arg(X_ARG_REG, new_x_arg_reg(RSP));
    X_Arg *vc = new_x_arg(X_ARG_NUM, new_x_arg_num(variable_count));

    list_insert(bi, new_x_instr(PUSHQ, new_x_pushq(rbp)));
    list_insert(bi, new_x_instr(MOVQ, new_x_movq(rsp, rbp)));
    list_insert(bi, new_x_instr(SUBQ, new_x_subq(vc, rsp)));
    list_insert(bi, new_x_instr(JMP, new_x_jmp("body")));

    list_insert(ei, new_x_instr(ADDQ, new_x_addq(vc, rsp)));
    list_insert(ei, new_x_instr(POPQ, new_x_popq(rbp)));
    list_insert(ei, new_x_instr(RETQ, new_x_retq()));

    Node *node =
      list_find(xp->labels, new_lbl_blk_pair("body", NULL), lbl_blk_pair_cmp);
    if (node == NULL)
      die("[SELECT_INSTR] NO BODY LABEL!");
    X_Block *xb = ((lbl_blk_pair_t *) node->data)->block;

    node = *(info->vars);
    while (node != NULL) {
      list_insert(var_to_num, new_var_num_pair(node->data, offset++));
      node = node->next;
    }

    node = *(xb->instrs);
    while (node != NULL) {
      list_insert(new_instrs, assign_instr(node->data, var_to_num));
      node = node->next;
    }
    return new_x_prog(NULL, lbls);
  }
  die("[assign_homes] XP OR INFO IS NULL!");
  return NULL;
}

X_Program *patch_instrs(X_Program * xp) {
  Node *head, *node =
    list_find(xp->labels, new_lbl_blk_pair("body", NULL), lbl_blk_pair_cmp);
  if (node == NULL)
    die("PATCH_INSTRS] NO BODY LABEL!");
  list_t new_instrs = list_create(), lbls = list_create();
  head = *(((lbl_blk_pair_t *) node->data)->block->instrs);

  while (head != NULL) {
    patch_instr(head->data, new_instrs);
    head = head->next;
  }

  node =
    list_find(xp->labels, new_lbl_blk_pair("begin", NULL), lbl_blk_pair_cmp);
  if (node == NULL)
    die("PATCH_INSTRS] NO BEGIN LABEL!");

  list_insert(lbls, node->data);
  list_insert(lbls, new_lbl_blk_pair("body", new_x_block(NULL, new_instrs)));

  node = list_find(xp->labels, new_lbl_blk_pair("end", NULL), lbl_blk_pair_cmp);
  if (node == NULL)
    die("PATCH_INSTRS] NO END LABEL!");

  list_insert(lbls, node->data);

  return new_x_prog(NULL, lbls);
}

X_Program *main_pass(X_Program * xp) {
  X_Arg *rax = new_x_arg(X_ARG_REG, new_x_arg_reg(RAX));
  X_Arg *rdi = new_x_arg(X_ARG_REG, new_x_arg_reg(RDI));

  list_t main_instrs = list_create();
  list_insert(main_instrs, new_x_instr(CALLQ, new_x_callq("begin")));
  list_insert(main_instrs, new_x_instr(MOVQ, new_x_movq(rax, rdi)));
  list_insert(main_instrs, new_x_instr(CALLQ, new_x_callq("print_int")));
  list_insert(main_instrs, new_x_instr(RETQ, new_x_retq()));

  list_insert(xp->labels,
              new_lbl_blk_pair("main", new_x_block(NULL, main_instrs)));
  return xp;
}

void patch_instr(X_Instr * xp, list_t instrs) {
  X_Arg *left, *right, *tmp = new_x_arg(X_ARG_REG, new_x_arg_reg(TMP_REG));
  if (xp)
    switch (xp->type) {
      case ADDQ:
        left = ((X_Addq *) xp->instr)->left;
        right = ((X_Addq *) xp->instr)->right;
        if (left->type == X_ARG_MEM && right->type == X_ARG_MEM) {
          list_insert(instrs, new_x_instr(MOVQ, new_x_movq(left, tmp)));
          list_insert(instrs, new_x_instr(ADDQ, new_x_addq(tmp, right)));
        } else
          list_insert(instrs, xp);
        break;
      case MOVQ:
        left = ((X_Movq *) xp->instr)->left;
        right = ((X_Movq *) xp->instr)->right;
        if (left->type == X_ARG_MEM && right->type == X_ARG_MEM) {
          list_insert(instrs, new_x_instr(MOVQ, new_x_movq(left, tmp)));
          list_insert(instrs, new_x_instr(MOVQ, new_x_movq(tmp, right)));
        } else
          list_insert(instrs, xp);
        break;
      case CMPQ:
        left = ((X_Cmpq *) xp->instr)->left;
        right = ((X_Cmpq *) xp->instr)->right;
        if (left->type == X_ARG_MEM && right->type == X_ARG_MEM) {
          list_insert(instrs, new_x_instr(MOVQ, new_x_movq(left, tmp)));
          list_insert(instrs, new_x_instr(CMPQ, new_x_movq(tmp, right)));
        } else if (is_cons(right->type)) {
          list_insert(instrs, new_x_instr(MOVQ, new_x_movq(right, tmp)));
          list_insert(instrs, new_x_instr(CMPQ, new_x_cmpq(left, tmp)));
        } else
          list_insert(instrs, xp);
      case XORQ:
        left = ((X_Xorq *) xp->instr)->left;
        right = ((X_Xorq *) xp->instr)->right;
        if (left->type == X_ARG_MEM && right->type == X_ARG_MEM) {
          list_insert(instrs, new_x_instr(MOVQ, new_x_movq(left, tmp)));
          list_insert(instrs, new_x_instr(XORQ, new_x_xorq(tmp, right)));
        } else list_insert(instrs, xp);
      case MOVZBQ:
        left = ((X_Movzbq *) xp->instr)->left;
        right = ((X_Movzbq *) xp->instr)->right;
        if (left->type == X_ARG_MEM && right->type == X_ARG_MEM) {
          list_insert(instrs, new_x_instr(MOVQ, new_x_movq(left, tmp)));
          list_insert(instrs, new_x_instr(MOVZBQ, new_x_movzbq(tmp, right)));
        } else if (is_cons(right->type)) {
          list_insert(instrs, new_x_instr(MOVQ, new_x_movq(right, tmp)));
          list_insert(instrs, new_x_instr(MOVZBQ, new_x_movzbq(left, tmp)));
        } else
          list_insert(instrs, xp);
      default:
        list_insert(instrs, xp);
    };
}

X_Instr *assign_instr(X_Instr * xi, list_t map) {
  if (xi)
    switch (xi->type) {
      case ADDQ:
        return new_x_instr(ADDQ,
                           new_x_addq(assign_arg
                                      (((X_Addq *) xi->instr)->left, map),
                                      assign_arg(((X_Addq *) xi->instr)->right,
                                                 map)));
      case SUBQ:
        return new_x_instr(SUBQ,
                           new_x_subq(assign_arg
                                      (((X_Subq *) xi->instr)->left, map),
                                      assign_arg(((X_Subq *) xi->instr)->right,
                                                 map)));
      case MOVQ:
        return new_x_instr(MOVQ,
                           new_x_movq(assign_arg
                                      (((X_Movq *) xi->instr)->left, map),
                                      assign_arg(((X_Movq *) xi->instr)->right,
                                                 map)));
      case NEGQ:
        return new_x_instr(NEGQ,
                           new_x_negq(assign_arg
                                      (((X_Negq *) xi->instr)->arg, map)));
      default:
        return xi;
    };
  return NULL;
}

X_Arg *map_arg(X_Arg * xa, list_t map) {
  Node *node;
  if (xa)
    switch (xa->type) {
      case X_ARG_NUM:
        return xa;
      case X_ARG_VAR:
        node = list_find(map, new_x_arg_pair(xa, 0), x_arg_pair_cmp);
        if (node == NULL)
          die("[ASSIGN_ARG] VARIABLE MAPPING NOT FOUND!");
        return ((x_arg_pair_t *) node->data)->arg2;
      default:
        return xa;
    };
  return NULL;
}

X_Instr *map_instr(X_Instr * xi, list_t map) {
  if (xi)
    switch (xi->type) {
      case ADDQ:
        return new_x_instr(ADDQ,
                           new_x_addq(map_arg
                                      (((X_Addq *) xi->instr)->left, map),
                                      map_arg(((X_Addq *) xi->instr)->right,
                                              map)));
      case SUBQ:
        return new_x_instr(SUBQ,
                           new_x_subq(map_arg
                                      (((X_Subq *) xi->instr)->left, map),
                                      map_arg(((X_Subq *) xi->instr)->right,
                                              map)));
      case MOVQ:
        return new_x_instr(MOVQ,
                           new_x_movq(map_arg
                                      (((X_Movq *) xi->instr)->left, map),
                                      map_arg(((X_Movq *) xi->instr)->right,
                                              map)));
      case NEGQ:
        return new_x_instr(NEGQ,
                           new_x_negq(map_arg
                                      (((X_Negq *) xi->instr)->arg, map)));
      default:
        return xi;
    };
  return NULL;
}

X_Arg *assign_arg(X_Arg * xa, list_t map) {
  Node *node;
  if (xa)
    switch (xa->type) {
      case X_ARG_NUM:
        return xa;
      case X_ARG_VAR:
        node = list_find(map, new_var_num_pair(xa->arg, 0), var_num_pair_cmp);
        if (node == NULL)
          die("[ASSIGN_ARG] VARIABLE MAPPING NOT FOUND!");
        return new_x_arg(X_ARG_MEM,
                         new_x_arg_mem(RSP,
                                       8 *
                                       ((var_num_pair_t *) node->data)->num));
      default:
        return xa;
    };
  return NULL;
}

list_t select_instr_tail(C_Tail * ct) {
  list_t instrs_smt = list_create(), instrs_tail;
  X_Arg *rax;
  X_Jmp *xj;
  C_Cmp *c_cmp;
  C_Goto_If *cg;
  X_Cmpq *x_cmp;
  if (ct)
    switch (ct->type) {
      case C_TAIL_RET:
        rax = new_x_arg(X_ARG_REG, new_x_arg_reg(RAX));
        list_insert(instrs_smt,
                    new_x_instr(MOVQ,
                                new_x_movq(select_instr_arg
                                           (((C_Ret *) ct->tail)->arg), rax)));
        list_insert(instrs_smt, new_x_instr(JMP, new_x_jmp("end")));
        break;
      case C_TAIL_SEQ:
        instrs_smt = select_instr_smt(((C_Seq *) ct->tail)->smt);
        instrs_tail = select_instr_tail(((C_Seq *) ct->tail)->tail);
        return list_concat(instrs_smt, instrs_tail);
      case C_TAIL_GOTO:
        xj = new_x_jmp(((C_Goto *) ct->tail)->lbl);
        list_insert(instrs_smt, new_x_instr(JMP, xj));
        break;
      case C_TAIL_GOTO_IF:
        cg = ct->tail;
        c_cmp = cg->cmp->expr;
        x_cmp =
          new_x_cmpq(select_instr_arg(c_cmp->left),
                     select_instr_arg(c_cmp->right));
        list_insert(instrs_smt, new_x_instr(CMPQ, x_cmp));
        list_insert(instrs_smt,
                    new_x_instr(JMPIF,
                                new_x_jmpif(c_cmp->cmp_type, cg->true_lbl)));
        list_insert(instrs_smt, new_x_instr(JMP, new_x_jmp(cg->false_lbl)));
        break;
      default:
        break;
    }
  return instrs_smt;
}

list_t select_instr_smt(C_Smt * cs) {
  list_t res;
  if (cs) {
    C_Arg *ca = new_c_arg(C_VAR, cs->var);
    res = select_instr_expr(cs->expr, select_instr_arg(ca));
  }
  return res;
}

list_t select_instr_expr(C_Expr * ce, X_Arg * dst) {
  X_Arg *rax = new_x_arg(X_ARG_REG, new_x_arg_reg(RAX));
  X_Arg *al = new_x_arg(X_ARG_BYTE_REG, new_x_arg_byte_reg(RAX));
  X_Arg *res, *one;
  list_t instrs = list_create();
  if (ce)
    switch (ce->type) {
      case C_ARG:
        list_insert(instrs,
                    new_x_instr(MOVQ,
                                new_x_movq(select_instr_arg(ce->expr), dst)));
        break;
      case C_READ:

        list_insert(instrs, new_x_instr(CALLQ, new_x_callq(READ_INT)));
        list_insert(instrs, new_x_instr(MOVQ, new_x_movq(rax, dst)));
        break;
      case C_NEG:
        list_insert(instrs,
                    new_x_instr(MOVQ,
                                new_x_movq(select_instr_arg
                                           (((C_Neg *) ce->expr)->arg), dst)));
        list_insert(instrs, new_x_instr(NEGQ, new_x_negq(dst)));
        break;
      case C_ADD:
        list_insert(instrs,
                    new_x_instr(MOVQ,
                                new_x_movq(select_instr_arg
                                           (((C_Add *) ce->expr)->right),
                                           dst)));
        list_insert(instrs,
                    new_x_instr(ADDQ,
                                new_x_addq(select_instr_arg
                                           (((C_Add *) ce->expr)->left), dst)));
        break;
      case C_NOT:
        one = new_x_arg(X_ARG_NUM, new_x_arg_num(1));
        res = select_instr_arg(((C_Not *) ce->expr)->arg);
        list_insert(instrs, new_x_instr(MOVQ, new_x_movq(res, dst)));
        list_insert(instrs, new_x_instr(XORQ, new_x_xorq(one, dst)));
        break;
      case C_CMP:
        one = select_instr_arg(((C_Cmp *) ce->expr)->left);
        res = select_instr_arg(((C_Cmp *) ce->expr)->right);
        list_insert(instrs, new_x_instr(CMPQ, new_x_cmpq(res, one)));
        list_insert(instrs,
                    new_x_instr(SETCC,
                                new_x_setcc(((C_Cmp *) ce->expr)->cmp_type,
                                            al)));
        list_insert(instrs, new_x_instr(MOVZBQ, new_x_movzbq(al, dst)));
        break;
      default:
        break;
    };
  return instrs;
}

X_Arg *select_instr_arg(C_Arg * ca) {
  if (ca)
    switch (ca->type) {
      case C_NUM:
        return new_x_arg(X_ARG_NUM, new_x_arg_num(((C_Num *) ca->arg)->num));
      case C_VAR:
        return new_x_arg(X_ARG_VAR, new_x_arg_var(((C_Var *) ca->arg)->name));
      case C_TRUE:
        return new_x_arg(X_ARG_NUM, new_x_arg_num(1));
      case C_FALSE:
        return new_x_arg(X_ARG_NUM, new_x_arg_num(0));
      default:
        break;
    };
  die("[SELECT_INSTR_ARG] INVALID CA");
  return NULL;
}
