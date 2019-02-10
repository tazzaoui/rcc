#include <string.h>
#include "pairs.h"
#include "list.h"
#include "utils.h"
#include "x.h"

const char* registers[NUM_REGS] = { 
    "%rsp", "%rbp", "%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%rdi", 
    "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"
};

X_Program* new_prog(void* info, list_t labels) {
  X_Program* xp = malloc_or_die(sizeof(X_Program));
  xp->info = info;
  if(labels == NULL)
      labels = list_create();
  xp->labels = labels;
  return xp;
}

Block* new_block(void* info, list_t instrs){
  Block* b = malloc_or_die(sizeof(Block));
  b->info = info;
  if(instrs == NULL)
      instrs = list_create();
  b->instrs = instrs;
  return b;
}

Instr* new_instr(INSTR_TYPE type, void* instr){
    Instr *i = malloc_or_die(sizeof(Instr));
    i->type = type;
    i->instr = instr;
    return i;
}

State* new_state(list_t lbls){
    State *s = malloc_or_die(sizeof(State));
    for(int i = 0; i < NUM_REGS; ++i)
        s->regs[i] = 0;
    
    s->nums = list_create();
    s->vars = list_create();
    s->lbls = lbls;
    return s;
}

Arg* new_arg(ARG_TYPE type, void* arg){
    Arg* a = malloc_or_die(sizeof(Arg));
    a->type = type;
    a->arg = arg;
    return a;
}

Addq* new_addq(Arg* left, Arg* right){
    Addq *a = malloc_or_die(sizeof(Addq));
    a->left = left;
    a->right = right;
    return a;
}

Subq* new_subq(Arg* left, Arg* right){
    Subq *s = malloc_or_die(sizeof(Subq));
    s->left = left;
    s->right = right;
    return s;
}

Movq* new_movq(Arg* left, Arg* right){
    Movq *m = malloc_or_die(sizeof(Movq));
    m->left = left;
    m->right = right;
    return m;
}

Retq* new_retq(){
    return malloc_or_die(sizeof(Retq));
}

Negq* new_negq(Arg* arg){
    Negq *n = malloc_or_die(sizeof(Negq));
    n->arg = arg;
    return n;
}

Callq* new_callq(label_t label){
    Callq *c = malloc_or_die(sizeof(Callq));
    c->label = label;
    return c;
}

Jmp* new_jmp(label_t label){
   Jmp *j = malloc_or_die(sizeof(Jmp));
   j->label = label;
   return j;
}

Pushq* new_pushq(Arg* arg){
    Pushq *p = malloc_or_die(sizeof(Pushq));
    p->arg = arg;
    return p;
}

Popq* new_popq(Arg* arg){
    Popq *p = malloc_or_die(sizeof(Popq));
    p->arg = arg;
    return p;
}

Arg_Num* new_arg_num(int n){
    Arg_Num *an = malloc_or_die(sizeof(Arg_Num));
    an->num = n;
    return an;
}


Arg_Reg* new_arg_reg(REGISTER reg){
    Arg_Reg *ar = malloc_or_die(sizeof(Arg_Reg));
    ar->reg = reg;
    return ar;
}

Arg_Mem* new_arg_mem(REGISTER reg, int offset){
    Arg_Mem *am = malloc_or_die(sizeof(Arg_Mem));
    am->reg = reg;
    am->offset = offset;
    return am;
}

Arg_Var* new_arg_var(const char* name){
    Arg_Var *av = malloc_or_die(sizeof(Arg_Var));
    av->name = name;
    return av;
}

void x_emit(X_Program *xp, const char* file_name){
    if(xp){
        if(file_name)
            freopen(file_name, "w", stdout);
        printf("\t.globl main\n");
        list_print(xp->labels, print_lbl_blk_pair);
        if(file_name) 
            freopen("/dev/tty", "w", stdout);
    }
}

