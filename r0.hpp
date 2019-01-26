#ifndef R0_HPP
#define R0_HPP

typedef enum EXPR_TYPE { NEG, ADD, READ, NUM } EXPR_TYPE;

class Expr {
 public:
  virtual void print() = 0;
  virtual Expr* interp() = 0;
};

class Program {
  void* info;
  Expr* expr;

 public:
  Program(void* i, Expr* e) : info(i), expr(e) {};
  void print(void);
  Expr* interp(void);
};

class Neg : public Expr {
  Expr* expr;

 public:
  Neg(Expr* e) : expr(e) {};
  void print(void);
  Expr* interp(void);
};

class Add : public Expr {
  Expr *left, *right;

 public:
  Add(Expr* l, Expr* r) : left(l), right(r) {};
  void print(void);
  Expr* interp(void);
};

class Num : public Expr {
  int num;

 public:
  Num(int n) : num(n){};
  int get_num() { return this->num; };
  void set_num(int n) { this->num = n; };
  void print(void);
  Expr* interp(void);
};
#endif /* R0_HPP */
