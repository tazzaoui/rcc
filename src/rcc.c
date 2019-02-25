#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> 
#include "utils.h"
#include "list.h"
#include "pairs.h"
#include "r.h"
#include "x.h"
#include "rcc.h"

#define TMP_REG RAX

char *append_int(const char* old, int n){
    int num_digits = n == 0 ? 1 : floor(log10(abs(n))) + 1; 
    char *new = malloc_or_die(strlen(old) + num_digits + 1); 
    sprintf(new,"%s_%d", old, n);
    return new;
}

void c_tail_extract_vars(C_Tail* tail, list_t vars){
    if(tail && tail->type == C_TAIL_SEQ){
        C_Seq *cs = tail->tail;
        C_Var *cv = cs->smt->var;
        Node* node = list_find(vars, cv, c_var_cmp);
        if(node == NULL) list_insert(vars, cv);
        c_tail_extract_vars(cs->tail, vars);
    }
}

int x_compile(X_Program *xp){
    FILE *fp;
    int cc_result;
    x_emit(xp, "test_compile.s");
    system("cc src/runtime.c test_compile.s -o test.bin");
    if ((fp = popen("./test.bin", "r")) == NULL)
        die("[compile_and_check] Failed to run test_bin!");
    fscanf(fp, "%d", &cc_result);
    pclose(fp);
    return cc_result;
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
                else list_update(env_p, new_r_var_var_pair(re->expr, NULL), 
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

R_Expr* combine_lets(Node* head, R_Expr* var){
    if(head == NULL) return var;
    env_pair_t *ep = head->data;
    return new_let(ep->var, ep->val, combine_lets(head->next, var)); 
}

R_Expr* rco(R_Expr* expr, list_t* new_vars){
    int var_seed = 0;
    list_t env = list_create();
    R_Expr* res = resolve_complex_expr(expr, env, new_vars, &var_seed);
    return combine_lets(**new_vars, res);
}

R_Expr* resolve_complex_expr(R_Expr* expr, list_t env, list_t *new_vars, int *rco_calls){
    char *c;
    Node *n;
    list_t l1 = list_create(), l2, l3;
    R_Expr *r1, *r2, *r3, *r4;
    if(expr)
        switch(expr->type){
            case R_EXPR_VAR:
                *new_vars = l1;
                n = list_find(env, new_env_pair(expr, NULL), ep_cmp);
                if(n == NULL) die("[RESOLVE_COMPLEX] UNBOUND VAR!");
                return ((env_pair_t*)n->data)->val;
            case R_EXPR_NUM:
                *new_vars = l1;
                return expr;
            case R_EXPR_ADD:
                l2 = list_create();
                r1 = resolve_complex_expr(((R_Add*)expr->expr)->left, env, &l1, rco_calls);
                r2 = resolve_complex_expr(((R_Add*)expr->expr)->right, env, &l2, rco_calls);
                r3 = new_add(r1, r2);
                l3 = list_concat(l1, l2);
                c = append_int("X", (*rco_calls)++);
                r4 = new_var(c);
                list_insert(l3, new_env_pair(r4, r3)); 
                *new_vars = l3;
                return r4;
            case R_EXPR_NEG:
                r1 = resolve_complex_expr(((R_Neg*)expr->expr)->expr, env, &l1, rco_calls);
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
                r1 = resolve_complex_expr(((R_Let*)expr->expr)->expr, env, &l1, rco_calls);
                l2 = list_copy(env, ep_cpy);
                list_insert(l2, new_env_pair(((R_Let*)expr->expr)->var, r1));
                r2 = resolve_complex_expr(((R_Let*)expr->expr)->body, l2, &l3, rco_calls); 
                *new_vars = list_concat(l1, l3); 
                return r2;
        };
    die("[RESOLVE_COMPLEX] NO RESOLUTION!");
    return NULL;
}

C_Tail* econ_expr(R_Expr* r_expr){
    C_Smt *st;
    C_Seq *cs;
    R_Var *rv;
    if(r_expr)
        switch(r_expr->type){
            case R_EXPR_NUM:
            case R_EXPR_VAR:
                return new_c_tail(C_TAIL_RET, new_c_ret(econ_arg(r_expr)));
            case R_EXPR_LET:
                rv = ((R_Let*)r_expr->expr)->var->expr; 
                st = new_c_smt(new_c_var(rv->name), econ_cmplx(((R_Let*)r_expr->expr)->expr));
                cs = new_c_seq(st, econ_expr(((R_Let*)r_expr->expr)->body));
                return new_c_tail(C_TAIL_SEQ, cs);
            default:
                break;
        };
    die("[econ_expr] INVALID EXPR"); 
    return NULL;
}

C_Expr* econ_cmplx(R_Expr* r_expr){
    R_Add *ra;
    if(r_expr)
        switch(r_expr->type){
            case R_EXPR_READ:
                return new_c_expr(C_READ, new_c_read(NULL));
            case R_EXPR_NEG:
                return new_c_expr(C_NEG, new_c_neg(econ_arg(((R_Neg*)r_expr->expr)->expr)));
            case R_EXPR_ADD:
                ra = r_expr->expr;
                return new_c_expr(C_ADD, new_c_add(econ_arg(ra->left), econ_arg(ra->right)));
            default:
                break;
        };
    die("[econ_cmplx] INVALID EXPR"); 
    return NULL;
}

C_Arg* econ_arg(R_Expr *r_expr){
    if(r_expr)
        switch(r_expr->type){
            case R_EXPR_NUM:
                return new_c_arg(C_NUM, new_c_num(((R_Num*)r_expr->expr)->num));
            case R_EXPR_VAR: 
                return new_c_arg(C_VAR, new_c_var(((R_Var*)r_expr->expr)->name));
            default:
                break;
        };
    die("[econ_arg] INVALID EXPR"); 
    return NULL;
}

C_Program* uncover_locals(C_Program* cp){
    if(cp){
        Node *node = list_find(cp->labels, new_lbl_tail_pair("body", NULL), lbl_tail_cmp);
        if(node == NULL) die("[UNCOVER_LOCALS] NO BODY LABEL!");
        C_Tail *tail = ((lbl_tail_pair_t*)node->data)->tail;
        list_t vars = list_create();
        c_tail_extract_vars(tail, vars);
        return new_c_program(new_info(vars), cp->labels); 
    }
    return cp;
}

X_Program* select_instr(C_Program* cp){
    Node *node = list_find(cp->labels, new_lbl_tail_pair("body", NULL), lbl_tail_cmp);
    if(node == NULL) die("[SELECT_INSTR] NO BODY LABEL!");
    C_Tail *ct = ((lbl_tail_pair_t*)node->data)->tail; 
    list_t x_instrs = select_instr_tail(ct);
    list_t lbls = list_create();
    lbl_blk_pair_t *lbl = new_lbl_blk_pair("body", new_x_block(NULL, x_instrs));  
    list_insert(lbls, lbl);
    list_t ret_instr = list_create();
    list_insert(ret_instr, new_x_instr(RETQ, new_x_retq()));
    lbl_blk_pair_t *end_lbl = new_lbl_blk_pair("end", new_x_block(NULL, ret_instr)); 
    list_insert(lbls, end_lbl);
    return new_x_prog(cp->info, lbls);
}

X_Program* assign_homes(X_Program* xp){
    if(xp && xp->info){
        int offset = 1, variable_count;
        Info *info = xp->info;
        list_t bi = list_create(), ei = list_create(), new_instrs = list_create(),
               var_to_num = list_create(), lbls = list_create();
        
        lbl_blk_pair_t *begin_lbp = new_lbl_blk_pair("begin", new_x_block(NULL, bi));
        lbl_blk_pair_t *end_lbp = new_lbl_blk_pair("end", new_x_block(NULL, ei));
        lbl_blk_pair_t *body_lbp = new_lbl_blk_pair("body", new_x_block(NULL, new_instrs));

        list_insert(lbls, begin_lbp);
        list_insert(lbls, body_lbp);
        list_insert(lbls, end_lbp);

        variable_count = list_size(info->vars) * 8;
        variable_count = (variable_count % 16 == 0) ? variable_count : variable_count + 8;

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
        
        Node *node = list_find(xp->labels, new_lbl_blk_pair("body", NULL), lbl_blk_pair_cmp);
        if(node == NULL) die("[SELECT_INSTR] NO BODY LABEL!");
        X_Block *xb = ((lbl_blk_pair_t*)node->data)->block;

        node = *(info->vars); 
        while(node != NULL){
            list_insert(var_to_num, new_var_num_pair(node->data, offset++));
            node = node->next;
        }

        node = *(xb->instrs);
        while(node != NULL){
            list_insert(new_instrs, assign_instr(node->data, var_to_num));
            node = node->next;
        }
        return new_x_prog(NULL, lbls);
    }
    die("[assign_homes] XP OR INFO IS NULL!");
    return NULL;
}

X_Program* patch_instrs(X_Program *xp){
    Node *head, *node = list_find(xp->labels, new_lbl_blk_pair("body", NULL), lbl_blk_pair_cmp);
    if(node == NULL) die("PATCH_INSTRS] NO BODY LABEL!"); 
    list_t new_instrs = list_create(), lbls = list_create();
    head = *(((lbl_blk_pair_t*)node->data)->block->instrs);

    while(head != NULL){
        patch_instr(head->data, new_instrs);
        head = head->next; 
    }
    
    node = list_find(xp->labels, new_lbl_blk_pair("begin", NULL), lbl_blk_pair_cmp);
    if(node == NULL) die("PATCH_INSTRS] NO BEGIN LABEL!");
    
    list_insert(lbls, node->data);
    list_insert(lbls, new_lbl_blk_pair("body", new_x_block(NULL, new_instrs)));
    
    node = list_find(xp->labels, new_lbl_blk_pair("end", NULL), lbl_blk_pair_cmp);
    if(node == NULL) die("PATCH_INSTRS] NO END LABEL!");
    
    list_insert(lbls, node->data);
    
    return new_x_prog(NULL, lbls);
}

X_Program* main_pass(X_Program* xp){
    X_Arg *rax = new_x_arg(X_ARG_REG, new_x_arg_reg(RAX));
    X_Arg *rdi = new_x_arg(X_ARG_REG, new_x_arg_reg(RDI));
    
    list_t main_instrs = list_create();
    list_insert(main_instrs, new_x_instr(CALLQ, new_x_callq("begin")));
    list_insert(main_instrs, new_x_instr(MOVQ, new_x_movq(rax, rdi)));
    list_insert(main_instrs, new_x_instr(CALLQ, new_x_callq("print_int")));
    list_insert(main_instrs, new_x_instr(RETQ, new_x_retq()));
  
    list_insert(xp->labels, new_lbl_blk_pair("main", new_x_block(NULL, main_instrs)));
    return xp;
}

void patch_instr(X_Instr *xp, list_t instrs){
    X_Arg *left, *right, *tmp = new_x_arg(X_ARG_REG, new_x_arg_reg(TMP_REG));
    if(xp)
        switch(xp->type){
            case ADDQ:
                left = ((X_Addq*)xp->instr)->left;
                right = ((X_Addq*)xp->instr)->right;
                if(left->type == X_ARG_MEM && right->type == X_ARG_MEM){
                    list_insert(instrs, new_x_instr(MOVQ, new_x_movq(left, tmp))); 
                    list_insert(instrs, new_x_instr(ADDQ, new_x_addq(tmp, right))); 
                } else list_insert(instrs, xp);
                break;
            case MOVQ:
                left = ((X_Movq*)xp->instr)->left;
                right = ((X_Movq*)xp->instr)->right;
                if(left->type == X_ARG_MEM && right->type == X_ARG_MEM){
                    list_insert(instrs, new_x_instr(MOVQ, new_x_movq(left, tmp))); 
                    list_insert(instrs, new_x_instr(MOVQ, new_x_movq(tmp, right))); 
                } else list_insert(instrs, xp);
                break;
            default:
                list_insert(instrs, xp);
        };
}

X_Instr* assign_instr(X_Instr *xi, list_t map){
    if(xi)
        switch(xi->type){
            case ADDQ:
                return new_x_instr(ADDQ, new_x_addq(assign_arg(((X_Addq*)xi->instr)->left, map),
                                                    assign_arg(((X_Addq*)xi->instr)->right, map)));
            case SUBQ:
                return new_x_instr(SUBQ, new_x_subq(assign_arg(((X_Subq*)xi->instr)->left, map),
                                                    assign_arg(((X_Subq*)xi->instr)->right, map)));
            case MOVQ:
                return new_x_instr(MOVQ, new_x_movq(assign_arg(((X_Movq*)xi->instr)->left, map),
                                                    assign_arg(((X_Movq*)xi->instr)->right, map)));
            case NEGQ:
                return new_x_instr(NEGQ, new_x_negq(assign_arg(((X_Negq*)xi->instr)->arg, map)));
            default: 
                return xi;
        };
    return NULL;
}

X_Arg* assign_arg(X_Arg* xa, list_t map){
    Node *node;
    if(xa)
        switch(xa->type){
            case X_ARG_NUM:
                return xa;
            case X_ARG_VAR:
                node = list_find(map, new_var_num_pair(xa->arg, 0), var_num_pair_cmp);
                if(node == NULL) die("[ASSIGN_ARG] VARIABLE MAPPING NOT FOUND!");
                return new_x_arg(X_ARG_MEM, new_x_arg_mem(RSP, 8 * ((var_num_pair_t*)node->data)->num));
            default:
                return xa;
        };
    return NULL;
}

list_t select_instr_tail(C_Tail* ct){
    list_t instrs_smt, instrs_tail; 
    X_Arg *rax;
    if(ct)
        switch(ct->type){
            case C_TAIL_RET: 
               instrs_smt = list_create();
               rax = new_x_arg(X_ARG_REG, new_x_arg_reg(RAX));
               list_insert(instrs_smt, 
                       new_x_instr(MOVQ, new_x_movq(select_instr_arg(((C_Ret*)ct->tail)->arg), rax)));
               list_insert(instrs_smt, new_x_instr(JMP, new_x_jmp("end")));
               return instrs_smt;
            case C_TAIL_SEQ:
               instrs_smt = select_instr_smt(((C_Seq*)ct->tail)->smt);
               instrs_tail = select_instr_tail(((C_Seq*)ct->tail)->tail);
               return list_concat(instrs_smt, instrs_tail); 
        }
    return list_create();
}

list_t select_instr_smt(C_Smt* cs){
    list_t res;
    if(cs){
        C_Arg *ca = new_c_arg(C_VAR, cs->var); 
        res = select_instr_expr(cs->expr, select_instr_arg(ca)); 
    }
    return res;
}

list_t select_instr_expr(C_Expr* ce, X_Arg* dst){
    X_Arg *rax;
    list_t instrs = list_create();
    if(ce)
        switch(ce->type){
            case C_ARG:
               list_insert(instrs, new_x_instr(MOVQ, new_x_movq(select_instr_arg(ce->expr), dst))); 
               break;
            case C_READ:
               rax = new_x_arg(X_ARG_REG, new_x_arg_reg(RAX));
               list_insert(instrs, new_x_instr(CALLQ, new_x_callq(READ_INT))); 
               list_insert(instrs, new_x_instr(MOVQ, new_x_movq(rax, dst)));
               break;
            case C_NEG:
               list_insert(instrs, new_x_instr(MOVQ, new_x_movq(select_instr_arg(((C_Neg*)ce->expr)->arg), dst))); 
               list_insert(instrs, new_x_instr(NEGQ, new_x_negq(dst)));
               break;
            case C_ADD:
               list_insert(instrs, new_x_instr(MOVQ, new_x_movq(select_instr_arg(((C_Add*)ce->expr)->right), dst))); 
               list_insert(instrs, new_x_instr(ADDQ, new_x_addq(select_instr_arg(((C_Add*)ce->expr)->left), dst))); 
               break;
        };
    return instrs;
}

X_Arg* select_instr_arg(C_Arg* ca){
    if(ca)
        switch(ca->type){
            case C_NUM:
                return new_x_arg(X_ARG_NUM, new_x_arg_num(((C_Num*)ca->arg)->num)); 
            case C_VAR:
                return new_x_arg(X_ARG_VAR, new_x_arg_var(((C_Var*)ca->arg)->name));
        };
    die("[SELECT_INSTR_ARG] INVALID CA");
    return NULL;
}
