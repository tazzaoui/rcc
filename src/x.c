#include <string.h>
#include "pairs.h"
#include "list.h"
#include "utils.h"
#include "x.h"

const char *registers[NUM_REGS] = {
  "%rsp", "%rbp", "%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%rdi",
  "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"
};

X_Program *new_x_prog(void *info, list_t labels) {
  X_Program *xp = malloc_or_die(sizeof(X_Program));
  xp->info = info;
  if (labels == NULL)
    labels = list_create();
  xp->labels = labels;
  return xp;
}

X_Block *new_x_block(void *info, list_t instrs) {
  X_Block *b = malloc_or_die(sizeof(X_Block));
  b->info = info;
  if (instrs == NULL)
    instrs = list_create();
  b->instrs = instrs;
  return b;
}

X_Instr *new_x_instr(X_INSTR_TYPE type, void *instr) {
  X_Instr *i = malloc_or_die(sizeof(X_Instr));
  i->type = type;
  i->instr = instr;
  return i;
}

X_State *new_x_state(list_t lbls) {
  X_State *s = malloc_or_die(sizeof(X_State));
  for (int i = 0; i < NUM_REGS; ++i)
    s->regs[i] = 0;

  s->nums = list_create();
  s->vars = list_create();
  s->lbls = lbls;
  return s;
}

X_Arg *new_x_arg(X_ARG_TYPE type, void *arg) {
  X_Arg *a = malloc_or_die(sizeof(X_Arg));
  a->type = type;
  a->arg = arg;
  return a;
}

X_Addq *new_x_addq(X_Arg * left, X_Arg * right) {
  X_Addq *a = malloc_or_die(sizeof(X_Addq));
  a->left = left;
  a->right = right;
  return a;
}

X_Subq *new_x_subq(X_Arg * left, X_Arg * right) {
  X_Subq *s = malloc_or_die(sizeof(X_Subq));
  s->left = left;
  s->right = right;
  return s;
}

X_Movq *new_x_movq(X_Arg * left, X_Arg * right) {
  X_Movq *m = malloc_or_die(sizeof(X_Movq));
  m->left = left;
  m->right = right;
  return m;
}

X_Retq *new_x_retq() {
  return malloc_or_die(sizeof(X_Retq));
}

X_Negq *new_x_negq(X_Arg * arg) {
  X_Negq *n = malloc_or_die(sizeof(X_Negq));
  n->arg = arg;
  return n;
}

X_Callq *new_x_callq(label_t label) {
  X_Callq *c = malloc_or_die(sizeof(X_Callq));
  c->label = label;
  return c;
}

X_Jmp *new_x_jmp(label_t label) {
  X_Jmp *j = malloc_or_die(sizeof(X_Jmp));
  j->label = label;
  return j;
}

X_Pushq *new_x_pushq(X_Arg * arg) {
  X_Pushq *p = malloc_or_die(sizeof(X_Pushq));
  p->arg = arg;
  return p;
}

X_Popq *new_x_popq(X_Arg * arg) {
  X_Popq *p = malloc_or_die(sizeof(X_Popq));
  p->arg = arg;
  return p;
}

X_Xorq *new_x_xorq(X_Arg * left, X_Arg * right) {
  X_Xorq *q = malloc_or_die(sizeof(X_Xorq));
  q->left = left;
  q->right = right;
  return q;
}

X_Cmpq *new_x_cmpq(X_Arg * left, X_Arg * right) {
  X_Cmpq *c = malloc_or_die(sizeof(X_Cmpq));
  c->left = left;
  c->right = right;
  return c;
}

X_Setcc *new_x_setcc(X_CC_TYPE cc, X_Arg * arg) {
  X_Setcc *c = malloc_or_die(sizeof(X_Setcc));
  c->cc = cc;
  c->arg = arg;
  return c;
}

X_Movzbq *new_x_movzbq(X_Arg * left, X_Arg * right) {
  X_Movzbq *m = malloc_or_die(sizeof(X_Movzbq));
  m->left = left;
  m->right = right;
  return m;
}

X_Jmpif *new_x_jmpif(X_CC_TYPE cc, X_Arg * arg) {
  X_Jmpif *j = malloc_or_die(sizeof(X_Jmpif));
  j->cc = cc;
  j->arg = arg;
  return j;
}

X_Arg_Num *new_x_arg_num(int n) {
  X_Arg_Num *an = malloc_or_die(sizeof(X_Arg_Num));
  an->num = n;
  return an;
}

X_Arg_Reg *new_x_arg_reg(REGISTER reg) {
  X_Arg_Reg *ar = malloc_or_die(sizeof(X_Arg_Reg));
  ar->reg = reg;
  return ar;
}

X_Arg_Byte_Reg *new_x_arg_byte_reg(REGISTER reg) {
  X_Arg_Byte_Reg *br = malloc_or_die(sizeof(X_Arg_Byte_Reg));
  br->reg = reg;
  return br;
}

