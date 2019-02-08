#include "list.h"
#include "utils.h"
#include "x.h"

const char* registers[NUM_REGS] = { "%rsp", "%rbp", "%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%rdi", 
                                    "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"};

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

State* new_state(){
    State *s = malloc_or_die(sizeof(State));
    s->regs = list_create();
    s->nums = list_create();
    s->vars = list_create();
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
