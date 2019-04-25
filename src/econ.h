#ifndef ECON_H
#define ECON_H

#include "r.h"
#include "c.h"
#include "rcc.h"

/* 
 * ECON: Explicate Control Pass 
 */

/* 
 * Kontinuation Function Ptr 
 * 
 * C_Tail* X lbl_tail_pair_t -> C_tail
 */

typedef C_Tail *(*kont_func_t)(void *, void *);

C_Tail *kont(void *, void *);
C_Tail *nkont(void *, void *);

/* Econ a general R_Expr */
C_Tail *econ_e(R_Expr *, kont_func_t, list_t, C_Tail *);

/*  Econ a simple expression (Var, Num, Bool) */
C_Tail *econ_e_simple(R_Expr *, kont_func_t, list_t, C_Tail *);

/*  Econ an R_Let expression */
C_Tail *econ_e_let(R_Expr *, kont_func_t, list_t, C_Tail *);

/*  Econ a tail R_If expression */
C_Tail *econ_e_if(R_Expr *, kont_func_t, list_t, C_Tail *);

/*  Econ a non-tail R_Let expression */
C_Tail *econ_e_non_tail_let(R_Expr *, kont_func_t, list_t, C_Tail *);

/* Return a new kontinuation arg */
C_Tail *new_kont_tail(C_Var *, label_t);

#endif                          /* ECON_H */
