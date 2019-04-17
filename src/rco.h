#ifndef RCO_H
#define RCO_H

#include "rcc.h"
#include "r.h"

/*  RCO: Resolve Complex Operations
 *  
 *  The purpose of the RCO pass is to remove the 
 *  recursive structure of R in order to transition to C
 */

/*  Check if binary operation */
int is_bin_op(R_Expr *);

/* RCO a general expression */
R_Expr *rco_e(R_Expr *, list_t, int);

/* RCO a general argument */
R_Expr *rco_a(R_Expr *, list_t, int, list_t *, int *);

/* RCO a unary operation argument */
R_Expr *rco_a_unary(R_Expr *, list_t, int, list_t *, int *);

/* RCO a binary operation argument */
R_Expr *rco_a_binary(R_Expr *, list_t, int, list_t *, int *);

/* RCO a let expr argument */
R_Expr *rco_a_let(R_Expr *, list_t, int, list_t *, int *);

/* RCO an if expr argument */
R_Expr *rco_a_if(R_Expr *, list_t, int, list_t *, int *);

/* RCO a comparison expr */
R_Expr *rco_c(R_Expr *, list_t, list_t *, int *);

/* RCO a binary comparison expr */
R_Expr *rco_c_bin(R_Expr *, list_t, list_t *, int *);

/* RCO a let comparison expr */
R_Expr *rco_c_let(R_Expr *, list_t, list_t *, int *);

/* RCO a non-binary, non-let comparison expr */
R_Expr *rco_c_other(R_Expr *, list_t, list_t *, int *);

#endif                          /* RCO_H */
