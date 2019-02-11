#ifndef X_H
#define X_H

#include "list.h"

#define NUM_REGS 16
#define X_PRINT_ARG_ALLOW_VARS 1

extern const char* registers[NUM_REGS];
typedef enum ARG_TYPE { ARG_NUM, ARG_REG, ARG_MEM, ARG_VAR } ARG_TYPE;

typedef enum INSTR_TYPE {
  ADDQ,
  SUBQ,
  MOVQ,
  RETQ,
  NEGQ,
  CALLQ,
  JMP,
  PUSHQ,
  POPQ
} INSTR_TYPE;

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
  void* info;
  list_t labels;  // labels: label -> block
} X_Program;

typedef struct Block {
  void* info;
  list_t instrs;
} Block;

typedef struct Instr {
  INSTR_TYPE type;
  void* instr;
} Instr;

typedef struct Arg {
  ARG_TYPE type;
  void* arg;
} Arg;

typedef struct State {
  int regs[NUM_REGS];  // (register -> num)
  list_t nums;         // (num -> num)
  list_t vars;         // (var -> num)
  list_t lbls;         // (lbl - >blks)
} State;

typedef struct Addq {
  Arg *left, *right;
} Addq;

typedef struct Subq {
  Arg *left, *right;
} Subq;

typedef struct Movq {
  Arg *left, *right;
} Movq;

typedef struct Retq {
  void* ret;
} Retq;

typedef struct Negq {
  Arg* arg;
} Negq;

typedef struct Callq {
  label_t label;
} Callq;

typedef struct Jmp {
  label_t label;
} Jmp;

typedef struct Pushq {
  Arg* arg;
} Pushq;

typedef struct Popq {
  Arg* arg;
} Popq;

typedef struct Arg_Num {
  int num;
} Arg_Num;

typedef struct Arg_Reg {
  REGISTER reg;
} Arg_Reg;

typedef struct Arg_Mem {
  REGISTER reg;
  int offset;
} Arg_Mem;

typedef struct Arg_Var {
  const char* name;
} Arg_Var;

/* Return a new program */
X_Program* new_prog(void*, list_t);

/* Return a new block */
Block* new_block(void*, list_t);

/* Return a new instruction */
Instr* new_instr(INSTR_TYPE, void*);

/* Return an empty machine state */
State* new_state(list_t);

/* Return a new arg */
Arg* new_arg(ARG_TYPE, void*);

/* Return a new addq instruction */
Addq* new_addq(Arg*, Arg*);

/* Return a new addq instruction */
Subq* new_subq(Arg*, Arg*);

/* Return a new movq instruction */
Movq* new_movq(Arg*, Arg*);

/* Return a new retq instruction */
Retq* new_retq(void);

/* Return a new negq instruction */
Negq* new_negq(Arg*);

/* Return a new callq instruction */
Callq* new_callq(label_t);

/* Return a new jmp instruction */
Jmp* new_jmp(label_t);

/* Return a new pushq instruction */
Pushq* new_pushq(Arg*);

/* Return a new popq instruction */
Popq* new_popq(Arg*);

/* Return a new num arg */
Arg_Num* new_arg_num(int);

/* Return a new register arg */
Arg_Reg* new_arg_reg(REGISTER);

/* Return a new memory ref arg */
Arg_Mem* new_arg_mem(REGISTER, int);

/* Return a new variable arg */
Arg_Var* new_arg_var(const char*);

/* Emit an X Program */
void x_emit(X_Program*, const char*);

/* Print a block instruction */
void print_lbl_blk_pair(void*);

/* Print a single instruction */
void print_instr(void*);

/* Print a single argument */
void x_print_arg(Arg*);

/* Interp an X Program */
int x_interp(X_Program*);

/* Interp a block */
int x_blk_interp(label_t, State**);

/* Interpret a list of instructions */
int x_instrs_interp(list_t, State**);

/* Interpret a single instruction */
int x_instr_interp(Instr*, State**);

/* Update the Machine State */
int update_state(State**, Arg*, int);

/* Returns an element from the machine state*/
int lookup_state(State*, Arg*);
#endif /* X_H */
