#include <iostream>
#include "../r0.hpp"
#include "tests.hpp"

#define DEBUG 1
#define RAND_PROGS 20

int main(int argc, char* argv[]) { 
   
    std::cout << "===================================================" << std::endl;
    std::cout << "Testing Interpreter...." << std::endl;
    test_interp();
    std::cout << std::endl;

    std::cout << "===================================================" << std::endl;
    std::cout << "Testing 2^n...." << std::endl;

    for(size_t i = 0; i < 10; ++i){
        Program prog(NULL, test_2n(i));
        int res = prog.interp();
        //std::cout << NRM << prog << std::endl;
        if(res == (1 << i))
            std::cout << GRN << res << "\t==\t"<< (1 << i) << NRM << std::endl;
        else
            std::cout << RED << res << "\t!=\t" << (1 << i) << NRM << std::endl; 
    }
    
    std::cout << "===================================================" << std::endl;
    std::cout << "Testing Random Program Generation...." << std::endl;
 
    for(size_t i = 0; i < RAND_PROGS; ++i){ 
        std::cout << "Generating random prog of depth: " << i << std::endl;
        Program prog(NULL, randp(i));
        int res = prog.interp();
        if(DEBUG)
            std::cout << prog << " -> " << res << std::endl;
    }
    
    std::cout << "===================================================" << std::endl;
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

    std::cout << "===================================================" << std::endl;
    std::cout << "Neg Optimizer Checks...." << std::endl;
   
    Neg f(new Num(7));
    Neg g(new Neg(new Add(new Num(-7), new Num(10))));
    Neg h(new Add(new Num(42), new Read()));

    std::cout << f << " -> " << *f.optimize() << std::endl;
    std::cout << g << " -> " << *g.optimize() << std::endl;
    std::cout << h << " -> " << *h.optimize() << std::endl;

    std::cout << "===================================================" << std::endl;
    std::cout << "Neg Optimizer Checks...." << std::endl;
   
    for(size_t i = 0; i < 5; ++i){
        Program prog(NULL, randp(1));
        int res = prog.interp();
        if(DEBUG)
            std::cout << "Normal   : " << prog << " -> " << res << std::endl;
        int res_opt = prog.optimize()->interp();
        if (DEBUG)
            std::cout << "Optimized: " << *prog.optimize() << " -> " << res_opt << std::endl;
        if(res == res_opt)
            std::cout << GRN << res << " == " << res_opt << NRM << std::endl;
        else
            std::cout << RED << res << " != " << res_opt << NRM << std::endl;
    }
 
    
    std::cout << "===================================================" << std::endl;
    return 0; 

}


