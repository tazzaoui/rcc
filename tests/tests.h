#ifndef TESTS_H
#define TESTS_H

/* R0 Interpreter Sanity Checks (A dozen R0 progs)*/
void test_dozen_r0(void);

/* Generates an R0 program that computes 2^N for a given N.*/
Expr* test_2n(int);

/* A function that generates a random R0 program of depth N.*/
Expr* randp_r0(int);

/* A function that generates a random R1 program of depth N */
Expr* randp(list_t, int);

/* R1 Interpreter Sanity Checks (A dozen R0 progs)*/
void test_dozen_r1(void);

/* Test linked list */
void test_list(void);

#endif /* TESTS_H */
