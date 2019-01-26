#include <iostream>
#include "r0.hpp"

int main(int argc, char* argv[]){
    /* (3) Test numbers */
    Num x(42), y(-42), z(0);
    std::cout << "x, y, z = ";
    x.print();
    std::cout << ", ";
    y.print();
    std::cout << ", ";
    z.print();
    std::cout << std::endl;

    /* (3) Test Additions */
    Add a(&x, &y);
    a.print();
    std::cout << std::endl;

    Add b(&x, &z);
    b.print();
    std::cout << std::endl;

    Add c(&a, &b);
    c.print();
    std::cout << std::endl;

    
    /* (3) Test Negations */
    Neg m(&x);
    m.print();
    std::cout << std::endl;

    Neg n(&a);
    n.print();
    std::cout << std::endl;

    Neg l(&c);
    l.print();
    std::cout << std::endl;


    /* (3) Test Reads */
    Read r1;
    r1.print();
    std::cout << std::endl;

    Read r2;
    Add r_add(&r2, &r1);
    r_add.print();
    std::cout << std::endl;

    Read r3;
    Neg r_neg(&r3);
    r_neg.print();
    std::cout << std::endl;

    return 0;
}
