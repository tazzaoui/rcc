#include <iostream>
#include <random>
#include "../rcc.hpp"

#define DEBUG 0

std::mt19937_64 rng(DEBUG ? 0 : time(0));
std::uniform_int_distribution<int> unii(-1024, 1024);

Expr* test_2n(int n) {
  if (n <= 0) return new Num(1);
  return new Add(test_2n(n - 1), test_2n(n - 1));
}

Expr* randp(int n) {
  int rand_num = unii(rng);
  if (n <= 0) {
    if (rand_num % 2)
      return new Read();
    else
      return new Num(rand_num);
  } else {
    if (rand_num % 2) return new Neg(randp(n - 1));
    return new Add(randp(n - 1), randp(n - 1));
  }
}

void test_interp() {
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
  std::cout << r_neg.interp();
}
