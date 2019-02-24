#ifndef TESTS_H
#define TESTS_H

/* R0 Interpreter Sanity Checks (A dozen R0 progs)*/
void test_dozen_r0(void);

/* Generates an R0 program that computes 2^N for a given N.*/
R_Expr* test_2n(int);

/* A function that generates a random R0 program of depth N.*/
R_Expr* randp_r0(int);

/* A function that generates a random R1 program of depth N */
R_Expr* randp(list_t, int);

/* R1 Interpreter Sanity Checks (A dozen R0 progs)*/
void test_dozen_r1(void);

/* Test linked list */
void test_list(void);

/* Test X0 Emitter */
void test_x0_emit(void);

/* Test a dozen X0 programs */
void test_dozen_x0(void);

/* Interp and output the result of an X0 program */
void test_prog_interp(X_Program*, const char*, int);

/* Test a dozen C0 programs */
void test_dozen_c0();

/* A few uniquify tests */
void test_uniquify();

/* A few rco tests */
void test_rco();

/* A few econ tests */
void test_econ();

/* A few tests for uncover-locals */
void test_uncover_locals();

/* A few tests for select-instr */
void test_select_instr();

#endif /* TESTS_H */
