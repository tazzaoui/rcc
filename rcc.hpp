#ifndef RCC_HPP
#define RCC_HPP

#include <iostream>
#include <string>

typedef enum EXPR_TYPE { NEG, ADD, READ, NUM } EXPR_TYPE;

class Expr {
 public:
  EXPR_TYPE type;
  virtual void print(std::ostream&) = 0;
  virtual int interp() = 0;
  virtual Expr* optimize() = 0;
  friend std::ostream& operator<<(std::ostream&, Expr&);
};

class Program {
  void* info;
  Expr* expr;

 public:
  Program(void* i, Expr* e) : info(i), expr(e){};
  Program* optimize(void);
  void print(std::ostream&);
  int interp(void);
  Expr* get_expr() { return this->expr; };
  EXPR_TYPE get_type() { return this->expr->type; };
  friend std::ostream& operator<<(std::ostream&, Program&);
};

class Var : public Expr {
  std::string var;
  Expr* val;

 public:
  Var(std::string&, Expr* val);
  void print(std::ostream&);
  int interp(void);
  Expr* optimize();
}

class Let : public Expr {
  Var* var;
  Expr *expr, *body;

 public:
  Let(Var*, Expr*, Expr*);
  void print(std::ostream&);
  int interp(void);
  Expr* optimize();
}

class Neg : public Expr {
  Expr* expr;

 public:
  Neg(Expr* e) : expr(e) { type = NEG; };
  void print(std::ostream&);
  int interp(void);
  Expr* optimize();
};

class Add : public Expr {
  Expr *left, *right;

 public:
  Add(Expr* l, Expr* r) : left(l), right(r) { type = ADD; };
  void print(std::ostream&);
  int interp(void);
  Expr* optimize();
  Expr* get_left() { return this->left; };
  Expr* get_right() { return this->right; };
};

class Num : public Expr {
  int num;

 public:
  Num(int n) : num(n) { type = NUM; };
  int get_num() { return this->num; };
  void set_num(int n) { this->num = n; };
  void print(std::ostream&);
  int interp(void);
  Expr* optimize();
};

class Read : public Expr {
  int num;
  bool read;

 public:
  Read(void) : num(0), read(false) { type = READ; };
  Read(int n) : num(n), read(true) { type = READ; };
  void print(std::ostream&);
  int interp(void);
  Expr* optimize();
};
#endif /* RCC_HPP */
