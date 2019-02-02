#ifndef TESTS_H
#define TESTS_H

#define NRM "\x1B[0m"
#define RED "\x1B[31m"
#define GRN "\x1B[32m"

/* R0 Interpreter Sanity Checks (A dozen R0 progs)*/
void test_dozen_r0(void);

/* Generates an R0 program that computes 2^N for a given N.*/
Expr* test_2n(int);

/* A function that generates a random R0 program of depth N.*/
Expr* randp(int);

/* R1 Interpreter Sanity Checks (A dozen R0 progs)*/
void test_dozen_r1(void);

/* Test linked list */
void test_list(void);
#endif /* TESTS_H */
