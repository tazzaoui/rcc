#include <iostream>
#include "../r0.hpp"

int main(int argc, char* argv[]) {
  /* (3) Test numbers */
  Num x(42), y(-42), z(0);
  std::cout << "x = " << x << std::endl;
  std::cout << "y = " << y << std::endl;
  std::cout << "z = " << z << std::endl;

  /* (3) Test Additions */
  Add a(&x, &y);
  std::cout << a << std::endl;

  Add b(&x, &z);
  std::cout << b << std::endl;

  Add c(&a, &b);
  std::cout << c << std::endl;

  /* (3) Test Negations */
  Neg m(&x);
  std::cout << m << std::endl;

  Neg n(&a);
  std::cout << n << std::endl;

  Neg l(&c);
  std::cout << l << std::endl;

  /* (3) Test Reads */
  Read r1;
  std::cout << r1.interp() << std::endl;

  Read r2;
  Add r_add(&r2, &r1);
  std::cout << r_add.interp() << std::endl;

  Read r3;
  Neg r_neg(&r3);
  std::cout << r_neg.interp() << std::endl;

  return 0;
}
