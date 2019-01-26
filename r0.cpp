#include <iostream>
#include "r0.hpp"

void Program::print(){
    this->expr->print();
    std::cout << std::endl;
}

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
