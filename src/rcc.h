#ifndef RCC_H
#define RCC_H

#include "c.h"
#include "list.h"
#include "r.h"
#include "utils.h"
#include "x.h" 
#include "pairs.h"

#define NUM_CALLER_SAVED_REGS 9
#define NO_COLOR -9999

extern const REGISTER Caller_Saved_Regs[NUM_CALLER_SAVED_REGS];

/* Uniquify pass */
R_Expr* uniquify(R_Expr*, list_t, int*);

/* RCO Pass */
R_Expr* rco(R_Expr*, list_t*);

/* Turn an R_Expr to a C_Tail */
C_Tail* econ_expr(R_Expr*);

/* Turn an R_Arg to a C_Arg */
C_Arg* econ_arg(R_Expr*);

/* Turn an R_Expr to a C_Expr */
C_Expr* econ_cmplx(R_Expr*);

/* Explicate Control Pass */
C_Expr* explicate_control(R_Expr*, list_t*);

/* Uncover Locals Pass */
C_Program* uncover_locals(C_Program*);

/* Select Instructions Pass */
X_Program* select_instr(C_Program*);

/* Assign Homes Pass (naive register allocation) */
X_Program* assign_homes(X_Program*);

/* Patch Instructions Pass (remove double memory refs) */
X_Program* patch_instrs(X_Program*);

/* Main Pass */
X_Program* main_pass(X_Program*);

/* Compile an X_Program with system assembler */
int x_compile(X_Program*);

/* uncover-live pass (liveness analysis) */
X_Program* uncover_live(X_Program*);

/* build-interferences pass */
X_Program* build_interferences(X_Program*);

/* color-graph pass */
X_Program* color_graph(X_Program*);

/* Patch a single instruction */
void patch_instr(X_Instr*, list_t);

/* Return a list of vars live after the current instr */
list_t live_after(Node*);

/* Return a list of vars live before the current instr */
list_t live_before(Node*);

/* Returns a list of the X_Args read by the instr */
void instrs_read(X_Instr*, list_t);

/* Returns a list of the X_Args written by the instr */
void instrs_written(X_Instr*, list_t);

/* Returns a list of the X_Args read by the argument */
void args_read(X_Arg*, list_t);

/* Returns a list of the X_Args written by the argument */
void args_written(X_Arg*, list_t);

/* Single X_Instr Register Alloc */
X_Instr* assign_instr(X_Instr*, list_t);

/* Single X_Arg Register Alloc */
X_Arg* assign_arg(X_Arg*, list_t);

/* C_Tail -> list of X instructions */
list_t select_instr_tail(C_Tail*);

/* C_Smt -> list of X instructions */
list_t select_instr_smt(C_Smt*);

/* C_Expr + dst -> list of X instructions */
list_t select_instr_expr(C_Expr*, X_Arg*);

/* C_Arg -> X_Arg */
X_Arg* select_instr_arg(C_Arg*);

/* Combine simple exprs into nested lets */
R_Expr* combine_lets(Node*, R_Expr*);

/* Resolve complex expressions */
R_Expr* resolve_complex_expr(R_Expr*, list_t, list_t*, int*);

/* Rename an R_Var */
R_Expr* rename_var(R_Expr*);

/* Add the live after pairs to graph */
void add_live_after(const list_t, list_t, X_Arg*, X_Arg*);

/* Add caller saved regs to interference graph */
void add_caller_saved_regs(list_t, list_t);

/* Add move bias node to move graph */
void move_bias(list_t, X_Instr*);

/* Computes Saturation of an X_Arg and returns the max */
int saturation(X_Arg*, list_t, list_t, list_t);

/* Returns the vertex with the maximium saturation */
x_arg_list_pair_t* max_sat(list_t, list_t);

/* Extract the set of variables used in a C_Tail */
void c_tail_extract_vars(C_Tail*, list_t);

/* Helper function to append an int to a c_str */
char* append_int(const char*, int);

/* Make the adjacency list symmetric */
void make_symmetric(list_t);

/* Wrapper around uniquify */
static inline R_Expr* unique(R_Expr* e) {
  int n = 0;
  return uniquify(e, list_create(), &n);
}

#endif /* RCC_H */