X_Arg_Mem *new_x_arg_mem(REGISTER reg, int offset) {
  X_Arg_Mem *am = malloc_or_die(sizeof(X_Arg_Mem));
  am->reg = reg;
  am->offset = offset;
  return am;
}

X_Arg_Var *new_x_arg_var(const char *name) {
  X_Arg_Var *av = malloc_or_die(sizeof(X_Arg_Var));
  av->name = name;
  return av;
}

void x_emit(X_Program * xp, const char *file_name) {
  if (xp) {
    if (file_name)
      freopen(file_name, "w", stdout);
    printf("\t.globl main\n");
    list_print(xp->labels, print_lbl_blk_pair);
    if (file_name)
      freopen("/dev/tty", "w", stdout);
  }
}

void print_x_instr(void *instr) {
  if (instr) {
    X_Instr *i = (X_Instr *) instr;
    switch (i->type) {
      case ADDQ:
        printf("\taddq\t");
        x_print_arg(((X_Addq *) i->instr)->left);
        printf(", ");
        x_print_arg(((X_Addq *) i->instr)->right);
        break;
      case SUBQ:
        printf("\tsubq\t");
        x_print_arg(((X_Subq *) i->instr)->left);
        printf(", ");
        x_print_arg(((X_Subq *) i->instr)->right);
        break;
      case MOVQ:
        printf("\tmovq\t");
        x_print_arg(((X_Movq *) i->instr)->left);
        printf(", ");
        x_print_arg(((X_Movq *) i->instr)->right);
        break;
      case RETQ:
        printf("\tretq\t");
        break;
      case NEGQ:
        printf("\tnegq\t");
        x_print_arg(((X_Negq *) i->instr)->arg);
        break;
      case CALLQ:
        printf("\tcallq\t%s", ((X_Callq *) i->instr)->label);
        break;
      case JMP:
        printf("\tjmp\t%s", ((X_Jmp *) i->instr)->label);
        break;
      case PUSHQ:
        printf("\tpushq\t");
        x_print_arg(((X_Pushq *) i->instr)->arg);
        break;
      case POPQ:
        printf("\tpopq\t");
        x_print_arg(((X_Popq *) i->instr)->arg);
        break;
      default:
        die("Invalid Instruction!");
    };
    printf("\n");
  }
}

void x_print_arg(X_Arg * arg) {
  if (arg) {
    switch (arg->type) {
      case X_ARG_NUM:
        printf("$%d", ((X_Arg_Num *) arg->arg)->num);
        break;
      case X_ARG_REG:
        printf("%s", registers[((X_Arg_Reg *) arg->arg)->reg]);
        break;
      case X_ARG_MEM:
        printf("%d(%s)", ((X_Arg_Mem *) arg->arg)->offset,
               registers[((X_Arg_Mem *) arg->arg)->reg]);
        break;
      case X_ARG_VAR:
        if (X_PRINT_ARG_ALLOW_VARS)
          printf("!%s", ((X_Arg_Var *) arg->arg)->name);
        break;
      default:
        die("Invalid Arg!");
    }
  }
}

int x_interp(X_Program * xp) {
  X_State *ms = new_x_state(xp->labels);
  Node *main_node =
    list_find(ms->lbls, new_lbl_blk_pair("main", NULL), lbl_blk_pair_cmp);
  Node *body_node =
    list_find(ms->lbls, new_lbl_blk_pair("body", NULL), lbl_blk_pair_cmp);
  if (main_node == NULL && body_node == NULL)
    die("[X_INTERP] NO MAIN OR BODY BLOCK!");
  return x_blk_interp(body_node == NULL ? "main" : "body", &ms);
}

int x_blk_interp(label_t lbl, X_State ** ms) {
  if (ms) {
    Node *n =
      list_find((*ms)->lbls, new_lbl_blk_pair(lbl, NULL), lbl_blk_pair_cmp);
    if (n) {
      X_Block *b = ((lbl_blk_pair_t *) n->data)->block;
      list_t instrs = b->instrs;
      return x_instrs_interp(instrs, ms);
    } else {
      printf("\n%s\n", lbl);
      die("x_blk_interp: label not found!");
    }
  }

  die("x_blk_interp: NULL ms");
  return I32MIN;
}

int x_instrs_interp(list_t instrs, X_State ** ms) {
  if (instrs && ms) {
    int val;
    X_Instr *instr;
    Node *ins_node = *instrs;
    while (ins_node != NULL) {
      instr = (X_Instr *) ins_node->data;
      val = x_instr_interp(instr, ms);
      if (instr->type == RETQ || instr->type == JMP)
        return val;
      ins_node = ins_node->next;
    }
  }
  return I32MIN;
}

