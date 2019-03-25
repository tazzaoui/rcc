#ifndef C_H
#define C_H

#include "list.h"
#include "utils.h"

typedef enum C_ARG_TYPE { C_NUM, C_VAR } C_ARG_TYPE;
typedef enum C_TAIL_TYPE { C_TAIL_RET, C_TAIL_SEQ } C_TAIL_TYPE;
typedef enum C_EXPR_TYPE { C_ARG, C_READ, C_NEG, C_ADD } C_EXPR_TYPE;

typedef struct C_Program {
  Info *info;
  list_t labels;                // labels: label -> tail
} C_Program;

typedef struct C_Tail {
  C_TAIL_TYPE type;
  void *tail;
} C_Tail;

typedef struct C_Expr {
  C_EXPR_TYPE type;
  void *expr;
} C_Expr;

typedef struct C_Arg {
  C_ARG_TYPE type;
  void *arg;
} C_Arg;

typedef struct C_Num {
  int num;
} C_Num;

typedef struct C_Var {
  const char *name;
} C_Var;

typedef struct C_Ret {
  C_Arg *arg;
} C_Ret;

typedef struct C_Read {
  C_Arg *arg;
} C_Read;

typedef struct C_Neg {
  C_Arg *arg;
} C_Neg;

typedef struct C_Add {
  C_Arg *left, *right;
} C_Add;

typedef struct C_Smt {
  C_Var *var;
  C_Expr *expr;
} C_Smt;

typedef struct C_Seq {
  C_Smt *smt;
  C_Tail *tail;
} C_Seq;

/* Return a new c program */
C_Program *new_c_program(void *, list_t);

/* Return a new tail */
C_Tail *new_c_tail(C_TAIL_TYPE, void *);

/* Return a new statement */
C_Smt *new_c_smt(C_Var *, C_Expr *);

/* Return a new c expression */
C_Expr *new_c_expr(C_EXPR_TYPE, void *);

/* Return a new c arg */
C_Arg *new_c_arg(C_ARG_TYPE type, void *);

/* Return a new c number */
C_Num *new_c_num(int);

/* Return a new c variable */
C_Var *new_c_var(const char *);

/* Return a new c ret statement */
C_Ret *new_c_ret(C_Arg *);

/* Return a new c sequence */
C_Seq *new_c_seq(C_Smt *, C_Tail *);

/* Return a new call to read */
C_Read *new_c_read(C_Arg *);

/* Return a new neg statement */
C_Neg *new_c_neg(C_Arg *);

/* Return a new c add statement */
C_Add *new_c_add(C_Arg *, C_Arg *);

/* Print out a C_Smt */
void c_print_smt(C_Smt *);

/* Print out a C_Tail */
void c_print_tail(C_Tail *);

/* Print out a C_Arg */
void c_print_arg(C_Arg *);

/* Print out a C_Smt */
void c_print_expr(C_Expr *);

/* Print out a C_Program */
void c_print(C_Program *);

/* Interp a C program */
int c_p_interp(C_Program *);

/* Interp a C tail */
int c_t_interp(C_Tail *, list_t);

/* Interp a C statement */
int c_s_interp(C_Smt *, list_t);

/* Interp a C expression */
int c_e_interp(C_Expr *, list_t);

/* Interp a C argument */
int c_a_interp(C_Arg *, list_t);
#endif                          /* C_H */
