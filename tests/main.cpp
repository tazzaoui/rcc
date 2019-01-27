#include <iostream>
#include "../r0.hpp"
#include "tests.hpp"

#define DEBUG 0
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
   
    return 0; 

}


