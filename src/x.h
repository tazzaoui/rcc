#ifndef X_H
#define X_H

#include "list.h"

#define NUM_REGS 16
#define X_PRINT_ARG_ALLOW_VARS 1

extern const char *registers[NUM_REGS];

typedef enum X_ARG_TYPE {
  X_ARG_NUM,
  X_ARG_REG,
  X_ARG_MEM,
  X_ARG_VAR,
  X_ARG_BYTE_REG
} X_ARG_TYPE;

typedef enum X_CC_TYPE {
  X_CC_E,
  X_CC_L,
  X_CC_LE,
  X_CC_G,
  X_CC_GE
} X_CC_TYPE;

typedef enum X_INSTR_TYPE {
  ADDQ,
  SUBQ,
  MOVQ,
  RETQ,
  NEGQ,
  CALLQ,
  JMP,
  PUSHQ,
  POPQ,
  XORQ,
  CMPQ,
  SETCC,
  MOVZBQ,
  JMPIF
} X_INSTR_TYPE;

typedef enum REGISTER {
  RSP,
  RBP,
  RAX,
  RBX,
  RCX,
  RDX,
  RSI,
  RDI,
  R8,
  R9,
  R10,
  R11,
  R12,
  R13,
  R14,
  R15
} REGISTER;

typedef struct X_Program {
  Info *info;
  list_t labels;                // labels: label -> block
} X_Program;

typedef struct X_Block {
  Info *info;
  list_t instrs;
} X_Block;

typedef struct X_Instr {
  X_INSTR_TYPE type;
  void *instr;
} X_Instr;

typedef struct X_Arg {
  X_ARG_TYPE type;
  void *arg;
} X_Arg;

typedef struct X_State {
  int regs[NUM_REGS];           // (register -> num)
  list_t nums;                  // (num -> num)
  list_t vars;                  // (var -> num)
  list_t lbls;                  // (lbl - >blks)
} X_State;

typedef struct X_Addq {
  X_Arg *left, *right;
} X_Addq;

typedef struct X_Subq {
  X_Arg *left, *right;
} X_Subq;

typedef struct X_Movq {
  X_Arg *left, *right;
} X_Movq;

typedef struct X_Retq {
  void *ret;
} X_Retq;

typedef struct X_Negq {
  X_Arg *arg;
} X_Negq;

typedef struct X_Callq {
  label_t label;
} X_Callq;

typedef struct X_Jmp {
  label_t label;
} X_Jmp;

typedef struct X_Pushq {
  X_Arg *arg;
} X_Pushq;

typedef struct X_Popq {
  X_Arg *arg;
} X_Popq;

typedef struct X_Xorq {
  X_Arg *left, *right;
} X_Xorq;

typedef struct X_Cmpq {
  X_Arg *left, *right;
} X_Cmpq;

typedef struct X_Setcc {
  X_CC_TYPE cc;
  X_Arg *arg;
} X_Setcc;

typedef struct X_Movzbq {
  X_Arg *left, *right;
} X_Movzbq;

typedef struct X_Jmpif {
  X_CC_TYPE cc;
  X_Arg *arg;
} X_Jmpif;

typedef struct X_Arg_Num {
  int num;
} X_Arg_Num;

typedef struct X_Arg_Reg {
  REGISTER reg;
} X_Arg_Reg;

typedef struct X_Arg_Mem {
  REGISTER reg;
  int offset;
} X_Arg_Mem;

typedef struct X_Arg_Var {
  const char *name;
} X_Arg_Var;

typedef struct X_Arg_Byte_Reg {
  REGISTER reg;
} X_Arg_Byte_Reg;

/* Return a new program */
X_Program *new_x_prog(void *, list_t);

/* Return a new block */
X_Block *new_x_block(void *, list_t);

/* Return a new instruction */
X_Instr *new_x_instr(X_INSTR_TYPE, void *);

/* Return an empty machine state */
X_State *new_x_state(list_t);

/* Return a new arg */
X_Arg *new_x_arg(X_ARG_TYPE, void *);

/* Return a new addq instruction */
X_Addq *new_x_addq(X_Arg *, X_Arg *);

/* Return a new addq instruction */
X_Subq *new_x_subq(X_Arg *, X_Arg *);

/* Return a new movq instruction */
X_Movq *new_x_movq(X_Arg *, X_Arg *);

/* Return a new retq instruction */
X_Retq *new_x_retq(void);

/* Return a new negq instruction */
X_Negq *new_x_negq(X_Arg *);

/* Return a new callq instruction */
X_Callq *new_x_callq(label_t);

/* Return a new jmp instruction */
X_Jmp *new_x_jmp(label_t);

/* Return a new pushq instruction */
X_Pushq *new_x_pushq(X_Arg *);

/* Return a new popq instruction */
X_Popq *new_x_popq(X_Arg *);

/*  Return a new xorq instruction */
X_Xorq *new_x_xorq(X_Arg *, X_Arg *);

/*  Return a new cmpq instruction */
X_Cmpq *new_x_cmpq(X_Arg *, X_Arg *);

/*  Return a new setcc instruction */
X_Setcc *new_x_setcc(X_CC_TYPE, X_Arg *);

/*  Return a new movzbq instruction */
X_Movzbq *new_x_movzbq(X_Arg *, X_Arg *);

/*  Return a new jmpif instruction */
X_Jmpif *new_x_jmpif(X_CC_TYPE, X_Arg *);

/* Return a new num arg */
X_Arg_Num *new_x_arg_num(int);

/* Return a new register arg */
X_Arg_Reg *new_x_arg_reg(REGISTER);

/* Return a new memory ref arg */
X_Arg_Mem *new_x_arg_mem(REGISTER, int);

/* Return a new variable arg */
X_Arg_Var *new_x_arg_var(const char *);

/* Return a new byte register arg */
X_Arg_Byte_Reg *new_x_arg_byte_reg(REGISTER);

/* Emit an X Program */
void x_emit(X_Program *, const char *);

/* Print a block instruction */
void print_lbl_blk_pair(void *);

/* Print a single instruction */
void print_x_instr(void *);

/* Print a single argument */
void x_print_arg(X_Arg *);

/* Interp an X Program */
int x_interp(X_Program *);

/* Interp a block */
int x_blk_interp(label_t, X_State **);

/* Interpret a list of instructions */
int x_instrs_interp(list_t, X_State **);

/* Interpret a single instruction */
int x_instr_interp(X_Instr *, X_State **);

/* Update the Machine State */
int update_state(X_State **, X_Arg *, int);

/* Returns an element from the machine state*/
int lookup_state(X_State *, X_Arg *);
#endif                          /* X_H */
