#ifndef C_H
#define C_H 

typedef enum C_ARG_TYPE { C_NUM, C_VAR } C_ARG_TYPE;
typedef enum C_TAIL_TYPE { C_TAIL_RET, C_TAIL_SEQ } C_TAIL_TYPE;
typedef enum C_EXPR_TYPE { C_ARG, C_READ, C_NEG, C_ADD } C_EXPR_TYPE;

typedef struct C_Program {
  list_t labels;  // labels: label -> tail
  void* info;
} C_Program;

typedef struct C_Tail {
    C_TAIL_TYPE type;
    void *tail;
} C_Tail;

typedef struct C_Stmt{
    C_Var *var;
    C_Expr *expr;
} C_Stmt;

typedef struct C_Expr {
    C_EXPR_TYPE type;
    void expr;
};

typedef struct C_Arg {
    C_ARG_TYPE type;
    void *arg;
} C_Arg;

#endif /* C_H */
