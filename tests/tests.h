#ifndef TESTS_H
#define TESTS_H

/* R0 Interpreter Sanity Checks (A dozen R0 progs)*/
void test_dozen_r0(void);

/* Generates an R0 program that computes 2^N for a given N.*/
R_Expr *test_2n(int);

/* Generates a random R0 program of depth N.*/
R_Expr *randp_r0(int);

/* Generates a random R1 program of depth N */
R_Expr *randp(list_t, int);

/* Generates a random R1 program of depth N guaranteed to type check */
R_Expr *randp_typed(int);

/* Generates a random R1 expression that is guaranteed to type check */
R_Expr *rande(list_t, R_TYPE, int);

/* Generates a random let expression that is guaranteed to type check */
R_Expr *rand_let(list_t, R_TYPE, int);

/* Generates a random if expression that is guaranteed to type check */
R_Expr *rand_if(list_t, R_TYPE, int);

/* Return a random var */
R_Expr *rand_var();

/* R1 Interpreter Sanity Checks (A dozen R0 progs)*/
void test_dozen_r1(void);

/* Test linked list */
void test_list(void);

/* Test X0 Emitter */
void test_x0_emit(void);

/* Test a dozen X0 programs */
void test_dozen_x0(void);

/* Interp and output the result of an X0 program */
void test_prog_interp(X_Program *, const char *, int);

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

/* A few tests for assign-homes */
void test_assign_homes();

/* A few tests for patch-instrs */
void test_patch_instrs();

/* A few tests for uncover-live */
void test_uncover_live();

/* A few tests for build-interferences */
void test_build_interferences();

/* A few tests for color-graph */
void test_color_graph();

/* A few tests for assign-registers */
void test_assign_registers();

/* A few tests for allocate-registers */
void test_allocate_registers();

/* A few tests for move biasing */
void test_move_biasing();

/* A few tests for R2 */
void test_r2();

/* A few type checker tests */
void test_type_checker();
#endif                          /* TESTS_H */