int x_instr_interp(X_Instr * instr, X_State ** ms) {
  if (instr && ms) {
    int lval, rval;
    X_State *s = *ms;
    label_t lbl;
    X_Arg *mem_rsp = new_x_arg(X_ARG_MEM, new_x_arg_mem(RSP, 0));
    X_Arg *rsp = new_x_arg(X_ARG_REG, new_x_arg_reg(RSP));
    switch (instr->type) {
      case ADDQ:
        lval = lookup_state(s, ((X_Addq *) instr->instr)->left);
        rval = lookup_state(s, ((X_Addq *) instr->instr)->right);
        rval += lval;
        update_state(ms, ((X_Addq *) instr->instr)->right, rval);
        return rval;
      case SUBQ:
        lval = lookup_state(s, ((X_Subq *) instr->instr)->left);
        rval = lookup_state(s, ((X_Subq *) instr->instr)->right);
        rval -= lval;
        update_state(ms, ((X_Subq *) instr->instr)->right, rval);
        return rval;
      case NEGQ:
        lval = lookup_state(s, ((X_Negq *) instr->instr)->arg);
        lval *= -1;
        update_state(ms, ((X_Negq *) instr->instr)->arg, lval);
        return lval;
      case MOVQ:
        lval = lookup_state(s, ((X_Movq *) instr->instr)->left);
        update_state(ms, ((X_Movq *) instr->instr)->right, lval);
        rval = lookup_state(s, ((X_Movq *) instr->instr)->right);
        return rval;
      case PUSHQ:
        lval = lookup_state(s, rsp);
        rval = lookup_state(s, ((X_Pushq *) instr->instr)->arg);
        update_state(ms, rsp, lval - 8);
        update_state(ms, mem_rsp, rval);
        return rval;
      case POPQ:
        lval = lookup_state(s, rsp);
        rval = lookup_state(s, mem_rsp);
        update_state(ms, ((X_Popq *) instr->instr)->arg, rval);
        update_state(ms, rsp, lval + 8);
        return rval;
      case JMP:
        lbl = ((X_Jmp *) instr->instr)->label;
        return x_blk_interp(lbl, ms);
      case RETQ:
        return lookup_state(s, new_x_arg(X_ARG_REG, new_x_arg_reg(RAX)));
      case CALLQ:
        lbl = ((X_Callq *) instr->instr)->label;
        lval = 7;
        if (strcmp(lbl, READ_INT) == 0) {
          if (!QUIET_READ)
            scanf("%d", &lval);
          s->regs[RAX] = lval;
        }
        return lval;
      default:
        break;
    };
  }
  die("Invalid x_instr_interp!");
  return I32MIN;
}

int update_state(X_State ** s, X_Arg * arg, int val) {
  if (s && arg) {
    int old, addr;
    Node *n;
    X_State *ms = *s;
    num_pair_t *new_np;
    var_num_pair_t *new_vnp;
    switch (arg->type) {
      case X_ARG_NUM:
        return ((X_Arg_Num *) arg->arg)->num;
      case X_ARG_REG:
        old = ms->regs[((X_Arg_Reg *) arg->arg)->reg];
        ms->regs[((X_Arg_Reg *) arg->arg)->reg] = val;
        return old;
      case X_ARG_MEM:
        addr = ms->regs[((X_Arg_Mem *) arg->arg)->reg] +
          ((X_Arg_Mem *) arg->arg)->offset;
        new_np = new_num_pair(addr, val);
        n = list_find(ms->nums, new_np, num_pair_cmp);
        if (n == NULL) {
          list_insert(ms->nums, new_np);
          return 0;
        } else {
          old = ((num_pair_t *) n->data)->n2;
          list_update(ms->nums, n->data, new_np, num_pair_cmp);
          return old;
        }
      case X_ARG_VAR:
        new_vnp = new_var_num_pair(((X_Arg_Var *) arg->arg), val);
        n = list_find(ms->vars, new_vnp, var_num_pair_cmp);
        if (n == NULL) {
          list_insert(ms->vars, new_vnp);
          return 0;
        } else {
          old = ((var_num_pair_t *) n->data)->num;
          list_update(ms->vars, n->data, new_vnp, var_num_pair_cmp);
          return old;
        }
      default:
        break;
    };
  }

  die("Invalid State Lookup!");
  return I32MIN;
}

int lookup_state(X_State * ms, X_Arg * arg) {
  if (ms && arg) {
    int val;
    Node *n;
    switch (arg->type) {
      case X_ARG_NUM:
        return ((X_Arg_Num *) arg->arg)->num;
      case X_ARG_REG:
        return ms->regs[((X_Arg_Reg *) arg->arg)->reg];
      case X_ARG_MEM:
        val = ms->regs[((X_Arg_Mem *) arg->arg)->reg] +
          ((X_Arg_Mem *) arg->arg)->offset;
        n = list_find(ms->nums, new_num_pair(val, 0), num_pair_cmp);
        if (n != NULL)
          return ((num_pair_t *) n->data)->n2;
        return I32MIN;
      case X_ARG_VAR:
        n =
          list_find(ms->vars, new_var_num_pair(arg->arg, 0), var_num_pair_cmp);
        if (n != NULL)
          return ((var_num_pair_t *) n->data)->num;
        return I32MIN;
      default:
        break;
    };
  }
  die("Invalid State Lookup!");
  return I32MIN;
}
