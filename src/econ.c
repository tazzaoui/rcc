#include <assert.h>
#include "econ.h"
#include "rcc.h"
#include "r.h"
#include "c.h"

C_Tail *kont(void *a){
    return a;
}

C_Tail* econ_e_simple(R_Expr* expr, kont_func_t k, list_t l2t){
    l2t = list_create();
    C_Tail* ct = econ_expr(expr);
    return k(ct);
}

C_Tail* econ_e_let(R_Expr* expr, kont_func_t k, list_t l2t){
   assert(expr && expr->type == R_EXPR_LET);
   l2t = list_create();
   
   C_Expr *ct_expr = econ_cmplx(((R_Let*)expr->expr)->expr); 
   C_Tail *ct_body = econ_e(((R_Let*)expr->expr)->body, k, l2t);
    
   const char* var = ((R_Var*)((R_Let*)expr->expr)->var->expr)->name; 
   C_Smt *x_smt = new_c_smt(new_c_var(var), ct_expr);
   return new_c_tail(C_TAIL_SEQ, new_c_seq(x_smt, ct_body));
}

/*  Econ a tail R_If expression */
C_Tail* econ_e_tail_if(R_Expr*, kont_func_t, list_t);

/*  Econ a non-tail R_If expression */
C_Tail* econ_e_non_tail_if(R_Expr*, kont_func_t, list_t);