void print_instr(void *instr){
    if(instr){
        Instr *i = (Instr*) instr;
        switch (i->type){
            case ADDQ:
                printf("\taddq\t");
                x_print_arg(((Addq*)i->instr)->left);
                printf(", ");
                x_print_arg(((Addq*)i->instr)->right);
                break;
            case SUBQ:
                printf("\tsubq\t");
                x_print_arg(((Subq*)i->instr)->left);
                printf(", ");
                x_print_arg(((Subq*)i->instr)->right);
                break;
            case MOVQ:
                printf("\tmovq\t");
                x_print_arg(((Movq*)i->instr)->left);
                printf(", ");
                x_print_arg(((Movq*)i->instr)->right);
                break;
            case RETQ:
                printf("\tretq\t");
                break;
            case NEGQ:
                printf("\tnegq\t");
                x_print_arg(((Negq*)i->instr)->arg);
                break;
            case CALLQ:
                printf("\tcallq\t%s", ((Callq*)i->instr)->label);
                break;
             case JMP:
                printf("\tjmp\t%s", ((Jmp*)i->instr)->label);
                break; 
            case PUSHQ:
                printf("\tpushq\t");
                x_print_arg(((Pushq*)i->instr)->arg);
                break;
            case POPQ:
                printf("\tpopq\t");
                x_print_arg(((Popq*)i->instr)->arg);
                break; 
            default:
                die("Invalid Instruction!");
        };
        printf("\n");
    }
}

void x_print_arg(Arg* arg){
    if(arg){
        switch(arg->type){
            case ARG_NUM:
                printf("$%d", ((Arg_Num*)arg->arg)->num);
                break;
            case ARG_REG:
                printf("%s", registers[((Arg_Reg*)arg->arg)->reg]);
                break; 
            case ARG_MEM:
                printf("%d(%s)", ((Arg_Mem*)arg)->offset, 
                                 registers[((Arg_Mem*)arg->arg)->reg]);
                break; 
            case ARG_VAR:
                if(X_PRINT_ARG_ALLOW_VARS)
                    printf("!%s", ((Arg_Var*)arg->arg)->name);
                break;
            default:
                die("Invalid Arg!");
        }
    }
}

int x_interp(X_Program *xp){
    State *ms = new_state(xp->labels);
    return x_blk_interp("main", &ms);
}

int x_blk_interp(label_t lbl, State **ms){
    if(ms){
        Node *n = list_find((*ms)->lbls, new_lbl_blk_pair(lbl, NULL), lbl_blk_pair_cmp);
        if(n){
            Block *b = ((lbl_blk_pair_t*) n->data)->block;
            list_t instrs = b->instrs;
            if(n) return x_instrs_interp(instrs, ms); 
        } else die("x_blk_interp: label not found!");
    }
    return I32MIN;
}

int x_instrs_interp(list_t instrs, State **ms){
    if(instrs && ms){ 
        int val;
        Instr *instr;
        Node *ins_node =  *instrs;
        while(ins_node != NULL){
            instr = (Instr*) ins_node->data;
            val = x_instr_interp(instr, ms);
            if(instr->type == RETQ)
                return val;
            ins_node = ins_node->next;
        }
    }
    return I32MIN;
}

