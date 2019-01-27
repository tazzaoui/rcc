#ifndef R0_HPP
#define R0_HPP

typedef enum EXPR_TYPE { NEG, ADD, READ, NUM } EXPR_TYPE;

class Expr {
 public:
  virtual void print(std::ostream&) = 0;
  virtual Expr* interp() = 0;
  friend std::ostream & operator << (std::ostream&, Expr&); 
};

class Program {
  void* info;
  Expr* expr;

 public:
  Program(void* i, Expr* e) : info(i), expr(e) {};
  void print(std::ostream&);
  Expr* interp(void);
  friend std::ostream & operator << (std::ostream &, Program &); 
};

class Neg : public Expr {
  Expr* expr;

 public:
  Neg(Expr* e) : expr(e) {};
  void print(std::ostream&);
  Expr* interp(void);
};

class Add : public Expr {
  Expr *left, *right;

 public:
  Add(Expr* l, Expr* r) : left(l), right(r) {};
  void print(std::ostream&);
  Expr* interp(void);
};

class Num : public Expr {
  int num;

 public:
  Num(int n) : num(n){};
  int get_num() { return this->num; };
  void set_num(int n) { this->num = n; };
  void print(std::ostream&);
  Expr* interp(void);
};

class Read : public Expr {
  int num;

 public:
  Read(void);
  Read(int n) : num(n) {};
  void print(std::ostream&);
  Expr* interp(void);
};
#endif /* R0_HPP */
