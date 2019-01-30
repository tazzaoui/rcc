#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include "../rcc.hpp"
#include "tests.hpp"

#define DEBUG 0
#define NUM_PROGS 1000

int main(int argc, char* argv[]) {
  int count = 0, full_count, res, res_opt, rand_depth;
  Program *prog_opt;
  srand(time(0));
  std::cout << "==================================================="
            << std::endl;
  std::cout << "Testing Interpreter...." << std::endl;
  test_interp();
  std::cout << std::endl;

  std::cout << "==================================================="
            << std::endl;
  std::cout << "Testing 2^n...." << std::endl;

  for (size_t i = 0; i < 10; ++i) {
    Program prog(NULL, test_2n(i));
    int res = prog.interp();
    if (res == (1 << i))
      std::cout << GRN << res << "\t==\t" << (1 << i) << NRM << std::endl;
    else
      std::cout << RED << res << "\t!=\t" << (1 << i) << NRM << std::endl;
  }

  std::cout << "==================================================="
            << std::endl;
  std::cout << "Testing Random Program Generation...." << std::endl;

  for (size_t i = 0; i < NUM_PROGS; ++i) {
    rand_depth = rand() % 20;
    Program prog(NULL, randp(rand() % 20));
    int res = prog.interp();
    if (DEBUG) std::cout << prog << " -> " << res << std::endl;
    ++count;
  }

  std::cout << "Successfully generated " << count << "/" << NUM_PROGS
            << " programs." << std::endl;
  assert(count == NUM_PROGS);
  std::cout << "==================================================="
            << std::endl;
  std::cout << "Add Optimizer Checks...." << std::endl;

  // Should reduce to the actual value
  Add a(new Num(7), new Num(10));
  std::cout << a << " -> " << *a.optimize() << std::endl;

  // Should combine accross the additions
  Add b(new Num(10), new Add(new Num(7), new Read()));
  Add c(new Num(10), new Add(new Read(), new Num(7)));
  Add d(new Add(new Num(42), new Read()), new Num(7));
  Add e(new Add(new Read(), new Num(42)), new Num(7));

  std::cout << b << " -> " << *b.optimize() << std::endl;
  std::cout << c << " -> " << *c.optimize() << std::endl;
  std::cout << d << " -> " << *d.optimize() << std::endl;
  std::cout << e << " -> " << *e.optimize() << std::endl;

  std::cout << "==================================================="
            << std::endl;
  std::cout << "Neg Optimizer Checks...." << std::endl;

  Neg f(new Num(7));
  Neg g(new Neg(new Add(new Num(-7), new Num(10))));
  Neg h(new Add(new Num(42), new Read()));

  std::cout << f << " -> " << *f.optimize() << std::endl;
  std::cout << g << " -> " << *g.optimize() << std::endl;
  std::cout << h << " -> " << *h.optimize() << std::endl;

  std::cout << "==================================================="
            << std::endl;
  std::cout << "General Optimizer Checks...." << std::endl;

  count = full_count = 0;
  for (size_t i = 0; i < NUM_PROGS; ++i) {
    rand_depth = rand() % 20;
    Program prog(NULL, randp(rand_depth)); 
    res = prog.interp();
    if (DEBUG) std::cout << "Normal   : " << prog << " -> " << res << std::endl;
    prog_opt = prog.optimize();
    res_opt = prog_opt->interp(); 
    if (DEBUG)
      std::cout << "Optimized: " <<  *prog_opt << std::endl;
    count += (res_opt == res);
    full_count += (res_opt == res && prog_opt->get_type() == NUM);
  }

  std::cout << GRN << "Successfully optimized " << count << "/" << NUM_PROGS << " programs." << std::endl;
  std::cout << "Fully optimized " << count << "/" << NUM_PROGS << " programs." << std::endl << NRM;
  assert(count == NUM_PROGS);
  std::cout << "==================================================="
            << std::endl;
  return 0;
}
