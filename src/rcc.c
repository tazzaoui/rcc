#include <stdio.h>
#include <string.h>
#include <math.h>

#include "utils.h"
#include "list.h"
#include "pairs.h"
#include "r.h"
#include "x.h"
#include "rcc.h"


char *append_int(const char* old, int n){
    int num_digits = n == 0 ? 1 : floor(log10(abs(n))) + 1; 
    char *new = malloc_or_die(strlen(old) + num_digits + 1); 
    sprintf(new,"%s_%d", old, n);
    return new;
}

R_Expr* uniquify(R_Expr* expr, list_t env, int *cnt){
    Node *node;
    list_t env_p;
    R_Expr *re, *nv, *ne, *nb;
    char *c;
    if(expr && env)
        switch(expr->type) {
            case R_EXPR_VAR:
                node = list_find(env, new_r_var_var_pair(expr->expr, NULL), r_var_var_pair_cmp);
                if (node != NULL)
                    return new_expr(((r_var_var_pair_t*)node->data)->v2, R_EXPR_VAR);
                break;
            case R_EXPR_LET:
                re = ((R_Let*)expr->expr)->var;
                c = append_int(((R_Var*)re->expr)->name, (*cnt)++);
                nv = new_var(c);
                env_p = list_copy(env, r_var_var_cpy);
                node = list_find(env, new_r_var_var_pair(re->expr, NULL), r_var_var_pair_cmp);
                if (node == NULL)
                    list_insert(env_p, new_r_var_var_pair(re->expr, nv->expr));
                else
                    list_update(env_p, new_r_var_var_pair(re->expr, NULL), 
                            new_r_var_var_pair(re->expr, nv->expr), r_var_var_pair_cmp);
                ne = uniquify(((R_Let*)expr->expr)->expr, env, cnt);
                nb = uniquify(((R_Let*)expr->expr)->body, env_p, cnt);
                return new_let(nv, ne, nb);
            case R_EXPR_NEG:
                re = uniquify(((R_Neg*)expr->expr)->expr, env, cnt);
                return new_neg(re);
            case R_EXPR_ADD:
                re = uniquify(((R_Add*)expr->expr)->left, env, cnt);
                nv = uniquify(((R_Add*)expr->expr)->right, env, cnt);
                return new_add(re, nv);
            default:
                break;
        };
    return expr;
}
