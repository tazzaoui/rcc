#include <time.h>
#include <random>
#include <iostream>
#include "r0.hpp"

#define READ_DEBUG 1

void Program::print(){
    this->expr->print();
    std::cout << std::endl; }

Expr* Program::interp(){
    return this->expr->interp();
}

void Neg::print(){
    std::cout << "(-(";
    this->expr->print();
    std::cout << "))";
}

Expr* Neg::interp(){
    return this->expr;
}

void Add::print(){
    std::cout << "(+ ";
    this->left->print(); 
    std::cout << " ";
    this->right->print();
    std::cout << ")";
}

Expr* Add::interp(){
    return this->left;
}

void Num::print(){
    std::cout << this->num;
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

void Read::print(){
    std::cout << this->num;
}

Expr* Read::interp(){ 
    return new Num(this->num);
}
