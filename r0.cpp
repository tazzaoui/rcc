#include "r0.hpp"
#include <time.h>
#include <iostream>
#include <random>

#define QUIET_READ 1
#define DEBUG 0

std::ostream &operator<<(std::ostream &out, Expr &e) {
  e.print(out);
  return out;
}

std::ostream &operator<<(std::ostream &out, Program &p) {
  p.expr->print(out);
  return out;
}

int Program::interp() { return this->expr->interp(); }

Program *Program::optimize() {
  return new Program(this->info, this->expr->optimize());
}

void Neg::print(std::ostream &out) {
  out << "(N(";
  this->expr->print(out);
  out << "))";
}

int Neg::interp() { return -1 * this->expr->interp(); }

Expr *Neg::optimize() {
  Expr *res = this->expr->optimize();

  if (res->type == NUM)
    res = new Num(-1 * ((Num *)res)->get_num());

  else if (res->type == NEG)
    res = (((Neg *)res)->expr)->optimize();

  else if (res->type == ADD)
    res = new Add(new Neg(((Add *)res)->get_left()),
                  new Neg(((Add *)res)->get_right()));

  return res;
}

void Add::print(std::ostream &out) {
  out << "(+ ";
  this->left->print(out);
  out << " ";
  this->right->print(out);
  out << ")";
}

int Add::interp() { return this->left->interp() + this->right->interp(); }

Expr *Add::optimize() {
  Expr *l_opt = this->left->optimize();
  Expr *r_opt = this->right->optimize();
  Add *res = new Add(l_opt->optimize(), r_opt->optimize());

  /* Both Nums: just reduce to the addition itself */
  if (l_opt->type == NUM && r_opt->type == NUM)
    return new Num(((Num *)l_opt)->get_num() + ((Num *)r_opt)->get_num());

  /* Left is a num, right is an add */
  if (l_opt->type == NUM && r_opt->type == ADD) {
    Add *r_opt_add = (Add *)r_opt;
    if (r_opt_add->left->type == NUM) {
      Num *res_num = new Num(((Num *)l_opt)->get_num() +
                             ((Num *)r_opt_add->left)->get_num());
      return (new Add(res_num, r_opt_add->right));
    }

    if (r_opt_add->right->type == NUM) {
      Num *res_num = new Num(((Num *)l_opt)->get_num() +
                             ((Num *)r_opt_add->right)->get_num());
      return (new Add(res_num, r_opt_add->left));
    }
  }

  /* Right is a num, left is an add */
  if (r_opt->type == NUM && l_opt->type == ADD) {
    Add *l_opt_add = (Add *)l_opt;
    if (l_opt_add->left->type == NUM) {
      Num *res_num = new Num(((Num *)r_opt)->get_num() +
                             ((Num *)l_opt_add->left)->get_num());
      return (new Add(res_num, l_opt_add->right));
    }
    if (l_opt_add->right->type == NUM) {
      Num *res_num = new Num(((Num *)r_opt)->get_num() +
                             ((Num *)l_opt_add->right)->get_num());
      return (new Add(res_num, l_opt_add->left));
    }
  }

  return res;
}

void Num::print(std::ostream &out) { out << this->num; }

int Num::interp() { return this->num; }

Expr *Num::optimize() { return this; }

void Read::print(std::ostream &out) {
  if (this->read)
    out << this->num;
  else
    out << "READ";
}

int Read::interp() {
  if (!QUIET_READ) {
    std::cout << "read> ";
    std::cin >> this->num;
  } else {
    std::mt19937_64 rng(time(0));
    std::uniform_int_distribution<int> unii(-1e2, 1e2);
    this->num = unii(rng);
  }
  this->read = true;
  return this->num;
}

Expr *Read::optimize() {
  return this->read ? (Expr *)new Num(this->num) : (Expr *)this;
}
