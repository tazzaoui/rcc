#include <iostream>
#include <random>
#include <time.h>
#include "../r0.hpp"

#define DEBUG 1
#define NRM  "\x1B[0m"
#define RED  "\x1B[31m"
#define GRN  "\x1B[32m"

Expr* randp(int);
std::mt19937_64 rng(time(0));
std::uniform_int_distribution<int> unii(-1024, 1024);

int main(int argc, char* argv[]) { 

    for(size_t i = 0; i < 10; ++i){
        if(DEBUG)
            std::cout << "Generating random prog of depth: " << i << std::endl;
        Program prog(NULL, randp(i));
        int res = prog.interp();
        std::cout << prog << " -> " << res << std::endl;
    }
    
    return 0; 
}


Expr* randp(int n){
    int rand_num = unii(rng);
    if(n <= 0){
        if(rand_num % 2) 
            return new Read();
        else
            return new Num(rand_num);
    }else{
        if(rand_num % 2) 
            return new Neg(randp(n - 1));
        return new Add(randp(n - 1), randp(n -1));
    }
}
