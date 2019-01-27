#include "r0.hpp"
#include <time.h>
#include <iostream>
#include <random>

#define QUIET_READ 1

std::ostream &operator<<(std::ostream &out, Expr &e) {
  e.print(out);
  return out;
}

std::ostream &operator<<(std::ostream &out, Program &p) {
  p.expr->print(out);
  return out;
}

int Program::interp() { return this->expr->interp(); }

void Neg::print(std::ostream &out) {
  out << "(-(";
  this->expr->print(out);
  out << "))";
}

int Neg::interp() { return -1 * this->expr->interp(); }

void Add::print(std::ostream &out) {
  out << "(+ ";
  this->left->print(out);
  out << " ";
  this->right->print(out);
  out << ")";
}

int Add::interp() { return this->left->interp() + this->right->interp(); }

void Num::print(std::ostream &out) { out << this->num; }

int Num::interp() { return this->num; }

void Read::print(std::ostream &out) { out << this->num; }

int Read::interp() {
    if(!QUIET_READ){
        std::cout << "read> ";
        std::cin >> this->num;
    }else{
        std::mt19937_64 rng(time(0));
        std::uniform_int_distribution<int> unii(-1e2, 1e2);
        this->num = unii(rng);
    }   
    return this->num;
}
