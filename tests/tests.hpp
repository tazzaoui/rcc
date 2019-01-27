#ifndef TESTS_HPP
#define TESTS_HPP 

#define NRM  "\x1B[0m"
#define RED  "\x1B[31m"
#define GRN  "\x1B[32m"

/* Initial Interpreter Sanity Checks*/
void test_interp();

/* Generates an R0 program that computes 2^N for a given N.*/
Expr* test_2n(int);

/* A function that generates a random R0 program of depth N.*/
Expr* randp(int);

#endif /* TESTS_HPP */
