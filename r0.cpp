#include <iostream>
#include "r0.hpp"

void Neg::print(){
    std::cout << "Neg!" << std::endl;
}

Expr* Neg::interp(){
    return this->expr;
}

void Add::print(){
    std::cout << "Add!" << std::endl;
}

Expr* Add::interp(){
    return this->left;
}

void Num::print(){
    std::cout << "Num!" << std::endl;
}

Expr* Num::interp(){
    return this;
}
