#include <iostream>
#include "../r0.hpp"

#define DEBUG 1
#define NRM  "\x1B[0m"
#define RED  "\x1B[31m"
#define GRN  "\x1B[32m"

Expr* test_2n(int);

int main(int argc, char* argv[]) { 

    for(size_t i = 0; i < 10; ++i){
        Program prog(NULL, test_2n(i));
        int res = prog.interp();
        if(DEBUG)
            std::cout << NRM << prog << std::endl;
        if(res == (1 << i))
            std::cout << GRN << res << "\t==\t"<< (1 << i) << std::endl;
        else
            std::cout << RED << res << "\t!=\t" << (1 << i) << std::endl; 
    }
    
    return 0; 
}


Expr* test_2n(int n){
    if(n <= 0) return new Num(1);
    return new Add(test_2n(n - 1), test_2n(n-1));
}
