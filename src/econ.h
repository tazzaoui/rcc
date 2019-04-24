#ifndef ECON_H
#define ECON_H

#include "r.h"
#include "c.h"
#include "rcc.h"

/* 
 * ECON: Explicate Control Pass 
 */

/* Kontinuation Function Ptr */
typedef C_Tail* (*kont_func_t) (void *);

C_Tail *kont(void *);

/* Econ a general R_Expr */
C_Tail* econ_e(R_Expr*, kont_func_t, list_t);

/*  Econ a simple expression (Var, Num, Bool) */
C_Tail* econ_e_simple(R_Expr*, kont_func_t, list_t);

/*  Econ an R_Let expression */
C_Tail* econ_e_let(R_Expr*, kont_func_t, list_t);

/*  Econ a tail R_If expression */
C_Tail* econ_e_tail_if(R_Expr*, kont_func_t, list_t);

/*  Econ a non-tail R_If expression */
C_Tail* econ_e_non_tail_if(R_Expr*, kont_func_t, list_t);

#endif                          /* ECON_H */
