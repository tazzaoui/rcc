#include <string.h>
#include "pairs.h"
#include "utils.h"
#include "r.h"
#include "x.h"
#include "rcc.h"

env_pair_t *new_env_pair(Expr* var, Expr* val){
    env_pair_t *ep = malloc_or_die(sizeof(env_pair_t));
    ep->var = var;
    ep->val = val;
    return ep;
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
    return old ? (void*) new_env_pair(old_pair->var, old_pair->val) : NULL;
}

void ep_print(void *data){
    if(data){
        env_pair_t *ep = (env_pair_t*)data;
        printf("<");
        print(ep->var);
        printf(",");
        print(ep->val);
        printf(">");
    }
}

lbl_blk_pair_t* new_lbl_blk_pair(label_t label, Block* blk){
    lbl_blk_pair_t *lbp = malloc_or_die(sizeof(lbl_blk_pair_t));
    lbp->label = label;
    lbp->block = blk;
    return lbp;
}

int lbl_blk_pair_cmp(void *a, void *b){
    if(a && b){        
        lbl_blk_pair_t *lbp_a = (lbl_blk_pair_t*) a;
        lbl_blk_pair_t *lbp_b = (lbl_blk_pair_t*) b;
        return strcmp(lbp_a->label, lbp_b->label) == 0; 
    }
    return 0;
}

void print_lbl_blk_pair(void* l){
    if(l){
        lbl_blk_pair_t *lbp = (lbl_blk_pair_t*)l;
        Block* blk = (Block*) lbp->block; 
        printf("%s:\n", lbp->label);
        list_print(blk->instrs, print_instr); 
    }
}

reg_num_pair_t* new_reg_num_pair(REGISTER reg, int num){
    reg_num_pair_t *rnp = malloc_or_die(sizeof(reg_num_pair_t));
    rnp->reg = reg;
    rnp->num = num;
    return rnp;
}

int reg_num_pair_cmp(void *a, void *b){
    if(a && b){
        reg_num_pair_t *rnp_a = (reg_num_pair_t*) a;
        reg_num_pair_t *rnp_b = (reg_num_pair_t*) b;
        return (rnp_a->reg == rnp_b->reg && rnp_a->num == rnp_b->num);
    }
    return 0;
}

void* reg_num_pair_cpy(void *old){
    reg_num_pair_t *r = (reg_num_pair_t*) old;
    return old ? new_reg_num_pair(r->reg, r->num) : NULL; 
}

void print_reg_num_pair(void *rnp){
    if(rnp){
        reg_num_pair_t *r = (reg_num_pair_t*) rnp;
        printf("<%s, %d>", registers[r->reg], r->num);
    }
}

num_pair_t* new_num_pair(int n1, int n2){
    num_pair_t *np = malloc_or_die(sizeof(num_pair_t));
    np->n1 = n1;
    np->n2 = n2;
    return np;
}

int num_pair_cmp(void *a, void *b){
    if(a && b){
        num_pair_t *np_a = (num_pair_t*) a;
        num_pair_t *np_b = (num_pair_t*) b;
        return (np_a->n1 == np_b->n1);
    }
    return 0;
}

void* num_pair_cpy(void *old){
    num_pair_t *n = (num_pair_t*) old;
    return old ? new_num_pair(n->n1, n->n2) : NULL; 
}

void print_num_pair(void *np){
    if(np){
        num_pair_t *n = (num_pair_t*) np;
        printf("<%d, %d>", n->n1, n->n2);
    }
}

var_num_pair_t* new_var_num_pair(Arg_Var* var, int num){
    var_num_pair_t *vnp = malloc_or_die(sizeof(var_num_pair_t*));
    vnp->var = var;
    vnp->num = num;
    return vnp;
}

int var_num_pair_cmp(void *a, void *b){
    if(a && b){
        var_num_pair_t *vnp_a = (var_num_pair_t*) a;
        var_num_pair_t *vnp_b = (var_num_pair_t*) b;
        return strcmp(vnp_a->var->name, vnp_b->var->name) == 0;
    }
    return 0;
}

void* var_num_pair_cpy(void *old){
    var_num_pair_t *v = (var_num_pair_t*) old;
    return old ? new_var_num_pair(v->var, v->num) : NULL; 
}

void print_var_num_pair(void *vnp){
    if(vnp){
        var_num_pair_t *v = (var_num_pair_t*) vnp;
        printf("<%s, %d>", v->var->name, v->num);
    }
}
