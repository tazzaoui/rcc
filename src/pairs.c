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
