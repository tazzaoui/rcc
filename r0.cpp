#include <time.h>
#include <random>
#include <iostream>
#include "r0.hpp"

#define READ_DEBUG 1

std::ostream & operator << (std::ostream &out, Expr &e){ 
    e.print(out);
    return out;
} 
 
std::ostream & operator << (std::ostream &out, Program &p){ 
    out << p.expr;
    return out;
} 
 
Expr* Program::interp(){
    return this->expr->interp();
}

void Neg::print(std::ostream &out){
    out << "(-(";
    this->expr->print(out);
    out << "))";
}

Expr* Neg::interp(){
    return this->expr;
}

void Add::print(std::ostream &out){
    out << "(+ ";
    this->left->print(out); 
    out << " ";
    this->right->print(out);
    out << ")";
}

Expr* Add::interp(){
    return this->left;
}

void Num::print(std::ostream &out){
    out << this->num;
}

Expr* Num::interp(){
    return this;
}

Read::Read(){
#if !READ_DEBUG
    std::cin >> this->num;
#else
    std::mt19937_64 rng(time(0));
    std::uniform_int_distribution<int> unii(-1e2, 1e2);
    this->num = unii(rng);
#endif
} 

void Read::print(std::ostream &out){
    out << this->num;
}

Expr* Read::interp(){ 
    return new Num(this->num);
}
