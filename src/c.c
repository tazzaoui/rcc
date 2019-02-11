#include "utils.h"
#include "pairs.h"
#include "list.h"
#include "c.h"

C_Program* new_c_program(void* info, list_t labels){
  C_Program* cp = malloc_or_die(sizeof(C_Program));
  cp->info = info;
  if(labels == NULL)
      labels = list_create();
  cp->labels = labels;
  return cp;
}

C_Tail* new_c_tail(C_TAIL_TYPE type, void* tail){
  C_Tail* ct = malloc_or_die(sizeof(C_Tail));
  ct->type = type;
  ct->tail = tail;
  return ct;
}

C_Smt* new_c_smt(C_Var* var, C_Expr* expr){
  C_Smt* cs = malloc_or_die(sizeof(C_Smt));
  cs->var = var;
  cs->expr = expr;
  return cs;
}

C_Expr* new_c_expr(C_EXPR_TYPE type, void* expr){
    C_Expr *ce = malloc_or_die(sizeof(C_Expr));
    ce->type = type;
    ce->expr = expr;
    return ce;
}

C_Arg* new_c_arg(C_ARG_TYPE type, void* arg){
  C_Arg *ca = malloc_or_die(sizeof(C_Arg));
  ca->type = type;
  ca->arg = arg;
  return arg;
}

C_Num* new_c_num(int n){
  C_Num *cn = malloc_or_die(sizeof(C_Num));
  cn->num = n;
  return cn;
}

C_Var* new_c_var(const char* name){
    C_Var *cv = malloc_or_die(sizeof(C_Var));
    cv->name = name;
    return cv;
}

C_Ret* new_c_ret(C_Arg* arg){
    C_Ret *cr = malloc_or_die(sizeof(C_Ret));
    cr->arg = arg;
    return cr;
}

C_Seq* new_c_seq(C_Smt* cs, C_Tail* ct){
    C_Seq *seq = malloc_or_die(sizeof(C_Seq));
    seq->smt = cs;
    seq->tail = ct;
    return seq;
}

C_Read* new_c_read(C_Arg* ca){
    C_Read *cr = malloc_or_die(sizeof(C_Read));
    cr->arg = ca;
    return cr;
}

C_Neg* new_c_neg(C_Arg* arg){
    C_Neg *cn = malloc_or_die(sizeof(C_Neg));
    cn->arg = arg;
    return cn;
}

C_Add* new_c_add(C_Arg* left, C_Arg* right){
    C_Add *ca = malloc_or_die(sizeof(C_Add));
    ca->left = left;
    ca->right = right;
    return ca;
}

void c_print_smt(C_Smt *cs){
    if(cs){
        printf("(set! %s ", ((C_Var*)cs->var)->name);
        c_print_expr(cs->expr);
    }
}

void c_print_tail(C_Tail *ct){
    if(ct)
        switch(ct->type){
            case C_TAIL_RET:
                printf("ret ");
                c_print_arg(((C_Ret*)ct->tail)->arg);
                printf("\n");
                break;
            case C_TAIL_SEQ:
                printf("(seq\n");
                c_print_smt(((C_Seq*)ct->tail)->smt);
                c_print_tail(((C_Seq*)ct->tail)->tail);
                printf(")\n");
                break;
            default:
                die("Invalid c_print_tail!");
                break;
        };
}

void c_print_arg(C_Arg* ca){
    if(ca)
        switch(ca->type){
            case C_NUM:
                printf("%d", ((C_Num*) ca->arg)->num);
                break;
            case C_VAR:
                printf("%s", ((C_Var*)ca->arg)->name);
                break;
            default:
                die("Invalid c_print_arg!");
        }; 
}

void c_print_expr(C_Expr *ce){
    if(ce)
        switch(ce->type){
            case C_ARG:
                c_print_arg(ce->expr);
                printf("\n");
                break;
            case C_READ:
                printf("READ\n");
                break;
            case C_NEG:
                printf("(- ");
                c_print_arg(ce->expr);
                printf(")\n");
                break;
            case C_ADD:
                printf("(+ ");
                c_print_arg(((C_Add*)ce->expr)->left);
                printf(" ");
                c_print_arg(((C_Add*)ce->expr)->right);
                printf(")\n");
                break;
            default:
                die("Invalid c_expr_print!");
                break;
        };
}

void c_print(C_Program *cp){
    if(cp) list_print(cp->labels, lbl_tail_print);
}
