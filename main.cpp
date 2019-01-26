#include <iostream>
#include "r0.hpp"

int main(int argc, char* argv[]){
    Num x(42), y(2), z(5);
    x.print();
    std::cout << std::endl;

    Add a(&x, &y);
    a.print();
    std::cout << std::endl;

    Neg m(&z);
    m.print();
    std::cout << std::endl;

    Add b(&m, &a);
    b.print();
    std::cout << std::endl;

    Neg k(&a);
    Add c(&k, &a);  

    Read r;

    Program p(NULL, &r);
    p.print();

    return 0;
}