int x_instr_interp(Instr *instr, State **ms){ 
    if(instr && ms){
        int lval, rval;
        State *s = *ms;
        label_t lbl;
        Arg *mem_rsp = new_arg(ARG_MEM, new_arg_mem(RSP, 0)); 
        Arg *rsp = new_arg(ARG_REG, new_arg_reg(RSP));
        switch(instr->type){
            case ADDQ:
                lval = lookup_state(s, ((Addq*)instr->instr)->left);
                rval = lookup_state(s, ((Addq*)instr->instr)->right);
                rval += lval;
                update_state(ms, ((Addq*)instr->instr)->right, rval);
                return rval;
            case SUBQ:
                lval = lookup_state(s, ((Subq*)instr->instr)->left);
                rval = lookup_state(s, ((Subq*)instr->instr)->right);
                rval -= lval;
                update_state(ms, ((Subq*)instr->instr)->right, rval);
                return rval;
            case NEGQ:
                lval = lookup_state(s, ((Negq*)instr->instr)->arg);
                lval *= -1;
                update_state(ms, ((Negq*)instr->instr)->arg, lval);
                return lval;
            case MOVQ:
                lval = lookup_state(s, ((Movq*)instr->instr)->left);
                update_state(ms, ((Movq*)instr->instr)->right, lval);
                rval = lookup_state(s, ((Movq*)instr->instr)->right);
                return rval;
            case PUSHQ:
                lval = lookup_state(s, rsp);
                rval = lookup_state(s, ((Pushq*)instr->instr)->arg);
                update_state(ms, rsp, lval - 8);
                update_state(ms, mem_rsp , rval);
                return rval; 
            case POPQ:
                lval = lookup_state(s, rsp);
                rval = lookup_state(s, mem_rsp);
                update_state(ms, ((Popq*)instr->instr)->arg, rval);
                update_state(ms, rsp, lval + 8);
                return rval;
             case JMP:
                lbl = ((Jmp*)instr->instr)->label; 
                return x_blk_interp(lbl, ms);
             case RETQ:
                return lookup_state(s, new_arg(ARG_REG, new_arg_reg(RAX)));
             case CALLQ: 
                lbl = ((Callq*)instr->instr)->label; 
                lval = GET_RAND();
                if(strcmp(lbl, "_read_int") == 0){
                    if(!QUIET_READ)
                        scanf("%d", &lval);
                    s->regs[RAX] = lval;
                }
                return lval;
             default:
                die("Invalid x_instr_interp!");
        };
    }
    return -1;
}

int update_state(State** s, Arg* arg, int val){
    if(s && arg){
        int old, addr; 
        Node *n;
        State *ms = *s;  
        num_pair_t *new_np;
        var_num_pair_t  *new_vnp;
        switch(arg->type){
            case ARG_NUM:
                return ((Arg_Num*)arg->arg)->num; 
            case ARG_REG:
                old = ms->regs[((Arg_Reg*)arg->arg)->reg];
                ms->regs[((Arg_Reg*)arg->arg)->reg] = val;
                return old;
            case ARG_MEM:
                addr = ms->regs[((Arg_Mem*)arg->arg)->reg] + 
                     ((Arg_Mem*)arg->arg)->offset;
                new_np = new_num_pair(addr, val);
                n = list_find(ms->nums, new_np, num_pair_cmp);
                if(n == NULL){
                    list_insert(ms->nums, new_np);
                    return 0;
                }else {
                    old = ((num_pair_t*)n->data)->n2;
                    list_update(ms->nums, n, new_np, num_pair_cmp);
                    return old;
                }
            case ARG_VAR:
                new_vnp = new_var_num_pair(((Arg_Var*)arg->arg), val);
                n = list_find(ms->vars, new_vnp, var_num_pair_cmp);
                if(n == NULL){
                    list_insert(ms->vars, new_vnp);
                    return 0;
                }else {
                    old = ((var_num_pair_t*)n->data)->num; 
                    list_update(ms->vars, n, new_vnp, var_num_pair_cmp);
                    return old;
                }
            default:
                die("Invalid State Update!");
        };
    }
    return I32MIN;
}

int lookup_state(State* ms, Arg* arg){
    if(ms && arg){
        int val;
        Node *n;
        switch(arg->type){
            case ARG_NUM:
                return ((Arg_Num*)arg->arg)->num;
            case ARG_REG:
                return ms->regs[((Arg_Reg*)arg->arg)->reg];
            case ARG_MEM:
                val = ms->regs[((Arg_Mem*)arg->arg)->reg] + 
                     ((Arg_Mem*)arg->arg)->offset;
                n = list_find(ms->nums, new_num_pair(val, 0), num_pair_cmp);
                if(n != NULL)
                    return ((num_pair_t*)n->data)->n2;
                return I32MIN; 
            case ARG_VAR:
                n = list_find(ms->vars, new_var_num_pair(arg->arg, 0), var_num_pair_cmp);
                if(n != NULL)
                    return ((var_num_pair_t*)n->data)->num;
                return I32MIN;
            default:
                die("Invalid State Lookup!");
        };
    }
    return I32MIN;
}
