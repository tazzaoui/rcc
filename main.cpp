#include <iostream>
#include "r0.hpp"

int main(int argc, char* argv[]) {
  Num n(-14);
  Neg g(&n);
  Add a(&n, &g);
  Add b(&n, &a);
  Add c(&b, &a);
  Program p(NULL, &c);
  std::cerr << "Original: " << p << std::endl;
  p.optimize();
  return 0;
}
